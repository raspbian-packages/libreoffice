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



#include <tools/stream.hxx>
#include "svl/brdcst.hxx"

#include <basic/sbx.hxx>
#include "sbxres.hxx"
#include "sbxconv.hxx"
#include <math.h>
#include <ctype.h>

#include "com/sun/star/uno/XInterface.hpp"
using namespace com::sun::star::uno;

///////////////////////////// SbxVariable //////////////////////////////

TYPEINIT1(SbxVariable,SbxValue)
TYPEINIT1(SbxHint,SfxSimpleHint)

extern sal_uInt32 nVarCreator;          // in SBXBASE.CXX, fuer LoadData()
#ifdef DBG_UTIL
static sal_uIntPtr nVar = 0;
#endif

///////////////////////////// SbxVariableImpl ////////////////////////////

class SbxVariableImpl
{
    friend class SbxVariable;
    String                      m_aDeclareClassName;
    Reference< XInterface >     m_xComListener;
    StarBASIC*                  m_pComListenerParentBasic;

    SbxVariableImpl( void )
        : m_pComListenerParentBasic( NULL )
    {}
    SbxVariableImpl( const SbxVariableImpl& r )
        : m_aDeclareClassName( r.m_aDeclareClassName )
        , m_xComListener( r.m_xComListener )
        , m_pComListenerParentBasic( r.m_pComListenerParentBasic )
    {
    }
};


///////////////////////////// Constructors //////////////////////////////

SbxVariable::SbxVariable() : SbxValue()
{
    mpSbxVariableImpl = NULL;
    pCst = NULL;
    pParent = NULL;
    nUserData = 0;
    nHash = 0;
#ifdef DBG_UTIL
    DbgOutf( "SbxVariable::Ctor %lx=%ld", (void*)this, ++nVar );
#endif
}

void registerComListenerVariableForBasic( SbxVariable* pVar, StarBASIC* pBasic );

SbxVariable::SbxVariable( const SbxVariable& r )
           : SvRefBase( r ), SbxValue( r ), mpPar( r.mpPar ), pInfo( r.pInfo )
{
    mpSbxVariableImpl = NULL;
    if( r.mpSbxVariableImpl != NULL )
    {
        mpSbxVariableImpl = new SbxVariableImpl( *r.mpSbxVariableImpl );
        if( mpSbxVariableImpl->m_xComListener.is() )
            registerComListenerVariableForBasic( this, mpSbxVariableImpl->m_pComListenerParentBasic );
    }
    pCst = NULL;
    if( r.CanRead() )
    {
        pParent = r.pParent;
        nUserData = r.nUserData;
        maName = r.maName;
        nHash = r.nHash;
    }
    else
    {
        pParent = NULL;
        nUserData = 0;
        nHash = 0;
    }
#ifdef DBG_UTIL
    static sal_Char const aCellsStr[] = "Cells";
    if ( maName.EqualsAscii( aCellsStr ) )
        maName.AssignAscii( aCellsStr, sizeof( aCellsStr )-1 );
    DbgOutf( "SbxVariable::Ctor %lx=%ld", (void*)this, ++nVar );
#endif
}

SbxVariable::SbxVariable( SbxDataType t, void* p ) : SbxValue( t, p )
{
    mpSbxVariableImpl = NULL;
    pCst = NULL;
    pParent = NULL;
    nUserData = 0;
    nHash = 0;
#ifdef DBG_UTIL
    DbgOutf( "SbxVariable::Ctor %lx=%ld", (void*)this, ++nVar );
#endif
}

void removeDimAsNewRecoverItem( SbxVariable* pVar );

SbxVariable::~SbxVariable()
{
#ifdef DBG_UTIL
    rtl::OString aBStr(rtl::OUStringToOString(maName, RTL_TEXTENCODING_ASCII_US));
    DbgOutf( "SbxVariable::Dtor %lx (%s)", (void*)this, aBStr.getStr() );
    static sal_Char const aCellsStr[] = "Cells";
    if ( maName.EqualsAscii( aCellsStr ) )
        maName.AssignAscii( aCellsStr, sizeof( aCellsStr )-1 );
#endif
    if( IsSet( SBX_DIM_AS_NEW ))
        removeDimAsNewRecoverItem( this );
    delete mpSbxVariableImpl;
    delete pCst;
}

