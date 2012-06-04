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


#undef SC_DLLIMPLEMENTATION

#include <vcl/msgbox.hxx>
#include <i18npool/mslangid.hxx>
#include <svtools/collatorres.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>

#include "scitems.hxx"
#include "uiitems.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "global.hxx"
#include "globalnames.hxx"
#include "dbdata.hxx"
#include "userlist.hxx"
#include "rangeutl.hxx"
#include "scresid.hxx"
#include "sc.hrc"       // -> Slot IDs
#include "globstr.hrc"

#include "sortdlg.hxx"
#include "sortdlg.hrc"

#define _TPSORT_CXX
#include "tpsort.hxx"
#undef _TPSORT_CXX

using namespace com::sun::star;

// STATIC DATA -----------------------------------------------------------

static sal_uInt16 pSortRanges[] =
{
    SID_SORT,
    SID_SORT,
    0
};

// -----------------------------------------------------------------------

/*
 * Da sich Einstellungen auf der zweiten TabPage (Optionen) auf
 * die erste TabPage auswirken, muss es die Moeglichkeit geben,
 * dies der jeweils anderen Seite mitzuteilen.
 *
 * Im Moment wird dieses Problem ueber zwei Datenmember des TabDialoges
 * geloest. Wird eine Seite Aktiviert/Deaktiviert, so gleicht sie diese
 * Datenmember mit dem eigenen Zustand ab (->Activate()/Deactivate()).
 *
 * Die Klasse SfxTabPage bietet mittlerweile ein Verfahren an:
 *
 * virtual sal_Bool HasExchangeSupport() const; -> return sal_True;
 * virtual void ActivatePage(const SfxItemSet &);
 * virtual int  DeactivatePage(SfxItemSet * = 0);
 *
 * muss noch geaendert werden!
 */

//========================================================================
//========================================================================
// Sortierkriterien-Tabpage:

ScTabPageSortFields::ScTabPageSortFields( Window*           pParent,
                                          const SfxItemSet& rArgSet )

    :   SfxTabPage      ( pParent,
                          ScResId( RID_SCPAGE_SORT_FIELDS ),
                          rArgSet ),
        //
        aFlSort1        ( this, ScResId( FL_SORT1  ) ),
        aLbSort1        ( this, ScResId( LB_SORT1  ) ),
        aBtnUp1         ( this, ScResId( BTN_UP1   ) ),
        aBtnDown1       ( this, ScResId( BTN_DOWN1 ) ),
        //
        aFlSort2        ( this, ScResId( FL_SORT2  ) ),
        aLbSort2        ( this, ScResId( LB_SORT2  ) ),
        aBtnUp2         ( this, ScResId( BTN_UP2   ) ),
        aBtnDown2       ( this, ScResId( BTN_DOWN2 ) ),
        //
        aFlSort3        ( this, ScResId( FL_SORT3  ) ),
        aLbSort3        ( this, ScResId( LB_SORT3  ) ),
        aBtnUp3         ( this, ScResId( BTN_UP3   ) ),
        aBtnDown3       ( this, ScResId( BTN_DOWN3 ) ),

        aStrUndefined   ( ScResId( SCSTR_UNDEFINED ) ),
        aStrColumn      ( ScResId( SCSTR_COLUMN ) ),
        aStrRow         ( ScResId( SCSTR_ROW ) ),
        //
        nWhichSort      ( rArgSet.GetPool()->GetWhich( SID_SORT ) ),
        pDlg            ( (ScSortDlg*)(GetParent()->GetParent()) ),
        pViewData       ( NULL ),
        rSortData       ( ((const ScSortItem&)
                           rArgSet.Get( nWhichSort )).
                                GetSortData() ),
        nFieldCount     ( 0 ),
        bHasHeader      ( false ),
        bSortByRows     ( false )
{
    Init();
    FreeResource();
    SetExchangeSupport();
}

// -----------------------------------------------------------------------

ScTabPageSortFields::~ScTabPageSortFields()
{
}

// -----------------------------------------------------------------------

