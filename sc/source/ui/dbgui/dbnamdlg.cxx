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


// System - Includes ---------------------------------------------------------



// INCLUDE -------------------------------------------------------------------

#include <vcl/msgbox.hxx>

#include "reffact.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "dbnamdlg.hrc"
#include "rangenam.hxx"     // IsNameValid
#include "globalnames.hxx"

#define _DBNAMDLG_CXX
#include "dbnamdlg.hxx"
#undef _DBNAMDLG_CXX


//============================================================================

#define ABS_SREF          SCA_VALID \
                        | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF          ABS_SREF \
                        | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define ABS_SREF3D      ABS_SREF | SCA_TAB_3D
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

//----------------------------------------------------------------------------

class DBSaveData;

static DBSaveData* pSaveObj = NULL;

#define ERRORBOX(s) ErrorBox(this,WinBits(WB_OK|WB_DEF_OK),s).Execute()

//============================================================================
//  class DBSaveData

class DBSaveData
{
public:
    DBSaveData( Edit& rEd, CheckBox& rHdr, CheckBox& rSize, CheckBox& rFmt,
                            CheckBox& rStrip, ScRange& rArea )
        : rEdAssign(rEd),
          rBtnHeader(rHdr), rBtnSize(rSize), rBtnFormat(rFmt), rBtnStrip(rStrip),
          rCurArea(rArea),
          bHeader(false), bSize(false), bFormat(false), bDirty(false) {}
    void Save();
    void Restore();

private:
    Edit&       rEdAssign;
    CheckBox&   rBtnHeader;
    CheckBox&   rBtnSize;
    CheckBox&   rBtnFormat;
    CheckBox&   rBtnStrip;
    ScRange&    rCurArea;
    String      aStr;
    ScRange     aArea;
    sal_Bool        bHeader:1;
    sal_Bool        bSize:1;
    sal_Bool        bFormat:1;
    sal_Bool        bStrip:1;
    sal_Bool        bDirty:1;
};



//----------------------------------------------------------------------------

void DBSaveData::Save()
{
    aArea   = rCurArea;
    aStr    = rEdAssign.GetText();
    bHeader = rBtnHeader.IsChecked();
    bSize   = rBtnSize.IsChecked();
    bFormat = rBtnFormat.IsChecked();
    bStrip  = rBtnStrip.IsChecked();
    bDirty  = sal_True;
}


//----------------------------------------------------------------------------

void DBSaveData::Restore()
{
    if ( bDirty )
    {
        rCurArea = aArea;
        rEdAssign.SetText( aStr );
        rBtnHeader.Check ( bHeader );
        rBtnSize.Check   ( bSize );
        rBtnFormat.Check ( bFormat );
        rBtnStrip.Check  ( bStrip );
        bDirty = false;
    }
}


//============================================================================
//  class ScDbNameDlg

//----------------------------------------------------------------------------