////////////////////////////// Broadcasting //////////////////////////////

SfxBroadcaster& SbxVariable::GetBroadcaster()
{
    if( !pCst )
        pCst = new SfxBroadcaster;
    return *pCst;
}

SbxArray* SbxVariable::GetParameters() const
{
    return mpPar;
}

SbxObject* SbxVariable::GetParent()
{
    return pParent;
}

// Perhaps some day one could cut the parameter 0.
// then the copying will be dropped ...

void SbxVariable::Broadcast( sal_uIntPtr nHintId )
{
    if( pCst && !IsSet( SBX_NO_BROADCAST ) )
    {
        // Because the method could be called from outside, check the Da die Methode von aussen aufrufbar ist, hier noch einmal
        // rights here again
        if( nHintId & SBX_HINT_DATAWANTED )
            if( !CanRead() )
                return;
        if( nHintId & SBX_HINT_DATACHANGED )
            if( !CanWrite() )
                return;
        // Avoid further broadcasting
        SfxBroadcaster* pSave = pCst;
        pCst = NULL;
        sal_uInt16 nSaveFlags = GetFlags();
        SetFlag( SBX_READWRITE );
        if( mpPar.Is() )
            // Register this as element 0, but don't change over the parent!
            mpPar->GetRef( 0 ) = this;
        pSave->Broadcast( SbxHint( nHintId, this ) );
        delete pCst; // who knows already, onto which thoughts someone comes?
        pCst = pSave;
        SetFlags( nSaveFlags );
    }
}

SbxInfo* SbxVariable::GetInfo()
{
    if( !pInfo )
    {
        Broadcast( SBX_HINT_INFOWANTED );
        if( pInfo.Is() )
            SetModified( sal_True );
    }
    return pInfo;
}

void SbxVariable::SetInfo( SbxInfo* p )
{
    pInfo = p;
}

void SbxVariable::SetParameters( SbxArray* p )
{
    mpPar = p;
}


/////////////////////////// Name of the variables ///////////////////////////

void SbxVariable::SetName( const XubString& rName )
{
    maName = rName;
    nHash = MakeHashCode( rName );
}

const XubString& SbxVariable::GetName( SbxNameType t ) const
{
    static char cSuffixes[] = "  %&!#@ $";
    if( t == SbxNAME_NONE )
        return maName;
    // Request parameter-information (not for objects)
    ((SbxVariable*)this)->GetInfo();
    // Append nothing, if it is a simple property (no empty brackets)
    if( !pInfo
     || ( !pInfo->aParams.Count() && GetClass() == SbxCLASS_PROPERTY ) )
        return maName;
    xub_Unicode cType = ' ';
    XubString aTmp( maName );
    // short type? Then fetch it, posible this is 0.
    SbxDataType et = GetType();
    if( t == SbxNAME_SHORT_TYPES )
    {
        if( et <= SbxSTRING )
            cType = cSuffixes[ et ];
        if( cType != ' ' )
            aTmp += cType;
    }
    aTmp += '(';
    for( sal_uInt16 i = 0; i < pInfo->aParams.Count(); i++ )
    {
        const SbxParamInfo* q = pInfo->aParams.GetObject( i );
        int nt = q->eType & 0x0FFF;
        if( i )
            aTmp += ',';
        if( q->nFlags & SBX_OPTIONAL )
            aTmp += String( SbxRes( STRING_OPTIONAL ) );
        if( q->eType & SbxBYREF )
            aTmp += String( SbxRes( STRING_BYREF ) );
        aTmp += q->aName;
        cType = ' ';
        // short type? Then fetch it, posible this is 0.
        if( t == SbxNAME_SHORT_TYPES )
        {
            if( nt <= SbxSTRING )
                cType = cSuffixes[ nt ];
        }
        if( cType != ' ' )
        {
            aTmp += cType;
            if( q->eType & SbxARRAY )
                aTmp.AppendAscii( "()" );
        }
        else
        {
            if( q->eType & SbxARRAY )
                aTmp.AppendAscii( "()" );
            // long type?
            if( t != SbxNAME_SHORT )
            {
                aTmp += String( SbxRes( STRING_AS ) );
                if( nt < 32 )
                    aTmp += String( SbxRes(
                        sal::static_int_cast< sal_uInt16 >( STRING_TYPES + nt ) ) );
                else
                    aTmp += String( SbxRes( STRING_ANY ) );
            }
        }
    }
    aTmp += ')';
    // Long type? Then fetch it
    if( t == SbxNAME_LONG_TYPES && et != SbxEMPTY )
    {
        aTmp += String( SbxRes( STRING_AS ) );
        if( et < 32 )
            aTmp += String( SbxRes(
                sal::static_int_cast< sal_uInt16 >( STRING_TYPES + et ) ) );
        else
            aTmp += String( SbxRes( STRING_ANY ) );
    }
    ((SbxVariable*) this)->aToolString = aTmp;
    return aToolString;
}

