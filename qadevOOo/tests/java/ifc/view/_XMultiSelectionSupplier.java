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

package ifc.view;

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XEnumeration;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.view.XMultiSelectionSupplier;
import java.util.Comparator;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

/**
 * Testing <code>com.sun.star.view.XSelectionSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> addSelection()</code></li>
 *  <li><code> removeSelection()</code></li>
 *  <li><code> clearSelection()</code></li>
 *  <li><code> getSelectionCount()</code></li>
 *  <li><code> createSelectionEnumeration()</code></li>
 *  <li><code> createReverseSelectionEnumeration()</code></li>
 * </ul>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'Selections'</code> of type <code>Object[]</code> :
 *   the array of the instances which can be selected.</li>
 *  <li> <code>'Comparer'</code> of type <code>Comparator</code> :
 *   the interface for comparing of selected instances</li>
 * <ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.view.XSelectionSupplier
 */
public class _XMultiSelectionSupplier extends MultiMethodTest {

    public XMultiSelectionSupplier oObj = null;
    public boolean selectionChanged = false;
    Object[] selections = null;
    Comparator ObjCompare = null;

    protected void before() {
        selections = (Object[])tEnv.getObjRelation("Selections");
        if (selections == null) {
            throw new StatusException(Status.failed(
                "Couldn't get relation 'Selections'"));
        }

        ObjCompare = (Comparator)tEnv.getObjRelation("Comparer");
    }

    protected void after() {
        disposeEnvironment();
    }

    /**
     * Selects an instance from relation 'First'. <p>
     * Has <b> OK </b> status if no exceptions were thrown. <p>
     */
    public void _addSelection() {

        boolean bOK = true;

        log.println("clear selections");
        oObj.clearSelection();

        int count = oObj.getSelectionCount();

        bOK &= (count == 0);

        if ( ! bOK) log.println("ERROR: after clear selection I got a selection count of '" + count + "' => FAILED");

        for(int i = 0; i < selections.length; i++) {
            try {
                log.println("select object from object relation 'selections["+i+"]'");
                oObj.addSelection(selections[i]);
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                log.println("ERROR: could not add selection from object relation 'selections[" + i + "]': " + ex.toString());
                bOK = false;
            }
            count = oObj.getSelectionCount();
            if (count != (i+1)){
                log.println("ERROR: add a selection but selection count ("+count+ ") " +
                    "is not as expected (" + (i+1) + ") => FAILED");
                bOK = false;
            }
        }

        log.println("try to select object relation 'selections[0]' second time...");
        try {
            count = oObj.getSelectionCount();
            oObj.addSelection(selections[0]);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("ERROR: could not add selection from object relation 'selections[0] a second time': " + ex.toString());
        }
        if (count != oObj.getSelectionCount()){
            log.println("ERROR: the selected count ("+oObj.getSelectionCount() +") is not that before (" + count + ")");
            bOK = false;
        }

        log.println("try to select invalid object...");
        try {

            oObj.addSelection(oObj);

            log.println("ERORR: expected exception 'com.sun.star.lang.IllegalArgumentException' was not thrown => FAILED");
            bOK = false;
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("expected exception 'com.sun.star.lang.IllegalArgumentException' => OK");
        }

        tRes.tested("addSelection()", bOK);
    }

