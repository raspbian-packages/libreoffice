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

#include <tools/stream.hxx>

#include <editobj2.hxx>
#include <editeng/editdata.hxx>
#include <editattr.hxx>
#include <editeng/editeng.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/cscoitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/bulitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/brshitem.hxx>
#include <vcl/graph.hxx>
#include <svl/intitem.hxx>
#include <unotools/fontcvt.hxx>
#include <tools/tenccvt.hxx>

DBG_NAME( EE_EditTextObject )
DBG_NAME( XEditAttribute )

//--------------------------------------------------------------

XEditAttribute* MakeXEditAttribute( SfxItemPool& rPool, const SfxPoolItem& rItem, sal_uInt16 nStart, sal_uInt16 nEnd )
{
    // Create thw new attribute in the pool
    const SfxPoolItem& rNew = rPool.Put( rItem );

    XEditAttribute* pNew = new XEditAttribute( rNew, nStart, nEnd );
    return pNew;
}

XEditAttribute::XEditAttribute( const SfxPoolItem& rAttr, sal_uInt16 nS, sal_uInt16 nE )
{
    DBG_CTOR( XEditAttribute, 0 );
    pItem = &rAttr;
    nStart = nS;
    nEnd = nE;
}

XEditAttribute::~XEditAttribute()
{
    DBG_DTOR( XEditAttribute, 0 );
    pItem = 0;  // belongs to the Pool.
}

XEditAttribute* XEditAttributeList::FindAttrib( sal_uInt16 _nWhich, sal_uInt16 nChar ) const
{
    for ( sal_uInt16 n = Count(); n; )
    {
        XEditAttribute* pAttr = GetObject( --n );
        if( ( pAttr->GetItem()->Which() == _nWhich ) && ( pAttr->GetStart() <= nChar ) && ( pAttr->GetEnd() > nChar ) )
            return pAttr;
    }
    return NULL;
}

ContentInfo::ContentInfo( SfxItemPool& rPool ) : aParaAttribs( rPool, EE_PARA_START, EE_CHAR_END )
{
    eFamily = SFX_STYLE_FAMILY_PARA;
    pWrongs = NULL;
}

// the real Copy constructor is nonsens, since I have to work with another Pool!
ContentInfo::ContentInfo( const ContentInfo& rCopyFrom, SfxItemPool& rPoolToUse )
    : aParaAttribs( rPoolToUse, EE_PARA_START, EE_CHAR_END )
    , pWrongs(0)
{
    // this should ensure that the Items end up in the correct Pool!
    aParaAttribs.Set( rCopyFrom.GetParaAttribs() );
    aText = rCopyFrom.GetText();
    aStyle = rCopyFrom.GetStyle();
    eFamily = rCopyFrom.GetFamily();

    for ( sal_uInt16 n = 0; n < rCopyFrom.GetAttribs().Count(); n++  )
    {
        XEditAttribute* pAttr = rCopyFrom.GetAttribs().GetObject( n );
        XEditAttribute* pMyAttr = MakeXEditAttribute( rPoolToUse, *pAttr->GetItem(), pAttr->GetStart(), pAttr->GetEnd() );
        aAttribs.Insert( pMyAttr, aAttribs.Count()  );
    }

    if ( rCopyFrom.GetWrongList() )
        pWrongs = rCopyFrom.GetWrongList()->Clone();
}

ContentInfo::~ContentInfo()
{
    for ( sal_uInt16 nAttr = 0; nAttr < aAttribs.Count(); nAttr++ )
    {
        XEditAttribute* pAttr = aAttribs.GetObject(nAttr);
        aParaAttribs.GetPool()->Remove( *pAttr->GetItem() );
        delete pAttr;
    }
    aAttribs.Remove( 0, aAttribs.Count() );
    delete pWrongs;
}

// #i102062#
bool ContentInfo::isWrongListEqual(const ContentInfo& rCompare) const
{
    if(GetWrongList() == rCompare.GetWrongList())
        return true;

    if(!GetWrongList() || !rCompare.GetWrongList())
        return false;

    return (*GetWrongList() == *rCompare.GetWrongList());
}

bool ContentInfo::operator==( const ContentInfo& rCompare ) const
{
    if( (aText == rCompare.aText) &&
            (aStyle == rCompare.aStyle ) &&
            (aAttribs.Count() == rCompare.aAttribs.Count() ) &&
            (eFamily == rCompare.eFamily ) &&
            (aParaAttribs == rCompare.aParaAttribs ) )
    {
        const sal_uInt16 nCount = aAttribs.Count();
        if( nCount == rCompare.aAttribs.Count() )
        {
            sal_uInt16 n;
            for( n = 0; n < nCount; n++ )
            {
                if( !(*aAttribs.GetObject(n) == *rCompare.aAttribs.GetObject(n)) )
                    return false;
            }

            return true;
        }
    }

    return false;
}

EditTextObject::EditTextObject( sal_uInt16 n)
{
    DBG_CTOR( EE_EditTextObject, 0 );
    nWhich = n;
}

EditTextObject::EditTextObject( const EditTextObject& r )
{
    DBG_CTOR( EE_EditTextObject, 0 );
    nWhich = r.nWhich;
}

EditTextObject::~EditTextObject()
{
    DBG_DTOR( EE_EditTextObject, 0 );
}

sal_uInt16 EditTextObject::GetParagraphCount() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return 0;
}

XubString EditTextObject::GetText( sal_uInt16 /* nParagraph */ ) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return XubString();
}

void EditTextObject::Insert( const EditTextObject& /* rObj */, sal_uInt16 /* nPara */)
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

EditTextObject* EditTextObject::CreateTextObject( sal_uInt16 /*nPara*/, sal_uInt16 /*nParas*/ ) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return 0;
}

void EditTextObject::RemoveParagraph( sal_uInt16 /*nPara*/ )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_Bool EditTextObject::HasPortionInfo() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return sal_False;
}

void EditTextObject::ClearPortionInfo()
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_Bool EditTextObject::HasOnlineSpellErrors() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return sal_False;
}

sal_Bool EditTextObject::HasCharAttribs( sal_uInt16 ) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return sal_False;
}

void EditTextObject::GetCharAttribs( sal_uInt16 /*nPara*/, EECharAttribArray& /*rLst*/ ) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

void EditTextObject::MergeParaAttribs( const SfxItemSet& /*rAttribs*/, sal_uInt16 /*nStart*/, sal_uInt16 /*nEnd*/ )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_Bool EditTextObject::IsFieldObject() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return sal_False;
}

