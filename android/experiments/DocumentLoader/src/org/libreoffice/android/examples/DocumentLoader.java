// -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-

// Version: MPL 1.1 / GPLv3+ / LGPLv3+
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License or as specified alternatively below. You may obtain a copy of
// the License at http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// Major Contributor(s):
// Copyright (C) 2011 Tor Lillqvist <tml@iki.fi> (initial developer)
// Copyright (C) 2011 SUSE Linux http://suse.com (initial developer's employer)
//
// All Rights Reserved.
//
// For minor contributions see the git repository.
//
// Alternatively, the contents of this file may be used under the terms of
// either the GNU General Public License Version 3 or later (the "GPLv3+"), or
// the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
// in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
// instead of those above.

package org.libreoffice.android.examples;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import com.sun.star.uno.UnoRuntime;

import org.libreoffice.android.Bootstrap;

public class DocumentLoader
    extends Activity {

    private static String TAG = "DocumentLoader";

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        try {

            Bootstrap.setup(this);

            Bootstrap.dlopen("libjuh.so");

            // Load more shlibs here explicitly in advance because
            // that makes debugging work better, sigh
            Bootstrap.dlopen("libuno_cppu.so");
            Bootstrap.dlopen("libuno_salhelpergcc3.so");
            Bootstrap.dlopen("libuno_cppuhelpergcc3.so");
            Bootstrap.dlopen("libbootstrap.uno.so");
            Bootstrap.dlopen("libgcc3_uno.so");
            Bootstrap.dlopen("libjava_uno.so");
            
            Bootstrap.putenv("UNO_TYPES=file:///assets/bin/udkapi.rdb file:///assets/bin/types.rdb");
            Bootstrap.putenv("UNO_SERVICES=file:///assets/xml/ure/services.rdb");
            // Bootstrap.putenv("INIFILENAME=vnd.sun.star.pathname:/assets/uno.ini");

            com.sun.star.uno.XComponentContext xContext = null;

            xContext = com.sun.star.comp.helper.Bootstrap.defaultBootstrap_InitialComponentContext();

            Log.i(TAG, "xContext is" + (xContext!=null ? " not" : "") + " null");

            com.sun.star.lang.XMultiComponentFactory xMCF =
                xContext.getServiceManager();

            Object oDesktop = xMCF.createInstanceWithContext(
                "com.sun.star.frame.Desktop", xContext);

            Log.i(TAG, "oDesktop is" + (oDesktop!=null ? " not" : "") + " null");

            com.sun.star.frame.XComponentLoader xCompLoader =
                (com.sun.star.frame.XComponentLoader)
                     UnoRuntime.queryInterface(
                         com.sun.star.frame.XComponentLoader.class, oDesktop);

            Log.i(TAG, "xCompLoader is" + (xCompLoader!=null ? " not" : "") + " null");

            String input = getIntent().getStringExtra("input");
            if (input == null)
                input = "/assets/test1.odt";

            String sUrl = "file://" + input;

            // Loading the wanted document
            com.sun.star.beans.PropertyValue propertyValues[] =
                new com.sun.star.beans.PropertyValue[1];
            propertyValues[0] = new com.sun.star.beans.PropertyValue();
            propertyValues[0].Name = "Hidden";
            propertyValues[0].Value = new Boolean(true);

            Object oDoc =
                xCompLoader.loadComponentFromURL
                (sUrl, "_blank", 0, propertyValues);
        }
        catch (Exception e) {
            e.printStackTrace(System.err);
            System.exit(1);
        }
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
