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

#include <vcl/wrkwin.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/introwin.hxx>

#include <impbmp.hxx>
#include <svdata.hxx>
#include <salframe.hxx>

void IntroWindow::ImplInitIntroWindowData()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpIntroWindow = this;
}

IntroWindow::IntroWindow( ) :
    WorkWindow( WINDOW_INTROWINDOW )
{
    ImplInitIntroWindowData();
    WorkWindow::ImplInit( 0, WB_INTROWIN, NULL );
}

IntroWindow::~IntroWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->mpIntroWindow == this )
        pSVData->mpIntroWindow = NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