void ScTabPageSortFields::Init()
{
    const ScSortItem& rSortItem = (const ScSortItem&)
                                  GetItemSet().Get( nWhichSort );

    pViewData = rSortItem.GetViewData();

    OSL_ENSURE( pViewData, "ViewData not found!" );

    nFieldArr[0] = 0;
    nFirstCol = 0;
    nFirstRow = 0;

    aLbSort1.SetSelectHdl( LINK( this, ScTabPageSortFields, SelectHdl ) );
    aLbSort2.SetSelectHdl( LINK( this, ScTabPageSortFields, SelectHdl ) );
    aLbSort3.SetSelectHdl( LINK( this, ScTabPageSortFields, SelectHdl ) );
    aLbSort1.Clear();
    aLbSort2.Clear();
    aLbSort3.Clear();

    aSortLbArr[0]       = &aLbSort1;
    aSortLbArr[1]       = &aLbSort2;
    aSortLbArr[2]       = &aLbSort3;
    aDirBtnArr[0][0]    = &aBtnUp1;
    aDirBtnArr[0][1]    = &aBtnDown1;
    aDirBtnArr[1][0]    = &aBtnUp2;
    aDirBtnArr[1][1]    = &aBtnDown2;
    aDirBtnArr[2][0]    = &aBtnUp3;
    aDirBtnArr[2][1]    = &aBtnDown3;
    aFlArr[0]           = &aFlSort1;
    aFlArr[1]           = &aFlSort2;
    aFlArr[2]           = &aFlSort3;
}

//------------------------------------------------------------------------

sal_uInt16* ScTabPageSortFields::GetRanges()
{
    return pSortRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* ScTabPageSortFields::Create( Window*    pParent,
                                         const SfxItemSet&  rArgSet )
{
    return ( new ScTabPageSortFields( pParent, rArgSet ) );
}

// -----------------------------------------------------------------------

void ScTabPageSortFields::Reset( const SfxItemSet& /* rArgSet */ )
{
    bSortByRows = rSortData.bByRow;
    bHasHeader  = rSortData.bHasHeader;

    if ( aLbSort1.GetEntryCount() == 0 )
        FillFieldLists();

    // Selektieren der ListBoxen:

    if ( rSortData.bDoSort[0] )
    {
        for ( sal_uInt16 i=0; i<3; i++ )
        {
            if ( rSortData.bDoSort[i] )
            {
                aSortLbArr[i]->SelectEntryPos(
                     GetFieldSelPos( rSortData.nField[i] ) );

                (rSortData.bAscending[i])
                    ? aDirBtnArr[i][0]->Check()     // Up
                    : aDirBtnArr[i][1]->Check();    // Down
            }
            else
            {
                aSortLbArr[i]->SelectEntryPos( 0 ); // "keiner" selektieren
                aDirBtnArr[i][0]->Check();          // Up
            }
        }

        EnableField( 1 );
        EnableField( 2 );
        EnableField( 3 );
        if ( aLbSort1.GetSelectEntryPos() == 0 )
            DisableField( 2 );
        if ( aLbSort2.GetSelectEntryPos() == 0 )
            DisableField( 3 );
    }
    else
    {
        SCCOL  nCol = pViewData->GetCurX();

        if( nCol < rSortData.nCol1 )
            nCol = rSortData.nCol1;
        else if( nCol > rSortData.nCol2 )
            nCol = rSortData.nCol2;

        sal_uInt16  nSort1Pos = nCol - rSortData.nCol1+1;
        aLbSort1.SelectEntryPos( nSort1Pos );
        aLbSort2.SelectEntryPos( 0 );
        aLbSort3.SelectEntryPos( 0 );
        aBtnUp1.Check();
        aBtnUp2.Check();
        aBtnUp3.Check();
        EnableField ( 1 );
        EnableField ( 2 );
        DisableField( 3 );
    }

    if ( pDlg )
    {
        pDlg->SetByRows ( bSortByRows );
        pDlg->SetHeaders( bHasHeader );
    }
}

// -----------------------------------------------------------------------

sal_Bool ScTabPageSortFields::FillItemSet( SfxItemSet& rArgSet )
{
    ScSortParam theSortData = rSortData;
    if (pDlg)
    {
        const SfxItemSet* pExample = pDlg->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExample && pExample->GetItemState( nWhichSort, sal_True, &pItem ) == SFX_ITEM_SET )
            theSortData = ((const ScSortItem*)pItem)->GetSortData();
    }

    sal_uInt16  nSort1Pos = aLbSort1.GetSelectEntryPos();
    sal_uInt16  nSort2Pos = aLbSort2.GetSelectEntryPos();
    sal_uInt16  nSort3Pos = aLbSort3.GetSelectEntryPos();

    OSL_ENSURE(    (nSort1Pos <= SC_MAXFIELDS)
                && (nSort2Pos <= SC_MAXFIELDS)
                && (nSort3Pos <= SC_MAXFIELDS),
                "Array-Range Fehler!" );

    if ( nSort1Pos == LISTBOX_ENTRY_NOTFOUND ) nSort1Pos = 0;
    if ( nSort2Pos == LISTBOX_ENTRY_NOTFOUND ) nSort2Pos = 0;
    if ( nSort3Pos == LISTBOX_ENTRY_NOTFOUND ) nSort3Pos = 0;

    if ( nSort1Pos > 0 )
    {
        theSortData.bDoSort[0] = (nSort1Pos > 0);
        theSortData.bDoSort[1] = (nSort2Pos > 0);
        theSortData.bDoSort[2] = (nSort3Pos > 0);

        // wenn auf Optionen-Seite "OK" gewaehlt wurde und
        // dabei die Sortierrichtung umgestellt wurde, so
        // wird das erste Feld der jeweiligen Richtung als
        // Sortierkriterium gewaehlt (steht in nFieldArr[0]):
        if ( bSortByRows != pDlg->GetByRows() )
        {
            theSortData.nField[0] =
            theSortData.nField[1] =
            theSortData.nField[2] = ( bSortByRows ?
                    static_cast<SCCOLROW>(nFirstRow) :
                    static_cast<SCCOLROW>(nFirstCol) );
        }
        else
        {
            theSortData.nField[0] = nFieldArr[nSort1Pos];
            theSortData.nField[1] = nFieldArr[nSort2Pos];
            theSortData.nField[2] = nFieldArr[nSort3Pos];
        }

        theSortData.bAscending[0] = aBtnUp1.IsChecked();
        theSortData.bAscending[1] = aBtnUp2.IsChecked();
        theSortData.bAscending[2] = aBtnUp3.IsChecked();
        //  bHasHeader ist in ScTabPageSortOptions::FillItemSet, wo es hingehoert
    }
    else
    {
        theSortData.bDoSort[0] =
        theSortData.bDoSort[1] =
        theSortData.bDoSort[2] = false;
    }

    rArgSet.Put( ScSortItem( SCITEM_SORTDATA, NULL, &theSortData ) );

    return sal_True;
}

