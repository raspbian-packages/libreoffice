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


// System - Includes -----------------------------------------------------



#include <limits.h>

// INCLUDE ---------------------------------------------------------------

#include "olinetab.hxx"
#include "global.hxx"
#include "rechead.hxx"
#include "address.hxx"
#include "table.hxx"

//------------------------------------------------------------------------

ScOutlineEntry::ScOutlineEntry( SCCOLROW nNewStart, SCCOLROW nNewSize, bool bNewHidden ) :
    nStart  ( nNewStart ),
    nSize   ( nNewSize ),
    bHidden ( bNewHidden ),
    bVisible( sal_True )
{
}

ScOutlineEntry::ScOutlineEntry( const ScOutlineEntry& rEntry ) :
    ScDataObject(),
    nStart  ( rEntry.nStart ),
    nSize   ( rEntry.nSize ),
    bHidden ( rEntry.bHidden ),
    bVisible( rEntry.bVisible )
{
}

ScDataObject* ScOutlineEntry::Clone() const
{
    return new ScOutlineEntry( *this );
}

void ScOutlineEntry::Move( SCsCOLROW nDelta )
{
    SCCOLROW nNewPos = nStart + nDelta;
    if (nNewPos<0)
    {
        OSL_FAIL("OutlineEntry < 0");
        nNewPos = 0;
    }
    nStart = nNewPos;
}

void ScOutlineEntry::SetSize( SCSIZE nNewSize )
{
    if (nNewSize>0)
        nSize = nNewSize;
    else
    {
        OSL_FAIL("ScOutlineEntry Size == 0");
    }
}

void ScOutlineEntry::SetPosSize( SCCOLROW nNewPos, SCSIZE nNewSize )
{
    nStart = nNewPos;
    SetSize( nNewSize );
}

void ScOutlineEntry::SetHidden( bool bNewHidden )
{
    bHidden = bNewHidden;
}

void ScOutlineEntry::SetVisible( bool bNewVisible )
{
    bVisible = bNewVisible;
}

//------------------------------------------------------------------------

ScOutlineCollection::ScOutlineCollection() :
    ScSortedCollection( 4,4,false )
{
}

inline short IntCompare( SCCOLROW nX, SCCOLROW nY )
{
    if ( nX==nY ) return 0;
    else if ( nX<nY ) return -1;
    else return 1;
}

short ScOutlineCollection::Compare(ScDataObject* pKey1, ScDataObject* pKey2) const
{
    return IntCompare( ((ScOutlineEntry*)pKey1)->GetStart(),
                        ((ScOutlineEntry*)pKey2)->GetStart() );
}

sal_uInt16 ScOutlineCollection::FindStart( SCCOLROW nMinStart )
{
    //!                 binaer suchen ?

    sal_uInt16 nPos = 0;
    sal_uInt16 nLocalCount = GetCount();
    while ( (nPos<nLocalCount) ? (((ScOutlineEntry*)At(nPos))->GetStart() < nMinStart) : false )
        ++nPos;

    return nPos;
}

//------------------------------------------------------------------------

ScOutlineArray::ScOutlineArray() :
    nDepth( 0 )
{
}

ScOutlineArray::ScOutlineArray( const ScOutlineArray& rArray ) :
    nDepth( rArray.nDepth )
{
    for (sal_uInt16 nLevel=0; nLevel<nDepth; nLevel++)
    {
        sal_uInt16 nCount = rArray.aCollections[nLevel].GetCount();
        for (sal_uInt16 nEntry=0; nEntry<nCount; nEntry++)
        {
            ScOutlineEntry* pEntry = (ScOutlineEntry*) rArray.aCollections[nLevel].At(nEntry);
            aCollections[nLevel].Insert( new ScOutlineEntry( *pEntry ) );
        }
    }
}

void ScOutlineArray::FindEntry( SCCOLROW nSearchPos, sal_uInt16& rFindLevel, sal_uInt16& rFindIndex,
                                sal_uInt16 nMaxLevel )
{
    rFindLevel = rFindIndex = 0;

    if (nMaxLevel > nDepth)
        nMaxLevel = nDepth;

    for (sal_uInt16 nLevel=0; nLevel<nMaxLevel; nLevel++)               //! rueckwaerts suchen ?
    {
        ScOutlineCollection* pCollect = &aCollections[nLevel];
        sal_uInt16 nCount = pCollect->GetCount();
        for (sal_uInt16 i=0; i<nCount; i++)
        {
            ScOutlineEntry* pEntry = (ScOutlineEntry*) pCollect->At(i);
            if ( pEntry->GetStart() <= nSearchPos && pEntry->GetEnd() >= nSearchPos )
            {
                rFindLevel = nLevel + 1;            // naechster Level (zum Einfuegen)
                rFindIndex = i;
            }
        }
    }
}