ScDbNameDlg::ScDbNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                          ScViewData*   ptrViewData )

    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_DBNAMES ),
        //
        aFlName         ( this, ScResId( FL_NAME ) ),
        aEdName         ( this, ScResId( ED_NAME ) ),

        aFlAssign       ( this, ScResId( FL_ASSIGN ) ),
        aEdAssign       ( this, this, ScResId( ED_DBAREA ) ),
        aRbAssign       ( this, ScResId( RB_DBAREA ), &aEdAssign, this ),

        aFlOptions      ( this, ScResId( FL_OPTIONS ) ),
        aBtnHeader      ( this, ScResId( BTN_HEADER ) ),
        aBtnDoSize      ( this, ScResId( BTN_SIZE ) ),
        aBtnKeepFmt     ( this, ScResId( BTN_FORMAT ) ),
        aBtnStripData   ( this, ScResId( BTN_STRIPDATA ) ),
        aFTSource       ( this, ScResId( FT_SOURCE ) ),
        aFTOperations   ( this, ScResId( FT_OPERATIONS ) ),

        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aBtnAdd         ( this, ScResId( BTN_ADD ) ),
        aBtnRemove      ( this, ScResId( BTN_REMOVE ) ),
        aBtnMore        ( this, ScResId( BTN_MORE ) ),

        aStrAdd         ( ScResId( STR_ADD ) ),
        aStrModify      ( ScResId( STR_MODIFY ) ),
        aStrInvalid     ( ScResId( STR_DB_INVALID ) ),
        //
        pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData->GetDocument() ),
        bRefInputMode   ( false ),
        aAddrDetails    ( pDoc->GetAddressConvention(), 0, 0 ),
        aLocalDbCol     ( *(pDoc->GetDBCollection()) )
{
    // WB_NOLABEL can't be set in resource...
    aFTSource.SetStyle( aFTSource.GetStyle() | WB_NOLABEL );
    aFTOperations.SetStyle( aFTOperations.GetStyle() | WB_NOLABEL );

    //  damit die Strings in der Resource bei den FixedTexten bleiben koennen:
    aStrSource      = aFTSource.GetText();
    aStrOperations  = aFTOperations.GetText();

    pSaveObj = new DBSaveData( aEdAssign, aBtnHeader,
                        aBtnDoSize, aBtnKeepFmt, aBtnStripData, theCurArea );
    Init();
    FreeResource();
    aRbAssign.SetAccessibleRelationMemberOf(&aFlAssign);
}


//----------------------------------------------------------------------------

ScDbNameDlg::~ScDbNameDlg()
{
    DELETEZ( pSaveObj );
}


//----------------------------------------------------------------------------

void ScDbNameDlg::Init()
{
    aBtnHeader.Check( sal_True );       // Default: mit Spaltenkoepfen

    aBtnMore.AddWindow( &aFlOptions );
    aBtnMore.AddWindow( &aBtnHeader );
    aBtnMore.AddWindow( &aBtnDoSize );
    aBtnMore.AddWindow( &aBtnKeepFmt );
    aBtnMore.AddWindow( &aBtnStripData );
    aBtnMore.AddWindow( &aFTSource );
    aBtnMore.AddWindow( &aFTOperations );

    aBtnOk.SetClickHdl      ( LINK( this, ScDbNameDlg, OkBtnHdl ) );
    aBtnCancel.SetClickHdl  ( LINK( this, ScDbNameDlg, CancelBtnHdl ) );
    aBtnAdd.SetClickHdl     ( LINK( this, ScDbNameDlg, AddBtnHdl ) );
    aBtnRemove.SetClickHdl  ( LINK( this, ScDbNameDlg, RemoveBtnHdl ) );
    aEdName.SetModifyHdl    ( LINK( this, ScDbNameDlg, NameModifyHdl ) );
    aEdAssign.SetModifyHdl  ( LINK( this, ScDbNameDlg, AssModifyHdl ) );
    UpdateNames();

    String  theAreaStr;

    if ( pViewData && pDoc )
    {
        SCCOL   nStartCol   = 0;
        SCROW   nStartRow   = 0;
        SCTAB   nStartTab   = 0;
        SCCOL   nEndCol     = 0;
        SCROW   nEndRow     = 0;
        SCTAB   nEndTab     = 0;

        ScDBCollection* pDBColl = pDoc->GetDBCollection();
        ScDBData*       pDBData = NULL;

        pViewData->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                  nEndCol,   nEndRow,  nEndTab );

        theCurArea = ScRange( ScAddress( nStartCol, nStartRow, nStartTab ),
                              ScAddress( nEndCol,   nEndRow,   nEndTab ) );

        theCurArea.Format( theAreaStr, ABS_DREF3D, pDoc, aAddrDetails );

        if ( pDBColl )
        {
            // Feststellen, ob definierter DB-Bereich markiert wurde:
            pDBData = pDBColl->GetDBAtCursor( nStartCol, nStartRow, nStartTab, sal_True );
            if ( pDBData )
            {
                ScAddress&  rStart = theCurArea.aStart;
                ScAddress&  rEnd   = theCurArea.aEnd;
                SCCOL nCol1;
                SCCOL  nCol2;
                SCROW  nRow1;
                SCROW  nRow2;
                SCTAB  nTab;

                pDBData->GetArea( nTab, nCol1, nRow1, nCol2, nRow2 );

                if (   (rStart.Tab() == nTab)
                    && (rStart.Col() == nCol1) && (rStart.Row() == nRow1)
                    && (rEnd.Col()   == nCol2) && (rEnd.Row()   == nRow2 ) )
                {
                    rtl::OUString aDBName = pDBData->GetName();
                    if (!aDBName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(STR_DB_LOCAL_NONAME)))
                        aEdName.SetText(aDBName);

                    aBtnHeader.Check( pDBData->HasHeader() );
                    aBtnDoSize.Check( pDBData->IsDoSize() );
                    aBtnKeepFmt.Check( pDBData->IsKeepFmt() );
                    aBtnStripData.Check( pDBData->IsStripData() );
                    SetInfoStrings( pDBData );
                }
            }
        }
    }

    aEdAssign.SetText( theAreaStr );
    aEdName.GrabFocus();
    bSaved=sal_True;
    pSaveObj->Save();
    NameModifyHdl( 0 );
}


