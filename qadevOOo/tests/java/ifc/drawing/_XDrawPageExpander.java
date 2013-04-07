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

package ifc.drawing;

import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPageExpander;
import com.sun.star.drawing.XDrawPages;

/**
* Testing <code>com.sun.star.drawing.XDrawPageExpander</code>
* interface methods :
* <ul>
*  <li><code> expand()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'DrawPage'</code> (of type <code>XDrawPage</code>):
*   draw page to be expanded. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XDrawPageExpander
*/
public class _XDrawPageExpander extends MultiMethodTest {
    public XDrawPageExpander oObj = null;

    /**
    * First the relation is retrieved. Then the method is called.<p>
    * Has <b> OK </b> status if the method returns not null value. <p>
    */
    public void _expand() {
        boolean result = true;
        XDrawPage oDrawPage = (XDrawPage) tEnv.getObjRelation("DrawPage");
        if ( oDrawPage == null ) {
            // SKIPPED.FAILED - the ObjectRelation is not available
            Status.failed( "ObjectRelation('DrawPages') XDrawPages n.a.");
            return;
        }
        log.println("testing expand() ... ");

        XDrawPages pages = oObj.expand(oDrawPage);
        result = pages != null;

        tRes.tested("expand()", result);
    } // end expand
} // end DrawPageExpander