    public void _removeSelection() {
        requiredMethod("addSelection()");

        boolean bOK = true;

        log.println("clear selections");
        oObj.clearSelection();

        int count = oObj.getSelectionCount();

        bOK &= (count == 0);

        if ( ! bOK) log.println("ERROR: after clear selection I got a selection count of '" + count + "' => FAILED");

        log.println("add some selections...");
        for(int i = 0; i < selections.length; i++) {
            try {
                log.println("select object from object relation 'selections["+i+"]'");
                oObj.addSelection(selections[i]);
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                log.println("ERROR: could not add selection from object relation 'selections[" + i + "]': " + ex.toString());
                bOK = false;
            }
            count = oObj.getSelectionCount();
            if (count != (i+1)){
                log.println("ERROR: added a selection but selection count ("+count+ ") " +
                    "is not as expected (" + (i+1) + ") => FAILED");
                bOK = false;
            }
        }

        log.println("try now to remove selections...");

        count = oObj.getSelectionCount();
        int oldCount = oObj.getSelectionCount();
        for(int i = 0; i < selections.length; i++) {
            try {
                log.println("remove selection for object relation 'selections["+i+"]'");
                oObj.removeSelection(selections[i]);
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                log.println("ERROR: could not remove selection from object relation 'selections[" + i + "]': " + ex.toString());
                bOK = false;
            }
            count = oObj.getSelectionCount();
            if (count !=  (oldCount - i - 1)){
                log.println("ERROR: removed a selection but selection count ("+count+ ") " +
                    "is not as expected (" + (oldCount -i -1) + ") => FAILED");
                bOK = false;
            }
        }

        log.println("try to remove a removed selection a second time...");
        count = oObj.getSelectionCount();
        try {
            oObj.removeSelection(selections[0]);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("ERROR: could not remove selection from object relation 'selections[0] a second time': " + ex.toString());
        }
        if (count != oObj.getSelectionCount()){
            log.println("ERROR: the selected count ("+oObj.getSelectionCount() +") is not that before (" + count + ")");
            bOK = false;
        }

        log.println("try to remove invalid object...");
        try {

            oObj.removeSelection(oObj);

            log.println("ERORR: expected exception 'com.sun.star.lang.IllegalArgumentException' was not thrown => FAILED");
            bOK = false;
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("expected exception 'com.sun.star.lang.IllegalArgumentException' => OK");
        }

        tRes.tested("removeSelection()", bOK);
    }


    /**
     * First test changes selection of the object : if nothing is
     * currently selected or first instance ('First' relation) is
     * selected then selects second instance; if second instance
     * is currently selected then the first instance is selected. <p>
     * Then <code>getSelection</code> is called and values set and
     * get are compared. Comparison has some special cases. For
     * example if selection is a Cell, then the values contained
     * in cells are compared. <p>
     * Has <b>OK</b> status if selection changed properly.
     */
    public void _getSelectionCount() {
        requiredMethod("addSelection()");
        tRes.tested("getSelectionCount()", true);
    }

    public void _clearSelection() {
        requiredMethod("addSelection()");
        boolean bOK = true;

        log.println("clear selections");
        oObj.clearSelection();

        int count = oObj.getSelectionCount();

        bOK &= (count == 0);

        if ( ! bOK) log.println("ERROR: after clear selection I got a selection count of '" + count + "' => FAILED");

        log.println("add some selections...");
        for(int i = 0; i < selections.length; i++) {
            try {
                log.println("select object from object relation 'selections["+i+"]'");
                oObj.addSelection(selections[i]);
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                log.println("ERROR: could not add selection from object relation 'selections[" + i + "]': " + ex.toString());
                bOK = false;
            }
            count = oObj.getSelectionCount();
            if (count != (i+1)){
                log.println("ERROR: added a selection but selection count ("+count+ ") " +
                    "is not as expected (" + (i+1) + ") => FAILED");
                bOK = false;
            }
        }

        count = oObj.getSelectionCount();

        log.println("clear selections...");
        oObj.clearSelection();

        count = oObj.getSelectionCount();

        bOK &= (count == 0);

        if ( ! bOK) log.println("ERROR: after clear selection I got a selection count of '" + count + "' => FAILED");

        tRes.tested("clearSelection()", bOK);
    }