void ScDbNameDlg::SetInfoStrings( const ScDBData* pDBData )
{
    ::rtl::OUStringBuffer aBuf;
    aBuf.append(aStrSource);
    if (pDBData)
    {
        aBuf.append(sal_Unicode(' '));
        aBuf.append(pDBData->GetSourceString());
    }
    aFTSource.SetText(aBuf.makeStringAndClear());

    aBuf.append(aStrOperations);
    String aOper = aStrOperations;
    if (pDBData)
    {
        aBuf.append(sal_Unicode(' '));
        aBuf.append(pDBData->GetOperations());
    }
    aFTOperations.SetText(aBuf.makeStringAndClear());
}

//----------------------------------------------------------------------------
// Uebergabe eines mit der Maus selektierten Tabellenbereiches, der dann als
//  neue Selektion im Referenz-Fenster angezeigt wird.

void ScDbNameDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( aEdAssign.IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( &aEdAssign );

        theCurArea = rRef;

        String aRefStr;
        theCurArea.Format( aRefStr, ABS_DREF3D, pDocP, aAddrDetails );
        aEdAssign.SetRefString( aRefStr );
        aBtnHeader.Enable();
        aBtnDoSize.Enable();
        aBtnKeepFmt.Enable();
        aBtnStripData.Enable();
        aFTSource.Enable();
        aFTOperations.Enable();
        aBtnAdd.Enable();
        bSaved=sal_True;
        pSaveObj->Save();
    }
}


//----------------------------------------------------------------------------

sal_Bool ScDbNameDlg::Close()
{
    return DoClose( ScDbNameDlgWrapper::GetChildWindowId() );
}

//------------------------------------------------------------------------

void ScDbNameDlg::SetActive()
{
    aEdAssign.GrabFocus();

    //  kein NameModifyHdl, weil sonst Bereiche nicht geaendert werden koennen
    //  (nach dem Aufziehen der Referenz wuerde der alte Inhalt wieder angezeigt)
    //  (der ausgewaehlte DB-Name hat sich auch nicht veraendert)

    RefInputDone();
}

//------------------------------------------------------------------------

