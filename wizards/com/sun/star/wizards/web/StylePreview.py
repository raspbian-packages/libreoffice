from common.FileAccess import FileAccess
import traceback

'''
@author rpiterman
the style preview, which is a OOo Document Preview in
an Image Control.
This class copies the files needed for this
preview from the web wizard work directory
to a given temporary directory, and updates them
on request, according to the current style/background selection
of the user.
'''

class StylePreview(object):

    '''
    copies the html file to the temp directory, and calculates the
    destination names of the background and css files.
    @param wwRoot is the root directory of the web wizard files (
    usually [oo]/share/template/[lang]/wizard/web
    '''

    def __init__(self, xmsf, wwRoot_):
        self.fileAccess = FileAccess(xmsf)
        self.tempDir = self.createTempDir(xmsf)
        self.htmlFilename = FileAccess.connectURLs(
            self.tempDir, "wwpreview.html")
        self.cssFilename = FileAccess.connectURLs(self.tempDir, "style.css")
        self.backgroundFilename = FileAccess.connectURLs(
            self.tempDir, "images/background.gif")
        self.wwRoot = wwRoot_
        self.fileAccess.copy(FileAccess.connectURLs(
            self.wwRoot, "preview.html"), self.htmlFilename)

    '''
    copies the given style and background files to the temporary
    directory.
    @param style
    @param background
    @throws Exception
    '''

    def refresh(self, style, background):
        css = FileAccess.connectURLs(self.wwRoot, "styles/" + style.cp_CssHref)
        if background is None or background == "":
            #delete the background image
            if self.fileAccess.exists(self.backgroundFilename, False):
                self.fileAccess.delete(self.backgroundFilename)
        else:
            # a solaris bug workaround
            # TODO
            #copy the background image to the temp directory.
            self.fileAccess.copy(background, self.backgroundFilename)

        #copy the actual css to the temp directory
        self.fileAccess.copy(css, self.cssFilename)

    def cleanup(self):
        try:
            self.fileAccess.delete(self.tempDir)
        except Exception:
            traceback.print_exc()

    '''
    creates a temporary directory.
    @param xmsf
    @return the url of the new directory.
    @throws Exception
    '''

    def createTempDir(self, xmsf):
        tempPath = FileAccess.getOfficePath2(xmsf, "Temp", "", "")
        s = self.fileAccess.createNewDir(tempPath, "wwiz")
        self.fileAccess.createNewDir(s, "images")
        return s
