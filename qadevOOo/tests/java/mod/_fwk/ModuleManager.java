/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package mod._fwk;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import lib.*;
import util.DesktopTools;
import util.SOfficeFactory;


import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.text.XTextDocument;

/** Test for object that implements the following interfaces :
 * <ul>
 *  <li><code>com::sun::star::frame::XModuleManager</code></li>
 *  <li><code>com::sun::star::container::XNameAccess</code></li>
 *  <li><code>com::sun::star::container::XElementAccess</code></li>
 * </ul><p>
 * @see com.sun.star.frame.XModuleManager
 * @see com.sun.star.container.XNameAccess
 * @see com.sun.star.container.XElementAccess
 * @see ifc.frame._XModuleManager
 * @see ifc.container._XNameAccess
 * @see ifc.container._XElementAccess
 */
public class ModuleManager extends TestCase {

    XTextDocument xTextDoc = null;
    XComponent xWebDoc = null;
    XComponent xGlobalDoc = null;
    XSpreadsheetDocument xSheetDoc = null;
    XComponent xDrawDoc = null;
    XComponent xImpressDoc = null;
    XComponent xMathDoc = null;

    /** Creating a Testenvironment for the interfaces to be tested.
     * @param Param test parameters
     * @param log simple logger
     * @throws StatusException was thrwon on error
     * @return the test environement
     */
    public TestEnvironment createTestEnvironment( TestParameters Param,
        PrintWriter log ) throws StatusException {

        XInterface oObj = null;

        try {
            oObj = (XInterface)((XMultiServiceFactory)Param.getMSF()).createInstance(
                "com.sun.star.comp.framework.ModuleManager");
        } catch(Exception e) {
            e.printStackTrace(log);
            throw new StatusException(
                Status.failed("Couldn't create instance"));
        }
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     (XMultiServiceFactory) Param.getMSF());

