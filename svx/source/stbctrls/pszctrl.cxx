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

#include <limits.h>
#include <tools/shl.hxx>
#include <vcl/status.hxx>
#include <vcl/menu.hxx>
#include <vcl/image.hxx>
#include <svl/stritem.hxx>
#include <svl/ptitem.hxx>
#include <svl/itempool.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <svl/intitem.hxx>
#define _SVX_PSZCTRL_CXX

#include "svx/pszctrl.hxx"

#define PAINT_OFFSET    5

#include <editeng/sizeitem.hxx>
#include <svx/dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include "stbctrls.h"
#include "sfx2/module.hxx"

#include <svx/dialogs.hrc>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>

// -----------------------------------------------------------------------

/*  [Description]

    Function used to create a text representation of
    a metric value

    nVal is the metric value in the unit eUnit.

    [cross reference]

    <SvxPosSizeStatusBarControl::Paint(const UserDrawEvent&)>
*/

String SvxPosSizeStatusBarControl::GetMetricStr_Impl( long nVal )
{
    // deliver and set the Metric of the application
    FieldUnit eOutUnit = SfxModule::GetModuleFieldUnit( getFrameInterface() );
    FieldUnit eInUnit = FUNIT_100TH_MM;

    String sMetric;
    const sal_Unicode cSep = Application::GetSettings().GetLocaleDataWrapper().getNumDecimalSep().GetChar(0);
    sal_Int64 nConvVal = MetricField::ConvertValue( nVal * 100, 0L, 0, eInUnit, eOutUnit );

    if ( nConvVal < 0 && ( nConvVal / 100 == 0 ) )
        sMetric += '-';
    sMetric += String::CreateFromInt64( nConvVal / 100 );

    if( FUNIT_NONE != eOutUnit )
    {
        sMetric += cSep;
        sal_Int64 nFract = nConvVal % 100;

        if ( nFract < 0 )
            nFract *= -1;
        if ( nFract < 10 )
            sMetric += '0';
        sMetric += String::CreateFromInt64( nFract );
    }

    return sMetric;
}

// -----------------------------------------------------------------------

SFX_IMPL_STATUSBAR_CONTROL(SvxPosSizeStatusBarControl, SvxSizeItem);

// class FunctionPopup_Impl ----------------------------------------------

class FunctionPopup_Impl : public PopupMenu
{
public:
    FunctionPopup_Impl( sal_uInt16 nCheck );

    sal_uInt16          GetSelected() const { return nSelected; }

private:
    sal_uInt16          nSelected;

    virtual void    Select();
};

// -----------------------------------------------------------------------

FunctionPopup_Impl::FunctionPopup_Impl( sal_uInt16 nCheck ) :
    PopupMenu( ResId( RID_SVXMNU_PSZ_FUNC, DIALOG_MGR() ) ),
    nSelected( 0 )
{
    if (nCheck)
        CheckItem( nCheck );
}

// -----------------------------------------------------------------------

void FunctionPopup_Impl::Select()
{
    nSelected = GetCurItemId();
}

// struct SvxPosSizeStatusBarControl_Impl --------------------------------

struct SvxPosSizeStatusBarControl_Impl

/*  [Description]

    This implementation-structure of the class SvxPosSizeStatusBarControl
    is done for the un-linking of the changes of the exported interface such as
    the toning down of symbols that are visible externaly.

    One instance exists for each SvxPosSizeStatusBarControl-instance
    during it's life time
*/

{
    Point   aPos;       // valid when a position is shown
    Size    aSize;      // valid when a size is shown
    String  aStr;       // valid when a text is shown
    sal_Bool    bPos;       // show position ?
    sal_Bool    bSize;      // set size ?
    sal_Bool    bTable;     // set table index ?
    sal_Bool    bHasMenu;   // set StarCalc popup menu ?
    sal_uInt16  nFunction;  // the selected StarCalc function
    Image   aPosImage;  // Image to show the position
    Image   aSizeImage; // Image to show the size
};

// class SvxPosSizeStatusBarControl ------------------------------------------

/*  [Description]

    Ctor():
    Create an instance of the implementation class,
    load the images for the position and size
*/

#define STR_POSITION ".uno:Position"
#define STR_TABLECELL ".uno:StateTableCell"
#define STR_FUNC ".uno:StatusBarFunc"

