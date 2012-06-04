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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "com/sun/star/beans/Property.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/ucb/CommandInfo.hpp"
#include "com/sun/star/uno/Sequence.hxx"

#include "myucp_content.hxx"

#ifdef IMPLEMENT_COMMAND_INSERT
#include "com/sun/star/ucb/InsertCommandArgument.hpp"
#endif

#ifdef IMPLEMENT_COMMAND_OPEN
#include "com/sun/star/ucb/OpenCommandArgument2.hpp"
#endif

using namespace com::sun::star;

// @@@ Adjust namespace name.
using namespace myucp;

//=========================================================================
//
// Content implementation.
//
//=========================================================================

//=========================================================================
//
// IMPORTENT: If any property data ( name / type / ... ) are changed, then
//            Content::getPropertyValues(...) must be adapted too!
//
//=========================================================================

// virtual
uno::Sequence< beans::Property > Content::getProperties(
    const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    // @@@ Add additional properties...

    // @@@ Note: If your data source supports adding/removing properties,
    //           you should implement the interface XPropertyContainer
    //           by yourself and supply your own logic here. The base class
    //           uses the service "com.sun.star.ucb.Store" to maintain
    //           Additional Core properties. But using server functionality
    //           is preferred! In fact you should return a table conatining
    //           even that dynamicly added properties.

//  osl::Guard< osl::Mutex > aGuard( m_aMutex );

    //=================================================================
    //
    // Supported properties
    //
    //=================================================================

    #define PROPERTY_COUNT 4

    static beans::Property aPropertyInfoTable[] =
    {
        ///////////////////////////////////////////////////////////////
        // Mandatory properties
        ///////////////////////////////////////////////////////////////
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
            -1,
            getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
            -1,
            getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            beans::PropertyAttribute::BOUND
        )
        ///////////////////////////////////////////////////////////////
        // Optional standard properties
        ///////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////
        // New properties
        ///////////////////////////////////////////////////////////////
    };
    return uno::Sequence<
            beans::Property >( aPropertyInfoTable, PROPERTY_COUNT );
}

//=========================================================================
// virtual
uno::Sequence< ucb::CommandInfo > Content::getCommands(
    const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    // @@@ Add additional commands...

//  osl::Guard< osl::Mutex > aGuard( m_aMutex );

    //=================================================================
    //
    // Supported commands
    //
    //=================================================================

    sal_uInt32 nCommandCount = 4;
#ifdef IMPLEMENT_COMMAND_DELETE
    nCommandCount++;
#endif
#ifdef IMPLEMENT_COMMAND_INSERT
    nCommandCount++;
#endif
#ifdef IMPLEMENT_COMMAND_OPEN
    nCommandCount++;
#endif

    static const ucb::CommandInfo aCommandInfoTable[] =
    {
        ///////////////////////////////////////////////////////////////
        // Mandatory commands
        ///////////////////////////////////////////////////////////////
        ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
            -1,
            getCppuVoidType()
        ),
        ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
            -1,
            getCppuVoidType()
        ),
        ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
            -1,
            getCppuType(
                static_cast< uno::Sequence< beans::Property > * >( 0 ) )
        ),
        ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
            -1,
            getCppuType(
                static_cast< uno::Sequence< beans::PropertyValue > * >( 0 ) )
        )
        ///////////////////////////////////////////////////////////////
        // Optional standard commands
        ///////////////////////////////////////////////////////////////

#ifdef IMPLEMENT_COMMAND_DELETE
        , ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
            -1,
            getCppuBooleanType()
        )
#endif
#ifdef IMPLEMENT_COMMAND_INSERT
        , ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
            -1,
            getCppuType(
                static_cast< ucb::InsertCommandArgument * >( 0 ) )
        )
#endif
#ifdef IMPLEMENT_COMMAND_OPEN
        , ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
            -1,
            getCppuType( static_cast< ucb::OpenCommandArgument2 * >( 0 ) )
        )
#endif
        ///////////////////////////////////////////////////////////////
        // New commands
        ///////////////////////////////////////////////////////////////
    };

    return uno::Sequence<
        ucb::CommandInfo >( aCommandInfoTable, nCommandCount );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
