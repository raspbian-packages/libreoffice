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


// INCLUDE ---------------------------------------------------------------

#include "dpsave.hxx"
#include "dpdimsave.hxx"
#include "miscuno.hxx"
#include "scerrors.hxx"
#include "unonames.hxx"
#include "global.hxx"

#include <sal/types.h>

#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#include <com/sun/star/sheet/TableFilterField.hpp>
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#include <com/sun/star/sheet/XMembersSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include "dptabsrc.hxx"
#include "dpglobal.hxx"
using namespace ScDPGlobal;
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
using namespace com::sun::star::sheet;

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OUStringHash;
using ::std::auto_ptr;

#define SC_DPSAVEMODE_NO 0
#define SC_DPSAVEMODE_YES 1
#define SC_DPSAVEMODE_DONTKNOW 2

void lcl_SetBoolProperty( const uno::Reference<beans::XPropertySet>& xProp,
                            const rtl::OUString& rName, sal_Bool bValue )
{
    //! move to ScUnoHelpFunctions?

    xProp->setPropertyValue( rName, uno::Any( &bValue, getBooleanCppuType() ) );
}

ScDPSaveMember::ScDPSaveMember(const ::rtl::OUString& rName) :
    aName( rName ),
    mpLayoutName(NULL),
    nVisibleMode( SC_DPSAVEMODE_DONTKNOW ),
    nShowDetailsMode( SC_DPSAVEMODE_DONTKNOW )
{
}

ScDPSaveMember::ScDPSaveMember(const ScDPSaveMember& r) :
    aName( r.aName ),
    mpLayoutName(NULL),
    nVisibleMode( r.nVisibleMode ),
    nShowDetailsMode( r.nShowDetailsMode )
{
    if (r.mpLayoutName.get())
        mpLayoutName.reset(new OUString(*r.mpLayoutName));
}

ScDPSaveMember::~ScDPSaveMember()
{
}

bool ScDPSaveMember::operator== ( const ScDPSaveMember& r ) const
{
    if ( aName            != r.aName           ||
         nVisibleMode     != r.nVisibleMode    ||
         nShowDetailsMode != r.nShowDetailsMode )
        return false;

    return true;
}

bool ScDPSaveMember::HasIsVisible() const
{
    return nVisibleMode != SC_DPSAVEMODE_DONTKNOW;
}

void ScDPSaveMember::SetIsVisible(bool bSet)
{
    nVisibleMode = bSet;
}

bool ScDPSaveMember::HasShowDetails() const
{
    return nShowDetailsMode != SC_DPSAVEMODE_DONTKNOW;
}

void ScDPSaveMember::SetShowDetails(bool bSet)
{
    nShowDetailsMode = bSet;
}

void ScDPSaveMember::SetName( const ::rtl::OUString& rNew )
{
    // Used only if the source member was renamed (groups).
    // For UI renaming of members, a layout name must be used.

    aName = rNew;
}

void ScDPSaveMember::SetLayoutName( const OUString& rName )
{
    mpLayoutName.reset(new OUString(rName));
}

const OUString* ScDPSaveMember::GetLayoutName() const
{
    return mpLayoutName.get();
}

void ScDPSaveMember::RemoveLayoutName()
{
    mpLayoutName.reset(NULL);
}

void ScDPSaveMember::WriteToSource( const uno::Reference<uno::XInterface>& xMember, sal_Int32 nPosition )
{
    uno::Reference<beans::XPropertySet> xMembProp( xMember, uno::UNO_QUERY );
    OSL_ENSURE( xMembProp.is(), "no properties at member" );
    if ( xMembProp.is() )
    {
        // exceptions are caught at ScDPSaveData::WriteToSource

        if ( nVisibleMode != SC_DPSAVEMODE_DONTKNOW )
            lcl_SetBoolProperty( xMembProp,
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ISVISIBLE)), (bool)nVisibleMode );

        if ( nShowDetailsMode != SC_DPSAVEMODE_DONTKNOW )
            lcl_SetBoolProperty( xMembProp,
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_SHOWDETAILS)), (bool)nShowDetailsMode );

        if (mpLayoutName.get())
            ScUnoHelpFunctions::SetOptionalPropertyValue(xMembProp, SC_UNO_LAYOUTNAME, *mpLayoutName);

        if ( nPosition >= 0 )
            ScUnoHelpFunctions::SetOptionalPropertyValue(xMembProp, DP_PROP_POSITION, nPosition);
    }
}

ScDPSaveDimension::ScDPSaveDimension(const ::rtl::OUString& rName, bool bDataLayout) :
    aName( rName ),
    pSelectedPage( NULL ),
    mpLayoutName(NULL),
    mpSubtotalName(NULL),
    bIsDataLayout( bDataLayout ),
    bDupFlag( false ),
    nOrientation( sheet::DataPilotFieldOrientation_HIDDEN ),
    nFunction( sheet::GeneralFunction_AUTO ),
    nUsedHierarchy( -1 ),
    nShowEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
    bSubTotalDefault( true ),
    nSubTotalCount( 0 ),
    pSubTotalFuncs( NULL ),
    pReferenceValue( NULL ),
    pSortInfo( NULL ),
    pAutoShowInfo( NULL ),
    pLayoutInfo( NULL )
{
}

