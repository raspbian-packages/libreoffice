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

package ifc.sdbcx;

import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.sdbcx.XDropCatalog;

/**
* Testing <code>com.sun.star.sdbcx.XDropCatalog</code>
* interface methods :
* <ul>
*  <li><code>dropCatalog()</code></li>
* </ul> <p>
* @see com.sun.star.sdbcx.XDropCatalog
*/
public class _XDropCatalog extends MultiMethodTest {
    // oObj filled by MultiMethodTest
    public XDropCatalog oObj = null;

    public void _dropCatalog() {
        tRes.tested("dropCatalog()", Status.skipped(false));
    }
}