const SvxFieldItem* EditTextObject::GetField() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return 0;
}

sal_Bool EditTextObject::HasField( TypeId /*aType*/ ) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return sal_False;
}

SfxItemSet EditTextObject::GetParaAttribs( sal_uInt16 /*nPara*/ ) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return SfxItemSet( *(SfxItemPool*)NULL );
}

void EditTextObject::SetParaAttribs( sal_uInt16 /*nPara*/, const SfxItemSet& /*rAttribs*/ )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_Bool EditTextObject::RemoveCharAttribs( sal_uInt16 /*nWhich*/ )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return sal_False;
}

sal_Bool EditTextObject::RemoveParaAttribs( sal_uInt16 /*nWhich*/ )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return sal_False;
}

sal_Bool EditTextObject::HasStyleSheet( const XubString& /*rName*/, SfxStyleFamily /*eFamily*/ ) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return sal_False;
}

void EditTextObject::GetStyleSheet( sal_uInt16 /*nPara*/, XubString& /*rName*/, SfxStyleFamily& /*eFamily*/ ) const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

void EditTextObject::SetStyleSheet( sal_uInt16 /*nPara*/, const XubString& /*rName*/, const SfxStyleFamily& /*eFamily*/ )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_Bool EditTextObject::ChangeStyleSheets( const XubString&, SfxStyleFamily,
                                            const XubString&, SfxStyleFamily )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return sal_False;
}

void EditTextObject::ChangeStyleSheetName( SfxStyleFamily /*eFamily*/,
                const XubString& /*rOldName*/, const XubString& /*rNewName*/ )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_uInt16 EditTextObject::GetUserType() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return 0;
}

void EditTextObject::SetUserType( sal_uInt16 )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_uLong EditTextObject::GetObjectSettings() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return 0;
}

void EditTextObject::SetObjectSettings( sal_uLong )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
}

sal_Bool EditTextObject::IsVertical() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return sal_False;
}

void EditTextObject::SetVertical( sal_Bool bVertical )
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    ((BinTextObject*)this)->SetVertical( bVertical );
}

sal_uInt16 EditTextObject::GetScriptType() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return ((const BinTextObject*)this)->GetScriptType();
}


sal_Bool EditTextObject::Store( SvStream& rOStream ) const
{
    if ( rOStream.GetError() )
        return sal_False;

    sal_Size nStartPos = rOStream.Tell();

    rOStream << (sal_uInt16)Which();

    sal_uInt32 nStructSz = 0;
    rOStream << nStructSz;

    StoreData( rOStream );

    sal_Size nEndPos = rOStream.Tell();
    nStructSz = nEndPos - nStartPos - sizeof( nWhich ) - sizeof( nStructSz );
    rOStream.Seek( nStartPos + sizeof( nWhich ) );
    rOStream << nStructSz;
    rOStream.Seek( nEndPos );

    return rOStream.GetError() ? sal_False : sal_True;
}

EditTextObject* EditTextObject::Create( SvStream& rIStream, SfxItemPool* pGlobalTextObjectPool )
{
    sal_uLong nStartPos = rIStream.Tell();

    // First check what type of Object...
    sal_uInt16 nWhich;
    rIStream >> nWhich;

    sal_uInt32 nStructSz;
    rIStream >> nStructSz;

    DBG_ASSERT( ( nWhich == 0x22 /*EE_FORMAT_BIN300*/ ) || ( nWhich == EE_FORMAT_BIN ), "CreateTextObject: Unknown Object!" );

    if ( rIStream.GetError() )
        return NULL;

    EditTextObject* pTxtObj = NULL;
    switch ( nWhich )
    {
        case 0x22 /*BIN300*/:       pTxtObj = new BinTextObject( 0 );
                                    ((BinTextObject*)pTxtObj)->CreateData300( rIStream );
                                    break;
        case EE_FORMAT_BIN:         pTxtObj = new BinTextObject( pGlobalTextObjectPool );
                                    pTxtObj->CreateData( rIStream );
                                    break;
        default:
        {
            // If I do not know the format, I overwrite the contents:
            rIStream.SetError( EE_READWRITE_WRONGFORMAT );
        }
    }

    // Make sure that the stream is left at the correct place.
    sal_Size nFullSz = sizeof( nWhich ) + sizeof( nStructSz ) + nStructSz;
    rIStream.Seek( nStartPos + nFullSz );
    return pTxtObj;
}

void EditTextObject::StoreData( SvStream& ) const
{
    OSL_FAIL( "StoreData: Base class!" );
}

void EditTextObject::CreateData( SvStream& )
{
    OSL_FAIL( "CreateData: Base class!" );
}

sal_uInt16 EditTextObject::GetVersion() const
{
    OSL_FAIL( "Virtual method direct from EditTextObject!" );
    return 0;
}

bool EditTextObject::operator==( const EditTextObject& rCompare ) const
{
    return static_cast< const BinTextObject* >( this )->operator==( static_cast< const BinTextObject& >( rCompare ) );
}

// #i102062#
bool EditTextObject::isWrongListEqual(const EditTextObject& rCompare) const
{
    return static_cast< const BinTextObject* >(this)->isWrongListEqual(static_cast< const BinTextObject& >(rCompare));
}

// from SfxItemPoolUser
void BinTextObject::ObjectInDestruction(const SfxItemPool& rSfxItemPool)
{
    if(!bOwnerOfPool && pPool && pPool == &rSfxItemPool)
    {
        // The pool we are based on gets destructed; get owner of pool by creating own one.
        // No need to call RemoveSfxItemPoolUser(), this is done from the pool's destructor
        // Base new pool on EditEnginePool; it would also be possible to clone the used
        // pool if needed, but only text attributes should be used.
        SfxItemPool* pNewPool = EditEngine::CreatePool();

        if(pPool)
        {
            pNewPool->SetDefaultMetric(pPool->GetMetric(DEF_METRIC));
        }

        for(sal_uInt16 n(0); n < aContents.Count(); n++)
        {
            // clone ContentInfos for new pool
            ContentInfo* pOrg = aContents.GetObject(n);
            DBG_ASSERT(pOrg, "NULL-Pointer in ContentList!");

            ContentInfo* pNew = new ContentInfo(*pOrg, *pNewPool);
            aContents.Replace(pNew, n);
            delete pOrg;
        }

        // set local variables
        pPool = pNewPool;
        bOwnerOfPool = sal_True;
    }
}