ScDPSaveDimension::ScDPSaveDimension(const ScDPSaveDimension& r) :
    aName( r.aName ),
    mpLayoutName(NULL),
    mpSubtotalName(NULL),
    bIsDataLayout( r.bIsDataLayout ),
    bDupFlag( r.bDupFlag ),
    nOrientation( r.nOrientation ),
    nFunction( r.nFunction ),
    nUsedHierarchy( r.nUsedHierarchy ),
    nShowEmptyMode( r.nShowEmptyMode ),
    bSubTotalDefault( r.bSubTotalDefault ),
    nSubTotalCount( r.nSubTotalCount ),
    pSubTotalFuncs( NULL )
{
    if ( nSubTotalCount && r.pSubTotalFuncs )
    {
        pSubTotalFuncs = new sal_uInt16[nSubTotalCount];
        for (long nSub=0; nSub<nSubTotalCount; nSub++)
            pSubTotalFuncs[nSub] = r.pSubTotalFuncs[nSub];
    }

    for (MemberList::const_iterator i=r.maMemberList.begin(); i != r.maMemberList.end() ; ++i)
    {
        const ::rtl::OUString& rName =  (*i)->GetName();
        ScDPSaveMember* pNew = new ScDPSaveMember( **i );
        maMemberHash[rName] = pNew;
        maMemberList.push_back( pNew );
    }
    if (r.pReferenceValue)
        pReferenceValue = new sheet::DataPilotFieldReference( *(r.pReferenceValue) );
    else
        pReferenceValue = NULL;
    if (r.pSortInfo)
        pSortInfo = new sheet::DataPilotFieldSortInfo( *(r.pSortInfo) );
    else
        pSortInfo = NULL;
    if (r.pAutoShowInfo)
        pAutoShowInfo = new sheet::DataPilotFieldAutoShowInfo( *(r.pAutoShowInfo) );
    else
        pAutoShowInfo = NULL;
    if (r.pLayoutInfo)
        pLayoutInfo = new sheet::DataPilotFieldLayoutInfo( *(r.pLayoutInfo) );
    else
        pLayoutInfo = NULL;
    if (r.pSelectedPage)
        pSelectedPage = new ::rtl::OUString( *(r.pSelectedPage) );
    else
        pSelectedPage = NULL;
    if (r.mpLayoutName.get())
        mpLayoutName.reset(new OUString(*r.mpLayoutName));
    if (r.mpSubtotalName.get())
        mpSubtotalName.reset(new OUString(*r.mpSubtotalName));
}

ScDPSaveDimension::~ScDPSaveDimension()
{
    for (MemberHash::const_iterator i=maMemberHash.begin(); i != maMemberHash.end() ; ++i)
        delete i->second;
    delete pReferenceValue;
    delete pSortInfo;
    delete pAutoShowInfo;
    delete pLayoutInfo;
    delete pSelectedPage;
    delete [] pSubTotalFuncs;
}

bool ScDPSaveDimension::operator== ( const ScDPSaveDimension& r ) const
{
    if ( aName            != r.aName            ||
         bIsDataLayout    != r.bIsDataLayout    ||
         bDupFlag         != r.bDupFlag         ||
         nOrientation     != r.nOrientation     ||
         nFunction        != r.nFunction        ||
         nUsedHierarchy   != r.nUsedHierarchy   ||
         nShowEmptyMode   != r.nShowEmptyMode   ||
         bSubTotalDefault != r.bSubTotalDefault ||
         nSubTotalCount   != r.nSubTotalCount    )
        return false;

    if ( nSubTotalCount && ( !pSubTotalFuncs || !r.pSubTotalFuncs ) ) // should not happen
        return false;

    long i;
    for (i=0; i<nSubTotalCount; i++)
        if ( pSubTotalFuncs[i] != r.pSubTotalFuncs[i] )
            return false;

    if (maMemberHash.size() != r.maMemberHash.size() )
        return false;

    MemberList::const_iterator a=maMemberList.begin();
    MemberList::const_iterator b=r.maMemberList.begin();
    for (; a != maMemberList.end() ; ++a, ++b)
        if (!(**a == **b))
            return false;

    if ( this->HasCurrentPage() && r.HasCurrentPage() )
    {
        if ( this->GetCurrentPage() != r.GetCurrentPage() )
        {
            return false;
        }
    }
    else if ( this->HasCurrentPage() || r.HasCurrentPage() )
    {
        return false;
    }
    if( pReferenceValue && r.pReferenceValue )
    {
        if ( !(*pReferenceValue == *r.pReferenceValue) )
        {
            return false;
        }
    }
    else if ( pReferenceValue || r.pReferenceValue )
    {
        return false;
    }
    if( this->pSortInfo && r.pSortInfo )
    {
        if ( !(*this->pSortInfo == *r.pSortInfo) )
        {
            return false;
        }
    }
    else if ( this->pSortInfo || r.pSortInfo )
    {
        return false;
    }
    if( this->pAutoShowInfo && r.pAutoShowInfo )
    {
        if ( !(*this->pAutoShowInfo == *r.pAutoShowInfo) )
        {
            return false;
        }
    }
    else if ( this->pAutoShowInfo || r.pAutoShowInfo )
    {
        return false;
    }

    return true;
}

void ScDPSaveDimension::AddMember(ScDPSaveMember* pMember)
{
    const ::rtl::OUString & rName = pMember->GetName();
    MemberHash::iterator aExisting = maMemberHash.find( rName );
    if ( aExisting == maMemberHash.end() )
    {
        std::pair< const ::rtl::OUString, ScDPSaveMember *> key( rName, pMember );
        maMemberHash.insert ( key );
    }
    else
    {
        maMemberList.remove( aExisting->second );
        delete aExisting->second;
        aExisting->second = pMember;
    }
    maMemberList.push_back( pMember );
}

void ScDPSaveDimension::SetName( const ::rtl::OUString& rNew )
{
    // Used only if the source dim was renamed (groups).
    // For UI renaming of dimensions, the layout name must be used.

    aName = rNew;
}

void ScDPSaveDimension::SetOrientation(sal_uInt16 nNew)
{
    nOrientation = nNew;
}

void ScDPSaveDimension::SetSubTotals(long nCount, const sal_uInt16* pFuncs)
{
    if (pSubTotalFuncs)
        delete [] pSubTotalFuncs;
    nSubTotalCount = nCount;
    if ( nCount && pFuncs )
    {
        pSubTotalFuncs = new sal_uInt16[nCount];
        for (long i=0; i<nCount; i++)
            pSubTotalFuncs[i] = pFuncs[i];
    }
    else
        pSubTotalFuncs = NULL;

    bSubTotalDefault = false;
}

bool ScDPSaveDimension::HasShowEmpty() const
{
    return nShowEmptyMode != SC_DPSAVEMODE_DONTKNOW;
}

void ScDPSaveDimension::SetShowEmpty(bool bSet)
{
    nShowEmptyMode = bSet;
}

