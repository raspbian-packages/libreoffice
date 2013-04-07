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

#ifndef __SBX_SBX_FACTORY_HXX
#define __SBX_SBX_FACTORY_HXX

#include <basic/sbxdef.hxx>
#include "basicdllapi.h"

class SbxBase;
class SbxObject;
class String;
class UniString;

class BASIC_DLLPUBLIC SbxFactory
{
    sal_Bool bHandleLast;   // sal_True: Factory is asked at last because of its expensiveness
public:
    SbxFactory( sal_Bool bLast=sal_False )  { bHandleLast = bLast; }
    sal_Bool IsHandleLast( void )       { return bHandleLast; }
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX );
    virtual SbxObject* CreateObject( const String& );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
