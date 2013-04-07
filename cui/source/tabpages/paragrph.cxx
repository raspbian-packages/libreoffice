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

#include <svl/style.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/module.hxx>
#include <vcl/mnemonic.hxx>
#include <svx/dialogs.hrc>

#define _SVX_PARAGRPH_CXX   0

#include <svl/languageoptions.hxx>
#include <svl/cjkoptions.hxx>
#include <editeng/pgrditem.hxx>
#include <cuires.hrc>
#include "paragrph.hrc"
#include "paragrph.hxx"
#include <editeng/frmdiritem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/pmdlitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/hyznitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/keepitem.hxx>
#include "svx/dlgutil.hxx"
#include <dialmgr.hxx>
#include "svx/htmlmode.hxx"
#include <editeng/paravertalignitem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/request.hxx>
#include <svl/intitem.hxx>

// static ----------------------------------------------------------------

static sal_uInt16 pStdRanges[] =
{
    SID_ATTR_PARA_LINESPACE,        // 10033
    SID_ATTR_PARA_LINESPACE,
    SID_ATTR_LRSPACE,               // 10048 -
    SID_ATTR_ULSPACE,               // 10049
    SID_ATTR_PARA_REGISTER,         // 10413
    SID_ATTR_PARA_REGISTER,
    0
};

static sal_uInt16 pAlignRanges[] =
{
    SID_ATTR_PARA_ADJUST,           // 10027
    SID_ATTR_PARA_ADJUST,
    0
};

static sal_uInt16 pExtRanges[] =
{
    SID_ATTR_PARA_PAGEBREAK,        // 10037 -
    SID_ATTR_PARA_WIDOWS,           // 10041
    SID_ATTR_PARA_MODEL,            // 10065 -
    SID_ATTR_PARA_KEEP,             // 10066
    0
};

// define ----------------------------------------------------------------

#define MAX_DURCH 5670      // 10 cm ist sinnvoll als maximaler Durchschuss
                            // laut BP
#define FIX_DIST_DEF 283    // Standard-Fix-Abstand 0,5cm

// enum ------------------------------------------------------------------

enum LineSpaceList
{
    LLINESPACE_1    = 0,
    LLINESPACE_15   = 1,
    LLINESPACE_2    = 2,
    LLINESPACE_PROP = 3,
    LLINESPACE_MIN  = 4,
    LLINESPACE_DURCH= 5,
    LLINESPACE_FIX  = 6,
    LLINESPACE_END
};

// C-Funktion ------------------------------------------------------------

void SetLineSpace_Impl( SvxLineSpacingItem&, int, long lValue = 0 );

void SetLineSpace_Impl( SvxLineSpacingItem& rLineSpace,
                        int eSpace, long lValue )
{
    switch ( eSpace )
    {
        case LLINESPACE_1:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
            break;

        case LLINESPACE_15:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetPropLineSpace( 150 );
            break;

        case LLINESPACE_2:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetPropLineSpace( 200 );
            break;

        case LLINESPACE_PROP:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetPropLineSpace( (sal_uInt8)lValue );
            break;

        case LLINESPACE_MIN:
            rLineSpace.SetLineHeight( (sal_uInt16)lValue );
            rLineSpace.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
            break;

        case LLINESPACE_DURCH:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetInterLineSpace( (sal_uInt16)lValue );
            break;

        case LLINESPACE_FIX:
            rLineSpace.SetLineHeight((sal_uInt16)lValue);
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_FIX;
            rLineSpace.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
        break;
    }
}


sal_uInt16 GetHtmlMode_Impl(const SfxItemSet& rSet)
{
    sal_uInt16 nHtmlMode = 0;
    const SfxPoolItem* pItem = 0;
    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, sal_False, &pItem) ||
        ( 0 != (pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        nHtmlMode = ((SfxUInt16Item*)pItem)->GetValue();
    }
    return nHtmlMode;

}

// class SvxStdParagraphTabPage ------------------------------------------

IMPL_LINK( SvxStdParagraphTabPage, ELRLoseFocusHdl, Edit *, EMPTYARG )
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit =
        MapToFieldUnit( pPool->GetMetric( GetWhich( SID_ATTR_LRSPACE ) ) );

    sal_Int64 nL = aLeftIndent.Denormalize( aLeftIndent.GetValue( eUnit ) );
    sal_Int64 nR = aRightIndent.Denormalize( aRightIndent.GetValue( eUnit ) );
    String aTmp = aFLineIndent.GetText();

    // Erstzeilen Einzug
    if( aLeftIndent.GetMin() < 0 )
        aFLineIndent.SetMin( -99999, FUNIT_MM );
    else
        aFLineIndent.SetMin( aFLineIndent.Normalize( -nL ), eUnit );

    // Check nur fuer konkrete Breite (Shell)
    sal_Int64 nTmp = nWidth - nL - nR - MM50;
    aFLineIndent.SetMax( aFLineIndent.Normalize( nTmp ), eUnit );

    if ( !aTmp.Len() )
        aFLineIndent.SetEmptyFieldValue();
    // Maximum Links Rechts
    aTmp = aLeftIndent.GetText();
    nTmp = nWidth - nR - MM50;
    aLeftIndent.SetMax( aLeftIndent.Normalize( nTmp ), eUnit );

    if ( !aTmp.Len() )
        aLeftIndent.SetEmptyFieldValue();
    aTmp = aRightIndent.GetText();
    nTmp = nWidth - nL - MM50;
    aRightIndent.SetMax( aRightIndent.Normalize( nTmp ), eUnit );

    if ( !aTmp.Len() )
        aRightIndent.SetEmptyFieldValue();
    return 0;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxStdParagraphTabPage::Create( Window* pParent,
                                            const SfxItemSet& rSet)
{
    return new SvxStdParagraphTabPage( pParent, rSet );
}

// -----------------------------------------------------------------------

