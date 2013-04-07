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
#ifndef _SFX_ARRDECL_HXX
#define _SFX_ARRDECL_HXX

#include <svl/svarray.hxx>
#include <sfx2/minarray.hxx>
#include <vector>

struct CntUpdateResult;

SV_DECL_PTRARR_DEL(CntUpdateResults_Impl, CntUpdateResult*, 4, 4)

class SfxObjectShell;
SV_DECL_PTRARR( SfxObjectShellArr_Impl, SfxObjectShell*, 4, 4 )

class SfxViewFrame;
SV_DECL_PTRARR( SfxViewFrameArr_Impl, SfxViewFrame*, 4, 4 )

class SfxViewShell;
SV_DECL_PTRARR( SfxViewShellArr_Impl, SfxViewShell*, 4, 4 )

class SfxObjectFactory;
typedef SfxObjectFactory* SfxObjectFactoryPtr;
SV_DECL_PTRARR( SfxObjectFactoryArr_Impl, SfxObjectFactoryPtr, 3, 3 )

struct SfxTbxCtrlFactory;
SV_DECL_PTRARR_DEL( SfxTbxCtrlFactArr_Impl, SfxTbxCtrlFactory*, 8, 4 )

struct SfxStbCtrlFactory;
SV_DECL_PTRARR_DEL( SfxStbCtrlFactArr_Impl, SfxStbCtrlFactory*, 8, 4 )

struct SfxMenuCtrlFactory;
SV_DECL_PTRARR_DEL( SfxMenuCtrlFactArr_Impl, SfxMenuCtrlFactory*, 2, 2 )

struct SfxChildWinFactory;
SV_DECL_PTRARR_DEL( SfxChildWinFactArr_Impl, SfxChildWinFactory*, 2, 2 )

class SfxModule;
SV_DECL_PTRARR( SfxModuleArr_Impl, SfxModule*, 2, 2 )

class SfxFilter;
DECL_PTRARRAY( SfxFilterArr_Impl, SfxFilter*, 4, 4 )

class SfxFrame;
typedef SfxFrame* SfxFramePtr;
SV_DECL_PTRARR( SfxFrameArr_Impl, SfxFramePtr, 4, 4 )

typedef ::std::vector< SfxFilter* > SfxFilterList_Impl;

struct SfxExternalLib_Impl;
typedef SfxExternalLib_Impl* SfxExternalLibPtr;
SV_DECL_PTRARR_DEL( SfxExternalLibArr_Impl, SfxExternalLibPtr, 2, 2 )

class SfxSlot;
typedef SfxSlot* SfxSlotPtr;
SV_DECL_PTRARR( SfxSlotArr_Impl, SfxSlotPtr, 20, 20 )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