SvxPosSizeStatusBarControl::SvxPosSizeStatusBarControl( sal_uInt16 _nSlotId,
                                                        sal_uInt16 _nId,
                                                        StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    pImp( new SvxPosSizeStatusBarControl_Impl )
{
    pImp->bPos = sal_False;
    pImp->bSize = sal_False;
    pImp->bTable = sal_False;
    pImp->bHasMenu = sal_False;
    pImp->nFunction = 0;
    pImp->aPosImage = Image( ResId( RID_SVXBMP_POSITION, DIALOG_MGR() ) );
    pImp->aSizeImage = Image( ResId( RID_SVXBMP_SIZE, DIALOG_MGR() ) );

    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( STR_POSITION )));         // SID_ATTR_POSITION
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( STR_TABLECELL )));   // SID_TABLE_CELL
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( STR_FUNC )));    // SID_PSZ_FUNCTION
}

// -----------------------------------------------------------------------

/*  [Description]

    Dtor():
    remove the pointer to the implementation class, so that the timer is stopped

*/

SvxPosSizeStatusBarControl::~SvxPosSizeStatusBarControl()
{
    delete pImp;
}

// -----------------------------------------------------------------------

/*  [Description]

    SID_PSZ_FUNCTION activates the popup menu for Calc:

    Status overview
    Depending on the type of the item, a special setting is enabled, the others disabled.

                NULL/Void   SfxPointItem    SvxSizeItem     SfxStringItem
    ------------------------------------------------------------------------
    Position    sal_False                                       FALSE
    Size        FALSE                       TRUE            FALSE
    Text        sal_False                       sal_False           TRUE

*/

void SvxPosSizeStatusBarControl::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                               const SfxPoolItem* pState )
{
    // Because the combi-controller, always sets the curent Id as HelpId
    // first clean the cached HelpText
    GetStatusBar().SetHelpText( GetId(), String() );

    switch ( nSID )
    {
        case SID_ATTR_POSITION : GetStatusBar().SetHelpId( GetId(), STR_POSITION ); break;
        case SID_TABLE_CELL: GetStatusBar().SetHelpId( GetId(), STR_TABLECELL ); break;
        case SID_PSZ_FUNCTION: GetStatusBar().SetHelpId( GetId(), STR_FUNC ); break;
        default: break;
    }

    if ( nSID == SID_PSZ_FUNCTION )
    {
        if ( eState == SFX_ITEM_AVAILABLE )
        {
            pImp->bHasMenu = sal_True;
            if ( pState && pState->ISA(SfxUInt16Item) )
                pImp->nFunction = ((const SfxUInt16Item*)pState)->GetValue();
        }
        else
            pImp->bHasMenu = sal_False;
    }
    else if ( SFX_ITEM_AVAILABLE != eState )
    {
        // don't switch to empty display before an empty state was
        // notified for all display types

        if ( nSID == SID_TABLE_CELL )
            pImp->bTable = sal_False;
        else if ( nSID == SID_ATTR_POSITION )
            pImp->bPos = sal_False;
        else if ( nSID == GetSlotId() )     // controller is registered for SID_ATTR_SIZE
            pImp->bSize = sal_False;
        else
        {
            DBG_ERRORFILE("unknown slot id");
        }
    }
    else if ( pState->ISA( SfxPointItem ) )
    {
        // show position
        pImp->aPos = ( (SfxPointItem*)pState )->GetValue();
        pImp->bPos = sal_True;
        pImp->bTable = sal_False;
    }
    else if ( pState->ISA( SvxSizeItem ) )
    {
        // show size
        pImp->aSize = ( (SvxSizeItem*)pState )->GetSize();
        pImp->bSize = sal_True;
        pImp->bTable = sal_False;
    }
    else if ( pState->ISA( SfxStringItem ) )
    {
        // show string (table cel or different)
        pImp->aStr = ( (SfxStringItem*)pState )->GetValue();
        pImp->bTable = sal_True;
        pImp->bPos = sal_False;
        pImp->bSize = sal_False;
    }
    else
    {
        DBG_ERRORFILE( "invalid item type" );
        pImp->bPos = sal_False;
        pImp->bSize = sal_False;
        pImp->bTable = sal_False;
    }

    if ( GetStatusBar().AreItemsVisible() )
        GetStatusBar().SetItemData( GetId(), 0 );

    //  set only strings as text at the statusBar, so that the Help-Tips
    //  can work with the text, when it is too long for the statusBar
    String aText;
    if ( pImp->bTable )
        aText = pImp->aStr;
    GetStatusBar().SetItemText( GetId(), aText );
}

// -----------------------------------------------------------------------

/*  [Description]

    execute popup menu, when the status enables this
*/

void SvxPosSizeStatusBarControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU && pImp->bHasMenu )
    {
        sal_uInt16 nSelect = pImp->nFunction;
        if (!nSelect)
            nSelect = PSZ_FUNC_NONE;
        FunctionPopup_Impl aMenu( nSelect );
        if ( aMenu.Execute( &GetStatusBar(), rCEvt.GetMousePosPixel() ) )
        {
            nSelect = aMenu.GetSelected();
            if (nSelect)
            {
                if (nSelect == PSZ_FUNC_NONE)
                    nSelect = 0;

                ::com::sun::star::uno::Any a;
                SfxUInt16Item aItem( SID_PSZ_FUNCTION, nSelect );

                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
                aArgs[0].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StatusBarFunc" ));
                aItem.QueryValue( a );
                aArgs[0].Value = a;

                execute( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:StatusBarFunc" )), aArgs );
//              GetBindings().GetDispatcher()->Execute( SID_PSZ_FUNCTION, SFX_CALLMODE_RECORD, &aItem, 0L );
            }
        }
    }
    else
        SfxStatusBarControl::Command( rCEvt );
}

// -----------------------------------------------------------------------

/*  [Description]

    Depending on the type to be shown, the value us shown. First the
    rectangle is repainted (removed).
*/

void SvxPosSizeStatusBarControl::Paint( const UserDrawEvent& rUsrEvt )
{
    OutputDevice* pDev = rUsrEvt.GetDevice();
    DBG_ASSERT( pDev, "no OutputDevice on UserDrawEvent" );
    const Rectangle& rRect = rUsrEvt.GetRect();
    StatusBar& rBar = GetStatusBar();
    Point aItemPos = rBar.GetItemTextPos( GetId() );
    Color aOldLineColor = pDev->GetLineColor();
    Color aOldFillColor = pDev->GetFillColor();
    pDev->SetLineColor();
    pDev->SetFillColor( pDev->GetBackground().GetColor() );

    if ( pImp->bPos || pImp->bSize )
    {
        // count the position for showing the size
        long nSizePosX =
            rRect.Left() + rRect.GetWidth() / 2 + PAINT_OFFSET;
        // draw position
        Point aPnt = rRect.TopLeft();
        aPnt.Y() = aItemPos.Y();
        aPnt.X() += PAINT_OFFSET;
        pDev->DrawImage( aPnt, pImp->aPosImage );
        aPnt.X() += pImp->aPosImage.GetSizePixel().Width();
        aPnt.X() += PAINT_OFFSET;
        String aStr = GetMetricStr_Impl( pImp->aPos.X());
        aStr.AppendAscii(" / ");
        aStr += GetMetricStr_Impl( pImp->aPos.Y());
        pDev->DrawRect(
            Rectangle( aPnt, Point( nSizePosX, rRect.Bottom() ) ) );
        pDev->DrawText( aPnt, aStr );

        // draw the size, when available
        aPnt.X() = nSizePosX;

        if ( pImp->bSize )
        {
            pDev->DrawImage( aPnt, pImp->aSizeImage );
            aPnt.X() += pImp->aSizeImage.GetSizePixel().Width();
            Point aDrwPnt = aPnt;
            aPnt.X() += PAINT_OFFSET;
            aStr = GetMetricStr_Impl( pImp->aSize.Width() );
            aStr.AppendAscii(" x ");
            aStr += GetMetricStr_Impl( pImp->aSize.Height() );
            pDev->DrawRect( Rectangle( aDrwPnt, rRect.BottomRight() ) );
            pDev->DrawText( aPnt, aStr );
        }
        else
            pDev->DrawRect( Rectangle( aPnt, rRect.BottomRight() ) );
    }
    else if ( pImp->bTable )
    {
        pDev->DrawRect( rRect );
        pDev->DrawText( Point(
            rRect.Left() + rRect.GetWidth() / 2 - pDev->GetTextWidth( pImp->aStr ) / 2,
            aItemPos.Y() ), pImp->aStr );
    }
    else
    {
        // Empty display if neither size nor table position are available.
        // Date/Time are no longer used (#65302#).
        pDev->DrawRect( rRect );
    }

    pDev->SetLineColor( aOldLineColor );
    pDev->SetFillColor( aOldFillColor );
}

// -----------------------------------------------------------------------

sal_uIntPtr SvxPosSizeStatusBarControl::GetDefItemWidth(const StatusBar& rStb)
{
    Image aTmpPosImage( ResId( RID_SVXBMP_POSITION, DIALOG_MGR() ) );
    Image aTmpSizeImage( ResId( RID_SVXBMP_SIZE, DIALOG_MGR() ) );

    sal_uIntPtr nWidth=PAINT_OFFSET+aTmpPosImage.GetSizePixel().Width();
    nWidth+=PAINT_OFFSET+aTmpSizeImage.GetSizePixel().Width();
    nWidth+=2*(PAINT_OFFSET+rStb.GetTextWidth(String::CreateFromAscii("XXXX,XX / XXXX,XX")));

    return nWidth;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