sal_Bool SvxStdParagraphTabPage::FillItemSet( SfxItemSet& rOutSet )
{
    SfxItemState eState = SFX_ITEM_UNKNOWN;
    const SfxPoolItem* pOld = 0;
    SfxItemPool* pPool = rOutSet.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );

    sal_Bool bModified = sal_False;
    sal_uInt16 nWhich;
    sal_uInt16 nPos = aLineDist.GetSelectEntryPos();

    if ( LISTBOX_ENTRY_NOTFOUND != nPos &&
         ( nPos != aLineDist.GetSavedValue() ||
           aLineDistAtPercentBox.IsValueModified() ||
           aLineDistAtMetricBox.IsValueModified() ) )
    {
        nWhich = GetWhich( SID_ATTR_PARA_LINESPACE );
        SfxMapUnit eUnit = pPool->GetMetric( nWhich );
        SvxLineSpacingItem aSpacing(
            (const SvxLineSpacingItem&)GetItemSet().Get( nWhich ) );

        switch ( nPos )
        {
            case LLINESPACE_1:
            case LLINESPACE_15:
            case LLINESPACE_2:
                SetLineSpace_Impl( aSpacing, nPos );
                break;

            case LLINESPACE_PROP:
                SetLineSpace_Impl( aSpacing, nPos,
                                   static_cast<long>(aLineDistAtPercentBox.Denormalize(
                                   aLineDistAtPercentBox.GetValue() )) );
                break;

            case LLINESPACE_MIN:
            case LLINESPACE_DURCH:
            case LLINESPACE_FIX:
                SetLineSpace_Impl( aSpacing, nPos,
                    GetCoreValue( aLineDistAtMetricBox, eUnit ) );
            break;

            default:
                OSL_FAIL( "unbekannter Type fuer Zeilenabstand." );
                break;
        }
        eState = GetItemSet().GetItemState( nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_LINESPACE );

        if ( !pOld || !( *(const SvxLineSpacingItem*)pOld == aSpacing ) ||
             SFX_ITEM_DONTCARE == eState )
        {
            rOutSet.Put( aSpacing );
            bModified = sal_True;
        }
    }

    if ( aTopDist.IsValueModified() || aBottomDist.IsValueModified() )
    {
        nWhich = GetWhich( SID_ATTR_ULSPACE );
        SfxMapUnit eUnit = pPool->GetMetric( nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_ULSPACE );
        SvxULSpaceItem aMargin( nWhich );

        if ( bRelativeMode )
        {
            DBG_ASSERT( GetItemSet().GetParent(), "No ParentSet" );

            const SvxULSpaceItem& rOldItem =
                (const SvxULSpaceItem&)GetItemSet().GetParent()->Get( nWhich );

            if ( aTopDist.IsRelative() )
                aMargin.SetUpper( rOldItem.GetUpper(),
                                  (sal_uInt16)aTopDist.GetValue() );
            else
                aMargin.SetUpper( (sal_uInt16)GetCoreValue( aTopDist, eUnit ) );

            if ( aBottomDist.IsRelative() )
                aMargin.SetLower( rOldItem.GetLower(),
                                  (sal_uInt16)aBottomDist.GetValue() );
            else
                aMargin.SetLower( (sal_uInt16)GetCoreValue( aBottomDist, eUnit ) );

        }
        else
        {
            aMargin.SetUpper( (sal_uInt16)GetCoreValue( aTopDist, eUnit ) );
            aMargin.SetLower( (sal_uInt16)GetCoreValue( aBottomDist, eUnit ) );
        }
        eState = GetItemSet().GetItemState( nWhich );

        if ( !pOld || !( *(const SvxULSpaceItem*)pOld == aMargin ) ||
             SFX_ITEM_DONTCARE == eState )
        {
            rOutSet.Put( aMargin );
            bModified = sal_True;
        }
    }
    bool bNullTab = false;

    if ( aLeftIndent.IsValueModified() ||
         aFLineIndent.IsValueModified() ||
         aRightIndent.IsValueModified()
         ||  aAutoCB.GetSavedValue() != aAutoCB.IsChecked() )
    {
        nWhich = GetWhich( SID_ATTR_LRSPACE );
        SfxMapUnit eUnit = pPool->GetMetric( nWhich );
        SvxLRSpaceItem aMargin( nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_LRSPACE );

        if ( bRelativeMode )
        {
            DBG_ASSERT( GetItemSet().GetParent(), "No ParentSet" );

            const SvxLRSpaceItem& rOldItem =
                (const SvxLRSpaceItem&)GetItemSet().GetParent()->Get( nWhich );

            if ( aLeftIndent.IsRelative() )
                aMargin.SetTxtLeft( rOldItem.GetTxtLeft(),
                                    (sal_uInt16)aLeftIndent.GetValue() );
            else
                aMargin.SetTxtLeft( GetCoreValue( aLeftIndent, eUnit ) );

            if ( aRightIndent.IsRelative() )
                aMargin.SetRight( rOldItem.GetRight(),
                                  (sal_uInt16)aRightIndent.GetValue() );
            else
                aMargin.SetRight( GetCoreValue( aRightIndent, eUnit ) );

            if ( aFLineIndent.IsRelative() )
                aMargin.SetTxtFirstLineOfst( rOldItem.GetTxtFirstLineOfst(),
                                             (sal_uInt16)aFLineIndent.GetValue() );
            else
                aMargin.SetTxtFirstLineOfst(
                    (sal_uInt16)GetCoreValue( aFLineIndent, eUnit ) );
        }
        else
        {
            aMargin.SetTxtLeft( GetCoreValue( aLeftIndent, eUnit ) );
            aMargin.SetRight( GetCoreValue( aRightIndent, eUnit ) );
            aMargin.SetTxtFirstLineOfst(
                (sal_uInt16)GetCoreValue( aFLineIndent, eUnit ) );
        }
        aMargin.SetAutoFirst(aAutoCB.IsChecked());
        if ( aMargin.GetTxtFirstLineOfst() < 0 )
            bNullTab = true;
        eState = GetItemSet().GetItemState( nWhich );

        if ( !pOld || !( *(const SvxLRSpaceItem*)pOld == aMargin ) ||
             SFX_ITEM_DONTCARE == eState )
        {
            rOutSet.Put( aMargin );
            bModified = sal_True;
        }
    }

    if ( bNullTab )
    {
        MapUnit eUnit = (MapUnit)pPool->GetMetric( GetWhich( SID_ATTR_TABSTOP ) );
        if ( MAP_100TH_MM != eUnit )
        {

            // negativer Erstzeileneinzug -> ggf. Null Default-Tabstop setzen
            sal_uInt16 _nWhich = GetWhich( SID_ATTR_TABSTOP );
            const SfxItemSet& rInSet = GetItemSet();

            if ( rInSet.GetItemState( _nWhich ) >= SFX_ITEM_AVAILABLE )
            {
                const SvxTabStopItem& rTabItem =
                    (const SvxTabStopItem&)rInSet.Get( _nWhich );
                SvxTabStopItem aNullTab( rTabItem );
                SvxTabStop aNull( 0, SVX_TAB_ADJUST_DEFAULT );
                aNullTab.Insert( aNull );
                rOutSet.Put( aNullTab );
            }
        }
    }
    if( aRegisterCB.IsVisible())
    {
        const SfxBoolItem* pBoolItem = (SfxBoolItem*)GetOldItem(
                            rOutSet, SID_ATTR_PARA_REGISTER);
        SfxBoolItem* pRegItem = (SfxBoolItem*)pBoolItem->Clone();
        sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_REGISTER );
        sal_Bool bSet = pRegItem->GetValue();

        if(aRegisterCB.IsChecked() != bSet )
        {
            pRegItem->SetValue(!bSet);
            rOutSet.Put(*pRegItem);
            bModified = sal_True;
        }
        else if ( SFX_ITEM_DEFAULT == GetItemSet().GetItemState( _nWhich, sal_False ) )
            rOutSet.ClearItem(_nWhich);
        delete pRegItem;
    }

    return bModified;
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::Reset( const SfxItemSet& rSet )
{
    SfxItemPool* pPool = rSet.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    String aEmpty;

    // Metrik einstellen
    FieldUnit eFUnit = GetModuleFieldUnit( rSet );

    bool bApplyCharUnit = GetApplyCharUnit( rSet );

    SvtCJKOptions aCJKOptions;
    if(aCJKOptions.IsAsianTypographyEnabled() && bApplyCharUnit )
        eFUnit = FUNIT_CHAR;

    SetFieldUnit( aLeftIndent, eFUnit );
    SetFieldUnit( aRightIndent, eFUnit );
    SetFieldUnit( aFLineIndent, eFUnit );
    if ( eFUnit == FUNIT_CHAR )
    {
        SetFieldUnit( aTopDist, FUNIT_LINE );
        SetFieldUnit( aBottomDist, FUNIT_LINE );
        SetFieldUnit( aLineDistAtMetricBox, FUNIT_POINT );
    }
    else
    {
        SetFieldUnit( aTopDist, eFUnit );
        SetFieldUnit( aBottomDist, eFUnit );
        SetFieldUnit( aLineDistAtMetricBox, eFUnit );
    }

    sal_uInt16 _nWhich = GetWhich( SID_ATTR_LRSPACE );
    SfxItemState eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        SfxMapUnit eUnit = pPool->GetMetric( _nWhich );

        if ( bRelativeMode )
        {
            const SvxLRSpaceItem& rOldItem =
                (const SvxLRSpaceItem&)rSet.Get( _nWhich );

            if ( rOldItem.GetPropLeft() != 100 )
            {
                aLeftIndent.SetRelative( sal_True );
                aLeftIndent.SetValue( rOldItem.GetPropLeft() );
            }
            else
            {
                aLeftIndent.SetRelative();
                SetFieldUnit( aLeftIndent, eFUnit );
                SetMetricValue( aLeftIndent, rOldItem.GetTxtLeft(), eUnit );
            }

            if ( rOldItem.GetPropRight() != 100 )
            {
                aRightIndent.SetRelative( sal_True );
                aRightIndent.SetValue( rOldItem.GetPropRight() );
            }
            else
            {
                aRightIndent.SetRelative();
                SetFieldUnit( aRightIndent, eFUnit );
                SetMetricValue( aRightIndent, rOldItem.GetRight(), eUnit );
            }

            if ( rOldItem.GetPropTxtFirstLineOfst() != 100 )
            {
                aFLineIndent.SetRelative( sal_True );
                aFLineIndent.SetValue( rOldItem.GetPropTxtFirstLineOfst() );
            }
            else
            {
                aFLineIndent.SetRelative();
                aFLineIndent.SetMin(-9999);
                SetFieldUnit( aFLineIndent, eFUnit );
                SetMetricValue( aFLineIndent, rOldItem.GetTxtFirstLineOfst(),
                                eUnit );
            }
            aAutoCB.Check(rOldItem.IsAutoFirst());
        }
        else
        {
            const SvxLRSpaceItem& rSpace =
                (const SvxLRSpaceItem&)rSet.Get( _nWhich );

            SetMetricValue( aLeftIndent, rSpace.GetTxtLeft(), eUnit );
            SetMetricValue( aRightIndent, rSpace.GetRight(), eUnit );
            SetMetricValue( aFLineIndent, rSpace.GetTxtFirstLineOfst(), eUnit );
            aAutoCB.Check(rSpace.IsAutoFirst());
        }
        AutoHdl_Impl(&aAutoCB);
    }
    else
    {
        aLeftIndent.SetEmptyFieldValue();
        aRightIndent.SetEmptyFieldValue();
        aFLineIndent.SetEmptyFieldValue();
    }

    _nWhich = GetWhich( SID_ATTR_ULSPACE );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        SfxMapUnit eUnit = pPool->GetMetric( _nWhich );

        if ( bRelativeMode )
        {
            const SvxULSpaceItem& rOldItem =
                (const SvxULSpaceItem&)rSet.Get( _nWhich );

            if ( rOldItem.GetPropUpper() != 100 )
            {
                aTopDist.SetRelative( sal_True );
                aTopDist.SetValue( rOldItem.GetPropUpper() );
            }
            else
            {
                aTopDist.SetRelative();
                if ( eFUnit == FUNIT_CHAR )
                    SetFieldUnit( aTopDist, FUNIT_LINE );
                else
                    SetFieldUnit( aTopDist, eFUnit );
                SetMetricValue( aTopDist, rOldItem.GetUpper(), eUnit );
            }

            if ( rOldItem.GetPropLower() != 100 )
            {
                aBottomDist.SetRelative( sal_True );
                aBottomDist.SetValue( rOldItem.GetPropLower() );
            }
            else
            {
                aBottomDist.SetRelative();
                if ( eFUnit == FUNIT_CHAR )
                    SetFieldUnit( aBottomDist, FUNIT_LINE );
                else
                    SetFieldUnit( aBottomDist, eFUnit );
                SetMetricValue( aBottomDist, rOldItem.GetLower(), eUnit );
            }
        }
        else
        {
            const SvxULSpaceItem& rTopMargin =
                (const SvxULSpaceItem&)rSet.Get( _nWhich );
            SetMetricValue( aTopDist, rTopMargin.GetUpper(), eUnit );
            SetMetricValue( aBottomDist, rTopMargin.GetLower(), eUnit );
        }
    }
    else
    {
        aTopDist.SetEmptyFieldValue();
        aBottomDist.SetEmptyFieldValue();
    }

    _nWhich = GetWhich( SID_ATTR_PARA_LINESPACE );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
        SetLineSpacing_Impl( (const SvxLineSpacingItem &)rSet.Get( _nWhich ) );
    else
        aLineDist.SetNoSelection();


    _nWhich = GetWhich( SID_ATTR_PARA_REGISTER );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
        aRegisterCB.Check( ((const SfxBoolItem &)rSet.Get( _nWhich )).GetValue());
    aRegisterCB.SaveValue();
    sal_uInt16 nHtmlMode = GetHtmlMode_Impl(rSet);
    if(nHtmlMode & HTMLMODE_ON)
    {
        aRegisterCB.Hide();
        aRegisterFL.Hide();
        aAutoCB.Hide();
        if(!(nHtmlMode & HTMLMODE_SOME_STYLES)) // IE oder SW
        {
            aRightLabel.Disable();
            aRightIndent.Disable();
            aTopDist.Disable();  //HTML3.2 und NS 3.0
            aBottomDist.Disable();
            if(!(nHtmlMode & HTMLMODE_FIRSTLINE)) //NS 3.0
            {
                aFLineIndent.Disable();
                aFLineLabel.Disable();
            }
        }
    }

    ELRLoseFocusHdl( NULL );
    aAutoCB.SaveValue();
    aLineDist.SaveValue();
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::EnableRelativeMode()
{
    DBG_ASSERT( GetItemSet().GetParent(), "RelativeMode, but no parent-set!" );

    aLeftIndent.EnableRelativeMode( 0, 999 );
    aFLineIndent.EnableRelativeMode( 0, 999 );
    aRightIndent.EnableRelativeMode( 0, 999 );
    aTopDist.EnableRelativeMode( 0, 999 );
    aBottomDist.EnableRelativeMode( 0, 999 );
    bRelativeMode = sal_True;
}

// -----------------------------------------------------------------------

int SvxStdParagraphTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    ELRLoseFocusHdl( NULL );

    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

