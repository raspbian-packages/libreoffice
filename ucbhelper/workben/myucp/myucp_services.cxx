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


#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"

#include "myucp_provider.hxx"

using namespace com::sun::star;

namespace {

//=========================================================================
sal_Bool writeInfo( void * pRegistryKey,
                    const rtl::OUString & rImplementationName,
                    uno::Sequence< rtl::OUString > const & rServiceNames )
{
    rtl::OUString aKeyName( RTL_CONSTASCII_USTRINGPARAM("/") );
    aKeyName += rImplementationName;
    aKeyName += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));

    uno::Reference< registry::XRegistryKey > xKey;
    try
    {
        xKey = static_cast< registry::XRegistryKey * >(
                                    pRegistryKey )->createKey( aKeyName );
    }
    catch ( registry::InvalidRegistryException const & )
    {
    }

    if ( !xKey.is() )
        return sal_False;

    sal_Bool bSuccess = sal_True;

    for ( sal_Int32 n = 0; n < rServiceNames.getLength(); ++n )
    {
        try
        {
            xKey->createKey( rServiceNames[ n ] );
        }
        catch ( registry::InvalidRegistryException const & )
        {
            bSuccess = sal_False;
            break;
        }
    }
    return bSuccess;
}

}

//=========================================================================
extern "C" sal_Bool SAL_CALL component_writeInfo(
    void * /*pServiceManager*/, void * pRegistryKey )
{
    return pRegistryKey &&

    //////////////////////////////////////////////////////////////////////
    // Write info into registry.
    //////////////////////////////////////////////////////////////////////

    // @@@ Adjust namespace names.
    writeInfo( pRegistryKey,
               ::myucp::ContentProvider::getImplementationName_Static(),
               ::myucp::ContentProvider::getSupportedServiceNames_Static() );
}

//=========================================================================
extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    uno::Reference< lang::XMultiServiceFactory > xSMgr(
            reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    //////////////////////////////////////////////////////////////////////
    // Create factory, if implementation name matches.
    //////////////////////////////////////////////////////////////////////

    // @@@ Adjust namespace names.
    if ( ::myucp::ContentProvider::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = ::myucp::ContentProvider::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
