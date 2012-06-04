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

#include <precomp.h>
#include "defdescr.hxx"


// NOT FULLY DEFINED SERVICES
#include <prprpr.hxx>




namespace cpp
{




DefineDescription::DefineDescription( const String  &       i_sName,
                                      const str_vector &    i_rDefinition )
    :   sName(i_sName),
        // aParams,
        aDefinition(i_rDefinition),
        eDefineType(type_define)
{
}

DefineDescription::DefineDescription( const String  &       i_sName,
                                      const str_vector &    i_rParams,
                                      const str_vector &    i_rDefinition )
    :   sName(i_sName),
        aParams(i_rParams),
        aDefinition(i_rDefinition),
        eDefineType(type_macro)
{
}

DefineDescription::~DefineDescription()
{
}

void
DefineDescription::GetDefineText( csv::StreamStr & o_rText ) const
{
    if ( aDefinition.begin() == aDefinition.end() OR eDefineType != type_define )
        return;


    bool bSwitch_Stringify = false;
    bool bSwitch_Concatenate = false;

    for ( str_vector::const_iterator it = aDefinition.begin();
          it != aDefinition.end();
          ++it )
    {
        if ( HandleOperatorsBeforeTextItem( o_rText,
                                            bSwitch_Stringify,
                                            bSwitch_Concatenate,
                                            *it ) )
        {
            continue;
        }

        o_rText << (*it);

        Do_bStringify_end(o_rText, bSwitch_Stringify);
        o_rText << " ";
    }
    o_rText.seekp(-1, csv::cur);
}

void
DefineDescription::GetMacroText( csv::StreamStr &               o_rText,
                                 const StringVector & i_rGivenArguments ) const
{
    bool bSwitch_Stringify = false;
    bool bSwitch_Concatenate = false;
    intt nActiveParamNr = -1;

    if ( aDefinition.begin() == aDefinition.end() OR eDefineType != type_macro )
        return;

    for ( str_vector::const_iterator it = aDefinition.begin();
          it != aDefinition.end();
          ++it )
    {
        if ( HandleOperatorsBeforeTextItem( o_rText,
                                            bSwitch_Stringify,
                                            bSwitch_Concatenate,
                                            *it ) )
        {
            continue;
        }

        for ( str_vector::const_iterator param_it = aParams.begin();
              param_it != aParams.end() AND nActiveParamNr == -1;
              ++param_it )
        {
             if ( strcmp(*it, *param_it) == 0 )
                nActiveParamNr = param_it - aParams.begin();
        }
        if ( nActiveParamNr == -1 )
        {
            o_rText << (*it);
        }
        else
        {
            o_rText << i_rGivenArguments[nActiveParamNr];
            nActiveParamNr = -1;
        }

        Do_bStringify_end(o_rText, bSwitch_Stringify);
        o_rText << " ";
    }
    o_rText.seekp(-1, csv::cur);
}



}   // end namespace cpp





bool
CheckForOperator( bool &              o_bStringify,
                  bool &              o_bConcatenate,
                  const String &      i_sTextItem )
{
    if ( strcmp(i_sTextItem, "##") == 0 )
    {
        o_bConcatenate = true;
        return true;
    }
    else if ( strcmp(i_sTextItem, "#") == 0 )
    {
        o_bStringify = true;
        return true;
    }
    return false;
}

void
Do_bConcatenate( csv::StreamStr &    o_rText,
                 bool &              io_bConcatenate )
{
    if ( io_bConcatenate )
    {
        uintt nPos;
        for ( nPos = o_rText.tellp() - 1;
              nPos > 0 ? o_rText.c_str()[nPos] == ' ' : false;
              --nPos ) ;
        o_rText.seekp(nPos+1);
        io_bConcatenate = false;
    }
}

void
Do_bStringify_begin( csv::StreamStr & o_rText,
                     bool             i_bStringify )
{
    if ( i_bStringify )
    {
        o_rText << "\"";
    }
}

void
Do_bStringify_end( csv::StreamStr & o_rText,
                   bool &           io_bStringify )
{
    if ( io_bStringify )
    {
        o_rText << "\"";
        io_bStringify = false;
    }
}


bool
HandleOperatorsBeforeTextItem( csv::StreamStr &    o_rText,
                               bool &              io_bStringify,
                               bool &              io_bConcatenate,
                               const String  &     i_sTextItem )
{
    if ( CheckForOperator( io_bStringify,
                           io_bConcatenate,
                           i_sTextItem) )
    {
        return true;
    }
    Do_bConcatenate(o_rText, io_bConcatenate);
    Do_bStringify_begin(o_rText, io_bStringify);

    return false;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