SvxStdParagraphTabPage::SvxStdParagraphTabPage( Window* pParent,
                                                const SfxItemSet& rAttr ) :

    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_STD_PARAGRAPH ), rAttr ),

    aIndentFrm              ( this, CUI_RES( FL_INDENT ) ),
    aLeftLabel              ( this, CUI_RES( FT_LEFTINDENT ) ),
    aLeftIndent             ( this, CUI_RES( ED_LEFTINDENT ) ),
    aRightLabel             ( this, CUI_RES( FT_RIGHTINDENT ) ),
    aRightIndent            ( this, CUI_RES( ED_RIGHTINDENT ) ),

    aFLineLabel             ( this, CUI_RES( FT_FLINEINDENT ) ),
    aFLineIndent            ( this, CUI_RES( ED_FLINEINDENT ) ),
    aAutoCB                 ( this, CUI_RES( CB_AUTO ) ),
    aDistFrm                ( this, CUI_RES( FL_DIST ) ),
    aTopLabel               ( this, CUI_RES( FT_TOPDIST ) ),
    aTopDist                ( this, CUI_RES( ED_TOPDIST ) ),
    aBottomLabel            ( this, CUI_RES( FT_BOTTOMDIST ) ),
    aBottomDist             ( this, CUI_RES( ED_BOTTOMDIST ) ),

    aLineDistFrm            ( this, CUI_RES( FL_LINEDIST ) ),
    aLineDist               ( this, CUI_RES( LB_LINEDIST ) ),
    aLineDistAtLabel        ( this, CUI_RES( FT_LINEDIST ) ),
    aLineDistAtPercentBox   ( this, CUI_RES( ED_LINEDISTPERCENT ) ),
    aLineDistAtMetricBox    ( this, CUI_RES( ED_LINEDISTMETRIC ) ),
    sAbsDist                ( CUI_RES(ST_LINEDIST_ABS) ),
    aExampleWin             ( this, CUI_RES( WN_EXAMPLE ) ),
    aRegisterFL             ( this, CUI_RES( FL_REGISTER ) ),
    aRegisterCB             ( this, CUI_RES( CB_REGISTER ) ),
    pActLineDistFld ( &aLineDistAtPercentBox ),
    nAbst           ( MAX_DURCH ),
    nWidth          ( 11905 /*567 * 50*/ ),
    nMinFixDist(0L),

    bRelativeMode   ( sal_False ),
    bNegativeIndents(sal_False)

{
    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    aLineDistAtMetricBox.Hide();
    FreeResource();
    Init_Impl();
    aFLineIndent.SetMin(-9999);    // wird default auf 0 gesetzt

    aExampleWin.SetAccessibleName(String(CUI_RES(STR_EXAMPLE)));

}


// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::EnableNegativeMode()
{
    aLeftIndent.SetMin(-9999);
    aRightIndent.SetMin(-9999);
    aRightIndent.EnableNegativeMode();
    aLeftIndent.EnableNegativeMode();
    bNegativeIndents = sal_True;
}

// -----------------------------------------------------------------------

sal_uInt16* SvxStdParagraphTabPage::GetRanges()
{
    return pStdRanges;
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::SetLineSpacing_Impl
(
    const SvxLineSpacingItem &rAttr
)
{
    SfxMapUnit eUnit = GetItemSet().GetPool()->GetMetric( rAttr.Which() );

    switch( rAttr.GetLineSpaceRule() )
    {
        case SVX_LINE_SPACE_AUTO:
        {
            SvxInterLineSpace eInter = rAttr.GetInterLineSpaceRule();

            switch( eInter )
            {
                // Default einzeilig
                case SVX_INTER_LINE_SPACE_OFF:
                    aLineDist.SelectEntryPos( LLINESPACE_1 );
                    break;

                // Default einzeilig
                case SVX_INTER_LINE_SPACE_PROP:
                    if ( 100 == rAttr.GetPropLineSpace() )
                    {
                        aLineDist.SelectEntryPos( LLINESPACE_1 );
                        break;
                    }
                    // 1.5zeilig
                    if ( 150 == rAttr.GetPropLineSpace() )
                    {
                        aLineDist.SelectEntryPos( LLINESPACE_15 );
                        break;
                    }
                    // 2zeilig
                    if ( 200 == rAttr.GetPropLineSpace() )
                    {
                        aLineDist.SelectEntryPos( LLINESPACE_2 );
                        break;
                    }
                    // eingestellter Prozentwert
                    aLineDistAtPercentBox.
                        SetValue( aLineDistAtPercentBox.Normalize(
                                        rAttr.GetPropLineSpace() ) );
                    aLineDist.SelectEntryPos( LLINESPACE_PROP );
                    break;

                case SVX_INTER_LINE_SPACE_FIX:
                    SetMetricValue( aLineDistAtMetricBox,
                                    rAttr.GetInterLineSpace(), eUnit );
                    aLineDist.SelectEntryPos( LLINESPACE_DURCH );
                    break;
                default: ;//prevent warning
            }
        }
        break;
        case SVX_LINE_SPACE_FIX:
            SetMetricValue(aLineDistAtMetricBox, rAttr.GetLineHeight(), eUnit);
            aLineDist.SelectEntryPos( LLINESPACE_FIX );
        break;

        case SVX_LINE_SPACE_MIN:
            SetMetricValue(aLineDistAtMetricBox, rAttr.GetLineHeight(), eUnit);
            aLineDist.SelectEntryPos( LLINESPACE_MIN );
            break;
        default: ;//prevent warning
    }
    LineDistHdl_Impl( &aLineDist );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxStdParagraphTabPage, LineDistHdl_Impl, ListBox *, pBox )
{
    switch( pBox->GetSelectEntryPos() )
    {
        case LLINESPACE_1:
        case LLINESPACE_15:
        case LLINESPACE_2:
            aLineDistAtLabel.Enable(sal_False);
            pActLineDistFld->Enable(sal_False);
            pActLineDistFld->SetText( String() );
            break;

        case LLINESPACE_DURCH:
            // Setzen eines sinnvollen Defaults?
            // MS Begrenzen min(10, aPageSize)
            aLineDistAtPercentBox.Hide();
            pActLineDistFld = &aLineDistAtMetricBox;
            aLineDistAtMetricBox.SetMin(0);


            if ( !aLineDistAtMetricBox.GetText().Len() )
                aLineDistAtMetricBox.SetValue(
                    aLineDistAtMetricBox.Normalize( 1 ) );
            aLineDistAtPercentBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            aLineDistAtLabel.Enable();
            break;

        case LLINESPACE_MIN:
            aLineDistAtPercentBox.Hide();
            pActLineDistFld = &aLineDistAtMetricBox;
            aLineDistAtMetricBox.SetMin(0);

            if ( !aLineDistAtMetricBox.GetText().Len() )
                aLineDistAtMetricBox.SetValue(
                    aLineDistAtMetricBox.Normalize( 10 ), FUNIT_TWIP );
            aLineDistAtPercentBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            aLineDistAtLabel.Enable();
            break;

        case LLINESPACE_PROP:
            aLineDistAtMetricBox.Hide();
            pActLineDistFld = &aLineDistAtPercentBox;

            if ( !aLineDistAtPercentBox.GetText().Len() )
                aLineDistAtPercentBox.SetValue(
                    aLineDistAtPercentBox.Normalize( 100 ), FUNIT_TWIP );
            aLineDistAtMetricBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            aLineDistAtLabel.Enable();
            break;
        case LLINESPACE_FIX:
        {
            aLineDistAtPercentBox.Hide();
            pActLineDistFld = &aLineDistAtMetricBox;
            sal_Int64 nTemp = aLineDistAtMetricBox.GetValue();
            aLineDistAtMetricBox.SetMin(aLineDistAtMetricBox.Normalize(nMinFixDist), FUNIT_TWIP);

            // wurde der Wert beim SetMin veraendert, dann ist es Zeit
            // fuer den default
            if ( aLineDistAtMetricBox.GetValue() != nTemp )
                SetMetricValue( aLineDistAtMetricBox,
                                    FIX_DIST_DEF, SFX_MAPUNIT_TWIP ); // fix gibt's nur im Writer
            aLineDistAtPercentBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            aLineDistAtLabel.Enable();
        }
        break;
    }
    UpdateExample_Impl( sal_True );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxStdParagraphTabPage, ModifyHdl_Impl, SvxRelativeField *, EMPTYARG )
{
    UpdateExample_Impl();
    return 0;
}
IMPL_LINK_INLINE_END( SvxStdParagraphTabPage, ModifyHdl_Impl, SvxRelativeField *, EMPTYARG )

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::Init_Impl()
{
    aLineDist.SetSelectHdl(
        LINK( this, SvxStdParagraphTabPage, LineDistHdl_Impl ) );

    Link aLink = LINK( this, SvxStdParagraphTabPage, ELRLoseFocusHdl );
    aFLineIndent.SetLoseFocusHdl( aLink );
    aLeftIndent.SetLoseFocusHdl( aLink );
    aRightIndent.SetLoseFocusHdl( aLink );

    aLink = LINK( this, SvxStdParagraphTabPage, ModifyHdl_Impl );
    aFLineIndent.SetModifyHdl( aLink );
    aLeftIndent.SetModifyHdl( aLink );
    aRightIndent.SetModifyHdl( aLink );
    aTopDist.SetModifyHdl( aLink );
    aBottomDist.SetModifyHdl( aLink );

    aAutoCB.SetClickHdl( LINK( this, SvxStdParagraphTabPage, AutoHdl_Impl ));
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit =
        MapToFieldUnit( pPool->GetMetric( GetWhich( SID_ATTR_LRSPACE ) ) );

    aTopDist.SetMax( aTopDist.Normalize( nAbst ), eUnit );
    aBottomDist.SetMax( aBottomDist.Normalize( nAbst ), eUnit );
    aLineDistAtMetricBox.SetMax(
        aLineDistAtMetricBox.Normalize( nAbst ), eUnit );
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::UpdateExample_Impl( sal_Bool bAll )
{
    aExampleWin.SetFirstLineOfst(
        (short)aFLineIndent.Denormalize( aFLineIndent.GetValue( FUNIT_TWIP ) ) );
    aExampleWin.SetLeftMargin(
        static_cast<long>(aLeftIndent.Denormalize( aLeftIndent.GetValue( FUNIT_TWIP ) ) ) );
    aExampleWin.SetRightMargin(
        static_cast<long>(aRightIndent.Denormalize( aRightIndent.GetValue( FUNIT_TWIP ) ) ) );
    aExampleWin.SetUpper(
        (sal_uInt16)aTopDist.Denormalize( aTopDist.GetValue( FUNIT_TWIP ) ) );
    aExampleWin.SetLower(
        (sal_uInt16)aBottomDist.Denormalize( aBottomDist.GetValue( FUNIT_TWIP ) ) );



    sal_uInt16 nPos = aLineDist.GetSelectEntryPos();

    switch ( nPos )
    {
        case LLINESPACE_1:
        case LLINESPACE_15:
        case LLINESPACE_2:
            aExampleWin.SetLineSpace( (SvxPrevLineSpace)nPos );
            break;

        case LLINESPACE_PROP:
            aExampleWin.SetLineSpace( (SvxPrevLineSpace)nPos,
                (sal_uInt16)aLineDistAtPercentBox.Denormalize(
                aLineDistAtPercentBox.GetValue() ) );
            break;

        case LLINESPACE_MIN:
        case LLINESPACE_DURCH:
        case LLINESPACE_FIX:
            aExampleWin.SetLineSpace( (SvxPrevLineSpace)nPos,
                (sal_uInt16)GetCoreValue( aLineDistAtMetricBox, SFX_MAPUNIT_TWIP ) );
            break;
    }
    aExampleWin.Draw( bAll );
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::EnableRegisterMode()
{
    aRegisterCB.Show();
    aRegisterFL.Show();
}

IMPL_LINK( SvxStdParagraphTabPage, AutoHdl_Impl, CheckBox*, pBox )
{
    sal_Bool bEnable = !pBox->IsChecked();
    aFLineLabel .Enable(bEnable);
    aFLineIndent.Enable(bEnable);
    return 0;
}

void SvxStdParagraphTabPage::SetPageWidth( sal_uInt16 nPageWidth )
{
    nWidth = nPageWidth;
}


void SvxStdParagraphTabPage::EnableAutoFirstLine()
{
    aAutoCB.Show();
}


void    SvxStdParagraphTabPage::EnableAbsLineDist(long nMinTwip)
{
    aLineDist.InsertEntry(sAbsDist);
    nMinFixDist = nMinTwip;
}


void    SvxStdParagraphTabPage::PageCreated(SfxAllItemSet aSet)
{

/* different bit represent call to different method of SvxStdParagraphTabPage
                        0x0001 --->EnableRelativeMode()
                        0x0002 --->EnableRegisterMode()
                        0x0004 --->EnableAutoFirstLine()
                        0x0008 --->EnableNegativeMode()
            */
    SFX_ITEMSET_ARG (&aSet,pPageWidthItem,SfxUInt16Item,SID_SVXSTDPARAGRAPHTABPAGE_PAGEWIDTH,sal_False);
    SFX_ITEMSET_ARG (&aSet,pFlagSetItem,SfxUInt32Item,SID_SVXSTDPARAGRAPHTABPAGE_FLAGSET,sal_False);
    SFX_ITEMSET_ARG (&aSet,pLineDistItem,SfxUInt32Item,SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST,sal_False);

    if (pPageWidthItem)
        SetPageWidth(pPageWidthItem->GetValue());

    if (pFlagSetItem )
        if (( 0x0001 & pFlagSetItem->GetValue())== 0x0001 )
            EnableRelativeMode();

    if (pFlagSetItem)
        if (( 0x0002 & pFlagSetItem->GetValue())== 0x0002 )
                EnableRegisterMode();

    if (pFlagSetItem)
        if ( ( 0x0004 & pFlagSetItem->GetValue())== 0x0004 )
            EnableAutoFirstLine();

    if(pLineDistItem)
        EnableAbsLineDist(pLineDistItem->GetValue());

    if (pFlagSetItem)
        if  (( 0x0008 & pFlagSetItem->GetValue()) == 0x0008 )
                EnableNegativeMode();

}


#define LASTLINEPOS_DEFAULT     0
#define LASTLINEPOS_LEFT        1

#define LASTLINECOUNT_OLD       3
#define LASTLINECOUNT_NEW       4

// class SvxParaAlignTabPage ------------------------------------------------

SvxParaAlignTabPage::SvxParaAlignTabPage( Window* pParent, const SfxItemSet& rSet )
    : SfxTabPage(pParent, CUI_RES( RID_SVXPAGE_ALIGN_PARAGRAPH ),rSet),
    aAlignFrm           ( this, CUI_RES( FL_ALIGN ) ),
    aLeft               ( this, CUI_RES( BTN_LEFTALIGN ) ),
    aRight              ( this, CUI_RES( BTN_RIGHTALIGN ) ),
    aCenter             ( this, CUI_RES( BTN_CENTERALIGN ) ),
    aJustify            ( this, CUI_RES( BTN_JUSTIFYALIGN ) ),
    aLastLineFT         ( this, CUI_RES( FT_LASTLINE ) ),
    aLastLineLB         ( this, CUI_RES( LB_LASTLINE ) ),
    aExpandCB           ( this, CUI_RES( CB_EXPAND ) ),
    aSnapToGridCB       ( this, CUI_RES( CB_SNAP ) ),
    aExampleWin         ( this, CUI_RES( WN_EXAMPLE ) ),

    aVertAlignFL        ( this, CUI_RES( FL_VERTALIGN ) ),
    aVertAlignFT        ( this, CUI_RES( FT_VERTALIGN ) ),
    aVertAlignLB        ( this, CUI_RES( LB_VERTALIGN ) ),

    aPropertiesFL       ( this, CUI_RES( FL_PROPERTIES    )),
    aTextDirectionFT    ( this, CUI_RES( FT_TEXTDIRECTION )),
    aTextDirectionLB    ( this, CUI_RES( LB_TEXTDIRECTION ))
{
    SvtLanguageOptions aLangOptions;
    sal_uInt16 nLastLinePos = LASTLINEPOS_DEFAULT;

    if ( aLangOptions.IsAsianTypographyEnabled() )
    {
        String sLeft(CUI_RES(ST_LEFTALIGN_ASIAN));
        aLeft.SetText(sLeft);
        aRight.SetText(String(CUI_RES(ST_RIGHTALIGN_ASIAN)));
        sLeft = MnemonicGenerator::EraseAllMnemonicChars( sLeft );

        if ( aLastLineLB.GetEntryCount() == LASTLINECOUNT_OLD )
        {
            aLastLineLB.RemoveEntry( 0 );
            aLastLineLB.InsertEntry( sLeft, 0 );
        }
        else
            nLastLinePos = LASTLINEPOS_LEFT;
    }
    // remove "Default" or "Left" entry, depends on CJKOptions
    if ( aLastLineLB.GetEntryCount() == LASTLINECOUNT_NEW )
        aLastLineLB.RemoveEntry( nLastLinePos );

    FreeResource();
    Link aLink = LINK( this, SvxParaAlignTabPage, AlignHdl_Impl );
    aLeft.SetClickHdl( aLink );
    aRight.SetClickHdl( aLink );
    aCenter.SetClickHdl( aLink );
    aJustify.SetClickHdl( aLink );
    aLastLineLB.SetSelectHdl( LINK( this, SvxParaAlignTabPage, LastLineHdl_Impl ) );
    aTextDirectionLB.SetSelectHdl( LINK( this, SvxParaAlignTabPage, TextDirectionHdl_Impl ) );

    sal_uInt16 nHtmlMode = GetHtmlMode_Impl(rSet);
    if(!(nHtmlMode & HTMLMODE_ON) || (0 != (nHtmlMode & HTMLMODE_SOME_STYLES)) )
    {
        if( aLangOptions.IsCTLFontEnabled() )
        {
            aTextDirectionLB.InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_LTR ), FRMDIR_HORI_LEFT_TOP );
            aTextDirectionLB.InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_RTL ), FRMDIR_HORI_RIGHT_TOP );
            aTextDirectionLB.InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_SUPER ), FRMDIR_ENVIRONMENT );

            aPropertiesFL.Show();
            aTextDirectionFT.Show();
            aTextDirectionLB.Show();
        }
    }

    aExampleWin.SetAccessibleName(String(CUI_RES(STR_EXAMPLE)));

}

