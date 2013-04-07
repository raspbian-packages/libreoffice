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

// include ---------------------------------------------------------------
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <tools/resary.hxx>
#include <vcl/graph.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/itemiter.hxx>
#include <svl/languageoptions.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/configitem.hxx>
#include "svx/htmlmode.hxx"
#include <sal/macros.h>

#define _SVX_PAGE_CXX

#include <cuires.hrc>
#include "page.hrc"
#include "helpid.hrc"
#include "page.hxx"
#include <svx/pageitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include "svx/dlgutil.hxx"
#include <editeng/paperinf.hxx>
#include <dialmgr.hxx>
#include <sfx2/module.hxx>
#include <svl/stritem.hxx>
#include <svx/dialogs.hrc>  // for RID_SVXPAGE_PAGE
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc> // for RID_SVXSTR_PAPERBIN...,
#include <svx/svxids.hrc>

// #i4219#
#include <svtools/optionsdrawinglayer.hxx>
#include <svl/slstitm.hxx>
#include <svl/aeitem.hxx>
#include <sfx2/request.hxx>

// static ----------------------------------------------------------------

static const long MINBODY       = 284;  // 0,5cm in twips aufgerundet
//static const long PRINT_OFFSET    = 17;   // 0,03cm in twips abgerundet
static const long PRINT_OFFSET  = 0;    // why was this ever set to 17 ? it led to wrong right and bottom margins.

static sal_uInt16 pRanges[] =
{
    SID_ATTR_BORDER_OUTER,
    SID_ATTR_BORDER_SHADOW,
    SID_ATTR_LRSPACE,
    SID_ATTR_PAGE_SHARED,
    SID_SWREGISTER_COLLECTION,
    SID_SWREGISTER_MODE,
    0
};

// ------- Mapping Seitenlayout ------------------------------------------

sal_uInt16 aArr[] =
{
    SVX_PAGE_ALL,
    SVX_PAGE_MIRROR,
    SVX_PAGE_RIGHT,
    SVX_PAGE_LEFT
};

// -----------------------------------------------------------------------

sal_uInt16 PageUsageToPos_Impl( sal_uInt16 nUsage )
{
    for ( sal_uInt16 i = 0; i < SAL_N_ELEMENTS(aArr); ++i )
        if ( aArr[i] == ( nUsage & 0x000f ) )
            return i;
    return SVX_PAGE_ALL;
}

// -----------------------------------------------------------------------

sal_uInt16 PosToPageUsage_Impl( sal_uInt16 nPos )
{
    if ( nPos >= SAL_N_ELEMENTS(aArr) )
        return 0;
    return aArr[nPos];
}

// -----------------------------------------------------------------------

Size GetMinBorderSpace_Impl( const SvxShadowItem& rShadow, const SvxBoxItem& rBox )
{
    Size aSz;
    aSz.Height() = rShadow.CalcShadowSpace( SHADOW_BOTTOM ) + rBox.CalcLineSpace( BOX_LINE_BOTTOM );
    aSz.Height() += rShadow.CalcShadowSpace( SHADOW_TOP ) + rBox.CalcLineSpace( BOX_LINE_TOP );
    aSz.Width() = rShadow.CalcShadowSpace( SHADOW_LEFT ) + rBox.CalcLineSpace( BOX_LINE_LEFT );
    aSz.Width() += rShadow.CalcShadowSpace( SHADOW_RIGHT ) + rBox.CalcLineSpace( BOX_LINE_RIGHT );
    return aSz;
}

// -----------------------------------------------------------------------

long ConvertLong_Impl( const long nIn, SfxMapUnit eUnit )
{
    return OutputDevice::LogicToLogic( nIn, (MapUnit)eUnit, MAP_TWIP );
}

sal_Bool IsEqualSize_Impl( const SvxSizeItem* pSize, const Size& rSize )
{
    if ( pSize )
    {
        Size aSize = pSize->GetSize();
        long nDiffW = Abs( rSize.Width () - aSize.Width () );
        long nDiffH = Abs( rSize.Height() - aSize.Height() );
        return ( nDiffW < 10 && nDiffH < 10 );
    }
    else
        return sal_False;
}

// -----------------------------------------------------------------------

#define MARGIN_LEFT     ( (MarginPosition)0x0001 )
#define MARGIN_RIGHT    ( (MarginPosition)0x0002 )
#define MARGIN_TOP      ( (MarginPosition)0x0004 )
#define MARGIN_BOTTOM   ( (MarginPosition)0x0008 )

struct SvxPage_Impl
{
    MarginPosition  m_nPos;
    Printer*        mpDefPrinter;
    bool            mbDelPrinter;

    SvxPage_Impl() :
        m_nPos( 0 ),
        mpDefPrinter( 0 ),
        mbDelPrinter( false ) {}

    ~SvxPage_Impl() { if ( mbDelPrinter ) delete mpDefPrinter; }
};

// class SvxPageDescPage --------------------------------------------------

// gibt den Bereich der Which-Werte zur"uck

sal_uInt16* SvxPageDescPage::GetRanges()
{
    return pRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxPageDescPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxPageDescPage( pParent, rSet );
}

// -----------------------------------------------------------------------

SvxPageDescPage::SvxPageDescPage( Window* pParent, const SfxItemSet& rAttr ) :

    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_PAGE ), rAttr ),

    aPaperSizeFl        ( this, CUI_RES( FL_PAPER_SIZE ) ),
    aPaperFormatText    ( this, CUI_RES( FT_PAPER_FORMAT ) ),
    aPaperSizeBox       ( this, CUI_RES( LB_PAPER_SIZE ) ),
    aPaperWidthText     ( this, CUI_RES( FT_PAPER_WIDTH ) ),
    aPaperWidthEdit     ( this, CUI_RES( ED_PAPER_WIDTH ) ),
    aPaperHeightText    ( this, CUI_RES( FT_PAPER_HEIGHT ) ),
    aPaperHeightEdit    ( this, CUI_RES( ED_PAPER_HEIGHT ) ),
    aOrientationFT      ( this, CUI_RES( FT_ORIENTATION ) ),
    aPortraitBtn        ( this, CUI_RES( RB_PORTRAIT ) ),
    aLandscapeBtn       ( this, CUI_RES( RB_LANDSCAPE ) ),
    aBspWin             ( this, CUI_RES( WN_BSP ) ),
    aTextFlowLbl        ( this, CUI_RES( FT_TEXT_FLOW ) ),
    aTextFlowBox        ( this, CUI_RES( LB_TEXT_FLOW ) ),
    aPaperTrayLbl       ( this, CUI_RES( FT_PAPER_TRAY ) ),
    aPaperTrayBox       ( this, CUI_RES( LB_PAPER_TRAY ) ),
    aMarginFl           ( this, CUI_RES( FL_MARGIN ) ),
    aLeftMarginLbl      ( this, CUI_RES( FT_LEFT_MARGIN ) ),
    aLeftMarginEdit     ( this, CUI_RES( ED_LEFT_MARGIN ) ),
    aRightMarginLbl     ( this, CUI_RES( FT_RIGHT_MARGIN ) ),
    aRightMarginEdit    ( this, CUI_RES( ED_RIGHT_MARGIN ) ),
    aTopMarginLbl       ( this, CUI_RES( FT_TOP_MARGIN ) ),
    aTopMarginEdit      ( this, CUI_RES( ED_TOP_MARGIN ) ),
    aBottomMarginLbl    ( this, CUI_RES( FT_BOTTOM_MARGIN ) ),
    aBottomMarginEdit   ( this, CUI_RES( ED_BOTTOM_MARGIN ) ),
    aBottomSeparatorFl  ( this, CUI_RES( FL_BOTTOM_SEP ) ),
    aLayoutFL           ( this, CUI_RES( FL_LAYOUT ) ),
    aPageText           ( this, CUI_RES( FT_PAGELAYOUT ) ),
    aLayoutBox          ( this, CUI_RES( LB_LAYOUT ) ),
    aNumberFormatText   ( this, CUI_RES( FT_NUMBER_FORMAT ) ),
    aNumberFormatBox    ( this, CUI_RES( LB_NUMBER_FORMAT ) ),
    aTblAlignFT         ( this, CUI_RES( FT_TBL_ALIGN ) ),
    aHorzBox            ( this, CUI_RES( CB_HORZ ) ),
    aVertBox            ( this, CUI_RES( CB_VERT ) ),
    aAdaptBox           ( this, CUI_RES( CB_ADAPT ) ),
    aRegisterCB         ( this, CUI_RES( CB_REGISTER ) ),
    aRegisterFT         ( this, CUI_RES( FT_REGISTER ) ),
    aRegisterLB         ( this, CUI_RES( LB_REGISTER ) ),

    aInsideText         (       CUI_RES( STR_INSIDE ) ),
    aOutsideText        (       CUI_RES( STR_OUTSIDE ) ),
    aPrintRangeQueryText(       CUI_RES( STR_QUERY_PRINTRANGE ) ),

    bLandscape          ( sal_False ),
    eMode               ( SVX_PAGE_MODE_STANDARD ),
    ePaperStart         ( PAPER_A3 ),
    ePaperEnd           ( PAPER_ENV_DL ),
    pImpl               ( new SvxPage_Impl )

