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

#ifndef INCLUDED_SD_SOURCE_UI_INC_UNDOBACK_HXX
#define INCLUDED_SD_SOURCE_UI_INC_UNDOBACK_HXX

#include "sdundo.hxx"

class SdDrawDocument;
class SdPage;
class SfxItemSet;


// SdBackgroundObjUndoAction
class SdBackgroundObjUndoAction : public SdUndoAction
{
private:

    SdPage&                 mrPage;
    SfxItemSet*             mpItemSet;

    void                    ImplRestoreBackgroundObj();

public:

                            TYPEINFO_OVERRIDE();

                            SdBackgroundObjUndoAction(
                                SdDrawDocument& rDoc,
                                SdPage& rPage,
                                const SfxItemSet& rItenSet);
    virtual                 ~SdBackgroundObjUndoAction();

    virtual void            Undo() SAL_OVERRIDE;
    virtual void            Redo() SAL_OVERRIDE;

    virtual SdUndoAction*   Clone() const SAL_OVERRIDE;
};

#endif // INCLUDED_SD_SOURCE_UI_INC_UNDOBACK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
