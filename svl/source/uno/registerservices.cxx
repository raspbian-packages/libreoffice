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

#include "sal/types.h"
#include "rtl/ustring.hxx"
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <svl/svldllapi.h>

namespace css = com::sun::star;
using css::uno::Reference;
using css::uno::Sequence;
using rtl::OUString;

// -------------------------------------------------------------------------------------

#define DECLARE_CREATEINSTANCE( ImplName ) \
    Reference< css::uno::XInterface > SAL_CALL ImplName##_CreateInstance( const Reference< css::lang::XMultiServiceFactory >& );

DECLARE_CREATEINSTANCE( SvNumberFormatterServiceObj )
DECLARE_CREATEINSTANCE( SvNumberFormatsSupplierServiceObject )
DECLARE_CREATEINSTANCE( PathService )

// -------------------------------------------------------------------------------------

extern "C"
{

SVL_DLLPUBLIC void* SAL_CALL svl_component_getFactory (
    const sal_Char * pImplementationName, void * _pServiceManager, void * /* _pRegistryKey*/)
{
    void * pResult = 0;
    if ( _pServiceManager )
    {
        Reference< css::lang::XSingleServiceFactory > xFactory;
        if (rtl_str_compare(
                pImplementationName,
                "com.sun.star.uno.util.numbers.SvNumberFormatsSupplierServiceObject") == 0)
        {
            Sequence< OUString > aServiceNames(1);
            aServiceNames.getArray()[0] =
                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.NumberFormatsSupplier"));

            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< css::lang::XMultiServiceFactory* >(_pServiceManager),
                OUString::createFromAscii( pImplementationName ),
                SvNumberFormatsSupplierServiceObject_CreateInstance,
                aServiceNames);
        }
        else if (rtl_str_compare(
                     pImplementationName,
                     "com.sun.star.uno.util.numbers.SvNumberFormatterServiceObject") == 0)
        {
            Sequence< OUString > aServiceNames(1);
            aServiceNames.getArray()[0] =
                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.NumberFormatter"));

            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< css::lang::XMultiServiceFactory* >(_pServiceManager),
                OUString::createFromAscii( pImplementationName ),
                SvNumberFormatterServiceObj_CreateInstance,
                aServiceNames);
        }
        else if (rtl_str_compare (
                     pImplementationName, "com.sun.star.comp.svl.PathService") == 0)
        {
            Sequence< OUString > aServiceNames(1);
            aServiceNames.getArray()[0] =
                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.config.SpecialConfigManager"));
            xFactory = ::cppu::createSingleFactory (
                reinterpret_cast< css::lang::XMultiServiceFactory* >( _pServiceManager ),
                OUString::createFromAscii( pImplementationName ),
                PathService_CreateInstance,
                aServiceNames);
        }
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pResult = xFactory.get();
        }
    }
    return pResult;
}

}   // "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
