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

#ifndef _XMLOFF_ATTRTRANSFORMERACTION_HXX
#define _XMLOFF_ATTRTRANSFORMERACTION_HXX

#include "TransformerAction.hxx"

enum XMLAttrTransformerAction
{
    XML_ATACTION_EOT=XML_TACTION_EOT,   // uses for initialization only
    XML_ATACTION_COPY,                  // copy attr
    XML_ATACTION_RENAME,                // rename attr:
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_REMOVE,                // remove attr
    XML_ATACTION_IN2INCH,               // replace "in" with "inch"
    XML_ATACTION_INS2INCHS,             // replace "in" with "inch"
                                        // multiple times
    XML_ATACTION_IN2TWIPS,              // replace "in" with "inch" and
                                        // convert value from inch to twips
                                        // but only for writer documents
    XML_ATACTION_RENAME_IN2INCH,        // replace "in" with "inch" and rename
                                        // attr:
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_INCH2IN,               // replace "inch" with "in"
    XML_ATACTION_INCHS2INS,             // replace "inch" with "in"
                                        // multiple times
    XML_ATACTION_TWIPS2IN,              // replace "inch" with "in" and for writer
                                        // documents convert measure value from twips
                                        // to inch
    XML_ATACTION_RENAME_INCH2IN,        // replace "inch" with "in" and rename
                                        // attr:
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_STYLE_FAMILY,          // NOP, used for style:family
    XML_ATACTION_DECODE_STYLE_NAME,     // NOP, used for style:name
                                        // - param1: style family
    XML_ATACTION_STYLE_DISPLAY_NAME,    // NOP, used for style:display_name
                                        // - param1: style family
    XML_ATACTION_DECODE_STYLE_NAME_REF, // NOP, used for style:name reference
                                        // - param1: style family
    XML_ATACTION_RENAME_DECODE_STYLE_NAME_REF,  // NOP, used for style:name
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_ENCODE_STYLE_NAME,     // NOP, used for style:name
    XML_ATACTION_ENCODE_STYLE_NAME_REF, // NOP, used for style:name
    XML_ATACTION_RENAME_ENCODE_STYLE_NAME_REF,  // NOP, used for style:name
                                        // - param1: namespace +
                                        //           token of local name
                                        // - param2: style family
    XML_ATACTION_MOVE_TO_ELEM,          // turn attr into an elem
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_MOVE_FROM_ELEM,        // turn elem into an attr:
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_NEG_PERCENT,           // replace % val with 100-%
    XML_ATACTION_RENAME_NEG_PERCENT,    // replace % val with 100-%, rename attr
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_HREF,                  // xmlink:href
    XML_ATACTION_ADD_NAMESPACE_PREFIX,  // add a namespace prefix
                                        // - param1: prefix
    XML_ATACTION_ADD_APP_NAMESPACE_PREFIX,  // add a namespace prefix
                                        // - param1: default prefix
    XML_ATACTION_RENAME_ADD_NAMESPACE_PREFIX,   // add a namespace prefix
                                        // - param1: namespace +
                                        //           token of local name
                                        // - param2: prefix
    XML_ATACTION_REMOVE_NAMESPACE_PREFIX,// remove a namespace prefix
                                        // - param1: prefix
    XML_ATACTION_REMOVE_ANY_NAMESPACE_PREFIX,// remove any namespace prefix
    XML_ATACTION_RENAME_REMOVE_NAMESPACE_PREFIX,// remove a namespace prefix
                                        // - param1: namespace +
                                        //           token of local name
                                        // - param2: prefix
    XML_ATACTION_EVENT_NAME,
    XML_ATACTION_MACRO_NAME,
    XML_ATACTION_MACRO_LOCATION,
    XML_ATACTION_DLG_BORDER,
    XML_ATACTION_URI_OOO,                // an URI in OOo notation
                                         // - param1: pacakage URI are supported
    XML_ATACTION_URI_OASIS,              // an URI in OASIS notation
                                         // - param1: pacakage URI are supported
    XML_ATACTION_RENAME_ATTRIBUTE,       // rename up to 3 different possible values of an attrbiute
                                         // - param1: token of old attribute value (lower 16 bit)
                                         //           + token of new attribute value (upper 16 bit)
                                         // - param2: token of old attribute value
                                         //           + token of new attribute value
                                         // - param3: token of old attribute value
                                         //           + token of new attribute value
                                         // if param2 or param3 are unused they must contain
                                         // XML_TOKEN_INVALID
    XML_ATACTION_RNG2ISO_DATETIME,       // converts . into , in datetimes
    XML_ATACTION_RENAME_RNG2ISO_DATETIME,// converts . into , in datetimes and renames the attribute
                                         // - param1: namespace +
                                         //           token of local name
    XML_ATACTION_MOVE_FROM_ELEM_RNG2ISO_DATETIME, // turn elem into an attr and convert . to , in datetimes
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_SVG_WIDTH_HEIGHT_OOO,   // replace "inch" with "in" and subtracts 1/100th mm
    XML_ATACTION_SVG_WIDTH_HEIGHT_OASIS, // replace "in" with "inch" and adds 1/100th mm
    XML_ATACTION_DRAW_MIRROR_OOO,        // renames draw:mirror to style:mirror and adapts values
    // OpenDocument file format: attribute value of <style:mirror> wrong (#i49139#)
    XML_ATACTION_STYLE_MIRROR_OOO,        // adapts style:mirror values
    XML_ATACTION_DRAW_MIRROR_OASIS,      // renames style:mirror to draw:mirror and adapts values
    XML_ATACTION_GAMMA_OASIS,            // converts percentage to double value
    XML_ATACTION_GAMMA_OOO,              // converts double value to percentage
    XML_ATACTION_DECODE_ID,              // converts strings with non numeric characters to only numeric character ids
    XML_ATACTION_OPACITY_FIX,            // converts transparency to opacity and back
    XML_ATACTION_SHAPEID,                // convert shape id
    XML_ATACTION_USER_DEFINED=0x40000000,// user defined actions start here
    XML_ATACTION_END=XML_TACTION_END
};

#endif  //  _XMLOFF_ATTRTRANSFORMERACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