// -----------------------------------------------------------------------

// fuer Datenaustausch ohne Dialog-Umweg: (! noch zu tun !)

void ScTabPageSortFields::ActivatePage()
{
    if ( pDlg )
    {
        if (   bHasHeader  != pDlg->GetHeaders()
            || bSortByRows != pDlg->GetByRows()   )
        {
            sal_uInt16  nCurSel1 = aLbSort1.GetSelectEntryPos();
            sal_uInt16  nCurSel2 = aLbSort2.GetSelectEntryPos();
            sal_uInt16  nCurSel3 = aLbSort3.GetSelectEntryPos();

            bHasHeader  = pDlg->GetHeaders();
            bSortByRows = pDlg->GetByRows();
            FillFieldLists();
            aLbSort1.SelectEntryPos( nCurSel1 );
            aLbSort2.SelectEntryPos( nCurSel2 );
            aLbSort3.SelectEntryPos( nCurSel3 );
        }
    }
}

// -----------------------------------------------------------------------

int ScTabPageSortFields::DeactivatePage( SfxItemSet* pSetP )
{
    if ( pDlg )
    {
        if ( bHasHeader != pDlg->GetHeaders() )
            pDlg->SetHeaders( bHasHeader );

        if ( bSortByRows != pDlg->GetByRows() )
            pDlg->SetByRows( bSortByRows );
    }

    if ( pSetP )
        FillItemSet( *pSetP );

    return SfxTabPage::LEAVE_PAGE;
}

// -----------------------------------------------------------------------

void ScTabPageSortFields::DisableField( sal_uInt16 nField )
{
    nField--;

    if ( nField<=2 )
    {
        aSortLbArr[nField]   ->Disable();
        aDirBtnArr[nField][0]->Disable();
        aDirBtnArr[nField][1]->Disable();
        aFlArr[nField]       ->Disable();
    }
}

// -----------------------------------------------------------------------

void ScTabPageSortFields::EnableField( sal_uInt16 nField )
{
    nField--;

    if ( nField<=2 )
    {
        aSortLbArr[nField]   ->Enable();
        aDirBtnArr[nField][0]->Enable();
        aDirBtnArr[nField][1]->Enable();
        aFlArr[nField]       ->Enable();
    }
}

// -----------------------------------------------------------------------