SvxParaAlignTabPage::~SvxParaAlignTabPage()
{
}

int SvxParaAlignTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

SfxTabPage* SvxParaAlignTabPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxParaAlignTabPage(pParent, rSet);
}

sal_uInt16* SvxParaAlignTabPage::GetRanges()
{
    return pAlignRanges;

}

sal_Bool SvxParaAlignTabPage::FillItemSet( SfxItemSet& rOutSet )
{
    sal_Bool bModified = sal_False;

    bool bAdj = false, bChecked = false;
    SvxAdjust eAdjust = SVX_ADJUST_LEFT;

    if ( aLeft.IsChecked() )
    {
        eAdjust = SVX_ADJUST_LEFT;
        bAdj = !aLeft.GetSavedValue();
        bChecked = true;
    }
    else if ( aRight.IsChecked() )
    {
        eAdjust = SVX_ADJUST_RIGHT;
        bAdj = !aRight.GetSavedValue();
        bChecked = true;
    }
    else if ( aCenter.IsChecked() )
    {
        eAdjust = SVX_ADJUST_CENTER;
        bAdj = !aCenter.GetSavedValue();
        bChecked = true;
    }
    else if ( aJustify.IsChecked() )
    {
        eAdjust = SVX_ADJUST_BLOCK;
        bAdj = !aJustify.GetSavedValue() ||
            aExpandCB.IsChecked() != aExpandCB.GetSavedValue() ||
            aLastLineLB.GetSelectEntryPos() != aLastLineLB.GetSavedValue();
        bChecked = true;
    }
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_ADJUST );

    if ( bAdj )
    {
        const SvxAdjustItem* pOld =
            (const SvxAdjustItem*)GetOldItem( rOutSet, SID_ATTR_PARA_ADJUST );
        SvxAdjust eOneWord = aExpandCB.IsChecked() ? SVX_ADJUST_BLOCK
                                                   : SVX_ADJUST_LEFT;
        sal_uInt16 nLBPos = aLastLineLB.GetSelectEntryPos();
        SvxAdjust eLastBlock = SVX_ADJUST_LEFT;

        if ( 1 == nLBPos )
            eLastBlock = SVX_ADJUST_CENTER;
        else if ( 2 == nLBPos )
            eLastBlock = SVX_ADJUST_BLOCK;

        bool bNothingWasChecked =
            !aLeft.GetSavedValue() && !aRight.GetSavedValue() &&
            !aCenter.GetSavedValue() && !aJustify.GetSavedValue();

        if ( !pOld || pOld->GetAdjust() != eAdjust ||
             pOld->GetOneWord() != eOneWord ||
             pOld->GetLastBlock() != eLastBlock ||
             ( bChecked && bNothingWasChecked ) )
        {
            bModified = sal_True;
            SvxAdjustItem aAdj(
                (const SvxAdjustItem&)GetItemSet().Get( _nWhich ) );
            aAdj.SetAdjust( eAdjust );
            aAdj.SetOneWord( eOneWord );
            aAdj.SetLastBlock( eLastBlock );
            rOutSet.Put( aAdj );
        }
    }
    if(aSnapToGridCB.IsChecked() != aSnapToGridCB.GetSavedValue())
    {
        rOutSet.Put(SvxParaGridItem(aSnapToGridCB.IsChecked(), GetWhich( SID_ATTR_PARA_SNAPTOGRID )));
        bModified = sal_True;
    }
    if(aVertAlignLB.GetSavedValue() != aVertAlignLB.GetSelectEntryPos())
    {
        rOutSet.Put(SvxParaVertAlignItem(aVertAlignLB.GetSelectEntryPos(), GetWhich( SID_PARA_VERTALIGN )));
        bModified = sal_True;
    }

    if( aTextDirectionLB.IsVisible() )
    {
        SvxFrameDirection eDir = aTextDirectionLB.GetSelectEntryValue();
        if( eDir != aTextDirectionLB.GetSavedValue() )
        {
            rOutSet.Put( SvxFrameDirectionItem( eDir, GetWhich( SID_ATTR_FRAMEDIRECTION ) ) );
            bModified = sal_True;
        }
    }

    return bModified;
}

