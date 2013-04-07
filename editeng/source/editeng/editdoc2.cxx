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


#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <svl/smplhint.hxx>

#include <tools/rtti.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/tstpitem.hxx>

#include <editdoc.hxx>
#include <impedit.hxx>
#include <editdbg.hxx>

#include <editeng/numitem.hxx>

#include <editeng/akrnitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/charscaleitem.hxx>

#include <vcl/svapp.hxx>    // For AppWindow...

DBG_NAME( EE_ParaPortion )

SV_IMPL_VARARR( CharPosArray, sal_Int32 );


TextPortionList::TextPortionList()
{
}

TextPortionList::~TextPortionList()
{
    Reset();
}

void TextPortionList::Reset()
{
    for ( sal_uInt16 nPortion = 0; nPortion < Count(); nPortion++ )
        delete GetObject( nPortion );
    Remove( 0, Count() );
}

void TextPortionList::DeleteFromPortion( sal_uInt16 nDelFrom )
{
    DBG_ASSERT( ( nDelFrom < Count() ) || ( (nDelFrom == 0) && (Count() == 0) ), "DeleteFromPortion: Out of range" );
    for ( sal_uInt16 nP = nDelFrom; nP < Count(); nP++ )
        delete GetObject( nP );
    Remove( nDelFrom, Count()-nDelFrom );
}

sal_uInt16 TextPortionList::FindPortion( sal_uInt16 nCharPos, sal_uInt16& nPortionStart, sal_Bool bPreferStartingPortion )
{
    // When nCharPos at portion limit, the left portion is found
    sal_uInt16 nTmpPos = 0;
    for ( sal_uInt16 nPortion = 0; nPortion < Count(); nPortion++ )
    {
        TextPortion* pPortion = GetObject( nPortion );
        nTmpPos = nTmpPos + pPortion->GetLen();
        if ( nTmpPos >= nCharPos )
        {
            // take this one if we don't prefer the starting portion, or if it's the last one
            if ( ( nTmpPos != nCharPos ) || !bPreferStartingPortion || ( nPortion == Count() - 1 ) )
            {
                nPortionStart = nTmpPos - pPortion->GetLen();
                return nPortion;
            }
        }
    }
    OSL_FAIL( "FindPortion: Not found!" );
    return ( Count() - 1 );
}

sal_uInt16 TextPortionList::GetStartPos( sal_uInt16 nPortion )
{
    sal_uInt16 nPos = 0;
    for ( sal_uInt16 n = 0; n < nPortion; n++ )
    {
        TextPortion* pPortion = GetObject( n );
        nPos = nPos + pPortion->GetLen();
    }
    return nPos;
}


ExtraPortionInfo::ExtraPortionInfo()
{
    nOrgWidth = 0;
    nWidthFullCompression = 0;
    nMaxCompression100thPercent = 0;
    nAsianCompressionTypes = 0;
    nPortionOffsetX = 0;
    bFirstCharIsRightPunktuation = sal_False;
    bCompressed = sal_False;
    pOrgDXArray = NULL;
}

ExtraPortionInfo::~ExtraPortionInfo()
{
    delete[] pOrgDXArray;
}

void ExtraPortionInfo::SaveOrgDXArray( const sal_Int32* pDXArray, sal_uInt16 nLen )
{
    delete[] pOrgDXArray;
    pOrgDXArray = new sal_Int32[nLen];
    memcpy( pOrgDXArray, pDXArray, nLen*sizeof(sal_Int32) );
}


ParaPortion::ParaPortion( ContentNode* pN )
{
    DBG_CTOR( EE_ParaPortion, 0 );

    pNode               = pN;
    bInvalid            = sal_True;
    bVisible            = sal_True;
    bSimple             = sal_False;
    bForceRepaint       = sal_False;
    nInvalidPosStart    = 0;
    nInvalidDiff        = 0;
    nHeight             = 0;
    nFirstLineOffset    = 0;
    nBulletX            = 0;
}

ParaPortion::~ParaPortion()
{
    DBG_DTOR( EE_ParaPortion, 0 );
}