EditEngineItemPool* getEditEngineItemPool(SfxItemPool* pPool)
{
    EditEngineItemPool* pRetval = dynamic_cast< EditEngineItemPool* >(pPool);

    while(!pRetval && pPool && pPool->GetSecondaryPool())
    {
        pPool = pPool->GetSecondaryPool();

        if(pPool)
        {
            pRetval = dynamic_cast< EditEngineItemPool* >(pPool);
        }
    }

    return pRetval;
}

BinTextObject::BinTextObject( SfxItemPool* pP ) :
    EditTextObject( EE_FORMAT_BIN ),
    SfxItemPoolUser()
{
    nVersion = 0;
    nMetric = 0xFFFF;
    nUserType = 0;
    nObjSettings = 0;
    pPortionInfo = 0;

    // #i101239# ensure target is a EditEngineItemPool, else
    // fallback to pool ownership. This is needed to ensure that at
    // pool destruction time of an alien pool, the pool is still alive.
    // When registering would happen at an alien pool which just uses an
    // EditEngineItemPool as some sub-pool, that pool could already
    // be decoupled and deleted whcih would lead to crashes.
    pPool = getEditEngineItemPool(pP);

    if ( pPool )
    {
        bOwnerOfPool = sal_False;
    }
    else
    {
        pPool = EditEngine::CreatePool();
        bOwnerOfPool =  sal_True;
    }

    if(!bOwnerOfPool && pPool)
    {
        // it is sure now that the pool is an EditEngineItemPool
        pPool->AddSfxItemPoolUser(*this);
    }

    bVertical = sal_False;
    bStoreUnicodeStrings = sal_False;
    nScriptType = 0;
}

BinTextObject::BinTextObject( const BinTextObject& r ) :
    EditTextObject( r ),
    SfxItemPoolUser()
{
    nVersion = r.nVersion;
    nMetric = r.nMetric;
    nUserType = r.nUserType;
    nObjSettings = r.nObjSettings;
    bVertical = r.bVertical;
    nScriptType = r.nScriptType;
    pPortionInfo = NULL;    // Do not copy PortionInfo
    bStoreUnicodeStrings = sal_False;

    if ( !r.bOwnerOfPool )
    {
        // reuse alien pool; this must be a EditEngineItemPool
        // since there is no other way to construct a BinTextObject
        // than it's regular constructor where that is ensured
        pPool = r.pPool;
        bOwnerOfPool = sal_False;
    }
    else
    {
        pPool = EditEngine::CreatePool();
        bOwnerOfPool =  sal_True;

    }

    if(!bOwnerOfPool && pPool)
    {
        // it is sure now that the pool is an EditEngineItemPool
        pPool->AddSfxItemPoolUser(*this);
    }

    if ( bOwnerOfPool && pPool && r.pPool )
        pPool->SetDefaultMetric( r.pPool->GetMetric( DEF_METRIC ) );

    for ( sal_uInt16 n = 0; n < r.aContents.Count(); n++ )
    {
        ContentInfo* pOrg = r.aContents.GetObject( n );
        DBG_ASSERT( pOrg, "NULL-Pointer in ContentList!" );
        ContentInfo* pNew = new ContentInfo( *pOrg, *pPool );
        aContents.Insert( pNew, aContents.Count() );
    }
}

BinTextObject::~BinTextObject()
{
    if(!bOwnerOfPool && pPool)
    {
        pPool->RemoveSfxItemPoolUser(*this);
    }

    ClearPortionInfo();
    DeleteContents();
    if ( bOwnerOfPool )
    {
        SfxItemPool::Free(pPool);
    }
}

sal_uInt16 BinTextObject::GetUserType() const
{
    return nUserType;
}

void BinTextObject::SetUserType( sal_uInt16 n )
{
    nUserType = n;
}

sal_uLong BinTextObject::GetObjectSettings() const
{
    return nObjSettings;
}

void BinTextObject::SetObjectSettings( sal_uLong n )
{
    nObjSettings = n;
}

sal_Bool BinTextObject::IsVertical() const
{
    return bVertical;
}

void BinTextObject::SetVertical( sal_Bool b )
{
    if ( b != bVertical )
    {
        bVertical = b;
        ClearPortionInfo();
    }
}

sal_uInt16 BinTextObject::GetScriptType() const
{
    return nScriptType;
}

void BinTextObject::SetScriptType( sal_uInt16 nType )
{
    nScriptType = nType;
}


void BinTextObject::DeleteContents()
{
    for ( sal_uInt16 n = 0; n < aContents.Count(); n++ )
    {
        ContentInfo* p = aContents.GetObject( n );
        DBG_ASSERT( p, "NULL-Pointer in ContentList!" );
        delete p;
    }
    aContents.Remove( 0, aContents.Count() );
}

EditTextObject* BinTextObject::Clone() const
{
    return new BinTextObject( *this );
}

XEditAttribute* BinTextObject::CreateAttrib( const SfxPoolItem& rItem, sal_uInt16 nStart, sal_uInt16 nEnd )
{
    return MakeXEditAttribute( *pPool, rItem, nStart, nEnd );
}

void BinTextObject::DestroyAttrib( XEditAttribute* pAttr )
{
    pPool->Remove( *pAttr->GetItem() );
    delete pAttr;
}

ContentInfo* BinTextObject::CreateAndInsertContent()
{
    ContentInfo* pC = new ContentInfo( *pPool );
    aContents.Insert( pC, aContents.Count() );
    return pC;
}

sal_uInt16 BinTextObject::GetParagraphCount() const
{
    return aContents.Count();
}

XubString BinTextObject::GetText( sal_uInt16 nPara ) const
{
    DBG_ASSERT( nPara < aContents.Count(), "BinTextObject::GetText: Paragraph does not exist!" );
    if ( nPara < aContents.Count() )
    {
        ContentInfo* pC = aContents[ nPara ];
        return pC->GetText();
    }
    return XubString();
}

