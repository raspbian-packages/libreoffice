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

package org.libreoffice.android;

import android.app.Activity;
import android.app.NativeActivity;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;

import fi.iki.tml.CommandLine;

// We override NativeActivity so that we can get at the intent of the
// activity and its extra parameters, that we use to tell us what
// actual LibreOffice "program" to run. I.e. something that on desktop
// OSes would be a program, but for Androis is actually built as a
// shared object, with an "lo_main" function.

public class Bootstrap extends NativeActivity
{
    private static String TAG = "lo-bootstrap";

    private static native boolean setup(String dataDir,
                                        String apkFile,
                                        String[] ld_library_path);

    public static native boolean setup(int lo_main_ptr,
                                       Object lo_main_argument,
                                       int lo_main_delay);

    // This is not just a wrapper for the C library dlopen(), but also
    // loads recursively dependent libraries.
    public static native int dlopen(String library);

    // This is just a wrapper for the C library dlsym().
    public static native int dlsym(int handle, String symbol);

    // To be called after you are sure libgnustl_shared.so
    // has been loaded
    public static native void patch_libgnustl_shared();

    // Wrapper for getpid()
    public static native int getpid();

    // Wrapper for system()
    public static native void system(String cmdline);

    // Wrapper for putenv()
    public static native void putenv(String string);

    public static void setup(Activity activity)
    {
        String dataDir = null;

        ApplicationInfo ai = activity.getApplicationInfo();
        dataDir = ai.dataDir;
        Log.i(TAG, String.format("dataDir=%s\n", dataDir));

        String llp = System.getenv("LD_LIBRARY_PATH");
        if (llp == null)
            llp = "/vendor/lib:/system/lib";

        String[] llpa = llp.split(":");

        if (!setup(dataDir, activity.getApplication().getPackageResourcePath(), llpa))
            return;

    }

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        setup(this);

        String mainLibrary = getIntent().getStringExtra("lo-main-library");

        if (mainLibrary == null)
            mainLibrary = "libcppunittester";

        mainLibrary += ".so";

        Log.i(TAG, String.format("mainLibrary=%s", mainLibrary));

        // Get "command line" to pass to the LO "program"
        String cmdLine = getIntent().getStringExtra("lo-main-cmdline");

        if (cmdLine == null)
            cmdLine = "/data/data/org.libreoffice.android/lib/libqa_sal_types.so";

        // argv[0] will be replaced by android_main() in lo-bootstrap.c by the
        // pathname of the mainLibrary.
        cmdLine = "dummy-program-name " + cmdLine;

        Log.i(TAG, String.format("cmdLine=%s", cmdLine));

        String[] argv = CommandLine.split(cmdLine);

        // Load the LO "program" here and look up lo_main
        int loLib = dlopen(mainLibrary);

        if (loLib == 0) {
            Log.i(TAG, String.format("Could not load %s", mainLibrary));
            return;
        }

        int lo_main = dlsym(loLib, "lo_main");
        if (lo_main == 0) {
            Log.i(TAG, String.format("No lo_main in %s", mainLibrary));
            return;
        }

        // Tell lo-bootstrap to Start a strace on itself if requested
        String strace_args = getIntent().getStringExtra("lo-strace");
        if (strace_args != null)
            system("/system/xbin/strace -p " + getpid() + " " + (strace_args != "yes" ? strace_args : "" ) + " &");

        int delay = 0;
        String sdelay = getIntent().getStringExtra("lo-main-delay");
        if (sdelay != null)
            delay = Integer.parseInt(sdelay);

        // Tell lo-bootstrap.c the stuff it needs to know
        if (!setup(lo_main, argv, delay))
            return;

        // Finally, call our super-class, NativeActivity's onCreate(),
        // which eventually calls the ANativeActivity_onCreate() in
        // android_native_app_glue.c, which starts a thread in which
        // android_main() from lo-bootstrap.c is called.

        // android_main() calls the lo_main() defined in sal/main.h
        // through the function pointer passed to setup() above, with
        // the argc and argv also saved from the setup() call.
        super.onCreate(savedInstanceState);
    }

    // This is used to load the 'lo-bootstrap' library on application
    // startup. The library has already been unpacked into
    // /data/data/<app name>/lib/liblo-bootstrap.so at installation
    // time by the package manager.
    static {
        System.loadLibrary("lo-bootstrap");
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
