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
package complex.forms;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.beans.XPropertiesChangeListener;
import com.sun.star.lang.EventObject;
import com.sun.star.drawing.XControlShape;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
// import complexlib.ComplexTestCase;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.XCloseable;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;
import util.FormTools;
import util.SOfficeFactory;
import util.ValueChanger;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/**
 */
public class CheckOGroupBoxModel
{

    private XMultiPropertySet m_xPropSet;
    private XComponent m_xDrawDoc;

//    public String[] getTestMethodNames() {
//        return new String[] {"setPropertyValues"};
//    }
    @Before public void before()
    {
        // XComponent xDrawDoc = null;
        SOfficeFactory SOF = SOfficeFactory.getFactory(getMSF());

        try
        {
            System.out.println("creating a draw document");
            m_xDrawDoc = SOF.createDrawDoc(null);
        }
        catch (com.sun.star.uno.Exception e)
        {
            fail("Couldn't create document.");
        }

        String objName = "GroupBox";
        XControlShape shape = FormTools.insertControlShape(m_xDrawDoc, 5000, 7000, 2000, 2000, objName);
        m_xPropSet = UnoRuntime.queryInterface(XMultiPropertySet.class, shape.getControl());
    }

    @After public void after()
    {
        XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, m_xDrawDoc);
        if (xClose != null)
        {
            try
            {
                xClose.close(true);
            }
            catch (CloseVetoException ex)
            {
                fail("Can't close document. Exception caught: " + ex.getMessage());
                /* ignore! */
            }
        }
    }
    @Test public void setPropertyValues()
    {
        String[] boundPropsToTest = getBoundPropsToTest();

        MyChangeListener ml = new MyChangeListener();
        m_xPropSet.addPropertiesChangeListener(boundPropsToTest, ml);

        Object[] gValues = m_xPropSet.getPropertyValues(boundPropsToTest);
        Object[] newValue = new Object[gValues.length];
        System.out.println("Trying to change all properties.");
        for (int i = 0; i < boundPropsToTest.length; i++)
        {
            newValue[i] = ValueChanger.changePValue(gValues[i]);
        }
        try
        {
            m_xPropSet.setPropertyValues(boundPropsToTest, newValue);
        }
        catch (com.sun.star.beans.PropertyVetoException e)
        {
            fail("Exception occured while trying to change the properties.");
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            fail("Exception occured while trying to change the properties.");
        }
        catch (com.sun.star.lang.WrappedTargetException e)
        {
            fail("Exception occured while trying to change the properties.");
        } // end of try-catch

        assertTrue("Listener was not called.", ml.wasListenerCalled());
        m_xPropSet.removePropertiesChangeListener(ml);
    }

    private String[] getBoundPropsToTest()
    {
        Property[] properties = m_xPropSet.getPropertySetInfo().getProperties();
        String[] testPropsNames = null;

        Vector<String> tNames = new Vector<String>();

        for (int i = 0; i < properties.length; i++)
        {

            Property property = properties[i];
            String name = property.Name;
            boolean isWritable = ((property.Attributes
                    & PropertyAttribute.READONLY) == 0);
            boolean isNotNull = ((property.Attributes
                    & PropertyAttribute.MAYBEVOID) == 0);
            boolean isBound = ((property.Attributes
                    & PropertyAttribute.BOUND) != 0);

            if (isWritable && isNotNull && isBound)
            {
                tNames.add(name);
            }

        } // endfor

        //get a array of bound properties
        testPropsNames = new String[tNames.size()];
        testPropsNames = tNames.toArray(testPropsNames);
        return testPropsNames;
    }

    /**
     * Listener implementation which sets a flag when
     * listener was called.
     */
    public class MyChangeListener implements XPropertiesChangeListener
    {

        boolean propertiesChanged = false;

        public void propertiesChange(PropertyChangeEvent[] e)
        {
            propertiesChanged = true;
        }

        public void disposing(EventObject obj)
        {
        }

        public boolean wasListenerCalled()
        {
            return propertiesChanged;
        }

        public void reset()
        {
            propertiesChanged = false;
        }
    };

    private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass
    public static void setUpConnection() throws Exception
    {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass
    public static void tearDownConnection()
            throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }
    private static final OfficeConnection connection = new OfficeConnection();
}