void ScDPSaveDimension::SetFunction(sal_uInt16 nNew)
{
    nFunction = nNew;
}

void ScDPSaveDimension::SetUsedHierarchy(long nNew)
{
    nUsedHierarchy = nNew;
}

void ScDPSaveDimension::SetSubtotalName(const OUString& rName)
{
    mpSubtotalName.reset(new OUString(rName));
}

const OUString* ScDPSaveDimension::GetSubtotalName() const
{
    return mpSubtotalName.get();
}

bool ScDPSaveDimension::IsMemberNameInUse(const OUString& rName) const
{
    MemberList::const_iterator itr = maMemberList.begin(), itrEnd = maMemberList.end();
    for (; itr != itrEnd; ++itr)
    {
        const ScDPSaveMember* pMem = *itr;
        if (rName.equalsIgnoreAsciiCase(pMem->GetName()))
            return true;

        const OUString* pLayoutName = pMem->GetLayoutName();
        if (pLayoutName && rName.equalsIgnoreAsciiCase(*pLayoutName))
            return true;
    }
    return false;
}

void ScDPSaveDimension::SetLayoutName(const OUString& rName)
{
    mpLayoutName.reset(new OUString(rName));
}

const OUString* ScDPSaveDimension::GetLayoutName() const
{
    return mpLayoutName.get();
}

void ScDPSaveDimension::RemoveLayoutName()
{
    mpLayoutName.reset(NULL);
}

void ScDPSaveDimension::SetReferenceValue(const sheet::DataPilotFieldReference* pNew)
{
    delete pReferenceValue;
    if (pNew)
        pReferenceValue = new sheet::DataPilotFieldReference(*pNew);
    else
        pReferenceValue = NULL;
}

void ScDPSaveDimension::SetSortInfo(const sheet::DataPilotFieldSortInfo* pNew)
{
    delete pSortInfo;
    if (pNew)
        pSortInfo = new sheet::DataPilotFieldSortInfo(*pNew);
    else
        pSortInfo = NULL;
}

void ScDPSaveDimension::SetAutoShowInfo(const sheet::DataPilotFieldAutoShowInfo* pNew)
{
    delete pAutoShowInfo;
    if (pNew)
        pAutoShowInfo = new sheet::DataPilotFieldAutoShowInfo(*pNew);
    else
        pAutoShowInfo = NULL;
}

void ScDPSaveDimension::SetLayoutInfo(const sheet::DataPilotFieldLayoutInfo* pNew)
{
    delete pLayoutInfo;
    if (pNew)
        pLayoutInfo = new sheet::DataPilotFieldLayoutInfo(*pNew);
    else
        pLayoutInfo = NULL;
}

void ScDPSaveDimension::SetCurrentPage( const ::rtl::OUString* pPage )
{
    delete pSelectedPage;
    if (pPage)
        pSelectedPage = new ::rtl::OUString( *pPage );
    else
        pSelectedPage = NULL;
}

bool ScDPSaveDimension::HasCurrentPage() const
{
    return ( pSelectedPage != NULL );
}

const ::rtl::OUString& ScDPSaveDimension::GetCurrentPage() const
{
    static const ::rtl::OUString emptyOUString = ::rtl::OUString();
    if (pSelectedPage)
        return *pSelectedPage;
    return emptyOUString;
}

ScDPSaveMember* ScDPSaveDimension::GetExistingMemberByName(const ::rtl::OUString& rName)
{
    MemberHash::const_iterator res = maMemberHash.find (rName);
    if (res != maMemberHash.end())
        return res->second;
    return NULL;
}

ScDPSaveMember* ScDPSaveDimension::GetMemberByName(const ::rtl::OUString& rName)
{
    MemberHash::const_iterator res = maMemberHash.find (rName);
    if (res != maMemberHash.end())
        return res->second;

    ScDPSaveMember* pNew = new ScDPSaveMember( rName );
    maMemberHash[rName] = pNew;
    maMemberList.push_back( pNew );
    return pNew;
}

void ScDPSaveDimension::SetMemberPosition( const ::rtl::OUString& rName, sal_Int32 nNewPos )
{
    ScDPSaveMember* pMember = GetMemberByName( rName ); // make sure it exists and is in the hash

    maMemberList.remove( pMember );

    MemberList::iterator aIter = maMemberList.begin();
    for (sal_Int32 i=0; i<nNewPos && aIter != maMemberList.end(); i++)
        ++aIter;
    maMemberList.insert( aIter, pMember );
}

