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

#ifndef _XMXCOM_HXX
#define _XMXCOM_HXX

#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XXMLExtractor.hpp>

// -----------------------------------------------------------------------------

#define NMSP_CPPU       ::cppu
#define NMSP_RTL        ::rtl
#define NMSP_UNO        ::com::sun::star::uno
#define NMSP_LANG       ::com::sun::star::lang
#define NMSP_IO         ::com::sun::star::io
#define NMSP_REGISTRY   ::com::sun::star::registry


#define REF( _def_Obj )                     NMSP_UNO::Reference< _def_Obj >
#define SEQ( _def_Obj )                     NMSP_UNO::Sequence< _def_Obj >
#define ANY                                 NMSP_UNO::Any
#define B2UCONST( _def_pChar )              (NMSP_RTL::OUString(RTL_CONSTASCII_USTRINGPARAM(_def_pChar )))

#endif // _XMXCOM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
