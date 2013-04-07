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
#ifndef _TBXANCHR_HXX
#define _TBXANCHR_HXX

#include <sfx2/tbxctrl.hxx>

class SwTbxAnchor : public SfxToolBoxControl
{
    sal_uInt16 nActAnchorId;

public:

    virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void Click();
    virtual SfxPopupWindow* CreatePopupWindow();

    SFX_DECL_TOOLBOX_CONTROL();

    SwTbxAnchor( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SwTbxAnchor();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