void ScDPSaveDimension::WriteToSource( const uno::Reference<uno::XInterface>& xDim )
{
    uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
    OSL_ENSURE( xDimProp.is(), "no properties at dimension" );
    if ( xDimProp.is() )
    {
        // exceptions are caught at ScDPSaveData::WriteToSource
        uno::Any aAny;

        sheet::DataPilotFieldOrientation eOrient = (sheet::DataPilotFieldOrientation)nOrientation;
        aAny <<= eOrient;
        xDimProp->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ORIENTATION)), aAny );

        sheet::GeneralFunction eFunc = (sheet::GeneralFunction)nFunction;
        aAny <<= eFunc;
        xDimProp->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_FUNCTION)), aAny );

        if ( nUsedHierarchy >= 0 )
        {
            aAny <<= (sal_Int32)nUsedHierarchy;
            xDimProp->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_USEDHIERARCHY)), aAny );
        }

        if ( pReferenceValue )
        {
            aAny <<= *pReferenceValue;
            xDimProp->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_REFVALUE)), aAny );
        }

        uno::Sequence<sheet::TableFilterField> aFilter;
        // set the selected page field only if the dimension is used as page dimension
        if ( pSelectedPage && nOrientation == sheet::DataPilotFieldOrientation_PAGE )
        {
            // single filter field: first field equal to selected string
            sheet::TableFilterField aField( sheet::FilterConnection_AND, 0,
                    sheet::FilterOperator_EQUAL, false, 0.0, *pSelectedPage );
            aFilter = uno::Sequence<sheet::TableFilterField>( &aField, 1 );
        }
        // else keep empty sequence

        ScUnoHelpFunctions::SetOptionalPropertyValue(xDimProp, DP_PROP_FILTER, aFilter);
        if (mpLayoutName.get())
            ScUnoHelpFunctions::SetOptionalPropertyValue(xDimProp, SC_UNO_LAYOUTNAME, *mpLayoutName);

        const OUString* pSubTotalName = GetSubtotalName();
        if (pSubTotalName)
            // Custom subtotal name, with '?' being replaced by the visible field name later.
            ScUnoHelpFunctions::SetOptionalPropertyValue(xDimProp, SC_UNO_FIELD_SUBTOTALNAME, *pSubTotalName);
    }

    // Level loop outside of maMemberList loop
    // because SubTotals have to be set independently of known members

    long nCount = maMemberHash.size();

    long nHierCount = 0;
    uno::Reference<container::XIndexAccess> xHiers;
    uno::Reference<sheet::XHierarchiesSupplier> xHierSupp( xDim, uno::UNO_QUERY );
    if ( xHierSupp.is() )
    {
        uno::Reference<container::XNameAccess> xHiersName = xHierSupp->getHierarchies();
        xHiers = new ScNameToIndexAccess( xHiersName );
        nHierCount = xHiers->getCount();
    }

    sal_Bool bHasHiddenMember = false;

    for (long nHier=0; nHier<nHierCount; nHier++)
    {
        uno::Reference<uno::XInterface> xHierarchy = ScUnoHelpFunctions::AnyToInterface( xHiers->getByIndex(nHier) );

        long nLevCount = 0;
        uno::Reference<container::XIndexAccess> xLevels;
        uno::Reference<sheet::XLevelsSupplier> xLevSupp( xHierarchy, uno::UNO_QUERY );
        if ( xLevSupp.is() )
        {
            uno::Reference<container::XNameAccess> xLevelsName = xLevSupp->getLevels();
            xLevels = new ScNameToIndexAccess( xLevelsName );
            nLevCount = xLevels->getCount();
        }

        for (long nLev=0; nLev<nLevCount; nLev++)
        {
            uno::Reference<uno::XInterface> xLevel = ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex(nLev) );
            uno::Reference<beans::XPropertySet> xLevProp( xLevel, uno::UNO_QUERY );
            OSL_ENSURE( xLevProp.is(), "no properties at level" );
            if ( xLevProp.is() )
            {
                uno::Any aAny;
                if ( !bSubTotalDefault )
                {
                    if ( !pSubTotalFuncs )
                        nSubTotalCount = 0;

                    uno::Sequence<sheet::GeneralFunction> aSeq(nSubTotalCount);
                    sheet::GeneralFunction* pArray = aSeq.getArray();
                    for (long i=0; i<nSubTotalCount; i++)
                        pArray[i] = (sheet::GeneralFunction)pSubTotalFuncs[i];
                    aAny <<= aSeq;
                    xLevProp->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_SUBTOTALS)), aAny );
                }
                if ( nShowEmptyMode != SC_DPSAVEMODE_DONTKNOW )
                    lcl_SetBoolProperty( xLevProp,
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_SHOWEMPTY)), (bool)nShowEmptyMode );

                if ( pSortInfo )
                    ScUnoHelpFunctions::SetOptionalPropertyValue(xLevProp, SC_UNO_SORTING, *pSortInfo);

                if ( pAutoShowInfo )
                    ScUnoHelpFunctions::SetOptionalPropertyValue(xLevProp, SC_UNO_AUTOSHOW, *pAutoShowInfo);

                if ( pLayoutInfo )
                    ScUnoHelpFunctions::SetOptionalPropertyValue(xLevProp, SC_UNO_LAYOUT, *pLayoutInfo);

                // exceptions are caught at ScDPSaveData::WriteToSource
            }

            if ( nCount > 0 )
            {
                uno::Reference<sheet::XMembersSupplier> xMembSupp( xLevel, uno::UNO_QUERY );
                if ( xMembSupp.is() )
                {
                    uno::Reference<container::XNameAccess> xMembers = xMembSupp->getMembers();
                    if ( xMembers.is() )
                    {
                        sal_Int32 nPosition = -1; // set position only in manual mode
                        if ( !pSortInfo || pSortInfo->Mode == sheet::DataPilotFieldSortMode::MANUAL )
                            nPosition = 0;

                        for (MemberList::const_iterator i=maMemberList.begin(); i != maMemberList.end() ; ++i)
                        {
                            ScDPSaveMember* pMember = *i;
                            if (!pMember->GetIsVisible())
                                bHasHiddenMember = true;
                            rtl::OUString aMemberName = pMember->GetName();
                            if ( xMembers->hasByName( aMemberName ) )
                            {
                                uno::Reference<uno::XInterface> xMemberInt = ScUnoHelpFunctions::AnyToInterface(
                                    xMembers->getByName( aMemberName ) );
                                pMember->WriteToSource( xMemberInt, nPosition );

                                if ( nPosition >= 0 )
                                    ++nPosition; // increase if initialized
                            }
                            // missing member is no error
                        }
                    }
                }
            }
        }
    }

    if (xDimProp.is())
        ScUnoHelpFunctions::SetOptionalPropertyValue(xDimProp, SC_UNO_HAS_HIDDEN_MEMBER, bHasHiddenMember);
}

void ScDPSaveDimension::UpdateMemberVisibility(const boost::unordered_map<OUString, bool, OUStringHash>& rData)
{
    typedef boost::unordered_map<OUString, bool, OUStringHash> DataMap;
    MemberList::iterator itrMem = maMemberList.begin(), itrMemEnd = maMemberList.end();
    for (; itrMem != itrMemEnd; ++itrMem)
    {
        ScDPSaveMember* pMem = *itrMem;
        const ::rtl::OUString& rMemName = pMem->GetName();
        DataMap::const_iterator itr = rData.find(rMemName);
        if (itr != rData.end())
            pMem->SetIsVisible(itr->second);
    }
}

