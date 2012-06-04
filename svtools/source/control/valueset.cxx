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

#include <tools/debug.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/help.hxx>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <rtl/ustring.hxx>

#include "valueimp.hxx"

#define _SV_VALUESET_CXX
#include <svtools/valueset.hxx>

// ------------
// - ValueSet -
// ------------

void ValueSet::ImplInit()
{
    // Size aWinSize        = GetSizePixel();
    mpImpl              = new ValueSet_Impl;
    mpNoneItem          = NULL;
    mpScrBar            = NULL;
    mnTextOffset        = 0;
    mnVisLines          = 0;
    mnLines             = 0;
    mnUserItemWidth     = 0;
    mnUserItemHeight    = 0;
    mnFirstLine         = 0;
    mnOldItemId         = 0;
    mnSelItemId         = 0;
    mnHighItemId        = 0;
    mnDropPos           = VALUESET_ITEM_NOTFOUND;
    mnCols              = 0;
    mnCurCol            = 0;
    mnUserCols          = 0;
    mnUserVisLines      = 0;
    mnSpacing           = 0;
    mnFrameStyle        = 0;
    mbFormat            = sal_True;
    mbHighlight         = sal_False ;
    mbSelection         = sal_False;
    mbNoSelection       = sal_True;
    mbDrawSelection     = sal_True;
    mbBlackSel          = sal_False;
    mbDoubleSel         = sal_False;
    mbScroll            = sal_False;
    mbDropPos           = sal_False;
    mbFullMode          = sal_True;

    // #106446#, #106601# force mirroring of virtual device
    maVirDev.EnableRTL( GetParent()->IsRTLEnabled() );

    ImplInitSettings( sal_True, sal_True, sal_True );
}

// -----------------------------------------------------------------------

ValueSet::ValueSet( Window* pParent, WinBits nWinStyle, bool bDisableTransientChildren ) :
    Control( pParent, nWinStyle ),
    maVirDev( *this ),
    maColor( COL_TRANSPARENT )
{
    ImplInit();
    if( mpImpl )
        mpImpl->mbIsTransientChildrenDisabled = bDisableTransientChildren;
}

// -----------------------------------------------------------------------

ValueSet::ValueSet( Window* pParent, const ResId& rResId, bool bDisableTransientChildren ) :
    Control( pParent, rResId ),
    maVirDev( *this ),
    maColor( COL_TRANSPARENT )
{
    ImplInit();
    if( mpImpl )
        mpImpl->mbIsTransientChildrenDisabled = bDisableTransientChildren;
}

// -----------------------------------------------------------------------

ValueSet::~ValueSet()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>
          xComponent (GetAccessible(sal_False), ::com::sun::star::uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose ();

    if ( mpScrBar )
        delete mpScrBar;

    if ( mpNoneItem )
        delete mpNoneItem;

    ImplDeleteItems();
    delete mpImpl;
}

// -----------------------------------------------------------------------

void ValueSet::ImplDeleteItems()
{
    for ( size_t i = 0, n = mpImpl->mpItemList->size(); i < n; ++i )
    {
        ValueSetItem* pItem = (*mpImpl->mpItemList)[ i ];
        if( !pItem->maRect.IsEmpty() && ImplHasAccessibleListeners() )
        {
            ::com::sun::star::uno::Any aOldAny, aNewAny;

            aOldAny <<= pItem->GetAccessible( mpImpl->mbIsTransientChildrenDisabled );
            ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
        }

        delete pItem;
    }

    mpImpl->mpItemList->clear();
}

// -----------------------------------------------------------------------