{
    bBorderModified = sal_False;
    FreeResource();
    aBspWin.EnableRTL( sal_False );

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    SvtLanguageOptions aLangOptions;
    sal_Bool bCJK = aLangOptions.IsAsianTypographyEnabled();
    sal_Bool bCTL = aLangOptions.IsCTLFontEnabled();
    sal_Bool bWeb = sal_False;
    const SfxPoolItem* pItem;

    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rAttr.GetItemState(SID_HTML_MODE, sal_False, &pItem) ||
        ( 0 != (pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
        bWeb = 0 != (((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON);

    //  fill text flow listbox with valid entries
    aTextFlowBox.InsertEntryValue( CUI_RESSTR( RID_SVXSTR_PAGEDIR_LTR_HORI ), FRMDIR_HORI_LEFT_TOP );
    if( bCTL )
        aTextFlowBox.InsertEntryValue( CUI_RESSTR( RID_SVXSTR_PAGEDIR_RTL_HORI ), FRMDIR_HORI_RIGHT_TOP );
    // #109989# do not show vertical directions in Writer/Web
    if( !bWeb )
    {
        if( bCJK )
        {
            aTextFlowBox.InsertEntryValue( CUI_RESSTR( RID_SVXSTR_PAGEDIR_RTL_VERT ), FRMDIR_VERT_TOP_RIGHT );
//            aTextFlowBox.InsertEntryValue( CUI_RESSTR( RID_SVXSTR_PAGEDIR_LTR_VERT ), FRMDIR_VERT_TOP_LEFT );
        }
    }

    // #109989# show the text direction box in Writer/Web too
    if( (bCJK || bCTL) &&
        SFX_ITEM_UNKNOWN < rAttr.GetItemState(GetWhich( SID_ATTR_FRAMEDIRECTION )))
    {
        aTextFlowLbl.Show();
        aTextFlowBox.Show();
        aTextFlowBox.SetSelectHdl(LINK(this, SvxPageDescPage, FrameDirectionModify_Impl ));

        aBspWin.EnableFrameDirection(sal_True);
    }
    Init_Impl();

    FieldUnit eFUnit = GetModuleFieldUnit( rAttr );
    SetFieldUnit( aLeftMarginEdit, eFUnit );
    SetFieldUnit( aRightMarginEdit, eFUnit );
    SetFieldUnit( aTopMarginEdit, eFUnit );
    SetFieldUnit( aBottomMarginEdit, eFUnit );
    SetFieldUnit( aPaperWidthEdit, eFUnit );
    SetFieldUnit( aPaperHeightEdit, eFUnit );

    if ( SfxViewShell::Current() && SfxViewShell::Current()->GetPrinter() )
        pImpl->mpDefPrinter = (Printer*)SfxViewShell::Current()->GetPrinter();
    else
    {
        pImpl->mpDefPrinter = new Printer;
        pImpl->mbDelPrinter = true;
    }

    MapMode aOldMode = pImpl->mpDefPrinter->GetMapMode();
    pImpl->mpDefPrinter->SetMapMode( MAP_TWIP );

    // First- und Last-Werte f"ur die R"ander setzen
    Size aPaperSize = pImpl->mpDefPrinter->GetPaperSize();
    Size aPrintSize = pImpl->mpDefPrinter->GetOutputSize();
    /*
     * einen Punkt ( 0,0 ) in logische Koordinaten zu konvertieren,
     * sieht aus wie Unsinn; ist aber sinnvoll, wenn der Ursprung des
     * Koordinatensystems verschoben ist.
     */
    Point aPrintOffset = pImpl->mpDefPrinter->GetPageOffset() -
                         pImpl->mpDefPrinter->PixelToLogic( Point() );
    pImpl->mpDefPrinter->SetMapMode( aOldMode );

    long nOffset = !aPrintOffset.X() && !aPrintOffset.Y() ? 0 : PRINT_OFFSET;
    aLeftMarginEdit.SetFirst( aLeftMarginEdit.Normalize( aPrintOffset.X() ), FUNIT_TWIP );
    nFirstLeftMargin = static_cast<long>(aLeftMarginEdit.GetFirst());
    aRightMarginEdit.SetFirst( aRightMarginEdit.Normalize(
        aPaperSize.Width() - aPrintSize.Width() - aPrintOffset.X() + nOffset ), FUNIT_TWIP);
    nFirstRightMargin = static_cast<long>(aRightMarginEdit.GetFirst());
    aTopMarginEdit.SetFirst( aTopMarginEdit.Normalize( aPrintOffset.Y() ), FUNIT_TWIP );
    nFirstTopMargin = static_cast<long>(aTopMarginEdit.GetFirst());
    aBottomMarginEdit.SetFirst( aBottomMarginEdit.Normalize(
        aPaperSize.Height() - aPrintSize.Height() - aPrintOffset.Y() + nOffset ), FUNIT_TWIP );
    nFirstBottomMargin = static_cast<long>(aBottomMarginEdit.GetFirst());
    aLeftMarginEdit.SetLast( aLeftMarginEdit.Normalize(
        aPrintOffset.X() + aPrintSize.Width() ), FUNIT_TWIP );
    nLastLeftMargin = static_cast<long>(aLeftMarginEdit.GetLast());
    aRightMarginEdit.SetLast( aRightMarginEdit.Normalize(
        aPrintOffset.X() + aPrintSize.Width() ), FUNIT_TWIP );
    nLastRightMargin = static_cast<long>(aRightMarginEdit.GetLast());
    aTopMarginEdit.SetLast( aTopMarginEdit.Normalize(
        aPrintOffset.Y() + aPrintSize.Height() ), FUNIT_TWIP );
    nLastTopMargin = static_cast<long>(aTopMarginEdit.GetLast());
    aBottomMarginEdit.SetLast( aBottomMarginEdit.Normalize(
        aPrintOffset.Y() + aPrintSize.Height() ), FUNIT_TWIP );
    nLastBottomMargin = static_cast<long>(aBottomMarginEdit.GetLast());

    // #i4219# get DrawingLayer options
    const SvtOptionsDrawinglayer aDrawinglayerOpt;

    // #i4219# take Maximum now from configuration (1/100th cm)
    // was: 11900 -> 119 cm ;new value 3 meters -> 300 cm -> 30000
    aPaperWidthEdit.SetMax(aPaperWidthEdit.Normalize(aDrawinglayerOpt.GetMaximumPaperWidth()), FUNIT_CM);
    aPaperWidthEdit.SetLast(aPaperWidthEdit.Normalize(aDrawinglayerOpt.GetMaximumPaperWidth()), FUNIT_CM);
    aPaperHeightEdit.SetMax(aPaperHeightEdit.Normalize(aDrawinglayerOpt.GetMaximumPaperHeight()), FUNIT_CM);
    aPaperHeightEdit.SetLast(aPaperHeightEdit.Normalize(aDrawinglayerOpt.GetMaximumPaperHeight()), FUNIT_CM);

    // #i4219# also for margins (1/100th cm). Was: 9999, keeping.
    aLeftMarginEdit.SetMax(aDrawinglayerOpt.GetMaximumPaperLeftMargin());
    aLeftMarginEdit.SetLast(aDrawinglayerOpt.GetMaximumPaperLeftMargin());
    aRightMarginEdit.SetMax(aDrawinglayerOpt.GetMaximumPaperRightMargin());
    aRightMarginEdit.SetLast(aDrawinglayerOpt.GetMaximumPaperRightMargin());
    aTopMarginEdit.SetMax(aDrawinglayerOpt.GetMaximumPaperTopMargin());
    aTopMarginEdit.SetLast(aDrawinglayerOpt.GetMaximumPaperTopMargin());
    aBottomMarginEdit.SetMax(aDrawinglayerOpt.GetMaximumPaperBottomMargin());
    aBottomMarginEdit.SetLast(aDrawinglayerOpt.GetMaximumPaperBottomMargin());

    aPortraitBtn.SetAccessibleRelationMemberOf(&aOrientationFT);
    aLandscapeBtn.SetAccessibleRelationMemberOf(&aOrientationFT);
}

// -----------------------------------------------------------------------

SvxPageDescPage::~SvxPageDescPage()
{
    delete pImpl;
}

// -----------------------------------------------------------------------

void SvxPageDescPage::Init_Impl()
{
    aLeftText = aLeftMarginLbl.GetText();
    aRightText = aRightMarginLbl.GetText();

        // Handler einstellen
    aLayoutBox.SetSelectHdl( LINK( this, SvxPageDescPage, LayoutHdl_Impl ) );
    aPaperSizeBox.SetDropDownLineCount(10);

    aPaperTrayBox.SetGetFocusHdl(
        LINK( this, SvxPageDescPage, PaperBinHdl_Impl ) );
    aPaperSizeBox.SetSelectHdl(
        LINK( this, SvxPageDescPage, PaperSizeSelect_Impl ) );
    aPaperWidthEdit.SetModifyHdl(
        LINK( this, SvxPageDescPage, PaperSizeModify_Impl ) );
    aPaperHeightEdit.SetModifyHdl(
        LINK( this, SvxPageDescPage, PaperSizeModify_Impl ) );
    aLandscapeBtn.SetClickHdl(
        LINK( this, SvxPageDescPage, SwapOrientation_Impl ) );
    aPortraitBtn.SetClickHdl(
        LINK( this, SvxPageDescPage, SwapOrientation_Impl ) );

    Link aLink = LINK( this, SvxPageDescPage, BorderModify_Impl );
    aLeftMarginEdit.SetModifyHdl( aLink );
    aRightMarginEdit.SetModifyHdl( aLink );
    aTopMarginEdit.SetModifyHdl( aLink );
    aBottomMarginEdit.SetModifyHdl( aLink );

    aLink = LINK( this, SvxPageDescPage, RangeHdl_Impl );
    aPaperWidthEdit.SetLoseFocusHdl( aLink );
    aPaperHeightEdit.SetLoseFocusHdl( aLink );
    aLeftMarginEdit.SetLoseFocusHdl( aLink );
    aRightMarginEdit.SetLoseFocusHdl( aLink );
    aTopMarginEdit.SetLoseFocusHdl( aLink );
    aBottomMarginEdit.SetLoseFocusHdl( aLink );

    aHorzBox.SetClickHdl( LINK( this, SvxPageDescPage, CenterHdl_Impl ) );
    aVertBox.SetClickHdl( LINK( this, SvxPageDescPage, CenterHdl_Impl ) );

}

// -----------------------------------------------------------------------

void SvxPageDescPage::Reset( const SfxItemSet& rSet )
{
    SfxItemPool* pPool = rSet.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    SfxMapUnit eUnit = pPool->GetMetric( GetWhich( SID_ATTR_LRSPACE ) );

    // R"ander (Links/Rechts) einstellen
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_LRSPACE );

    if ( pItem )
    {
        const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&)*pItem;
        SetMetricValue( aLeftMarginEdit, rLRSpace.GetLeft(), eUnit );
        aBspWin.SetLeft(
            (sal_uInt16)ConvertLong_Impl( (long)rLRSpace.GetLeft(), eUnit ) );
        SetMetricValue( aRightMarginEdit, rLRSpace.GetRight(), eUnit );
        aBspWin.SetRight(
            (sal_uInt16)ConvertLong_Impl( (long)rLRSpace.GetRight(), eUnit ) );
    }

    // R"ander (Oben/Unten) einstellen
    pItem = GetItem( rSet, SID_ATTR_ULSPACE );

    if ( pItem )
    {
        const SvxULSpaceItem& rULSpace = (const SvxULSpaceItem&)*pItem;
        SetMetricValue( aTopMarginEdit, rULSpace.GetUpper(), eUnit );
        aBspWin.SetTop(
            (sal_uInt16)ConvertLong_Impl( (long)rULSpace.GetUpper(), eUnit ) );
        SetMetricValue( aBottomMarginEdit, rULSpace.GetLower(), eUnit );
        aBspWin.SetBottom(
            (sal_uInt16)ConvertLong_Impl( (long)rULSpace.GetLower(), eUnit ) );
    }

    // allgemeine Seitendaten
    SvxNumType eNumType = SVX_ARABIC;
    bLandscape = ( pImpl->mpDefPrinter->GetOrientation() == ORIENTATION_LANDSCAPE );
    sal_uInt16 nUse = (sal_uInt16)SVX_PAGE_ALL;
    pItem = GetItem( rSet, SID_ATTR_PAGE );

    if ( pItem )
    {
        const SvxPageItem& rItem = (const SvxPageItem&)*pItem;
        eNumType = rItem.GetNumType();
        nUse = rItem.GetPageUsage();
        bLandscape = rItem.IsLandscape();
    }

    // Ausrichtung
    aLayoutBox.SelectEntryPos( ::PageUsageToPos_Impl( nUse ) );
    aBspWin.SetUsage( nUse );
    LayoutHdl_Impl( 0 );

    // Numerierungsart der Seitenvorlage einstellen
    aNumberFormatBox.SelectEntryPos( sal::static_int_cast< sal_uInt16 >(eNumType) );

    // Aktueller Papierschacht
    aPaperTrayBox.Clear();
    sal_uInt8 nPaperBin = PAPERBIN_PRINTER_SETTINGS;
    pItem = GetItem( rSet, SID_ATTR_PAGE_PAPERBIN );

    if ( pItem )
    {
        nPaperBin = ( (const SvxPaperBinItem*)pItem )->GetValue();

        if ( nPaperBin >= pImpl->mpDefPrinter->GetPaperBinCount() )
            nPaperBin = PAPERBIN_PRINTER_SETTINGS;
    }

    String aBinName;

    if ( PAPERBIN_PRINTER_SETTINGS  == nPaperBin )
        aBinName = EE_RESSTR( RID_SVXSTR_PAPERBIN_SETTINGS );
    else
        aBinName = pImpl->mpDefPrinter->GetPaperBinName( (sal_uInt16)nPaperBin );

    sal_uInt16 nEntryPos = aPaperTrayBox.InsertEntry( aBinName );
    aPaperTrayBox.SetEntryData( nEntryPos, (void*)(sal_uLong)nPaperBin );
    aPaperTrayBox.SelectEntry( aBinName );

    // Size rausholen
    Size aPaperSize = SvxPaperInfo::GetPaperSize( pImpl->mpDefPrinter );
    pItem = GetItem( rSet, SID_ATTR_PAGE_SIZE );

    if ( pItem )
        aPaperSize = ( (const SvxSizeItem*)pItem )->GetSize();

    bool bOrientationSupport =
        pImpl->mpDefPrinter->HasSupport( SUPPORT_SET_ORIENTATION );

    if ( !bOrientationSupport &&
         aPaperSize.Width() > aPaperSize.Height() )
        bLandscape = sal_True;

    aLandscapeBtn.Check( bLandscape );
    aPortraitBtn.Check( !bLandscape );

    aBspWin.SetSize( Size( ConvertLong_Impl( aPaperSize.Width(), eUnit ),
                           ConvertLong_Impl( aPaperSize.Height(), eUnit ) ) );

    aPaperSize = OutputDevice::LogicToLogic(aPaperSize, (MapUnit)eUnit, MAP_100TH_MM);
    if ( bLandscape )
        Swap( aPaperSize );

    // Actual Paper Format
    Paper ePaper = SvxPaperInfo::GetSvxPaper( aPaperSize, MAP_100TH_MM, sal_True );

    if ( PAPER_USER != ePaper )
        aPaperSize = SvxPaperInfo::GetPaperSize( ePaper, MAP_100TH_MM );

    if ( bLandscape )
        Swap( aPaperSize );

    // Werte in die Edits eintragen
    SetMetricValue( aPaperHeightEdit, aPaperSize.Height(), SFX_MAPUNIT_100TH_MM );
    SetMetricValue( aPaperWidthEdit, aPaperSize.Width(), SFX_MAPUNIT_100TH_MM );
    aPaperSizeBox.Clear();

    sal_uInt16 nActPos = LISTBOX_ENTRY_NOTFOUND;
    sal_uInt16 nAryId = RID_SVXSTRARY_PAPERSIZE_STD;

    if ( ePaperStart != PAPER_A3 )
        nAryId = RID_SVXSTRARY_PAPERSIZE_DRAW;
    ResStringArray aPaperAry( CUI_RES( nAryId ) );
    sal_uInt32 nCnt = aPaperAry.Count();

    sal_uInt16 nUserPos = LISTBOX_ENTRY_NOTFOUND;
    for ( sal_uInt32 i = 0; i < nCnt; ++i )
    {
        String aStr = aPaperAry.GetString(i);
        Paper eSize = (Paper)aPaperAry.GetValue(i);
        sal_uInt16 nPos = aPaperSizeBox.InsertEntry( aStr );
        aPaperSizeBox.SetEntryData( nPos, (void*)(sal_uLong)eSize );

        if ( eSize == ePaper )
            nActPos = nPos;
        if( eSize == PAPER_USER )
            nUserPos = nPos;
    }
    // preselect current paper format - #115915#: ePaper might not be in aPaperSizeBox so use PAPER_USER instead
    aPaperSizeBox.SelectEntryPos( nActPos != LISTBOX_ENTRY_NOTFOUND ? nActPos : nUserPos );

    // Applikationsspezifisch

    switch ( eMode )
    {
        case SVX_PAGE_MODE_CENTER:
        {
            aTblAlignFT.Show();
            aHorzBox.Show();
            aVertBox.Show();
            DisableVerticalPageDir();

            // Horizontale Ausrichtung
            pItem = GetItem( rSet, SID_ATTR_PAGE_EXT1 );
            aHorzBox.Check( pItem ? ( (const SfxBoolItem*)pItem )->GetValue()
                                  : sal_False );

            // Vertikale Ausrichtung
            pItem = GetItem( rSet, SID_ATTR_PAGE_EXT2 );
            aVertBox.Check( pItem ? ( (const SfxBoolItem*)pItem )->GetValue()
                                  : sal_False );

            // Beispiel-Fenster auf Tabelle setzen
            aBspWin.SetTable( sal_True );
            aBspWin.SetHorz( aHorzBox.IsChecked() );
            aBspWin.SetVert( aVertBox.IsChecked() );

            break;
        }

        case SVX_PAGE_MODE_PRESENTATION:
        {
            DisableVerticalPageDir();
            aAdaptBox.Show();
            pItem = GetItem( rSet, SID_ATTR_PAGE_EXT1 );
            aAdaptBox.Check( pItem ?
                ( (const SfxBoolItem*)pItem )->GetValue() : sal_False );

            //!!! hidden, weil von StarDraw nicht implementiert
            aLayoutBox.Hide();
            aPageText.Hide();

            break;
        }
        default: ;//prevent warning
    }


    // im Beispiel Hintergrund und Umrandung anzeigen
    ResetBackground_Impl( rSet );
//! UpdateExample_Impl();
    RangeHdl_Impl( 0 );

    // Header Footer anzeigen
    InitHeadFoot_Impl( rSet );

    // R"ander auf Hoch/Quer updaten, dann Beispiel updaten
    bBorderModified = sal_False;
    SwapFirstValues_Impl( sal_False );
    UpdateExample_Impl();

    // Alte Werte sichern
    aLeftMarginEdit.SaveValue();
    aRightMarginEdit.SaveValue();
    aTopMarginEdit.SaveValue();
    aBottomMarginEdit.SaveValue();
    aLayoutBox.SaveValue();
    aNumberFormatBox.SaveValue();
    aPaperSizeBox.SaveValue();
    aPaperWidthEdit.SaveValue();
    aPaperHeightEdit.SaveValue();
    aPortraitBtn.SaveValue();
    aLandscapeBtn.SaveValue();
    aPaperTrayBox.SaveValue();
    aVertBox.SaveValue();
    aHorzBox.SaveValue();
    aAdaptBox.SaveValue();

    CheckMarginEdits( true );

    // Registerhaltigkeit
    if(SFX_ITEM_SET == rSet.GetItemState(SID_SWREGISTER_MODE))
    {
        aRegisterCB.Check(((const SfxBoolItem&)rSet.Get(
                                SID_SWREGISTER_MODE)).GetValue());
        aRegisterCB.SaveValue();
        RegisterModify(&aRegisterCB);
    }
    if(SFX_ITEM_SET == rSet.GetItemState(SID_SWREGISTER_COLLECTION))
    {
        aRegisterLB.SelectEntry(
                ((const SfxStringItem&)rSet.Get(SID_SWREGISTER_COLLECTION)).GetValue());
        aRegisterLB.SaveValue();
    }

    SfxItemState eState = rSet.GetItemState( GetWhich( SID_ATTR_FRAMEDIRECTION ),
                                                sal_True, &pItem );
    if( SFX_ITEM_UNKNOWN != eState )
    {
        sal_uInt32 nVal  = SFX_ITEM_SET == eState
                                ? ((SvxFrameDirectionItem*)pItem)->GetValue()
                                : 0;
        aTextFlowBox.SelectEntryValue( static_cast< SvxFrameDirection >( nVal ) );
        aTextFlowBox.SaveValue();
        aBspWin.SetFrameDirection(nVal);
    }
}

// -----------------------------------------------------------------------

void SvxPageDescPage::FillUserData()
{
    if ( SVX_PAGE_MODE_PRESENTATION == eMode )
        SetUserData( UniString::CreateFromInt32( (sal_Int32)aAdaptBox.IsChecked() ) );
}

// -----------------------------------------------------------------------

sal_Bool SvxPageDescPage::FillItemSet( SfxItemSet& rSet )
{
    sal_Bool bModified = sal_False;
    const SfxItemSet& rOldSet = GetItemSet();
    SfxItemPool* pPool = rOldSet.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    sal_uInt16 nWhich = GetWhich( SID_ATTR_LRSPACE );
    SfxMapUnit eUnit = pPool->GetMetric( nWhich );
    const SfxPoolItem* pOld = 0;

    // alten linken und rechten Rand kopieren
    SvxLRSpaceItem aMargin( (const SvxLRSpaceItem&)rOldSet.Get( nWhich ) );

    // alten  oberen und unteren Rand kopieren
    nWhich = GetWhich( SID_ATTR_ULSPACE );
    SvxULSpaceItem aTopMargin( (const SvxULSpaceItem&)rOldSet.Get( nWhich ) );

    if ( aLeftMarginEdit.GetText() != aLeftMarginEdit.GetSavedValue() )
    {
        aMargin.SetLeft( (sal_uInt16)GetCoreValue( aLeftMarginEdit, eUnit ) );
        bModified |= sal_True;
    }

    if ( aRightMarginEdit.GetText() != aRightMarginEdit.GetSavedValue() )
    {
        aMargin.SetRight( (sal_uInt16)GetCoreValue( aRightMarginEdit, eUnit ) );
        bModified |= sal_True;
    }

    // Linken und rechten Rand setzen
    if ( bModified )
    {
        pOld = GetOldItem( rSet, SID_ATTR_LRSPACE );

        if ( !pOld || !( *(const SvxLRSpaceItem*)pOld == aMargin ) )
            rSet.Put( aMargin );
        else
            bModified = sal_False;
    }

    sal_Bool bMod = sal_False;

    if ( aTopMarginEdit.GetText() != aTopMarginEdit.GetSavedValue() )
    {
        aTopMargin.SetUpper( (sal_uInt16)GetCoreValue( aTopMarginEdit, eUnit ) );
        bMod |= sal_True;
    }

    if ( aBottomMarginEdit.GetText() != aBottomMarginEdit.GetSavedValue() )
    {
        aTopMargin.SetLower( (sal_uInt16)GetCoreValue( aBottomMarginEdit, eUnit ) );
        bMod |= sal_True;
    }

    // unteren oberen Rand setzen
    //
    if ( bMod )
    {
        pOld = GetOldItem( rSet, SID_ATTR_ULSPACE );

        if ( !pOld || !( *(const SvxULSpaceItem*)pOld == aTopMargin ) )
        {
            bModified |= sal_True;
            rSet.Put( aTopMargin );
        }
    }

    // Druckerschacht
    nWhich = GetWhich( SID_ATTR_PAGE_PAPERBIN );
    sal_uInt16 nPos = aPaperTrayBox.GetSelectEntryPos();
    sal_uInt16 nBin = (sal_uInt16)(sal_uLong)aPaperTrayBox.GetEntryData( nPos );
    pOld = GetOldItem( rSet, SID_ATTR_PAGE_PAPERBIN );

    if ( !pOld || ( (const SvxPaperBinItem*)pOld )->GetValue() != nBin )
    {
        rSet.Put( SvxPaperBinItem( nWhich, (sal_uInt8)nBin ) );
        bModified |= sal_True;
    }

    nPos = aPaperSizeBox.GetSelectEntryPos();
    Paper ePaper = (Paper)(sal_uLong)aPaperSizeBox.GetEntryData( nPos );
    const sal_uInt16 nOld = aPaperSizeBox.GetSavedValue();
    sal_Bool bChecked = aLandscapeBtn.IsChecked();

    if ( PAPER_USER == ePaper )
    {
        if ( nOld != nPos                       ||
             aPaperWidthEdit.IsValueModified()  ||
             aPaperHeightEdit.IsValueModified() ||
             bChecked != aLandscapeBtn.GetSavedValue() )
        {
            Size aSize( GetCoreValue( aPaperWidthEdit, eUnit ),
                        GetCoreValue( aPaperHeightEdit, eUnit ) );
            pOld = GetOldItem( rSet, SID_ATTR_PAGE_SIZE );

            if ( !pOld || ( (const SvxSizeItem*)pOld )->GetSize() != aSize )
            {
                rSet.Put( SvxSizeItem( GetWhich(SID_ATTR_PAGE_SIZE), aSize ) );
                bModified |= sal_True;
            }
        }
    }
    else
    {
        if ( nOld != nPos || bChecked != aLandscapeBtn.GetSavedValue() )
        {
            Size aSize( SvxPaperInfo::GetPaperSize( ePaper, (MapUnit)eUnit ) );

            if ( bChecked )
                Swap( aSize );

            pOld = GetOldItem( rSet, SID_ATTR_PAGE_SIZE );

            if ( !pOld || ( (const SvxSizeItem*)pOld )->GetSize() != aSize )
            {
                rSet.Put( SvxSizeItem( GetWhich(SID_ATTR_PAGE_SIZE), aSize ) );
                bModified |= sal_True;
            }
        }
    }

    // sonstiges Zeug der Page
    nWhich = GetWhich( SID_ATTR_PAGE );
    SvxPageItem aPage( (const SvxPageItem&)rOldSet.Get( nWhich ) );
    bMod =  aLayoutBox.GetSelectEntryPos()  != aLayoutBox.GetSavedValue();

    if ( bMod )
        aPage.SetPageUsage(
            ::PosToPageUsage_Impl( aLayoutBox.GetSelectEntryPos() ) );

    if ( bChecked != aLandscapeBtn.GetSavedValue() )
    {
        aPage.SetLandscape(bChecked);
        bMod |= sal_True;
    }

    // Einstellen der Numerierungsart der Seite
    nPos = aNumberFormatBox.GetSelectEntryPos();

    if ( nPos != aNumberFormatBox.GetSavedValue() )
    {
        aPage.SetNumType( (SvxNumType)nPos );
        bMod |= sal_True;
    }

    if ( bMod )
    {
        pOld = GetOldItem( rSet, SID_ATTR_PAGE );

        if ( !pOld || !( *(const SvxPageItem*)pOld == aPage ) )
        {
            rSet.Put( aPage );
            bModified |= sal_True;
        }
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich ) )
        rSet.ClearItem( nWhich );
    else
        rSet.Put( rOldSet.Get( nWhich ) );

    // Modispezifische Controls auswerten

    switch ( eMode )
    {
        case SVX_PAGE_MODE_CENTER:
        {
            if ( aHorzBox.IsChecked() != aHorzBox.GetSavedValue() )
            {
                SfxBoolItem aHorz( GetWhich( SID_ATTR_PAGE_EXT1 ),
                                   aHorzBox.IsChecked() );
                rSet.Put( aHorz );
                bModified |= sal_True;
            }

            if ( aVertBox.IsChecked() != aVertBox.GetSavedValue() )
            {
                SfxBoolItem aVert( GetWhich( SID_ATTR_PAGE_EXT2 ),
                                   aVertBox.IsChecked() );
                rSet.Put( aVert );
                bModified |= sal_True;
            }
            break;
        }

        case SVX_PAGE_MODE_PRESENTATION:
        {
            // immer putten, damit Draw das auswerten kann
            rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_PAGE_EXT1 ),
                      aAdaptBox.IsChecked() ) );
            bModified |= sal_True;
            break;
        }
        default: ;//prevent warning

    }

    if(aRegisterCB.IsVisible() &&
        (aRegisterCB.IsChecked() || aRegisterCB.GetSavedValue() != aRegisterCB.IsChecked()))
    {
        const SfxBoolItem& rRegItem = (const SfxBoolItem&)rOldSet.Get(SID_SWREGISTER_MODE);
        SfxBoolItem* pRegItem = (SfxBoolItem*)rRegItem.Clone();
        sal_Bool bCheck = aRegisterCB.IsChecked();
        pRegItem->SetValue(bCheck);
        rSet.Put(*pRegItem);
        bModified |= sal_True;
        if(bCheck)
        {
            bModified |= sal_True;
            rSet.Put(SfxStringItem(SID_SWREGISTER_COLLECTION,
                            aRegisterLB.GetSelectEntry()));
        }
        delete pRegItem;
    }

    SvxFrameDirection eDirection = aTextFlowBox.GetSelectEntryValue();
    if( aTextFlowBox.IsVisible() && (eDirection != aTextFlowBox.GetSavedValue()) )
    {
        rSet.Put( SvxFrameDirectionItem( eDirection, GetWhich( SID_ATTR_FRAMEDIRECTION ) ) );
        bModified = sal_True;
    }

    return bModified;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, LayoutHdl_Impl, ListBox *, EMPTYARG )
{
    // innen au\sen umschalten
    const sal_uInt16 nPos = PosToPageUsage_Impl( aLayoutBox.GetSelectEntryPos() );

    if ( nPos == SVX_PAGE_MIRROR )
    {
        if ( aLeftMarginLbl.GetText() != aInsideText )
            aLeftMarginLbl.SetText( aInsideText );

        if ( aRightMarginLbl.GetText() != aOutsideText )
            aRightMarginLbl.SetText( aOutsideText );
    }
    else
    {
        if ( aLeftMarginLbl.GetText() != aLeftText )
            aLeftMarginLbl.SetText( aLeftText );

        if ( aRightMarginLbl.GetText() != aRightText )
            aRightMarginLbl.SetText( aRightText );
    }
    UpdateExample_Impl( true );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, PaperBinHdl_Impl, ListBox *, EMPTYARG )
{
    if ( aPaperTrayBox.GetEntryCount() > 1 )
        // schon gef"ullt
        return 0;

    // Schacht-Box initialisieren
    String aOldName = aPaperTrayBox.GetSelectEntry();
    aPaperTrayBox.SetUpdateMode( sal_False );
    aPaperTrayBox.Clear();
    sal_uInt16 nEntryPos = aPaperTrayBox.InsertEntry(
        EE_RESSTR( RID_SVXSTR_PAPERBIN_SETTINGS ) );
    aPaperTrayBox.SetEntryData( nEntryPos,
        (void*)(sal_uLong)PAPERBIN_PRINTER_SETTINGS );
    String aPaperBin( EditResId( RID_SVXSTR_PAPERBIN ) );
    sal_uInt16 nBinCount = pImpl->mpDefPrinter->GetPaperBinCount();

    for ( sal_uInt16 i = 0; i < nBinCount; ++i )
    {
        String aName = pImpl->mpDefPrinter->GetPaperBinName(i);

        if ( !aName.Len() )
        {
            aName = aPaperBin;
            aName.Append( sal_Unicode(' ') );
            aName.Append( UniString::CreateFromInt32( i+1 ) );
        }
        nEntryPos = aPaperTrayBox.InsertEntry( aName );
        aPaperTrayBox.SetEntryData( nEntryPos, (void*)(sal_uLong)i );
    }
    aPaperTrayBox.SelectEntry( aOldName );
    aPaperTrayBox.SetUpdateMode( sal_True );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, PaperSizeSelect_Impl, ListBox *, pBox )
{
    const sal_uInt16 nPos = pBox->GetSelectEntryPos();
    Paper ePaper = (Paper)(sal_uLong)aPaperSizeBox.GetEntryData( nPos );

    if ( ePaper != PAPER_USER )
    {
        Size aSize( SvxPaperInfo::GetPaperSize( ePaper, MAP_100TH_MM ) );

        if ( aLandscapeBtn.IsChecked() )
            Swap( aSize );

        if ( aSize.Height() < aPaperHeightEdit.GetMin( FUNIT_100TH_MM ) )
            aPaperHeightEdit.SetMin(
                aPaperHeightEdit.Normalize( aSize.Height() ), FUNIT_100TH_MM );
        if ( aSize.Width() < aPaperWidthEdit.GetMin( FUNIT_100TH_MM ) )
            aPaperWidthEdit.SetMin(
                aPaperWidthEdit.Normalize( aSize.Width() ), FUNIT_100TH_MM );
        SetMetricValue( aPaperHeightEdit, aSize.Height(), SFX_MAPUNIT_100TH_MM );
        SetMetricValue( aPaperWidthEdit, aSize.Width(), SFX_MAPUNIT_100TH_MM );

        // R"ander ggf. neu berechnen
        CalcMargin_Impl();

        RangeHdl_Impl( 0 );
        UpdateExample_Impl( true );

        if ( eMode == SVX_PAGE_MODE_PRESENTATION )
        {
            // Draw: bei Papierformat soll der Rand 1cm betragen
            long nTmp = 0;
            sal_Bool bScreen = ( PAPER_SCREEN == ePaper );

            if ( !bScreen )
                // bei Bildschirm keinen Rand
                nTmp = 1; // entspr. 1cm

            // Abfragen, ob fuer Raender 0 gesetzt ist:
            if ( bScreen || aRightMarginEdit.GetValue() == 0 )
            {
                SetMetricValue( aRightMarginEdit, nTmp, SFX_MAPUNIT_CM );
                if ( !bScreen &&
                     aRightMarginEdit.GetFirst() > aRightMarginEdit.GetValue() )
                    aRightMarginEdit.SetValue( aRightMarginEdit.GetFirst() );
            }
            if ( bScreen || aLeftMarginEdit.GetValue() == 0 )
            {
                SetMetricValue( aLeftMarginEdit, nTmp, SFX_MAPUNIT_CM );
                if ( !bScreen &&
                     aLeftMarginEdit.GetFirst() > aLeftMarginEdit.GetValue() )
                    aLeftMarginEdit.SetValue( aLeftMarginEdit.GetFirst() );
            }
            if ( bScreen || aBottomMarginEdit.GetValue() == 0 )
            {
                SetMetricValue( aBottomMarginEdit, nTmp, SFX_MAPUNIT_CM );
                if ( !bScreen &&
                     aBottomMarginEdit.GetFirst() > aBottomMarginEdit.GetValue() )
                    aBottomMarginEdit.SetValue( aBottomMarginEdit.GetFirst() );
            }
            if ( bScreen || aTopMarginEdit.GetValue() == 0 )
            {
                SetMetricValue( aTopMarginEdit, nTmp, SFX_MAPUNIT_CM );
                if ( !bScreen &&
                     aTopMarginEdit.GetFirst() > aTopMarginEdit.GetValue() )
                    aTopMarginEdit.SetValue( aTopMarginEdit.GetFirst() );
            }
            UpdateExample_Impl( true );
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, PaperSizeModify_Impl, Edit *, EMPTYARG )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_LRSPACE );
    SfxMapUnit eUnit = GetItemSet().GetPool()->GetMetric( nWhich );
    Size aSize( GetCoreValue( aPaperWidthEdit, eUnit ),
                GetCoreValue( aPaperHeightEdit, eUnit ) );
    Paper ePaper = SvxPaperInfo::GetSvxPaper( aSize, (MapUnit)eUnit, sal_True );
    sal_uInt16 nEntryCount = aPaperSizeBox.GetEntryCount();

    for ( sal_uInt16 i = 0; i < nEntryCount; ++i )
    {
        Paper eTmp = (Paper)(sal_uLong)aPaperSizeBox.GetEntryData(i);

        if ( eTmp == ePaper )
        {
            aPaperSizeBox.SelectEntryPos(i);
            break;
        }
    }
    UpdateExample_Impl( true );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, SwapOrientation_Impl, RadioButton *, pBtn )
{
    if (
        (!bLandscape && pBtn == &aLandscapeBtn) ||
        (bLandscape  && pBtn == &aPortraitBtn)
       )
    {
        bLandscape = aLandscapeBtn.IsChecked();

        const long lWidth = GetCoreValue( aPaperWidthEdit, SFX_MAPUNIT_100TH_MM );
        const long lHeight = GetCoreValue( aPaperHeightEdit, SFX_MAPUNIT_100TH_MM );

        // swap with and height
        SetMetricValue( aPaperWidthEdit, lHeight, SFX_MAPUNIT_100TH_MM );
        SetMetricValue( aPaperHeightEdit, lWidth, SFX_MAPUNIT_100TH_MM );

        // recalculate margins if necessary
        CalcMargin_Impl();

        PaperSizeSelect_Impl( &aPaperSizeBox );
        RangeHdl_Impl( 0 );
        SwapFirstValues_Impl( bBorderModified );
        UpdateExample_Impl( true );
    }
    return 0;
}