bool ScDPSaveDimension::HasInvisibleMember() const
{
    MemberList::const_iterator itrMem = maMemberList.begin(), itrMemEnd = maMemberList.end();
    for (; itrMem != itrMemEnd; ++itrMem)
    {
        const ScDPSaveMember* pMem = *itrMem;
        if (!pMem->GetIsVisible())
            return true;
    }
    return false;
}

ScDPSaveData::ScDPSaveData() :
    pDimensionData( NULL ),
    nColumnGrandMode( SC_DPSAVEMODE_DONTKNOW ),
    nRowGrandMode( SC_DPSAVEMODE_DONTKNOW ),
    nIgnoreEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
    nRepeatEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
    bFilterButton( true ),
    bDrillDown( true ),
    mbDimensionMembersBuilt(false),
    mpGrandTotalName(NULL)
{
}

ScDPSaveData::ScDPSaveData(const ScDPSaveData& r) :
    nColumnGrandMode( r.nColumnGrandMode ),
    nRowGrandMode( r.nRowGrandMode ),
    nIgnoreEmptyMode( r.nIgnoreEmptyMode ),
    nRepeatEmptyMode( r.nRepeatEmptyMode ),
    bFilterButton( r.bFilterButton ),
    bDrillDown( r.bDrillDown ),
    mbDimensionMembersBuilt(r.mbDimensionMembersBuilt),
    mpGrandTotalName(NULL)
{
    if ( r.pDimensionData )
        pDimensionData = new ScDPDimensionSaveData( *r.pDimensionData );
    else
        pDimensionData = NULL;

    aDimList = r.aDimList.clone();

    if (r.mpGrandTotalName.get())
        mpGrandTotalName.reset(new OUString(*r.mpGrandTotalName));
}

ScDPSaveData& ScDPSaveData::operator= ( const ScDPSaveData& r )
{
    if ( &r != this )
    {
        this->~ScDPSaveData();
        new( this ) ScDPSaveData ( r );
    }
    return *this;
}

bool ScDPSaveData::operator== ( const ScDPSaveData& r ) const
{
    if ( nColumnGrandMode != r.nColumnGrandMode ||
         nRowGrandMode    != r.nRowGrandMode    ||
         nIgnoreEmptyMode != r.nIgnoreEmptyMode ||
         nRepeatEmptyMode != r.nRepeatEmptyMode ||
         bFilterButton    != r.bFilterButton    ||
         bDrillDown       != r.bDrillDown       ||
         mbDimensionMembersBuilt != r.mbDimensionMembersBuilt)
        return false;

    if ( pDimensionData || r.pDimensionData )
        if ( !pDimensionData || !r.pDimensionData || !( *pDimensionData == *r.pDimensionData ) )
            return false;

    if ( aDimList.size() != r.aDimList.size() )
        return false;

    if (aDimList != r.aDimList)
        return false;

    if (mpGrandTotalName.get())
    {
        if (!r.mpGrandTotalName.get())
            return false;
        if (!mpGrandTotalName->equals(*r.mpGrandTotalName))
            return false;
    }
    else if (r.mpGrandTotalName.get())
        return false;

    return true;
}

ScDPSaveData::~ScDPSaveData()
{
    delete pDimensionData;
}

void ScDPSaveData::SetGrandTotalName(const OUString& rName)
{
    mpGrandTotalName.reset(new OUString(rName));
}

const OUString* ScDPSaveData::GetGrandTotalName() const
{
    return mpGrandTotalName.get();
}

ScDPSaveDimension* ScDPSaveData::GetDimensionByName(const ::rtl::OUString& rName)
{
    boost::ptr_vector<ScDPSaveDimension>::const_iterator iter;
    for (iter = aDimList.begin(); iter != aDimList.end(); ++iter)
    {
        if (iter->GetName() == rName && !iter->IsDataLayout() )
            return const_cast<ScDPSaveDimension*>(&(*iter));
    }

    ScDPSaveDimension* pNew = new ScDPSaveDimension( rName, false );
    aDimList.push_back(pNew);
    return pNew;
}

ScDPSaveDimension* ScDPSaveData::GetExistingDimensionByName(const ::rtl::OUString& rName) const
{
    boost::ptr_vector<ScDPSaveDimension>::const_iterator iter;
    for (iter = aDimList.begin(); iter != aDimList.end(); ++iter)
    {
        if (iter->GetName() == rName && !iter->IsDataLayout() )
            return const_cast<ScDPSaveDimension*>(&(*iter));
    }
    return NULL; // don't create new
}

ScDPSaveDimension* ScDPSaveData::GetNewDimensionByName(const ::rtl::OUString& rName)
{
    boost::ptr_vector<ScDPSaveDimension>::const_iterator iter;
    for (iter = aDimList.begin(); iter != aDimList.end(); ++iter)
    {
        if (iter->GetName() == rName && !iter->IsDataLayout() )
            return DuplicateDimension(rName);
    }

    ScDPSaveDimension* pNew = new ScDPSaveDimension( rName, false );
    aDimList.push_back(pNew);
    return pNew;
}

ScDPSaveDimension* ScDPSaveData::GetDataLayoutDimension()
{
    ScDPSaveDimension* pDim = GetExistingDataLayoutDimension();
    if (pDim)
        return pDim;

    ScDPSaveDimension* pNew = new ScDPSaveDimension( ::rtl::OUString(), true );
    aDimList.push_back(pNew);
    return pNew;
}

ScDPSaveDimension* ScDPSaveData::GetExistingDataLayoutDimension() const
{
    boost::ptr_vector<ScDPSaveDimension>::const_iterator iter;
    for (iter = aDimList.begin(); iter != aDimList.end(); ++iter)
    {
        if ( iter->IsDataLayout() )
            return const_cast<ScDPSaveDimension*>(&(*iter));
    }
    return NULL;
}

ScDPSaveDimension* ScDPSaveData::DuplicateDimension(const ::rtl::OUString& rName)
{
    // always insert new
    //! check if dimension is there?

    ScDPSaveDimension* pOld = GetDimensionByName( rName );
    ScDPSaveDimension* pNew = new ScDPSaveDimension( *pOld );
    pNew->SetDupFlag( true );
    aDimList.push_back(pNew);
    return pNew;
}