    public void _createSelectionEnumeration() {
        requiredMethod("addSelection()");
        boolean bOK = true;

        log.println("clear selections");
        oObj.clearSelection();

        int count = oObj.getSelectionCount();

        bOK &= (count == 0);

        if ( ! bOK) log.println("ERROR: after clear selection I got a selection count of '" + count + "' => FAILED");

        log.println("add some selections...");
        for(int i = 0; i < selections.length; i++) {
            try {
                log.println("select object from object relation 'selections["+i+"]'");
                oObj.addSelection(selections[i]);
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                log.println("ERROR: could not add selection from object relation 'selections[" + i + "]': " + ex.toString());
                bOK = false;
            }
            count = oObj.getSelectionCount();
            if (count != (i+1)){
                log.println("ERROR: added a selection but selection count ("+count+ ") " +
                    "is not as expected (" + (i+1) + ") => FAILED");
                bOK = false;
            }
        }

        log.println("create enumeration...");
        XEnumeration xEnum = oObj.createSelectionEnumeration();

        boolean compRes = true; //compare result
        int i = 0;

        while (xEnum.hasMoreElements()){
            log.println("try to get first element..");
            Object nextElement = null;
            try {
                nextElement = xEnum.nextElement();
            } catch (WrappedTargetException ex) {
                log.println("ERROR: could not get nextElement: " + ex.toString());
                bOK = false;
            } catch (NoSuchElementException ex) {
                log.println("ERROR: could not get nextElement: " + ex.toString());
                bOK = false;
            }
            Object shouldElement = selections[i];
            i++;

            if (ObjCompare != null) {
                ObjCompare.compare(shouldElement, nextElement);
            } else {
                compRes = util.ValueComparer.equalValue(shouldElement, nextElement);
            }

            log.println("nextElement()-object and expected object 'selections["+i+"]' are equal: "+compRes);

            if (!compRes) {
                if ((selections[i]) instanceof Object[]){
                    if (((Object[])selections[i])[0] instanceof Integer) {
                        log.println("Getting: "+((Integer) ((Object[])shouldElement)[0]).intValue());
                        log.println("Expected: "+((Integer) ((Object[])selections[i])[0]).intValue());
                    }
                }
            }
            bOK &= compRes;
        }

        tRes.tested("createSelectionEnumeration()", bOK);
    }

    public void _createReverseSelectionEnumeration() {
        requiredMethod("addSelection()");
        boolean bOK = true;

        log.println("clear selections");
        oObj.clearSelection();

        int count = oObj.getSelectionCount();

        bOK &= (count == 0);

        if ( ! bOK) log.println("ERROR: after clear selection I got a selection count of '" + count + "' => FAILED");

        log.println("add some selections...");
        for(int i = 0; i < selections.length; i++) {
            try {
                log.println("select object from object relation 'selections["+i+"]'");
                oObj.addSelection(selections[i]);
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                log.println("ERROR: could not add selection from object relation 'selections[" + i + "]': " + ex.toString());
                bOK = false;
            }
            count = oObj.getSelectionCount();
            if (count != (i+1)){
                log.println("ERROR: added a selection but selection count ("+count+ ") " +
                    "is not as expected (" + (i+1) + ") => FAILED");
                bOK = false;
            }
        }

        log.println("create enumeration...");
        XEnumeration xEnum = oObj.createSelectionEnumeration();

        boolean compRes = true; //compare result
        int i = selections.length - 1;

        while (xEnum.hasMoreElements()){
            log.println("try to get first element..");
            Object nextElement = null;
            try {
                nextElement = xEnum.nextElement();
            } catch (WrappedTargetException ex) {
                log.println("ERROR: could not get nextElement: " + ex.toString());
                bOK = false;
            } catch (NoSuchElementException ex) {
                log.println("ERROR: could not get nextElement: " + ex.toString());
                bOK = false;
            }
            Object shouldElement = selections[i];
            i--;

            if (ObjCompare != null) {
                ObjCompare.compare(shouldElement, nextElement);
            } else {
                compRes = util.ValueComparer.equalValue(shouldElement, nextElement);
            }

            log.println("nextElement()-object and expected object 'selections["+i+"]' are equal: "+compRes);

            if (!compRes) {
                if ((selections[i]) instanceof Object[]){
                    if (((Object[])selections[i])[0] instanceof Integer) {
                        log.println("Getting: "+((Integer) ((Object[])shouldElement)[0]).intValue());
                        log.println("Expected: "+((Integer) ((Object[])selections[i])[0]).intValue());
                    }
                }
            }
            bOK &= compRes;
        }

        tRes.tested("createReverseSelectionEnumeration()", bOK);
    }

}  // finish class _XMultiSelectionSupplier