void BinTextObject::Insert( const EditTextObject& rObj, sal_uInt16 nDestPara )
{
    DBG_ASSERT( rObj.Which() == EE_FORMAT_BIN, "UTO: unknown Textobjekt" );

    const BinTextObject& rBinObj = (const BinTextObject&)rObj;

    if ( nDestPara > aContents.Count() )
        nDestPara = aContents.Count();

    const sal_uInt16 nParas = rBinObj.GetContents().Count();
    for ( sal_uInt16 nP = 0; nP < nParas; nP++ )
    {
        ContentInfo* pC = rBinObj.GetContents()[ nP ];
        ContentInfo* pNew = new ContentInfo( *pC, *GetPool() );
        aContents.Insert( pNew, nDestPara+nP );
    }
    ClearPortionInfo();
}

EditTextObject* BinTextObject::CreateTextObject( sal_uInt16 nPara, sal_uInt16 nParas ) const
{
    if ( ( nPara >= aContents.Count() ) || !nParas )
        return NULL;

    // Only split the Pool, when a the Pool is set externally.
    BinTextObject* pObj = new BinTextObject( bOwnerOfPool ? 0 : pPool );
    if ( bOwnerOfPool && pPool )
        pObj->GetPool()->SetDefaultMetric( pPool->GetMetric( DEF_METRIC ) );

    // If complete text is only one ScriptType, this is valid.
    // If text contains different ScriptTypes, this shouldn't be a problem...
    pObj->nScriptType = nScriptType;

    const sal_uInt16 nEndPara = nPara+nParas-1;
    for ( sal_uInt16 nP = nPara; nP <= nEndPara; nP++ )
    {
        ContentInfo* pC = aContents[ nP ];
        ContentInfo* pNew = new ContentInfo( *pC, *pObj->GetPool() );
        pObj->GetContents().Insert( pNew, pObj->GetContents().Count() );
    }
    return pObj;
}

void BinTextObject::RemoveParagraph( sal_uInt16 nPara )
{
    DBG_ASSERT( nPara < aContents.Count(), "BinTextObject::GetText: Paragraph does not exist!" );
    if ( nPara < aContents.Count() )
    {
        ContentInfo* pC = aContents[ nPara ];
        aContents.Remove( nPara );
        delete pC;
        ClearPortionInfo();
    }
}

sal_Bool BinTextObject::HasPortionInfo() const
{
    return pPortionInfo ? sal_True : sal_False;
}

void BinTextObject::ClearPortionInfo()
{
    if ( pPortionInfo )
    {
        for ( sal_uInt16 n = pPortionInfo->Count(); n; )
            delete pPortionInfo->GetObject( --n );
        delete pPortionInfo;
        pPortionInfo = NULL;
    }
}

sal_Bool BinTextObject::HasOnlineSpellErrors() const
{
    for ( sal_uInt16 n = 0; n < aContents.Count(); n++ )
    {
        ContentInfo* p = aContents.GetObject( n );
        if ( p->GetWrongList() && p->GetWrongList()->Count() )
            return sal_True;
    }
    return sal_False;

}

sal_Bool BinTextObject::HasCharAttribs( sal_uInt16 _nWhich ) const
{
    for ( sal_uInt16 nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );

        sal_uInt16 nAttribs = pC->GetAttribs().Count();
        if ( nAttribs && !_nWhich )
            return sal_True;

        for ( sal_uInt16 nAttr = nAttribs; nAttr; )
        {
            XEditAttribute* pX = pC->GetAttribs().GetObject( --nAttr );
            if ( pX->GetItem()->Which() == _nWhich )
                return sal_True;
        }
    }
    return sal_False;
}

void BinTextObject::GetCharAttribs( sal_uInt16 nPara, EECharAttribArray& rLst ) const
{
    rLst.Remove( 0, rLst.Count() );
    ContentInfo* pC = GetContents().GetObject( nPara );
    if ( pC )
    {
        for ( sal_uInt16 nAttr = 0; nAttr < pC->GetAttribs().Count(); nAttr++ )
        {
            XEditAttribute* pAttr = pC->GetAttribs().GetObject( nAttr );
            EECharAttrib aEEAttr;
            aEEAttr.pAttr = pAttr->GetItem();
            aEEAttr.nPara = nPara;
            aEEAttr.nStart = pAttr->GetStart();
            aEEAttr.nEnd = pAttr->GetEnd();
            rLst.Insert( aEEAttr, rLst.Count() );
        }
    }
}

void BinTextObject::MergeParaAttribs( const SfxItemSet& rAttribs, sal_uInt16 nStart, sal_uInt16 nEnd )
{
    sal_Bool bChanged = sal_False;

    for ( sal_uInt16 nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );

        for ( sal_uInt16 nW = nStart; nW <= nEnd; nW++ )
        {
            if ( ( pC->GetParaAttribs().GetItemState( nW, sal_False ) != SFX_ITEM_ON )
                    && ( rAttribs.GetItemState( nW, sal_False ) == SFX_ITEM_ON ) )
            {
                pC->GetParaAttribs().Put( rAttribs.Get( nW ) );
                bChanged = sal_True;
            }
        }
    }

    if ( bChanged )
        ClearPortionInfo();
}

sal_Bool BinTextObject::IsFieldObject() const
{
    return BinTextObject::GetField() ? sal_True : sal_False;
}

const SvxFieldItem* BinTextObject::GetField() const
{
    if ( GetContents().Count() == 1 )
    {
        ContentInfo* pC = GetContents()[0];
        if ( pC->GetText().Len() == 1 )
        {
            sal_uInt16 nAttribs = pC->GetAttribs().Count();
            for ( sal_uInt16 nAttr = nAttribs; nAttr; )
            {
                XEditAttribute* pX = pC->GetAttribs().GetObject( --nAttr );
                if ( pX->GetItem()->Which() == EE_FEATURE_FIELD )
                    return (const SvxFieldItem*)pX->GetItem();
            }
        }
    }
    return 0;
}

sal_Bool BinTextObject::HasField( TypeId aType ) const
{
    sal_uInt16 nParagraphs = GetContents().Count();
    for ( sal_uInt16 nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = GetContents().GetObject( nPara );
        sal_uInt16 nAttrs = pC->GetAttribs().Count();
        for ( sal_uInt16 nAttr = 0; nAttr < nAttrs; nAttr++ )
        {
            XEditAttribute* pAttr = pC->GetAttribs()[nAttr];
            if ( pAttr->GetItem()->Which() == EE_FEATURE_FIELD )
            {
                if ( !aType )
                    return sal_True;

                const SvxFieldData* pFldData = ((const SvxFieldItem*)pAttr->GetItem())->GetField();
                if ( pFldData && pFldData->IsA( aType ) )
                    return sal_True;
            }
        }
    }
    return sal_False;
}