void SvxParaAlignTabPage::Reset( const SfxItemSet& rSet )
{
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_ADJUST );
    SfxItemState eItemState = rSet.GetItemState( _nWhich );

    sal_uInt16 nLBSelect = 0;
    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        const SvxAdjustItem& rAdj = (const SvxAdjustItem&)rSet.Get( _nWhich );

        switch ( rAdj.GetAdjust() /*!!! VB fragen rAdj.GetLastBlock()*/ )
        {
            case SVX_ADJUST_LEFT: aLeft.Check(); break;

            case SVX_ADJUST_RIGHT: aRight.Check(); break;

            case SVX_ADJUST_CENTER: aCenter.Check(); break;

            case SVX_ADJUST_BLOCK: aJustify.Check(); break;
            default: ; //prevent warning
        }
        sal_Bool bEnable = aJustify.IsChecked();
        aLastLineFT.Enable(bEnable);
        aLastLineLB.Enable(bEnable);
        aExpandCB  .Enable(bEnable);

        aExpandCB.Check(SVX_ADJUST_BLOCK == rAdj.GetOneWord());
        switch(rAdj.GetLastBlock())
        {
            case SVX_ADJUST_LEFT:  nLBSelect = 0; break;

            case SVX_ADJUST_CENTER: nLBSelect = 1;  break;

            case SVX_ADJUST_BLOCK: nLBSelect = 2;  break;
            default: ; //prevent warning
        }
    }
    else
    {
        aLeft.Check( sal_False );
        aRight.Check( sal_False );
        aCenter.Check( sal_False );
        aJustify.Check( sal_False );
    }
    aLastLineLB.SelectEntryPos(nLBSelect);

    sal_uInt16 nHtmlMode = GetHtmlMode_Impl(rSet);
    if(nHtmlMode & HTMLMODE_ON)
    {
        aLastLineLB.Hide();
        aLastLineFT.Hide();
        aExpandCB.Hide();
        if(!(nHtmlMode & (HTMLMODE_FULL_STYLES|HTMLMODE_FIRSTLINE)) )
            aJustify.Disable();
        aSnapToGridCB.Show(sal_False);
    }
    _nWhich = GetWhich(SID_ATTR_PARA_SNAPTOGRID);
    eItemState = rSet.GetItemState( _nWhich );
    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        const SvxParaGridItem& rSnap = (const SvxParaGridItem&)rSet.Get( _nWhich );
        aSnapToGridCB.Check(rSnap.GetValue());
    }

    _nWhich = GetWhich( SID_PARA_VERTALIGN );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        aVertAlignLB.Show();
        aVertAlignFL.Show();
        aVertAlignFT.Show();

        const SvxParaVertAlignItem& rAlign = (const SvxParaVertAlignItem&)rSet.Get( _nWhich );
        aVertAlignLB.SelectEntryPos(rAlign.GetValue());
    }

    _nWhich = GetWhich( SID_ATTR_FRAMEDIRECTION );
    //text direction
    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( _nWhich ) )
    {
        const SvxFrameDirectionItem& rFrameDirItem = ( const SvxFrameDirectionItem& ) rSet.Get( _nWhich );
        aTextDirectionLB.SelectEntryValue( (SvxFrameDirection)rFrameDirItem.GetValue() );
        aTextDirectionLB.SaveValue();
    }

    aSnapToGridCB.SaveValue();
    aVertAlignLB.SaveValue();
    aLeft.SaveValue();
    aRight.SaveValue();
    aCenter.SaveValue();
    aJustify.SaveValue();
    aLastLineLB.SaveValue();
    aExpandCB.SaveValue();

    UpdateExample_Impl(sal_True);
}

IMPL_LINK( SvxParaAlignTabPage, AlignHdl_Impl, RadioButton*, EMPTYARG )
{
    sal_Bool bJustify = aJustify.IsChecked();
    aLastLineFT.Enable(bJustify);
    aLastLineLB.Enable(bJustify);
    aExpandCB.Enable(bJustify);
    UpdateExample_Impl(sal_False);
    return 0;
}

IMPL_LINK( SvxParaAlignTabPage, LastLineHdl_Impl, ListBox*, EMPTYARG )
{
    UpdateExample_Impl(sal_False);
    return 0;
}

IMPL_LINK( SvxParaAlignTabPage, TextDirectionHdl_Impl, ListBox*, EMPTYARG )
{
    SvxFrameDirection eDir = aTextDirectionLB.GetSelectEntryValue();
    switch ( eDir )
    {
        // check the default alignment for this text direction
        case FRMDIR_HORI_LEFT_TOP :     aLeft.Check( sal_True ); break;
        case FRMDIR_HORI_RIGHT_TOP :    aRight.Check( sal_True ); break;
        case FRMDIR_ENVIRONMENT :       /* do nothing */ break;
        default:
        {
            DBG_ERRORFILE( "SvxParaAlignTabPage::TextDirectionHdl_Impl(): other directions not supported" );
        }
    }

    return 0;
}

void    SvxParaAlignTabPage::UpdateExample_Impl( sal_Bool bAll )
{
    if ( aLeft.IsChecked() )
        aExampleWin.SetAdjust( SVX_ADJUST_LEFT );
    else if ( aRight.IsChecked() )
        aExampleWin.SetAdjust( SVX_ADJUST_RIGHT );
    else if ( aCenter.IsChecked() )
        aExampleWin.SetAdjust( SVX_ADJUST_CENTER );
    else if ( aJustify.IsChecked() )
    {
        aExampleWin.SetAdjust( SVX_ADJUST_BLOCK );
        SvxAdjust eLastBlock = SVX_ADJUST_LEFT;
        sal_uInt16 nLBPos = aLastLineLB.GetSelectEntryPos();
        if(nLBPos == 1)
            eLastBlock = SVX_ADJUST_CENTER;
        else if(nLBPos == 2)
            eLastBlock = SVX_ADJUST_BLOCK;
        aExampleWin.SetLastLine( eLastBlock );
    }

    aExampleWin.Draw( bAll );
}

// Erweiterungen fuer den Blocksatz einschalten
void SvxParaAlignTabPage::EnableJustifyExt()
{
    aLastLineFT.Show();
    aLastLineLB.Show();
    aExpandCB  .Show();
    SvtLanguageOptions aCJKOptions;
    if(aCJKOptions.IsAsianTypographyEnabled())
        aSnapToGridCB.Show();

}

void SvxParaAlignTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pBoolItem,SfxBoolItem,SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT,sal_False);
    if (pBoolItem)
        if(pBoolItem->GetValue())
            EnableJustifyExt();
}

SfxTabPage* SvxExtParagraphTabPage::Create( Window* pParent,
                                            const SfxItemSet& rSet )
{
    return new SvxExtParagraphTabPage( pParent, rSet );
}

// -----------------------------------------------------------------------

