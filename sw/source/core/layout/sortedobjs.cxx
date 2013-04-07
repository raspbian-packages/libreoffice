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

#include <sortedobjs.hxx>

#include <algorithm>
#include <anchoredobject.hxx>
#include <fmtanchr.hxx>
#include <fmtsrnd.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <frmfmt.hxx>
#include <pam.hxx>
#include <svx/svdobj.hxx>
#include <IDocumentDrawModelAccess.hxx>


using namespace ::com::sun::star;


SwSortedObjs::SwSortedObjs()
{
}

SwSortedObjs::~SwSortedObjs()
{
}

sal_uInt32 SwSortedObjs::Count() const
{
    return maSortedObjLst.size();
}

SwAnchoredObject* SwSortedObjs::operator[]( sal_uInt32 _nIndex ) const
{
    SwAnchoredObject* pAnchoredObj = 0L;

    if ( _nIndex >= Count() )
    {
        OSL_FAIL( "<SwSortedObjs::operator[]> - index out of range" );
    }
    else
    {
        pAnchoredObj = maSortedObjLst[ _nIndex ];
    }

    return pAnchoredObj;
}

struct ObjAnchorOrder
{
    bool operator()( const SwAnchoredObject* _pListedAnchoredObj,
                     const SwAnchoredObject* _pNewAnchoredObj )
    {
        // get attributes of listed object
        const SwFrmFmt& rFmtListed = _pListedAnchoredObj->GetFrmFmt();
        const SwFmtAnchor* pAnchorListed = &(rFmtListed.GetAnchor());

        // get attributes of new object
        const SwFrmFmt& rFmtNew = _pNewAnchoredObj->GetFrmFmt();
        const SwFmtAnchor* pAnchorNew = &(rFmtNew.GetAnchor());

        // check for to-page anchored objects
        if ((pAnchorListed->GetAnchorId() == FLY_AT_PAGE) &&
            (pAnchorNew   ->GetAnchorId() != FLY_AT_PAGE))
        {
            return true;
        }
        else if ((pAnchorListed->GetAnchorId() != FLY_AT_PAGE) &&
                 (pAnchorNew   ->GetAnchorId() == FLY_AT_PAGE))
        {
            return false;
        }
        else if ((pAnchorListed->GetAnchorId() == FLY_AT_PAGE) &&
                 (pAnchorNew   ->GetAnchorId() == FLY_AT_PAGE))
        {
            return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
        }

        // Both objects aren't anchored to page.
        // Thus, check for to-fly anchored objects
        if ((pAnchorListed->GetAnchorId() == FLY_AT_FLY) &&
            (pAnchorNew   ->GetAnchorId() != FLY_AT_FLY))
        {
            return true;
        }
        else if ((pAnchorListed->GetAnchorId() != FLY_AT_FLY) &&
                 (pAnchorNew   ->GetAnchorId() == FLY_AT_FLY))
        {
            return false;
        }
        else if ((pAnchorListed->GetAnchorId() == FLY_AT_FLY) &&
                 (pAnchorNew   ->GetAnchorId() == FLY_AT_FLY))
        {
            return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
        }

        // Both objects aren't anchor to page or to fly
        // Thus, compare content anchor nodes, if existing.
        const SwPosition* pCntntAnchorListed = pAnchorListed->GetCntntAnchor();
        const SwPosition* pCntntAnchorNew = pAnchorNew->GetCntntAnchor();
        if ( pCntntAnchorListed && pCntntAnchorNew &&
             pCntntAnchorListed->nNode != pCntntAnchorNew->nNode )
        {
            return pCntntAnchorListed->nNode < pCntntAnchorNew->nNode;
        }

        // objects anchored at the same content.
        // --> OD 2006-11-29 #???# - objects have to be ordered by anchor node position
        // Thus, compare content anchor node positions and anchor type,
        // if not anchored at-paragraph
        if ((pAnchorListed->GetAnchorId() != FLY_AT_PARA) &&
            (pAnchorNew   ->GetAnchorId() != FLY_AT_PARA) &&
             pCntntAnchorListed && pCntntAnchorNew )
        {
            if ( pCntntAnchorListed->nContent != pCntntAnchorNew->nContent )
            {
                return pCntntAnchorListed->nContent < pCntntAnchorNew->nContent;
            }
            else if ((pAnchorListed->GetAnchorId() == FLY_AT_CHAR) &&
                     (pAnchorNew   ->GetAnchorId() == FLY_AS_CHAR))
            {
                return true;
            }
            else if ((pAnchorListed->GetAnchorId() == FLY_AS_CHAR) &&
                     (pAnchorNew   ->GetAnchorId() == FLY_AT_CHAR))
            {
                return false;
            }
        }

        // objects anchored at the same content and at the same content anchor
        // node position with the same anchor type
        // Thus, compare its wrapping style including its layer
        const IDocumentDrawModelAccess* pIDDMA = rFmtListed.getIDocumentDrawModelAccess();
        const SdrLayerID nHellId = pIDDMA->GetHellId();
        const SdrLayerID nInvisibleHellId = pIDDMA->GetInvisibleHellId();
        const bool bWrapThroughOrHellListed =
                    rFmtListed.GetSurround().GetSurround() == SURROUND_THROUGHT ||
                    _pListedAnchoredObj->GetDrawObj()->GetLayer() == nHellId ||
                    _pListedAnchoredObj->GetDrawObj()->GetLayer() == nInvisibleHellId;
        const bool bWrapThroughOrHellNew =
                    rFmtNew.GetSurround().GetSurround() == SURROUND_THROUGHT ||
                    _pNewAnchoredObj->GetDrawObj()->GetLayer() == nHellId ||
                    _pNewAnchoredObj->GetDrawObj()->GetLayer() == nInvisibleHellId;
        if ( bWrapThroughOrHellListed != bWrapThroughOrHellNew )
        {
            if ( bWrapThroughOrHellListed )
                return false;
            else
                return true;
        }
        else if ( bWrapThroughOrHellListed && bWrapThroughOrHellNew )
        {
            return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
        }

        // objects anchored at the same content with a set text wrapping
        // Thus, compare wrap influences on object position
        const SwFmtWrapInfluenceOnObjPos* pWrapInfluenceOnObjPosListed =
                                        &(rFmtListed.GetWrapInfluenceOnObjPos());
        const SwFmtWrapInfluenceOnObjPos* pWrapInfluenceOnObjPosNew =
                                        &(rFmtNew.GetWrapInfluenceOnObjPos());
        // #i35017# - handle ITERATIVE as ONCE_SUCCESSIVE
        if ( pWrapInfluenceOnObjPosListed->GetWrapInfluenceOnObjPos( true ) !=
                pWrapInfluenceOnObjPosNew->GetWrapInfluenceOnObjPos( true ) )
        {
            // #i35017# - constant name has changed
            if ( pWrapInfluenceOnObjPosListed->GetWrapInfluenceOnObjPos( true )
                            == text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE )
                return true;
            else
                return false;
        }

        // objects anchored at the same content position/page/fly with same
        // wrap influence.
        // Thus, compare anchor order number
        return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
    }
};

