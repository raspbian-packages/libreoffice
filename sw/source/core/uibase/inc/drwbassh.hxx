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

#ifndef INCLUDED_SW_SOURCE_CORE_UIBASE_INC_DRWBASSH_HXX
#define INCLUDED_SW_SOURCE_CORE_UIBASE_INC_DRWBASSH_HXX

#include "basesh.hxx"

class SwView;
class SfxItemSet;
class SwDrawBase;
class AbstractSvxNameDialog;
struct SvxSwFrameValidation;

class SwDrawBaseShell: public SwBaseShell
{
    SwDrawBase* pDrawActual;

    sal_uInt16  eDrawMode;

    DECL_LINK( CheckGroupShapeNameHdl, AbstractSvxNameDialog* );
    DECL_LINK(ValidatePosition, SvxSwFrameValidation* );
public:
                SwDrawBaseShell(SwView &rShell);
    virtual     ~SwDrawBaseShell();

    SFX_DECL_INTERFACE(SW_DRAWBASESHELL)
    TYPEINFO_OVERRIDE();

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    void        Execute(SfxRequest &);
    void        GetState(SfxItemSet &);
    void        GetDrawAttrStateForIFBX( SfxItemSet& rSet );
    void        DisableState(SfxItemSet &rSet)               { Disable(rSet);}
    bool    Disable(SfxItemSet& rSet, sal_uInt16 nWhich = 0);

    void        StateStatusline(SfxItemSet &rSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