sal_Bool SvxExtParagraphTabPage::FillItemSet( SfxItemSet& rOutSet )
{
    sal_Bool bModified = sal_False;
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_HYPHENZONE );
    const TriState eHyphenState = aHyphenBox.GetState();
    const SfxPoolItem* pOld = GetOldItem( rOutSet, SID_ATTR_PARA_HYPHENZONE );

    if ( eHyphenState != aHyphenBox.GetSavedValue()     ||
         aExtHyphenBeforeBox.IsValueModified()          ||
         aExtHyphenAfterBox.IsValueModified()           ||
         aMaxHyphenEdit.IsValueModified() )
    {
        SvxHyphenZoneItem aHyphen(
            (const SvxHyphenZoneItem&)GetItemSet().Get( _nWhich ) );
        aHyphen.SetHyphen( eHyphenState == STATE_CHECK );

        if ( eHyphenState == STATE_CHECK )
        {
            aHyphen.GetMinLead() = (sal_uInt8)aExtHyphenBeforeBox.GetValue();
            aHyphen.GetMinTrail() = (sal_uInt8)aExtHyphenAfterBox.GetValue();
        }
        aHyphen.GetMaxHyphens() = (sal_uInt8)aMaxHyphenEdit.GetValue();

        if ( !pOld ||
            !( *(SvxHyphenZoneItem*)pOld == aHyphen ) ||
                eHyphenState != aHyphenBox.GetSavedValue())
        {
            rOutSet.Put( aHyphen );
            bModified = sal_True;
        }
    }

    if (aPagenumEdit.IsEnabled() && aPagenumEdit.IsValueModified())
    {
        SfxUInt16Item aPageNum( SID_ATTR_PARA_PAGENUM,
                                (sal_uInt16)aPagenumEdit.GetValue() );

        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_PAGENUM );

        if ( !pOld || ( (const SfxUInt16Item*)pOld )->GetValue() != aPageNum.GetValue() )
        {
            rOutSet.Put( aPageNum );
            bModified = sal_True;
        }
    }

    // Seitenumbruch

    TriState eState = aApplyCollBtn.GetState();
    bool bIsPageModel = false;

    _nWhich = GetWhich( SID_ATTR_PARA_MODEL );
    String sPage;
    if ( eState != aApplyCollBtn.GetSavedValue() ||
         ( STATE_CHECK == eState &&
           aApplyCollBox.GetSelectEntryPos() != aApplyCollBox.GetSavedValue() ) )
    {
        if ( eState == STATE_CHECK )
        {
            sPage = aApplyCollBox.GetSelectEntry();
            bIsPageModel = 0 != sPage.Len();
        }
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_MODEL );

        if ( !pOld || ( (const SvxPageModelItem*)pOld )->GetValue() != sPage )
        {
            rOutSet.Put( SvxPageModelItem( sPage, sal_False, _nWhich ) );
            bModified = sal_True;
        }
        else
            bIsPageModel = false;
    }
    else if(STATE_CHECK == eState && aApplyCollBtn.IsEnabled())
        bIsPageModel = true;
    else
        rOutSet.Put( SvxPageModelItem( sPage, sal_False, _nWhich ) );

    _nWhich = GetWhich( SID_ATTR_PARA_PAGEBREAK );

    if ( bIsPageModel )
        // wird PageModel eingeschaltet, dann immer PageBreak ausschalten
        rOutSet.Put( SvxFmtBreakItem( SVX_BREAK_NONE, _nWhich ) );
    else
    {
        eState = aPageBreakBox.GetState();
        SfxItemState eModelState = GetItemSet().GetItemState(SID_ATTR_PARA_MODEL, sal_False);

        if ( (eModelState == SFX_ITEM_SET && STATE_CHECK == aPageBreakBox.GetState()) ||
             eState != aPageBreakBox.GetSavedValue()                ||
             aBreakTypeLB.GetSelectEntryPos() != aBreakTypeLB.GetSavedValue()   ||
             aBreakPositionLB.GetSelectEntryPos() != aBreakPositionLB.GetSavedValue() )
        {
            const SvxFmtBreakItem rOldBreak(
                    (const SvxFmtBreakItem&)GetItemSet().Get( _nWhich ));
            SvxFmtBreakItem aBreak(rOldBreak.GetBreak(), rOldBreak.Which());

            switch ( eState )
            {
                case STATE_CHECK:
                {
                    sal_Bool bBefore = aBreakPositionLB.GetSelectEntryPos() == 0;

                    if ( aBreakTypeLB.GetSelectEntryPos() == 0 )
                    {
                        if ( bBefore )
                            aBreak.SetValue( SVX_BREAK_PAGE_BEFORE );
                        else
                            aBreak.SetValue( SVX_BREAK_PAGE_AFTER );
                    }
                    else
                    {
                        if ( bBefore )
                            aBreak.SetValue( SVX_BREAK_COLUMN_BEFORE );
                        else
                            aBreak.SetValue( SVX_BREAK_COLUMN_AFTER );
                    }
                    break;
                }

                case STATE_NOCHECK:
                    aBreak.SetValue( SVX_BREAK_NONE );
                    break;
                default: ; //prevent warning
            }
            pOld = GetOldItem( rOutSet, SID_ATTR_PARA_PAGEBREAK );

            if ( eState != aPageBreakBox.GetSavedValue()                ||
                    !pOld || !( *(const SvxFmtBreakItem*)pOld == aBreak ) )
            {
                bModified = sal_True;
                rOutSet.Put( aBreak );
            }
        }
    }


    // Absatztrennung
    _nWhich = GetWhich( SID_ATTR_PARA_SPLIT );
    eState = aKeepTogetherBox.GetState();

    if ( eState != aKeepTogetherBox.GetSavedValue() )
    {
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_SPLIT );

        if ( !pOld || ( (const SvxFmtSplitItem*)pOld )->GetValue() !=
                      ( eState == STATE_NOCHECK ) )
        {
            rOutSet.Put( SvxFmtSplitItem( eState == STATE_NOCHECK, _nWhich ) );
            bModified = sal_True;
        }
    }

    // Absaetze zusammenhalten
    _nWhich = GetWhich( SID_ATTR_PARA_KEEP );
    eState = aKeepParaBox.GetState();

    if ( eState != aKeepParaBox.GetSavedValue() )
    {
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_KEEP );

        // hat sich der Status geaendert, muss immer geputtet werden
        rOutSet.Put( SvxFmtKeepItem( eState == STATE_CHECK, _nWhich ) );
        bModified = sal_True;
    }

    // Witwen und Waisen
    _nWhich = GetWhich( SID_ATTR_PARA_WIDOWS );
    eState = aWidowBox.GetState();

    if ( eState != aWidowBox.GetSavedValue() ||
         aWidowRowNo.IsValueModified() )
    {
        SvxWidowsItem rItem( eState == STATE_CHECK ?
                             (sal_uInt8)aWidowRowNo.GetValue() : 0, _nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_WIDOWS );

        if ( eState != aWidowBox.GetSavedValue() || !pOld || !( *(const SvxWidowsItem*)pOld == rItem ) )
        {
            rOutSet.Put( rItem );
            bModified = sal_True;
        }
    }

    _nWhich = GetWhich( SID_ATTR_PARA_ORPHANS );
    eState = aOrphanBox.GetState();

    if ( eState != aOrphanBox.GetSavedValue() ||
         aOrphanRowNo.IsValueModified() )
    {
        SvxOrphansItem rItem( eState == STATE_CHECK ?
                             (sal_uInt8)aOrphanRowNo.GetValue() : 0, _nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_ORPHANS );

        if ( eState != aOrphanBox.GetSavedValue() ||
                !pOld ||
                    !( *(const SvxOrphansItem*)pOld == rItem ) )
        {
            rOutSet.Put( rItem );
            bModified = sal_True;
        }
    }

    return bModified;
}

// -----------------------------------------------------------------------

void SvxExtParagraphTabPage::Reset( const SfxItemSet& rSet )
{
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_HYPHENZONE );
    SfxItemState eItemState = rSet.GetItemState( _nWhich );

    sal_Bool bItemAvailable = eItemState >= SFX_ITEM_AVAILABLE;
    sal_Bool bIsHyphen = sal_False;
    if( !bHtmlMode && bItemAvailable )
    {
        const SvxHyphenZoneItem& rHyphen =
            (const SvxHyphenZoneItem&)rSet.Get( _nWhich );
        aHyphenBox.EnableTriState( sal_False );

        bIsHyphen = rHyphen.IsHyphen();
        aHyphenBox.SetState( bIsHyphen ? STATE_CHECK : STATE_NOCHECK );

        aExtHyphenBeforeBox.SetValue( rHyphen.GetMinLead() );
        aExtHyphenAfterBox.SetValue( rHyphen.GetMinTrail() );
        aMaxHyphenEdit.SetValue( rHyphen.GetMaxHyphens() );
    }
    else
    {
        aHyphenBox.SetState( STATE_DONTKNOW );
    }
    sal_Bool bEnable = bItemAvailable && bIsHyphen;
    aExtHyphenBeforeBox.Enable(bEnable);
    aExtHyphenAfterBox.Enable(bEnable);
    aBeforeText.Enable(bEnable);
    aAfterText.Enable(bEnable);
    aMaxHyphenLabel.Enable(bEnable);
    aMaxHyphenEdit.Enable(bEnable);

    _nWhich = GetWhich( SID_ATTR_PARA_PAGENUM );

    if ( rSet.GetItemState(_nWhich) >= SFX_ITEM_AVAILABLE )
    {
        const sal_uInt16 nPageNum =
            ( (const SfxUInt16Item&)rSet.Get( _nWhich ) ).GetValue();
        aPagenumEdit.SetValue( nPageNum );
    }

    if ( bPageBreak )
    {
        // zuerst PageModel behandeln
        _nWhich = GetWhich( SID_ATTR_PARA_MODEL );
        sal_Bool bIsPageModel = sal_False;
        eItemState = rSet.GetItemState( _nWhich );

        if ( eItemState >= SFX_ITEM_SET )
        {
            aApplyCollBtn.EnableTriState( sal_False );

            const SvxPageModelItem& rModel =
                (const SvxPageModelItem&)rSet.Get( _nWhich );
            String aStr( rModel.GetValue() );

            if ( aStr.Len() &&
                 aApplyCollBox.GetEntryPos( aStr ) != LISTBOX_ENTRY_NOTFOUND )
            {
                aApplyCollBox.SelectEntry( aStr );
                aApplyCollBtn.SetState( STATE_CHECK );
                bIsPageModel = sal_True;

                aPageBreakBox.Enable();
                aPageBreakBox.EnableTriState( sal_False );
                aBreakTypeFT.Enable();
                aBreakTypeLB.Enable();
                aBreakPositionFT.Enable();
                aBreakPositionLB.Enable();
                aApplyCollBtn.Enable();
                aPageBreakBox.SetState( STATE_CHECK );

                //select page break
                aBreakTypeLB.SelectEntryPos(0);
                //select break before
                aBreakPositionLB.SelectEntryPos(0);
            }
            else
            {
                aApplyCollBox.SetNoSelection();
                aApplyCollBtn.SetState( STATE_NOCHECK );
            }
        }
        else if ( SFX_ITEM_DONTCARE == eItemState )
        {
            aApplyCollBtn.EnableTriState( sal_True );
            aApplyCollBtn.SetState( STATE_DONTKNOW );
            aApplyCollBox.SetNoSelection();
        }
        else
        {
            aApplyCollBtn.Enable(sal_False);
            aApplyCollBox.Enable(sal_False);
            aPagenumEdit.Enable(sal_False);
            aPagenumText.Enable(sal_False);
        }

        if ( !bIsPageModel )
        {
            _nWhich = GetWhich( SID_ATTR_PARA_PAGEBREAK );
            eItemState = rSet.GetItemState( _nWhich );

            if ( eItemState >= SFX_ITEM_AVAILABLE )
            {
                const SvxFmtBreakItem& rPageBreak =
                    (const SvxFmtBreakItem&)rSet.Get( _nWhich );

                SvxBreak eBreak = (SvxBreak)rPageBreak.GetValue();

                // PageBreak nicht ueber CTRL-RETURN,
                // dann kann CheckBox frei gegeben werden
                aPageBreakBox.Enable();
                aPageBreakBox.EnableTriState( sal_False );
                aBreakTypeFT.Enable();
                aBreakTypeLB.Enable();
                aBreakPositionFT.Enable();
                aBreakPositionLB.Enable();

                aPageBreakBox.SetState( STATE_CHECK );

                sal_Bool _bEnable =     eBreak != SVX_BREAK_NONE &&
                                eBreak != SVX_BREAK_COLUMN_BEFORE &&
                                eBreak != SVX_BREAK_COLUMN_AFTER;
                aApplyCollBtn.Enable(_bEnable);
                if(!_bEnable)
                {
                    aApplyCollBox.Enable(_bEnable);
                    aPagenumEdit.Enable(_bEnable);
                }

                if ( eBreak == SVX_BREAK_NONE )
                    aPageBreakBox.SetState( STATE_NOCHECK );

                sal_uInt16 nType = 0; // selection position in break type ListBox : Page
                sal_uInt16 nPosition = 0; //  selection position in break position ListBox : Before
                switch ( eBreak )
                {
                    case SVX_BREAK_PAGE_BEFORE:
                        break;
                    case SVX_BREAK_PAGE_AFTER:
                        nPosition = 1;
                        break;
                    case SVX_BREAK_COLUMN_BEFORE:
                        nType = 1;
                        break;
                    case SVX_BREAK_COLUMN_AFTER:
                        nType = 1;
                        nPosition = 1;
                        break;
                    default: ;//prevent warning
                }
                aBreakTypeLB.SelectEntryPos(nType);
                aBreakPositionLB.SelectEntryPos(nPosition);
            }
            else if ( SFX_ITEM_DONTCARE == eItemState )
                aPageBreakBox.SetState( STATE_DONTKNOW );
            else
            {
                aPageBreakBox.Enable(sal_False);
                aBreakTypeFT.Enable(sal_False);
                aBreakTypeLB.Enable(sal_False);
                aBreakPositionFT.Enable(sal_False);
                aBreakPositionLB.Enable(sal_False);
            }
        }

        PageBreakPosHdl_Impl( &aBreakPositionLB );
        PageBreakHdl_Impl( &aPageBreakBox );
    }

    _nWhich = GetWhich( SID_ATTR_PARA_KEEP );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        aKeepParaBox.EnableTriState( sal_False );
        const SvxFmtKeepItem& rKeep =
            (const SvxFmtKeepItem&)rSet.Get( _nWhich );

        if ( rKeep.GetValue() )
            aKeepParaBox.SetState( STATE_CHECK );
        else
            aKeepParaBox.SetState( STATE_NOCHECK );
    }
    else if ( SFX_ITEM_DONTCARE == eItemState )
        aKeepParaBox.SetState( STATE_DONTKNOW );
    else
        aKeepParaBox.Enable(sal_False);

    _nWhich = GetWhich( SID_ATTR_PARA_SPLIT );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        const SvxFmtSplitItem& rSplit =
            (const SvxFmtSplitItem&)rSet.Get( _nWhich );
        aKeepTogetherBox.EnableTriState( sal_False );

        if ( !rSplit.GetValue() )
            aKeepTogetherBox.SetState( STATE_CHECK );
        else
        {
            aKeepTogetherBox.SetState( STATE_NOCHECK );

            // Witwen und Waisen
            aWidowBox.Enable();
            _nWhich = GetWhich( SID_ATTR_PARA_WIDOWS );
            SfxItemState eTmpState = rSet.GetItemState( _nWhich );

            if ( eTmpState >= SFX_ITEM_AVAILABLE )
            {
                const SvxWidowsItem& rWidow =
                    (const SvxWidowsItem&)rSet.Get( _nWhich );
                aWidowBox.EnableTriState( sal_False );
                const sal_uInt16 nLines = rWidow.GetValue();

                sal_Bool _bEnable = nLines > 0;
                aWidowRowNo.SetValue( aWidowRowNo.Normalize( nLines ) );
                aWidowBox.SetState( _bEnable ? STATE_CHECK : STATE_NOCHECK);
                aWidowRowNo.Enable(_bEnable);
                aWidowRowLabel.Enable(_bEnable);

            }
            else if ( SFX_ITEM_DONTCARE == eTmpState )
                aWidowBox.SetState( STATE_DONTKNOW );
            else
                aWidowBox.Enable(sal_False);

            aOrphanBox.Enable();
            _nWhich = GetWhich( SID_ATTR_PARA_ORPHANS );
            eTmpState = rSet.GetItemState( _nWhich );

            if ( eTmpState >= SFX_ITEM_AVAILABLE )
            {
                const SvxOrphansItem& rOrphan =
                    (const SvxOrphansItem&)rSet.Get( _nWhich );
                const sal_uInt16 nLines = rOrphan.GetValue();
                aOrphanBox.EnableTriState( sal_False );

                sal_Bool _bEnable = nLines > 0;
                aOrphanBox.SetState( _bEnable ? STATE_CHECK : STATE_NOCHECK);
                aOrphanRowNo.SetValue( aOrphanRowNo.Normalize( nLines ) );
                aOrphanRowNo.Enable(_bEnable);
                aOrphanRowLabel.Enable(_bEnable);

            }
            else if ( SFX_ITEM_DONTCARE == eTmpState )
                aOrphanBox.SetState( STATE_DONTKNOW );
            else
                aOrphanBox.Enable(sal_False);
        }
    }
    else if ( SFX_ITEM_DONTCARE == eItemState )
        aKeepTogetherBox.SetState( STATE_DONTKNOW );
    else
        aKeepTogetherBox.Enable(sal_False);

    // damit alles richt enabled wird
    KeepTogetherHdl_Impl( 0 );
    WidowHdl_Impl( 0 );
    OrphanHdl_Impl( 0 );

    aHyphenBox.SaveValue();
    aExtHyphenBeforeBox.SaveValue();
    aExtHyphenAfterBox.SaveValue();
    aMaxHyphenEdit.SaveValue();
    aPageBreakBox.SaveValue();
    aBreakPositionLB.SaveValue();
    aBreakTypeLB.SaveValue();
    aApplyCollBtn.SaveValue();
    aApplyCollBox.SaveValue();
    aPagenumEdit.SaveValue();
    aKeepTogetherBox.SaveValue();
    aKeepParaBox.SaveValue();
    aWidowBox.SaveValue();
    aOrphanBox.SaveValue();
}