// Create a simple hashcode: the first six characters were evaluated.

sal_uInt16 SbxVariable::MakeHashCode( const XubString& rName )
{
    sal_uInt16 n = 0;
    sal_uInt16 nLen = rName.Len();
    if( nLen > 6 )
        nLen = 6;
    const xub_Unicode* p = rName.GetBuffer();
    while( nLen-- )
    {
        sal_uInt8 c = (sal_uInt8)*p;
        p++;
        // If we have a commen sigen break!!
        if( c >= 0x80 )
            return 0;
        n = sal::static_int_cast< sal_uInt16 >( ( n << 3 ) + toupper( c ) );
    }
    return n;
}

////////////////////////////// Operators ////////////////////////////////

SbxVariable& SbxVariable::operator=( const SbxVariable& r )
{
    SbxValue::operator=( r );
    delete mpSbxVariableImpl;
    if( r.mpSbxVariableImpl != NULL )
    {
        mpSbxVariableImpl = new SbxVariableImpl( *r.mpSbxVariableImpl );
        if( mpSbxVariableImpl->m_xComListener.is() )
            registerComListenerVariableForBasic( this, mpSbxVariableImpl->m_pComListenerParentBasic );
    }
    else
        mpSbxVariableImpl = NULL;
    return *this;
}

//////////////////////////////// Conversion ////////////////////////////////

SbxDataType SbxVariable::GetType() const
{
    if( aData.eType == SbxOBJECT )
        return aData.pObj ? aData.pObj->GetType() : SbxOBJECT;
    else if( aData.eType == SbxVARIANT )
        return aData.pObj ? aData.pObj->GetType() : SbxVARIANT;
    else
        return aData.eType;
}

SbxClassType SbxVariable::GetClass() const
{
    return SbxCLASS_VARIABLE;
}

void SbxVariable::SetModified( sal_Bool b )
{
    if( IsSet( SBX_NO_MODIFY ) )
        return;
    SbxBase::SetModified( b );
    if( pParent && pParent != this ) //??? HotFix: Recursion out here MM
        pParent->SetModified( b );
}

void SbxVariable::SetParent( SbxObject* p )
{
#ifdef DBG_UTIL
    // Will the parent of a SbxObject be set?
    if ( p && ISA(SbxObject) )
    {
        // then this had to be a child of the new parent
        sal_Bool bFound = sal_False;
        SbxArray *pChilds = p->GetObjects();
        if ( pChilds )
        {
            for ( sal_uInt16 nIdx = 0; !bFound && nIdx < pChilds->Count(); ++nIdx )
                bFound = ( this == pChilds->Get(nIdx) );
        }
        if ( !bFound )
        {
            String aMsg = String::CreateFromAscii( "dangling: [" );
            aMsg += GetName();
            aMsg.AppendAscii( "].SetParent([" );
            aMsg += p->GetName();
            aMsg.AppendAscii( "])" );
            rtl::OString aBStr(rtl::OUStringToOString(aMsg, RTL_TEXTENCODING_ASCII_US));
            DbgOut( aBStr.getStr(), DBG_OUT_WARNING, __FILE__, __LINE__);
        }
    }
#endif

    pParent = p;
}

SbxVariableImpl* SbxVariable::getImpl( void )
{
    if( mpSbxVariableImpl == NULL )
        mpSbxVariableImpl = new SbxVariableImpl();
    return mpSbxVariableImpl;
}

const String& SbxVariable::GetDeclareClassName( void )
{
    SbxVariableImpl* pImpl = getImpl();
    return pImpl->m_aDeclareClassName;
}

void SbxVariable::SetDeclareClassName( const String& rDeclareClassName )
{
    SbxVariableImpl* pImpl = getImpl();
    pImpl->m_aDeclareClassName = rDeclareClassName;
}