void ScDPSaveData::RemoveDimensionByName(const ::rtl::OUString& rName)
{
    boost::ptr_vector<ScDPSaveDimension>::iterator iter;
    for (iter = aDimList.begin(); iter != aDimList.end(); ++iter)
    {
        if ( iter->GetName() == rName && !iter->IsDataLayout() )
        {
            aDimList.erase(iter);
            break;
        }
    }
}

ScDPSaveDimension& ScDPSaveData::DuplicateDimension( const ScDPSaveDimension& rDim )
{
    ScDPSaveDimension* pNew = new ScDPSaveDimension( rDim );
    pNew->SetDupFlag( true );
    aDimList.push_back(pNew);
    return *pNew;
}

ScDPSaveDimension* ScDPSaveData::GetInnermostDimension(sal_uInt16 nOrientation)
{
    // return the innermost dimension for the given orientation,
    // excluding data layout dimension

    boost::ptr_vector<ScDPSaveDimension>::const_reverse_iterator iter;
    for (iter = aDimList.rbegin(); iter != aDimList.rend(); ++iter)
    {
        if (iter->GetOrientation() == nOrientation && !iter->IsDataLayout())
            return const_cast<ScDPSaveDimension*>(&(*iter));
    }

    return NULL;
}

ScDPSaveDimension* ScDPSaveData::GetFirstDimension(sheet::DataPilotFieldOrientation eOrientation)
{
    boost::ptr_vector<ScDPSaveDimension>::const_iterator iter;
    for (iter = aDimList.begin(); iter != aDimList.end(); ++iter)
    {
        if (iter->GetOrientation() == eOrientation && !iter->IsDataLayout())
            return const_cast<ScDPSaveDimension*>(&(*iter));
    }
    return NULL;
}

long ScDPSaveData::GetDataDimensionCount() const
{
    long nDataCount = 0;

    boost::ptr_vector<ScDPSaveDimension>::const_iterator iter;
    for (iter = aDimList.begin(); iter != aDimList.end(); ++iter)
    {
        if (iter->GetOrientation() == sheet::DataPilotFieldOrientation_DATA)
            ++nDataCount;
    }

    return nDataCount;
}

void ScDPSaveData::SetPosition( ScDPSaveDimension* pDim, long nNew )
{
    // position (nNew) is counted within dimensions of the same orientation

    sal_uInt16 nOrient = pDim->GetOrientation();

    boost::ptr_vector<ScDPSaveDimension>::iterator it;
    for ( it = aDimList.begin(); it != aDimList.end(); ++it)
    {
        if (pDim == &(*it))
        {
            // Tell ptr_vector to give up ownership of this element.  Don't
            // delete this instance as it is re-inserted into the container
            // later.
            aDimList.release(it).release();
            break;
        }
    }

    boost::ptr_vector<ScDPSaveDimension>::iterator iterInsert = aDimList.begin();
    while ( nNew > 0 && iterInsert != aDimList.end())
    {
        if (iterInsert->GetOrientation() == nOrient )
            --nNew;

        ++iterInsert;
    }

    aDimList.insert(iterInsert,pDim);
}

void ScDPSaveData::SetColumnGrand(bool bSet)
{
    nColumnGrandMode = bSet;
}

void ScDPSaveData::SetRowGrand(bool bSet)
{
    nRowGrandMode = bSet;
}

void ScDPSaveData::SetIgnoreEmptyRows(bool bSet)
{
    nIgnoreEmptyMode = bSet;
}

void ScDPSaveData::SetRepeatIfEmpty(bool bSet)
{
    nRepeatEmptyMode = bSet;
}

void ScDPSaveData::SetFilterButton(bool bSet)
{
    bFilterButton = bSet;
}

void ScDPSaveData::SetDrillDown(bool bSet)
{
    bDrillDown = bSet;
}

void lcl_ResetOrient( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    sheet::DataPilotFieldOrientation eOrient = sheet::DataPilotFieldOrientation_HIDDEN;

    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
    long nIntCount = xIntDims->getCount();
    for (long nIntDim=0; nIntDim<nIntCount; nIntDim++)
    {
        uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nIntDim) );
        uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
        if (xDimProp.is())
        {
            uno::Any aAny;
            aAny <<= eOrient;
            xDimProp->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ORIENTATION)), aAny );
        }
    }
}

