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

#ifndef _MULTMRK_HXX
#define _MULTMRK_HXX


#include <svx/stddlg.hxx>
#include <vcl/fixed.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>

class SwTOXMgr;

/*--------------------------------------------------------------------
     Description:   insert mark for index entry
 --------------------------------------------------------------------*/

class SwMultiTOXMarkDlg : public SvxStandardDialog
{
    DECL_LINK( SelectHdl, ListBox * );

    FixedLine           aTOXFL;
    FixedText           aEntryFT;
    FixedInfo           aTextFT;
    FixedText           aTOXFT;
    ListBox             aTOXLB;
    OKButton            aOkBT;
    CancelButton        aCancelBT;

    SwTOXMgr           &rMgr;
    sal_uInt16              nPos;

    void                Apply();
public:
    SwMultiTOXMarkDlg( Window* pParent, SwTOXMgr &rTOXMgr );
    ~SwMultiTOXMarkDlg();
};


#endif // _MULTMRK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
