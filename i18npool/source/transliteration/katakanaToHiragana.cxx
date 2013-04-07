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


// prevent internal compiler error with MSVC6SP3
#include <utility>

#define TRANSLITERATION_katakanaToHiragana
#include <transliteration_OneToOne.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

// see http://charts.unicode.org/Web/U3040.html Hiragana (U+3040..U+309F)
// see http://charts.unicode.org/Web/U30A0.html Katakana (U+30A0..U+30FF)
static sal_Unicode toHiragana (const sal_Unicode c)
{
    if ( (0x30a1 <= c && c <= 0x30f6) || (0x30fd <= c && c <= 0x30ff) ) { // 30A0 - 30FF KATAKANA LETTER
        // shift code point by 0x0060
        return c - (0x30a0 - 0x3040);
    }
    return c;
}

katakanaToHiragana::katakanaToHiragana()
{
    func = toHiragana;
    table = 0;
    transliterationName = "katakanaToHiragana";
    implementationName = "com.sun.star.i18n.Transliteration.KATAKANA_HIRAGANA";
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