void SbxVariable::SetComListener( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xComListener,
                                  StarBASIC* pParentBasic )
{
    SbxVariableImpl* pImpl = getImpl();
    pImpl->m_xComListener = xComListener;
    pImpl->m_pComListenerParentBasic = pParentBasic;
    registerComListenerVariableForBasic( this, pParentBasic );
}

void SbxVariable::ClearComListener( void )
{
    SbxVariableImpl* pImpl = getImpl();
    pImpl->m_xComListener.clear();
}


////////////////////////////// Loading/Saving /////////////////////////////

sal_Bool SbxVariable::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    sal_uInt16 nType;
    sal_uInt8 cMark;
    rStrm >> cMark;
    if( cMark == 0xFF )
    {
        if( !SbxValue::LoadData( rStrm, nVer ) )
            return sal_False;
        rStrm.ReadByteString( maName, RTL_TEXTENCODING_ASCII_US );
        sal_uInt32 nTemp;
        rStrm >> nTemp;
        nUserData = nTemp;
    }
    else
    {
        rStrm.SeekRel( -1L );
        rStrm >> nType;
        rStrm.ReadByteString( maName, RTL_TEXTENCODING_ASCII_US );
        sal_uInt32 nTemp;
        rStrm >> nTemp;
        nUserData = nTemp;
        // correction: old methods have instead of SbxNULL now SbxEMPTY
        if( nType == SbxNULL && GetClass() == SbxCLASS_METHOD )
            nType = SbxEMPTY;
        SbxValues aTmp;
        String aTmpString;
        ::rtl::OUString aVal;
        aTmp.eType = aData.eType = (SbxDataType) nType;
        aTmp.pOUString = &aVal;
        switch( nType )
        {
            case SbxBOOL:
            case SbxERROR:
            case SbxINTEGER:
                rStrm >> aTmp.nInteger; break;
            case SbxLONG:
                rStrm >> aTmp.nLong; break;
            case SbxSINGLE:
            {
                // Floats as ASCII
                rStrm.ReadByteString( aTmpString, RTL_TEXTENCODING_ASCII_US );
                double d;
                SbxDataType t;
                if( ImpScan( aTmpString, d, t, NULL ) != SbxERR_OK || t == SbxDOUBLE )
                {
                    aTmp.nSingle = 0;
                    return sal_False;
                }
                aTmp.nSingle = (float) d;
                break;
            }
            case SbxDATE:
            case SbxDOUBLE:
            {
                // Floats as ASCII
                rStrm.ReadByteString( aTmpString, RTL_TEXTENCODING_ASCII_US );
                SbxDataType t;
                if( ImpScan( aTmpString, aTmp.nDouble, t, NULL ) != SbxERR_OK )
                {
                    aTmp.nDouble = 0;
                    return sal_False;
                }
                break;
            }
            case SbxSTRING:
                rStrm.ReadByteString( aTmpString, RTL_TEXTENCODING_ASCII_US );
                aVal = aTmpString;
                break;
            case SbxEMPTY:
            case SbxNULL:
                break;
            default:
                aData.eType = SbxNULL;
                DBG_ASSERT( !this, "Nicht unterstuetzer Datentyp geladen" );
                return sal_False;
        }
        // putt value
        if( nType != SbxNULL && nType != SbxEMPTY && !Put( aTmp ) )
            return sal_False;
    }
    rStrm >> cMark;
    // cMark is also a version number!
    // 1: initial version
    // 2: with nUserData
    if( cMark )
    {
        if( cMark > 2 )
            return sal_False;
        pInfo = new SbxInfo;
        pInfo->LoadData( rStrm, (sal_uInt16) cMark );
    }
    // Load private data only, if it is a SbxVariable
    if( GetClass() == SbxCLASS_VARIABLE && !LoadPrivateData( rStrm, nVer ) )
        return sal_False;
    ((SbxVariable*) this)->Broadcast( SBX_HINT_DATACHANGED );
    nHash =  MakeHashCode( maName );
    SetModified( sal_True );
    return sal_True;
}

