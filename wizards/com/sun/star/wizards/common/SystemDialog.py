import uno
import traceback
from wizards.common.Configuration import Configuration
from wizards.common.Desktop import Desktop
from wizards.common.Helper import Helper

from com.sun.star.ui.dialogs.TemplateDescription import FILESAVE_AUTOEXTENSION, FILEOPEN_SIMPLE
from com.sun.star.ui.dialogs.ExtendedFilePickerElementIds import CHECKBOX_AUTOEXTENSION
from com.sun.star.awt import WindowDescriptor
from com.sun.star.awt.WindowClass import MODALTOP
from com.sun.star.lang import IllegalArgumentException
from com.sun.star.awt.VclWindowPeerAttribute import OK

class SystemDialog(object):

    '''
    @param xMSF
    @param ServiceName
    @param type  according to com.sun.star.ui.dialogs.TemplateDescription
    '''

    def __init__(self, xMSF, ServiceName, Type):
        try:
            self.xMSF = xMSF
            self.systemDialog = xMSF.createInstance(ServiceName)
            self.xStringSubstitution = self.createStringSubstitution(xMSF)

        except Exception, exception:
            traceback.print_exc()

    @classmethod
    def createStoreDialog(self, xmsf):
        return SystemDialog(
            xmsf, "com.sun.star.ui.dialogs.FilePicker",FILESAVE_AUTOEXTENSION)

    @classmethod
    def createOpenDialog(self, xmsf):
        return SystemDialog(
            xmsf, "com.sun.star.ui.dialogs.FilePicker", FILEOPEN_SIMPLE)

    @classmethod
    def createFolderDialog(self, xmsf):
        return SystemDialog(
            xmsf, "com.sun.star.ui.dialogs.FolderPicker", 0)

    @classmethod
    def createOfficeFolderDialog(self, xmsf):
        return SystemDialog(
            xmsf, "com.sun.star.ui.dialogs.OfficeFolderPicker", 0)

    def subst(self, path):
        try:
            s = self.xStringSubstitution.substituteVariables(path, False)
            return s
        except Exception, ex:
            traceback.print_exc()
            return path

    '''
    @param displayDir
    @param defaultName
    given url to a local path.
    @return
    '''

    def callStoreDialog(self, displayDir, defaultName, sDocuType=None):
        if sDocuType is not None:
            self.addFilterToDialog(defaultName[-3:], sDocuType, True)

        self.sStorePath = None
        try:
            self.systemDialog.setValue(CHECKBOX_AUTOEXTENSION, 0, True)
            self.systemDialog.setDefaultName(defaultName)
            self.systemDialog.setDisplayDirectory(self.subst(displayDir))
            if self.execute(self.systemDialog):
                sPathList = self.systemDialog.getFiles()
                self.sStorePath = sPathList[0]

        except Exception, exception:
            traceback.print_exc()

        return self.sStorePath

    def callFolderDialog(self, title, description, displayDir):
        try:
            self.systemDialog.setDisplayDirectoryxPropertyValue(
                subst(displayDir))
        except IllegalArgumentException, iae:
            traceback.print_exc()
            raise AttributeError(iae.getMessage());

        self.systemDialog.setTitle(title)
        self.systemDialog.setDescription(description)
        if self.execute(self.systemDialog):
            return self.systemDialog.getDirectory()
        else:
            return None

    def execute(self, execDialog):
        return execDialog.execute() == 1

    def callOpenDialog(self, multiSelect, displayDirectory):
        try:
            self.systemDialog.setMultiSelectionMode(multiSelect)
            self.systemDialog.setDisplayDirectory(self.subst(displayDirectory))
            if self.execute(self.systemDialog):
                return self.systemDialog.getFiles()

        except Exception, exception:
            traceback.print_exc()

        return None

    def addFilterToDialog(self, sExtension, filterName, setToDefault):
        try:
            #get the localized filtername
            uiName = self.getFilterUIName(filterName)
            pattern = "*." + sExtension
            #add the filter
            self.addFilter(uiName, pattern, setToDefault)
        except Exception, exception:
            traceback.print_exc()

    def addFilter(self, uiName, pattern, setToDefault):
        try:
            self.systemDialog.appendFilter(uiName, pattern)
            if setToDefault:
                self.systemDialog.setCurrentFilter(uiName)

        except Exception, ex:
            traceback.print_exc()

    '''
    converts the name returned from getFilterUIName_(...) so the
    product name is correct.
    @param filterName
    @return
    '''

    def getFilterUIName(self, filterName):
        prodName = Configuration.getProductName(self.xMSF)
        s = [[self.getFilterUIName_(filterName)]]
        s[0][0] = s[0][0].replace("%productname%", prodName)
        return s[0][0]

    '''
    note the result should go through conversion of the product name.
    @param filterName
    @return the UI localized name of the given filter name.
    '''

    def getFilterUIName_(self, filterName):
        try:
            oFactory = self.xMSF.createInstance(
                "com.sun.star.document.FilterFactory")
            oObject = Helper.getUnoObjectbyName(oFactory, filterName)
            xPropertyValue = list(oObject)
            for i in xPropertyValue:
                if i is not None and i.Name == "UIName":
                    return str(i.Value)

            raise NullPointerException(
                "UIName property not found for Filter " + filterName);
        except Exception, exception:
            traceback.print_exc()
            return None

    @classmethod
    def showErrorBox(self, xMSF, ResName, ResPrefix,
            ResID, AddTag=None, AddString=None):
        ProductName = Configuration.getProductName(xMSF)
        oResource = Resource(xMSF, ResPrefix)
        sErrorMessage = oResource.getResText(ResID)
        sErrorMessage = sErrorMessage.replace( ProductName, "%PRODUCTNAME")
        sErrorMessage = sErrorMessage.replace(str(13), "<BR>")
        if AddTag and AddString:
            sErrorMessage = sErrorMessage.replace( AddString, AddTag)
        return self.showMessageBox(xMSF, "ErrorBox", OK, sErrorMessage)

    '''
    example:
    (xMSF, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, "message")

    @param windowServiceName one of the following strings:
    "ErrorBox", "WarningBox", "MessBox", "InfoBox", "QueryBox".
    There are other values possible, look
    under src/toolkit/source/awt/vcltoolkit.cxx
    @param windowAttribute see com.sun.star.awt.VclWindowPeerAttribute
    @return 0 = cancel, 1 = ok, 2 = yes,  3 = no(I'm not sure here)
    other values check for yourself ;-)
    '''
    @classmethod
    def showMessageBox(self, xMSF, windowServiceName, windowAttribute,
            MessageText, peer=None):

        if MessageText is None:
            return 0

        iMessage = 0
        try:
            # If the peer is null we try to get one from the desktop...
            if peer is None:
                xFrame = Desktop.getActiveFrame(xMSF)
                peer = xFrame.getComponentWindow()

            xToolkit = xMSF.createInstance("com.sun.star.awt.Toolkit")
            oDescriptor = WindowDescriptor()
            oDescriptor.WindowServiceName = windowServiceName
            oDescriptor.Parent = peer
            oDescriptor.Type = MODALTOP
            oDescriptor.WindowAttributes = windowAttribute
            xMsgPeer = xToolkit.createWindow(oDescriptor)
            xMsgPeer.MessageText = MessageText
            iMessage = xMsgPeer.execute()
            xMsgPeer.dispose()
        except Exception:
            traceback.print_exc()

        return iMessage

    @classmethod
    def createStringSubstitution(self, xMSF):
        xPathSubst = None
        try:
            xPathSubst = xMSF.createInstance(
                "com.sun.star.util.PathSubstitution")
            return xPathSubst
        except Exception, e:
            traceback.print_exc()
            return None