void ScDPSaveData::WriteToSource( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    if (!xSource.is())
        return;

    // source options must be first!

    uno::Reference<beans::XPropertySet> xSourceProp( xSource, uno::UNO_QUERY );
    OSL_ENSURE( xSourceProp.is(), "no properties at source" );
    if ( xSourceProp.is() )
    {
        // source options are not available for external sources
        //! use XPropertySetInfo to test for availability?

        try
        {
            if ( nIgnoreEmptyMode != SC_DPSAVEMODE_DONTKNOW )
                lcl_SetBoolProperty( xSourceProp,
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_IGNOREEMPTY)), (bool)nIgnoreEmptyMode );
            if ( nRepeatEmptyMode != SC_DPSAVEMODE_DONTKNOW )
                lcl_SetBoolProperty( xSourceProp,
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_REPEATIFEMPTY)), (bool)nRepeatEmptyMode );
        }
        catch(uno::Exception&)
        {
            // no error
        }

        const OUString* pGrandTotalName = GetGrandTotalName();
        if (pGrandTotalName)
            ScUnoHelpFunctions::SetOptionalPropertyValue(xSourceProp, SC_UNO_GRANDTOTAL_NAME, *pGrandTotalName);
    }

    // exceptions in the other calls are errors
    try
    {
        // reset all orientations
        //! "forgetSettings" or similar at source ?????
        //! reset all duplicated dimensions, or reuse them below !!!
        OSL_FAIL( "ScDPSaveData::WriteToSource" );

        lcl_ResetOrient( xSource );

        boost::ptr_vector<ScDPSaveDimension>::iterator iter = aDimList.begin();
        for (long i = 0; iter != aDimList.end(); ++iter, ++i)
        {
            rtl::OUString aName = iter->GetName();

            OSL_TRACE( "%s", aName.getStr() );

            bool bData = iter->IsDataLayout();

            //! getByName for ScDPSource, including DataLayoutDimension !!!!!!!!

            uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
            uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
            long nIntCount = xIntDims->getCount();
            bool bFound = false;
            for (long nIntDim=0; nIntDim<nIntCount && !bFound; nIntDim++)
            {
                uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nIntDim) );
                if ( bData )
                {
                    uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
                    if ( xDimProp.is() )
                    {
                        bFound = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ISDATALAYOUT)) );
                        //! error checking -- is "IsDataLayoutDimension" property required??
                    }
                }
                else
                {
                    uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
                    if ( xDimName.is() && xDimName->getName() == aName )
                        bFound = true;
                }

                if ( bFound )
                {
                    if ( iter->GetDupFlag() )
                    {
                        OUStringBuffer aBuf(iter->GetName());

                        // different name for each duplication of a (real) dimension...
                        for (long j=0; j<=i; ++j) //! Test !!!!!!
                            aBuf.append(sal_Unicode('*')); //! modify name at creation of SaveDimension

                        uno::Reference<util::XCloneable> xCloneable( xIntDim, uno::UNO_QUERY );
                        OSL_ENSURE( xCloneable.is(), "cannot clone dimension" );
                        if (xCloneable.is())
                        {
                            uno::Reference<util::XCloneable> xNew = xCloneable->createClone();
                            uno::Reference<container::XNamed> xNewName( xNew, uno::UNO_QUERY );
                            if (xNewName.is())
                            {
                                xNewName->setName(aBuf.makeStringAndClear());
                                iter->WriteToSource( xNew );
                            }
                        }
                    }
                    else
                        iter->WriteToSource( xIntDim );
                }
            }
            OSL_ENSURE(bFound, "WriteToSource: Dimension not found");
        }

        if ( xSourceProp.is() )
        {
            if ( nColumnGrandMode != SC_DPSAVEMODE_DONTKNOW )
                lcl_SetBoolProperty( xSourceProp,
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_COLUMNGRAND)), (bool)nColumnGrandMode );
            if ( nRowGrandMode != SC_DPSAVEMODE_DONTKNOW )
                lcl_SetBoolProperty( xSourceProp,
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ROWGRAND)), (bool)nRowGrandMode );
        }
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("exception in WriteToSource");
    }
}

bool ScDPSaveData::IsEmpty() const
{
    boost::ptr_vector<ScDPSaveDimension>::const_iterator iter;
    for (iter = aDimList.begin(); iter != aDimList.end(); ++iter)
    {
        if (iter->GetOrientation() != sheet::DataPilotFieldOrientation_HIDDEN && !iter->IsDataLayout())
            return false;
    }
    return true; // no entries that are not hidden
}

ScDPDimensionSaveData* ScDPSaveData::GetDimensionData()
{
    if (!pDimensionData)
        pDimensionData = new ScDPDimensionSaveData;
    return pDimensionData;
}

void ScDPSaveData::SetDimensionData( const ScDPDimensionSaveData* pNew )
{
    delete pDimensionData;
    if ( pNew )
        pDimensionData = new ScDPDimensionSaveData( *pNew );
    else
        pDimensionData = NULL;
}

void ScDPSaveData::BuildAllDimensionMembers(ScDPTableData* pData)
{
    if (mbDimensionMembersBuilt)
        return;

    // First, build a dimension name-to-index map.
    typedef boost::unordered_map<OUString, long, ::rtl::OUStringHash> NameIndexMap;
    NameIndexMap aMap;
    long nColCount = pData->GetColumnCount();
    for (long i = 0; i < nColCount; ++i)
        aMap.insert( NameIndexMap::value_type(pData->getDimensionName(i), i));

    NameIndexMap::const_iterator itrEnd = aMap.end();

    boost::ptr_vector<ScDPSaveDimension>::iterator iter;
    for (iter = aDimList.begin(); iter != aDimList.end(); ++iter)
    {
        const ::rtl::OUString& rDimName = iter->GetName();
        if (!rDimName.getLength())
            // empty dimension name. It must be data layout.
            continue;

        NameIndexMap::const_iterator itr = aMap.find(rDimName);
        if (itr == itrEnd)
            // dimension name not in the data. This should never happen!
            continue;

        long nDimIndex = itr->second;
        const std::vector<SCROW>& rMembers = pData->GetColumnEntries(nDimIndex);
        size_t mMemberCount = rMembers.size();
        for (size_t j = 0; j < mMemberCount; ++j)
        {
            const ScDPItemData* pMemberData = pData->GetMemberById( nDimIndex, rMembers[j] );
            ::rtl::OUString aMemName = pMemberData->GetString();
            if (iter->GetExistingMemberByName(aMemName))
                // this member instance already exists. nothing to do.
                continue;

            auto_ptr<ScDPSaveMember> pNewMember(new ScDPSaveMember(aMemName));
            pNewMember->SetIsVisible(true);
            iter->AddMember(pNewMember.release());
        }
    }

    mbDimensionMembersBuilt = true;
}

bool ScDPSaveData::HasInvisibleMember(const OUString& rDimName) const
{
    ScDPSaveDimension* pDim = GetExistingDimensionByName(rDimName);
    if (!pDim)
        return false;

    return pDim->HasInvisibleMember();
}

void ScDPSaveData::Refresh( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    try
    {
        std::list<rtl::OUString> deletedDims;

        {
            boost::ptr_vector<ScDPSaveDimension>::iterator iter = aDimList.end()-1;
            for (long i = aDimList.size()-1; i >= 0; ++i,--iter)
            {
                rtl::OUString aName = iter->GetName();
                if ( iter->IsDataLayout() )
                    continue;

                uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
                uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
                long nIntCount = xIntDims->getCount();
                bool bFound = false;
                for (long nIntDim=0; nIntDim<nIntCount && !bFound; nIntDim++)
                {
                    uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nIntDim) );
                    uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
                    if ( xDimName.is() && xDimName->getName() == aName )
                        bFound = true;
                }

                if ( !bFound )
                {
                    deletedDims.push_back( aName );
                    iter = aDimList.erase(iter);
                    OSL_FAIL( "\n Remove dim: \t" );
                    OSL_TRACE( "%s", aName.getStr() );
                }

            }
        }

        {
            boost::ptr_vector<ScDPSaveDimension>::reverse_iterator iter;
            for (iter = aDimList.rbegin(); iter != aDimList.rend(); ++iter) //check every dimension ??
            {
                rtl::OUString aName = iter->GetName();
                if ( iter->IsDataLayout() )
                    continue;
                iter->Refresh( xSource, deletedDims );

            }
        }

        mbDimensionMembersBuilt = false; // there may be new members
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("error in ScDPSaveData::Refresh");
    }
}

