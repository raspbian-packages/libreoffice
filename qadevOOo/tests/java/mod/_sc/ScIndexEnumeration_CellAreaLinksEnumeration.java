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

package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XAreaLinks;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.CellAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ScIndexEnumeration_CellAreaLinksEnumeration extends TestCase {
    static XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
   protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = (XComponent) UnoRuntime.queryInterface
            (XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }


    public synchronized TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log){

      XInterface oObj = null;
      TestEnvironment tEnv = null ;

      try {

        // creation of testobject here
        XPropertySet props = (XPropertySet)UnoRuntime.queryInterface
            (XPropertySet.class, xSheetDoc);
        oObj = (XInterface) AnyConverter.toObject(
                new Type(XInterface.class),props.getPropertyValue("AreaLinks")) ;
        XAreaLinks links = null ;

        // adding one link into collection (for best testing)
        links = (XAreaLinks) UnoRuntime.queryInterface(XAreaLinks.class, oObj) ;
        CellAddress addr = new CellAddress ((short) 1,2,3) ;
        String aSourceArea = util.utils.getFullTestURL("calcshapes.sxc");
        links.insertAtPosition (addr, aSourceArea, "a2:b5", "", "") ;

        XEnumerationAccess ea = (XEnumerationAccess)
                    UnoRuntime.queryInterface(XEnumerationAccess.class,oObj);

        oObj = ea.createEnumeration();

        log.println("ImplementationName: "+util.utils.getImplName(oObj));
        // creating test environment
        tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("ENUM",ea);

      } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println ("Exception occurred while creating test Object.") ;
            e.printStackTrace(log) ;
            throw new StatusException("Couldn't create test object", e);
      } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println ("Exception occurred while creating test Object.") ;
            e.printStackTrace(log) ;
            throw new StatusException("Couldn't create test object", e);
      } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println ("Exception occurred while creating test Object.") ;
            e.printStackTrace(log) ;
            throw new StatusException("Couldn't create test object", e);
      }

       return tEnv ;
    }

}

