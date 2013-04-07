/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2010 Novell, Inc.
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

#include "gstmanager.hxx"

using namespace ::com::sun::star;

// -------------------
// - factory methods -
// -------------------

static uno::Reference< uno::XInterface > SAL_CALL create_MediaPlayer( const uno::Reference< lang::XMultiServiceFactory >& rxFact )
{
    return uno::Reference< uno::XInterface >( *new ::avmedia::gstreamer::Manager( rxFact ) );
}

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /*pRegistryKey*/ )
{
    uno::Reference< lang::XSingleServiceFactory > xFactory;
    void*                                   pRet = 0;

    if( rtl_str_compare( pImplName, "com.sun.star.comp.media.Manager_GStreamer" ) == 0 )
    {
        const ::rtl::OUString aServiceName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.media.Manager_GStreamer" )) );

        xFactory = uno::Reference< lang::XSingleServiceFactory >( ::cppu::createSingleFactory(
                        reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.media.Manager_GStreamer" )),
                        create_MediaPlayer, uno::Sequence< ::rtl::OUString >( &aServiceName, 1 ) ) );
    }

    if( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