void ScTabPageSortFields::FillFieldLists()
{
    if ( pViewData )
    {
        ScDocument* pDoc = pViewData->GetDocument();

        if ( pDoc )
        {
            aLbSort1.Clear();
            aLbSort2.Clear();
            aLbSort3.Clear();
            aLbSort1.InsertEntry( aStrUndefined, 0 );
            aLbSort2.InsertEntry( aStrUndefined, 0 );
            aLbSort3.InsertEntry( aStrUndefined, 0 );

            SCCOL   nFirstSortCol   = rSortData.nCol1;
            SCROW   nFirstSortRow   = rSortData.nRow1;
            SCTAB   nTab        = pViewData->GetTabNo();
            sal_uInt16  i           = 1;

            if ( bSortByRows )
            {
                String  aFieldName;
                SCCOL   nMaxCol = rSortData.nCol2;
                SCCOL   col;

                for ( col=nFirstSortCol; col<=nMaxCol && i<SC_MAXFIELDS; col++ )
                {
                    pDoc->GetString( col, nFirstSortRow, nTab, aFieldName );
                    if ( !bHasHeader || (aFieldName.Len() == 0) )
                    {
                        aFieldName  = aStrColumn;
                        aFieldName += ' ';
                        aFieldName += ScColToAlpha( col );
                    }
                    nFieldArr[i] = col;
                    aLbSort1.InsertEntry( aFieldName, i );
                    aLbSort2.InsertEntry( aFieldName, i );
                    aLbSort3.InsertEntry( aFieldName, i );
                    i++;
                }
            }
            else
            {
                String  aFieldName;
                SCROW   nMaxRow = rSortData.nRow2;
                SCROW   row;

                for ( row=nFirstSortRow; row<=nMaxRow && i<SC_MAXFIELDS; row++ )
                {
                    pDoc->GetString( nFirstSortCol, row, nTab, aFieldName );
                    if ( !bHasHeader || (aFieldName.Len() == 0) )
                    {
                        aFieldName  = aStrRow;
                        aFieldName += ' ';
                        aFieldName += String::CreateFromInt32( row+1 );
                    }
                    nFieldArr[i] = row;
                    aLbSort1.InsertEntry( aFieldName, i );
                    aLbSort2.InsertEntry( aFieldName, i );
                    aLbSort3.InsertEntry( aFieldName, i );
                    i++;
                }
            }
            nFieldCount = i;
        }
    }
}

//------------------------------------------------------------------------

sal_uInt16 ScTabPageSortFields::GetFieldSelPos( SCCOLROW nField )
{
    sal_uInt16  nFieldPos   = 0;
    sal_Bool    bFound      = false;

    for ( sal_uInt16 n=1; n<nFieldCount && !bFound; n++ )
    {
        if ( nFieldArr[n] == nField )
        {
            nFieldPos = n;
            bFound = sal_True;
        }
    }

    return nFieldPos;
}

// -----------------------------------------------------------------------
// Handler:
//---------

IMPL_LINK( ScTabPageSortFields, SelectHdl, ListBox *, pLb )
{
    String aSelEntry = pLb->GetSelectEntry();

    if ( pLb == &aLbSort1 )
    {
        if ( aSelEntry == aStrUndefined )
        {
            aLbSort2.SelectEntryPos( 0 );
            aLbSort3.SelectEntryPos( 0 );

            if ( aFlSort2.IsEnabled() )
                DisableField( 2 );

            if ( aFlSort3.IsEnabled() )
                DisableField( 3 );
        }
        else
        {
            if ( !aFlSort2.IsEnabled() )
                EnableField( 2 );
        }
    }
    else if ( pLb == &aLbSort2 )
    {
        if ( aSelEntry == aStrUndefined )
        {
            aLbSort3.SelectEntryPos( 0 );
            if ( aFlSort3.IsEnabled() )
                DisableField( 3 );
        }
        else
        {
            if ( !aFlSort3.IsEnabled() )
                EnableField( 3 );
        }
    }
    return 0;
}

//========================================================================
// Sortieroptionen-Tabpage:
//========================================================================