void ParaPortion::MarkInvalid( sal_uInt16 nStart, short nDiff )
{
    if ( bInvalid == sal_False )
    {
//      nInvalidPosEnd = nStart;    // ??? => CreateLines
        nInvalidPosStart = ( nDiff >= 0 ) ? nStart : ( nStart + nDiff );
        nInvalidDiff = nDiff;
    }
    else
    {
        // Simple tap in succession
        if ( ( nDiff > 0 ) && ( nInvalidDiff > 0 ) &&
             ( ( nInvalidPosStart+nInvalidDiff ) == nStart ) )
        {
            nInvalidDiff = nInvalidDiff + nDiff;
        }
        // Simple delete in succession
        else if ( ( nDiff < 0 ) && ( nInvalidDiff < 0 ) && ( nInvalidPosStart == nStart ) )
        {
            nInvalidPosStart = nInvalidPosStart + nDiff;
            nInvalidDiff = nInvalidDiff + nDiff;
        }
        else
        {
//          nInvalidPosEnd = pNode->Len();
            DBG_ASSERT( ( nDiff >= 0 ) || ( (nStart+nDiff) >= 0 ), "MarkInvalid: Diff out of Range" );
            nInvalidPosStart = Min( nInvalidPosStart, (sal_uInt16) ( nDiff < 0 ? nStart+nDiff : nDiff ) );
            nInvalidDiff = 0;
            bSimple = sal_False;
        }
    }
    bInvalid = sal_True;
    aScriptInfos.clear();
    aWritingDirectionInfos.clear();
}

void ParaPortion::MarkSelectionInvalid( sal_uInt16 nStart, sal_uInt16 /* nEnd */ )
{
    if ( bInvalid == sal_False )
    {
        nInvalidPosStart = nStart;
//      nInvalidPosEnd = nEnd;
    }
    else
    {
        nInvalidPosStart = Min( nInvalidPosStart, nStart );
//      nInvalidPosEnd = pNode->Len();
    }
    nInvalidDiff = 0;
    bInvalid = sal_True;
    bSimple = sal_False;
    aScriptInfos.clear();
    aWritingDirectionInfos.clear();
}

sal_uInt16 ParaPortion::GetLineNumber( sal_uInt16 nIndex )
{
    DBG_ASSERTWARNING( aLineList.Count(), "Empty ParaPortion in GetLine!" );
    DBG_ASSERT( bVisible, "Why GetLine() on an invisible paragraph?" );

    for ( sal_uInt16 nLine = 0; nLine < aLineList.Count(); nLine++ )
    {
        if ( aLineList[nLine]->IsIn( nIndex ) )
            return nLine;
    }

    // Then it should be at the end of the last line!
    DBG_ASSERT( nIndex == aLineList[ aLineList.Count() - 1 ]->GetEnd(), "Index dead wrong!" );
    return (aLineList.Count()-1);
}

void ParaPortion::SetVisible( sal_Bool bMakeVisible )
{
    bVisible = bMakeVisible;
}

void ParaPortion::CorrectValuesBehindLastFormattedLine( sal_uInt16 nLastFormattedLine )
{
    sal_uInt16 nLines = aLineList.Count();
    DBG_ASSERT( nLines, "CorrectPortionNumbersFromLine: Empty Portion?" );
    if ( nLastFormattedLine < ( nLines - 1 ) )
    {
        const EditLine* pLastFormatted = aLineList[ nLastFormattedLine ];
        const EditLine* pUnformatted = aLineList[ nLastFormattedLine+1 ];
        short nPortionDiff = pUnformatted->GetStartPortion() - pLastFormatted->GetEndPortion();
        short nTextDiff = pUnformatted->GetStart() - pLastFormatted->GetEnd();
        nTextDiff++;    // LastFormatted->GetEnd() was included => 1 deducted too much!

        // The first unformatted must begin exactly one Portion behind the last
        // of the formatted:
        // If the modified line was split into one portion, can
        // nLastEnd > nNextStart!
        int nPDiff = -( nPortionDiff-1 );
        int nTDiff = -( nTextDiff-1 );
        if ( nPDiff || nTDiff )
        {
            for ( sal_uInt16 nL = nLastFormattedLine+1; nL < nLines; nL++ )
            {
                EditLine* pLine = aLineList[ nL ];

                pLine->GetStartPortion() = sal::static_int_cast< sal_uInt16 >(
                    pLine->GetStartPortion() + nPDiff);
                pLine->GetEndPortion() = sal::static_int_cast< sal_uInt16 >(
                    pLine->GetEndPortion() + nPDiff);

                pLine->GetStart() = sal::static_int_cast< sal_uInt16 >(
                    pLine->GetStart() + nTDiff);
                pLine->GetEnd() = sal::static_int_cast< sal_uInt16 >(
                    pLine->GetEnd() + nTDiff);

                pLine->SetValid();
            }
        }
    }
    DBG_ASSERT( aLineList[ aLineList.Count()-1 ]->GetEnd() == pNode->Len(), "CorrectLines: The end is not right!" );
}

// Shared reverse lookup acceleration pieces ...

