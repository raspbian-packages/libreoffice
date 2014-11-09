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

package lib;

/**
 * The class implements Status behaviour for exception runstate Status objects.
 */
class ExceptionStatus extends Status {

    /**
     * Creates an instance of Status object with EXCEPTION runstate.
     *
     * @param t the exception an activity terminated with.
     */
    ExceptionStatus( Throwable t ) {
        super(EXCEPTION, FAILED);
        String message = t.getMessage();
        if (message != null)
            runStateString = message;
        else
            runStateString = t.toString();
    }
}