bool SwSortedObjs::Insert( SwAnchoredObject& _rAnchoredObj )
{
    // #i51941#
    if ( Contains( _rAnchoredObj ) )
    {
        // list already contains object
        OSL_FAIL( "<SwSortedObjs::Insert()> - already contains object" );
        return true;
    }

    // find insert position
    std::vector< SwAnchoredObject* >::iterator aInsPosIter =
        std::lower_bound( maSortedObjLst.begin(), maSortedObjLst.end(),
                          &_rAnchoredObj, ObjAnchorOrder() );

    // insert object into list
    maSortedObjLst.insert( aInsPosIter, &_rAnchoredObj );

    return Contains( _rAnchoredObj );
}

bool SwSortedObjs::Remove( SwAnchoredObject& _rAnchoredObj )
{
    bool bRet = true;

    std::vector< SwAnchoredObject* >::iterator aDelPosIter =
        std::find( maSortedObjLst.begin(), maSortedObjLst.end(), &_rAnchoredObj );

    if ( aDelPosIter == maSortedObjLst.end() )
    {
        // object not found.
        bRet = false;
        OSL_FAIL( "<SwSortedObjs::Remove()> - object not found" );
    }
    else
    {
        maSortedObjLst.erase( aDelPosIter );
    }

    return bRet;
}

bool SwSortedObjs::Contains( const SwAnchoredObject& _rAnchoredObj ) const
{
    std::vector< SwAnchoredObject* >::const_iterator aIter =
        std::find( maSortedObjLst.begin(), maSortedObjLst.end(), &_rAnchoredObj );

    return aIter != maSortedObjLst.end();
}

bool SwSortedObjs::Update( SwAnchoredObject& _rAnchoredObj )
{
    if ( !Contains( _rAnchoredObj ) )
    {
        // given anchored object not found in list
        OSL_FAIL( "<SwSortedObjs::Update(..) - sorted list doesn't contain given anchored object" );
        return false;
    }

    if ( Count() == 1 )
    {
        // given anchored object is the only one in the list.
        return true;
    }

    Remove( _rAnchoredObj );
    Insert( _rAnchoredObj );

    return Contains( _rAnchoredObj );
}

sal_uInt32 SwSortedObjs::ListPosOf( const SwAnchoredObject& _rAnchoredObj ) const
{
    sal_uInt32 nRetLstPos = Count();

    std::vector< SwAnchoredObject* >::const_iterator aIter =
        std::find( maSortedObjLst.begin(), maSortedObjLst.end(), &_rAnchoredObj );

    if ( aIter != maSortedObjLst.end() )
    {
        // #i51941#
//        nRetLstPos = aIter - maSortedObjLst.begin();
        std::vector< SwAnchoredObject* >::difference_type nPos =
                                                aIter - maSortedObjLst.begin();
        nRetLstPos = sal_uInt32( nPos );
    }

    return nRetLstPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