ScTabPageSortOptions::ScTabPageSortOptions( Window*             pParent,
                                            const SfxItemSet&   rArgSet )

    :   SfxTabPage      ( pParent,
                          ScResId( RID_SCPAGE_SORT_OPTIONS ),
                          rArgSet ),
        //
        aBtnCase        ( this, ScResId( BTN_CASESENSITIVE ) ),
        aBtnHeader      ( this, ScResId( BTN_LABEL ) ),
        aBtnFormats     ( this, ScResId( BTN_FORMATS ) ),
        aBtnNaturalSort ( this, ScResId( BTN_NATURALSORT ) ),
        aBtnCopyResult  ( this, ScResId( BTN_COPYRESULT ) ),
        aLbOutPos       ( this, ScResId( LB_OUTAREA ) ),
        aEdOutPos       ( this, ScResId( ED_OUTAREA ) ),
        aBtnSortUser    ( this, ScResId( BTN_SORT_USER ) ),
        aLbSortUser     ( this, ScResId( LB_SORT_USER ) ),
        aFtLanguage     ( this, ScResId( FT_LANGUAGE ) ),
        aLbLanguage     ( this, ScResId( LB_LANGUAGE ) ),
        aFtAlgorithm    ( this, ScResId( FT_ALGORITHM ) ),
        aLbAlgorithm    ( this, ScResId( LB_ALGORITHM ) ),
        aLineDirection  ( this, ScResId( FL_DIRECTION ) ),
        aBtnTopDown     ( this, ScResId( BTN_TOP_DOWN ) ),
        aBtnLeftRight   ( this, ScResId( BTN_LEFT_RIGHT ) ),
        //
        aStrRowLabel    ( ScResId( STR_ROW_LABEL ) ),
        aStrColLabel    ( ScResId( STR_COL_LABEL ) ),
        aStrUndefined   ( ScResId( SCSTR_UNDEFINED ) ),
        //
        nWhichSort      ( rArgSet.GetPool()->GetWhich( SID_SORT ) ),
        rSortData       ( ((const ScSortItem&)
                          rArgSet.Get( nWhichSort )).GetSortData() ),
        pViewData       ( NULL ),
        pDoc            ( NULL ),
        pDlg            ( (ScSortDlg*)(GetParent() ? GetParent()->GetParent() : 0 ) ),
        pColRes         ( NULL ),
        pColWrap        ( NULL )
{
    Init();
    FreeResource();
    SetExchangeSupport();

    aLbOutPos.SetAccessibleRelationLabeledBy(&aBtnCopyResult);
    aLbOutPos.SetAccessibleName(aBtnCopyResult.GetText());
    aEdOutPos.SetAccessibleRelationLabeledBy(&aBtnCopyResult);
    aEdOutPos.SetAccessibleName(aBtnCopyResult.GetText());
    aLbSortUser.SetAccessibleRelationLabeledBy(&aBtnSortUser);
    aLbSortUser.SetAccessibleName(aBtnSortUser.GetText());
}

// -----------------------------------------------------------------------

ScTabPageSortOptions::~ScTabPageSortOptions()
{
    sal_uInt16 nEntries = aLbOutPos.GetEntryCount();

    for ( sal_uInt16 i=1; i<nEntries; i++ )
        delete (String*)aLbOutPos.GetEntryData( i );

    delete pColRes;
    delete pColWrap;        //! not if from document
}

// -----------------------------------------------------------------------

void ScTabPageSortOptions::Init()
{
    //  CollatorRessource has user-visible names for sort algorithms
    pColRes = new CollatorRessource();

    //! use CollatorWrapper from document?
    pColWrap = new CollatorWrapper( comphelper::getProcessServiceFactory() );

    const ScSortItem&   rSortItem = (const ScSortItem&)
                                    GetItemSet().Get( nWhichSort );

    aLbOutPos.SetSelectHdl    ( LINK( this, ScTabPageSortOptions, SelOutPosHdl ) );
    aBtnCopyResult.SetClickHdl( LINK( this, ScTabPageSortOptions, EnableHdl ) );
    aBtnSortUser.SetClickHdl  ( LINK( this, ScTabPageSortOptions, EnableHdl ) );
    aBtnTopDown.SetClickHdl   ( LINK( this, ScTabPageSortOptions, SortDirHdl ) );
    aBtnLeftRight.SetClickHdl ( LINK( this, ScTabPageSortOptions, SortDirHdl ) );
    aLbLanguage.SetSelectHdl  ( LINK( this, ScTabPageSortOptions, FillAlgorHdl ) );

    pViewData = rSortItem.GetViewData();
    pDoc      = pViewData ? pViewData->GetDocument() : NULL;

    OSL_ENSURE( pViewData, "ViewData not found! :-/" );

    if ( pViewData && pDoc )
    {
        String          theArea;
        ScDBCollection* pDBColl     = pDoc->GetDBCollection();
        String          theDbArea;
        const SCTAB nCurTab     = pViewData->GetTabNo();
        String          theDbName   = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(STR_DB_LOCAL_NONAME));
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        aLbOutPos.Clear();
        aLbOutPos.InsertEntry( aStrUndefined, 0 );
        aLbOutPos.Disable();

        ScAreaNameIterator aIter( pDoc );
        String aName;
        ScRange aRange;
        String aRefStr;
        while ( aIter.Next( aName, aRange ) )
        {
            sal_uInt16 nInsert = aLbOutPos.InsertEntry( aName );

            aRange.aStart.Format( aRefStr, SCA_ABS_3D, pDoc, eConv );
            aLbOutPos.SetEntryData( nInsert, new String( aRefStr ) );
        }

        aLbOutPos.SelectEntryPos( 0 );
        aEdOutPos.SetText( EMPTY_STRING );

        /*
         * Ueberpruefen, ob es sich bei dem uebergebenen
         * Bereich um einen Datenbankbereich handelt:
         */

        ScAddress aScAddress( rSortData.nCol1, rSortData.nRow1, nCurTab );
        ScRange( aScAddress,
                 ScAddress( rSortData.nCol2, rSortData.nRow2, nCurTab )
               ).Format( theArea, SCR_ABS, pDoc, eConv );

        if ( pDBColl )
        {
            ScDBData* pDBData
                    = pDBColl->GetDBAtArea( nCurTab,
                                            rSortData.nCol1, rSortData.nRow1,
                                            rSortData.nCol2, rSortData.nRow2 );
            if ( pDBData )
            {
                theDbName = pDBData->GetName();
                aBtnHeader.Check( pDBData->HasHeader() );
            }
        }

        theArea.AppendAscii(RTL_CONSTASCII_STRINGPARAM(" ("));
        theArea += theDbName;
        theArea += ')';

        aBtnHeader.SetText( aStrColLabel );
    }

    FillUserSortListBox();

    //  get available languages

    aLbLanguage.SetLanguageList( LANG_LIST_ALL | LANG_LIST_ONLY_KNOWN, false );
    aLbLanguage.InsertLanguage( LANGUAGE_SYSTEM );
}

