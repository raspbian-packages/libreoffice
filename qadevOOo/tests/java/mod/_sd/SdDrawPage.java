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

package mod._sd;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.InstCreator;
import util.SOfficeFactory;
import util.ShapeDsc;

import com.sun.star.container.XIndexAccess;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XMasterPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.drawing.DrawPage</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::drawing::GenericDrawPage</code></li>
*  <li> <code>com::sun::star::drawing::XShapeBinder</code></li>
*  <li> <code>com::sun::star::drawing::XShapeGrouper</code></li>
*  <li> <code>com::sun::star::drawing::XShapes</code></li>
*  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::drawing::XMasterPageTarget</code></li>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::drawing::XShapeCombiner</code></li>
* </ul>
* @see com.sun.star.drawing.DrawPage
* @see com.sun.star.drawing.GenericDrawPage
* @see com.sun.star.drawing.XShapeBinder
* @see com.sun.star.drawing.XShapeGrouper
* @see com.sun.star.drawing.XShapes
* @see com.sun.star.lang.XServiceInfo
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.drawing.XMasterPageTarget
* @see com.sun.star.container.XNamed
* @see com.sun.star.drawing.XShapeCombiner
* @see ifc.drawing._GenericDrawPage
* @see ifc.drawing._XShapeBinder
* @see ifc.drawing._XShapeGrouper
* @see ifc.drawing._XShapes
* @see ifc.lang._XServiceInfo
* @see ifc.beans._XPropertySet
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.drawing._XMasterPageTarget
* @see ifc.container._XNamed
* @see ifc.drawing._XShapeCombiner
*/
public class SdDrawPage extends TestCase {
    XComponent xDrawDoc;

    /**
    * Creates Drawing document.
    */
    protected void initialize(TestParameters Param, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                    (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating a draw document" );
            xDrawDoc = SOF.createDrawDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes Drawing document.
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing xDrawDoc");
        util.DesktopTools.closeDoc(xDrawDoc);;
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the collection of the draw pages from the drawing document using
    * the interface <code>XDrawPagesSupplier</code> and take one of them.
    * The retrieved draw page is the instance of the service
    * <code>com.sun.star.drawing.DrawPage</code>. Inserts some shapes
    * into the draw page. Obtains the MasterPages collection. Creates
    * the rectangle shape.
    * Object relations created :
    * <ul>
    *  <li> <code>'Shape'</code> for
    *      {@link ifc.drawing._XShapes}( the created rectangle shape )</li>
    *  <li> <code>'DrawPage'</code> for
    *      {@link ifc.drawing._XShapeBinder}, {@link ifc.drawing._XShapeGrouper},
    *      {@link ifc.drawing._XShapeCombiner}( the retrieved draw page )</li>
    *  <li> <code>'MasterPageSupplier'</code> for
    *      {@link ifc.drawing._XMasterPageTarget}(the obtained master pages
    *      collection)</li>
    * </ul>
    * @see com.sun.star.drawing.DrawPage
    * @see com.sun.star.drawing.XDrawPagesSupplier
    */
    protected synchronized TestEnvironment createTestEnvironment(
                                    TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                (XMultiServiceFactory)Param.getMSF());

        // get the drawpage of drawing here
        log.println( "getting Drawpage" );
        XDrawPagesSupplier oDPS = (XDrawPagesSupplier)
            UnoRuntime.queryInterface(XDrawPagesSupplier.class, xDrawDoc);
        XDrawPages oDPn = oDPS.getDrawPages();
        XIndexAccess oDPi = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class,oDPn);
        try {
            oObj = (XDrawPage) AnyConverter.toObject(
                    new Type(XDrawPage.class),oDPi.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't get DrawPage", e );
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't get DrawPage", e );
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't get DrawPage", e );
        }

        //put something on the drawpage
        log.println( "inserting some Shapes" );
        XShapes oShapes = (XShapes) UnoRuntime.queryInterface
            (XShapes.class,oObj);
        oShapes.add(SOF.createShape
            (xDrawDoc, 2000, 1500, 1000, 1000, "Line"));
        oShapes.add(SOF.createShape
            (xDrawDoc, 3000, 4500, 15000, 1000, "Ellipse"));
        oShapes.add(SOF.createShape
            (xDrawDoc, 5000, 3500, 7500, 5000, "Rectangle"));

        //get the XMasterPagesSupplier
        log.println("get XMasterPagesSupplier");
        XMasterPagesSupplier oMPS = (XMasterPagesSupplier)
            UnoRuntime.queryInterface(XMasterPagesSupplier.class, xDrawDoc);
        XDrawPages oGroup = oMPS.getMasterPages();

        log.println( "creating a new environment for drawpage object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        XControlShape aShape = FormTools.createControlShape(
                                xDrawDoc,3000,4500,15000,10000,"CommandButton");

        oShapes.add((XShape) aShape);

        ShapeDsc sDsc = new ShapeDsc(5000, 3500, 7500, 10000, "Rectangle");
        log.println( "adding Shape as mod relation to environment" );
        tEnv.addObjRelation("Shape", new InstCreator( xDrawDoc, sDsc));
        tEnv.addObjRelation("DrawPage", oObj);
        tEnv.addObjRelation("MasterPageSupplier",oGroup);

        return tEnv;
    } // finish method createTestEnvironment

}    // finish class SdDrawPage