static sal_uInt16 FastGetPos( const VoidPtr *pPtrArray, sal_uInt16 nPtrArrayLen,
                          VoidPtr pPtr, sal_uInt16 &rLastPos )
{
  // Through certain filter code-paths we do a lot of appends, which in
  // turn call GetPos - creating some N^2 nightmares. If we have a
  // non-trivially large list, do a few checks from the end first.
  if( rLastPos > 16 )
    {
      sal_uInt16 nEnd;
      if (rLastPos > nPtrArrayLen - 2)
        nEnd = nPtrArrayLen;
      else
        nEnd = rLastPos + 2;

      for( sal_uInt16 nIdx = rLastPos - 2; nIdx < nEnd; nIdx++ )
        {
          if( pPtrArray[ nIdx ] == pPtr )
            {
              rLastPos = nIdx;
              return nIdx;
            }
        }
    }
  // The world's lamest linear search from svarray ...
  for( sal_uInt16 nIdx = 0; nIdx < nPtrArrayLen; nIdx++ )
    if (pPtrArray[ nIdx ] == pPtr )
      return rLastPos = nIdx;
  return USHRT_MAX;
}

ParaPortionList::ParaPortionList() : nLastCache( 0 )
{
}

ParaPortionList::~ParaPortionList()
{
    Reset();
}

sal_uInt16 ParaPortionList::GetPos( const ParaPortionPtr &rPtr ) const
{
    return FastGetPos( reinterpret_cast<const VoidPtr *>( GetData() ),
                       Count(), static_cast<VoidPtr>( rPtr ),
                       ((ParaPortionList *)this)->nLastCache );
}

sal_uInt16 ContentList::GetPos( const ContentNodePtr &rPtr ) const
{
    return FastGetPos( reinterpret_cast<const VoidPtr *>( GetData() ),
                       Count(), static_cast<VoidPtr>( rPtr ),
                       ((ContentList *)this)->nLastCache );
}

void ParaPortionList::Reset()
{
    for ( sal_uInt16 nPortion = 0; nPortion < Count(); nPortion++ )
        delete GetObject( nPortion );
    Remove( 0, Count() );
}

long ParaPortionList::GetYOffset( ParaPortion* pPPortion )
{
    long nHeight = 0;
    for ( sal_uInt16 nPortion = 0; nPortion < Count(); nPortion++ )
    {
        ParaPortion* pTmpPortion = GetObject(nPortion);
        if ( pTmpPortion == pPPortion )
            return nHeight;
        nHeight += pTmpPortion->GetHeight();
    }
    OSL_FAIL( "GetYOffset: Portion not found" );
    return nHeight;
}

sal_uInt16 ParaPortionList::FindParagraph( long nYOffset )
{
    long nY = 0;
    for ( sal_uInt16 nPortion = 0; nPortion < Count(); nPortion++ )
    {
        nY += GetObject(nPortion)->GetHeight(); // should also be correct even in bVisible!
        if ( nY > nYOffset )
            return nPortion;
    }
    return 0xFFFF;  // Should be reachable through EE_PARA_NOT_FOUND!
}

#if OSL_DEBUG_LEVEL > 2
void ParaPortionList::DbgCheck( EditDoc& rDoc)
{
    DBG_ASSERT( Count() == rDoc.Count(), "ParaPortionList::DbgCheck() - Count() unequal!" );
    for ( sal_uInt16 i = 0; i < Count(); i++ )
    {
        DBG_ASSERT( SaveGetObject(i), "ParaPortionList::DbgCheck() - Null-Pointer in List!" );
        DBG_ASSERT( GetObject(i)->GetNode(), "ParaPortionList::DbgCheck() - Null-Pointer in List(2)!" );
        DBG_ASSERT( GetObject(i)->GetNode() == rDoc.GetObject(i), "ParaPortionList::DbgCheck() - Entries intersect!" );
    }
}
#endif

ContentAttribsInfo::ContentAttribsInfo( const SfxItemSet& rParaAttribs ) :
        aPrevParaAttribs( rParaAttribs)
{
}