bool ScOutlineArray::Insert( SCCOLROW nStartCol, SCCOLROW nEndCol, bool& rSizeChanged,
                                bool bHidden, bool bVisible )
{
    rSizeChanged = false;

    sal_uInt16 nStartLevel;
    sal_uInt16 nStartIndex;
    sal_uInt16 nEndLevel;
    sal_uInt16 nEndIndex;
    bool bFound = false;

    bool bCont;
    sal_uInt16 nFindMax;
    FindEntry( nStartCol, nStartLevel, nStartIndex );       // nLevel = neuer Level (alter+1) !!!
    FindEntry( nEndCol, nEndLevel, nEndIndex );
    nFindMax = Max(nStartLevel,nEndLevel);
    do
    {
        bCont = false;

        if ( nStartLevel == nEndLevel && nStartIndex == nEndIndex && nStartLevel < SC_OL_MAXDEPTH )
            bFound = true;

        if (!bFound)
        {
            if (nFindMax>0)
            {
                --nFindMax;
                if (nStartLevel)
                    if ( ((ScOutlineEntry*)aCollections[nStartLevel-1].At(nStartIndex))->
                                GetStart() == nStartCol )
                        FindEntry( nStartCol, nStartLevel, nStartIndex, nFindMax );
                if (nEndLevel)
                    if ( ((ScOutlineEntry*)aCollections[nEndLevel-1].At(nEndIndex))->
                                GetEnd() == nEndCol )
                        FindEntry( nEndCol, nEndLevel, nEndIndex, nFindMax );
                bCont = true;
            }
        }
    }
    while ( !bFound && bCont );

    if (!bFound)
        return false;

    sal_uInt16 nLevel = nStartLevel;

    //  untere verschieben

    bool bNeedSize = false;
    for ( short nMoveLevel = nDepth-1; nMoveLevel >= (short) nLevel; nMoveLevel-- )
    {
        sal_uInt16 nCount = aCollections[nMoveLevel].GetCount();
        bool bMoved = false;
        for ( sal_uInt16 i=0; i<nCount; i += bMoved ? 0 : 1 )
        {
            ScOutlineEntry* pEntry = (ScOutlineEntry*) aCollections[nMoveLevel].At(i);
            SCCOLROW nEntryStart = pEntry->GetStart();
            if ( nEntryStart >= nStartCol && nEntryStart <= nEndCol )
            {
                if (nMoveLevel >= SC_OL_MAXDEPTH - 1)
                {
                    rSizeChanged = false;               // kein Platz
                    return false;
                }
                aCollections[nMoveLevel+1].Insert( new ScOutlineEntry( *pEntry ) );
                aCollections[nMoveLevel].AtFree( i );
                nCount = aCollections[nMoveLevel].GetCount();
                bMoved = true;
                if (nMoveLevel == (short) nDepth - 1)
                    bNeedSize = sal_True;
            }
            else
                bMoved = false;
        }
    }

    if (bNeedSize)
    {
        ++nDepth;
        rSizeChanged = true;
    }

    if (nDepth <= nLevel)
    {
        nDepth = nLevel+1;
        rSizeChanged = true;
    }

    ScOutlineEntry* pNewEntry = new ScOutlineEntry( nStartCol, nEndCol+1-nStartCol, bHidden );
    pNewEntry->SetVisible( bVisible );
    aCollections[nLevel].Insert( pNewEntry );

    return true;
}

sal_Bool ScOutlineArray::FindTouchedLevel( SCCOLROW nBlockStart, SCCOLROW nBlockEnd, sal_uInt16& rFindLevel ) const
{
    sal_Bool bFound = false;
    rFindLevel = 0;

    for (sal_uInt16 nLevel=0; nLevel<nDepth; nLevel++)
    {
        const ScOutlineCollection* pCollect = &aCollections[nLevel];
        sal_uInt16 nCount = pCollect->GetCount();
        for (sal_uInt16 i=0; i<nCount; i++)
        {
            ScOutlineEntry* pEntry = (ScOutlineEntry*) pCollect->At(i);
            SCCOLROW nStart = pEntry->GetStart();
            SCCOLROW nEnd   = pEntry->GetEnd();

            if ( ( nBlockStart>=nStart && nBlockStart<=nEnd ) ||
                 ( nBlockEnd  >=nStart && nBlockEnd  <=nEnd ) )
            {
                rFindLevel = nLevel;            // wirklicher Level
                bFound = sal_True;
            }
        }
    }

    return bFound;
}