sal_Bool SbxVariable::StoreData( SvStream& rStrm ) const
{
    rStrm << (sal_uInt8) 0xFF;      // Marker
    sal_Bool bValStore;
    if( this->IsA( TYPE(SbxMethod) ) )
    {
        // #50200 Avoid that objects , which during the runtime
        // as return-value are saved in the method as a value were saved
        SbxVariable* pThis = (SbxVariable*)this;
        sal_uInt16 nSaveFlags = GetFlags();
        pThis->SetFlag( SBX_WRITE );
        pThis->SbxValue::Clear();
        pThis->SetFlags( nSaveFlags );

        // So that the method will not be executed in any case!
        // CAST, to avoid const!
        pThis->SetFlag( SBX_NO_BROADCAST );
        bValStore = SbxValue::StoreData( rStrm );
        pThis->ResetFlag( SBX_NO_BROADCAST );
    }
    else
        bValStore = SbxValue::StoreData( rStrm );
    if( !bValStore )
        return sal_False;
    rStrm.WriteByteString( maName, RTL_TEXTENCODING_ASCII_US );
    rStrm << (sal_uInt32)nUserData;
    if( pInfo.Is() )
    {
        rStrm << (sal_uInt8) 2;     // Version 2: with UserData!
        pInfo->StoreData( rStrm );
    }
    else
        rStrm << (sal_uInt8) 0;
    // Save private data only, if it is a SbxVariable
    if( GetClass() == SbxCLASS_VARIABLE )
        return StorePrivateData( rStrm );
    else
        return sal_True;
}

////////////////////////////// SbxInfo ///////////////////////////////////

SbxInfo::SbxInfo() : aHelpFile(), nHelpId( 0 ), aParams()
{}

SbxInfo::SbxInfo( const String& r, sal_uInt32 n )
       : aHelpFile( r ), nHelpId( n ), aParams()
{}

////////////////////////////// SbxAlias //////////////////////////////////

SbxAlias::SbxAlias( const SbxAlias& r )
        : SvRefBase( r ), SbxVariable( r ),
          SfxListener( r ), xAlias( r.xAlias )
{}

SbxAlias& SbxAlias::operator=( const SbxAlias& r )
{
    xAlias = r.xAlias;
    return *this;
}

SbxAlias::~SbxAlias()
{
    if( xAlias.Is() )
        EndListening( xAlias->GetBroadcaster() );
}

void SbxAlias::Broadcast( sal_uIntPtr nHt )
{
    if( xAlias.Is() )
    {
        xAlias->SetParameters( GetParameters() );
        if( nHt == SBX_HINT_DATAWANTED )
            SbxVariable::operator=( *xAlias );
        else if( nHt == SBX_HINT_DATACHANGED || nHt == SBX_HINT_CONVERTED )
            *xAlias = *this;
        else if( nHt == SBX_HINT_INFOWANTED )
        {
            xAlias->Broadcast( nHt );
            pInfo = xAlias->GetInfo();
        }
    }
}

void SbxAlias::SFX_NOTIFY( SfxBroadcaster&, const TypeId&,
                           const SfxHint& rHint, const TypeId& )
{
    const SbxHint* p = PTR_CAST(SbxHint,&rHint);
    if( p && p->GetId() == SBX_HINT_DYING )
    {
        xAlias.Clear();
        // delete the alias?
        if( pParent )
            pParent->Remove( this );
    }
}

void SbxVariable::Dump( SvStream& rStrm, sal_Bool bFill )
{
    rtl::OString aBNameStr(rtl::OUStringToOString(GetName( SbxNAME_SHORT_TYPES ), RTL_TEXTENCODING_ASCII_US));
    rStrm << "Variable( "
          << rtl::OString::valueOf(reinterpret_cast<sal_Int64>(this)).getStr() << "=="
          << aBNameStr.getStr();
    rtl::OString aBParentNameStr(rtl::OUStringToOString(GetParent()->GetName(), RTL_TEXTENCODING_ASCII_US));
    if ( GetParent() )
        rStrm << " in parent '" << aBParentNameStr.getStr() << "'";
    else
        rStrm << " no parent";
    rStrm << " ) ";

    // output also the object at object-vars
    if ( GetValues_Impl().eType == SbxOBJECT &&
            GetValues_Impl().pObj &&
            GetValues_Impl().pObj != this &&
            GetValues_Impl().pObj != GetParent() )
    {
        rStrm << " contains ";
        ((SbxObject*) GetValues_Impl().pObj)->Dump( rStrm, bFill );
    }
    else
        rStrm << endl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