//------------------------------------------------------------------------

sal_uInt16* ScTabPageSortOptions::GetRanges()
{
    return pSortRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* ScTabPageSortOptions::Create(
                                            Window*             pParent,
                                            const SfxItemSet&   rArgSet )
{
    return ( new ScTabPageSortOptions( pParent, rArgSet ) );
}

// -----------------------------------------------------------------------

void ScTabPageSortOptions::Reset( const SfxItemSet& /* rArgSet */ )
{
    if ( rSortData.bUserDef )
    {
        aBtnSortUser.Check( sal_True );
        aLbSortUser.Enable();
        aLbSortUser.SelectEntryPos( rSortData.nUserIndex );
    }
    else
    {
        aBtnSortUser.Check( false );
        aLbSortUser.Disable();
        aLbSortUser.SelectEntryPos( 0 );
    }

    aBtnCase.Check          ( rSortData.bCaseSens );
    aBtnFormats.Check       ( rSortData.bIncludePattern );
    aBtnHeader.Check        ( rSortData.bHasHeader );
    aBtnNaturalSort.Check   ( rSortData.bNaturalSort );

    if ( rSortData.bByRow )
    {
        aBtnTopDown.Check();
        aBtnHeader.SetText( aStrColLabel );
    }
    else
    {
        aBtnLeftRight.Check();
        aBtnHeader.SetText( aStrRowLabel );
    }

    LanguageType eLang = MsLangId::convertLocaleToLanguage( rSortData.aCollatorLocale );
    if ( eLang == LANGUAGE_DONTKNOW )
        eLang = LANGUAGE_SYSTEM;
    aLbLanguage.SelectLanguage( eLang );
    FillAlgorHdl( &aLbLanguage );               // get algorithms, select default
    if ( !rSortData.aCollatorAlgorithm.isEmpty() )
        aLbAlgorithm.SelectEntry( pColRes->GetTranslation( rSortData.aCollatorAlgorithm ) );

    if ( pDoc && !rSortData.bInplace )
    {
        String aStr;
        sal_uInt16 nFormat = (rSortData.nDestTab != pViewData->GetTabNo())
                            ? SCR_ABS_3D
                            : SCR_ABS;

        theOutPos.Set( rSortData.nDestCol,
                       rSortData.nDestRow,
                       rSortData.nDestTab );

        theOutPos.Format( aStr, nFormat, pDoc, pDoc->GetAddressConvention() );
        aBtnCopyResult.Check();
        aLbOutPos.Enable();
        aEdOutPos.Enable();
        aEdOutPos.SetText( aStr );
        EdOutPosModHdl( &aEdOutPos );
        aEdOutPos.GrabFocus();
        aEdOutPos.SetSelection( Selection( 0, SELECTION_MAX ) );
    }
    else
    {
        aBtnCopyResult.Check( false );
        aLbOutPos.Disable();
        aEdOutPos.Disable();
        aEdOutPos.SetText( EMPTY_STRING );
    }
}

// -----------------------------------------------------------------------

sal_Bool ScTabPageSortOptions::FillItemSet( SfxItemSet& rArgSet )
{
    ScSortParam theSortData = rSortData;
    if (pDlg)
    {
        const SfxItemSet* pExample = pDlg->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExample && pExample->GetItemState( nWhichSort, sal_True, &pItem ) == SFX_ITEM_SET )
            theSortData = ((const ScSortItem*)pItem)->GetSortData();
    }

    theSortData.bByRow          = aBtnTopDown.IsChecked();
    theSortData.bHasHeader      = aBtnHeader.IsChecked();
    theSortData.bCaseSens       = aBtnCase.IsChecked();
    theSortData.bNaturalSort    = aBtnNaturalSort.IsChecked();
    theSortData.bIncludePattern = aBtnFormats.IsChecked();
    theSortData.bInplace        = !aBtnCopyResult.IsChecked();
    theSortData.nDestCol        = theOutPos.Col();
    theSortData.nDestRow        = theOutPos.Row();
    theSortData.nDestTab        = theOutPos.Tab();
    theSortData.bUserDef        = aBtnSortUser.IsChecked();
    theSortData.nUserIndex      = (aBtnSortUser.IsChecked())
                                    ? aLbSortUser.GetSelectEntryPos()
                                    : 0;

    // get locale
    LanguageType eLang = aLbLanguage.GetSelectLanguage();
    theSortData.aCollatorLocale = MsLangId::convertLanguageToLocale( eLang, false );

    // get algorithm
    String sAlg;
    if ( eLang != LANGUAGE_SYSTEM )
    {
        uno::Sequence<rtl::OUString> aAlgos = pColWrap->listCollatorAlgorithms(
                theSortData.aCollatorLocale );
        sal_uInt16 nSel = aLbAlgorithm.GetSelectEntryPos();
        if ( nSel < aAlgos.getLength() )
            sAlg = aAlgos[nSel];
    }
    theSortData.aCollatorAlgorithm = sAlg;

    rArgSet.Put( ScSortItem( SCITEM_SORTDATA, &theSortData ) );

    return sal_True;
}