void ScOutlineArray::RemoveSub( SCCOLROW nStartPos, SCCOLROW nEndPos, sal_uInt16 nLevel )
{
    if ( nLevel >= nDepth )
        return;
    ScOutlineCollection* pCollect = &aCollections[nLevel];
    sal_uInt16 nCount = pCollect->GetCount();
    sal_Bool bFound = false;
    for ( sal_uInt16 i=0; i<nCount; i += ( bFound ? 0 : 1 ) )
    {
        bFound = false;
        ScOutlineEntry* pEntry = (ScOutlineEntry*) pCollect->At(i);
        SCCOLROW nStart = pEntry->GetStart();
        SCCOLROW nEnd   = pEntry->GetEnd();

        if ( nStart>=nStartPos && nEnd<=nEndPos )
        {
            RemoveSub( nStart, nEnd, nLevel+1 );
            pCollect->AtFree(i);
            nCount = pCollect->GetCount();
            bFound = sal_True;
        }
    }
}

void ScOutlineArray::PromoteSub( SCCOLROW nStartPos, SCCOLROW nEndPos, sal_uInt16 nStartLevel )
{
    if (nStartLevel==0)
    {
        OSL_FAIL("PromoteSub mit Level 0");
        return;
    }

    for (sal_uInt16 nLevel = nStartLevel; nLevel < nDepth; nLevel++)
    {
        ScOutlineCollection* pCollect = &aCollections[nLevel];
        sal_uInt16 nCount = pCollect->GetCount();
        sal_Bool bFound = false;
        for ( sal_uInt16 i=0; i<nCount; i += ( bFound ? 0 : 1 ) )
        {
            bFound = false;
            ScOutlineEntry* pEntry = (ScOutlineEntry*) pCollect->At(i);
            SCCOLROW nStart = pEntry->GetStart();
            SCCOLROW nEnd   = pEntry->GetEnd();

            if ( nStart>=nStartPos && nEnd<=nEndPos )
            {
                aCollections[nLevel-1].Insert( new ScOutlineEntry( *pEntry ) );
                pCollect->AtFree(i);
                nCount = pCollect->GetCount();
                bFound = sal_True;
            }
        }
    }
}

sal_Bool ScOutlineArray::DecDepth()                         // nDepth auf leere Levels anpassen
{
    sal_Bool bChanged = false;
    sal_Bool bCont;
    do
    {
        bCont = false;
        if (nDepth)
            if (aCollections[nDepth-1].GetCount() == 0)
            {
                --nDepth;
                bChanged = sal_True;
                bCont = sal_True;
            }
    }
    while (bCont);
    return bChanged;
}

sal_Bool ScOutlineArray::Remove( SCCOLROW nBlockStart, SCCOLROW nBlockEnd, sal_Bool& rSizeChanged )
{
    sal_uInt16 nLevel;
    FindTouchedLevel( nBlockStart, nBlockEnd, nLevel );

    ScOutlineCollection* pCollect = &aCollections[nLevel];
    sal_uInt16 nCount = pCollect->GetCount();
    sal_Bool bFound = false;
    sal_Bool bAny = false;
    for ( sal_uInt16 i=0; i<nCount; i += ( bFound ? 0 : 1 ) )
    {
        bFound = false;
        ScOutlineEntry* pEntry = (ScOutlineEntry*) pCollect->At(i);
        SCCOLROW nStart = pEntry->GetStart();
        SCCOLROW nEnd   = pEntry->GetEnd();

        if ( nBlockStart<=nEnd && nBlockEnd>=nStart )
        {
            pCollect->AtFree(i);
            PromoteSub( nStart, nEnd, nLevel+1 );
            nCount = pCollect->GetCount();
            i = pCollect->FindStart( nEnd+1 );
            bFound = sal_True;
            bAny = sal_True;
        }
    }

    if (bAny)                                   // Depth anpassen
        if (DecDepth())
            rSizeChanged = sal_True;

    return bAny;
}

