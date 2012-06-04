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


#include <swcache.hxx>
#include <rtl/strbuf.hxx>

SV_IMPL_PTRARR(SwCacheObjArr,SwCacheObj*);

#ifdef DBG_UTIL
#define INCREMENT( nVar )   ++nVar
#else
#define INCREMENT( nVar )
#endif

/*************************************************************************
|*  SwCache::Check()
|*************************************************************************/

#ifdef DBG_UTIL
void SwCache::Check()
{
    if ( !pRealFirst )
        return;

    //Konsistenspruefung.
    OSL_ENSURE( !pLast->GetNext(), "Last but not last." );
    OSL_ENSURE( !pRealFirst->GetPrev(), "First but not first." );
    sal_uInt16 nCnt = 0;
    sal_Bool bFirstFound = sal_False;
    SwCacheObj *pObj = pRealFirst;
    SwCacheObj *pRekursive = pObj;
    while ( pObj )
    {
        //Das Objekt muss auch auf dem Rueckwaertsweg gefunden werden.
        SwCacheObj *pTmp = pLast;
        while ( pTmp && pTmp != pObj )
            pTmp = pTmp->GetPrev();
        OSL_ENSURE( pTmp, "Objekt not found." );

        ++nCnt;
        if ( pObj == pFirst )
            bFirstFound = sal_True;
        if ( !pObj->GetNext() )
            OSL_ENSURE( pObj == pLast, "Last not Found." );
        pObj = pObj->GetNext();
        OSL_ENSURE( pObj != pRekursive, "Recursion in SwCache." );
    }
    OSL_ENSURE( bFirstFound, "First not Found." );
    OSL_ENSURE( (nCnt + aFreePositions.size()) == Count(), "Lost Chain." );
    OSL_ENSURE(
        Count() != nCurMax || nCurMax == aFreePositions.size() + nCnt,
        "Lost FreePositions." );
}
#endif

#ifdef DBG_UTIL
#define CHECK Check();
#else
#define CHECK
#endif

/*************************************************************************
|*  SwCache::SwCache(), ~SwCache()
|*************************************************************************/


SwCache::SwCache( const sal_uInt16 nInitSize, const sal_uInt16 nGrowSize
#ifdef DBG_UTIL
    , const rtl::OString &rNm
#endif
    ) :
    SwCacheObjArr( (sal_uInt8)nInitSize, (sal_uInt8)nGrowSize ),
    pRealFirst( 0 ),
    pFirst( 0 ),
    pLast( 0 ),
    nMax( nInitSize ),
    nCurMax( nInitSize )
#ifdef DBG_UTIL
    , m_aName( rNm )
    , m_nAppend( 0 )
    , m_nInsertFree( 0 )
    , m_nReplace( 0 )
    , m_nGetSuccess( 0 )
    , m_nGetFail( 0 )
    , m_nToTop( 0 )
    , m_nDelete( 0 )
    , m_nGetSeek( 0 )
    , m_nAverageSeekCnt( 0 )
    , m_nFlushCnt( 0 )
    , m_nFlushedObjects( 0 )
    , m_nIncreaseMax( 0 )
    , m_nDecreaseMax( 0 )
#endif
{
}