// -----------------------------------------------------------------------

int SvxExtParagraphTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

void SvxExtParagraphTabPage::DisablePageBreak()
{
    bPageBreak = sal_False;
    aPageBreakBox.Enable(sal_False);
    aBreakTypeLB.RemoveEntry(0);
    aBreakPositionFT.Enable(sal_False);
    aBreakPositionLB.Enable(sal_False);
    aApplyCollBtn.Enable(sal_False);
    aApplyCollBox.Enable(sal_False);
    aPagenumEdit.Enable(sal_False);
}

// -----------------------------------------------------------------------

SvxExtParagraphTabPage::SvxExtParagraphTabPage( Window* pParent, const SfxItemSet& rAttr ) :

    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_EXT_PARAGRAPH ), rAttr ),

    aExtFL              ( this, CUI_RES( FL_HYPHEN ) ),
    aHyphenBox          ( this, CUI_RES( BTN_HYPHEN ) ),
    aBeforeText         ( this, CUI_RES( FT_HYPHENBEFORE ) ),
    aExtHyphenBeforeBox ( this, CUI_RES( ED_HYPHENBEFORE ) ),
    aAfterText          ( this, CUI_RES( FT_HYPHENAFTER ) ),
    aExtHyphenAfterBox  ( this, CUI_RES( ED_HYPHENAFTER ) ),
    aMaxHyphenLabel     ( this, CUI_RES( FT_MAXHYPH ) ),
    aMaxHyphenEdit      ( this, CUI_RES( ED_MAXHYPH ) ),
    aBreaksFL           ( this, CUI_RES( FL_BREAKS ) ),
    aPageBreakBox       ( this, CUI_RES( BTN_PAGEBREAK ) ),
    aBreakTypeFT        ( this, CUI_RES( FT_BREAKTYPE     )),
    aBreakTypeLB        ( this, CUI_RES( LB_BREAKTYPE     )),
    aBreakPositionFT    ( this, CUI_RES( FT_BREAKPOSITION )),
    aBreakPositionLB    ( this, CUI_RES( LB_BREAKPOSITION )),
    aApplyCollBtn       ( this, CUI_RES( BTN_PAGECOLL ) ),
    aApplyCollBox       ( this, CUI_RES( LB_PAGECOLL ) ),
    aPagenumText        ( this, CUI_RES( FT_PAGENUM ) ),
    aPagenumEdit        ( this, CUI_RES( ED_PAGENUM ) ),
    aExtendFL           ( this, CUI_RES( FL_OPTIONS ) ),
    aKeepTogetherBox    ( this, CUI_RES( BTN_KEEPTOGETHER ) ),
    aKeepParaBox        ( this, CUI_RES( CB_KEEPTOGETHER ) ),
    aOrphanBox          ( this, CUI_RES( BTN_ORPHANS ) ),
    aOrphanRowNo        ( this, CUI_RES( ED_ORPHANS ) ),
    aOrphanRowLabel     ( this, CUI_RES( FT_ORPHANS ) ),
    aWidowBox           ( this, CUI_RES( BTN_WIDOWS ) ),
    aWidowRowNo         ( this, CUI_RES( ED_WIDOWS ) ),
    aWidowRowLabel      ( this, CUI_RES( FT_WIDOWS ) ),
    bPageBreak  ( sal_True ),
    bHtmlMode   ( sal_False ),
    nStdPos     ( 0 )
{
    FreeResource();

    aApplyCollBox.SetAccessibleRelationLabeledBy(&aApplyCollBtn);
    aApplyCollBox.SetAccessibleName(String(CUI_RES(STR_PAGE_STYLE)));
    aOrphanRowNo.SetAccessibleRelationLabeledBy(&aOrphanBox);
    aWidowRowNo.SetAccessibleRelationLabeledBy(&aWidowBox);

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    aHyphenBox.SetClickHdl(         LINK( this, SvxExtParagraphTabPage, HyphenClickHdl_Impl ) );
    aPageBreakBox.SetClickHdl(      LINK( this, SvxExtParagraphTabPage, PageBreakHdl_Impl ) );
    aKeepTogetherBox.SetClickHdl(   LINK( this, SvxExtParagraphTabPage, KeepTogetherHdl_Impl ) );
    aWidowBox.SetClickHdl(          LINK( this, SvxExtParagraphTabPage, WidowHdl_Impl ) );
    aOrphanBox.SetClickHdl(         LINK( this, SvxExtParagraphTabPage, OrphanHdl_Impl ) );
    aApplyCollBtn.SetClickHdl(      LINK( this, SvxExtParagraphTabPage, ApplyCollClickHdl_Impl ) );
    aBreakTypeLB.SetSelectHdl(      LINK( this, SvxExtParagraphTabPage, PageBreakTypeHdl_Impl ) );
    aBreakPositionLB.SetSelectHdl(  LINK( this, SvxExtParagraphTabPage, PageBreakPosHdl_Impl ) );

    SfxObjectShell* pSh = SfxObjectShell::Current();
    if ( pSh )
    {
        SfxStyleSheetBasePool* pPool = pSh->GetStyleSheetPool();
        pPool->SetSearchMask( SFX_STYLE_FAMILY_PAGE );
        SfxStyleSheetBase* pStyle = pPool->First();
        String aStdName;

        while( pStyle )
        {
            if ( aStdName.Len() == 0 )
                // first style == standard style
                aStdName = pStyle->GetName();
            aApplyCollBox.InsertEntry( pStyle->GetName() );
            pStyle = pPool->Next();
        }
        nStdPos = aApplyCollBox.GetEntryPos( aStdName );
    }

    sal_uInt16 nHtmlMode = GetHtmlMode_Impl( rAttr );
    if ( nHtmlMode & HTMLMODE_ON )
    {
        bHtmlMode = sal_True;
        aHyphenBox           .Enable(sal_False);
        aBeforeText          .Enable(sal_False);
        aExtHyphenBeforeBox  .Enable(sal_False);
        aAfterText           .Enable(sal_False);
        aExtHyphenAfterBox   .Enable(sal_False);
        aMaxHyphenLabel      .Enable(sal_False);
        aMaxHyphenEdit       .Enable(sal_False);
        aExtFL               .Enable(sal_False);
        aPagenumText         .Enable(sal_False);
        aPagenumEdit         .Enable(sal_False);
        // no column break in HTML
        aBreakTypeLB.RemoveEntry(1);
    }
}