void ScDbNameDlg::UpdateNames()
{
    typedef ScDBCollection::NamedDBs DBsType;

    const DBsType& rDBs = aLocalDbCol.getNamedDBs();

    aEdName.SetUpdateMode( false );
    //-----------------------------------------------------------
    aEdName.Clear();
    aEdAssign.SetText( EMPTY_STRING );

    if (!rDBs.empty())
    {
        DBsType::const_iterator itr = rDBs.begin(), itrEnd = rDBs.end();
        for (; itr != itrEnd; ++itr)
            aEdName.InsertEntry(itr->GetName());
    }
    else
    {
        aBtnAdd.SetText( aStrAdd );
        aBtnAdd.Disable();
        aBtnRemove.Disable();
    }
    //-----------------------------------------------------------
    aEdName.SetUpdateMode( sal_True );
    aEdName.Invalidate();
}

//------------------------------------------------------------------------

void ScDbNameDlg::UpdateDBData( const String& rStrName )
{

    const ScDBData* pData = aLocalDbCol.getNamedDBs().findByName(rStrName);

    if ( pData )
    {
        SCCOL nColStart = 0;
        SCROW nRowStart = 0;
        SCCOL nColEnd    = 0;
        SCROW nRowEnd    = 0;
        SCTAB nTab       = 0;

        pData->GetArea( nTab, nColStart, nRowStart, nColEnd, nRowEnd );
        theCurArea = ScRange( ScAddress( nColStart, nRowStart, nTab ),
                              ScAddress( nColEnd,   nRowEnd,   nTab ) );
        ::rtl::OUString theArea;
        theCurArea.Format( theArea, ABS_DREF3D, pDoc, aAddrDetails );
        aEdAssign.SetText( theArea );
        aBtnAdd.SetText( aStrModify );
        aBtnHeader.Check( pData->HasHeader() );
        aBtnDoSize.Check( pData->IsDoSize() );
        aBtnKeepFmt.Check( pData->IsKeepFmt() );
        aBtnStripData.Check( pData->IsStripData() );
        SetInfoStrings( pData );
    }

    aBtnAdd.SetText( aStrModify );
    aBtnAdd.Enable();
    aBtnRemove.Enable();
    aBtnHeader.Enable();
    aBtnDoSize.Enable();
    aBtnKeepFmt.Enable();
    aBtnStripData.Enable();
    aFTSource.Enable();
    aFTOperations.Enable();
}

//------------------------------------------------------------------------


sal_Bool ScDbNameDlg::IsRefInputMode() const
{
    return bRefInputMode;
}

//------------------------------------------------------------------------
// Handler:
// ========

IMPL_LINK( ScDbNameDlg, OkBtnHdl, void *, EMPTYARG )
{
    AddBtnHdl( 0 );

    // Der View die Aenderungen und die Remove-Liste uebergeben:
    // beide werden nur als Referenz uebergeben, so dass an dieser
    // Stelle keine Speicherleichen entstehen koennen:
    if ( pViewData )
        pViewData->GetView()->
            NotifyCloseDbNameDlg( aLocalDbCol, aRemoveList );

    Close();
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScDbNameDlg, CancelBtnHdl, void *, EMPTYARG )
{
    Close();
    return 0;
}
IMPL_LINK_INLINE_END( ScDbNameDlg, CancelBtnHdl, void *, EMPTYARG )

//------------------------------------------------------------------------

