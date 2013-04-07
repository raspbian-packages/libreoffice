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

package ifc.text;

import lib.MultiPropertyTest;

import com.sun.star.text.XTextColumns;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.text.TextColumns</code>
* service properties :
* <ul>
*  <li><code> IsAutomatic</code></li>
*  <li><code> AutomaticDistance</code></li>
*  <li><code> SeparatorLineWidth</code></li>
*  <li><code> SeparatorLineColor</code></li>
*  <li><code> SeparatorLineRelativeHeight</code></li>
*  <li><code> SeparatorLineVerticalAlignment</code></li>
*  <li><code> SeparatorLineIsOn</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.text.TextColumns
*/
public class _TextColumns extends MultiPropertyTest {


    /**
    * Redefined method returns specific value, that differs from property
    * value. ( (oldValue + referenceValue) / 2 ).
    */
    public void _AutomaticDistance() {
        log.println("Testing with custom Property tester") ;
        testProperty("AutomaticDistance", new PropertyTester() {
            protected Object getNewValue(String propName, Object oldValue) {
                XTextColumns xTC = (XTextColumns)
                    UnoRuntime.queryInterface
                        (XTextColumns.class,tEnv.getTestObject());
                int ref = xTC.getReferenceValue();
                int setting = ( ( (Integer) oldValue).intValue() + ref) / 2;
                return new Integer(setting);
            }
        });
    }


} //finish class _TextColumns