// -----------------------------------------------------------------------

void SvxPageDescPage::SwapFirstValues_Impl( bool bSet )
{
    MapMode aOldMode = pImpl->mpDefPrinter->GetMapMode();
    Orientation eOri = ORIENTATION_PORTRAIT;

    if ( bLandscape )
        eOri = ORIENTATION_LANDSCAPE;
    Orientation eOldOri = pImpl->mpDefPrinter->GetOrientation();
    pImpl->mpDefPrinter->SetOrientation( eOri );
    pImpl->mpDefPrinter->SetMapMode( MAP_TWIP );

    // First- und Last-Werte f"ur die R"ander setzen
    Size aPaperSize = pImpl->mpDefPrinter->GetPaperSize();
    Size aPrintSize = pImpl->mpDefPrinter->GetOutputSize();
    /*
     * einen Punkt ( 0,0 ) in logische Koordinaten zu konvertieren,
     * sieht aus wie Unsinn; ist aber sinnvoll, wenn der Ursprung des
     * Koordinatensystems verschoben ist.
     */
    Point aPrintOffset = pImpl->mpDefPrinter->GetPageOffset() -
                         pImpl->mpDefPrinter->PixelToLogic( Point() );
    pImpl->mpDefPrinter->SetMapMode( aOldMode );
    pImpl->mpDefPrinter->SetOrientation( eOldOri );

    sal_Int64 nSetL = aLeftMarginEdit.Denormalize(
                    aLeftMarginEdit.GetValue( FUNIT_TWIP ) );
    sal_Int64 nSetR = aRightMarginEdit.Denormalize(
                    aRightMarginEdit.GetValue( FUNIT_TWIP ) );
    sal_Int64 nSetT = aTopMarginEdit.Denormalize(
                    aTopMarginEdit.GetValue( FUNIT_TWIP ) );
    sal_Int64 nSetB = aBottomMarginEdit.Denormalize(
                    aBottomMarginEdit.GetValue( FUNIT_TWIP ) );

    long nOffset = !aPrintOffset.X() && !aPrintOffset.Y() ? 0 : PRINT_OFFSET;
    long nNewL = aPrintOffset.X();
    long nNewR =
        aPaperSize.Width() - aPrintSize.Width() - aPrintOffset.X() + nOffset;
    long nNewT = aPrintOffset.Y();
    long nNewB =
        aPaperSize.Height() - aPrintSize.Height() - aPrintOffset.Y() + nOffset;

    aLeftMarginEdit.SetFirst( aLeftMarginEdit.Normalize( nNewL ), FUNIT_TWIP );
    nFirstLeftMargin = static_cast<long>(aLeftMarginEdit.GetFirst());
    aRightMarginEdit.SetFirst( aRightMarginEdit.Normalize( nNewR ), FUNIT_TWIP );
    nFirstRightMargin = static_cast<long>(aRightMarginEdit.GetFirst());
    aTopMarginEdit.SetFirst( aTopMarginEdit.Normalize( nNewT ), FUNIT_TWIP );
    nFirstTopMargin = static_cast<long>(aTopMarginEdit.GetFirst());
    aBottomMarginEdit.SetFirst( aBottomMarginEdit.Normalize( nNewB ), FUNIT_TWIP );
    nFirstBottomMargin = static_cast<long>(aBottomMarginEdit.GetFirst());

    if ( bSet )
    {
        // ggf. auch die Werte umsetzen,
        if ( nSetL < nNewL )
            aLeftMarginEdit.SetValue( aLeftMarginEdit.Normalize( nNewL ),
                                      FUNIT_TWIP );
        if ( nSetR < nNewR )
            aRightMarginEdit.SetValue( aRightMarginEdit.Normalize( nNewR ),
                                       FUNIT_TWIP );
        if ( nSetT < nNewT )
            aTopMarginEdit.SetValue( aTopMarginEdit.Normalize( nNewT ),
                                     FUNIT_TWIP );
        if ( nSetB < nNewB )
            aBottomMarginEdit.SetValue( aBottomMarginEdit.Normalize( nNewB ),
                                        FUNIT_TWIP );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxPageDescPage, BorderModify_Impl, MetricField *, EMPTYARG )
{
    if ( !bBorderModified )
        bBorderModified = sal_True;
    UpdateExample_Impl();
    return 0;
}
IMPL_LINK_INLINE_END( SvxPageDescPage, BorderModify_Impl, MetricField *, EMPTYARG )

// -----------------------------------------------------------------------

void SvxPageDescPage::UpdateExample_Impl( bool bResetbackground )
{
    // Size
    Size aSize( GetCoreValue( aPaperWidthEdit, SFX_MAPUNIT_TWIP ),
                GetCoreValue( aPaperHeightEdit, SFX_MAPUNIT_TWIP ) );

    aBspWin.SetSize( aSize );

    // R"ander
    aBspWin.SetTop( GetCoreValue( aTopMarginEdit, SFX_MAPUNIT_TWIP ) );
    aBspWin.SetBottom( GetCoreValue( aBottomMarginEdit, SFX_MAPUNIT_TWIP ) );
    aBspWin.SetLeft( GetCoreValue( aLeftMarginEdit, SFX_MAPUNIT_TWIP ) );
    aBspWin.SetRight( GetCoreValue( aRightMarginEdit, SFX_MAPUNIT_TWIP ) );

    // Layout
    aBspWin.SetUsage( PosToPageUsage_Impl( aLayoutBox.GetSelectEntryPos() ) );
    if ( bResetbackground )
        aBspWin.ResetBackground();
    aBspWin.Invalidate();
}

// -----------------------------------------------------------------------

void SvxPageDescPage::ResetBackground_Impl( const SfxItemSet& rSet )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_PAGE_HEADERSET );

    if ( rSet.GetItemState( nWhich, sal_False ) == SFX_ITEM_SET )
    {
        const SvxSetItem& rSetItem =
            (const SvxSetItem&)rSet.Get( nWhich, sal_False );
        const SfxItemSet& rTmpSet = rSetItem.GetItemSet();
        const SfxBoolItem& rOn =
            (const SfxBoolItem&)rTmpSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rOn.GetValue() )
        {
            nWhich = GetWhich( SID_ATTR_BRUSH );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBrushItem& rItem =
                    (const SvxBrushItem&)rTmpSet.Get( nWhich );
                aBspWin.SetHdColor( rItem.GetColor() );
            }
            nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBoxItem& rItem =
                    (const SvxBoxItem&)rTmpSet.Get( nWhich );
                aBspWin.SetHdBorder( rItem );
            }
        }
    }

    nWhich = GetWhich( SID_ATTR_PAGE_FOOTERSET );

    if ( rSet.GetItemState( nWhich, sal_False ) == SFX_ITEM_SET )
    {
        const SvxSetItem& rSetItem =
            (const SvxSetItem&)rSet.Get( nWhich, sal_False );
        const SfxItemSet& rTmpSet = rSetItem.GetItemSet();
        const SfxBoolItem& rOn =
            (const SfxBoolItem&)rTmpSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rOn.GetValue() )
        {
            nWhich = GetWhich( SID_ATTR_BRUSH );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBrushItem& rItem =
                    (const SvxBrushItem&)rTmpSet.Get( nWhich );
                aBspWin.SetFtColor( rItem.GetColor() );
            }
            nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBoxItem& rItem =
                    (const SvxBoxItem&)rTmpSet.Get( nWhich );
                aBspWin.SetFtBorder( rItem );
            }
        }
    }

    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_BRUSH );

    if ( pItem )
    {
        aBspWin.SetColor( ( (const SvxBrushItem*)pItem )->GetColor() );
        const Graphic* pGrf = ( (const SvxBrushItem*)pItem )->GetGraphic();

        if ( pGrf )
        {
            Bitmap aBitmap = pGrf->GetBitmap();
            aBspWin.SetBitmap( &aBitmap );
        }
        else
            aBspWin.SetBitmap( NULL );
    }

    pItem = GetItem( rSet, SID_ATTR_BORDER_OUTER );

    if ( pItem )
        aBspWin.SetBorder( (SvxBoxItem&)*pItem );
}