IMPL_LINK( ScDbNameDlg, AddBtnHdl, void *, EMPTYARG )
{
    String  aNewName = aEdName.GetText();
    String  aNewArea = aEdAssign.GetText();

    aNewName.EraseLeadingChars( ' ' );
    aNewName.EraseTrailingChars( ' ' );

    if ( aNewName.Len() > 0 && aNewArea.Len() > 0 )
    {
        if ( ScRangeData::IsNameValid( aNewName, pDoc ) && !aNewName.EqualsAscii(STR_DB_LOCAL_NONAME) )
        {
            //  weil jetzt editiert werden kann, muss erst geparst werden
            ScRange aTmpRange;
            String aText = aEdAssign.GetText();
            if ( aTmpRange.ParseAny( aText, pDoc, aAddrDetails ) & SCA_VALID )
            {
                theCurArea = aTmpRange;
                ScAddress aStart = theCurArea.aStart;
                ScAddress aEnd   = theCurArea.aEnd;

                ScDBData* pOldEntry = aLocalDbCol.getNamedDBs().findByName(aNewName);
                if (pOldEntry)
                {
                    //  Bereich veraendern

                    pOldEntry->MoveTo( aStart.Tab(), aStart.Col(), aStart.Row(),
                                                        aEnd.Col(), aEnd.Row() );
                    pOldEntry->SetByRow( sal_True );
                    pOldEntry->SetHeader( aBtnHeader.IsChecked() );
                    pOldEntry->SetDoSize( aBtnDoSize.IsChecked() );
                    pOldEntry->SetKeepFmt( aBtnKeepFmt.IsChecked() );
                    pOldEntry->SetStripData( aBtnStripData.IsChecked() );
                }
                else
                {
                    //  neuen Bereich einfuegen

                    ScDBData* pNewEntry = new ScDBData( aNewName, aStart.Tab(),
                                                        aStart.Col(), aStart.Row(),
                                                        aEnd.Col(), aEnd.Row(),
                                                        sal_True, aBtnHeader.IsChecked() );
                    pNewEntry->SetDoSize( aBtnDoSize.IsChecked() );
                    pNewEntry->SetKeepFmt( aBtnKeepFmt.IsChecked() );
                    pNewEntry->SetStripData( aBtnStripData.IsChecked() );

                    aLocalDbCol.getNamedDBs().insert(pNewEntry);
                }

                UpdateNames();

                aEdName.SetText( EMPTY_STRING );
                aEdName.GrabFocus();
                aBtnAdd.SetText( aStrAdd );
                aBtnAdd.Disable();
                aBtnRemove.Disable();
                aEdAssign.SetText( EMPTY_STRING );
                aBtnHeader.Check( sal_True );       // Default: mit Spaltenkoepfen
                aBtnDoSize.Check( false );
                aBtnKeepFmt.Check( false );
                aBtnStripData.Check( false );
                SetInfoStrings( NULL );     // leer
                theCurArea = ScRange();
                bSaved=sal_True;
                pSaveObj->Save();
                NameModifyHdl( 0 );
            }
            else
            {
                ERRORBOX( aStrInvalid );
                aEdAssign.SetSelection( Selection( 0, SELECTION_MAX ) );
                aEdAssign.GrabFocus();
            }
        }
        else
        {
            ERRORBOX( ScGlobal::GetRscString(STR_INVALIDNAME) );
            aEdName.SetSelection( Selection( 0, SELECTION_MAX ) );
            aEdName.GrabFocus();
        }
    }
    return 0;
}

namespace {

class FindByName : public ::std::unary_function<ScDBData, bool>
{
    const ::rtl::OUString& mrName;
public:
    FindByName(const ::rtl::OUString& rName) : mrName(rName) {}
    bool operator() (const ScDBData& r) const
    {
        return r.GetName().equals(mrName);
    }
};

}