void ScDPSaveDimension::Refresh( const com::sun::star::uno::Reference<
                                com::sun::star::sheet::XDimensionsSupplier>& xSource ,
                                const std::list<rtl::OUString>& deletedDims)
{
    if ( xSource.is() )
    {
        ScDPSource* pTabSource = static_cast<ScDPSource*>( xSource.get() );
        const ScDPCache*  pCache = pTabSource->GetCache();

        SCCOL nSrcDim = pCache->GetDimensionIndex( GetName() );

        if ( nSrcDim == -1 )
            return;
        if ( pSelectedPage )
        {//check pSelected page
            OSL_TRACE( "%s", ::rtl::OUString(*pSelectedPage).getStr() );
            if ( pCache->GetIdByItemData( nSrcDim, *pSelectedPage ) == -1 )
            {
                delete pSelectedPage;
                pSelectedPage = NULL;
            }

        };

        if ( pReferenceValue && pReferenceValue->ReferenceItemType == DataPilotFieldReferenceItemType::NAMED )
        {//check pReferenceValue
#if OSL_DEBUG_LEVEL > 1
            switch( pReferenceValue->ReferenceType)
            {
            case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE: //both
                OSL_FAIL( "\n sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE \n" );
                break;
            case sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE: //both
                OSL_FAIL( "\n sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE \n" );
                break;
            case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE: //both
                OSL_FAIL( "\n sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE \n" );
                break;
            case sheet::DataPilotFieldReferenceType::RUNNING_TOTAL:
                OSL_FAIL( "\n sheet::DataPilotFieldReferenceType::RUNNING_TOTAL \n" ); //enable name
                break;
            }
#endif
            switch( pReferenceValue->ReferenceType)
            {
            case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE:
            case sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE:
            case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE:
            case sheet::DataPilotFieldReferenceType::RUNNING_TOTAL:
                {
                    if( pReferenceValue->ReferenceItemType == DataPilotFieldReferenceItemType::NAMED )
                    {
                        const ::rtl::OUString& sReferenceFieldName = pReferenceValue->ReferenceField;
                        OSL_TRACE( "%s", sReferenceFieldName.getStr() );
                        SCCOL nRefDim = pCache->GetDimensionIndex( sReferenceFieldName );
                        bool bValid = true;
                        if ( nRefDim == -1 )
                            bValid = false;
                        else if ( pReferenceValue->ReferenceType != sheet::DataPilotFieldReferenceType::RUNNING_TOTAL )
                        { //running total has not reference item
                            const ::rtl::OUString& sReferenceItemName = pReferenceValue->ReferenceItemName;
                            OSL_TRACE( "%s", sReferenceItemName.getStr() );
                            if ( pCache->GetIdByItemData( nRefDim, sReferenceItemName ) == -1 )
                                bValid = false;
                        }
                        if ( !bValid )
                        {
                            delete pReferenceValue;
                            pReferenceValue = NULL;
                        }
                    }
                }
                break;
            }

        };

        if ( pSortInfo )
        { //check sortinfo
            if ( pSortInfo->Mode == DataPilotFieldSortMode::DATA )
            {
                OSL_FAIL( "\n DataPilotFieldSortMode::DATA \n" );
                const ::rtl::OUString& sFieldDimName = pSortInfo->Field;
                std::list<rtl::OUString>::const_iterator iter = std::find( deletedDims.begin(), deletedDims.end(), sFieldDimName );
                if ( iter != deletedDims.end() && pCache->GetDimensionIndex( sFieldDimName ) == -1 )
                {
                    pSortInfo->Mode = DataPilotFieldSortMode::MANUAL;
                    pSortInfo->Field = GetName();
                }
            }
        };

        if ( pAutoShowInfo )
        { //check autoshow
            const ::rtl::OUString& sFieldDimName = pAutoShowInfo->DataField;
            std::list<rtl::OUString>::const_iterator iter = std::find( deletedDims.begin(), deletedDims.end(), sFieldDimName );
            if ( iter != deletedDims.end() && pCache->GetDimensionIndex( sFieldDimName ) == -1 )
            {
                delete pAutoShowInfo;
                pAutoShowInfo = NULL;
            }
        };

        //remove unused members
        //SODC_19124
        for (MemberList::iterator i=maMemberList.begin(); i != maMemberList.end() ; )
        {
            rtl::OUString aMemberName = (*i)->GetName();
            if ( pCache->GetIdByItemData( nSrcDim, aMemberName ) == -1 )
                i = maMemberList.erase( i );
            else
                ++i;
        }
    }
}

bool operator == (const ::com::sun::star::sheet::DataPilotFieldSortInfo &l, const ::com::sun::star::sheet::DataPilotFieldSortInfo &r )
{
    return l.Field == r.Field && l.IsAscending == r.IsAscending && l.Mode == r.Mode;
}
bool operator == (const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo &l, const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo &r )
{
    return l.IsEnabled == r.IsEnabled &&
        l.ShowItemsMode == r.ShowItemsMode &&
        l.ItemCount == r.ItemCount &&
        l.DataField == r.DataField;
}
bool operator == (const ::com::sun::star::sheet::DataPilotFieldReference &l, const ::com::sun::star::sheet::DataPilotFieldReference &r )
{
    return l.ReferenceType == r.ReferenceType &&
        l.ReferenceField == r.ReferenceField &&
        l.ReferenceItemType == r.ReferenceItemType &&
        l.ReferenceItemName == r.ReferenceItemName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
