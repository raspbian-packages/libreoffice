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

package mod._tdmgr;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;

public class TypeDescriptionManager extends TestCase {

    protected void initialize ( TestParameters Param, PrintWriter log) {

    }

    /**
     *    creating a Testenvironment for the interfaces to be tested
     */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
            XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(
                                                XPropertySet.class, xMSF);
            // get context
            XComponentContext xContext = (XComponentContext)
                            UnoRuntime.queryInterface(XComponentContext.class,
                            xProp.getPropertyValue("DefaultContext"));
            // get the type description manager from context
            oInterface = xContext.getValueByName("/singletons/" +
                        "com.sun.star.reflection.theTypeDescriptionManager");
        }
        catch( Exception e ) {
            log.println("Introspection Service not available" );
        }
        oObj = (XInterface) oInterface;



        log.println( "    creating a new environment for Introspection object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // Object relation for XHierarchicalNameAccess ifc
        // Name of the existing object
        tEnv.addObjRelation("ElementName", "com.sun.star.container.XNameAccess") ;
        tEnv.addObjRelation("SearchString", "com.sun.star.loader");

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class TypeDescriptionManager