void ConvertItem( SfxPoolItem& rPoolItem, MapUnit eSourceUnit, MapUnit eDestUnit )
{
    DBG_ASSERT( eSourceUnit != eDestUnit, "ConvertItem - Why?!" );

    switch ( rPoolItem.Which() )
    {
        case EE_PARA_LRSPACE:
        {
            DBG_ASSERT( rPoolItem.IsA( TYPE( SvxLRSpaceItem ) ), "ConvertItem: invalid Item!" );
            SvxLRSpaceItem& rItem = (SvxLRSpaceItem&)rPoolItem;
            rItem.SetTxtFirstLineOfst( sal::static_int_cast< short >( OutputDevice::LogicToLogic( rItem.GetTxtFirstLineOfst(), eSourceUnit, eDestUnit ) ) );
            rItem.SetTxtLeft( OutputDevice::LogicToLogic( rItem.GetTxtLeft(), eSourceUnit, eDestUnit ) );
            rItem.SetRight( OutputDevice::LogicToLogic( rItem.GetRight(), eSourceUnit, eDestUnit ) );
        }
        break;
        case EE_PARA_ULSPACE:
        {
            DBG_ASSERT( rPoolItem.IsA( TYPE( SvxULSpaceItem ) ), "ConvertItem: Invalid Item!" );
            SvxULSpaceItem& rItem = (SvxULSpaceItem&)rPoolItem;
            rItem.SetUpper( sal::static_int_cast< sal_uInt16 >( OutputDevice::LogicToLogic( rItem.GetUpper(), eSourceUnit, eDestUnit ) ) );
            rItem.SetLower( sal::static_int_cast< sal_uInt16 >( OutputDevice::LogicToLogic( rItem.GetLower(), eSourceUnit, eDestUnit ) ) );
        }
        break;
        case EE_PARA_SBL:
        {
            DBG_ASSERT( rPoolItem.IsA( TYPE( SvxLineSpacingItem ) ), "ConvertItem: Invalid Item!" );
            SvxLineSpacingItem& rItem = (SvxLineSpacingItem&)rPoolItem;
            // SetLineHeight changes also eLineSpace!
            if ( rItem.GetLineSpaceRule() == SVX_LINE_SPACE_MIN )
                rItem.SetLineHeight( sal::static_int_cast< sal_uInt16 >( OutputDevice::LogicToLogic( rItem.GetLineHeight(), eSourceUnit, eDestUnit ) ) );
        }
        break;
        case EE_PARA_TABS:
        {
            DBG_ASSERT( rPoolItem.IsA( TYPE( SvxTabStopItem ) ), "ConvertItem: Invalid Item!" );
            SvxTabStopItem& rItem = (SvxTabStopItem&)rPoolItem;
            SvxTabStopItem aNewItem( EE_PARA_TABS );
            for ( sal_uInt16 i = 0; i < rItem.Count(); i++ )
            {
                const SvxTabStop& rTab = rItem[i];
                SvxTabStop aNewStop( OutputDevice::LogicToLogic( rTab.GetTabPos(), eSourceUnit, eDestUnit ), rTab.GetAdjustment(), rTab.GetDecimal(), rTab.GetFill() );
                aNewItem.Insert( aNewStop );
            }
            rItem = aNewItem;
        }
        break;
        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            DBG_ASSERT( rPoolItem.IsA( TYPE( SvxFontHeightItem ) ), "ConvertItem: Invalid Item!" );
            SvxFontHeightItem& rItem = (SvxFontHeightItem&)rPoolItem;
            rItem.SetHeight( OutputDevice::LogicToLogic( rItem.GetHeight(), eSourceUnit, eDestUnit ) );
        }
        break;
    }
}

void ConvertAndPutItems( SfxItemSet& rDest, const SfxItemSet& rSource, const MapUnit* pSourceUnit, const MapUnit* pDestUnit )
{
    const SfxItemPool* pSourcePool = rSource.GetPool();
    const SfxItemPool* pDestPool = rDest.GetPool();

    for ( sal_uInt16 nWhich = EE_PARA_START; nWhich <= EE_CHAR_END; nWhich++ )
    {
        // If possible go through SlotID ...

        sal_uInt16 nSourceWhich = nWhich;
        sal_uInt16 nSlot = pDestPool->GetTrueSlotId( nWhich );
        if ( nSlot )
        {
            sal_uInt16 nW = pSourcePool->GetTrueWhich( nSlot );
            if ( nW )
                nSourceWhich = nW;
        }

        if ( rSource.GetItemState( nSourceWhich, sal_False ) == SFX_ITEM_ON )
        {
            MapUnit eSourceUnit = pSourceUnit ? *pSourceUnit : (MapUnit)pSourcePool->GetMetric( nSourceWhich );
            MapUnit eDestUnit = pDestUnit ? *pDestUnit : (MapUnit)pDestPool->GetMetric( nWhich );
            if ( eSourceUnit != eDestUnit )
            {
                SfxPoolItem* pItem = rSource.Get( nSourceWhich ).Clone();
//              pItem->SetWhich( nWhich );
                ConvertItem( *pItem, eSourceUnit, eDestUnit );
                rDest.Put( *pItem, nWhich );
                delete pItem;
            }
            else
            {
                rDest.Put( rSource.Get( nSourceWhich ), nWhich );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
