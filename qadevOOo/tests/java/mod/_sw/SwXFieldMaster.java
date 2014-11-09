/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.FieldMaster.Database</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::text::TextFieldMaster</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.text.TextFieldMaster
 * @see com.sun.star.text.FieldMaster.Database
 * @see com.sun.star.beans.XPropertySet
 * @see ifc.text._TextFieldMaster
 * @see ifc.beans._XPropertySet
 */
public class SwXFieldMaster extends TestCase {
    XTextDocument xTextDoc;

    /**
    * Creates text document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes text document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }


    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.text.FieldMaster.Database</code>.
    */
    public TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log ) throws StatusException {
        XInterface oObj = null;

        log.println( "creating a test environment" );
        try {
            XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface( XMultiServiceFactory.class, xTextDoc );
            oObj = (XInterface)
                oDocMSF.createInstance( "com.sun.star.text.FieldMaster.Database" );
        } catch ( com.sun.star.uno.Exception e ) {
            log.println("Couldn't create instance");
            e.printStackTrace(log);
        }

        log.println( "creating a new environment for FieldMaster object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );
        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXFieldMaster

