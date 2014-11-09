/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svl/intitem.hxx>

#include <vcl/toolbox.hxx>
#include <sfx2/app.hxx>
#include <svx/subtoolboxcontrol.hxx>
#include <svx/svxids.hrc>


SFX_IMPL_TOOLBOX_CONTROL( SvxSubToolBoxControl, SfxUInt16Item );

SvxSubToolBoxControl::SvxSubToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
: SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

SvxSubToolBoxControl::~SvxSubToolBoxControl()
{
}

SfxPopupWindow* SvxSubToolBoxControl::CreatePopupWindow()
{
    const sal_Char* pResource = 0;
    switch( GetSlotId() )
    {
    case SID_OPTIMIZE_TABLE:
        pResource = "private:resource/toolbar/optimizetablebar";
        break;
    }

    if( pResource )
        createAndPositionSubToolBar( OUString::createFromAscii( pResource ) );
    return NULL;
}

SfxPopupWindowType  SvxSubToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