// -----------------------------------------------------------------------

void SvxPageDescPage::InitHeadFoot_Impl( const SfxItemSet& rSet )
{
    bLandscape = aLandscapeBtn.IsChecked();
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_PAGE_SIZE );

    if ( pItem )
        aBspWin.SetSize( ( (const SvxSizeItem*)pItem )->GetSize() );

    const SvxSetItem* pSetItem = 0;

    // Kopfzeilen-Attribute auswerten

    if ( SFX_ITEM_SET ==
         rSet.GetItemState( GetWhich( SID_ATTR_PAGE_HEADERSET ),
                            sal_False, (const SfxPoolItem**)&pSetItem ) )
    {
        const SfxItemSet& rHeaderSet = pSetItem->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            (const SfxBoolItem&)rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rHeaderOn.GetValue() )
        {
            const SvxSizeItem& rSize = (const SvxSizeItem&)
                rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) );
            const SvxULSpaceItem& rUL = (const SvxULSpaceItem&)
                rHeaderSet.Get( GetWhich( SID_ATTR_ULSPACE ) );
            long nDist = rUL.GetLower();
            aBspWin.SetHdHeight( rSize.GetSize().Height() - nDist );
            aBspWin.SetHdDist( nDist );
            const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)
                rHeaderSet.Get( GetWhich( SID_ATTR_LRSPACE ) );
            aBspWin.SetHdLeft( rLR.GetLeft() );
            aBspWin.SetHdRight( rLR.GetRight() );
            aBspWin.SetHeader( sal_True );
        }
        else
            aBspWin.SetHeader( sal_False );

        // im Beispiel Hintergrund und Umrandung anzeigen
        sal_uInt16 nWhich = GetWhich( SID_ATTR_BRUSH );

        if ( rHeaderSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
        {
            const SvxBrushItem& rItem =
                (const SvxBrushItem&)rHeaderSet.Get( nWhich );
            aBspWin.SetHdColor( rItem.GetColor() );
        }
        nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

        if ( rHeaderSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
        {
            const SvxBoxItem& rItem =
                (const SvxBoxItem&)rHeaderSet.Get( nWhich );
            aBspWin.SetHdBorder( rItem );
        }
    }

    // Fusszeilen-Attribute auswerten

    if ( SFX_ITEM_SET ==
         rSet.GetItemState( GetWhich( SID_ATTR_PAGE_FOOTERSET ),
                            sal_False, (const SfxPoolItem**)&pSetItem ) )
    {
        const SfxItemSet& rFooterSet = pSetItem->GetItemSet();
        const SfxBoolItem& rFooterOn =
            (const SfxBoolItem&)rFooterSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rFooterOn.GetValue() )
        {
            const SvxSizeItem& rSize = (const SvxSizeItem&)
                rFooterSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) );
            const SvxULSpaceItem& rUL = (const SvxULSpaceItem&)
                rFooterSet.Get( GetWhich( SID_ATTR_ULSPACE ) );
            long nDist = rUL.GetUpper();
            aBspWin.SetFtHeight( rSize.GetSize().Height() - nDist );
            aBspWin.SetFtDist( nDist );
            const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)
                rFooterSet.Get( GetWhich( SID_ATTR_LRSPACE ) );
            aBspWin.SetFtLeft( rLR.GetLeft() );
            aBspWin.SetFtRight( rLR.GetRight() );
            aBspWin.SetFooter( sal_True );
        }
        else
            aBspWin.SetFooter( sal_False );

        // im Beispiel Hintergrund und Umrandung anzeigen
        sal_uInt16 nWhich = GetWhich( SID_ATTR_BRUSH );

        if ( rFooterSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
        {
            const SvxBrushItem& rItem =
                (const SvxBrushItem&)rFooterSet.Get( nWhich );
            aBspWin.SetFtColor( rItem.GetColor() );
        }
        nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

        if ( rFooterSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
        {
            const SvxBoxItem& rItem =
                (const SvxBoxItem&)rFooterSet.Get( nWhich );
            aBspWin.SetFtBorder( rItem );
        }
    }
}

