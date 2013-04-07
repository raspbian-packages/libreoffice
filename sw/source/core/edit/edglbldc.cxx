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


#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <editsh.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <docary.hxx>
#include <swwait.hxx>
#include <swundo.hxx>       // fuer die UndoIds
#include <section.hxx>
#include <doctxm.hxx>
#include <edglbldc.hxx>


SV_IMPL_OP_PTRARR_SORT( SwGlblDocContents, SwGlblDocContentPtr )

sal_Bool SwEditShell::IsGlobalDoc() const
{
    return getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT);
}

void SwEditShell::SetGlblDocSaveLinks( sal_Bool bFlag )
{
    getIDocumentSettingAccess()->set(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS, bFlag);
    if( !GetDoc()->IsModified() )   // Bug 57028
    {
        GetDoc()->GetIDocumentUndoRedo().SetUndoNoResetModified();
    }
    GetDoc()->SetModified();
}

sal_Bool SwEditShell::IsGlblDocSaveLinks() const
{
    return getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS);
}

sal_uInt16 SwEditShell::GetGlobalDocContent( SwGlblDocContents& rArr ) const
{
    if( rArr.Count() )
        rArr.DeleteAndDestroy( 0, rArr.Count() );

    if( !getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        return 0;

    // dann alle gelinkten Bereiche auf der obersten Ebene
    SwDoc* pMyDoc = GetDoc();
    const SwSectionFmts& rSectFmts = pMyDoc->GetSections();
    sal_uInt16 n;

    for( n = rSectFmts.Count(); n; )
    {
        const SwSection* pSect = rSectFmts[ --n ]->GetGlobalDocSection();
        if( pSect )
        {
            SwGlblDocContentPtr pNew;
            switch( pSect->GetType() )
            {
            case TOX_HEADER_SECTION:    break;      // ignore
            case TOX_CONTENT_SECTION:
                OSL_ENSURE( pSect->ISA( SwTOXBaseSection ), "keine TOXBaseSection!" );
                pNew = new SwGlblDocContent( (SwTOXBaseSection*)pSect );
                break;

            default:
                pNew = new SwGlblDocContent( pSect );
                break;
            }
            if( !rArr.Insert( pNew ) )
                delete pNew;
        }
    }

    // und als letztes die Dummies (sonstiger Text) einfuegen
    SwNode* pNd;
    sal_uLong nSttIdx = pMyDoc->GetNodes().GetEndOfExtras().GetIndex() + 2;
    for( n = 0; n < rArr.Count(); ++n )
    {
        const SwGlblDocContent& rNew = *rArr[ n ];
        // suche von StartPos bis rNew.DocPos nach einem Content Node.
        // Existiert dieser, so muss ein DummyEintrag eingefuegt werden.
        for( ; nSttIdx < rNew.GetDocPos(); ++nSttIdx )
            if( ( pNd = pMyDoc->GetNodes()[ nSttIdx ])->IsCntntNode()
                || pNd->IsSectionNode() || pNd->IsTableNode() )
            {
                SwGlblDocContentPtr pNew = new SwGlblDocContent( nSttIdx );
                if( !rArr.Insert( pNew ) )
                    delete pNew;
                else
                    ++n;        // auf die naechste Position
                break;
            }

        // StartPosition aufs Ende setzen
        nSttIdx = pMyDoc->GetNodes()[ rNew.GetDocPos() ]->EndOfSectionIndex();
        ++nSttIdx;
    }

    // sollte man das Ende auch noch setzen??
    if( rArr.Count() )
    {
        sal_uLong nNdEnd = pMyDoc->GetNodes().GetEndOfContent().GetIndex();
        for( ; nSttIdx < nNdEnd; ++nSttIdx )
            if( ( pNd = pMyDoc->GetNodes()[ nSttIdx ])->IsCntntNode()
                || pNd->IsSectionNode() || pNd->IsTableNode() )
            {
                SwGlblDocContentPtr pNew = new SwGlblDocContent( nSttIdx );
                if( !rArr.Insert( pNew ) )
                    delete pNew;
                break;
            }
    }
    else
    {
        SwGlblDocContentPtr pNew = new SwGlblDocContent(
                    pMyDoc->GetNodes().GetEndOfExtras().GetIndex() + 2 );
        rArr.Insert( pNew );
    }
    return rArr.Count();
}

sal_Bool SwEditShell::InsertGlobalDocContent( const SwGlblDocContent& rInsPos,
        SwSectionData & rNew)
{
    if( !getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        return sal_False;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCrsr->GetPoint();
    rPos.nNode = rInsPos.GetDocPos();

    sal_Bool bEndUndo = sal_False;
    SwDoc* pMyDoc = GetDoc();
    SwTxtNode *const pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( pTxtNd )
        rPos.nContent.Assign( pTxtNd, 0 );
    else
    {
        bEndUndo = sal_True;
        pMyDoc->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
        rPos.nNode--;
        pMyDoc->AppendTxtNode( rPos );
        pCrsr->SetMark();
    }

    InsertSection( rNew );

    if( bEndUndo )
    {
        pMyDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
    EndAllAction();

    return sal_True;
}

sal_Bool SwEditShell::InsertGlobalDocContent( const SwGlblDocContent& rInsPos,
                                            const SwTOXBase& rTOX )
{
    if( !getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        return sal_False;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCrsr->GetPoint();
    rPos.nNode = rInsPos.GetDocPos();

    sal_Bool bEndUndo = sal_False;
    SwDoc* pMyDoc = GetDoc();
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( pTxtNd && pTxtNd->GetTxt().Len() && rPos.nNode.GetIndex() + 1 !=
        pMyDoc->GetNodes().GetEndOfContent().GetIndex() )
        rPos.nContent.Assign( pTxtNd, 0 );
    else
    {
        bEndUndo = sal_True;
        pMyDoc->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
        rPos.nNode--;
        pMyDoc->AppendTxtNode( rPos );
    }

    InsertTableOf( rTOX );

    if( bEndUndo )
    {
        pMyDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
    EndAllAction();

    return sal_True;
}

sal_Bool SwEditShell::InsertGlobalDocContent( const SwGlblDocContent& rInsPos )
{
    if( !getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        return sal_False;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCrsr->GetPoint();
    rPos.nNode = rInsPos.GetDocPos() - 1;
    rPos.nContent.Assign( 0, 0 );

    SwDoc* pMyDoc = GetDoc();
    pMyDoc->AppendTxtNode( rPos );
    EndAllAction();
    return sal_True;
}

sal_Bool SwEditShell::DeleteGlobalDocContent( const SwGlblDocContents& rArr ,
                                            sal_uInt16 nDelPos )
{
    if( !getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        return sal_False;

    SET_CURR_SHELL( this );
    StartAllAction();
    StartUndo( UNDO_START );

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCrsr->GetPoint();

    SwDoc* pMyDoc = GetDoc();
    const SwGlblDocContent& rDelPos = *rArr[ nDelPos ];
    sal_uLong nDelIdx = rDelPos.GetDocPos();
    if( 1 == rArr.Count() )
    {
        // ein Node muss aber da bleiben!
        rPos.nNode = nDelIdx - 1;
        rPos.nContent.Assign( 0, 0 );

        pMyDoc->AppendTxtNode( rPos );
        ++nDelIdx;
    }

    switch( rDelPos.GetType() )
    {
    case GLBLDOC_UNKNOWN:
        {
            rPos.nNode = nDelIdx;
            pCrsr->SetMark();
            if( ++nDelPos < rArr.Count() )
                rPos.nNode = rArr[ nDelPos ]->GetDocPos();
            else
                rPos.nNode = pMyDoc->GetNodes().GetEndOfContent();
            rPos.nNode--;
            if( !pMyDoc->DelFullPara( *pCrsr ) )
                Delete();
        }
        break;

    case GLBLDOC_TOXBASE:
        {
            SwTOXBaseSection* pTOX = (SwTOXBaseSection*)rDelPos.GetTOX();
            pMyDoc->DeleteTOX( *pTOX, sal_True );
        }
        break;

    case GLBLDOC_SECTION:
        {
            SwSectionFmt* pSectFmt = (SwSectionFmt*)rDelPos.GetSection()->GetFmt();
            pMyDoc->DelSectionFmt( pSectFmt, sal_True );
        }
        break;
    }

    EndUndo( UNDO_END );
    EndAllAction();
    return sal_True;
}

sal_Bool SwEditShell::MoveGlobalDocContent( const SwGlblDocContents& rArr ,
                                        sal_uInt16 nFromPos, sal_uInt16 nToPos,
                                        sal_uInt16 nInsPos )
{
    if( !getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) ||
        nFromPos >= rArr.Count() || nToPos > rArr.Count() ||
        nInsPos > rArr.Count() || nFromPos >= nToPos ||
        ( nFromPos <= nInsPos && nInsPos <= nToPos ) )
        return sal_False;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwDoc* pMyDoc = GetDoc();
    SwNodeRange aRg( pMyDoc->GetNodes(), rArr[ nFromPos ]->GetDocPos() );
    if( nToPos < rArr.Count() )
        aRg.aEnd = rArr[ nToPos ]->GetDocPos();
    else
        aRg.aEnd = pMyDoc->GetNodes().GetEndOfContent();

    SwNodeIndex aInsPos( pMyDoc->GetNodes() );
    if( nInsPos < rArr.Count() )
        aInsPos = rArr[ nInsPos ]->GetDocPos();
    else
        aInsPos  = pMyDoc->GetNodes().GetEndOfContent();

    bool bRet = pMyDoc->MoveNodeRange( aRg, aInsPos,
        static_cast<IDocumentContentOperations::SwMoveFlags>(
              IDocumentContentOperations::DOC_MOVEALLFLYS
            | IDocumentContentOperations::DOC_CREATEUNDOOBJ ));

    EndAllAction();
    return bRet;
}

sal_Bool SwEditShell::GotoGlobalDocContent( const SwGlblDocContent& rPos )
{
    if( !getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        return sal_False;

    SET_CURR_SHELL( this );
    SttCrsrMove();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rCrsrPos = *pCrsr->GetPoint();
    rCrsrPos.nNode = rPos.GetDocPos();

    SwDoc* pMyDoc = GetDoc();
    SwCntntNode * pCNd = rCrsrPos.nNode.GetNode().GetCntntNode();
    if( !pCNd )
        pCNd = pMyDoc->GetNodes().GoNext( &rCrsrPos.nNode );

    rCrsrPos.nContent.Assign( pCNd, 0 );

    EndCrsrMove();
    return sal_True;
}

SwGlblDocContent::SwGlblDocContent( sal_uLong nPos )
{
    eType = GLBLDOC_UNKNOWN;
    PTR.pTOX = 0;
    nDocPos = nPos;
}

SwGlblDocContent::SwGlblDocContent( const SwTOXBaseSection* pTOX )
{
    eType = GLBLDOC_TOXBASE;
    PTR.pTOX = pTOX;

    const SwSectionNode* pSectNd = pTOX->GetFmt()->GetSectionNode();
    nDocPos = pSectNd ? pSectNd->GetIndex() : 0;
}

SwGlblDocContent::SwGlblDocContent( const SwSection* pSect )
{
    eType = GLBLDOC_SECTION;
    PTR.pSect = pSect;

    const SwSectionNode* pSectNd = pSect->GetFmt()->GetSectionNode();
    nDocPos = pSectNd ? pSectNd->GetIndex() : 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
