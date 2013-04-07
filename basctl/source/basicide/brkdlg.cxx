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

#include <vcl/sound.hxx>

#define _SVX_NOIDERESIDS
#include "breakpoint.hxx"
#include "brkdlg.hxx"
#include "brkdlg.hrc"
#include "basidesh.hxx"
#include "basidesh.hrc"
#include "iderdll.hxx"

#include <comphelper/string.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

// FIXME  Why does BreakPointDialog allow only sal_uInt16 for break-point line
// numbers, whereas BreakPoint supports sal_uLong?

bool lcl_ParseText( const String &rText, size_t& rLineNr )
{
    // aText should look like "# n" where
    // n > 0 && n < std::numeric_limits< sal_uInt16 >::max().
    // All spaces are ignored, so there can even be spaces within the
    // number n.  (Maybe it would be better to ignore all whitespace instead
    // of just spaces.)
    String aText = comphelper::string::remove(rText, ' ');
    sal_Unicode cFirst = aText.GetChar(0);
    if (cFirst != '#' && !(cFirst >= '0' && cFirst <= '9'))
        return false;
    if (cFirst == '#')
        aText.Erase(0, 1);
    // XXX Assumes that sal_uInt16 is contained within sal_Int32:
    sal_Int32 n = aText.ToInt32();
    if ( n <= 0 )
        return false;
    rLineNr = static_cast< size_t >(n);
    return true;
}

BreakPointDialog::BreakPointDialog( Window* pParent, BreakPointList& rBrkPntList ) :
        ModalDialog( pParent, IDEResId( RID_BASICIDE_BREAKPOINTDLG ) ),
        aComboBox(      this, IDEResId( RID_CB_BRKPOINTS ) ),
        aOKButton(      this, IDEResId( RID_PB_OK ) ),
        aCancelButton(  this, IDEResId( RID_PB_CANCEL ) ),
        aNewButton(     this, IDEResId( RID_PB_NEW ) ),
        aDelButton(     this, IDEResId( RID_PB_DEL ) ),
        aCheckBox(      this, IDEResId( RID_CHKB_ACTIVE ) ),
        aBrkText(       this, IDEResId( RID_FT_BRKPOINTS ) ),
        aPassText(      this, IDEResId( RID_FT_PASS ) ),
        aNumericField(  this, IDEResId( RID_FLD_PASS ) ),
        m_rOriginalBreakPointList(rBrkPntList),
        m_aModifiedBreakPointList(rBrkPntList)
{
    FreeResource();

    aComboBox.SetUpdateMode( sal_False );
    for ( size_t i = 0, n = m_aModifiedBreakPointList.size(); i < n; ++i )
    {
        BreakPoint* pBrk = m_aModifiedBreakPointList.at( i );
        String aEntryStr( RTL_CONSTASCII_USTRINGPARAM( "# " ) );
        aEntryStr += String::CreateFromInt32( pBrk->nLine );
        aComboBox.InsertEntry( aEntryStr, COMBOBOX_APPEND );
    }
    aComboBox.SetUpdateMode( sal_True );

    aOKButton.SetClickHdl( LINK( this, BreakPointDialog, ButtonHdl ) );
    aNewButton.SetClickHdl( LINK( this, BreakPointDialog, ButtonHdl ) );
    aDelButton.SetClickHdl( LINK( this, BreakPointDialog, ButtonHdl ) );

    aCheckBox.SetClickHdl( LINK( this, BreakPointDialog, CheckBoxHdl ) );
    aComboBox.SetSelectHdl( LINK( this, BreakPointDialog, ComboBoxHighlightHdl ) );
    aComboBox.SetModifyHdl( LINK( this, BreakPointDialog, EditModifyHdl ) );
    aComboBox.GrabFocus();

    aNumericField.SetMin( 0 );
    aNumericField.SetMax( 0x7FFFFFFF );
    aNumericField.SetSpinSize( 1 );
    aNumericField.SetStrictFormat( sal_True );
    aNumericField.SetModifyHdl( LINK( this, BreakPointDialog, EditModifyHdl ) );

    aComboBox.SetText( aComboBox.GetEntry( 0 ) );
    UpdateFields( m_aModifiedBreakPointList.at( 0 ) );

    CheckButtons();
}

void BreakPointDialog::SetCurrentBreakPoint( BreakPoint* pBrk )
{
    String aStr( RTL_CONSTASCII_USTRINGPARAM( "# " ) );
    aStr += String::CreateFromInt32( pBrk->nLine );
    aComboBox.SetText( aStr );
    UpdateFields( pBrk );
}