// -----------------------------------------------------------------------

void SvxPageDescPage::ActivatePage( const SfxItemSet& rSet )
{
    InitHeadFoot_Impl( rSet );
    ResetBackground_Impl( rSet );
    RangeHdl_Impl( 0 );
}

// -----------------------------------------------------------------------

int SvxPageDescPage::DeactivatePage( SfxItemSet* _pSet )
{
    // Abfrage, ob die Seitenr"ander ausserhalb des Druckbereichs liegen
    // Wenn nicht, dann den Anwender fragen, ob sie "ubernommen werden sollen.
    // Wenn nicht, dann auf der TabPage bleiben.
    sal_uInt16 nPos = aPaperSizeBox.GetSelectEntryPos();
    Paper ePaper = (Paper)(sal_uLong)aPaperSizeBox.GetEntryData( nPos );

    if ( ePaper != PAPER_SCREEN && IsMarginOutOfRange() )
    {
        if ( QueryBox( this, WB_YES_NO | WB_DEF_NO, aPrintRangeQueryText ).Execute() == RET_NO )
        {
            MetricField* pField = NULL;
            if ( IsPrinterRangeOverflow( aLeftMarginEdit, nFirstLeftMargin, nLastLeftMargin, MARGIN_LEFT ) )
                pField = &aLeftMarginEdit;
            if (    IsPrinterRangeOverflow( aRightMarginEdit, nFirstRightMargin, nLastRightMargin, MARGIN_RIGHT )
                 && !pField )
                pField = &aRightMarginEdit;
            if (    IsPrinterRangeOverflow( aTopMarginEdit, nFirstTopMargin, nLastTopMargin, MARGIN_TOP )
                 && !pField )
                pField = &aTopMarginEdit;
            if (    IsPrinterRangeOverflow( aBottomMarginEdit, nFirstBottomMargin, nLastBottomMargin, MARGIN_BOTTOM )
                 && !pField )
                pField = &aBottomMarginEdit;
            if ( pField )
                pField->GrabFocus();
            UpdateExample_Impl();
            return KEEP_PAGE;
        }
        else
            CheckMarginEdits( false );
    }

    if ( _pSet )
    {
        FillItemSet( *_pSet );

        // ggf. hoch/quer putten
        sal_uInt16 nWh = GetWhich( SID_ATTR_PAGE_SIZE );
        SfxMapUnit eUnit = GetItemSet().GetPool()->GetMetric( nWh );
        Size aSize( GetCoreValue( aPaperWidthEdit, eUnit ),
                    GetCoreValue( aPaperHeightEdit, eUnit ) );

        // putten, wenn aktuelle Gr"o/se unterschiedlich zum Wert in _pSet
        const SvxSizeItem* pSize = (const SvxSizeItem*)GetItem( *_pSet, SID_ATTR_PAGE_SIZE );
        if ( aSize.Width() && ( !pSize || !IsEqualSize_Impl( pSize, aSize ) ) )
            _pSet->Put( SvxSizeItem( nWh, aSize ) );
    }

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, RangeHdl_Impl, Edit *, EMPTYARG )
{
    // Aktuelle Header-Breite/H"ohe aus dem Bsp
    long nHHeight = aBspWin.GetHdHeight();
    long nHDist = aBspWin.GetHdDist();

    // Aktuelle Footer-Breite/H"ohe aus dem Bsp
    long nFHeight = aBspWin.GetFtHeight();
    long nFDist = aBspWin.GetFtDist();

    // Aktuelle Header/Footer-R"ander aus dem Bsp
    long nHFLeft = Max( aBspWin.GetHdLeft(), aBspWin.GetFtLeft() );
    long nHFRight = Max( aBspWin.GetHdRight(), aBspWin.GetFtRight() );

    // Aktuelle Werte der Seitenr"ander
    long nBT = static_cast<long>(aTopMarginEdit.Denormalize(aTopMarginEdit.GetValue(FUNIT_TWIP)));
    long nBB = static_cast<long>(aBottomMarginEdit.Denormalize(aBottomMarginEdit.GetValue(FUNIT_TWIP)));
    long nBL = static_cast<long>(aLeftMarginEdit.Denormalize(aLeftMarginEdit.GetValue(FUNIT_TWIP)));
    long nBR = static_cast<long>(aRightMarginEdit.Denormalize(aRightMarginEdit.GetValue(FUNIT_TWIP)));

    // Breite Umrandung der Seite berechnen
    const SfxItemSet* _pSet = &GetItemSet();
    Size aBorder;

    if ( _pSet->GetItemState( GetWhich(SID_ATTR_BORDER_SHADOW) ) >=
            SFX_ITEM_AVAILABLE &&
         _pSet->GetItemState( GetWhich(SID_ATTR_BORDER_OUTER)  ) >=
            SFX_ITEM_AVAILABLE )
    {
        aBorder = ( GetMinBorderSpace_Impl(
            (const SvxShadowItem&)_pSet->Get(GetWhich(SID_ATTR_BORDER_SHADOW)),
            (const SvxBoxItem&)_pSet->Get(GetWhich(SID_ATTR_BORDER_OUTER))));
    }

    long nH  = static_cast<long>(aPaperHeightEdit.Denormalize(aPaperHeightEdit.GetValue(FUNIT_TWIP)));
    long nW  = static_cast<long>(aPaperWidthEdit.Denormalize(aPaperWidthEdit.GetValue(FUNIT_TWIP)));

    // Grenzen Papier
    // Maximum liegt bei 54cm
    //
    long nMin = nHHeight + nHDist + nFDist + nFHeight + nBT + nBB +
                MINBODY + aBorder.Height();
    aPaperHeightEdit.SetMin(aPaperHeightEdit.Normalize(nMin), FUNIT_TWIP);

    nMin = MINBODY + nBL + nBR + aBorder.Width();
    aPaperWidthEdit.SetMin(aPaperWidthEdit.Normalize(nMin), FUNIT_TWIP);

    // Falls sich die Papiergr"o\se ge"adert hat
    nH = static_cast<long>(aPaperHeightEdit.Denormalize(aPaperHeightEdit.GetValue(FUNIT_TWIP)));
    nW = static_cast<long>(aPaperWidthEdit.Denormalize(aPaperWidthEdit.GetValue(FUNIT_TWIP)));

    // Top
    long nMax = nH - nBB - aBorder.Height() - MINBODY -
                nFDist - nFHeight - nHDist - nHHeight;

    aTopMarginEdit.SetMax(aTopMarginEdit.Normalize(nMax), FUNIT_TWIP);

    // Bottom
    nMax = nH - nBT - aBorder.Height() - MINBODY -
           nFDist - nFHeight - nHDist - nHHeight;

    aBottomMarginEdit.SetMax(aTopMarginEdit.Normalize(nMax), FUNIT_TWIP);

    // Left
    nMax = nW - nBR - MINBODY - aBorder.Width() - nHFLeft - nHFRight;
    aLeftMarginEdit.SetMax(aLeftMarginEdit.Normalize(nMax), FUNIT_TWIP);

    // Right
    nMax = nW - nBL - MINBODY - aBorder.Width() - nHFLeft - nHFRight;
    aRightMarginEdit.SetMax(aRightMarginEdit.Normalize(nMax), FUNIT_TWIP);
    return 0;
}