// -----------------------------------------------------------------------

// fuer Datenaustausch ohne Dialog-Umweg: (! noch zu tun !)
void ScTabPageSortOptions::ActivatePage()
{
    if ( pDlg )
    {
        if ( aBtnHeader.IsChecked() != pDlg->GetHeaders() )
        {
            aBtnHeader.Check( pDlg->GetHeaders() );
        }

        if ( aBtnTopDown.IsChecked() != pDlg->GetByRows() )
        {
            aBtnTopDown.Check( pDlg->GetByRows() );
            aBtnLeftRight.Check( !pDlg->GetByRows() );
        }

        aBtnHeader.SetText( (pDlg->GetByRows())
                            ? aStrColLabel
                            : aStrRowLabel );
    }
}

// -----------------------------------------------------------------------

int ScTabPageSortOptions::DeactivatePage( SfxItemSet* pSetP )
{
    sal_Bool bPosInputOk = sal_True;

    if ( aBtnCopyResult.IsChecked() )
    {
        String      thePosStr = aEdOutPos.GetText();
        ScAddress   thePos;
        xub_StrLen  nColonPos = thePosStr.Search( ':' );

        if ( STRING_NOTFOUND != nColonPos )
            thePosStr.Erase( nColonPos );

        if ( pViewData )
        {
            //  visible table is default for input without table
            //  must be changed to GetRefTabNo when sorting has RefInput!
            thePos.SetTab( pViewData->GetTabNo() );
        }

        sal_uInt16 nResult = thePos.Parse( thePosStr, pDoc, pDoc->GetAddressConvention() );

        bPosInputOk = ( SCA_VALID == (nResult & SCA_VALID) );

        if ( !bPosInputOk )
        {
            ErrorBox( this, WinBits( WB_OK | WB_DEF_OK ),
                     ScGlobal::GetRscString( STR_INVALID_TABREF )
                    ).Execute();
            aEdOutPos.GrabFocus();
            aEdOutPos.SetSelection( Selection( 0, SELECTION_MAX ) );
            theOutPos.Set(0,0,0);
        }
        else
        {
            aEdOutPos.SetText( thePosStr );
            theOutPos = thePos;
        }
    }

    if ( pDlg && bPosInputOk )
    {
        pDlg->SetHeaders( aBtnHeader.IsChecked() );
        pDlg->SetByRows ( aBtnTopDown.IsChecked() );
    }

    if ( pSetP && bPosInputOk )
        FillItemSet( *pSetP );

    return bPosInputOk ? SfxTabPage::LEAVE_PAGE : SfxTabPage::KEEP_PAGE;
}