IMPL_LINK( ScDbNameDlg, RemoveBtnHdl, void *, EMPTYARG )
{
    ::rtl::OUString aStrEntry = aEdName.GetText();
    ScDBCollection::NamedDBs& rDBs = aLocalDbCol.getNamedDBs();
    ScDBCollection::NamedDBs::iterator itr =
        ::std::find_if(rDBs.begin(), rDBs.end(), FindByName(aStrEntry));

    if (itr != rDBs.end())
    {
        String aStrDelMsg = ScGlobal::GetRscString( STR_QUERY_DELENTRY );

        ::rtl::OUStringBuffer aBuf;
        aBuf.append(aStrDelMsg.GetToken(0, '#'));
        aBuf.append(aStrEntry);
        aBuf.append(aStrDelMsg.GetToken(1, '#'));
        QueryBox aBox(this, WinBits(WB_YES_NO|WB_DEF_YES), aBuf.makeStringAndClear());

        if (RET_YES == aBox.Execute())
        {
            SCTAB nTab;
            SCCOL nColStart, nColEnd;
            SCROW nRowStart, nRowEnd;
            itr->GetArea( nTab, nColStart, nRowStart, nColEnd, nRowEnd );
            aRemoveList.push_back(
                ScRange( ScAddress( nColStart, nRowStart, nTab ),
                         ScAddress( nColEnd,   nRowEnd,   nTab ) ) );

            rDBs.erase(itr);

            UpdateNames();

            aEdName.SetText( EMPTY_STRING );
            aEdName.GrabFocus();
            aBtnAdd.SetText( aStrAdd );
            aBtnAdd.Disable();
            aBtnRemove.Disable();
            aEdAssign.SetText( EMPTY_STRING );
            theCurArea = ScRange();
            aBtnHeader.Check( sal_True );       // Default: mit Spaltenkoepfen
            aBtnDoSize.Check( false );
            aBtnKeepFmt.Check( false );
            aBtnStripData.Check( false );
            SetInfoStrings( NULL );     // leer
            bSaved=false;
            pSaveObj->Restore();
            NameModifyHdl( 0 );
        }
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( ScDbNameDlg, NameModifyHdl, void *, EMPTYARG )
{
    String  theName     = aEdName.GetText();
    sal_Bool    bNameFound  = (COMBOBOX_ENTRY_NOTFOUND
                           != aEdName.GetEntryPos( theName ));

    if ( theName.Len() == 0 )
    {
        if ( aBtnAdd.GetText() != aStrAdd )
            aBtnAdd.SetText( aStrAdd );
        aBtnAdd     .Disable();
        aBtnRemove  .Disable();
        aFlAssign   .Disable();
        aBtnHeader  .Disable();
        aBtnDoSize  .Disable();
        aBtnKeepFmt .Disable();
        aBtnStripData.Disable();
        aFTSource   .Disable();
        aFTOperations.Disable();
        aEdAssign   .Disable();
        aRbAssign   .Disable();
        //bSaved=sal_False;
        //pSaveObj->Restore();
        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Disable(sal_False);        //! allgemeine Methode im ScAnyRefDlg
        bRefInputMode = false;
    }
    else
    {
        if ( bNameFound )
        {
            if ( aBtnAdd.GetText() != aStrModify )
                aBtnAdd.SetText( aStrModify );

            if(!bSaved)
            {
                bSaved=sal_True;
                pSaveObj->Save();
            }
            UpdateDBData( theName );
        }
        else
        {
            if ( aBtnAdd.GetText() != aStrAdd )
                aBtnAdd.SetText( aStrAdd );

            bSaved=false;
            pSaveObj->Restore();

            if ( aEdAssign.GetText().Len() > 0 )
            {
                aBtnAdd.Enable();
                aBtnHeader.Enable();
                aBtnDoSize.Enable();
                aBtnKeepFmt.Enable();
                aBtnStripData.Enable();
                aFTSource.Enable();
                aFTOperations.Enable();
            }
            else
            {
                aBtnAdd.Disable();
                aBtnHeader.Disable();
                aBtnDoSize.Disable();
                aBtnKeepFmt.Disable();
                aBtnStripData.Disable();
                aFTSource.Disable();
                aFTOperations.Disable();
            }
            aBtnRemove.Disable();
        }

        aFlAssign.Enable();
        aEdAssign.Enable();
        aRbAssign.Enable();

        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Enable();
        bRefInputMode = true;
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( ScDbNameDlg, AssModifyHdl, void *, EMPTYARG )
{
    //  hier parsen fuer Save() etc.

    ScRange aTmpRange;
    String aText = aEdAssign.GetText();
    if ( aTmpRange.ParseAny( aText, pDoc, aAddrDetails ) & SCA_VALID )
        theCurArea = aTmpRange;

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