// -----------------------------------------------------------------------

void SvxPageDescPage::CalcMargin_Impl()
{
    // Aktuelle Werte der Seitenr"ander
    long nBT = GetCoreValue( aTopMarginEdit, SFX_MAPUNIT_TWIP );
    long nBB = GetCoreValue( aBottomMarginEdit, SFX_MAPUNIT_TWIP );

    long nBL = GetCoreValue( aLeftMarginEdit, SFX_MAPUNIT_TWIP );
    long nBR = GetCoreValue( aRightMarginEdit, SFX_MAPUNIT_TWIP );

    long nH  = GetCoreValue( aPaperHeightEdit, SFX_MAPUNIT_TWIP );
    long nW  = GetCoreValue( aPaperWidthEdit, SFX_MAPUNIT_TWIP );

    long nWidth = nBL + nBR + MINBODY;
    long nHeight = nBT + nBB + MINBODY;

    if ( nWidth > nW || nHeight > nH )
    {
        if ( nWidth > nW )
        {
            long nTmp = nBL <= nBR ? nBR : nBL;
            nTmp -= nWidth - nW;

            if ( nBL <= nBR )
                SetMetricValue( aRightMarginEdit, nTmp, SFX_MAPUNIT_TWIP );
            else
                SetMetricValue( aLeftMarginEdit, nTmp, SFX_MAPUNIT_TWIP );
        }

        if ( nHeight > nH )
        {
            long nTmp = nBT <= nBB ? nBB : nBT;
            nTmp -= nHeight - nH;

            if ( nBT <= nBB )
                SetMetricValue( aBottomMarginEdit, nTmp, SFX_MAPUNIT_TWIP );
            else
                SetMetricValue( aTopMarginEdit, nTmp, SFX_MAPUNIT_TWIP );
        }
    }
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxPageDescPage, CenterHdl_Impl, CheckBox *, EMPTYARG )
{
    aBspWin.SetHorz( aHorzBox.IsChecked() );
    aBspWin.SetVert( aVertBox.IsChecked() );
    UpdateExample_Impl();
    return 0;
}
IMPL_LINK_INLINE_END( SvxPageDescPage, CenterHdl_Impl, CheckBox *, EMPTYARG )