void ValueSet::ImplInitSettings( sal_Bool bFont,
                                 sal_Bool bForeground, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont;
        aFont = rStyleSettings.GetAppFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );
    }

    if ( bForeground || bFont )
    {
        Color aColor;
        if ( IsControlForeground() )
            aColor = GetControlForeground();
        else
            aColor = rStyleSettings.GetButtonTextColor();
        SetTextColor( aColor );
        SetTextFillColor();
    }

    if ( bBackground )
    {
        Color aColor;
        if ( IsControlBackground() )
            aColor = GetControlBackground();
        else if ( GetStyle() & WB_MENUSTYLEVALUESET )
            aColor = rStyleSettings.GetMenuColor();
        else if ( IsEnabled() && (GetStyle() & WB_FLATVALUESET) )
            aColor = rStyleSettings.GetWindowColor();
        else
            aColor = rStyleSettings.GetFaceColor();
        SetBackground( aColor );
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplInitScrollBar()
{
    if ( GetStyle() & WB_VSCROLL )
    {
        if ( !mpScrBar )
        {
            mpScrBar = new ScrollBar( this, WB_VSCROLL | WB_DRAG );
            mpScrBar->SetScrollHdl( LINK( this, ValueSet, ImplScrollHdl ) );
        }
        else
        {
            // Wegen Einstellungsaenderungen passen wir hier die Breite an
            long nScrBarWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
            mpScrBar->SetPosSizePixel( 0, 0, nScrBarWidth, 0, WINDOW_POSSIZE_WIDTH );
        }
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplFormatItem( ValueSetItem* pItem )
{
    if ( pItem->meType == VALUESETITEM_SPACE )
        return;

    Rectangle aRect = pItem->maRect;
    WinBits nStyle = GetStyle();
    if ( nStyle & WB_ITEMBORDER )
    {
        aRect.Left()++;
        aRect.Top()++;
        aRect.Right()--;
        aRect.Bottom()--;
        if ( nStyle & WB_FLATVALUESET )
        {
            if ( nStyle  & WB_DOUBLEBORDER )
            {
                aRect.Left()    += 2;
                aRect.Top()     += 2;
                aRect.Right()   -= 2;
                aRect.Bottom()  -= 2;
            }
            else
            {
                aRect.Left()++;
                aRect.Top()++;
                aRect.Right()--;
                aRect.Bottom()--;
            }
        }
        else
        {
            DecorationView aView( &maVirDev );
            aRect = aView.DrawFrame( aRect, mnFrameStyle );
        }
    }

    if ( pItem == mpNoneItem )
        pItem->maText = GetText();

    if ( (aRect.GetHeight() > 0) && (aRect.GetWidth() > 0) )
    {
        if ( pItem == mpNoneItem )
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            maVirDev.SetFont( GetFont() );
            maVirDev.SetTextColor( ( nStyle & WB_MENUSTYLEVALUESET ) ? rStyleSettings.GetMenuTextColor() : rStyleSettings.GetWindowTextColor() );
            maVirDev.SetTextFillColor();
            maVirDev.SetFillColor( ( nStyle & WB_MENUSTYLEVALUESET ) ? rStyleSettings.GetMenuColor() : rStyleSettings.GetWindowColor() );
            maVirDev.DrawRect( aRect );
            Point   aTxtPos( aRect.Left()+2, aRect.Top() );
            long    nTxtWidth = GetTextWidth( pItem->maText );
            if ( nStyle & WB_RADIOSEL )
            {
                aTxtPos.X() += 4;
                aTxtPos.Y() += 4;
            }
            if ( (aTxtPos.X()+nTxtWidth) > aRect.Right() )
            {
                maVirDev.SetClipRegion( Region( aRect ) );
                maVirDev.DrawText( aTxtPos, pItem->maText );
                maVirDev.SetClipRegion();
            }
            else
                maVirDev.DrawText( aTxtPos, pItem->maText );
        }
        else if ( pItem->meType == VALUESETITEM_COLOR )
        {
            maVirDev.SetFillColor( pItem->maColor );
            maVirDev.DrawRect( aRect );
        }
        else
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            if ( IsColor() )
                maVirDev.SetFillColor( maColor );
            else if ( nStyle & WB_MENUSTYLEVALUESET )
                maVirDev.SetFillColor( rStyleSettings.GetMenuColor() );
            else if ( IsEnabled() )
                maVirDev.SetFillColor( rStyleSettings.GetWindowColor() );
            else
                maVirDev.SetFillColor( rStyleSettings.GetFaceColor() );
            maVirDev.DrawRect( aRect );

            if ( pItem->meType == VALUESETITEM_USERDRAW )
            {
                UserDrawEvent aUDEvt( &maVirDev, aRect, pItem->mnId );
                UserDraw( aUDEvt );
            }
            else
            {
                Size    aImageSize = pItem->maImage.GetSizePixel();
                Size    aRectSize = aRect.GetSize();
                Point   aPos( aRect.Left(), aRect.Top() );
                aPos.X() += (aRectSize.Width()-aImageSize.Width())/2;
                aPos.Y() += (aRectSize.Height()-aImageSize.Height())/2;

                sal_uInt16  nImageStyle  = 0;
                if( !IsEnabled() )
                    nImageStyle  |= IMAGE_DRAW_DISABLE;

                if ( (aImageSize.Width()  > aRectSize.Width()) ||
                     (aImageSize.Height() > aRectSize.Height()) )
                {
                    maVirDev.SetClipRegion( Region( aRect ) );
                    maVirDev.DrawImage( aPos, pItem->maImage, nImageStyle);
                    maVirDev.SetClipRegion();
                }
                else
                    maVirDev.DrawImage( aPos, pItem->maImage, nImageStyle );
            }
        }
    }
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > ValueSet::CreateAccessible()
{
    return new ValueSetAcc( this, mpImpl->mbIsTransientChildrenDisabled );
}

// -----------------------------------------------------------------------

void ValueSet::Format()
{
    Size        aWinSize = GetOutputSizePixel();
    size_t      nItemCount = mpImpl->mpItemList->size();
    WinBits     nStyle = GetStyle();
    long        nTxtHeight = GetTextHeight();
    long        nOff;
    long        nSpace;
    long        nNoneHeight;
    long        nNoneSpace;
    ScrollBar*  pDelScrBar = NULL;

    // Scrolling beruecksichtigen
    if ( nStyle & WB_VSCROLL )
        ImplInitScrollBar();
    else
    {
        if ( mpScrBar )
        {
            // ScrollBar erst spaeter zerstoeren, damit keine rekursiven
            // Aufrufe entstehen koennen
            pDelScrBar = mpScrBar;
            mpScrBar = NULL;
        }
    }

    // Item-Offset berechnen
    if ( nStyle & WB_ITEMBORDER )
    {
        if ( nStyle & WB_DOUBLEBORDER )
            nOff = ITEM_OFFSET_DOUBLE;
        else
            nOff = ITEM_OFFSET;
    }
    else
        nOff = 0;
    nSpace = mnSpacing;

    // Groesse beruecksichtigen, wenn NameField vorhanden
    if ( nStyle & WB_NAMEFIELD )
    {
        mnTextOffset = aWinSize.Height()-nTxtHeight-NAME_OFFSET;
        aWinSize.Height() -= nTxtHeight+NAME_OFFSET;

        if ( !(nStyle & WB_FLATVALUESET) )
        {
            mnTextOffset -= NAME_LINE_HEIGHT+NAME_LINE_OFF_Y;
            aWinSize.Height() -= NAME_LINE_HEIGHT+NAME_LINE_OFF_Y;
        }
    }
    else
        mnTextOffset = 0;

    // Offset und Groesse beruecksichtigen, wenn NoneField vorhanden
    if ( nStyle & WB_NONEFIELD )
    {
        nNoneHeight = nTxtHeight+nOff;
        nNoneSpace = nSpace;
        if ( nStyle & WB_RADIOSEL )
            nNoneHeight += 8;
    }
    else
    {
        nNoneHeight = 0;
        nNoneSpace = 0;

        if ( mpNoneItem )
        {
            delete mpNoneItem;
            mpNoneItem = NULL;
        }
    }

    // Breite vom ScrollBar berechnen
    long nScrBarWidth = 0;
    if ( mpScrBar )
        nScrBarWidth = mpScrBar->GetSizePixel().Width()+SCRBAR_OFFSET;

    // Spaltenanzahl berechnen
    if ( !mnUserCols )
    {
        if ( mnUserItemWidth )
        {
            mnCols = (sal_uInt16)((aWinSize.Width()-nScrBarWidth+nSpace) / (mnUserItemWidth+nSpace));
            if ( !mnCols )
                mnCols = 1;
        }
        else
            mnCols = 1;
    }
    else
        mnCols = mnUserCols;

    // Zeilenanzahl berechnen
    mbScroll = sal_False;
    mnLines = (long)mpImpl->mpItemList->size() / mnCols;
    if ( mpImpl->mpItemList->size() % mnCols )
        mnLines++;
    else if ( !mnLines )
        mnLines = 1;

    long nCalcHeight = aWinSize.Height()-nNoneHeight;
    if ( mnUserVisLines )
        mnVisLines = mnUserVisLines;
    else if ( mnUserItemHeight )
    {
        mnVisLines = (nCalcHeight-nNoneSpace+nSpace) / (mnUserItemHeight+nSpace);
        if ( !mnVisLines )
            mnVisLines = 1;
    }
    else
        mnVisLines = mnLines;
    if ( mnLines > mnVisLines )
        mbScroll = sal_True;
    if ( mnLines <= mnVisLines )
        mnFirstLine = 0;
    else
    {
        if ( mnFirstLine > (sal_uInt16)(mnLines-mnVisLines) )
            mnFirstLine = (sal_uInt16)(mnLines-mnVisLines);
    }

    // Itemgroessen berechnen
    long nColSpace  = (mnCols-1)*nSpace;
    long nLineSpace = ((mnVisLines-1)*nSpace)+nNoneSpace;
    long nItemWidth;
    long nItemHeight;
    if ( mnUserItemWidth && !mnUserCols )
    {
        nItemWidth = mnUserItemWidth;
        if ( nItemWidth > aWinSize.Width()-nScrBarWidth-nColSpace )
            nItemWidth = aWinSize.Width()-nScrBarWidth-nColSpace;
    }
    else
        nItemWidth = (aWinSize.Width()-nScrBarWidth-nColSpace) / mnCols;
    if ( mnUserItemHeight && !mnUserVisLines )
    {
        nItemHeight = mnUserItemHeight;
        if ( nItemHeight > nCalcHeight-nNoneSpace )
            nItemHeight = nCalcHeight-nNoneSpace;
    }
    else
    {
        nCalcHeight -= nLineSpace;
        nItemHeight = nCalcHeight / mnVisLines;
    }

    // Init VirDev
    maVirDev.SetSettings( GetSettings() );
    maVirDev.SetBackground( GetBackground() );
    maVirDev.SetOutputSizePixel( aWinSize, sal_True );

    // Bei zu kleinen Items machen wir nichts
    long nMinHeight = 2;
    if ( nStyle & WB_ITEMBORDER )
        nMinHeight = 4;
    if ( (nItemWidth <= 0) || (nItemHeight <= nMinHeight) || !nItemCount )
    {
        if ( nStyle & WB_NONEFIELD )
        {
            if ( mpNoneItem )
            {
                mpNoneItem->maRect.SetEmpty();
                mpNoneItem->maText = GetText();
            }
        }

        for ( size_t i = 0; i < nItemCount; i++ )
        {
            ValueSetItem* pItem = (*mpImpl->mpItemList)[ i ];
            pItem->maRect.SetEmpty();
        }

        if ( mpScrBar )
            mpScrBar->Hide();
    }
    else
    {
        // Frame-Style ermitteln
        if ( nStyle & WB_DOUBLEBORDER )
            mnFrameStyle = FRAME_DRAW_DOUBLEIN;
        else
            mnFrameStyle = FRAME_DRAW_IN;

        // Selektionsfarben und -breiten ermitteln
        // Gegebenenfalls die Farben anpassen, damit man die Selektion besser
        // erkennen kann
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        Color aHighColor( rStyleSettings.GetHighlightColor() );
        if ( ((aHighColor.GetRed() > 0x80) || (aHighColor.GetGreen() > 0x80) ||
              (aHighColor.GetBlue() > 0x80)) ||
             ((aHighColor.GetRed() == 0x80) && (aHighColor.GetGreen() == 0x80) &&
              (aHighColor.GetBlue() == 0x80)) )
            mbBlackSel = sal_True;
        else
            mbBlackSel = sal_False;

        // Wenn die Items groesser sind, dann die Selektion doppelt so breit
        // zeichnen
        if ( (nStyle & WB_DOUBLEBORDER) &&
             ((nItemWidth >= 25) && (nItemHeight >= 20)) )
            mbDoubleSel = sal_True;
        else
            mbDoubleSel = sal_False;

        // Calculate offsets
        long nStartX;
        long nStartY;
        if ( mbFullMode )
        {
            long nAllItemWidth = (nItemWidth*mnCols)+nColSpace;
            long nAllItemHeight = (nItemHeight*mnVisLines)+nNoneHeight+nLineSpace;
            nStartX = (aWinSize.Width()-nScrBarWidth-nAllItemWidth)/2;
            nStartY = (aWinSize.Height()-nAllItemHeight)/2;
        }
        else
        {
            nStartX = 0;
            nStartY = 0;
        }

        // Items berechnen und zeichnen
        maVirDev.SetLineColor();
        long x = nStartX;
        long y = nStartY;

        // NoSelection-Field erzeugen und anzeigen
        if ( nStyle & WB_NONEFIELD )
        {
            if ( !mpNoneItem )
                mpNoneItem = new ValueSetItem( *this );

            mpNoneItem->mnId            = 0;
            mpNoneItem->meType          = VALUESETITEM_NONE;
            mpNoneItem->maRect.Left()   = x;
            mpNoneItem->maRect.Top()    = y;
            mpNoneItem->maRect.Right()  = mpNoneItem->maRect.Left()+aWinSize.Width()-x-1;
            mpNoneItem->maRect.Bottom() = y+nNoneHeight-1;

            ImplFormatItem( mpNoneItem );

            y += nNoneHeight+nNoneSpace;
        }

        // draw items
        sal_uLong nFirstItem = mnFirstLine * mnCols;
        sal_uLong nLastItem = nFirstItem + (mnVisLines * mnCols);

        if ( !mbFullMode )
        {
            // If want also draw parts of items in the last line,
            // then we add one more line if parts of these line are
            // visible
            if ( y+(mnVisLines*(nItemHeight+nSpace)) < aWinSize.Height() )
                nLastItem += mnCols;
        }
        for ( size_t i = 0; i < nItemCount; i++ )
        {
            ValueSetItem*   pItem = (*mpImpl->mpItemList)[ i ];

            if ( (i >= nFirstItem) && (i < nLastItem) )
            {
                const sal_Bool bWasEmpty = pItem->maRect.IsEmpty();

                pItem->maRect.Left()    = x;
                pItem->maRect.Top()     = y;
                pItem->maRect.Right()   = pItem->maRect.Left()+nItemWidth-1;
                pItem->maRect.Bottom()  = pItem->maRect.Top()+nItemHeight-1;

                if( bWasEmpty && ImplHasAccessibleListeners() )
                {
                    ::com::sun::star::uno::Any aOldAny, aNewAny;

                    aNewAny <<= pItem->GetAccessible( mpImpl->mbIsTransientChildrenDisabled );
                    ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                ImplFormatItem( pItem );

                if ( !((i+1) % mnCols) )
                {
                    x = nStartX;
                    y += nItemHeight+nSpace;
                }
                else
                    x += nItemWidth+nSpace;
            }
            else
            {
                if( !pItem->maRect.IsEmpty() && ImplHasAccessibleListeners() )
                {
                    ::com::sun::star::uno::Any aOldAny, aNewAny;

                    aOldAny <<= pItem->GetAccessible( mpImpl->mbIsTransientChildrenDisabled );
                    ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                pItem->maRect.SetEmpty();
            }
        }

        // ScrollBar anordnen, Werte setzen und anzeigen
        if ( mpScrBar )
        {
            Point   aPos( aWinSize.Width()-nScrBarWidth+SCRBAR_OFFSET, 0 );
            Size    aSize( nScrBarWidth-SCRBAR_OFFSET, aWinSize.Height() );
            // If a none field is visible, then we center the scrollbar
            if ( nStyle & WB_NONEFIELD )
            {
                aPos.Y() = nStartY+nNoneHeight+1;
                aSize.Height() = ((nItemHeight+nSpace)*mnVisLines)-2-nSpace;
            }
            mpScrBar->SetPosSizePixel( aPos, aSize );
            mpScrBar->SetRangeMax( mnLines );
            mpScrBar->SetVisibleSize( mnVisLines );
            mpScrBar->SetThumbPos( (long)mnFirstLine );
            long nPageSize = mnVisLines;
            if ( nPageSize < 1 )
                nPageSize = 1;
            mpScrBar->SetPageSize( nPageSize );
            mpScrBar->Show();
        }
    }

    // Jetzt haben wir formatiert und warten auf das naechste
    mbFormat = sal_False;

    // ScrollBar loeschen
    if ( pDelScrBar )
        delete pDelScrBar;
}

// -----------------------------------------------------------------------

void ValueSet::ImplDrawItemText( const XubString& rText )
{
    if ( !(GetStyle() & WB_NAMEFIELD) )
        return;

    Size    aWinSize = GetOutputSizePixel();
    long    nTxtWidth = GetTextWidth( rText );
    long    nTxtOffset = mnTextOffset;

    // Rechteck loeschen und Text ausgeben
    if ( GetStyle() & WB_FLATVALUESET )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        SetLineColor();
        SetFillColor( rStyleSettings.GetFaceColor() );
        DrawRect( Rectangle( Point( 0, nTxtOffset ), Point( aWinSize.Width(), aWinSize.Height() ) ) );
        SetTextColor( rStyleSettings.GetButtonTextColor() );
    }
    else
    {
        nTxtOffset += NAME_LINE_HEIGHT+NAME_LINE_OFF_Y;
        Erase( Rectangle( Point( 0, nTxtOffset ), Point( aWinSize.Width(), aWinSize.Height() ) ) );
    }
    DrawText( Point( (aWinSize.Width()-nTxtWidth) / 2, nTxtOffset+(NAME_OFFSET/2) ), rText );
}

// -----------------------------------------------------------------------

void ValueSet::ImplDrawSelect()
{
    if ( !IsReallyVisible() )
        return;

    sal_Bool bFocus = HasFocus();
    sal_Bool bDrawSel;

    if ( (mbNoSelection && !mbHighlight) || (!mbDrawSelection && mbHighlight) )
        bDrawSel = sal_False;
    else
        bDrawSel = sal_True;

    if ( !bFocus &&
         ((mbNoSelection && !mbHighlight) || (!mbDrawSelection && mbHighlight)) )
    {
        XubString aEmptyStr;
        ImplDrawItemText( aEmptyStr );
        return;
    }

    sal_uInt16 nItemId = mnSelItemId;

    for( int stage = 0; stage < 2; stage++ )
    {
        if( stage == 1 )
        {
            if ( mbHighlight )
                nItemId = mnHighItemId;
            else
                break;
        }

        ValueSetItem* pItem;
        if ( nItemId )
            pItem = (*mpImpl->mpItemList)[ GetItemPos( nItemId ) ];
        else
        {
            if ( mpNoneItem )
                pItem = mpNoneItem;
            else
            {
                pItem = ImplGetFirstItem();
                if ( !bFocus || !pItem )
                    continue;
            }
        }

        if ( pItem->maRect.IsEmpty() )
            continue;

        // Selection malen
        const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
        Rectangle               aRect = pItem->maRect;
        Control::SetFillColor();

        Color aDoubleColor( rStyleSettings.GetHighlightColor() );
        Color aSingleColor( rStyleSettings.GetHighlightTextColor() );
        if( ! mbDoubleSel )
        {
            /*
            *  #99777# contrast enhancement for thin mode
            */
            const Wallpaper& rWall = GetDisplayBackground();
            if( ! rWall.IsBitmap() && ! rWall.IsGradient() )
            {
                const Color& rBack = rWall.GetColor();
                if( rBack.IsDark() && ! aDoubleColor.IsBright() )
                {
                    aDoubleColor = Color( COL_WHITE );
                    aSingleColor = Color( COL_BLACK );
                }
                else if( rBack.IsBright() && ! aDoubleColor.IsDark() )
                {
                    aDoubleColor = Color( COL_BLACK );
                    aSingleColor = Color( COL_WHITE );
                }
            }
        }

        // Selectionsausgabe festlegen
        WinBits nStyle = GetStyle();
        if ( nStyle & WB_MENUSTYLEVALUESET )
        {
            if ( bFocus )
                ShowFocus( aRect );

            if ( bDrawSel )
            {
                if ( mbBlackSel )
                    SetLineColor( Color( COL_BLACK ) );
                else
                    SetLineColor( aDoubleColor );
                DrawRect( aRect );
            }
        }
        else if ( nStyle & WB_RADIOSEL )
        {
            aRect.Left()    += 3;
            aRect.Top()     += 3;
            aRect.Right()   -= 3;
            aRect.Bottom()  -= 3;
            if ( nStyle & WB_DOUBLEBORDER )
            {
                aRect.Left()++;
                aRect.Top()++;
                aRect.Right()--;
                aRect.Bottom()--;
            }

            if ( bFocus )
                ShowFocus( aRect );

            aRect.Left()++;
            aRect.Top()++;
            aRect.Right()--;
            aRect.Bottom()--;

            if ( bDrawSel )
            {
                SetLineColor( aDoubleColor );
                aRect.Left()++;
                aRect.Top()++;
                aRect.Right()--;
                aRect.Bottom()--;
                DrawRect( aRect );
                aRect.Left()++;
                aRect.Top()++;
                aRect.Right()--;
                aRect.Bottom()--;
                DrawRect( aRect );
            }
        }
        else
        {
            if ( bDrawSel )
            {
                if ( mbBlackSel )
                    SetLineColor( Color( COL_BLACK ) );
                else
                    SetLineColor( aDoubleColor );
                DrawRect( aRect );
            }
            if ( mbDoubleSel )
            {
                aRect.Left()++;
                aRect.Top()++;
                aRect.Right()--;
                aRect.Bottom()--;
                if ( bDrawSel )
                    DrawRect( aRect );
            }
            aRect.Left()++;
            aRect.Top()++;
            aRect.Right()--;
            aRect.Bottom()--;
            Rectangle aRect2 = aRect;
            aRect.Left()++;
            aRect.Top()++;
            aRect.Right()--;
            aRect.Bottom()--;
            if ( bDrawSel )
                DrawRect( aRect );
            if ( mbDoubleSel )
            {
                aRect.Left()++;
                aRect.Top()++;
                aRect.Right()--;
                aRect.Bottom()--;
                if ( bDrawSel )
                    DrawRect( aRect );
            }

            if ( bDrawSel )
            {
                if ( mbBlackSel )
                    SetLineColor( Color( COL_WHITE ) );
                else
                    SetLineColor( aSingleColor );
            }
            else
                SetLineColor( Color( COL_LIGHTGRAY ) );
            DrawRect( aRect2 );

            if ( bFocus )
                ShowFocus( aRect2 );
        }

        ImplDrawItemText( pItem->maText );
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplHideSelect( sal_uInt16 nItemId )
{
    Rectangle aRect;

    sal_uInt16 nItemPos = GetItemPos( nItemId );
    if ( nItemPos != sal::static_int_cast<sal_uInt16>(LIST_ENTRY_NOTFOUND) )
        aRect = (*mpImpl->mpItemList)[ nItemPos ]->maRect;
    else
    {
        if ( mpNoneItem )
            aRect = mpNoneItem->maRect;
    }

    if ( !aRect.IsEmpty() )
    {
        HideFocus();
        Point aPos  = aRect.TopLeft();
        Size  aSize = aRect.GetSize();
        DrawOutDev( aPos, aSize, aPos, aSize, maVirDev );
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplHighlightItem( sal_uInt16 nItemId, sal_Bool bIsSelection )
{
    if ( mnHighItemId != nItemId )
    {
        // Alten merken, um vorherige Selektion zu entfernen
        sal_uInt16 nOldItem = mnHighItemId;
        mnHighItemId = nItemId;

        // Wenn keiner selektiert ist, dann Selektion nicht malen
        if ( !bIsSelection && mbNoSelection )
            mbDrawSelection = sal_False;

        // Neu ausgeben und alte Selection wegnehmen
        ImplHideSelect( nOldItem );
        ImplDrawSelect();
        mbDrawSelection = sal_True;
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplDraw()
{
    if ( mbFormat )
        Format();

    HideFocus();

    Point   aDefPos;
    Size    aSize = maVirDev.GetOutputSizePixel();

    if ( mpScrBar && mpScrBar->IsVisible() )
    {
        Point   aScrPos = mpScrBar->GetPosPixel();
        Size    aScrSize = mpScrBar->GetSizePixel();
        Point   aTempPos( 0, aScrPos.Y() );
        Size    aTempSize( aSize.Width(), aScrPos.Y() );

        DrawOutDev( aDefPos, aTempSize, aDefPos, aTempSize, maVirDev );
        aTempSize.Width()   = aScrPos.X()-1;
        aTempSize.Height()  = aScrSize.Height();
        DrawOutDev( aTempPos, aTempSize, aTempPos, aTempSize, maVirDev );
        aTempPos.Y()        = aScrPos.Y()+aScrSize.Height();
        aTempSize.Width()   = aSize.Width();
        aTempSize.Height()  = aSize.Height()-aTempPos.Y();
        DrawOutDev( aTempPos, aTempSize, aTempPos, aTempSize, maVirDev );
    }
    else
        DrawOutDev( aDefPos, aSize, aDefPos, aSize, maVirDev );

    // Trennlinie zum Namefield zeichnen
    if ( GetStyle() & WB_NAMEFIELD )
    {
        if ( !(GetStyle() & WB_FLATVALUESET) )
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            Size aWinSize = GetOutputSizePixel();
            Point aPos1( NAME_LINE_OFF_X, mnTextOffset+NAME_LINE_OFF_Y );
            Point aPos2( aWinSize.Width()-(NAME_LINE_OFF_X*2), mnTextOffset+NAME_LINE_OFF_Y );
            if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
            {
                SetLineColor( rStyleSettings.GetShadowColor() );
                DrawLine( aPos1, aPos2 );
                aPos1.Y()++;
                aPos2.Y()++;
                SetLineColor( rStyleSettings.GetLightColor() );
            }
            else
                SetLineColor( rStyleSettings.GetWindowTextColor() );
            DrawLine( aPos1, aPos2 );
        }
    }

    ImplDrawSelect();
}

// -----------------------------------------------------------------------

sal_Bool ValueSet::ImplScroll( const Point& rPos )
{
    Size aOutSize = GetOutputSizePixel();
    long nScrBarWidth;

    if ( mpScrBar )
        nScrBarWidth = mpScrBar->GetSizePixel().Width();
    else
        nScrBarWidth = 0;

    if ( !mbScroll || (rPos.X() < 0) || (rPos.X() > aOutSize.Width()-nScrBarWidth) )
        return sal_False;

    long             nScrollOffset;
    sal_uInt16           nOldLine = mnFirstLine;
    const Rectangle& rTopRect = (*mpImpl->mpItemList)[ mnFirstLine * mnCols ]->maRect;
    if ( rTopRect.GetHeight() <= 16 )
        nScrollOffset = VALUESET_SCROLL_OFFSET/2;
    else
        nScrollOffset = VALUESET_SCROLL_OFFSET;
    if ( (mnFirstLine > 0) && (rPos.Y() >= 0) )
    {
        long nTopPos = rTopRect.Top();
        if ( (rPos.Y() >= nTopPos) && (rPos.Y() <= nTopPos+nScrollOffset) )
            mnFirstLine--;
    }
    if ( (mnFirstLine == nOldLine) &&
         (mnFirstLine < (sal_uInt16)(mnLines-mnVisLines)) && (rPos.Y() < aOutSize.Height()) )
    {
        long nBottomPos = (*mpImpl->mpItemList)[ (mnFirstLine+mnVisLines-1)*mnCols ]->maRect.Bottom();
        if ( (rPos.Y() >= nBottomPos-nScrollOffset) && (rPos.Y() <= nBottomPos) )
            mnFirstLine++;
    }

    if ( mnFirstLine != nOldLine )
    {
        mbFormat = sal_True;
        ImplDraw();
        return sal_True;
    }
    else
        return sal_False;
}

// -----------------------------------------------------------------------

size_t ValueSet::ImplGetItem( const Point& rPos, sal_Bool bMove ) const
{
    if ( mpNoneItem )
    {
        if ( mpNoneItem->maRect.IsInside( rPos ) )
            return VALUESET_ITEM_NONEITEM;
    }

    Point     aDefPos;
    Rectangle aWinRect( aDefPos, maVirDev.GetOutputSizePixel() );

    size_t nItemCount = mpImpl->mpItemList->size();
    for ( size_t i = 0; i < nItemCount; i++ )
    {
        ValueSetItem* pItem = (*mpImpl->mpItemList)[ i ];
        if ( pItem->maRect.IsInside( rPos ) )
        {
            if ( aWinRect.IsInside( rPos ) )
                return i;
            else
                return VALUESET_ITEM_NOTFOUND;
        }
    }

    // Wenn Spacing gesetzt ist, wird der vorher selektierte
    // Eintrag zurueckgegeben, wenn die Maus noch nicht das Fenster
    // verlassen hat
    if ( bMove && mnSpacing && mnHighItemId )
    {
        if ( aWinRect.IsInside( rPos ) )
            return GetItemPos( mnHighItemId );
    }

    return VALUESET_ITEM_NOTFOUND;
}

// -----------------------------------------------------------------------

ValueSetItem* ValueSet::ImplGetItem( size_t nPos )
{
    if ( nPos == VALUESET_ITEM_NONEITEM )
        return mpNoneItem;
    else
        return ( nPos < mpImpl->mpItemList->size() ) ? (*mpImpl->mpItemList)[ nPos ] : NULL;
}

// -----------------------------------------------------------------------

ValueSetItem* ValueSet::ImplGetFirstItem()
{
    for ( size_t i = 0, nItemCount = mpImpl->mpItemList->size(); i < nItemCount; ++i ) {
        ValueSetItem* pItem = (*mpImpl->mpItemList)[ i ];
        if ( pItem->meType != VALUESETITEM_SPACE ) {
            return pItem;
        }
    }

    return NULL;
}

// -----------------------------------------------------------------------

sal_uInt16 ValueSet::ImplGetVisibleItemCount() const
{
    sal_uInt16 nRet = 0;

    for( size_t n = 0, nItemCount = mpImpl->mpItemList->size(); n < nItemCount; n++  )
    {
        ValueSetItem* pItem = (*mpImpl->mpItemList)[ n ];

        if( pItem->meType != VALUESETITEM_SPACE && !pItem->maRect.IsEmpty() )
            nRet++;
    }

    return nRet;
}

// -----------------------------------------------------------------------

ValueSetItem* ValueSet::ImplGetVisibleItem( sal_uInt16 nVisiblePos )
{
    ValueSetItem*   pRet = NULL;
    sal_uInt16          nFoundPos = 0;

    for( sal_Int32 n = 0, nItemCount = mpImpl->mpItemList->size(); ( n < nItemCount ) && !pRet; n++  )
    {
        ValueSetItem* pItem = (*mpImpl->mpItemList)[ n ];

        if( ( pItem->meType != VALUESETITEM_SPACE ) && !pItem->maRect.IsEmpty() && ( nVisiblePos == nFoundPos++ ) )
            pRet = pItem;
    }

    return pRet;
}

// -----------------------------------------------------------------------

void ValueSet::ImplFireAccessibleEvent( short nEventId, const ::com::sun::star::uno::Any& rOldValue, const ::com::sun::star::uno::Any& rNewValue )
{
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation( GetAccessible( sal_False ) );

    if( pAcc )
        pAcc->FireAccessibleEvent( nEventId, rOldValue, rNewValue );
}

// -----------------------------------------------------------------------

sal_Bool ValueSet::ImplHasAccessibleListeners()
{
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation( GetAccessible( sal_False ) );
    return( pAcc && pAcc->HasAccessibleListeners() );
}

// -----------------------------------------------------------------------

IMPL_LINK( ValueSet,ImplScrollHdl, ScrollBar*, pScrollBar )
{
    sal_uInt16 nNewFirstLine = (sal_uInt16)pScrollBar->GetThumbPos();
    if ( nNewFirstLine != mnFirstLine )
    {
        mnFirstLine = nNewFirstLine;
        mbFormat = sal_True;
        ImplDraw();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ValueSet,ImplTimerHdl, Timer*, EMPTYARG )
{
    ImplTracking( GetPointerPosPixel(), sal_True );
    return 0;
}

// -----------------------------------------------------------------------

void ValueSet::ImplTracking( const Point& rPos, sal_Bool bRepeat )
{
    if ( bRepeat || mbSelection )
    {
        if ( ImplScroll( rPos ) )
        {
            if ( mbSelection )
            {
                maTimer.SetTimeoutHdl( LINK( this, ValueSet, ImplTimerHdl ) );
                maTimer.SetTimeout( GetSettings().GetMouseSettings().GetScrollRepeat() );
                maTimer.Start();
            }
        }
    }

    ValueSetItem* pItem = ImplGetItem( ImplGetItem( rPos ) );
    if ( pItem && (pItem->meType != VALUESETITEM_SPACE) )
    {
        if( GetStyle() & WB_MENUSTYLEVALUESET )
            mbHighlight = sal_True;

        ImplHighlightItem( pItem->mnId );
    }
    else
    {
        if( GetStyle() & WB_MENUSTYLEVALUESET )
            mbHighlight = sal_True;

        ImplHighlightItem( mnSelItemId, sal_False );
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplEndTracking( const Point& rPos, sal_Bool bCancel )
{
    ValueSetItem* pItem;

    // Bei Abbruch, den alten Status wieder herstellen
    if ( bCancel )
        pItem = NULL;
    else
        pItem = ImplGetItem( ImplGetItem( rPos ) );

    if ( pItem && (pItem->meType != VALUESETITEM_SPACE) )
    {
        SelectItem( pItem->mnId );
        if ( !mbSelection && !(GetStyle() & WB_NOPOINTERFOCUS) )
            GrabFocus();
        mbHighlight = sal_False;
        mbSelection = sal_False;
        Select();
    }
    else
    {
        ImplHighlightItem( mnSelItemId, sal_False );
        mbHighlight = sal_False;
        mbSelection = sal_False;
    }
}

// -----------------------------------------------------------------------

void ValueSet::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        ValueSetItem* pItem = ImplGetItem( ImplGetItem( rMEvt.GetPosPixel() ) );
        if ( mbSelection )
        {
            mbHighlight = sal_True;
            if ( pItem && (pItem->meType != VALUESETITEM_SPACE) )
            {
                mnOldItemId  = mnSelItemId;
                mnHighItemId = mnSelItemId;
                ImplHighlightItem( pItem->mnId );
            }

            return;
        }
        else
        {
            if ( pItem && (pItem->meType != VALUESETITEM_SPACE) && !rMEvt.IsMod2() )
            {
                if ( rMEvt.GetClicks() == 1 )
                {
                    mnOldItemId  = mnSelItemId;
                    mbHighlight  = sal_True;
                    mnHighItemId = mnSelItemId;
                    ImplHighlightItem( pItem->mnId );
                    StartTracking( STARTTRACK_SCROLLREPEAT );
                }
                else if ( rMEvt.GetClicks() == 2 )
                    DoubleClick();

                return;
            }
        }
    }

    Control::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void ValueSet::MouseButtonUp( const MouseEvent& rMEvt )
{
    // Wegen SelectionMode
    if ( rMEvt.IsLeft() && mbSelection )
        ImplEndTracking( rMEvt.GetPosPixel(), sal_False );
    else
        Control::MouseButtonUp( rMEvt );
}

// -----------------------------------------------------------------------

void ValueSet::MouseMove( const MouseEvent& rMEvt )
{
    // Wegen SelectionMode
    if ( mbSelection || (GetStyle() & WB_MENUSTYLEVALUESET) )
        ImplTracking( rMEvt.GetPosPixel(), sal_False );
    Control::MouseMove( rMEvt );
}

// -----------------------------------------------------------------------

void ValueSet::Tracking( const TrackingEvent& rTEvt )
{
    Point aMousePos = rTEvt.GetMouseEvent().GetPosPixel();

    if ( rTEvt.IsTrackingEnded() )
        ImplEndTracking( aMousePos, rTEvt.IsTrackingCanceled() );
    else
        ImplTracking( aMousePos, rTEvt.IsTrackingRepeat() );
}

// -----------------------------------------------------------------------

namespace
{

size_t
lcl_gotoLastLine(size_t const nLastPos, size_t const nCols, size_t const nCurPos)
{
    size_t nItemPos = ((((nLastPos+1)/nCols)-1)*nCols)+(nCurPos%nCols);
    if ( nItemPos+nCols <= nLastPos )
        nItemPos = nItemPos + nCols;
    return nItemPos;
}

}

void ValueSet::KeyInput( const KeyEvent& rKEvt )
{
    size_t nLastItem = mpImpl->mpItemList->size();
    size_t nItemPos = VALUESET_ITEM_NOTFOUND;
    size_t nCurPos = VALUESET_ITEM_NONEITEM;
    size_t nCalcPos;

    if ( !nLastItem || !ImplGetFirstItem() )
    {
        Control::KeyInput( rKEvt );
        return;
    }
    else
        nLastItem--;

    if ( mnSelItemId )
        nCurPos = GetItemPos( mnSelItemId );
    nCalcPos = nCurPos;

    //switch off selection mode if key travelling is used
    sal_Bool bDefault = sal_False;
    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_HOME:
            if ( mpNoneItem )
                nItemPos = VALUESET_ITEM_NONEITEM;
            else
            {
                nItemPos = 0;
                while ( ImplGetItem( nItemPos )->meType == VALUESETITEM_SPACE )
                    nItemPos++;
            }
            break;

        case KEY_END:
            nItemPos = nLastItem;
            while ( ImplGetItem( nItemPos )->meType == VALUESETITEM_SPACE )
            {
                if ( nItemPos == 0 )
                    nItemPos = VALUESET_ITEM_NONEITEM;
                else
                    nItemPos--;
            }
            break;

        case KEY_LEFT:
        case KEY_RIGHT:
            if ( rKEvt.GetKeyCode().GetCode()==KEY_LEFT )
            {
                do
                {
                    if ( nCalcPos == VALUESET_ITEM_NONEITEM )
                        nItemPos = nLastItem;
                    else if ( !nCalcPos )
                    {
                        if ( mpNoneItem )
                            nItemPos = VALUESET_ITEM_NONEITEM;
                        else
                            nItemPos = nLastItem;
                    }
                    else
                        nItemPos = nCalcPos-1;
                    nCalcPos = nItemPos;
                }
                while ( ImplGetItem( nItemPos )->meType == VALUESETITEM_SPACE );
            }
            else
            {
                do
                {
                    if ( nCalcPos == VALUESET_ITEM_NONEITEM )
                        nItemPos = 0;
                    else if ( nCalcPos == nLastItem )
                    {
                        if ( mpNoneItem )
                            nItemPos = VALUESET_ITEM_NONEITEM;
                        else
                            nItemPos = 0;
                    }
                    else
                        nItemPos = nCalcPos+1;
                    nCalcPos = nItemPos;
                }
                while ( ImplGetItem( nItemPos )->meType == VALUESETITEM_SPACE );
            }
            break;

        case KEY_UP:
        case KEY_PAGEUP:
        {
            if( rKEvt.GetKeyCode().GetCode() != KEY_PAGEUP ||
                ( !rKEvt.GetKeyCode().IsShift() && !rKEvt.GetKeyCode().IsMod1() && !rKEvt.GetKeyCode().IsMod2() ) )
            {
                const size_t nLineCount = ( ( KEY_UP == rKEvt.GetKeyCode().GetCode() ) ? 1 : mnVisLines );
                do
                {
                    if ( nCalcPos == VALUESET_ITEM_NONEITEM )
                    {
                        if ( nLastItem+1 <= mnCols )
                            nItemPos = mnCurCol;
                        else
                            nItemPos = lcl_gotoLastLine(nLastItem, mnCols, mnCurCol);
                    }
                    else if ( nCalcPos >= mnCols ) // we can go up
                    {
                        if ( nCalcPos >= ( nLineCount * mnCols ) )
                            nItemPos = nCalcPos - ( nLineCount * mnCols );
                        else
                            // Go to the first line. This can only happen for KEY_PAGEUP
                            nItemPos = nCalcPos % mnCols;
                    }
                    else // wrap around
                    {
                        if ( mpNoneItem )
                        {
                            mnCurCol  = nCalcPos%mnCols;
                            nItemPos = VALUESET_ITEM_NONEITEM;
                        }
                        else
                        {
                            if ( nLastItem+1 <= mnCols )
                                nItemPos = nCalcPos;
                            else
                                nItemPos = lcl_gotoLastLine(nLastItem, mnCols, nCalcPos);
                        }
                    }
                    nCalcPos = nItemPos;
                }
                while ( ImplGetItem( nItemPos )->meType == VALUESETITEM_SPACE );
            }
            else
                Control::KeyInput( rKEvt );
        }
        break;

        case KEY_DOWN:
        case KEY_PAGEDOWN:
        {
            if( rKEvt.GetKeyCode().GetCode() != KEY_PAGEDOWN ||
                ( !rKEvt.GetKeyCode().IsShift() && !rKEvt.GetKeyCode().IsMod1() && !rKEvt.GetKeyCode().IsMod2() ) )
            {
                const long nLineCount = ( ( KEY_DOWN == rKEvt.GetKeyCode().GetCode() ) ? 1 : mnVisLines );
                do
                {
                    if ( nCalcPos == VALUESET_ITEM_NONEITEM )
                        nItemPos = mnCurCol;
                    else if ( nCalcPos + mnCols <= nLastItem ) // we can go down
                    {
                        if ( nCalcPos + ( nLineCount * mnCols ) <= nLastItem )
                            nItemPos = nCalcPos + ( nLineCount * mnCols );
                        else
                            // Go to the last line. This can only happen for KEY_PAGEDOWN
                            nItemPos = lcl_gotoLastLine(nLastItem, mnCols, nCalcPos);
                    }
                    else // wrap around
                    {
                        {
                            if ( mpNoneItem )
                            {
                                mnCurCol  = nCalcPos%mnCols;
                                nItemPos = VALUESET_ITEM_NONEITEM;
                            }
                            else
                                nItemPos = nCalcPos%mnCols;
                        }
                    }
                    nCalcPos = nItemPos;
                }
                while ( ImplGetItem( nItemPos )->meType == VALUESETITEM_SPACE );
            }
            else
                Control::KeyInput( rKEvt );

        }
        break;
        case KEY_RETURN:
            //enable default handling of KEY_RETURN in dialogs
            if(0 != (GetStyle()&WB_NO_DIRECTSELECT))
            {
                Select();
                break;
            }
            //no break;
        default:
            Control::KeyInput( rKEvt );
            bDefault = sal_True;
            break;
    }
    if(!bDefault)
        EndSelection();
    if ( nItemPos != VALUESET_ITEM_NOTFOUND )
    {
        sal_uInt16 nItemId;
        if ( nItemPos != VALUESET_ITEM_NONEITEM )
            nItemId = GetItemId( nItemPos );
        else
            nItemId = 0;

        if ( nItemId != mnSelItemId )
        {
            SelectItem( nItemId );
            //select only if WB_NO_DIRECTSELECT is not set
            if(0 == (GetStyle()&WB_NO_DIRECTSELECT))
                Select();
        }
    }
}

// -----------------------------------------------------------------------

void ValueSet::Command( const CommandEvent& rCEvt )
{
    if ( (rCEvt.GetCommand() == COMMAND_WHEEL) ||
         (rCEvt.GetCommand() == COMMAND_STARTAUTOSCROLL) ||
         (rCEvt.GetCommand() == COMMAND_AUTOSCROLL) )
    {
        if ( HandleScrollCommand( rCEvt, NULL, mpScrBar ) )
            return;
    }

    Control::Command( rCEvt );
}

// -----------------------------------------------------------------------

void ValueSet::Paint( const Rectangle& )
{
    if ( GetStyle() & WB_FLATVALUESET )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        SetLineColor();
        SetFillColor( rStyleSettings.GetFaceColor() );
        long nOffY = maVirDev.GetOutputSizePixel().Height();
        Size aWinSize = GetOutputSizePixel();
        DrawRect( Rectangle( Point( 0, nOffY ), Point( aWinSize.Width(), aWinSize.Height() ) ) );
    }

    ImplDraw();
}

// -----------------------------------------------------------------------

void ValueSet::GetFocus()
{
    OSL_TRACE ("value set getting focus");
    ImplDrawSelect();
    Control::GetFocus();

    // Tell the accessible object that we got the focus.
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation( GetAccessible( sal_False ) );
    if( pAcc )
        pAcc->GetFocus();
}

// -----------------------------------------------------------------------

void ValueSet::LoseFocus()
{
    OSL_TRACE ("value set losing focus");
    if ( mbNoSelection && mnSelItemId )
        ImplHideSelect( mnSelItemId );
    else
        HideFocus();
    Control::LoseFocus();

    // Tell the accessible object that we lost the focus.
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation( GetAccessible( sal_False ) );
    if( pAcc )
        pAcc->LoseFocus();
}

// -----------------------------------------------------------------------

void ValueSet::Resize()
{
    mbFormat = sal_True;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
    Control::Resize();
}

// -----------------------------------------------------------------------

void ValueSet::RequestHelp( const HelpEvent& rHEvt )
{
    if ( (rHEvt.GetMode() & (HELPMODE_QUICK | HELPMODE_BALLOON)) == HELPMODE_QUICK )
    {
        Point aPos = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        size_t nItemPos = ImplGetItem( aPos );
        if ( nItemPos != VALUESET_ITEM_NOTFOUND )
        {
            ValueSetItem* pItem = ImplGetItem( nItemPos );
            Rectangle aItemRect = pItem->maRect;
            Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
            aItemRect.Left()   = aPt.X();
            aItemRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aItemRect.BottomRight() );
            aItemRect.Right()  = aPt.X();
            aItemRect.Bottom() = aPt.Y();
            Help::ShowQuickHelp( this, aItemRect, GetItemText( pItem->mnId ) );
            return;
        }
    }

    Control::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------

void ValueSet::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
    {
        if ( mbFormat )
            Format();
    }
    else if ( nType == STATE_CHANGE_UPDATEMODE )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_TEXT )
    {
        if ( mpNoneItem && !mbFormat && IsReallyVisible() && IsUpdateMode() )
        {
            ImplFormatItem( mpNoneItem );
            Invalidate( mpNoneItem->maRect );
        }
    }
    else if ( (nType == STATE_CHANGE_ZOOM) ||
              (nType == STATE_CHANGE_CONTROLFONT) )
    {
        ImplInitSettings( sal_True, sal_False, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( sal_False, sal_True, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( sal_False, sal_False, sal_True );
        Invalidate();
    }
    else if ( (nType == STATE_CHANGE_STYLE) || (nType == STATE_CHANGE_ENABLE) )
    {
        mbFormat = sal_True;
        ImplInitSettings( sal_False, sal_False, sal_True );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        mbFormat = sal_True;
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::Select()
{
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void ValueSet::DoubleClick()
{
    maDoubleClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void ValueSet::UserDraw( const UserDrawEvent& )
{
}

// -----------------------------------------------------------------------

void ValueSet::InsertItem( sal_uInt16 nItemId, const Image& rImage, size_t nPos )
{
    DBG_ASSERT( nItemId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );

    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_IMAGE;
    pItem->maImage  = rImage;
    if ( nPos < mpImpl->mpItemList->size() ) {
        ValueItemList::iterator it = mpImpl->mpItemList->begin();
        ::std::advance( it, nPos );
        mpImpl->mpItemList->insert( it, pItem );
    } else {
        mpImpl->mpItemList->push_back( pItem );
    }

    mbFormat = sal_True;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::InsertItem( sal_uInt16 nItemId, const Color& rColor, size_t nPos )
{
    DBG_ASSERT( nItemId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );

    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_COLOR;
    pItem->maColor  = rColor;
    if ( nPos < mpImpl->mpItemList->size() ) {
        ValueItemList::iterator it = mpImpl->mpItemList->begin();
        ::std::advance( it, nPos );
        mpImpl->mpItemList->insert( it, pItem );
    } else {
        mpImpl->mpItemList->push_back( pItem );
    }

    mbFormat = sal_True;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::InsertItem( sal_uInt16 nItemId, const Image& rImage,
                           const XubString& rText, size_t nPos )
{
    DBG_ASSERT( nItemId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );

    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_IMAGE;
    pItem->maImage  = rImage;
    pItem->maText   = rText;
    if ( nPos < mpImpl->mpItemList->size() ) {
        ValueItemList::iterator it = mpImpl->mpItemList->begin();
        ::std::advance( it, nPos );
        mpImpl->mpItemList->insert( it, pItem );
    } else {
        mpImpl->mpItemList->push_back( pItem );
    }

    mbFormat = sal_True;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::InsertItem( sal_uInt16 nItemId, const Color& rColor,
                           const XubString& rText, size_t nPos )
{
    DBG_ASSERT( nItemId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );

    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_COLOR;
    pItem->maColor  = rColor;
    pItem->maText   = rText;
    if ( nPos < mpImpl->mpItemList->size() ) {
        ValueItemList::iterator it = mpImpl->mpItemList->begin();
        ::std::advance( it, nPos );
        mpImpl->mpItemList->insert( it, pItem );
    } else {
        mpImpl->mpItemList->push_back( pItem );
    }

    mbFormat = sal_True;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::InsertItem( sal_uInt16 nItemId, size_t nPos )
{
    DBG_ASSERT( nItemId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );

    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_USERDRAW;
    if ( nPos < mpImpl->mpItemList->size() ) {
        ValueItemList::iterator it = mpImpl->mpItemList->begin();
        ::std::advance( it, nPos );
        mpImpl->mpItemList->insert( it, pItem );
    } else {
        mpImpl->mpItemList->push_back( pItem );
    }

    mbFormat = sal_True;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::RemoveItem( sal_uInt16 nItemId )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    if ( nPos < mpImpl->mpItemList->size() ) {
        ValueItemList::iterator it = mpImpl->mpItemList->begin();
        ::std::advance( it, nPos );
        delete *it;
        mpImpl->mpItemList->erase( it );
    }

    // Variablen zuruecksetzen
    if ( (mnHighItemId == nItemId) || (mnSelItemId == nItemId) )
    {
        mnCurCol        = 0;
        mnOldItemId     = 0;
        mnHighItemId    = 0;
        mnSelItemId     = 0;
        mbNoSelection   = sal_True;
    }

    mbFormat = sal_True;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::Clear()
{
    ImplDeleteItems();

    // Variablen zuruecksetzen
    mnFirstLine     = 0;
    mnCurCol        = 0;
    mnOldItemId     = 0;
    mnHighItemId    = 0;
    mnSelItemId     = 0;
    mbNoSelection   = sal_True;

    mbFormat = sal_True;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

size_t ValueSet::GetItemCount() const
{
    return mpImpl->mpItemList->size();
}

// -----------------------------------------------------------------------

size_t ValueSet::GetItemPos( sal_uInt16 nItemId ) const
{
    for ( size_t i = 0, n = mpImpl->mpItemList->size(); i < n; ++i ) {
        ValueSetItem* pItem = (*mpImpl->mpItemList)[ i ];
        if ( pItem->mnId == nItemId ) {
            return i;
        }
    }
    return VALUESET_ITEM_NOTFOUND;
}

// -----------------------------------------------------------------------

sal_uInt16 ValueSet::GetItemId( size_t nPos ) const
{
    return ( nPos < mpImpl->mpItemList->size() ) ? (*mpImpl->mpItemList)[ nPos ]->mnId : 0 ;
}

// -----------------------------------------------------------------------

sal_uInt16 ValueSet::GetItemId( const Point& rPos ) const
{
    size_t nItemPos = ImplGetItem( rPos );
    if ( nItemPos != VALUESET_ITEM_NOTFOUND )
        return GetItemId( nItemPos );

    return 0;
}

// -----------------------------------------------------------------------

Rectangle ValueSet::GetItemRect( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return (*mpImpl->mpItemList)[ nPos ]->maRect;
    else
        return Rectangle();
}

// -----------------------------------------------------------------------

void ValueSet::EnableFullItemMode( sal_Bool bFullMode )
{
    mbFullMode = bFullMode;
}

// -----------------------------------------------------------------------

void ValueSet::SetColCount( sal_uInt16 nNewCols )
{
    if ( mnUserCols != nNewCols )
    {
        mnUserCols = nNewCols;
        mbFormat = sal_True;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::SetLineCount( sal_uInt16 nNewLines )
{
    if ( mnUserVisLines != nNewLines )
    {
        mnUserVisLines = nNewLines;
        mbFormat = sal_True;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::SetItemWidth( long nNewItemWidth )
{
    if ( mnUserItemWidth != nNewItemWidth )
    {
        mnUserItemWidth = nNewItemWidth;
        mbFormat = sal_True;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::SetItemHeight( long nNewItemHeight )
{
    if ( mnUserItemHeight != nNewItemHeight )
    {
        mnUserItemHeight = nNewItemHeight;
        mbFormat = sal_True;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::SelectItem( sal_uInt16 nItemId )
{
    size_t nItemPos = 0;

    if ( nItemId )
    {
        nItemPos = GetItemPos( nItemId );
        if ( nItemPos == VALUESET_ITEM_NOTFOUND )
            return;
        if ( (*mpImpl->mpItemList)[ nItemPos ]->meType == VALUESETITEM_SPACE )
            return;
    }

    if ( (mnSelItemId != nItemId) || mbNoSelection )
    {
        sal_uInt16 nOldItem = mnSelItemId ? mnSelItemId : 1;
        mnSelItemId = nItemId;
        mbNoSelection = sal_False;

        sal_Bool bNewOut;
        sal_Bool bNewLine;
        if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
            bNewOut = sal_True;
        else
            bNewOut = sal_False;
        bNewLine = sal_False;

        // Gegebenenfalls in den sichtbaren Bereich scrollen
        if ( mbScroll && nItemId )
        {
            sal_uInt16 nNewLine = (sal_uInt16)(nItemPos / mnCols);
            if ( nNewLine < mnFirstLine )
            {
                mnFirstLine = nNewLine;
                bNewLine = sal_True;
            }
            else if ( nNewLine > (sal_uInt16)(mnFirstLine+mnVisLines-1) )
            {
                mnFirstLine = (sal_uInt16)(nNewLine-mnVisLines+1);
                bNewLine = sal_True;
            }
        }

        if ( bNewOut )
        {
            if ( bNewLine )
            {
                // Falls sich der sichtbare Bereich geaendert hat,
                // alles neu ausgeben
                mbFormat = sal_True;
                ImplDraw();
            }
            else
            {
                // alte Selection wegnehmen und neue ausgeben
                ImplHideSelect( nOldItem );
                ImplDrawSelect();
            }
        }

        if( ImplHasAccessibleListeners() )
        {
            // focus event (deselect)
            if( nOldItem )
            {
                const size_t nPos = GetItemPos( nItemId );

                if( nPos != VALUESET_ITEM_NOTFOUND )
                {
                    ValueItemAcc* pItemAcc = ValueItemAcc::getImplementation(
                        (*mpImpl->mpItemList)[ nPos ]->GetAccessible( mpImpl->mbIsTransientChildrenDisabled ) );

                    if( pItemAcc )
                    {
                        ::com::sun::star::uno::Any aOldAny, aNewAny;
                        if( !mpImpl->mbIsTransientChildrenDisabled)
                        {
                            aOldAny <<= ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(
                                static_cast< ::cppu::OWeakObject* >( pItemAcc ));
                            ImplFireAccessibleEvent (::com::sun::star::accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
                        }
                        else
                        {
                            aOldAny <<= ::com::sun::star::accessibility::AccessibleStateType::FOCUSED;
                            pItemAcc->FireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );
                        }
                    }
                }
            }

            // focus event (select)
            const size_t nPos = GetItemPos( mnSelItemId );

            ValueSetItem* pItem;
            if( nPos != VALUESET_ITEM_NOTFOUND )
                pItem = (*mpImpl->mpItemList)[ nPos ];
            else
                pItem = mpNoneItem;

            ValueItemAcc* pItemAcc = NULL;
            if (pItem != NULL)
                pItemAcc = ValueItemAcc::getImplementation(pItem->GetAccessible( mpImpl->mbIsTransientChildrenDisabled ) );

            if( pItemAcc )
            {
                ::com::sun::star::uno::Any aOldAny, aNewAny;
                if( !mpImpl->mbIsTransientChildrenDisabled)
                {
                    aNewAny <<= ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(
                        static_cast< ::cppu::OWeakObject* >( pItemAcc ));
                    ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
                }
                else
                {
                    aNewAny <<= ::com::sun::star::accessibility::AccessibleStateType::FOCUSED;
                    pItemAcc->FireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );
                }
            }

            // selection event
            ::com::sun::star::uno::Any aOldAny, aNewAny;
            ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::SELECTION_CHANGED, aOldAny, aNewAny );
        }
        mpImpl->maHighlightHdl.Call(this);
    }
}

// -----------------------------------------------------------------------

void ValueSet::SetNoSelection()
{
    mbNoSelection   = sal_True;
    mbHighlight     = sal_False;
    mbSelection     = sal_False;

    if ( IsReallyVisible() && IsUpdateMode() )
        ImplDraw();
}

// -----------------------------------------------------------------------

void ValueSet::SetItemImage( sal_uInt16 nItemId, const Image& rImage )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    ValueSetItem* pItem = (*mpImpl->mpItemList)[ nPos ];
    pItem->meType  = VALUESETITEM_IMAGE;
    pItem->maImage = rImage;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        ImplFormatItem( pItem );
        Invalidate( pItem->maRect );
    }
    else
        mbFormat = sal_True;
}

// -----------------------------------------------------------------------

Image ValueSet::GetItemImage( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return (*mpImpl->mpItemList)[ nPos ]->maImage;
    else
        return Image();
}

// -----------------------------------------------------------------------

void ValueSet::SetItemColor( sal_uInt16 nItemId, const Color& rColor )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    ValueSetItem* pItem = (*mpImpl->mpItemList)[ nPos ];
    pItem->meType  = VALUESETITEM_COLOR;
    pItem->maColor = rColor;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        ImplFormatItem( pItem );
        Invalidate( pItem->maRect );
    }
    else
        mbFormat = sal_True;
}

// -----------------------------------------------------------------------

Color ValueSet::GetItemColor( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return (*mpImpl->mpItemList)[ nPos ]->maColor;
    else
        return Color();
}

// -----------------------------------------------------------------------

void ValueSet::SetItemData( sal_uInt16 nItemId, void* pData )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    ValueSetItem* pItem = (*mpImpl->mpItemList)[ nPos ];
    pItem->mpData = pData;

    if ( pItem->meType == VALUESETITEM_USERDRAW )
    {
        if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
        {
            ImplFormatItem( pItem );
            Invalidate( pItem->maRect );
        }
        else
            mbFormat = sal_True;
    }
}

// -----------------------------------------------------------------------

void* ValueSet::GetItemData( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return (*mpImpl->mpItemList)[ nPos ]->mpData;
    else
        return NULL;
}

// -----------------------------------------------------------------------

void ValueSet::SetItemText( sal_uInt16 nItemId, const XubString& rText )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;


    ValueSetItem* pItem = (*mpImpl->mpItemList)[ nPos ];

    // Remember old and new name for accessibility event.
    ::com::sun::star::uno::Any aOldName, aNewName;
    ::rtl::OUString sString (pItem->maText);
    aOldName <<= sString;
    sString = rText;
    aNewName <<= sString;

    pItem->maText = rText;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        sal_uInt16 nTempId = mnSelItemId;

        if ( mbHighlight )
            nTempId = mnHighItemId;

        if ( nTempId == nItemId )
            ImplDrawItemText( pItem->maText );
    }

    if (ImplHasAccessibleListeners())
    {
        ::com::sun::star::uno::Reference<
              ::com::sun::star::accessibility::XAccessible> xAccessible (
                  pItem->GetAccessible( mpImpl->mbIsTransientChildrenDisabled ) );
        static_cast<ValueItemAcc*>(xAccessible.get())->FireAccessibleEvent (
            ::com::sun::star::accessibility::AccessibleEventId::NAME_CHANGED,
            aOldName, aNewName);
    }
}

// -----------------------------------------------------------------------

XubString ValueSet::GetItemText( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return (*mpImpl->mpItemList)[ nPos ]->maText;
    else
        return XubString();
}

// -----------------------------------------------------------------------

void ValueSet::SetColor( const Color& rColor )
{
    maColor     = rColor;
    mbFormat    = sal_True;
    if ( IsReallyVisible() && IsUpdateMode() )
        ImplDraw();
}

// -----------------------------------------------------------------------

void ValueSet::SetExtraSpacing( sal_uInt16 nNewSpacing )
{
    if ( GetStyle() & WB_ITEMBORDER )
    {
        mnSpacing = nNewSpacing;

        mbFormat = sal_True;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::StartSelection()
{
    mnOldItemId     = mnSelItemId;
    mbHighlight     = sal_True;
    mbSelection     = sal_True;
    mnHighItemId    = mnSelItemId;
}

// -----------------------------------------------------------------------

void ValueSet::EndSelection()
{
    if ( mbHighlight )
    {
        if ( IsTracking() )
            EndTracking( ENDTRACK_CANCEL );

        ImplHighlightItem( mnSelItemId );
        mbHighlight = sal_False;
    }
    mbSelection = sal_False;
}

// -----------------------------------------------------------------------

sal_Bool ValueSet::StartDrag( const CommandEvent& rCEvt, Region& rRegion )
{
    if ( rCEvt.GetCommand() != COMMAND_STARTDRAG )
        return sal_False;

    // Gegebenenfalls eine vorhandene Aktion abbrechen
    EndSelection();

    // Testen, ob angeklickte Seite selektiert ist. Falls dies nicht
    // der Fall ist, setzen wir ihn als aktuellen Eintrag. Falls Drag and
    // Drop auch mal ueber Tastatur ausgeloest werden kann, testen wir
    // dies nur bei einer Mausaktion.
    sal_uInt16 nSelId;
    if ( rCEvt.IsMouseEvent() )
        nSelId = GetItemId( rCEvt.GetMousePosPixel() );
    else
        nSelId = mnSelItemId;

    // Falls kein Eintrag angeklickt wurde, starten wir kein Dragging
    if ( !nSelId )
        return sal_False;

    // Testen, ob Seite selektiertiert ist. Falls nicht, als aktuelle
    // Seite setzen und Select rufen.
    if ( nSelId != mnSelItemId )
    {
        SelectItem( nSelId );
        Update();
        Select();
    }

    Region aRegion;

    // Region zuweisen
    rRegion = aRegion;

    return sal_True;
}

// -----------------------------------------------------------------------

Size ValueSet::CalcWindowSizePixel( const Size& rItemSize, sal_uInt16 nDesireCols,
                                    sal_uInt16 nDesireLines )
{
    size_t nCalcCols = nDesireCols;
    size_t nCalcLines = nDesireLines;

    if ( !nCalcCols )
    {
        if ( mnUserCols )
            nCalcCols = mnUserCols;
        else
            nCalcCols = 1;
    }

    if ( !nCalcLines )
    {
        nCalcLines = mnVisLines;

        if ( mbFormat )
        {
            if ( mnUserVisLines )
                nCalcLines = mnUserVisLines;
            else
            {
                nCalcLines = mpImpl->mpItemList->size() / nCalcCols;
                if ( mpImpl->mpItemList->size() % nCalcCols )
                    nCalcLines++;
                else if ( !nCalcLines )
                    nCalcLines = 1;
            }
        }
    }

    Size        aSize( rItemSize.Width()*nCalcCols, rItemSize.Height()*nCalcLines );
    WinBits     nStyle = GetStyle();
    long        nTxtHeight = GetTextHeight();
    long        nSpace;
    long        n;

    if ( nStyle & WB_ITEMBORDER )
    {
        if ( nStyle & WB_DOUBLEBORDER )
            n = ITEM_OFFSET_DOUBLE;
        else
            n = ITEM_OFFSET;

        aSize.Width()  += n*nCalcCols;
        aSize.Height() += n*nCalcLines;
    }
    else
        n = 0;

    if ( mnSpacing )
    {
        nSpace = mnSpacing;
        aSize.Width()  += mnSpacing*(nCalcCols-1);
        aSize.Height() += mnSpacing*(nCalcLines-1);
    }
    else
        nSpace = 0;

    if ( nStyle & WB_NAMEFIELD )
    {
        aSize.Height() += nTxtHeight + NAME_OFFSET;
        if ( !(nStyle & WB_FLATVALUESET) )
            aSize.Height() += NAME_LINE_HEIGHT+NAME_LINE_OFF_Y;
    }

    if ( nStyle & WB_NONEFIELD )
    {
        aSize.Height() += nTxtHeight + n + nSpace;
        if ( nStyle & WB_RADIOSEL )
            aSize.Height() += 8;
    }

    // Evt. ScrollBar-Breite aufaddieren
    aSize.Width() += GetScrollWidth();

    return aSize;
}

// -----------------------------------------------------------------------

Size ValueSet::CalcItemSizePixel( const Size& rItemSize, sal_Bool bOut ) const
{
    Size aSize = rItemSize;

    WinBits nStyle = GetStyle();
    if ( nStyle & WB_ITEMBORDER )
    {
        long n;

        if ( nStyle & WB_DOUBLEBORDER )
            n = ITEM_OFFSET_DOUBLE;
        else
            n = ITEM_OFFSET;

        if ( bOut )
        {
            aSize.Width()  += n;
            aSize.Height() += n;
        }
        else
        {
            aSize.Width()  -= n;
            aSize.Height() -= n;
        }
    }

    return aSize;
}

// -----------------------------------------------------------------------

long ValueSet::GetScrollWidth() const
{
    if ( GetStyle() & WB_VSCROLL )
    {
        ((ValueSet*)this)->ImplInitScrollBar();
        return mpScrBar->GetSizePixel().Width()+SCRBAR_OFFSET;
    }
    else
        return 0;
}

// -----------------------------------------------------------------------

void ValueSet::SetHighlightHdl( const Link& rLink )
{
    mpImpl->maHighlightHdl = rLink;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