void BreakPointDialog::CheckButtons()
{
    // "New" button is enabled if the combo box edit contains a valid line
    // number that is not already present in the combo box list; otherwise
    // "OK" and "Delete" buttons are enabled:
    size_t nLine;
    if (lcl_ParseText(aComboBox.GetText(), nLine)
        && m_aModifiedBreakPointList.FindBreakPoint(nLine) == 0)
    {
        aNewButton.Enable();
        aOKButton.Disable();
        aDelButton.Disable();
    }
    else
    {
        aNewButton.Disable();
        aOKButton.Enable();
        aDelButton.Enable();
    }
}

IMPL_LINK_INLINE_START( BreakPointDialog, CheckBoxHdl, CheckBox *, pChkBx )
{
    BreakPoint* pBrk = GetSelectedBreakPoint();
    if ( pBrk )
        pBrk->bEnabled = pChkBx->IsChecked();

    return 0;
}
IMPL_LINK_INLINE_END( BreakPointDialog, CheckBoxHdl, CheckBox *, pChkBx )



IMPL_LINK( BreakPointDialog, ComboBoxHighlightHdl, ComboBox *, pBox )
{
    aNewButton.Disable();
    aOKButton.Enable();
    aDelButton.Enable();

    sal_uInt16 nEntry = pBox->GetEntryPos( pBox->GetText() );
    BreakPoint* pBrk = m_aModifiedBreakPointList.at( nEntry );
    DBG_ASSERT( pBrk, "Kein passender Breakpoint zur Liste ?" );
    UpdateFields( pBrk );

    return 0;
}



IMPL_LINK( BreakPointDialog, EditModifyHdl, Edit *, pEdit )
{
    if ( pEdit == &aComboBox )
        CheckButtons();
    else if ( pEdit == &aNumericField )
    {
        BreakPoint* pBrk = GetSelectedBreakPoint();
        if ( pBrk )
            pBrk->nStopAfter = pEdit->GetText().ToInt32();
    }
    return 0;
}



IMPL_LINK( BreakPointDialog, ButtonHdl, Button *, pButton )
{
    if ( pButton == &aOKButton )
    {
        m_rOriginalBreakPointList.transfer(m_aModifiedBreakPointList);
        EndDialog( 1 );
    }
    else if ( pButton == &aNewButton )
    {
        // keep checkbox in mind!
        String aText( aComboBox.GetText() );
        size_t nLine;
        bool bValid = lcl_ParseText( aText, nLine );
        if ( bValid )
        {
            BreakPoint* pBrk = new BreakPoint( nLine );
            pBrk->bEnabled = aCheckBox.IsChecked();
            pBrk->nStopAfter = (size_t) aNumericField.GetValue();
            m_aModifiedBreakPointList.InsertSorted( pBrk );
            String aEntryStr( RTL_CONSTASCII_USTRINGPARAM( "# " ) );
            aEntryStr += String::CreateFromInt32( pBrk->nLine );
            aComboBox.InsertEntry( aEntryStr, COMBOBOX_APPEND );
            BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
            SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
            SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
            if( pDispatcher )
            {
                pDispatcher->Execute( SID_BASICIDE_BRKPNTSCHANGED );
            }
        }
        else
        {
            aComboBox.SetText( aText );
            aComboBox.GrabFocus();
            Sound::Beep();
        }
        CheckButtons();
    }
    else if ( pButton == &aDelButton )
    {
        size_t nEntry = aComboBox.GetEntryPos( aComboBox.GetText() );
        BreakPoint* pBrk = m_aModifiedBreakPointList.at( nEntry );
        if ( pBrk )
        {
            delete m_aModifiedBreakPointList.remove( pBrk );
            aComboBox.RemoveEntry( nEntry );
            if ( nEntry && !( nEntry < aComboBox.GetEntryCount() ) )
                nEntry--;
            aComboBox.SetText( aComboBox.GetEntry( nEntry ) );
            BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
            SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
            SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;

            if( pDispatcher )
            {
                pDispatcher->Execute( SID_BASICIDE_BRKPNTSCHANGED );
            }
        }
        CheckButtons();
    }

    return 0;
}



void BreakPointDialog::UpdateFields( BreakPoint* pBrk )
{
    if ( pBrk )
    {
        aCheckBox.Check( pBrk->bEnabled );
        aNumericField.SetValue( pBrk->nStopAfter );
    }
}



BreakPoint* BreakPointDialog::GetSelectedBreakPoint()
{
    size_t nEntry = aComboBox.GetEntryPos( aComboBox.GetText() );
    BreakPoint* pBrk = m_aModifiedBreakPointList.at( nEntry );
    return pBrk;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