// -----------------------------------------------------------------------

void SvxPageDescPage::SetCollectionList(const std::vector<String> &aList)
{
    OSL_ENSURE(!aList.empty(), "Empty string list");

    sStandardRegister = aList[0];
    for( sal_uInt16 i = 1; i < aList.size(); i++   )
        aRegisterLB.InsertEntry(aList[i]);

    aRegisterCB  .Show();
    aRegisterFT  .Show();
    aRegisterLB.Show();
    aRegisterCB.SetClickHdl(LINK(this, SvxPageDescPage, RegisterModify));
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, RegisterModify, CheckBox*, pBox )
{
    sal_Bool bEnable = sal_False;
    if(pBox->IsChecked())
    {
        bEnable = sal_True;
        if(USHRT_MAX == aRegisterLB.GetSelectEntryPos())
            aRegisterLB.SelectEntry(sStandardRegister);
    }
    aRegisterFT.Enable( bEnable );
    aRegisterLB.Enable( bEnable );
    return 0;
}

// ----------------------------------------------------------------------------

void SvxPageDescPage::DisableVerticalPageDir()
{
    aTextFlowBox.RemoveEntryValue( FRMDIR_VERT_TOP_RIGHT );
    aTextFlowBox.RemoveEntryValue( FRMDIR_VERT_TOP_LEFT );
    if( aTextFlowBox.GetEntryCount() < 2 )
    {
        aTextFlowLbl.Hide();
        aTextFlowBox.Hide();
        aBspWin.EnableFrameDirection( sal_False );
    }
}

