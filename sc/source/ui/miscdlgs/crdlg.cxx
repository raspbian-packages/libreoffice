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

#undef SC_DLLIMPLEMENTATION

#include "crdlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"

ScColOrRowDlg::ScColOrRowDlg(Window* pParent, const OUString& rStrTitle,
    const OUString& rStrLabel, bool bColDefault)
    : ModalDialog(pParent, "ColOrRowDialog",
        "modules/scalc/ui/colorrowdialog.ui")
{
    get(m_pBtnOk, "ok");
    get(m_pBtnCols, "columns");
    get(m_pBtnRows, "rows");
    get(m_pFrame, "frame");

    SetText(rStrTitle);
    m_pFrame->set_label(rStrLabel);

    if (bColDefault)
        m_pBtnCols->Check();
    else
        m_pBtnRows->Check();

    m_pBtnOk->SetClickHdl( LINK( this, ScColOrRowDlg, OkHdl ) );
}

IMPL_LINK_NOARG_INLINE_START(ScColOrRowDlg, OkHdl)
{
    EndDialog( m_pBtnCols->IsChecked() ? SCRET_COLS : SCRET_ROWS );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(ScColOrRowDlg, OkHdl)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