ScOutlineEntry* ScOutlineArray::GetEntry( sal_uInt16 nLevel, sal_uInt16 nIndex ) const
{
    return (ScOutlineEntry*)((nLevel < nDepth) ? aCollections[nLevel].At(nIndex) : NULL);
}

sal_uInt16 ScOutlineArray::GetCount( sal_uInt16 nLevel ) const
{
    return (nLevel < nDepth) ? aCollections[nLevel].GetCount() : 0;
}

ScOutlineEntry* ScOutlineArray::GetEntryByPos( sal_uInt16 nLevel, SCCOLROW nPos ) const
{
    sal_uInt16          nCount  = GetCount( nLevel );
    ScOutlineEntry* pEntry;

    for (sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++)
    {
        pEntry = GetEntry( nLevel, nIndex );
        if ((pEntry->GetStart() <= nPos) && (nPos <= pEntry->GetEnd()))
            return pEntry;
    }
    return NULL;
}

sal_Bool ScOutlineArray::GetEntryIndex( sal_uInt16 nLevel, SCCOLROW nPos, sal_uInt16& rnIndex ) const
{
    // found entry contains passed position
    sal_uInt16 nCount  = GetCount( nLevel );
    for ( rnIndex = 0; rnIndex < nCount; ++rnIndex )
    {
        const ScOutlineEntry* pEntry = GetEntry( nLevel, rnIndex );
        if ( (pEntry->GetStart() <= nPos) && (nPos <= pEntry->GetEnd()) )
            return sal_True;
    }
    return false;
}

sal_Bool ScOutlineArray::GetEntryIndexInRange(
        sal_uInt16 nLevel, SCCOLROW nBlockStart, SCCOLROW nBlockEnd, sal_uInt16& rnIndex ) const
{
    // found entry will be completely inside of passed range
    sal_uInt16 nCount  = GetCount( nLevel );
    for ( rnIndex = 0; rnIndex < nCount; ++rnIndex )
    {
        const ScOutlineEntry* pEntry = GetEntry( nLevel, rnIndex );
        if ( (nBlockStart <= pEntry->GetStart()) && (pEntry->GetEnd() <= nBlockEnd) )
            return sal_True;
    }
    return false;
}

void ScOutlineArray::SetVisibleBelow( sal_uInt16 nLevel, sal_uInt16 nEntry, sal_Bool bValue, sal_Bool bSkipHidden )
{
    ScOutlineEntry* pEntry = GetEntry( nLevel, nEntry );
    if( pEntry )
    {
        SCCOLROW nStart = pEntry->GetStart();
        SCCOLROW nEnd   = pEntry->GetEnd();

        for (sal_uInt16 nSubLevel=nLevel+1; nSubLevel<nDepth; nSubLevel++)
        {
            sal_uInt16 i = 0;
            pEntry = (ScOutlineEntry*) aCollections[nSubLevel].At(i);
            while (pEntry)
            {
                if (pEntry->GetStart() >= nStart && pEntry->GetEnd() <= nEnd)
                {
                    pEntry->SetVisible(bValue);

                    if (bSkipHidden)
                        if (!pEntry->IsHidden())
                            SetVisibleBelow( nSubLevel, i, bValue, sal_True );
                }

                ++i;
                pEntry = (ScOutlineEntry*) aCollections[nSubLevel].At(i);
            }

            if (bSkipHidden)
                nSubLevel = nDepth;             // Abbruch
        }
    }
}

void ScOutlineArray::GetRange( SCCOLROW& rStart, SCCOLROW& rEnd ) const
{
    sal_uInt16 nCount = aCollections[0].GetCount();
    if (nCount)
    {
        rStart = ((ScOutlineEntry*) aCollections[0].At(0))->GetStart();
        rEnd   = ((ScOutlineEntry*) aCollections[0].At(nCount-1))->GetEnd();
    }
    else
        rStart = rEnd = 0;
}

void ScOutlineArray::ExtendBlock( sal_uInt16 nLevel, SCCOLROW& rBlkStart, SCCOLROW& rBlkEnd )
{
    sal_uInt16  nCount;
    SCCOLROW    nStart;
    SCCOLROW    nEnd;
    sal_uInt16  i;
    ScOutlineEntry* pEntry;

    nCount = GetCount(nLevel);
    for ( i=0; i<nCount; i++ )
    {
        pEntry = (ScOutlineEntry*) aCollections[nLevel].At(i);
        nStart = pEntry->GetStart();
        nEnd   = pEntry->GetEnd();

        if ( rBlkStart<=nEnd && rBlkEnd>=nStart )
        {
            if (nStart<rBlkStart) rBlkStart = nStart;
            if (nEnd>rBlkEnd) rBlkEnd = nEnd;
        }
    }
}