IMPL_LINK( SvxPageDescPage, FrameDirectionModify_Impl, ListBox*,  EMPTYARG)
{
    aBspWin.SetFrameDirection( (sal_uInt32) aTextFlowBox.GetSelectEntryValue() );
    aBspWin.Invalidate();
    return 0;
}

bool SvxPageDescPage::IsPrinterRangeOverflow(
    MetricField& rField, long nFirstMargin, long nLastMargin, MarginPosition nPos )
{
    bool bRet = false;
    bool bCheck = ( ( pImpl->m_nPos & nPos ) == 0 );
    long nValue = static_cast<long>(rField.GetValue());
    if ( bCheck &&
         (  nValue < nFirstMargin || nValue > nLastMargin ) &&
         rField.GetText() != rField.GetSavedValue() )
    {
        rField.SetValue( nValue < nFirstMargin ? nFirstMargin : nLastMargin );
         bRet = true;
    }

    return bRet;
}

/** Check if a value of a margin edit is outside the printer paper margins
    and save this information.
*/
void SvxPageDescPage::CheckMarginEdits( bool _bClear )
{
    if ( _bClear )
        pImpl->m_nPos = 0;

    sal_Int64 nValue = aLeftMarginEdit.GetValue();
    if (  nValue < nFirstLeftMargin || nValue > nLastLeftMargin )
        pImpl->m_nPos |= MARGIN_LEFT;
    nValue = aRightMarginEdit.GetValue();
    if (  nValue < nFirstRightMargin || nValue > nLastRightMargin )
        pImpl->m_nPos |= MARGIN_RIGHT;
    nValue = aTopMarginEdit.GetValue();
    if (  nValue < nFirstTopMargin || nValue > nLastTopMargin )
        pImpl->m_nPos |= MARGIN_TOP;
    nValue = aBottomMarginEdit.GetValue();
    if (  nValue < nFirstBottomMargin || nValue > nLastBottomMargin )
        pImpl->m_nPos |= MARGIN_BOTTOM;
}

bool SvxPageDescPage::IsMarginOutOfRange()
{
    bool bRet = ( ( ( !( pImpl->m_nPos & MARGIN_LEFT ) &&
                      ( aLeftMarginEdit.GetText() != aLeftMarginEdit.GetSavedValue() ) ) &&
                    ( aLeftMarginEdit.GetValue() < nFirstLeftMargin ||
                      aLeftMarginEdit.GetValue() > nLastLeftMargin ) ) ||
                  ( ( !( pImpl->m_nPos & MARGIN_RIGHT ) &&
                      ( aRightMarginEdit.GetText() != aRightMarginEdit.GetSavedValue() ) ) &&
                    ( aRightMarginEdit.GetValue() < nFirstRightMargin ||
                      aRightMarginEdit.GetValue() > nLastRightMargin ) ) ||
                  ( ( !( pImpl->m_nPos & MARGIN_TOP ) &&
                      ( aTopMarginEdit.GetText() != aTopMarginEdit.GetSavedValue() ) ) &&
                    ( aTopMarginEdit.GetValue() < nFirstTopMargin ||
                      aTopMarginEdit.GetValue() > nLastTopMargin ) ) ||
                  ( ( !( pImpl->m_nPos & MARGIN_BOTTOM ) &&
                      ( aBottomMarginEdit.GetText() != aBottomMarginEdit.GetSavedValue() ) ) &&
                    ( aBottomMarginEdit.GetValue() < nFirstBottomMargin ||
                      aBottomMarginEdit.GetValue() > nLastBottomMargin ) ) );
    return bRet;
}

void SvxPageDescPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pModeItem,SfxAllEnumItem,SID_ENUM_PAGE_MODE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pPaperStartItem,SfxAllEnumItem,SID_PAPER_START,sal_False);
    SFX_ITEMSET_ARG (&aSet,pPaperEndItem,SfxAllEnumItem,SID_PAPER_END,sal_False);
    SFX_ITEMSET_ARG (&aSet,pCollectListItem,SfxStringListItem,SID_COLLECT_LIST,sal_False);
    if (pModeItem)
        SetMode((SvxModeType)pModeItem->GetEnumValue());
    if (pPaperStartItem && pPaperEndItem)
        SetPaperFormatRanges( (Paper)pPaperStartItem->GetEnumValue(), (Paper)pPaperEndItem->GetEnumValue() );
    if (pCollectListItem)
        SetCollectionList(pCollectListItem->GetList());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
