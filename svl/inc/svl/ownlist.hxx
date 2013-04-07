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

#ifndef _OWNLIST_HXX
#define _OWNLIST_HXX

#include "svl/svldllapi.h"
#include <tools/stream.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <vector>

namespace com { namespace sun { namespace star {
    namespace beans {
        struct PropertyValue;
    }
}}}

//=========================================================================
class SvCommand
/*  [Beschreibung]

    Enth"alt einen String, welcher das Kommando angibt und eine weiteren
    String, der das Argument des Kommandos bildet. W"urde solch ein
    Kommando "uber die Kommandozeile angegeben werden, s"ahe es wie folgt
    aus: Kommando = Argument.
*/
{
    String  aCommand;
    String  aArgument;
public:
                    SvCommand() {}
                    SvCommand( const String & rCommand, const String & rArg )
                    {
                        aCommand = rCommand;
                        aArgument = rArg;
                    }
    const String & GetCommand() const { return aCommand; }
    const String & GetArgument() const { return aArgument; }
};

typedef ::std::vector< SvCommand > SvCommandList_impl;

//=========================================================================
class SVL_DLLPUBLIC SvCommandList
/*  [Beschreibung]

    Die Liste enth"alt Objekte vom Typ SvCommand. Wird ein Objekt
    eingef"ugt, dann wird es kopiert und das neue Objekt wird
    in die Liste gestellt.
*/
{
private:
    SvCommandList_impl  aCommandList;

public:
    SvCommand&      Append( const String & rCommand, const String & rArg );
    sal_Bool        AppendCommands( const String & rCmd, sal_uInt16 * pEaten );
    String          GetCommands() const;

    sal_Bool FillFromSequence( const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& );
    void FillSequence( com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& );

    size_t          size() const { return aCommandList.size(); }

    SvCommand       operator[]( size_t i) {
                        return aCommandList[ i ];
                    }

    void            clear() {
                        aCommandList.clear();
                    }
};

#endif // _OWNLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