SfxItemSet BinTextObject::GetParaAttribs( sal_uInt16 nPara ) const
{
    ContentInfo* pC = GetContents().GetObject( nPara );
    return pC->GetParaAttribs();
}

void BinTextObject::SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& rAttribs )
{
    ContentInfo* pC = GetContents().GetObject( nPara );
    pC->GetParaAttribs().Set( rAttribs );
    ClearPortionInfo();
}

sal_Bool BinTextObject::RemoveCharAttribs( sal_uInt16 _nWhich )
{
    sal_Bool bChanged = sal_False;

    for ( sal_uInt16 nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );

        for ( sal_uInt16 nAttr = pC->GetAttribs().Count(); nAttr; )
        {
            XEditAttribute* pAttr = pC->GetAttribs().GetObject( --nAttr );
            if ( !_nWhich || ( pAttr->GetItem()->Which() == _nWhich ) )
            {
                pC->GetAttribs().Remove( nAttr );
                DestroyAttrib( pAttr );
                bChanged = sal_True;
            }
        }
    }

    if ( bChanged )
        ClearPortionInfo();

    return bChanged;
}

sal_Bool BinTextObject::RemoveParaAttribs( sal_uInt16 _nWhich )
{
    sal_Bool bChanged = sal_False;

    for ( sal_uInt16 nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );

        if ( !_nWhich )
        {
            if( pC->GetParaAttribs().Count() )
                bChanged = sal_True;
            pC->GetParaAttribs().ClearItem();
        }
        else
        {
            if ( pC->GetParaAttribs().GetItemState( _nWhich ) == SFX_ITEM_ON )
            {
                pC->GetParaAttribs().ClearItem( _nWhich );
                bChanged = sal_True;
            }
        }
    }

    if ( bChanged )
        ClearPortionInfo();

    return bChanged;
}

sal_Bool BinTextObject::HasStyleSheet( const XubString& rName, SfxStyleFamily eFamily ) const
{
    sal_uInt16 nParagraphs = GetContents().Count();
    for ( sal_uInt16 nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = GetContents().GetObject( nPara );
        if ( ( pC->GetFamily() == eFamily ) && ( pC->GetStyle() == rName ) )
            return sal_True;
    }
    return sal_False;
}

void BinTextObject::GetStyleSheet( sal_uInt16 nPara, XubString& rName, SfxStyleFamily& rFamily ) const
{
    if ( nPara < aContents.Count() )
    {
        ContentInfo* pC = aContents[ nPara ];
        rName = pC->GetStyle();
        rFamily = pC->GetFamily();
    }
}

void BinTextObject::SetStyleSheet( sal_uInt16 nPara, const XubString& rName, const SfxStyleFamily& rFamily )
{
    if ( nPara < aContents.Count() )
    {
        ContentInfo* pC = aContents[ nPara ];
        pC->GetStyle() = rName;
        pC->GetFamily() = rFamily;
    }
}

sal_Bool BinTextObject::ImpChangeStyleSheets(
                    const XubString& rOldName, SfxStyleFamily eOldFamily,
                    const XubString& rNewName, SfxStyleFamily eNewFamily )
{
    const sal_uInt16 nParagraphs = GetContents().Count();
    sal_Bool bChanges = sal_False;

    for ( sal_uInt16 nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = GetContents().GetObject( nPara );
        if ( pC->GetFamily() == eOldFamily )
        {
            if ( pC->GetStyle() == rOldName )
            {
                pC->GetStyle() = rNewName;
                pC->GetFamily() = eNewFamily;
                bChanges = sal_True;
            }
        }
    }
    return bChanges;
}

sal_Bool BinTextObject::ChangeStyleSheets(
                    const XubString& rOldName, SfxStyleFamily eOldFamily,
                    const XubString& rNewName, SfxStyleFamily eNewFamily )
{
    sal_Bool bChanges = ImpChangeStyleSheets( rOldName, eOldFamily, rNewName, eNewFamily );
    if ( bChanges )
        ClearPortionInfo();

    return bChanges;
}

void BinTextObject::ChangeStyleSheetName( SfxStyleFamily eFamily,
                const XubString& rOldName, const XubString& rNewName )
{
    ImpChangeStyleSheets( rOldName, eFamily, rNewName, eFamily );
}

