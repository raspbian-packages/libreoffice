/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <tools/inetmime.hxx>

#include "rtl/textenc.h"
#include "rtl/ustring.hxx"

#include <cstdlib>
#include <iostream>

namespace {

bool testDecode(char const * input, char const * expected) {
    rtl::OUString result = INetMIME::decodeHeaderFieldBody(
        INetMIME::HEADER_FIELD_TEXT, input);
    bool success = result.equalsAscii(expected);
    if (!success) {
        std::cout
            << "FAILED: decodeHeaderFieldBody(\"" << input << "\"): \""
            << rtl::OUStringToOString(
                result, RTL_TEXTENCODING_ASCII_US).getStr()
            << "\" != \"" << expected << "\"\n";
    }
    return success;
}

}

int SAL_CALL main() {
    bool success = true;
    success &= testDecode("=?iso-8859-1?B?QQ==?=", "A");
    success &= testDecode("=?iso-8859-1?B?QUI=?=", "AB");
    success &= testDecode("=?iso-8859-1?B?QUJD?=", "ABC");
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