// -----------------------------------------------------------------------

SvxExtParagraphTabPage::~SvxExtParagraphTabPage()
{
}

// -----------------------------------------------------------------------

sal_uInt16* SvxExtParagraphTabPage::GetRanges()
{
    return pExtRanges;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, PageBreakHdl_Impl, TriStateBox *, EMPTYARG )
{
    switch ( aPageBreakBox.GetState() )
    {
        case STATE_CHECK:
            aBreakTypeFT.Enable();
            aBreakTypeLB.Enable();
            aBreakPositionFT.Enable();
            aBreakPositionLB.Enable();

            if ( 0 == aBreakTypeLB.GetSelectEntryPos()&&
                0 == aBreakPositionLB.GetSelectEntryPos() )
            {
                aApplyCollBtn.Enable();

                sal_Bool bEnable = STATE_CHECK == aApplyCollBtn.GetState() &&
                                            aApplyCollBox.GetEntryCount();
                aApplyCollBox.Enable(bEnable);
                if(!bHtmlMode)
                {
                    aPagenumText.Enable(bEnable);
                    aPagenumEdit.Enable(bEnable);
                }
            }
            break;

        case STATE_NOCHECK:
        case STATE_DONTKNOW:
            aApplyCollBtn.SetState( STATE_NOCHECK );
            aApplyCollBtn.Enable(sal_False);
            aApplyCollBox.Enable(sal_False);
            aPagenumText.Enable(sal_False);
            aPagenumEdit.Enable(sal_False);
            aBreakTypeFT.Enable(sal_False);
            aBreakTypeLB.Enable(sal_False);
            aBreakPositionFT.Enable(sal_False);
            aBreakPositionLB.Enable(sal_False);
            break;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, KeepTogetherHdl_Impl, TriStateBox *, EMPTYARG )
{
    sal_Bool bEnable = aKeepTogetherBox.GetState() == STATE_NOCHECK;
    aWidowBox.Enable(bEnable);
    aOrphanBox.Enable(bEnable);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, WidowHdl_Impl, TriStateBox *, EMPTYARG )
{
    switch ( aWidowBox.GetState() )
    {
        case STATE_CHECK:
            aWidowRowNo.Enable();
            aWidowRowLabel.Enable();
            aKeepTogetherBox.Enable(sal_False);
            break;

        case STATE_NOCHECK:
            if ( aOrphanBox.GetState() == STATE_NOCHECK )
                aKeepTogetherBox.Enable();

        // kein break
        case STATE_DONTKNOW:
            aWidowRowNo.Enable(sal_False);
            aWidowRowLabel.Enable(sal_False);
            break;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, OrphanHdl_Impl, TriStateBox *, EMPTYARG )
{
    switch( aOrphanBox.GetState() )
    {
        case STATE_CHECK:
            aOrphanRowNo.Enable();
            aOrphanRowLabel.Enable();
            aKeepTogetherBox.Enable(sal_False);
            break;

        case STATE_NOCHECK:
            if ( aWidowBox.GetState() == STATE_NOCHECK )
                aKeepTogetherBox.Enable();

        // kein break
        case STATE_DONTKNOW:
            aOrphanRowNo.Enable(sal_False);
            aOrphanRowLabel.Enable(sal_False);
            break;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, HyphenClickHdl_Impl, TriStateBox *, EMPTYARG )
{

    sal_Bool bEnable = aHyphenBox.GetState() == STATE_CHECK;
    aBeforeText.Enable(bEnable);
    aExtHyphenBeforeBox.Enable(bEnable);
    aAfterText.Enable(bEnable);
    aExtHyphenAfterBox.Enable(bEnable);
    aMaxHyphenLabel.Enable(bEnable);
    aMaxHyphenEdit.Enable(bEnable);
    aHyphenBox.SetState( bEnable ? STATE_CHECK : STATE_NOCHECK);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, ApplyCollClickHdl_Impl, TriStateBox *, EMPTYARG )
{
    sal_Bool bEnable = sal_False;
    if ( aApplyCollBtn.GetState() == STATE_CHECK &&
         aApplyCollBox.GetEntryCount() )
    {
        bEnable = sal_True;
        aApplyCollBox.SelectEntryPos( nStdPos );
    }
    else
    {
        aApplyCollBox.SetNoSelection();
    }
    aApplyCollBox.Enable(bEnable);
    if(!bHtmlMode)
    {
        aPagenumText.Enable(bEnable);
        aPagenumEdit.Enable(bEnable);
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, PageBreakPosHdl_Impl, ListBox *, pListBox )
{
    if ( 0 == pListBox->GetSelectEntryPos() )
    {
        aApplyCollBtn.Enable();

        sal_Bool bEnable = aApplyCollBtn.GetState() == STATE_CHECK &&
                                    aApplyCollBox.GetEntryCount();

        aApplyCollBox.Enable(bEnable);
        if(!bHtmlMode)
        {
            aPagenumText.Enable(bEnable);
            aPagenumEdit.Enable(bEnable);
        }
    }
    else if ( 1 == pListBox->GetSelectEntryPos() )
    {
        aApplyCollBtn.SetState( STATE_NOCHECK );
        aApplyCollBtn.Enable(sal_False);
        aApplyCollBox.Enable(sal_False);
        aPagenumText.Enable(sal_False);
        aPagenumEdit.Enable(sal_False);
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, PageBreakTypeHdl_Impl, ListBox *, pListBox )
{
    //column break or break break after
    sal_uInt16 nBreakPos = aBreakPositionLB.GetSelectEntryPos();
    if ( pListBox->GetSelectEntryPos() == 1 || 1 == nBreakPos)
    {
        aApplyCollBtn.SetState( STATE_NOCHECK );
        aApplyCollBtn.Enable(sal_False);
        aApplyCollBox.Enable(sal_False);
        aPagenumText.Enable(sal_False);
        aPagenumEdit.Enable(sal_False);
    }
    else
        PageBreakPosHdl_Impl( &aBreakPositionLB );
    return 0;
}

void SvxExtParagraphTabPage::PageCreated(SfxAllItemSet aSet)
{


    SFX_ITEMSET_ARG (&aSet,pDisablePageBreakItem,SfxBoolItem,SID_DISABLE_SVXEXTPARAGRAPHTABPAGE_PAGEBREAK,sal_False);

    if (pDisablePageBreakItem)
        if ( pDisablePageBreakItem->GetValue())
                    DisablePageBreak();


}

SvxAsianTabPage::SvxAsianTabPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, CUI_RES( RID_SVXPAGE_PARA_ASIAN ), rSet),
    aOptionsFL(         this, CUI_RES(FL_AS_OPTIONS       )),
    aForbiddenRulesCB(  this, CUI_RES(CB_AS_FORBIDDEN     )),
    aHangingPunctCB(    this, CUI_RES(CB_AS_HANG_PUNC     )),
    aScriptSpaceCB(     this, CUI_RES(CB_AS_SCRIPT_SPACE    ))//,

{
    FreeResource();

    Link aLink = LINK( this, SvxAsianTabPage, ClickHdl_Impl );
    aHangingPunctCB.SetClickHdl( aLink );
    aScriptSpaceCB.SetClickHdl( aLink );
    aForbiddenRulesCB.SetClickHdl( aLink );

}

SvxAsianTabPage::~SvxAsianTabPage()
{
}

SfxTabPage* SvxAsianTabPage::Create(    Window* pParent, const SfxItemSet& rSet )
{
    return new SvxAsianTabPage(pParent, rSet);
}

sal_uInt16*     SvxAsianTabPage::GetRanges()
{
    static sal_uInt16 pRanges[] =
    {
        SID_ATTR_PARA_SCRIPTSPACE, SID_ATTR_PARA_FORBIDDEN_RULES,
        0
    };
    return pRanges;
}

sal_Bool        SvxAsianTabPage::FillItemSet( SfxItemSet& rSet )
{
    sal_Bool bRet = sal_False;
    SfxItemPool* pPool = rSet.GetPool();
    if(aScriptSpaceCB.IsChecked() != aScriptSpaceCB.GetSavedValue())
    {
        SfxBoolItem* pNewItem = (SfxBoolItem*)rSet.Get(
            pPool->GetWhich(SID_ATTR_PARA_SCRIPTSPACE)).Clone();
        pNewItem->SetValue(aScriptSpaceCB.IsChecked());
        rSet.Put(*pNewItem);
        delete pNewItem;
        bRet = sal_True;
    }
    if(aHangingPunctCB.IsChecked() != aHangingPunctCB.GetSavedValue())
    {
        SfxBoolItem* pNewItem = (SfxBoolItem*)rSet.Get(
            pPool->GetWhich(SID_ATTR_PARA_HANGPUNCTUATION)).Clone();
        pNewItem->SetValue(aHangingPunctCB.IsChecked());
        rSet.Put(*pNewItem);
        delete pNewItem;
        bRet = sal_True;
    }
    if(aForbiddenRulesCB.IsChecked() != aForbiddenRulesCB.GetSavedValue())
    {
        SfxBoolItem* pNewItem = (SfxBoolItem*)rSet.Get(
            pPool->GetWhich(SID_ATTR_PARA_FORBIDDEN_RULES)).Clone();
        pNewItem->SetValue(aForbiddenRulesCB.IsChecked());
        rSet.Put(*pNewItem);
        delete pNewItem;
        bRet = sal_True;
    }
    return bRet;
}

void lcl_SetBox(const SfxItemSet& rSet, sal_uInt16 nSlotId, TriStateBox& rBox)
{
    sal_uInt16 _nWhich = rSet.GetPool()->GetWhich(nSlotId);
    SfxItemState eState = rSet.GetItemState(_nWhich, sal_True);
    if(!eState || eState == SFX_ITEM_DISABLED )
        rBox.Enable(sal_False);
    else if(eState >= SFX_ITEM_AVAILABLE)
    {
        rBox.EnableTriState( sal_False );
        rBox.Check(((const SfxBoolItem&)rSet.Get(_nWhich)).GetValue());
    }
    else
        rBox.SetState( STATE_DONTKNOW );
    rBox.SaveValue();
}


void SvxAsianTabPage::Reset( const SfxItemSet& rSet )
{
    lcl_SetBox(rSet, SID_ATTR_PARA_FORBIDDEN_RULES, aForbiddenRulesCB );
    lcl_SetBox(rSet, SID_ATTR_PARA_HANGPUNCTUATION, aHangingPunctCB );

    //character distance not yet available
    lcl_SetBox(rSet, SID_ATTR_PARA_SCRIPTSPACE, aScriptSpaceCB );
}

IMPL_LINK( SvxAsianTabPage, ClickHdl_Impl, TriStateBox*, pBox )
{
    pBox->EnableTriState( sal_False );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