void BinTextObject::StoreData( SvStream& rOStream ) const
{
    sal_uInt16 nVer = 602;
    rOStream << nVer;

    rOStream << bOwnerOfPool;

    // First store the pool, later only the Surregate
    if ( bOwnerOfPool )
    {
        GetPool()->SetFileFormatVersion( SOFFICE_FILEFORMAT_50 );
        GetPool()->Store( rOStream );
    }

    // Store Current text encoding ...
    rtl_TextEncoding eEncoding = GetSOStoreTextEncoding( osl_getThreadTextEncoding(), (sal_uInt16) rOStream.GetVersion() );
    rOStream << (sal_uInt16) eEncoding;

    // The number of paragraphs ...
    sal_uInt16 nParagraphs = GetContents().Count();
    rOStream << nParagraphs;

    sal_Unicode nUniChar = CH_FEATURE;
    char cFeatureConverted = rtl::OString(&nUniChar, 1, eEncoding).toChar();

    // The individual paragraphs ...
    for ( sal_uInt16 nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = GetContents().GetObject( nPara );

        // Text...
        ByteString aText(rtl::OUStringToOString(pC->GetText(), eEncoding));

        // Symbols?
        sal_Bool bSymbolPara = sal_False;
        if ( pC->GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SFX_ITEM_ON )
        {
            const SvxFontItem& rFontItem = (const SvxFontItem&)pC->GetParaAttribs().Get( EE_CHAR_FONTINFO );
            if ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
            {
                aText = rtl::OUStringToOString(pC->GetText(), RTL_TEXTENCODING_SYMBOL);
                bSymbolPara = sal_True;
            }
        }
        for ( sal_uInt16 nA = 0; nA < pC->GetAttribs().Count(); nA++ )
        {
            XEditAttribute* pAttr = pC->GetAttribs().GetObject( nA );

            if ( pAttr->GetItem()->Which() == EE_CHAR_FONTINFO )
            {
                const SvxFontItem& rFontItem = (const SvxFontItem&)*pAttr->GetItem();
                if ( ( !bSymbolPara && ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL ) )
                      || ( bSymbolPara && ( rFontItem.GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
                {
                    // Not correctly converted
                    String aPart( pC->GetText(), pAttr->GetStart(), pAttr->GetEnd() - pAttr->GetStart() );
                    rtl::OString aNew(rtl::OUStringToOString(aPart, rFontItem.GetCharSet()));
                    aText.Erase( pAttr->GetStart(), pAttr->GetEnd() - pAttr->GetStart() );
                    aText.Insert( aNew, pAttr->GetStart() );
                }

                // Convert StarSymbol back to StarBats
                FontToSubsFontConverter hConv = CreateFontToSubsFontConverter( rFontItem.GetFamilyName(), FONTTOSUBSFONT_EXPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
                if ( hConv )
                {
                    // Don't create a new Attrib with StarBats font, MBR changed the
                    // SvxFontItem::Store() to store StarBats instead of StarSymbol!
                    for ( sal_uInt16 nChar = pAttr->GetStart(); nChar < pAttr->GetEnd(); nChar++ )
                    {
                        sal_Unicode cOld = pC->GetText().GetChar( nChar );
                        char cConv = rtl::OUStringToOString(rtl::OUString(ConvertFontToSubsFontChar(hConv, cOld)), RTL_TEXTENCODING_SYMBOL).toChar();
                        if ( cConv )
                            aText.SetChar( nChar, cConv );
                    }

                    DestroyFontToSubsFontConverter( hConv );
                }
            }
        }

        // Convert StarSymbol back to StarBats
        // StarSymbol as paragraph attribute or in StyleSheet?

        FontToSubsFontConverter hConv = NULL;
        if ( pC->GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SFX_ITEM_ON )
        {
            hConv = CreateFontToSubsFontConverter( ((const SvxFontItem&)pC->GetParaAttribs().Get( EE_CHAR_FONTINFO )).GetFamilyName(), FONTTOSUBSFONT_EXPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
        }
        if ( hConv )
        {
            for ( sal_uInt16 nChar = 0; nChar < pC->GetText().Len(); nChar++ )
            {
                if ( !pC->GetAttribs().FindAttrib( EE_CHAR_FONTINFO, nChar ) )
                {
                    sal_Unicode cOld = pC->GetText().GetChar( nChar );
                    char cConv = rtl::OUStringToOString(rtl::OUString(ConvertFontToSubsFontChar(hConv, cOld)), RTL_TEXTENCODING_SYMBOL).toChar();
                    if ( cConv )
                        aText.SetChar( nChar, cConv );
                }
            }

            DestroyFontToSubsFontConverter( hConv );

        }


        // Convert CH_FEATURE to CH_FEATURE_OLD
        aText.SearchAndReplaceAll( cFeatureConverted, CH_FEATURE_OLD );
        rOStream.WriteByteString( aText );

        // StyleName and Family...
        rOStream.WriteByteString( rtl::OUStringToOString(pC->GetStyle(), eEncoding) );
        rOStream << (sal_uInt16)pC->GetFamily();

        // Paragraph attributes ...
        pC->GetParaAttribs().Store( rOStream );

        // The number of attributes ...
        sal_uInt16 nAttribs = pC->GetAttribs().Count();
        rOStream << nAttribs;

        // And the individual attributes
        // Items as Surregate => always 8 bytes per Attribute
        // Which = 2; Surregat = 2; Start = 2; End = 2;
        for ( sal_uInt16 nAttr = 0; nAttr < nAttribs; nAttr++ )
        {
            XEditAttribute* pX = pC->GetAttribs().GetObject( nAttr );

            rOStream << pX->GetItem()->Which();
            GetPool()->StoreSurrogate( rOStream, pX->GetItem() );
            rOStream << pX->GetStart();
            rOStream << pX->GetEnd();
        }
    }

    rOStream << nMetric;

    rOStream << nUserType;
    rOStream << nObjSettings;

    rOStream << bVertical;
    rOStream << nScriptType;

    rOStream << bStoreUnicodeStrings;
    if ( bStoreUnicodeStrings )
    {
        for ( sal_uInt16 nPara = 0; nPara < nParagraphs; nPara++ )
        {
            ContentInfo* pC = GetContents().GetObject( nPara );
            sal_uInt16 nL = pC->GetText().Len();
            rOStream << nL;
            rOStream.Write( pC->GetText().GetBuffer(), nL*sizeof(sal_Unicode) );

            // StyleSheetName must be Unicode too!
            // Copy/Paste from EA3 to BETA or from BETA to EA3 not possible, not needed...
            // If needed, change nL back to sal_uLong and increase version...
            nL = pC->GetStyle().Len();
            rOStream << nL;
            rOStream.Write( pC->GetStyle().GetBuffer(), nL*sizeof(sal_Unicode) );
        }
    }
}

void BinTextObject::CreateData( SvStream& rIStream )
{
    rIStream >> nVersion;

    // The text object was first created with the current setting of
    // pTextObjectPool.
    sal_Bool bOwnerOfCurrent = bOwnerOfPool;
    rIStream >> bOwnerOfPool;

    if ( bOwnerOfCurrent && !bOwnerOfPool )
    {
        // A global Pool was used, but not handed over to me, but I need it!
        OSL_FAIL( "Give me the global TextObjectPool!" );
        return;
    }
    else if ( !bOwnerOfCurrent && bOwnerOfPool )
    {
        // A global Pool should be used, but this Textobject has its own.
        pPool = EditEngine::CreatePool();
    }

    if ( bOwnerOfPool )
        GetPool()->Load( rIStream );

    // CharSet, in which it was saved:
    sal_uInt16 nCharSet;
    rIStream >> nCharSet;

    rtl_TextEncoding eSrcEncoding = GetSOLoadTextEncoding( (rtl_TextEncoding)nCharSet, (sal_uInt16)rIStream.GetVersion() );

    // The number of paragraphs ...
    sal_uInt16 nParagraphs;
    rIStream >> nParagraphs;

    // The individual paragraphs ...
    for ( sal_uLong nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = CreateAndInsertContent();

        // The Text...
        ByteString aByteString;
        rIStream.ReadByteString( aByteString );
        pC->GetText() = String( aByteString, eSrcEncoding );

        // StyleName and Family...
        rIStream.ReadByteString( pC->GetStyle(), eSrcEncoding );
        sal_uInt16 nStyleFamily;
        rIStream >> nStyleFamily;
        pC->GetFamily() = (SfxStyleFamily)nStyleFamily;

        // Paragraph attributes ...
        pC->GetParaAttribs().Load( rIStream );

        // The number of attributes ...
        sal_uInt16 nAttribs;
        rIStream >> nAttribs;

        // And the individual attributes
        // Items as Surregate => always 8 bytes per Attributes
        // Which = 2; Surregat = 2; Start = 2; End = 2;
        sal_uInt16 nAttr;
        for ( nAttr = 0; nAttr < nAttribs; nAttr++ )
        {
            sal_uInt16 _nWhich, nStart, nEnd;
            const SfxPoolItem* pItem;

            rIStream >> _nWhich;
            _nWhich = pPool->GetNewWhich( _nWhich );
            pItem = pPool->LoadSurrogate( rIStream, _nWhich, 0 );
            rIStream >> nStart;
            rIStream >> nEnd;
            if ( pItem )
            {
                if ( pItem->Which() == EE_FEATURE_NOTCONV )
                {
                    sal_Char cEncodedChar = aByteString.GetChar(nStart);
                    sal_Unicode cChar = rtl::OUString(&cEncodedChar, 1,
                        ((SvxCharSetColorItem*)pItem)->GetCharSet()).toChar();
                    pC->GetText().SetChar(nStart, cChar);
                }
                else
                {
                    XEditAttribute* pAttr = new XEditAttribute( *pItem, nStart, nEnd );
                    pC->GetAttribs().Insert( pAttr, pC->GetAttribs().Count() );

                    if ( ( _nWhich >= EE_FEATURE_START ) && ( _nWhich <= EE_FEATURE_END ) )
                    {
                        // Convert CH_FEATURE to CH_FEATURE_OLD
                        DBG_ASSERT( (sal_uInt8) aByteString.GetChar( nStart ) == CH_FEATURE_OLD, "CreateData: CH_FEATURE expected!" );
                        if ( (sal_uInt8) aByteString.GetChar( nStart ) == CH_FEATURE_OLD )
                            pC->GetText().SetChar( nStart, CH_FEATURE );
                    }
                }
            }
        }

        // But check for paragraph and character symbol attribs here,
        // FinishLoad will not be called in OpenOffice Calc, no StyleSheets...

        sal_Bool bSymbolPara = sal_False;
        if ( pC->GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SFX_ITEM_ON )
        {
            const SvxFontItem& rFontItem = (const SvxFontItem&)pC->GetParaAttribs().Get( EE_CHAR_FONTINFO );
            if ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
            {
                pC->GetText() = String( aByteString, RTL_TEXTENCODING_SYMBOL );
                bSymbolPara = sal_True;
            }
        }

        for ( nAttr = pC->GetAttribs().Count(); nAttr; )
        {
            XEditAttribute* pAttr = pC->GetAttribs().GetObject( --nAttr );
            if ( pAttr->GetItem()->Which() == EE_CHAR_FONTINFO )
            {
                const SvxFontItem& rFontItem = (const SvxFontItem&)*pAttr->GetItem();
                if ( ( !bSymbolPara && ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL ) )
                      || ( bSymbolPara && ( rFontItem.GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
                {
                    // Not correctly converted
                    ByteString aPart( aByteString, pAttr->GetStart(), pAttr->GetEnd()-pAttr->GetStart() );
                    String aNew( aPart, rFontItem.GetCharSet() );
                    pC->GetText().Erase( pAttr->GetStart(), pAttr->GetEnd()-pAttr->GetStart() );
                    pC->GetText().Insert( aNew, pAttr->GetStart() );
                }

                // Convert StarMath and StarBats to StarSymbol
                FontToSubsFontConverter hConv = CreateFontToSubsFontConverter( rFontItem.GetFamilyName(), FONTTOSUBSFONT_IMPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
                if ( hConv )
                {
                    SvxFontItem aNewFontItem( rFontItem );
                    aNewFontItem.SetFamilyName( GetFontToSubsFontName( hConv ) );

                    pC->GetAttribs().Remove( nAttr );
                    XEditAttribute* pNewAttr = CreateAttrib( aNewFontItem, pAttr->GetStart(), pAttr->GetEnd() );
                    pC->GetAttribs().Insert( pNewAttr, nAttr );
                    DestroyAttrib( pAttr );

                    for ( sal_uInt16 nChar = pNewAttr->GetStart(); nChar < pNewAttr->GetEnd(); nChar++ )
                    {
                        sal_Unicode cOld = pC->GetText().GetChar( nChar );
                        DBG_ASSERT( cOld >= 0xF000, "cOld not converted?!" );
                        sal_Unicode cConv = ConvertFontToSubsFontChar( hConv, cOld );
                        if ( cConv )
                            pC->GetText().SetChar( nChar, cConv );
                    }

                    DestroyFontToSubsFontConverter( hConv );
                }
            }
        }


        // Convert StarMath and StarBats to StarSymbol
        // Maybe old symbol font as paragraph attribute?
        if ( pC->GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SFX_ITEM_ON )
        {
            const SvxFontItem& rFontItem = (const SvxFontItem&)pC->GetParaAttribs().Get( EE_CHAR_FONTINFO );
            FontToSubsFontConverter hConv = CreateFontToSubsFontConverter( rFontItem.GetFamilyName(), FONTTOSUBSFONT_IMPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
            if ( hConv )
            {
                SvxFontItem aNewFontItem( rFontItem );
                aNewFontItem.SetFamilyName( GetFontToSubsFontName( hConv ) );
                pC->GetParaAttribs().Put( aNewFontItem );

                for ( sal_uInt16 nChar = 0; nChar < pC->GetText().Len(); nChar++ )
                {
                    if ( !pC->GetAttribs().FindAttrib( EE_CHAR_FONTINFO, nChar ) )
                    {
                        sal_Unicode cOld = pC->GetText().GetChar( nChar );
                        DBG_ASSERT( cOld >= 0xF000, "cOld not converted?!" );
                        sal_Unicode cConv = ConvertFontToSubsFontChar( hConv, cOld );
                        if ( cConv )
                            pC->GetText().SetChar( nChar, cConv );
                    }
                }

                DestroyFontToSubsFontConverter( hConv );
            }
        }
    }

    // From 400 also the DefMetric:
    if ( nVersion >= 400 )
    {
        sal_uInt16 nTmpMetric;
        rIStream >> nTmpMetric;
        if ( nVersion >= 401 )
        {
            // In the 400 there was a bug in text objects with the own Pool,
            // therefore evaluate only from 401
            nMetric = nTmpMetric;
            if ( bOwnerOfPool && pPool && ( nMetric != 0xFFFF ) )
                pPool->SetDefaultMetric( (SfxMapUnit)nMetric );
        }
    }

    if ( nVersion >= 600 )
    {
        rIStream >> nUserType;
        rIStream >> nObjSettings;
    }

    if ( nVersion >= 601 )
    {
        rIStream >> bVertical;
    }

    if ( nVersion >= 602 )
    {
        rIStream >> nScriptType;

        sal_Bool bUnicodeStrings;
        rIStream >> bUnicodeStrings;
        if ( bUnicodeStrings )
        {
            for ( sal_uInt16 nPara = 0; nPara < nParagraphs; nPara++ )
            {
                ContentInfo* pC = GetContents().GetObject( nPara );
                sal_uInt16 nL;

                // Text
                rIStream >> nL;
                if ( nL )
                {
                    pC->GetText().AllocBuffer( nL );
                    rIStream.Read( pC->GetText().GetBufferAccess(), nL*sizeof(sal_Unicode) );
                    pC->GetText().ReleaseBufferAccess( (sal_uInt16)nL );
                }

                // StyleSheetName
                rIStream >> nL;
                if ( nL )
                {
                    pC->GetStyle().AllocBuffer( nL );
                    rIStream.Read( pC->GetStyle().GetBufferAccess(), nL*sizeof(sal_Unicode) );
                    pC->GetStyle().ReleaseBufferAccess( (sal_uInt16)nL );
                }
            }
        }
    }


    // from 500 the tabs are interpreted differently: TabPos + LI, previously only TabPos.
    // Works only if tab positions are set, not when DefTab.
    if ( nVersion < 500 )
    {
        for ( sal_uInt16 n = 0; n < aContents.Count(); n++ )
        {
            ContentInfo* pC = aContents.GetObject( n );
            const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&) pC->GetParaAttribs().Get( EE_PARA_LRSPACE );
            if ( rLRSpace.GetTxtLeft() && ( pC->GetParaAttribs().GetItemState( EE_PARA_TABS ) == SFX_ITEM_ON ) )
            {
                const SvxTabStopItem& rTabs = (const SvxTabStopItem&) pC->GetParaAttribs().Get( EE_PARA_TABS );
                SvxTabStopItem aNewTabs( 0, 0, SVX_TAB_ADJUST_LEFT, EE_PARA_TABS );
                for ( sal_uInt16 t = 0; t < rTabs.Count(); t++ )
                {
                    const SvxTabStop& rT = rTabs[ t ];
                    aNewTabs.Insert( SvxTabStop( rT.GetTabPos() - rLRSpace.GetTxtLeft(),
                                rT.GetAdjustment(), rT.GetDecimal(), rT.GetFill() ) );
                }
                pC->GetParaAttribs().Put( aNewTabs );
            }
        }
    }
}

sal_uInt16 BinTextObject::GetVersion() const
{
    return nVersion;
}

bool BinTextObject::operator==( const BinTextObject& rCompare ) const
{
    if( this == &rCompare )
        return true;

    if( ( aContents.Count() != rCompare.aContents.Count() ) ||
            ( pPool != rCompare.pPool ) ||
            ( nMetric != rCompare.nMetric ) ||
            ( nUserType!= rCompare.nUserType ) ||
            ( nScriptType != rCompare.nScriptType ) ||
            ( bVertical != rCompare.bVertical ) )
        return false;

    sal_uInt16 n;
    for( n = 0; n < aContents.Count(); n++ )
    {
        if( !( *aContents.GetObject( n ) == *rCompare.aContents.GetObject( n ) ) )
            return false;
    }

    return true;
}

// #i102062#
bool BinTextObject::isWrongListEqual(const BinTextObject& rCompare) const
{
    if(GetContents().Count() != rCompare.GetContents().Count())
    {
        return false;
    }

    for(sal_uInt16 a(0); a < GetContents().Count(); a++)
    {
        const ContentInfo& rCandA(*GetContents().GetObject(a));
        const ContentInfo& rCandB(*rCompare.GetContents().GetObject(a));

        if(!rCandA.isWrongListEqual(rCandB))
        {
            return false;
        }
    }

    return true;
}

#define CHARSETMARKER   0x9999

void BinTextObject::CreateData300( SvStream& rIStream )
{
    // For forward compatibility.

    // First load the Pool...
    // Is always saved in the 300!
    GetPool()->Load( rIStream );

    // The number of paragraphs ...
    sal_uInt32 nParagraphs;
    rIStream >> nParagraphs;

    // The individual paragraphs...
    for ( sal_uLong nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = CreateAndInsertContent();

        // The Text...
        rIStream.ReadByteString( pC->GetText() );

        // StyleName and Family...
        rIStream.ReadByteString( pC->GetStyle() );
        sal_uInt16 nStyleFamily;
        rIStream >> nStyleFamily;
        pC->GetFamily() = (SfxStyleFamily)nStyleFamily;

        // Paragraph attributes ...
        pC->GetParaAttribs().Load( rIStream );

        // The number of attributes ...
        sal_uInt32 nAttribs;
        rIStream >> nAttribs;

        // And the individual attributes
        // Items as Surregate => always 8 bytes per Attribute
        // Which = 2; Surregat = 2; Start = 2; End = 2;
        for ( sal_uLong nAttr = 0; nAttr < nAttribs; nAttr++ )
        {
            sal_uInt16 _nWhich, nStart, nEnd;
            const SfxPoolItem* pItem;

            rIStream >> _nWhich;
            _nWhich = pPool->GetNewWhich( _nWhich );
            pItem = pPool->LoadSurrogate( rIStream, _nWhich, 0 );
            rIStream >> nStart;
            rIStream >> nEnd;
            if ( pItem )
            {
                XEditAttribute* pAttr = new XEditAttribute( *pItem, nStart, nEnd );
                pC->GetAttribs().Insert( pAttr, pC->GetAttribs().Count() );
            }
        }
    }

    // Check whether a font was saved
    sal_uInt16 nCharSetMarker;
    rIStream >> nCharSetMarker;
    if ( nCharSetMarker == CHARSETMARKER )
    {
        sal_uInt16 nCharSet;
        rIStream >> nCharSet;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