// -----------------------------------------------------------------------

void ScTabPageSortOptions::FillUserSortListBox()
{
    ScUserList* pUserLists = ScGlobal::GetUserList();

    aLbSortUser.Clear();
    if ( pUserLists )
    {
        size_t nCount = pUserLists->size();
        if ( nCount > 0 )
            for ( size_t i=0; i<nCount; ++i )
                aLbSortUser.InsertEntry( (*pUserLists)[i]->GetString() );
    }
}

// -----------------------------------------------------------------------
// Handler:

IMPL_LINK( ScTabPageSortOptions, EnableHdl, CheckBox *, pBox )
{
    if ( pBox == &aBtnCopyResult )
    {
        if ( pBox->IsChecked() )
        {
            aLbOutPos.Enable();
            aEdOutPos.Enable();
            aEdOutPos.GrabFocus();
        }
        else
        {
            aLbOutPos.Disable();
            aEdOutPos.Disable();
        }
    }
    else if ( pBox == &aBtnSortUser )
    {
        if ( pBox->IsChecked() )
        {
            aLbSortUser.Enable();
            aLbSortUser.GrabFocus();
        }
        else
            aLbSortUser.Disable();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ScTabPageSortOptions, SelOutPosHdl, ListBox *, pLb )
{
    if ( pLb == &aLbOutPos )
    {
        String  aString;
        sal_uInt16  nSelPos = aLbOutPos.GetSelectEntryPos();

        if ( nSelPos > 0 )
            aString = *(String*)aLbOutPos.GetEntryData( nSelPos );

        aEdOutPos.SetText( aString );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ScTabPageSortOptions, SortDirHdl, RadioButton *, pBtn )
{
    if ( pBtn == &aBtnTopDown )
    {
        aBtnHeader.SetText( aStrColLabel );
    }
    else if ( pBtn == &aBtnLeftRight )
    {
        aBtnHeader.SetText( aStrRowLabel );
    }
    return 0;
}

// -----------------------------------------------------------------------

void ScTabPageSortOptions::EdOutPosModHdl( Edit* pEd )
{
    if ( pEd == &aEdOutPos )
    {
        String  theCurPosStr = aEdOutPos.GetText();
        sal_uInt16  nResult = ScAddress().Parse( theCurPosStr, pDoc, pDoc->GetAddressConvention() );

        if ( SCA_VALID == (nResult & SCA_VALID) )
        {
            String* pStr    = NULL;
            sal_Bool    bFound  = false;
            sal_uInt16  i       = 0;
            sal_uInt16  nCount  = aLbOutPos.GetEntryCount();

            for ( i=2; i<nCount && !bFound; i++ )
            {
                pStr = (String*)aLbOutPos.GetEntryData( i );
                bFound = (theCurPosStr == *pStr);
            }

            if ( bFound )
                aLbOutPos.SelectEntryPos( --i );
            else
                aLbOutPos.SelectEntryPos( 0 );
        }
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( ScTabPageSortOptions, FillAlgorHdl, void *, EMPTYARG )
{
    aLbAlgorithm.SetUpdateMode( false );
    aLbAlgorithm.Clear();

    LanguageType eLang = aLbLanguage.GetSelectLanguage();
    if ( eLang == LANGUAGE_SYSTEM )
    {
        //  for LANGUAGE_SYSTEM no algorithm can be selected because
        //  it wouldn't necessarily exist for other languages
        //  -> leave list box empty if LANGUAGE_SYSTEM is selected
        aFtAlgorithm.Enable( false );           // nothing to select
        aLbAlgorithm.Enable( false );           // nothing to select
    }
    else
    {
        lang::Locale aLocale( MsLangId::convertLanguageToLocale( eLang ));
        uno::Sequence<rtl::OUString> aAlgos = pColWrap->listCollatorAlgorithms( aLocale );

        long nCount = aAlgos.getLength();
        const rtl::OUString* pArray = aAlgos.getConstArray();
        for (long i=0; i<nCount; i++)
        {
            String sAlg = pArray[i];
            String sUser = pColRes->GetTranslation( sAlg );
            aLbAlgorithm.InsertEntry( sUser, LISTBOX_APPEND );
        }
        aLbAlgorithm.SelectEntryPos( 0 );       // first entry is default
        aFtAlgorithm.Enable( nCount > 1 );      // enable only if there is a choice
        aLbAlgorithm.Enable( nCount > 1 );      // enable only if there is a choice
    }

    aLbAlgorithm.SetUpdateMode( sal_True );
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