        // get text document interfaces
        try {
            log.println("creating a text document");
            xTextDoc = SOF.createTextDoc(null);

        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document ", e);
        }
        XModel xTextMode1 = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                       xTextDoc);
        XController xTextController = xTextMode1.getCurrentController();
        XFrame xTextFrame = xTextController.getFrame();


        // get webdoc interfaces
        try {
            log.println("creating a web document");
            xWebDoc = SOF.loadDocument("private:factory/swriter/web");

        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document ", e);
        }
        XModel xWebMode1 = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                       xWebDoc);
        XController xWebController = xWebMode1.getCurrentController();
        XFrame xWebFrame = xWebController.getFrame();

        // get global document interfaces
        try {
            log.println("creating a global document");
            xGlobalDoc = SOF.loadDocument("private:factory/swriter/GlobalDocument");

        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document ", e);
        }
        XModel xGlobalMode1 = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                       xGlobalDoc);
        XController xGlobalController = xGlobalMode1.getCurrentController();
        XFrame xGlobalFrame = xGlobalController.getFrame();

        // get clac interfaces
        try {
            log.println("creating a spreadsheetdocument");
            xSheetDoc = SOF.createCalcDoc(null);
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document ", e);
        }
        XModel xSheetMode1 = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                       xSheetDoc);

        XController xSheetController = xSheetMode1.getCurrentController();
        XFrame xSheetFrame = xSheetController.getFrame();
        // get draw interfaces
        try {
            log.println("creating a draw document");
            xDrawDoc = SOF.createDrawDoc(null);
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document ", e);
        }
        XModel xDrawMode1 = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                       xDrawDoc);
        XController xDrawController = xDrawMode1.getCurrentController();
        XFrame xDrawFrame = xDrawController.getFrame();

        // get impress interfaces
        try {
            log.println("creating a impress document");
            xImpressDoc = SOF.createImpressDoc(null);
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document ", e);
        }
        XModel xImpressMode1 = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                       xImpressDoc);
        XController xImpressController = xImpressMode1.getCurrentController();
        XFrame xImpressFrame = xImpressController.getFrame();

        // get math interfaces
        try {
            log.println("creating a math document");
            xMathDoc = SOF.createMathDoc(null);
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document ", e);
        }
        XModel xMathMode1 = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                       xMathDoc);
        XController xMathController = xMathMode1.getCurrentController();
        XFrame xMathFrame = xMathController.getFrame();

        PropertyValue[] xFrameSeq = new PropertyValue[7];
        xFrameSeq[0] = new PropertyValue();
        xFrameSeq[0].Name = "com.sun.star.text.TextDocument";
        xFrameSeq[0].Value = xTextFrame;
        xFrameSeq[1] = new PropertyValue();
        xFrameSeq[1].Name = "com.sun.star.text.WebDocument";
        xFrameSeq[1].Value = xWebFrame;
        xFrameSeq[2] = new PropertyValue();
        xFrameSeq[2].Name = "com.sun.star.text.GlobalDocument";
        xFrameSeq[2].Value = xGlobalFrame;
        xFrameSeq[3] = new PropertyValue();
        xFrameSeq[3].Name = "com.sun.star.sheet.SpreadsheetDocument";
        xFrameSeq[3].Value = xSheetFrame;
        xFrameSeq[4] = new PropertyValue();
        xFrameSeq[4].Name = "com.sun.star.drawing.DrawingDocument";
        xFrameSeq[4].Value = xDrawFrame;
        xFrameSeq[5] = new PropertyValue();
        xFrameSeq[5].Name = "com.sun.star.formula.FormulaProperties";
        xFrameSeq[5].Value = xMathFrame;
        xFrameSeq[6] = new PropertyValue();
        xFrameSeq[6].Name = "com.sun.star.presentation.PresentationDocument";
        xFrameSeq[6].Value = xImpressFrame;

        PropertyValue[] xControllerSeq = new PropertyValue[7];
        xControllerSeq[0] = new PropertyValue();
        xControllerSeq[0].Name = "com.sun.star.text.TextDocument";
        xControllerSeq[0].Value = xTextController;
        xControllerSeq[1] = new PropertyValue();
        xControllerSeq[1].Name = "com.sun.star.text.WebDocument";
        xControllerSeq[1].Value = xWebController;
        xControllerSeq[2] = new PropertyValue();
        xControllerSeq[2].Name = "com.sun.star.text.GlobalDocument";
        xControllerSeq[2].Value = xGlobalController;
        xControllerSeq[3] = new PropertyValue();
        xControllerSeq[3].Name = "com.sun.star.sheet.SpreadsheetDocument";
        xControllerSeq[3].Value = xSheetController;
        xControllerSeq[4] = new PropertyValue();
        xControllerSeq[4].Name = "com.sun.star.drawing.DrawingDocument";
        xControllerSeq[4].Value = xDrawController;
        xControllerSeq[5] = new PropertyValue();
        xControllerSeq[5].Name = "com.sun.star.formula.FormulaProperties";
        xControllerSeq[5].Value = xMathController;
        xControllerSeq[6] = new PropertyValue();
        xControllerSeq[6].Name = "com.sun.star.presentation.PresentationDocument";
        xControllerSeq[6].Value = xImpressController;

        PropertyValue[] xModelSeq = new PropertyValue[7];
        xModelSeq[0] = new PropertyValue();
        xModelSeq[0].Name = "com.sun.star.text.TextDocument";
        xModelSeq[0].Value = xTextMode1;
        xModelSeq[1] = new PropertyValue();
        xModelSeq[1].Name = "com.sun.star.text.WebDocument";
        xModelSeq[1].Value = xWebMode1;
        xModelSeq[2] = new PropertyValue();
        xModelSeq[2].Name = "com.sun.star.text.GlobalDocument";
        xModelSeq[2].Value = xGlobalMode1;
        xModelSeq[3] = new PropertyValue();
        xModelSeq[3].Name = "com.sun.star.sheet.SpreadsheetDocument";
        xModelSeq[3].Value = xSheetMode1;
        xModelSeq[4] = new PropertyValue();
        xModelSeq[4].Name = "com.sun.star.drawing.DrawingDocument";
        xModelSeq[4].Value = xDrawMode1;
        xModelSeq[5] = new PropertyValue();
        xModelSeq[5].Name = "com.sun.star.presentation.PresentationDocument";
        xModelSeq[5].Value = xImpressMode1;
        xModelSeq[6] = new PropertyValue();
        xModelSeq[6].Name = "com.sun.star.formula.FormulaProperties";
        xModelSeq[6].Value = xMathMode1;

        TestEnvironment tEnv = new TestEnvironment( oObj );


        tEnv.addObjRelation("XModuleManager.XFrame", xFrameSeq);
        tEnv.addObjRelation("XModuleManager.XController", xControllerSeq);
        tEnv.addObjRelation("XModuleManager.XModel", xModelSeq);
        return tEnv;
    } // finish method getTestEnvironment

    /** Disposes text document.
     * @param tParam test parameters
     * @param log simple logger
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    closing all documents " );
        DesktopTools.closeDoc(xTextDoc);
        DesktopTools.closeDoc(xWebDoc);
        DesktopTools.closeDoc(xGlobalDoc);
        DesktopTools.closeDoc(xSheetDoc);
        DesktopTools.closeDoc(xDrawDoc);
        DesktopTools.closeDoc(xImpressDoc);
        DesktopTools.closeDoc(xMathDoc);
    }
}