sal_Bool ScOutlineArray::TestInsertSpace( SCSIZE nSize, SCCOLROW nMaxVal ) const
{
    sal_uInt16 nCount = aCollections[0].GetCount();
    if (nCount)
    {
        SCCOLROW nEnd = ((ScOutlineEntry*) aCollections[0].At(nCount-1))->GetEnd();
        return ( sal::static_int_cast<SCCOLROW>(nEnd+nSize) <= nMaxVal );
    }

    return sal_True;
}

void ScOutlineArray::InsertSpace( SCCOLROW nStartPos, SCSIZE nSize )
{
    ScSubOutlineIterator aIter( this );
    ScOutlineEntry* pEntry;
    while((pEntry=aIter.GetNext())!=NULL)
    {
        if ( pEntry->GetStart() >= nStartPos )
            pEntry->Move(static_cast<SCsCOLROW>(nSize));
        else
        {
            SCCOLROW nEnd = pEntry->GetEnd();
            //  immer erweitern, wenn innerhalb der Gruppe eingefuegt
            //  beim Einfuegen am Ende nur, wenn die Gruppe nicht ausgeblendet ist
            if ( nEnd >= nStartPos || ( nEnd+1 >= nStartPos && !pEntry->IsHidden() ) )
            {
                SCSIZE nEntrySize = pEntry->GetSize();
                nEntrySize += nSize;
                pEntry->SetSize( nEntrySize );
            }
        }
    }
}

sal_Bool ScOutlineArray::DeleteSpace( SCCOLROW nStartPos, SCSIZE nSize )
{
    SCCOLROW nEndPos = nStartPos + nSize - 1;
    sal_Bool bNeedSave = false;                         // Original fuer Undo benoetigt?
    sal_Bool bChanged = false;                          // fuer Test auf Level

    ScSubOutlineIterator aIter( this );
    ScOutlineEntry* pEntry;
    while((pEntry=aIter.GetNext())!=NULL)
    {
        SCCOLROW nEntryStart = pEntry->GetStart();
        SCCOLROW nEntryEnd   = pEntry->GetEnd();
        SCSIZE nEntrySize    = pEntry->GetSize();

        if ( nEntryEnd >= nStartPos )
        {
            if ( nEntryStart > nEndPos )                                        // rechts
                pEntry->Move(-(static_cast<SCsCOLROW>(nSize)));
            else if ( nEntryStart < nStartPos && nEntryEnd >= nEndPos )         // aussen
                pEntry->SetSize( nEntrySize-nSize );
            else
            {
                bNeedSave = sal_True;
                if ( nEntryStart >= nStartPos && nEntryEnd <= nEndPos )             // innen
                {
                    aIter.DeleteLast();
                    bChanged = sal_True;
                }
                else if ( nEntryStart >= nStartPos )                                // rechts ueber
                    pEntry->SetPosSize( nStartPos, static_cast<SCSIZE>(nEntryEnd-nEndPos) );
                else                                                                // links ueber
                    pEntry->SetSize( static_cast<SCSIZE>(nStartPos-nEntryStart) );
            }
        }
    }

    if (bChanged)
        DecDepth();

    return bNeedSave;
}

bool ScOutlineArray::ManualAction( SCCOLROW nStartPos, SCCOLROW nEndPos, bool bShow, ScTable& rTable, bool bCol )
{
    bool bModified = false;
    ScSubOutlineIterator aIter( this );
    ScOutlineEntry* pEntry;
    while((pEntry=aIter.GetNext())!=NULL)
    {
        SCCOLROW nEntryStart = pEntry->GetStart();
        SCCOLROW nEntryEnd   = pEntry->GetEnd();

        if (nEntryEnd>=nStartPos && nEntryStart<=nEndPos)
        {
            if ( pEntry->IsHidden() == bShow )
            {
                //  #i12341# hide if all columns/rows are hidden, show if at least one
                //  is visible
                SCCOLROW nEnd = rTable.LastHiddenColRow(nEntryStart, bCol);
                bool bAllHidden = (nEntryEnd <= nEnd && nEnd <
                        ::std::numeric_limits<SCCOLROW>::max());

                bool bToggle = ( bShow != bAllHidden );
                if ( bToggle )
                {
                    pEntry->SetHidden( !bShow );
                    SetVisibleBelow( aIter.LastLevel(), aIter.LastEntry(), bShow, bShow );
                    bModified = true;
                }
            }
        }
    }
    return bModified;
}