#ifdef DBG_UTIL
SwCache::~SwCache()
{
    {
        rtl::OStringBuffer sOut(m_aName);

        sOut.append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of new entries:                 ")).
            append(static_cast<sal_Int32>(m_nAppend)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of insert on free places:       ")).
            append(static_cast<sal_Int32>(m_nInsertFree)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of replacements:                ")).
            append(static_cast<sal_Int32>(m_nReplace)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of successful Get's:            ")).
            append(static_cast<sal_Int32>(m_nGetSuccess)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of failed Get's:                ")).
            append(static_cast<sal_Int32>(m_nGetFail)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number or reordering (LRU):            ")).
            append(static_cast<sal_Int32>(m_nToTop)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of suppressions:                ")).
            append(static_cast<sal_Int32>(m_nDelete)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of Get's without Index:         ")).
            append(static_cast<sal_Int32>(m_nGetSeek)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of Seek for Get without Index:  ")).
            append(static_cast<sal_Int32>(m_nAverageSeekCnt)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of Flush calls:                 " )).
            append(static_cast<sal_Int32>(m_nFlushCnt)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of flushed objects:             ")).
            append(static_cast<sal_Int32>(m_nFlushedObjects)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of Cache expansions:            ")).
            append(static_cast<sal_Int32>(m_nIncreaseMax)).
            append('\n').
            append(RTL_CONSTASCII_STRINGPARAM(
                "Number of Cache reductions:            ")).
            append(static_cast<sal_Int32>(m_nDecreaseMax)).
            append('\n');

        OSL_TRACE(sOut.getStr());
    }
    Check();
}
#endif

/*************************************************************************
|*  SwCache::Flush()
|*************************************************************************/


void SwCache::Flush( const sal_uInt8 )
{
    INCREMENT( m_nFlushCnt );
    SwCacheObj *pObj = pRealFirst;
    pRealFirst = pFirst = pLast = 0;
    SwCacheObj *pTmp;
    while ( pObj )
    {
#ifdef DBG_UTIL
        if ( pObj->IsLocked() )
        {
            OSL_FAIL( "Flushing locked objects." );
            if ( !pRealFirst )
            {
                pRealFirst = pFirst = pLast = pObj;
                pTmp = pObj->GetNext();
                pObj->SetNext( 0 ); pObj->SetPrev( 0 );
                pObj = pTmp;
            }
            else
            {   pLast->SetNext( pObj );
                pObj->SetPrev( pLast );
                pLast = pObj;
                pTmp = pObj->GetNext();
                pObj->SetNext( 0 );
                pObj = pTmp;
            }
        }
        else
#endif
        {
            pTmp = (SwCacheObj*)pObj;
            pObj = pTmp->GetNext();
            aFreePositions.push_back( pTmp->GetCachePos() );
            *(pData + pTmp->GetCachePos()) = (void*)0;
            delete pTmp;
            INCREMENT( m_nFlushedObjects );
        }
    }
}

/*************************************************************************
|*  SwCache::ToTop()
|*************************************************************************/


void SwCache::ToTop( SwCacheObj *pObj )
{
    INCREMENT( m_nToTop );

    //Objekt aus der LRU-Kette ausschneiden und am Anfang einfuegen.
    if ( pRealFirst == pObj )   //pFirst wurde vom Aufrufer geprueft!
    {   CHECK;
        return;
    }

    if ( !pRealFirst )
    {   //Der erste wird eingetragen.
        OSL_ENSURE( !pFirst && !pLast, "First not first." );
        pRealFirst = pFirst = pLast = pObj;
        CHECK;
        return;
    }

    //Ausschneiden.
    if ( pObj == pLast )
    {
        OSL_ENSURE( pObj->GetPrev(), "Last but no Prev." );
        pLast = pObj->GetPrev();
        pLast->SetNext( 0 );
    }
    else
    {
        if ( pObj->GetNext() )
            pObj->GetNext()->SetPrev( pObj->GetPrev() );
        if ( pObj->GetPrev() )
            pObj->GetPrev()->SetNext( pObj->GetNext() );
    }

    //Am (virtuellen) Anfang einfuegen.
    if ( pRealFirst == pFirst )
    {
        pRealFirst->SetPrev( pObj );
        pObj->SetNext( pRealFirst );
        pObj->SetPrev( 0 );
        pRealFirst = pFirst = pObj;
        CHECK;
    }
    else
    {
        OSL_ENSURE( pFirst, "ToTop, First ist not RealFirst an Empty." );

        if ( pFirst->GetPrev() )
        {
            pFirst->GetPrev()->SetNext( pObj );
            pObj->SetPrev( pFirst->GetPrev() );
        }
        else
            pObj->SetPrev( 0 );
        pFirst->SetPrev( pObj );
        pObj->SetNext( pFirst );
        pFirst = pObj;
        CHECK;
    }
}

/*************************************************************************
|*  SwCache::Get()
|*************************************************************************/


SwCacheObj *SwCache::Get( const void *pOwner, const sal_uInt16 nIndex,
                          const sal_Bool bToTop )
{
    SwCacheObj *pRet;
    if ( 0 != (pRet = nIndex < Count() ? operator[]( nIndex ) : 0) )
    {
        if ( !pRet->IsOwner( pOwner ) )
            pRet = 0;
        else if ( bToTop && pRet != pFirst )
            ToTop( pRet );
    }

#ifdef DBG_UTIL
        if ( pRet )
            ++m_nGetSuccess;
        else
            ++m_nGetFail;
#endif

    return pRet;
}



SwCacheObj *SwCache::Get( const void *pOwner, const sal_Bool bToTop )
{
    SwCacheObj *pRet = pRealFirst;
    while ( pRet && !pRet->IsOwner( pOwner ) )
    {
        INCREMENT( m_nAverageSeekCnt );
        pRet = pRet->GetNext();
    }

    if ( bToTop && pRet && pRet != pFirst )
        ToTop( pRet );

#ifdef DBG_UTIL
    if ( pRet )
        ++m_nGetSuccess;
    else
        ++m_nGetFail;
    ++m_nGetSeek;
#endif
    return pRet;
}

/*************************************************************************
|*  SwCache::Delete()
|*************************************************************************/


void SwCache::DeleteObj( SwCacheObj *pObj )
{
    CHECK;
    OSL_ENSURE( !pObj->IsLocked(), "SwCache::Delete: Object ist Locked." );
    if ( pObj->IsLocked() )
        return;

    if ( pFirst == pObj )
    {
        if ( pFirst->GetNext() )
            pFirst = pFirst->GetNext();
        else
            pFirst = pFirst->GetPrev();
    }
    if ( pRealFirst == pObj )
        pRealFirst = pRealFirst->GetNext();
    if ( pLast == pObj )
        pLast = pLast->GetPrev();
    if ( pObj->GetPrev() )
        pObj->GetPrev()->SetNext( pObj->GetNext() );
    if ( pObj->GetNext() )
        pObj->GetNext()->SetPrev( pObj->GetPrev() );

    aFreePositions.push_back( pObj->GetCachePos() );
    *(pData + pObj->GetCachePos()) = (void*)0;
    delete pObj;

    CHECK;
    if ( Count() > nCurMax &&
         (nCurMax <= (Count() - aFreePositions.size())) )
    {
        //Falls moeglich wieder verkleinern, dazu muessen allerdings ausreichend
        //Freie Positionen bereitstehen.
        //Unangenehmer Nebeneffekt ist, das die Positionen verschoben werden
        //muessen, und die Eigentuemer der Objekte diese wahrscheinlich nicht
        //wiederfinden werden.
        for ( sal_uInt16 i = 0; i < Count(); ++i )
        {
            SwCacheObj *pTmpObj = operator[](i);
            if ( !pTmpObj )
            {   SwCacheObjArr::Remove( i, 1 );
                --i;
            }
            else
                pTmpObj->SetCachePos( i );
        }
        aFreePositions.clear();
    }
    CHECK;
}

void SwCache::Delete( const void *pOwner )
{
    INCREMENT( m_nDelete );
    SwCacheObj *pObj;
    if ( 0 != (pObj = Get( pOwner, sal_Bool(sal_False) )) )
        DeleteObj( pObj );
}


/*************************************************************************
|*  SwCache::Insert()
|*************************************************************************/


sal_Bool SwCache::Insert( SwCacheObj *pNew )
{
    CHECK;
    OSL_ENSURE( !pNew->GetPrev() && !pNew->GetNext(), "New but not new." );

    sal_uInt16 nPos;//Wird hinter den if's zum setzen am Obj benutzt.
    if ( Count() < nCurMax )
    {
        //Es ist noch Platz frei, also einfach einfuegen.
        INCREMENT( m_nAppend );
        nPos = Count();
        SwCacheObjArr::C40_INSERT( SwCacheObj, pNew, nPos );
    }
    else if ( !aFreePositions.empty() )
    {
        //Es exitieren Platzhalter, also den letzten benutzen.
        INCREMENT( m_nInsertFree );
        const sal_uInt16 nFreePos = aFreePositions.size() - 1;
        nPos = aFreePositions[ nFreePos ];
        *(pData + nPos) = pNew;
        aFreePositions.erase( aFreePositions.begin() + nFreePos );
    }
    else
    {
        INCREMENT( m_nReplace );
        //Der letzte des LRU fliegt raus.
        SwCacheObj *pObj = pLast;

        while ( pObj && pObj->IsLocked() )
            pObj = pObj->GetPrev();
        if ( !pObj )
        {
            OSL_FAIL( "Cache overflow." );
            return sal_False;
        }

        nPos = pObj->GetCachePos();
        if ( pObj == pLast )
        { OSL_ENSURE( pObj->GetPrev(), "Last but no Prev" );
            pLast = pObj->GetPrev();
            pLast->SetNext( 0 );
        }
        else
        {
            if ( pObj->GetPrev() )
                pObj->GetPrev()->SetNext( pObj->GetNext() );
            if ( pObj->GetNext() )
                pObj->GetNext()->SetPrev( pObj->GetPrev() );
        }
        delete pObj;
        *(pData + nPos) = pNew;
    }
    pNew->SetCachePos( nPos );

    //Anstelle von ToTop, einfach als pFirst einfuegen.
//  ToTop( nPos );
    if ( pFirst )
    {
        if ( pFirst->GetPrev() )
        {   pFirst->GetPrev()->SetNext( pNew );
            pNew->SetPrev( pFirst->GetPrev() );
        }
        pFirst->SetPrev( pNew );
        pNew->SetNext( pFirst );
    }
    else
    { OSL_ENSURE( !pLast, "Last but no First." );
        pLast = pNew;
    }
    if ( pFirst == pRealFirst )
        pRealFirst = pNew;
    pFirst = pNew;

    CHECK;
    return sal_True;
}

/*************************************************************************
|*  SwCache::SetLRUOfst()
|*************************************************************************/


void SwCache::SetLRUOfst( const sal_uInt16 nOfst )
{
    if ( !pRealFirst || ((Count() - aFreePositions.size()) < nOfst) )
        return;

    CHECK;
    pFirst = pRealFirst;
    for ( sal_uInt16 i = 0; i < Count() && i < nOfst; ++i )
    {
        if ( pFirst->GetNext() && pFirst->GetNext()->GetNext() )
            pFirst = pFirst->GetNext();
        else
            break;
    }
    CHECK;
}

/*************************************************************************
|*  SwCacheObj::SwCacheObj()
|*************************************************************************/


SwCacheObj::SwCacheObj( const void *pOwn ) :
    pNext( 0 ),
    pPrev( 0 ),
    nCachePos( USHRT_MAX ),
    nLock( 0 ),
    pOwner( pOwn )
{
}



SwCacheObj::~SwCacheObj()
{
}

/*************************************************************************
|*  SwCacheObj::SetLock(), Unlock()
|*************************************************************************/

#ifdef DBG_UTIL
void SwCacheObj::Lock()
{
    OSL_ENSURE( nLock < UCHAR_MAX, "Too many Locks for CacheObject." );
    ++nLock;
}

void SwCacheObj::Unlock()
{
    OSL_ENSURE( nLock, "No more Locks available." );
    --nLock;
}
#endif


SwCacheAccess::~SwCacheAccess()
{
    if ( pObj )
        pObj->Unlock();
}

/*************************************************************************
|*  SwCacheAccess::Get()
|*************************************************************************/

void SwCacheAccess::_Get()
{
    OSL_ENSURE( !pObj, "SwCacheAcces Obj already available." );

    pObj = NewObj();
    if ( !rCache.Insert( pObj ) )
    {
        delete pObj;
        pObj = 0;
    }
    else
        pObj->Lock();
}

/*************************************************************************
|*  SwCacheAccess::IsAvailable()
|*************************************************************************/

sal_Bool SwCacheAccess::IsAvailable() const
{
    return pObj != 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