void ScOutlineArray::RemoveAll()
{
    for (sal_uInt16 nLevel=0; nLevel<nDepth; nLevel++)
        aCollections[nLevel].FreeAll();

    nDepth = 0;
}

//------------------------------------------------------------------------

ScOutlineTable::ScOutlineTable()
{
}

ScOutlineTable::ScOutlineTable( const ScOutlineTable& rOutline ) :
    aColOutline( rOutline.aColOutline ),
    aRowOutline( rOutline.aRowOutline )
{
}

sal_Bool ScOutlineTable::TestInsertCol( SCSIZE nSize )
{
    return aColOutline.TestInsertSpace( nSize, MAXCOL );
}

void ScOutlineTable::InsertCol( SCCOL nStartCol, SCSIZE nSize )
{
    aColOutline.InsertSpace( nStartCol, nSize );
}

sal_Bool ScOutlineTable::DeleteCol( SCCOL nStartCol, SCSIZE nSize )
{
    return aColOutline.DeleteSpace( nStartCol, nSize );
}

sal_Bool ScOutlineTable::TestInsertRow( SCSIZE nSize )
{
    return aRowOutline.TestInsertSpace( nSize, MAXROW );
}

void ScOutlineTable::InsertRow( SCROW nStartRow, SCSIZE nSize )
{
    aRowOutline.InsertSpace( nStartRow, nSize );
}

sal_Bool ScOutlineTable::DeleteRow( SCROW nStartRow, SCSIZE nSize )
{
    return aRowOutline.DeleteSpace( nStartRow, nSize );
}

//------------------------------------------------------------------------

ScSubOutlineIterator::ScSubOutlineIterator( ScOutlineArray* pOutlineArray ) :
        pArray( pOutlineArray ),
        nStart( 0 ),
        nEnd( SCCOLROW_MAX ),                           // alle durchgehen
        nSubLevel( 0 ),
        nSubEntry( 0 )
{
    nDepth = pArray->nDepth;
}

ScSubOutlineIterator::ScSubOutlineIterator( ScOutlineArray* pOutlineArray,
                                            sal_uInt16 nLevel, sal_uInt16 nEntry ) :
        pArray( pOutlineArray )
{
    ScOutlineEntry* pEntry = (ScOutlineEntry*) pArray->aCollections[nLevel].At(nEntry);
    nStart = pEntry->GetStart();
    nEnd   = pEntry->GetEnd();
    nSubLevel = nLevel + 1;
    nSubEntry = 0;
    nDepth = pArray->nDepth;
}

ScOutlineEntry* ScSubOutlineIterator::GetNext()
{
    ScOutlineEntry* pEntry;
    sal_Bool bFound = false;
    do
    {
        if (nSubLevel >= nDepth)
            return NULL;

        pEntry = (ScOutlineEntry*) pArray->aCollections[nSubLevel].At(nSubEntry);
        if (!pEntry)
        {
            nSubEntry = 0;
            ++nSubLevel;
        }
        else
        {
            if ( pEntry->GetStart() >= nStart && pEntry->GetEnd() <= nEnd )
                bFound = sal_True;
            ++nSubEntry;
        }
    }
    while (!bFound);
    return pEntry;                  // nSubLevel gueltig, wenn pEntry != 0
}

sal_uInt16 ScSubOutlineIterator::LastLevel() const
{
    return nSubLevel;
}

sal_uInt16 ScSubOutlineIterator::LastEntry() const
{
    if (nSubEntry == 0)
    {
        OSL_FAIL("ScSubOutlineIterator::LastEntry vor GetNext");
        return 0;
    }
    return nSubEntry-1;
}

void ScSubOutlineIterator::DeleteLast()
{
    if (nSubLevel >= nDepth)
    {
        OSL_FAIL("ScSubOutlineIterator::DeleteLast nach Ende");
        return;
    }
    if (nSubEntry == 0)
    {
        OSL_FAIL("ScSubOutlineIterator::DeleteLast vor GetNext");
        return;
    }

    --nSubEntry;
    pArray->aCollections[nSubLevel].AtFree(nSubEntry);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
