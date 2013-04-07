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

#include "dpobject.hxx"
#include "dptabsrc.hxx"
#include "dpsave.hxx"
#include "dpdimsave.hxx"
#include "dpoutput.hxx"
#include "dpshttab.hxx"
#include "dpsdbtab.hxx"
#include "dpgroup.hxx"
#include "document.hxx"
#include "rechead.hxx"
#include "pivot.hxx"        // PIVOT_DATA_FIELD
#include "dapiuno.hxx"      // ScDataPilotConversion
#include "miscuno.hxx"
#include "scerrors.hxx"
#include "refupdat.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "attrib.hxx"
#include "scitems.hxx"
#include "unonames.hxx"
#include "dpglobal.hxx"
#include "globstr.hrc"
#include "queryentry.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotTableHeaderData.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionData.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionType.hpp>
#include <com/sun/star/sheet/DimensionFlags.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/sheet/XDrillDownDataSupplier.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <sal/macros.h>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <svl/zforlist.hxx>     // IsNumberFormat
#include <vcl/msgbox.hxx>

#include <vector>
#include <memory>

using namespace com::sun::star;
using ::std::vector;
using ::std::unary_function;
using ::boost::shared_ptr;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::sheet::DataPilotTableHeaderData;
using ::com::sun::star::sheet::DataPilotTablePositionData;
using ::com::sun::star::sheet::XDimensionsSupplier;
using ::com::sun::star::beans::XPropertySet;
using ::rtl::OUString;

#define SC_SERVICE_ROWSET           "com.sun.star.sdb.RowSet"
#define SC_SERVICE_INTHANDLER       "com.sun.star.task.InteractionHandler"

#define SC_DBPROP_DATASOURCENAME    "DataSourceName"
#define SC_DBPROP_COMMAND           "Command"
#define SC_DBPROP_COMMANDTYPE       "CommandType"

// -----------------------------------------------------------------------

#define SCDPSOURCE_SERVICE  "com.sun.star.sheet.DataPilotSource"

// -----------------------------------------------------------------------

// incompatible versions of data pilot files
#define SC_DP_VERSION_CURRENT   6

// type of source data
#define SC_DP_SOURCE_SHEET      0
#define SC_DP_SOURCE_DATABASE   1
#define SC_DP_SOURCE_SERVICE    2

// -----------------------------------------------------------------------

//! move to a header file
#define DP_PROP_COLUMNGRAND         "ColumnGrand"
#define DP_PROP_FUNCTION            "Function"
#define DP_PROP_IGNOREEMPTY         "IgnoreEmptyRows"
#define DP_PROP_ISDATALAYOUT        "IsDataLayoutDimension"
#define DP_PROP_ORIENTATION         "Orientation"
#define DP_PROP_ORIGINAL            "Original"
#define DP_PROP_POSITION            "Position"
#define DP_PROP_REPEATIFEMPTY       "RepeatIfEmpty"
#define DP_PROP_ROWGRAND            "RowGrand"
#define DP_PROP_SHOWDETAILS         "ShowDetails"
#define DP_PROP_SHOWEMPTY           "ShowEmpty"
#define DP_PROP_SUBTOTALS           "SubTotals"
#define DP_PROP_USEDHIERARCHY       "UsedHierarchy"

// -----------------------------------------------------------------------

sal_uInt16 lcl_GetDataGetOrientation( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    long nRet = sheet::DataPilotFieldOrientation_HIDDEN;
    if ( xSource.is() )
    {
        uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
        uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
        long nIntCount = xIntDims->getCount();
        sal_Bool bFound = false;
        for (long nIntDim=0; nIntDim<nIntCount && !bFound; nIntDim++)
        {
            uno::Reference<uno::XInterface> xIntDim =
                ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nIntDim) );
            uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
            if ( xDimProp.is() )
            {
                bFound = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ISDATALAYOUT)) );
                //! error checking -- is "IsDataLayoutDimension" property required??
                if (bFound)
                    nRet = ScUnoHelpFunctions::GetEnumProperty(
                            xDimProp, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ORIENTATION)),
                            sheet::DataPilotFieldOrientation_HIDDEN );
            }
        }
    }
    return static_cast< sal_uInt16 >( nRet );
}

ScDPServiceDesc::ScDPServiceDesc(
    const OUString& rServ, const OUString& rSrc, const OUString& rNam,
    const OUString& rUser, const OUString& rPass ) :
    aServiceName( rServ ),
    aParSource( rSrc ),
    aParName( rNam ),
    aParUser( rUser ),
    aParPass( rPass ) {}

bool ScDPServiceDesc::operator== ( const ScDPServiceDesc& rOther ) const
{
    return aServiceName == rOther.aServiceName &&
        aParSource == rOther.aParSource &&
        aParName == rOther.aParName &&
        aParUser == rOther.aParUser &&
        aParPass == rOther.aParPass;
}

ScDPObject::ScDPObject( ScDocument* pD ) :
    pDoc( pD ),
    pSaveData( NULL ),
    pSheetDesc( NULL ),
    pImpDesc( NULL ),
    pServDesc( NULL ),
    mpTableData(static_cast<ScDPTableData*>(NULL)),
    pOutput( NULL ),
    mnAutoFormatIndex( 65535 ),
    nHeaderRows( 0 ),
    mbHeaderLayout(false),
    bAllowMove(false),
    bAlive(false),
    bSettingsChanged(false),
    mbEnableGetPivotData(true)
{
}

ScDPObject::ScDPObject(const ScDPObject& r) :
    pDoc( r.pDoc ),
    pSaveData( NULL ),
    aTableName( r.aTableName ),
    aTableTag( r.aTableTag ),
    aOutRange( r.aOutRange ),
    pSheetDesc( NULL ),
    pImpDesc( NULL ),
    pServDesc( NULL ),
    mpTableData(static_cast<ScDPTableData*>(NULL)),
    pOutput( NULL ),
    mnAutoFormatIndex( r.mnAutoFormatIndex ),
    nHeaderRows( r.nHeaderRows ),
    mbHeaderLayout( r.mbHeaderLayout ),
    bAllowMove(false),
    bAlive(false),
    bSettingsChanged(false),
    mbEnableGetPivotData(r.mbEnableGetPivotData)
{
    if (r.pSaveData)
        pSaveData = new ScDPSaveData(*r.pSaveData);
    if (r.pSheetDesc)
        pSheetDesc = new ScSheetSourceDesc(*r.pSheetDesc);
    if (r.pImpDesc)
        pImpDesc = new ScImportSourceDesc(*r.pImpDesc);
    if (r.pServDesc)
        pServDesc = new ScDPServiceDesc(*r.pServDesc);
    // xSource (and pOutput) is not copied
}

ScDPObject::~ScDPObject()
{
    delete pOutput;
    delete pSaveData;
    delete pSheetDesc;
    delete pImpDesc;
    delete pServDesc;
    ClearSource();
}

void ScDPObject::EnableGetPivotData(bool b)
{
    mbEnableGetPivotData = b;
}

void ScDPObject::SetAlive(bool bSet)
{
    bAlive = bSet;
}

void ScDPObject::SetAllowMove(bool bSet)
{
    bAllowMove = bSet;
}

void ScDPObject::SetSaveData(const ScDPSaveData& rData)
{
    if ( pSaveData != &rData )      // API implementation modifies the original SaveData object
    {
        delete pSaveData;
        pSaveData = new ScDPSaveData( rData );
    }

    InvalidateData();       // re-init source from SaveData
}

void ScDPObject::SetHeaderLayout (bool bUseGrid)
{
    mbHeaderLayout = bUseGrid;
}

bool ScDPObject::GetHeaderLayout() const
{
    return mbHeaderLayout;
}

void ScDPObject::SetOutRange(const ScRange& rRange)
{
    aOutRange = rRange;

    if ( pOutput )
        pOutput->SetPosition( rRange.aStart );
}

void ScDPObject::SetSheetDesc(const ScSheetSourceDesc& rDesc)
{
    if ( pSheetDesc && rDesc == *pSheetDesc )
        return;             // nothing to do

    DELETEZ( pImpDesc );
    DELETEZ( pServDesc );

    delete pSheetDesc;
    pSheetDesc = new ScSheetSourceDesc(rDesc);

    //  make valid QueryParam

    const ScRange& rSrcRange = pSheetDesc->GetSourceRange();
    ScQueryParam aParam = pSheetDesc->GetQueryParam();
    aParam.nCol1 = rSrcRange.aStart.Col();
    aParam.nRow1 = rSrcRange.aStart.Row();
    aParam.nCol2 = rSrcRange.aEnd.Col();
    aParam.nRow2 = rSrcRange.aEnd.Row();
    aParam.bHasHeader = true;
    pSheetDesc->SetQueryParam(aParam);

    ClearSource();      // new source must be created
}

void ScDPObject::SetImportDesc(const ScImportSourceDesc& rDesc)
{
    if ( pImpDesc && rDesc == *pImpDesc )
        return;             // nothing to do

    DELETEZ( pSheetDesc );
    DELETEZ( pServDesc );

    delete pImpDesc;
    pImpDesc = new ScImportSourceDesc(rDesc);

    ClearSource();      // new source must be created
}

void ScDPObject::SetServiceData(const ScDPServiceDesc& rDesc)
{
    if ( pServDesc && rDesc == *pServDesc )
        return;             // nothing to do

    DELETEZ( pSheetDesc );
    DELETEZ( pImpDesc );

    delete pServDesc;
    pServDesc = new ScDPServiceDesc(rDesc);

    ClearSource();      // new source must be created
}

void ScDPObject::WriteSourceDataTo( ScDPObject& rDest ) const
{
    if ( pSheetDesc )
        rDest.SetSheetDesc( *pSheetDesc );
    else if ( pImpDesc )
        rDest.SetImportDesc( *pImpDesc );
    else if ( pServDesc )
        rDest.SetServiceData( *pServDesc );

    //  name/tag are not source data, but needed along with source data

    rDest.aTableName = aTableName;
    rDest.aTableTag  = aTableTag;
}

void ScDPObject::WriteTempDataTo( ScDPObject& rDest ) const
{
    rDest.nHeaderRows = nHeaderRows;
}

bool ScDPObject::IsSheetData() const
{
    return ( pSheetDesc != NULL );
}

void ScDPObject::SetName(const OUString& rNew)
{
    aTableName = rNew;
}

void ScDPObject::SetTag(const OUString& rNew)
{
    aTableTag = rNew;
}

bool ScDPObject::IsDataDescriptionCell(const ScAddress& rPos)
{
    if (!pSaveData)
        return false;

    long nDataDimCount = pSaveData->GetDataDimensionCount();
    if (nDataDimCount != 1)
        // There has to be exactly one data dimension for the description to
        // appear at top-left corner.
        return false;

    CreateOutput();
    ScRange aTabRange = pOutput->GetOutputRange(sheet::DataPilotOutputRangeType::TABLE);
    return (rPos == aTabRange.aStart);
}

uno::Reference<sheet::XDimensionsSupplier> ScDPObject::GetSource()
{
    CreateObjects();
    return xSource;
}

void ScDPObject::CreateOutput()
{
    CreateObjects();
    if (!pOutput)
    {
        sal_Bool bFilterButton = IsSheetData() && pSaveData && pSaveData->GetFilterButton();
        pOutput = new ScDPOutput( pDoc, xSource, aOutRange.aStart, bFilterButton );
        pOutput->SetHeaderLayout ( mbHeaderLayout );

        long nOldRows = nHeaderRows;
        nHeaderRows = pOutput->GetHeaderRows();

        if ( bAllowMove && nHeaderRows != nOldRows )
        {
            long nDiff = nOldRows - nHeaderRows;
            if ( nOldRows == 0 )
                --nDiff;
            if ( nHeaderRows == 0 )
                ++nDiff;

            long nNewRow = aOutRange.aStart.Row() + nDiff;
            if ( nNewRow < 0 )
                nNewRow = 0;

            ScAddress aStart( aOutRange.aStart );
            aStart.SetRow(nNewRow);
            pOutput->SetPosition( aStart );

            //! modify aOutRange?

            bAllowMove = false;     // use only once
        }
    }
}

namespace {

class DisableGetPivotData
{
    ScDPObject& mrDPObj;
    bool mbOldState;
public:
    DisableGetPivotData(ScDPObject& rObj, bool bOld) : mrDPObj(rObj), mbOldState(bOld)
    {
        mrDPObj.EnableGetPivotData(false);
    }

    ~DisableGetPivotData()
    {
        mrDPObj.EnableGetPivotData(mbOldState);
    }
};

}

ScDPTableData* ScDPObject::GetTableData()
{
    if (!mpTableData)
    {
        shared_ptr<ScDPTableData> pData;
        if ( pImpDesc )
        {
            // database data
            const ScDPCache* pCache = pImpDesc->CreateCache();
            if (pCache)
            {
                pCache->AddReference(this);
                pData.reset(new ScDatabaseDPData(pDoc, *pImpDesc, pCache));
            }
        }
        else
        {
            // cell data
            if (!pSheetDesc)
            {
                OSL_FAIL("no source descriptor");
                pSheetDesc = new ScSheetSourceDesc(pDoc);     // dummy defaults
            }

            {
                // Temporarily disable GETPIVOTDATA to avoid having
                // GETPIVOTDATA called onto itself from within the source
                // range.
                DisableGetPivotData aSwitch(*this, mbEnableGetPivotData);
                const ScDPCache* pCache = pSheetDesc->CreateCache();
                if (pCache)
                {
                    pCache->AddReference(this);
                    pData.reset(new ScSheetDPData(pDoc, *pSheetDesc, pCache));
                }
            }
        }

        // grouping (for cell or database data)
        if (pData && pSaveData && pSaveData->GetExistingDimensionData())
        {
            shared_ptr<ScDPGroupTableData> pGroupData(new ScDPGroupTableData(pData, pDoc));
            pSaveData->GetExistingDimensionData()->WriteToData(*pGroupData);
            pData = pGroupData;
        }

        mpTableData = pData;                        // after SetCacheId
    }

    return mpTableData.get();
}

void ScDPObject::CreateObjects()
{
    // if groups are involved, create a new source with the ScDPGroupTableData
    if ( bSettingsChanged && pSaveData && pSaveData->GetExistingDimensionData() )
        ClearSource();

    if (!xSource.is())
    {
        //! cache DPSource and/or Output?

        OSL_ENSURE( bAlive, "CreateObjects on non-inserted DPObject" );

        DELETEZ( pOutput );     // not valid when xSource is changed

        if ( pServDesc )
        {
            xSource = CreateSource( *pServDesc );
        }

        if ( !xSource.is() )    // database or sheet data, or error in CreateSource
        {
            OSL_ENSURE( !pServDesc, "DPSource could not be created" );
            ScDPTableData* pData = GetTableData();
            if (pData)
            {
                ScDPSource* pSource = new ScDPSource( pData );
                xSource = pSource;
            }
        }

        if (pSaveData)
            pSaveData->WriteToSource( xSource );
    }
    else if (bSettingsChanged)
    {
        DELETEZ( pOutput );     // not valid when xSource is changed

        uno::Reference<util::XRefreshable> xRef( xSource, uno::UNO_QUERY );
        if (xRef.is())
        {
            try
            {
                xRef->refresh();
            }
            catch(uno::Exception&)
            {
                OSL_FAIL("exception in refresh");
            }
        }

        if (pSaveData)
            pSaveData->WriteToSource( xSource );
    }
    bSettingsChanged = false;
}

void ScDPObject::InvalidateData()
{
    bSettingsChanged = true;
}

void ScDPObject::ClearSource()
{
    Reference< XComponent > xObjectComp( xSource, UNO_QUERY );
    if ( xObjectComp.is() )
    {
        try
        {
            xObjectComp->dispose();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    xSource = NULL;
    if (mpTableData)
        mpTableData->GetCacheTable().getCache()->RemoveReference(this);
    mpTableData.reset();
}

ScRange ScDPObject::GetNewOutputRange( bool& rOverflow )
{
    CreateOutput();             // create xSource and pOutput if not already done

    rOverflow = pOutput->HasError();        // range overflow or exception from source
    if ( rOverflow )
        return ScRange( aOutRange.aStart );
    else
    {
        //  don't store the result in aOutRange, because nothing has been output yet
        return pOutput->GetOutputRange();
    }
}

void ScDPObject::Output( const ScAddress& rPos )
{
    //  clear old output area
    pDoc->DeleteAreaTab( aOutRange.aStart.Col(), aOutRange.aStart.Row(),
                         aOutRange.aEnd.Col(),   aOutRange.aEnd.Row(),
                         aOutRange.aStart.Tab(), IDF_ALL );
    pDoc->RemoveFlagsTab( aOutRange.aStart.Col(), aOutRange.aStart.Row(),
                          aOutRange.aEnd.Col(),   aOutRange.aEnd.Row(),
                          aOutRange.aStart.Tab(), SC_MF_AUTO );

    CreateOutput();             // create xSource and pOutput if not already done

    pOutput->SetPosition( rPos );

    pOutput->Output();

    //  aOutRange is always the range that was last output to the document
    aOutRange = pOutput->GetOutputRange();
    const ScAddress& s = aOutRange.aStart;
    const ScAddress& e = aOutRange.aEnd;
    pDoc->ApplyFlagsTab(s.Col(), s.Row(), e.Col(), e.Row(), s.Tab(), SC_MF_DP_TABLE);
}

const ScRange ScDPObject::GetOutputRangeByType( sal_Int32 nType )
{
    CreateOutput();

    if (pOutput->HasError())
        return ScRange(aOutRange.aStart);

    return pOutput->GetOutputRange(nType);
}

sal_Bool lcl_HasButton( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    return ((const ScMergeFlagAttr*)pDoc->GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->HasButton();
}

void ScDPObject::RefreshAfterLoad()
{
    // apply drop-down attribute, initialize nHeaderRows, without accessing the source
    // (button attribute must be present)

    // simple test: block of button cells at the top, followed by an empty cell

    SCCOL nFirstCol = aOutRange.aStart.Col();
    SCROW nFirstRow = aOutRange.aStart.Row();
    SCTAB nTab = aOutRange.aStart.Tab();

    SCROW nInitial = 0;
    SCROW nOutRows = aOutRange.aEnd.Row() + 1 - aOutRange.aStart.Row();
    while ( nInitial + 1 < nOutRows && lcl_HasButton( pDoc, nFirstCol, nFirstRow + nInitial, nTab ) )
        ++nInitial;

    if ( nInitial + 1 < nOutRows &&
        pDoc->IsBlockEmpty( nTab, nFirstCol, nFirstRow + nInitial, nFirstCol, nFirstRow + nInitial ) &&
        aOutRange.aEnd.Col() > nFirstCol )
    {
        sal_Bool bFilterButton = IsSheetData();         // when available, filter button setting must be checked here

        SCROW nSkip = bFilterButton ? 1 : 0;
        for (SCROW nPos=nSkip; nPos<nInitial; nPos++)
            pDoc->ApplyAttr( nFirstCol + 1, nFirstRow + nPos, nTab, ScMergeFlagAttr(SC_MF_AUTO) );

        nHeaderRows = nInitial;
    }
    else
        nHeaderRows = 0;        // nothing found, no drop-down lists
}

void ScDPObject::BuildAllDimensionMembers()
{
    if (!pSaveData)
        return;

    // #i111857# don't always create empty mpTableData for external service.
    // Ideally, xSource should be used instead of mpTableData.
    if (pServDesc)
        return;

    pSaveData->BuildAllDimensionMembers(GetTableData());
}

bool ScDPObject::GetMemberNames( sal_Int32 nDim, Sequence<OUString>& rNames )
{
    vector<ScDPLabelData::Member> aMembers;
    if (!GetMembers(nDim, GetUsedHierarchy(nDim), aMembers))
        return false;

    size_t n = aMembers.size();
    rNames.realloc(n);
    for (size_t i = 0; i < n; ++i)
        rNames[i] = aMembers[i].maName;

    return true;
}

bool ScDPObject::GetMembers( sal_Int32 nDim, sal_Int32 nHier, vector<ScDPLabelData::Member>& rMembers )
{
    Reference< container::XNameAccess > xMembersNA;
    if (!GetMembersNA( nDim, nHier, xMembersNA ))
        return false;

    Reference<container::XIndexAccess> xMembersIA( new ScNameToIndexAccess(xMembersNA) );
    sal_Int32 nCount = xMembersIA->getCount();
    vector<ScDPLabelData::Member> aMembers;
    aMembers.reserve(nCount);

    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        Reference<container::XNamed> xMember(xMembersIA->getByIndex(i), UNO_QUERY);
        ScDPLabelData::Member aMem;

        if (xMember.is())
            aMem.maName = xMember->getName();

        Reference<beans::XPropertySet> xMemProp(xMember, UNO_QUERY);
        if (xMemProp.is())
        {
            aMem.mbVisible     = ScUnoHelpFunctions::GetBoolProperty(xMemProp, OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ISVISIBL)));
            aMem.mbShowDetails = ScUnoHelpFunctions::GetBoolProperty(xMemProp, OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_SHOWDETA)));

            aMem.maLayoutName = ScUnoHelpFunctions::GetStringProperty(
                xMemProp, OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_LAYOUTNAME)), OUString());
        }

        aMembers.push_back(aMem);
    }
    rMembers.swap(aMembers);
    return true;
}

void ScDPObject::UpdateReference( UpdateRefMode eUpdateRefMode,
                                     const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    // Output area

    SCCOL nCol1 = aOutRange.aStart.Col();
    SCROW nRow1 = aOutRange.aStart.Row();
    SCTAB nTab1 = aOutRange.aStart.Tab();
    SCCOL nCol2 = aOutRange.aEnd.Col();
    SCROW nRow2 = aOutRange.aEnd.Row();
    SCTAB nTab2 = aOutRange.aEnd.Tab();

    ScRefUpdateRes eRes =
        ScRefUpdate::Update( pDoc, eUpdateRefMode,
            rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
            rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(), nDx, nDy, nDz,
            nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    if ( eRes != UR_NOTHING )
        SetOutRange( ScRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 ) );

    // sheet source data

    if ( pSheetDesc )
    {
        const OUString& rRangeName = pSheetDesc->GetRangeName();
        if (!rRangeName.isEmpty())
            // Source range is a named range.  No need to update.
            return;

        const ScRange& rSrcRange = pSheetDesc->GetSourceRange();
        nCol1 = rSrcRange.aStart.Col();
        nRow1 = rSrcRange.aStart.Row();
        nTab1 = rSrcRange.aStart.Tab();
        nCol2 = rSrcRange.aEnd.Col();
        nRow2 = rSrcRange.aEnd.Row();
        nTab2 = rSrcRange.aEnd.Tab();

        eRes = ScRefUpdate::Update( pDoc, eUpdateRefMode,
                rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
                rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(), nDx, nDy, nDz,
                nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        if ( eRes != UR_NOTHING )
        {
            SCsCOL nDiffX = nCol1 - pSheetDesc->GetSourceRange().aStart.Col();
            SCsROW nDiffY = nRow1 - pSheetDesc->GetSourceRange().aStart.Row();

            ScQueryParam aParam = pSheetDesc->GetQueryParam();
            aParam.nCol1 = sal::static_int_cast<SCCOL>( aParam.nCol1 + nDiffX );
            aParam.nCol2 = sal::static_int_cast<SCCOL>( aParam.nCol2 + nDiffX );
            aParam.nRow1 += nDiffY; //! used?
            aParam.nRow2 += nDiffY; //! used?
            SCSIZE nEC = aParam.GetEntryCount();
            for (SCSIZE i=0; i<nEC; i++)
                if (aParam.GetEntry(i).bDoQuery)
                    aParam.GetEntry(i).nField += nDiffX;

            pSheetDesc->SetQueryParam(aParam);
            pSheetDesc->SetSourceRange(ScRange(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2));
        }
    }
}

bool ScDPObject::RefsEqual( const ScDPObject& r ) const
{
    if ( aOutRange != r.aOutRange )
        return false;

    if ( pSheetDesc && r.pSheetDesc )
    {
        if ( pSheetDesc->GetSourceRange() != r.pSheetDesc->GetSourceRange() )
            return false;
    }
    else if ( pSheetDesc || r.pSheetDesc )
    {
        OSL_FAIL("RefsEqual: SheetDesc set at only one object");
        return false;
    }

    return true;
}

void ScDPObject::WriteRefsTo( ScDPObject& r ) const
{
    r.SetOutRange( aOutRange );
    if ( pSheetDesc )
        r.SetSheetDesc( *pSheetDesc );
}

void ScDPObject::GetPositionData(const ScAddress& rPos, DataPilotTablePositionData& rPosData)
{
    CreateOutput();
    pOutput->GetPositionData(rPos, rPosData);
}

bool ScDPObject::GetDataFieldPositionData(
    const ScAddress& rPos, Sequence<sheet::DataPilotFieldFilter>& rFilters)
{
    CreateOutput();

    vector<sheet::DataPilotFieldFilter> aFilters;
    if (!pOutput->GetDataResultPositionData(aFilters, rPos))
        return false;

    sal_Int32 n = static_cast<sal_Int32>(aFilters.size());
    rFilters.realloc(n);
    for (sal_Int32 i = 0; i < n; ++i)
        rFilters[i] = aFilters[i];

    return true;
}

void ScDPObject::GetDrillDownData(const ScAddress& rPos, Sequence< Sequence<Any> >& rTableData)
{
    CreateOutput();

    Reference<sheet::XDrillDownDataSupplier> xDrillDownData(xSource, UNO_QUERY);
    if (!xDrillDownData.is())
        return;

    Sequence<sheet::DataPilotFieldFilter> filters;
    if (!GetDataFieldPositionData(rPos, filters))
        return;

    rTableData = xDrillDownData->getDrillDownData(filters);
}

bool ScDPObject::IsDimNameInUse(const OUString& rName) const
{
    if (!xSource.is())
        return false;

    Reference<container::XNameAccess> xDims = xSource->getDimensions();
    Sequence<OUString> aDimNames = xDims->getElementNames();
    sal_Int32 n = aDimNames.getLength();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        const OUString& rDimName = aDimNames[i];
        if (rDimName.equalsIgnoreAsciiCase(rName))
            return true;

        Reference<beans::XPropertySet> xPropSet(xDims->getByName(rDimName), UNO_QUERY);
        if (!xPropSet.is())
            continue;

        OUString aLayoutName = ScUnoHelpFunctions::GetStringProperty(
            xPropSet, OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_LAYOUTNAME)), OUString());
        if (aLayoutName.equalsIgnoreAsciiCase(rName))
            return true;
    }
    return false;
}

OUString ScDPObject::GetDimName( long nDim, bool& rIsDataLayout, sal_Int32* pFlags )
{
    rIsDataLayout = false;
    OUString aRet;

    if ( xSource.is() )
    {
        uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
        uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
        long nDimCount = xDims->getCount();
        if ( nDim < nDimCount )
        {
            uno::Reference<uno::XInterface> xIntDim =
                ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
            uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
            uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
            if ( xDimName.is() && xDimProp.is() )
            {
                sal_Bool bData = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ISDATALAYOUT)) );
                //! error checking -- is "IsDataLayoutDimension" property required??

                rtl::OUString aName;
                try
                {
                    aName = xDimName->getName();
                }
                catch(uno::Exception&)
                {
                }
                if ( bData )
                    rIsDataLayout = true;
                else
                    aRet = aName;

                if (pFlags)
                    *pFlags = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_FLAGS)), 0 );
            }
        }
    }

    return aRet;
}

bool ScDPObject::IsDuplicated( long nDim )
{
    bool bDuplicated = false;
    if ( xSource.is() )
    {
        uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
        uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
        long nDimCount = xDims->getCount();
        if ( nDim < nDimCount )
        {
            uno::Reference<uno::XInterface> xIntDim =
                ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
            uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
            if ( xDimProp.is() )
            {
                try
                {
                    uno::Any aOrigAny = xDimProp->getPropertyValue(
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ORIGINAL)) );
                    uno::Reference<uno::XInterface> xIntOrig;
                    if ( (aOrigAny >>= xIntOrig) && xIntOrig.is() )
                        bDuplicated = true;
                }
                catch(uno::Exception&)
                {
                }
            }
        }
    }
    return bDuplicated;
}

long ScDPObject::GetDimCount()
{
    long nRet = 0;
    if ( xSource.is() )
    {
        try
        {
            uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
            if ( xDimsName.is() )
                nRet = xDimsName->getElementNames().getLength();
        }
        catch(uno::Exception&)
        {
        }
    }
    return nRet;
}

void ScDPObject::FillPageList( TypedScStrCollection& rStrings, long nField )
{
    //! merge members access with ToggleDetails?

    //! convert field index to dimension index?

    OSL_ENSURE( xSource.is(), "no source" );
    if ( !xSource.is() ) return;

    uno::Reference<container::XNamed> xDim;
    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
    long nIntCount = xIntDims->getCount();
    if ( nField < nIntCount )
    {
        uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface(
                                    xIntDims->getByIndex(nField) );
        xDim = uno::Reference<container::XNamed>( xIntDim, uno::UNO_QUERY );
    }
    OSL_ENSURE( xDim.is(), "dimension not found" );
    if ( !xDim.is() ) return;

    uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
    long nHierarchy = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_USEDHIERARCHY)) );
    long nLevel = 0;

    long nHierCount = 0;
    uno::Reference<container::XIndexAccess> xHiers;
    uno::Reference<sheet::XHierarchiesSupplier> xHierSupp( xDim, uno::UNO_QUERY );
    if ( xHierSupp.is() )
    {
        uno::Reference<container::XNameAccess> xHiersName = xHierSupp->getHierarchies();
        xHiers = new ScNameToIndexAccess( xHiersName );
        nHierCount = xHiers->getCount();
    }
    uno::Reference<uno::XInterface> xHier;
    if ( nHierarchy < nHierCount )
        xHier = ScUnoHelpFunctions::AnyToInterface( xHiers->getByIndex(nHierarchy) );
    OSL_ENSURE( xHier.is(), "hierarchy not found" );
    if ( !xHier.is() ) return;

    long nLevCount = 0;
    uno::Reference<container::XIndexAccess> xLevels;
    uno::Reference<sheet::XLevelsSupplier> xLevSupp( xHier, uno::UNO_QUERY );
    if ( xLevSupp.is() )
    {
        uno::Reference<container::XNameAccess> xLevsName = xLevSupp->getLevels();
        xLevels = new ScNameToIndexAccess( xLevsName );
        nLevCount = xLevels->getCount();
    }
    uno::Reference<uno::XInterface> xLevel;
    if ( nLevel < nLevCount )
        xLevel = ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex(nLevel) );
    OSL_ENSURE( xLevel.is(), "level not found" );
    if ( !xLevel.is() ) return;

    uno::Reference<container::XNameAccess> xMembers;
    uno::Reference<sheet::XMembersSupplier> xMbrSupp( xLevel, uno::UNO_QUERY );
    if ( xMbrSupp.is() )
        xMembers = xMbrSupp->getMembers();
    OSL_ENSURE( xMembers.is(), "members not found" );
    if ( !xMembers.is() ) return;

    uno::Sequence<rtl::OUString> aNames = xMembers->getElementNames();
    long nNameCount = aNames.getLength();
    const rtl::OUString* pNameArr = aNames.getConstArray();
    for (long nPos = 0; nPos < nNameCount; ++nPos)
    {
        // Make sure to insert only visible members.
        Reference<XPropertySet> xPropSet(xMembers->getByName(pNameArr[nPos]), UNO_QUERY);
        sal_Bool bVisible = false;
        if (xPropSet.is())
        {
            Any any = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ISVISIBL)));
            any >>= bVisible;
        }

        if (bVisible)
        {
            // use the order from getElementNames
            TypedStrData* pData = new TypedStrData( pNameArr[nPos] );
            if ( !rStrings.AtInsert( rStrings.GetCount(), pData ) )
                delete pData;
        }
    }

    //  add "-all-" entry to the top (unsorted)
    TypedStrData* pAllData = new TypedStrData( String( ScResId( SCSTR_ALL ) ) );    //! separate string? (also output)
    if ( !rStrings.AtInsert( 0, pAllData ) )
        delete pAllData;
}

void ScDPObject::GetHeaderPositionData(const ScAddress& rPos, DataPilotTableHeaderData& rData)
{
    using namespace ::com::sun::star::sheet::DataPilotTablePositionType;

    CreateOutput();             // create xSource and pOutput if not already done

    // Reset member values to invalid state.
    rData.Dimension = rData.Hierarchy = rData.Level = -1;
    rData.Flags = 0;

    DataPilotTablePositionData aPosData;
    pOutput->GetPositionData(rPos, aPosData);
    const sal_Int32 nPosType = aPosData.PositionType;
    if (nPosType == COLUMN_HEADER || nPosType == ROW_HEADER)
        aPosData.PositionData >>= rData;
}

// Returns sal_True on success and stores the result in rTarget
bool ScDPObject::GetPivotData( ScDPGetPivotDataField& rTarget,
                               const std::vector< ScDPGetPivotDataField >& rFilters )
{
    if (!mbEnableGetPivotData)
        return false;

    CreateOutput();             // create xSource and pOutput if not already done
    return pOutput->GetPivotData( rTarget, rFilters );
}

bool ScDPObject::IsFilterButton( const ScAddress& rPos )
{
    CreateOutput();             // create xSource and pOutput if not already done

    return pOutput->IsFilterButton( rPos );
}

long ScDPObject::GetHeaderDim( const ScAddress& rPos, sal_uInt16& rOrient )
{
    CreateOutput();             // create xSource and pOutput if not already done

    return pOutput->GetHeaderDim( rPos, rOrient );
}

bool ScDPObject::GetHeaderDrag( const ScAddress& rPos, bool bMouseLeft, bool bMouseTop, long nDragDim,
                                Rectangle& rPosRect, sal_uInt16& rOrient, long& rDimPos )
{
    CreateOutput();             // create xSource and pOutput if not already done

    return pOutput->GetHeaderDrag( rPos, bMouseLeft, bMouseTop, nDragDim, rPosRect, rOrient, rDimPos );
}

void ScDPObject::GetMemberResultNames( ScStrCollection& rNames, long nDimension )
{
    CreateOutput();             // create xSource and pOutput if not already done

    pOutput->GetMemberResultNames( rNames, nDimension );    // used only with table data -> level not needed
}

bool lcl_Dequote( const String& rSource, xub_StrLen nStartPos, xub_StrLen& rEndPos, String& rResult )
{
    // nStartPos has to point to opening quote

    bool bRet = false;
    const sal_Unicode cQuote = '\'';

    if ( rSource.GetChar(nStartPos) == cQuote )
    {
        rtl::OUStringBuffer aBuffer;
        xub_StrLen nPos = nStartPos + 1;
        const xub_StrLen nLen = rSource.Len();

        while ( nPos < nLen )
        {
            const sal_Unicode cNext = rSource.GetChar(nPos);
            if ( cNext == cQuote )
            {
                if ( nPos+1 < nLen && rSource.GetChar(nPos+1) == cQuote )
                {
                    // double quote is used for an embedded quote
                    aBuffer.append( cNext );    // append one quote
                    ++nPos;                     // skip the next one
                }
                else
                {
                    // end of quoted string
                    rResult = aBuffer.makeStringAndClear();
                    rEndPos = nPos + 1;         // behind closing quote
                    return true;
                }
            }
            else
                aBuffer.append( cNext );

            ++nPos;
        }
        // no closing quote before the end of the string -> error (bRet still false)
    }

    return bRet;
}

struct ScGetPivotDataFunctionEntry
{
    const sal_Char*         pName;
    sheet::GeneralFunction  eFunc;
};

bool lcl_ParseFunction( const String& rList, xub_StrLen nStartPos, xub_StrLen& rEndPos, sheet::GeneralFunction& rFunc )
{
    static const ScGetPivotDataFunctionEntry aFunctions[] =
    {
        // our names
        { "Sum",        sheet::GeneralFunction_SUM       },
        { "Count",      sheet::GeneralFunction_COUNT     },
        { "Average",    sheet::GeneralFunction_AVERAGE   },
        { "Max",        sheet::GeneralFunction_MAX       },
        { "Min",        sheet::GeneralFunction_MIN       },
        { "Product",    sheet::GeneralFunction_PRODUCT   },
        { "CountNums",  sheet::GeneralFunction_COUNTNUMS },
        { "StDev",      sheet::GeneralFunction_STDEV     },
        { "StDevp",     sheet::GeneralFunction_STDEVP    },
        { "Var",        sheet::GeneralFunction_VAR       },
        { "VarP",       sheet::GeneralFunction_VARP      },
        // compatibility names
        { "Count Nums", sheet::GeneralFunction_COUNTNUMS },
        { "StdDev",     sheet::GeneralFunction_STDEV     },
        { "StdDevp",    sheet::GeneralFunction_STDEVP    }
    };

    const xub_StrLen nListLen = rList.Len();
    while ( nStartPos < nListLen && rList.GetChar(nStartPos) == ' ' )
        ++nStartPos;

    bool bParsed = false;
    bool bFound = false;
    String aFuncStr;
    xub_StrLen nFuncEnd = 0;
    if ( nStartPos < nListLen && rList.GetChar(nStartPos) == '\'' )
        bParsed = lcl_Dequote( rList, nStartPos, nFuncEnd, aFuncStr );
    else
    {
        nFuncEnd = rList.Search( static_cast<sal_Unicode>(']'), nStartPos );
        if ( nFuncEnd != STRING_NOTFOUND )
        {
            aFuncStr = rList.Copy( nStartPos, nFuncEnd - nStartPos );
            bParsed = true;
        }
    }

    if ( bParsed )
    {
        aFuncStr = comphelper::string::strip(aFuncStr, ' ');

        const sal_Int32 nFuncCount = SAL_N_ELEMENTS(aFunctions);
        for ( sal_Int32 nFunc=0; nFunc<nFuncCount && !bFound; nFunc++ )
        {
            if ( aFuncStr.EqualsIgnoreCaseAscii( aFunctions[nFunc].pName ) )
            {
                rFunc = aFunctions[nFunc].eFunc;
                bFound = true;

                while ( nFuncEnd < nListLen && rList.GetChar(nFuncEnd) == ' ' )
                    ++nFuncEnd;
                rEndPos = nFuncEnd;
            }
        }
    }

    return bFound;
}

bool lcl_IsAtStart( const String& rList, const String& rSearch, sal_Int32& rMatched,
                    bool bAllowBracket, sheet::GeneralFunction* pFunc )
{
    sal_Int32 nMatchList = 0;
    sal_Int32 nMatchSearch = 0;
    sal_Unicode cFirst = rList.GetChar(0);
    if ( cFirst == '\'' || cFirst == '[' )
    {
        // quoted string or string in brackets must match completely

        String aDequoted;
        xub_StrLen nQuoteEnd = 0;
        bool bParsed = false;

        if ( cFirst == '\'' )
            bParsed = lcl_Dequote( rList, 0, nQuoteEnd, aDequoted );
        else if ( cFirst == '[' )
        {
            // skip spaces after the opening bracket

            xub_StrLen nStartPos = 1;
            const xub_StrLen nListLen = rList.Len();
            while ( nStartPos < nListLen && rList.GetChar(nStartPos) == ' ' )
                ++nStartPos;

            if ( rList.GetChar(nStartPos) == '\'' )         // quoted within the brackets?
            {
                if ( lcl_Dequote( rList, nStartPos, nQuoteEnd, aDequoted ) )
                {
                    // after the quoted string, there must be the closing bracket, optionally preceded by spaces,
                    // and/or a function name
                    while ( nQuoteEnd < nListLen && rList.GetChar(nQuoteEnd) == ' ' )
                        ++nQuoteEnd;

                    // semicolon separates function name
                    if ( nQuoteEnd < nListLen && rList.GetChar(nQuoteEnd) == ';' && pFunc )
                    {
                        xub_StrLen nFuncEnd = 0;
                        if ( lcl_ParseFunction( rList, nQuoteEnd + 1, nFuncEnd, *pFunc ) )
                            nQuoteEnd = nFuncEnd;
                    }
                    if ( nQuoteEnd < nListLen && rList.GetChar(nQuoteEnd) == ']' )
                    {
                        ++nQuoteEnd;        // include the closing bracket for the matched length
                        bParsed = true;
                    }
                }
            }
            else
            {
                // implicit quoting to the closing bracket

                xub_StrLen nClosePos = rList.Search( static_cast<sal_Unicode>(']'), nStartPos );
                if ( nClosePos != STRING_NOTFOUND )
                {
                    xub_StrLen nNameEnd = nClosePos;
                    xub_StrLen nSemiPos = rList.Search( static_cast<sal_Unicode>(';'), nStartPos );
                    if ( nSemiPos != STRING_NOTFOUND && nSemiPos < nClosePos && pFunc )
                    {
                        xub_StrLen nFuncEnd = 0;
                        if ( lcl_ParseFunction( rList, nSemiPos + 1, nFuncEnd, *pFunc ) )
                            nNameEnd = nSemiPos;
                    }

                    aDequoted = rList.Copy( nStartPos, nNameEnd - nStartPos );
                    aDequoted.EraseTrailingChars( ' ' );        // spaces before the closing bracket or semicolon
                    nQuoteEnd = nClosePos + 1;
                    bParsed = true;
                }
            }
        }

        if ( bParsed && ScGlobal::GetpTransliteration()->isEqual( aDequoted, rSearch ) )
        {
            nMatchList = nQuoteEnd;             // match count in the list string, including quotes
            nMatchSearch = rSearch.Len();
        }
    }
    else
    {
        // otherwise look for search string at the start of rList
        ScGlobal::GetpTransliteration()->equals( rList, 0, rList.Len(), nMatchList,
                                            rSearch, 0, rSearch.Len(), nMatchSearch );
    }

    if ( nMatchSearch == rSearch.Len() )
    {
        // search string is at start of rList - look for following space or end of string

        bool bValid = false;
        if ( sal::static_int_cast<xub_StrLen>(nMatchList) >= rList.Len() )
            bValid = true;
        else
        {
            sal_Unicode cNext = rList.GetChar(sal::static_int_cast<xub_StrLen>(nMatchList));
            if ( cNext == ' ' || ( bAllowBracket && cNext == '[' ) )
                bValid = true;
        }

        if ( bValid )
        {
            rMatched = nMatchList;
            return true;
        }
    }

    return false;
}

bool ScDPObject::ParseFilters( ScDPGetPivotDataField& rTarget,
                               std::vector< ScDPGetPivotDataField >& rFilters,
                               const OUString& rFilterList )
{
    // parse the string rFilterList into parameters for GetPivotData

    CreateObjects();            // create xSource if not already done

    std::vector<String> aDataNames;     // data fields (source name)
    std::vector<String> aGivenNames;    // data fields (compound name)
    std::vector<String> aFieldNames;    // column/row/data fields
    std::vector< uno::Sequence<rtl::OUString> > aFieldValues;

    //
    // get all the field and item names
    //

    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
    sal_Int32 nDimCount = xIntDims->getCount();
    for ( sal_Int32 nDim = 0; nDim<nDimCount; nDim++ )
    {
        uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nDim) );
        uno::Reference<container::XNamed> xDim( xIntDim, uno::UNO_QUERY );
        uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
        uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDim, uno::UNO_QUERY );
        sal_Bool bDataLayout = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ISDATALAYOUT)) );
        sal_Int32 nOrient = ScUnoHelpFunctions::GetEnumProperty(
                            xDimProp, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ORIENTATION)),
                            sheet::DataPilotFieldOrientation_HIDDEN );
        if ( !bDataLayout )
        {
            if ( nOrient == sheet::DataPilotFieldOrientation_DATA )
            {
                String aSourceName;
                String aGivenName;
                ScDPOutput::GetDataDimensionNames( aSourceName, aGivenName, xIntDim );
                aDataNames.push_back( aSourceName );
                aGivenNames.push_back( aGivenName );
            }
            else if ( nOrient != sheet::DataPilotFieldOrientation_HIDDEN )
            {
                // get level names, as in ScDPOutput

                uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess( xDimSupp->getHierarchies() );
                sal_Int32 nHierarchy = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_USEDHIERARCHY)) );
                if ( nHierarchy >= xHiers->getCount() )
                    nHierarchy = 0;

                uno::Reference<uno::XInterface> xHier = ScUnoHelpFunctions::AnyToInterface(
                                                    xHiers->getByIndex(nHierarchy) );
                uno::Reference<sheet::XLevelsSupplier> xHierSupp( xHier, uno::UNO_QUERY );
                if ( xHierSupp.is() )
                {
                    uno::Reference<container::XIndexAccess> xLevels = new ScNameToIndexAccess( xHierSupp->getLevels() );
                    sal_Int32 nLevCount = xLevels->getCount();
                    for (sal_Int32 nLev=0; nLev<nLevCount; nLev++)
                    {
                        uno::Reference<uno::XInterface> xLevel = ScUnoHelpFunctions::AnyToInterface(
                                                            xLevels->getByIndex(nLev) );
                        uno::Reference<container::XNamed> xLevNam( xLevel, uno::UNO_QUERY );
                        uno::Reference<sheet::XMembersSupplier> xLevSupp( xLevel, uno::UNO_QUERY );
                        if ( xLevNam.is() && xLevSupp.is() )
                        {
                            uno::Reference<container::XNameAccess> xMembers = xLevSupp->getMembers();

                            String aFieldName( xLevNam->getName() );
                            uno::Sequence<rtl::OUString> aMemberNames( xMembers->getElementNames() );

                            aFieldNames.push_back( aFieldName );
                            aFieldValues.push_back( aMemberNames );
                        }
                    }
                }
            }
        }
    }

    //
    // compare and build filters
    //

    SCSIZE nDataFields = aDataNames.size();
    SCSIZE nFieldCount = aFieldNames.size();
    OSL_ENSURE( aGivenNames.size() == nDataFields && aFieldValues.size() == nFieldCount, "wrong count" );

    bool bError = false;
    bool bHasData = false;
    String aRemaining(comphelper::string::strip(rFilterList, ' '));
    while ( aRemaining.Len() && !bError )
    {
        bool bUsed = false;

        // look for data field name

        for ( SCSIZE nDataPos=0; nDataPos<nDataFields && !bUsed; nDataPos++ )
        {
            String aFound;
            sal_Int32 nMatched = 0;
            if ( lcl_IsAtStart( aRemaining, aDataNames[nDataPos], nMatched, false, NULL ) )
                aFound = aDataNames[nDataPos];
            else if ( lcl_IsAtStart( aRemaining, aGivenNames[nDataPos], nMatched, false, NULL ) )
                aFound = aGivenNames[nDataPos];

            if ( aFound.Len() )
            {
                rTarget.maFieldName = aFound;
                aRemaining.Erase( 0, sal::static_int_cast<xub_StrLen>(nMatched) );
                bHasData = true;
                bUsed = true;
            }
        }

        // look for field name

        String aSpecField;
        bool bHasFieldName = false;
        if ( !bUsed )
        {
            sal_Int32 nMatched = 0;
            for ( SCSIZE nField=0; nField<nFieldCount && !bHasFieldName; nField++ )
            {
                if ( lcl_IsAtStart( aRemaining, aFieldNames[nField], nMatched, true, NULL ) )
                {
                    aSpecField = aFieldNames[nField];
                    aRemaining.Erase( 0, sal::static_int_cast<xub_StrLen>(nMatched) );
                    aRemaining.EraseLeadingChars( ' ' );

                    // field name has to be followed by item name in brackets
                    if ( aRemaining.GetChar(0) == '[' )
                    {
                        bHasFieldName = true;
                        // bUsed remains false - still need the item
                    }
                    else
                    {
                        bUsed = true;
                        bError = true;
                    }
                }
            }
        }

        // look for field item

        if ( !bUsed )
        {
            bool bItemFound = false;
            sal_Int32 nMatched = 0;
            String aFoundName;
            String aFoundValue;
            sheet::GeneralFunction eFunc = sheet::GeneralFunction_NONE;
            sheet::GeneralFunction eFoundFunc = sheet::GeneralFunction_NONE;

            for ( SCSIZE nField=0; nField<nFieldCount; nField++ )
            {
                // If a field name is given, look in that field only, otherwise in all fields.
                // aSpecField is initialized from aFieldNames array, so exact comparison can be used.
                if ( !bHasFieldName || aFieldNames[nField] == aSpecField )
                {
                    const uno::Sequence<rtl::OUString>& rItems = aFieldValues[nField];
                    sal_Int32 nItemCount = rItems.getLength();
                    const rtl::OUString* pItemArr = rItems.getConstArray();
                    for ( sal_Int32 nItem=0; nItem<nItemCount; nItem++ )
                    {
                        if ( lcl_IsAtStart( aRemaining, pItemArr[nItem], nMatched, false, &eFunc ) )
                        {
                            if ( bItemFound )
                                bError = true;      // duplicate (also across fields)
                            else
                            {
                                aFoundName = aFieldNames[nField];
                                aFoundValue = pItemArr[nItem];
                                eFoundFunc = eFunc;
                                bItemFound = true;
                                bUsed = true;
                            }
                        }
                    }
                }
            }

            if ( bItemFound && !bError )
            {
                ScDPGetPivotDataField aField;
                aField.maFieldName = aFoundName;
                aField.meFunction = eFoundFunc;
                aField.mbValIsStr = true;
                aField.maValStr = aFoundValue;
                aField.mnValNum = 0.0;
                rFilters.push_back( aField );

                aRemaining.Erase( 0, sal::static_int_cast<xub_StrLen>(nMatched) );
            }
        }

        if ( !bUsed )
            bError = true;

        aRemaining.EraseLeadingChars( ' ' );        // remove any number of spaces between entries
    }

    if ( !bError && !bHasData && aDataNames.size() == 1 )
    {
        // if there's only one data field, its name need not be specified
        rTarget.maFieldName = aDataNames[0];
        bHasData = true;
    }

    return bHasData && !bError;
}

void ScDPObject::ToggleDetails(const DataPilotTableHeaderData& rElemDesc, ScDPObject* pDestObj)
{
    CreateObjects();            // create xSource if not already done

    //  find dimension name

    uno::Reference<container::XNamed> xDim;
    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
    long nIntCount = xIntDims->getCount();
    if ( rElemDesc.Dimension < nIntCount )
    {
        uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface(
                                    xIntDims->getByIndex(rElemDesc.Dimension) );
        xDim = uno::Reference<container::XNamed>( xIntDim, uno::UNO_QUERY );
    }
    OSL_ENSURE( xDim.is(), "dimension not found" );
    if ( !xDim.is() ) return;
    String aDimName = xDim->getName();

    uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
    sal_Bool bDataLayout = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ISDATALAYOUT)) );
    if (bDataLayout)
    {
        //  the elements of the data layout dimension can't be found by their names
        //  -> don't change anything
        return;
    }

    //  query old state

    long nHierCount = 0;
    uno::Reference<container::XIndexAccess> xHiers;
    uno::Reference<sheet::XHierarchiesSupplier> xHierSupp( xDim, uno::UNO_QUERY );
    if ( xHierSupp.is() )
    {
        uno::Reference<container::XNameAccess> xHiersName = xHierSupp->getHierarchies();
        xHiers = new ScNameToIndexAccess( xHiersName );
        nHierCount = xHiers->getCount();
    }
    uno::Reference<uno::XInterface> xHier;
    if ( rElemDesc.Hierarchy < nHierCount )
        xHier = ScUnoHelpFunctions::AnyToInterface( xHiers->getByIndex(rElemDesc.Hierarchy) );
    OSL_ENSURE( xHier.is(), "hierarchy not found" );
    if ( !xHier.is() ) return;

    long nLevCount = 0;
    uno::Reference<container::XIndexAccess> xLevels;
    uno::Reference<sheet::XLevelsSupplier> xLevSupp( xHier, uno::UNO_QUERY );
    if ( xLevSupp.is() )
    {
        uno::Reference<container::XNameAccess> xLevsName = xLevSupp->getLevels();
        xLevels = new ScNameToIndexAccess( xLevsName );
        nLevCount = xLevels->getCount();
    }
    uno::Reference<uno::XInterface> xLevel;
    if ( rElemDesc.Level < nLevCount )
        xLevel = ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex(rElemDesc.Level) );
    OSL_ENSURE( xLevel.is(), "level not found" );
    if ( !xLevel.is() ) return;

    uno::Reference<container::XNameAccess> xMembers;
    uno::Reference<sheet::XMembersSupplier> xMbrSupp( xLevel, uno::UNO_QUERY );
    if ( xMbrSupp.is() )
        xMembers = xMbrSupp->getMembers();

    sal_Bool bFound = false;
    sal_Bool bShowDetails = sal_True;

    if ( xMembers.is() )
    {
        if ( xMembers->hasByName(rElemDesc.MemberName) )
        {
            uno::Reference<uno::XInterface> xMemberInt = ScUnoHelpFunctions::AnyToInterface(
                                            xMembers->getByName(rElemDesc.MemberName) );
            uno::Reference<beans::XPropertySet> xMbrProp( xMemberInt, uno::UNO_QUERY );
            if ( xMbrProp.is() )
            {
                bShowDetails = ScUnoHelpFunctions::GetBoolProperty( xMbrProp,
                                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_SHOWDETAILS)) );
                //! don't set bFound if property is unknown?
                bFound = sal_True;
            }
        }
    }

    OSL_ENSURE( bFound, "member not found" );
    (void)bFound;

    //! use Hierarchy and Level in SaveData !!!!

    //  modify pDestObj if set, this object otherwise
    ScDPSaveData* pModifyData = pDestObj ? ( pDestObj->pSaveData ) : pSaveData;
    OSL_ENSURE( pModifyData, "no data?" );
    if ( pModifyData )
    {
        const String aName = rElemDesc.MemberName;
        pModifyData->GetDimensionByName(aDimName)->
            GetMemberByName(aName)->SetShowDetails( !bShowDetails );    // toggle

        if ( pDestObj )
            pDestObj->InvalidateData();     // re-init source from SaveData
        else
            InvalidateData();               // re-init source from SaveData
    }
}

long lcl_FindName( const rtl::OUString& rString, const uno::Reference<container::XNameAccess>& xCollection )
{
    if ( xCollection.is() )
    {
        uno::Sequence<rtl::OUString> aSeq = xCollection->getElementNames();
        long nCount = aSeq.getLength();
        const rtl::OUString* pArr = aSeq.getConstArray();
        for (long nPos=0; nPos<nCount; nPos++)
            if ( pArr[nPos] == rString )
                return nPos;
    }
    return -1;      // not found
}

sal_uInt16 lcl_FirstSubTotal( const uno::Reference<beans::XPropertySet>& xDimProp )     // PIVOT_FUNC mask
{
    uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDimProp, uno::UNO_QUERY );
    if ( xDimProp.is() && xDimSupp.is() )
    {
        uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess( xDimSupp->getHierarchies() );
        long nHierarchy = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_USEDHIERARCHY)) );
        if ( nHierarchy >= xHiers->getCount() )
            nHierarchy = 0;

        uno::Reference<uno::XInterface> xHier = ScUnoHelpFunctions::AnyToInterface(
                                    xHiers->getByIndex(nHierarchy) );
        uno::Reference<sheet::XLevelsSupplier> xHierSupp( xHier, uno::UNO_QUERY );
        if ( xHierSupp.is() )
        {
            uno::Reference<container::XIndexAccess> xLevels = new ScNameToIndexAccess( xHierSupp->getLevels() );
            uno::Reference<uno::XInterface> xLevel =
                ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex( 0 ) );
            uno::Reference<beans::XPropertySet> xLevProp( xLevel, uno::UNO_QUERY );
            if ( xLevProp.is() )
            {
                uno::Any aSubAny;
                try
                {
                    aSubAny = xLevProp->getPropertyValue(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_SUBTOTALS)) );
                }
                catch(uno::Exception&)
                {
                }
                uno::Sequence<sheet::GeneralFunction> aSeq;
                if ( aSubAny >>= aSeq )
                {
                    sal_uInt16 nMask = 0;
                    const sheet::GeneralFunction* pArray = aSeq.getConstArray();
                    long nCount = aSeq.getLength();
                    for (long i=0; i<nCount; i++)
                        nMask |= ScDataPilotConversion::FunctionBit(pArray[i]);
                    return nMask;
                }
            }
        }
    }

    OSL_FAIL("FirstSubTotal: NULL");
    return 0;
}

sal_uInt16 lcl_CountBits( sal_uInt16 nBits )
{
    if (!nBits) return 0;

    sal_uInt16 nCount = 0;
    sal_uInt16 nMask = 1;
    for (sal_uInt16 i=0; i<16; i++)
    {
        if ( nBits & nMask )
            ++nCount;
        nMask <<= 1;
    }
    return nCount;
}

void lcl_FillOldFields(
    vector<PivotField>& rFields,
    const uno::Reference<sheet::XDimensionsSupplier>& xSource,
    sal_uInt16 nOrient, SCCOL nColAdd, bool bAddData )
{
    vector<PivotField> aFields;

    bool bDataFound = false;

    //! merge multiple occurrences (data field with different functions)
    //! force data field in one dimension

    vector<long> aPos;

    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
    long nDimCount = xDims->getCount();
    for (long nDim = 0; nDim < nDimCount; ++nDim)
    {
        // Get dimension object.
        uno::Reference<uno::XInterface> xIntDim =
            ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );

        // dimension properties
        uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );

        // dimension orientation, hidden by default.
        long nDimOrient = ScUnoHelpFunctions::GetEnumProperty(
                            xDimProp, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ORIENTATION)),
                            sheet::DataPilotFieldOrientation_HIDDEN );

        if ( xDimProp.is() && nDimOrient == nOrient )
        {
            // Let's take this dimension.

            // function mask.
            sal_uInt16 nMask = 0;
            if ( nOrient == sheet::DataPilotFieldOrientation_DATA )
            {
                sheet::GeneralFunction eFunc = (sheet::GeneralFunction)ScUnoHelpFunctions::GetEnumProperty(
                                            xDimProp, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_FUNCTION)),
                                            sheet::GeneralFunction_NONE );
                if ( eFunc == sheet::GeneralFunction_AUTO )
                {
                    //! test for numeric data
                    eFunc = sheet::GeneralFunction_SUM;
                }
                nMask = ScDataPilotConversion::FunctionBit(eFunc);
            }
            else
                nMask = lcl_FirstSubTotal( xDimProp );      // from first hierarchy

            // is this data layout dimension?
            bool bDataLayout = ScUnoHelpFunctions::GetBoolProperty(
                xDimProp, OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ISDATALAYOUT)));

            // is this dimension cloned?
            uno::Any aOrigAny;
            try
            {
                aOrigAny = xDimProp->getPropertyValue(
                    OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ORIGINAL)));
            }
            catch(uno::Exception&)
            {
            }

            long nDupSource = -1;
            uno::Reference<uno::XInterface> xIntOrig = ScUnoHelpFunctions::AnyToInterface( aOrigAny );
            if ( xIntOrig.is() )
            {
                uno::Reference<container::XNamed> xNameOrig( xIntOrig, uno::UNO_QUERY );
                if ( xNameOrig.is() )
                    nDupSource = lcl_FindName( xNameOrig->getName(), xDimsName );
            }

            bool bDupUsed = false;
            if ( nDupSource >= 0 )
            {
                // this dimension is cloned.

                //  add function bit to previous entry

                SCsCOL nCompCol; // column ID of the original dimension.
                if ( bDataLayout )
                    nCompCol = PIVOT_DATA_FIELD;
                else
                    nCompCol = static_cast<SCsCOL>(nDupSource)+nColAdd;     //! seek source column from name

                vector<PivotField>::iterator itr = aFields.begin(), itrEnd = aFields.end();
                for (; itr != itrEnd; ++itr)
                {
                    //  add to previous column only if new bits aren't already set there
                    if (itr->nCol == nCompCol && (itr->nFuncMask & nMask) == 0)
                    {
                        itr->nFuncMask |= nMask;
                        itr->nFuncCount = lcl_CountBits(itr->nFuncMask);
                        bDupUsed = true;
                        break;
                    }
                }
            }

            if ( !bDupUsed )        // also for duplicated dim if original has different orientation
            {
                aFields.push_back(PivotField());
                PivotField& rField = aFields.back();
                if (bDataLayout)
                {
                    rField.nCol = PIVOT_DATA_FIELD;
                    bDataFound = true;
                }
                else if (nDupSource >= 0)
                    rField.nCol = static_cast<SCsCOL>(nDupSource)+nColAdd;      //! seek from name
                else
                    rField.nCol = static_cast<SCsCOL>(nDim)+nColAdd;    //! seek source column from name

                rField.nFuncMask = nMask;
                rField.nFuncCount = lcl_CountBits(nMask);
                long nPos = ScUnoHelpFunctions::GetLongProperty(
                    xDimProp, OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_POSITION)));
                aPos.push_back(nPos);

                try
                {
                    if (nOrient == sheet::DataPilotFieldOrientation_DATA)
                        xDimProp->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_REFVALUE)))
                            >>= rField.maFieldRef;
                }
                catch (uno::Exception&)
                {
                }
            }
        }
    }

    //  sort by getPosition() value

    size_t nOutCount = aFields.size();
    if (nOutCount >= 1)
    {
        for (size_t i = 0; i < nOutCount - 1; ++i)
        {
            for (size_t j = 0; j + i < nOutCount - 1; ++j)
            {
                if ( aPos[j+1] < aPos[j] )
                {
                    std::swap( aPos[j], aPos[j+1] );
                    std::swap( aFields[j], aFields[j+1] );
                }
            }
        }
    }

    if (bAddData && !bDataFound)
        aFields.push_back(PivotField(PIVOT_DATA_FIELD, 0));

    rFields.swap(aFields);
}

bool ScDPObject::FillOldParam(ScPivotParam& rParam) const
{
    ((ScDPObject*)this)->CreateObjects();       // xSource is needed for field numbers

    if (!xSource.is())
        return false;

    rParam.nCol = aOutRange.aStart.Col();
    rParam.nRow = aOutRange.aStart.Row();
    rParam.nTab = aOutRange.aStart.Tab();
    // ppLabelArr / nLabels is not changed

    SCCOL nSrcColOffset = 0;

    bool bAddData = ( lcl_GetDataGetOrientation( xSource ) == sheet::DataPilotFieldOrientation_HIDDEN );
    lcl_FillOldFields(
        rParam.maPageFields, xSource, sheet::DataPilotFieldOrientation_PAGE, nSrcColOffset, false);
    lcl_FillOldFields(
        rParam.maColFields, xSource, sheet::DataPilotFieldOrientation_COLUMN, nSrcColOffset, bAddData);
    lcl_FillOldFields(
        rParam.maRowFields, xSource, sheet::DataPilotFieldOrientation_ROW, nSrcColOffset, false);
    lcl_FillOldFields(
        rParam.maDataFields, xSource, sheet::DataPilotFieldOrientation_DATA, nSrcColOffset, false);

    uno::Reference<beans::XPropertySet> xProp( xSource, uno::UNO_QUERY );
    if (xProp.is())
    {
        try
        {
            rParam.bMakeTotalCol = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_COLUMNGRAND)), true );
            rParam.bMakeTotalRow = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ROWGRAND)), true );

            // following properties may be missing for external sources
            rParam.bIgnoreEmptyRows = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_IGNOREEMPTY)) );
            rParam.bDetectCategories = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_REPEATIFEMPTY)) );
        }
        catch(uno::Exception&)
        {
            // no error
        }
    }
    return true;
}

void lcl_FillLabelData( ScDPLabelData& rData, const uno::Reference< beans::XPropertySet >& xDimProp )
{
    uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDimProp, uno::UNO_QUERY );
    if ( xDimProp.is() && xDimSupp.is() )
    {
        uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess( xDimSupp->getHierarchies() );
        long nHierarchy = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_USEDHIERARCHY)) );
        if ( nHierarchy >= xHiers->getCount() )
            nHierarchy = 0;
        rData.mnUsedHier = nHierarchy;

        uno::Reference<uno::XInterface> xHier = ScUnoHelpFunctions::AnyToInterface(
                                    xHiers->getByIndex(nHierarchy) );

        uno::Reference<sheet::XLevelsSupplier> xHierSupp( xHier, uno::UNO_QUERY );
        if ( xHierSupp.is() )
        {
            uno::Reference<container::XIndexAccess> xLevels = new ScNameToIndexAccess( xHierSupp->getLevels() );
            uno::Reference<uno::XInterface> xLevel =
                ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex( 0 ) );
            uno::Reference<beans::XPropertySet> xLevProp( xLevel, uno::UNO_QUERY );
            if ( xLevProp.is() )
            {
                rData.mbShowAll = ScUnoHelpFunctions::GetBoolProperty( xLevProp,
                                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_SHOWEMPTY)) );

                try
                {
                    xLevProp->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_SORTING ) ) )
                        >>= rData.maSortInfo;
                    xLevProp->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_LAYOUT ) ) )
                        >>= rData.maLayoutInfo;
                    xLevProp->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_AUTOSHOW ) ) )
                        >>= rData.maShowInfo;
                }
                catch(uno::Exception&)
                {
                }
            }
        }
    }
}

bool ScDPObject::FillLabelData(ScPivotParam& rParam)
{
    rParam.maLabelArray.clear();

    ((ScDPObject*)this)->CreateObjects();
    if (!xSource.is())
        return false;

    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
    long nDimCount = xDims->getCount();
    if ( nDimCount > MAX_LABELS )
        nDimCount = MAX_LABELS;
    if (!nDimCount)
        return false;

    for (long nDim=0; nDim < nDimCount; nDim++)
    {
        String aFieldName;
        uno::Reference<uno::XInterface> xIntDim =
            ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
        uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
        uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );

        if ( xDimName.is() && xDimProp.is() )
        {
            bool bDuplicated = false;
            bool bData = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ISDATALAYOUT)) );
            //! error checking -- is "IsDataLayoutDimension" property required??

            try
            {
                aFieldName = String( xDimName->getName() );

                uno::Any aOrigAny = xDimProp->getPropertyValue(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(DP_PROP_ORIGINAL)) );
                uno::Reference<uno::XInterface> xIntOrig;
                if ( (aOrigAny >>= xIntOrig) && xIntOrig.is() )
                    bDuplicated = true;
            }
            catch(uno::Exception&)
            {
            }

            OUString aLayoutName = ScUnoHelpFunctions::GetStringProperty(
                xDimProp, OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_LAYOUTNAME)), OUString());

            if ( aFieldName.Len() && !bData && !bDuplicated )
            {
                SCsCOL nCol = static_cast< SCsCOL >( nDim );           //! ???
                bool bIsValue = true;                               //! check

                ScDPLabelDataRef pNewLabel(new ScDPLabelData(aFieldName, nCol, bIsValue));
                pNewLabel->maLayoutName = aLayoutName;
                GetHierarchies(nDim, pNewLabel->maHiers);
                GetMembers(nDim, GetUsedHierarchy(nDim), pNewLabel->maMembers);
                lcl_FillLabelData(*pNewLabel, xDimProp);
                pNewLabel->mnFlags = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_FLAGS)), 0 );
                rParam.maLabelArray.push_back(pNewLabel);
            }
        }
    }

    return true;
}

bool ScDPObject::GetHierarchiesNA( sal_Int32 nDim, uno::Reference< container::XNameAccess >& xHiers )
{
    bool bRet = false;
    uno::Reference<container::XNameAccess> xDimsName( GetSource()->getDimensions() );
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    if( xIntDims.is() )
    {
        uno::Reference<sheet::XHierarchiesSupplier> xHierSup(xIntDims->getByIndex( nDim ), uno::UNO_QUERY);
        if (xHierSup.is())
        {
            xHiers.set( xHierSup->getHierarchies() );
            bRet = xHiers.is();
        }
    }
    return bRet;
}

bool ScDPObject::GetHierarchies( sal_Int32 nDim, uno::Sequence< rtl::OUString >& rHiers )
{
    bool bRet = false;
    uno::Reference< container::XNameAccess > xHiersNA;
    if( GetHierarchiesNA( nDim, xHiersNA ) )
    {
        rHiers = xHiersNA->getElementNames();
        bRet = true;
    }
    return bRet;
}

sal_Int32 ScDPObject::GetUsedHierarchy( sal_Int32 nDim )
{
    sal_Int32 nHier = 0;
    uno::Reference<container::XNameAccess> xDimsName( GetSource()->getDimensions() );
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    uno::Reference<beans::XPropertySet> xDim(xIntDims->getByIndex( nDim ), uno::UNO_QUERY);
    if (xDim.is())
        nHier = ScUnoHelpFunctions::GetLongProperty( xDim, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_USEDHIER ) ) );
    return nHier;
}

bool ScDPObject::GetMembersNA( sal_Int32 nDim, uno::Reference< container::XNameAccess >& xMembers )
{
    return GetMembersNA( nDim, GetUsedHierarchy( nDim ), xMembers );
}

bool ScDPObject::GetMembersNA( sal_Int32 nDim, sal_Int32 nHier, uno::Reference< container::XNameAccess >& xMembers )
{
    bool bRet = false;
    uno::Reference<container::XNameAccess> xDimsName( GetSource()->getDimensions() );
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    uno::Reference<beans::XPropertySet> xDim(xIntDims->getByIndex( nDim ), uno::UNO_QUERY);
    if (xDim.is())
    {
        uno::Reference<sheet::XHierarchiesSupplier> xHierSup(xDim, uno::UNO_QUERY);
        if (xHierSup.is())
        {
            uno::Reference<container::XIndexAccess> xHiers(new ScNameToIndexAccess(xHierSup->getHierarchies()));
            uno::Reference<sheet::XLevelsSupplier> xLevSupp( xHiers->getByIndex(nHier), uno::UNO_QUERY );
            if ( xLevSupp.is() )
            {
                uno::Reference<container::XIndexAccess> xLevels(new ScNameToIndexAccess( xLevSupp->getLevels()));
                if (xLevels.is())
                {
                    sal_Int32 nLevCount = xLevels->getCount();
                    if (nLevCount > 0)
                    {
                        uno::Reference<sheet::XMembersSupplier> xMembSupp( xLevels->getByIndex(0), uno::UNO_QUERY );
                        if ( xMembSupp.is() )
                        {
                            xMembers.set(xMembSupp->getMembers());
                            bRet = true;
                        }
                    }
                }
            }
        }
    }
    return bRet;
}

//------------------------------------------------------------------------
//  convert old pivot tables into new datapilot tables

namespace {

String lcl_GetDimName( const uno::Reference<sheet::XDimensionsSupplier>& xSource, long nDim )
{
    rtl::OUString aName;
    if ( xSource.is() )
    {
        uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
        uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
        long nDimCount = xDims->getCount();
        if ( nDim < nDimCount )
        {
            uno::Reference<uno::XInterface> xIntDim =
                ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
            uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
            if (xDimName.is())
            {
                try
                {
                    aName = xDimName->getName();
                }
                catch(uno::Exception&)
                {
                }
            }
        }
    }
    return aName;
}

bool hasFieldColumn(const vector<PivotField>* pRefFields, SCCOL nCol)
{
    if (!pRefFields)
        return false;

    vector<PivotField>::const_iterator itr = pRefFields->begin(), itrEnd = pRefFields->end();
    for (; itr != itrEnd; ++itr)
    {
        if (itr->nCol == nCol)
            // This array of fields contains the specified column.
            return true;
    }
    return false;
}

}

void ScDPObject::ConvertOrientation(
    ScDPSaveData& rSaveData, const vector<PivotField>& rFields, sal_uInt16 nOrient,
    const Reference<XDimensionsSupplier>& xSource,
    vector<PivotField>* pRefColFields, vector<PivotField>* pRefRowFields, vector<PivotField>* pRefPageFields )
{
    //  xSource must be set
    OSL_ENSURE( xSource.is(), "missing string source" );

    vector<PivotField>::const_iterator itr, itrBeg = rFields.begin(), itrEnd = rFields.end();
    for (itr = itrBeg; itr != itrEnd; ++itr)
    {
        const PivotField& rField = *itr;

        SCCOL nCol = rField.nCol;
        sal_uInt16 nFuncs = rField.nFuncMask;
        const sheet::DataPilotFieldReference& rFieldRef = rField.maFieldRef;

        ScDPSaveDimension* pDim = NULL;
        if ( nCol == PIVOT_DATA_FIELD )
            pDim = rSaveData.GetDataLayoutDimension();
        else
        {
            String aDocStr = lcl_GetDimName( xSource, nCol );   // cols must start at 0
            if ( aDocStr.Len() )
                pDim = rSaveData.GetDimensionByName(aDocStr);
            else
                pDim = NULL;
        }

        if (!pDim)
            continue;

        if ( nOrient == sheet::DataPilotFieldOrientation_DATA )     // set summary function
        {
            //  generate an individual entry for each function
            bool bFirst = true;

            //  if a dimension is used for column/row/page and data,
            //  use duplicated dimensions for all data occurrences
            if (hasFieldColumn(pRefColFields, nCol))
                bFirst = false;

            if (bFirst && hasFieldColumn(pRefRowFields, nCol))
                bFirst = false;

            if (bFirst && hasFieldColumn(pRefPageFields, nCol))
                bFirst = false;

            if (bFirst)
            {
                //  if set via api, a data column may occur several times
                //  (if the function hasn't been changed yet) -> also look for duplicate data column
                for (vector<PivotField>::const_iterator itr2 = itrBeg; itr2 != itr; ++itr2)
                {
                    if (itr2->nCol == nCol)
                    {
                        bFirst = false;
                        break;
                    }
                }
            }

            sal_uInt16 nMask = 1;
            for (sal_uInt16 nBit=0; nBit<16; nBit++)
            {
                if ( nFuncs & nMask )
                {
                    sheet::GeneralFunction eFunc = ScDataPilotConversion::FirstFunc( nMask );
                    ScDPSaveDimension* pCurrDim = bFirst ? pDim : rSaveData.DuplicateDimension(pDim->GetName());
                    pCurrDim->SetOrientation( nOrient );
                    pCurrDim->SetFunction( sal::static_int_cast<sal_uInt16>(eFunc) );

                    if( rFieldRef.ReferenceType == sheet::DataPilotFieldReferenceType::NONE )
                        pCurrDim->SetReferenceValue( 0 );
                    else
                        pCurrDim->SetReferenceValue( &rFieldRef );

                    bFirst = false;
                }
                nMask *= 2;
            }
        }
        else                                            // set SubTotals
        {
            pDim->SetOrientation( nOrient );

            sal_uInt16 nFuncArray[16];
            sal_uInt16 nFuncCount = 0;
            sal_uInt16 nMask = 1;
            for (sal_uInt16 nBit=0; nBit<16; nBit++)
            {
                if ( nFuncs & nMask )
                    nFuncArray[nFuncCount++] = sal::static_int_cast<sal_uInt16>(ScDataPilotConversion::FirstFunc( nMask ));
                nMask *= 2;
            }
            pDim->SetSubTotals( nFuncCount, nFuncArray );

            //  ShowEmpty was implicit in old tables,
            //  must be set for data layout dimension (not accessible in dialog)
            if ( nCol == PIVOT_DATA_FIELD )
                pDim->SetShowEmpty( true );
        }
    }
}

bool ScDPObject::IsOrientationAllowed( sal_uInt16 nOrient, sal_Int32 nDimFlags )
{
    bool bAllowed = true;
    switch (nOrient)
    {
        case sheet::DataPilotFieldOrientation_PAGE:
            bAllowed = ( nDimFlags & sheet::DimensionFlags::NO_PAGE_ORIENTATION ) == 0;
            break;
        case sheet::DataPilotFieldOrientation_COLUMN:
            bAllowed = ( nDimFlags & sheet::DimensionFlags::NO_COLUMN_ORIENTATION ) == 0;
            break;
        case sheet::DataPilotFieldOrientation_ROW:
            bAllowed = ( nDimFlags & sheet::DimensionFlags::NO_ROW_ORIENTATION ) == 0;
            break;
        case sheet::DataPilotFieldOrientation_DATA:
            bAllowed = ( nDimFlags & sheet::DimensionFlags::NO_DATA_ORIENTATION ) == 0;
            break;
        default:
            {
                // allowed to remove from previous orientation
            }
    }
    return bAllowed;
}

// -----------------------------------------------------------------------

bool ScDPObject::HasRegisteredSources()
{
    bool bFound = false;

    uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
    uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum = xEnAc->createContentEnumeration(
                                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SCDPSOURCE_SERVICE )) );
        if ( xEnum.is() && xEnum->hasMoreElements() )
            bFound = true;
    }

    return bFound;
}

uno::Sequence<rtl::OUString> ScDPObject::GetRegisteredSources()
{
    uno::Sequence<rtl::OUString> aSeq(0);

    //  use implementation names...

    uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
    uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum = xEnAc->createContentEnumeration(
                                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SCDPSOURCE_SERVICE )) );
        if ( xEnum.is() )
        {
            long nCount = 0;
            while ( xEnum->hasMoreElements() )
            {
                uno::Any aAddInAny = xEnum->nextElement();
//              if ( aAddInAny.getReflection()->getTypeClass() == TypeClass_INTERFACE )
                {
                    uno::Reference<uno::XInterface> xIntFac;
                    aAddInAny >>= xIntFac;
                    if ( xIntFac.is() )
                    {
                        uno::Reference<lang::XServiceInfo> xInfo( xIntFac, uno::UNO_QUERY );
                        if ( xInfo.is() )
                        {
                            rtl::OUString sName = xInfo->getImplementationName();

                            aSeq.realloc( nCount+1 );
                            aSeq.getArray()[nCount] = sName;
                            ++nCount;
                        }
                    }
                }
            }
        }
    }

    return aSeq;
}

// use getContext from addincol.cxx
uno::Reference<uno::XComponentContext> getContext(uno::Reference<lang::XMultiServiceFactory> xMSF);

uno::Reference<sheet::XDimensionsSupplier> ScDPObject::CreateSource( const ScDPServiceDesc& rDesc )
{
    rtl::OUString aImplName = rDesc.aServiceName;
    uno::Reference<sheet::XDimensionsSupplier> xRet = NULL;

    uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
    uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum = xEnAc->createContentEnumeration(
                                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SCDPSOURCE_SERVICE )) );
        if ( xEnum.is() )
        {
            while ( xEnum->hasMoreElements() && !xRet.is() )
            {
                uno::Any aAddInAny = xEnum->nextElement();
//              if ( aAddInAny.getReflection()->getTypeClass() == TypeClass_INTERFACE )
                {
                    uno::Reference<uno::XInterface> xIntFac;
                    aAddInAny >>= xIntFac;
                    if ( xIntFac.is() )
                    {
                        uno::Reference<lang::XServiceInfo> xInfo( xIntFac, uno::UNO_QUERY );
                        if ( xInfo.is() && xInfo->getImplementationName() == aImplName )
                        {
                            try
                            {
                                // #i113160# try XSingleComponentFactory in addition to (old) XSingleServiceFactory,
                                // passing the context to the component (see ScUnoAddInCollection::Initialize)

                                uno::Reference<uno::XInterface> xInterface;
                                uno::Reference<uno::XComponentContext> xCtx = getContext(xManager);
                                uno::Reference<lang::XSingleComponentFactory> xCFac( xIntFac, uno::UNO_QUERY );
                                if (xCtx.is() && xCFac.is())
                                    xInterface = xCFac->createInstanceWithContext(xCtx);

                                if (!xInterface.is())
                                {
                                    uno::Reference<lang::XSingleServiceFactory> xFac( xIntFac, uno::UNO_QUERY );
                                    if ( xFac.is() )
                                        xInterface = xFac->createInstance();
                                }

                                uno::Reference<lang::XInitialization> xInit( xInterface, uno::UNO_QUERY );
                                if (xInit.is())
                                {
                                    //  initialize
                                    uno::Sequence<uno::Any> aSeq(4);
                                    uno::Any* pArray = aSeq.getArray();
                                    pArray[0] <<= rtl::OUString( rDesc.aParSource );
                                    pArray[1] <<= rtl::OUString( rDesc.aParName );
                                    pArray[2] <<= rtl::OUString( rDesc.aParUser );
                                    pArray[3] <<= rtl::OUString( rDesc.aParPass );
                                    xInit->initialize( aSeq );
                                }
                                xRet = uno::Reference<sheet::XDimensionsSupplier>( xInterface, uno::UNO_QUERY );
                            }
                            catch(uno::Exception&)
                            {
                            }
                        }
                    }
                }
            }
        }
    }

    return xRet;
}

ScDPCollection::SheetCaches::SheetCaches(ScDocument* pDoc) : mpDoc(pDoc) {}

namespace {

struct FindInvalidRange : public std::unary_function<ScRange, bool>
{
    bool operator() (const ScRange& r) const
    {
        return !r.IsValid();
    }
};

}

bool ScDPCollection::SheetCaches::hasCache(const ScRange& rRange) const
{
    RangeIndexType::const_iterator it = std::find(maRanges.begin(), maRanges.end(), rRange);
    if (it == maRanges.end())
        return false;

    // Already cached.
    size_t nIndex = std::distance(maRanges.begin(), it);
    CachesType::const_iterator itCache = maCaches.find(nIndex);
    return itCache != maCaches.end();
}

const ScDPCache* ScDPCollection::SheetCaches::getCache(const ScRange& rRange)
{
    RangeIndexType::iterator it = std::find(maRanges.begin(), maRanges.end(), rRange);
    if (it != maRanges.end())
    {
        // Already cached.
        size_t nIndex = std::distance(maRanges.begin(), it);
        CachesType::iterator itCache = maCaches.find(nIndex);
        if (itCache == maCaches.end())
        {
            OSL_FAIL("Cache pool and index pool out-of-sync !!!");
            return NULL;
        }

        return itCache->second;
    }

    // Not cached.  Create a new cache.
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScDPCache> pCache(new ScDPCache(mpDoc));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    pCache->InitFromDoc(mpDoc, rRange);

    // Get the smallest available range index.
    it = std::find_if(maRanges.begin(), maRanges.end(), FindInvalidRange());

    size_t nIndex = maRanges.size();
    if (it == maRanges.end())
    {
        // All range indices are valid.  Append a new index.
        maRanges.push_back(rRange);
    }
    else
    {
        // Slot with invalid range.  Re-use this slot.
        *it = rRange;
        nIndex = std::distance(maRanges.begin(), it);
    }

    const ScDPCache* p = pCache.get();
    maCaches.insert(nIndex, pCache);
    return p;
}

size_t ScDPCollection::SheetCaches::size() const
{
    return maCaches.size();
}

void ScDPCollection::SheetCaches::updateReference(
    UpdateRefMode eMode, const ScRange& r, SCsCOL nDx, SCsROW nDy, SCsTAB nDz)
{
    if (maRanges.empty())
        // No caches.
        return;

    RangeIndexType::iterator it = maRanges.begin(), itEnd = maRanges.end();
    for (; it != itEnd; ++it)
    {
        const ScRange& rKeyRange = *it;
        SCCOL nCol1 = rKeyRange.aStart.Col();
        SCROW nRow1 = rKeyRange.aStart.Row();
        SCTAB nTab1 = rKeyRange.aStart.Tab();
        SCCOL nCol2 = rKeyRange.aEnd.Col();
        SCROW nRow2 = rKeyRange.aEnd.Row();
        SCTAB nTab2 = rKeyRange.aEnd.Tab();

        ScRefUpdateRes eRes = ScRefUpdate::Update(
            mpDoc, eMode,
            r.aStart.Col(), r.aStart.Row(), r.aStart.Tab(),
            r.aEnd.Col(), r.aEnd.Row(), r.aEnd.Tab(), nDx, nDy, nDz,
            nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);

        if (eRes != UR_NOTHING)
        {
            // range updated.
            ScRange aNew(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            *it = aNew;
        }
    }
}

void ScDPCollection::SheetCaches::updateCache(const ScRange& rRange, std::set<ScDPObject*>& rRefs)
{
    RangeIndexType::iterator it = std::find(maRanges.begin(), maRanges.end(), rRange);
    if (it == maRanges.end())
    {
        // Not cached.  Nothing to do.
        rRefs.clear();
        return;
    }

    size_t nIndex = std::distance(maRanges.begin(), it);
    CachesType::iterator itCache = maCaches.find(nIndex);
    if (itCache == maCaches.end())
    {
        OSL_FAIL("Cache pool and index pool out-of-sync !!!");
        rRefs.clear();
        return;
    }

    ScDPCache& rCache = *itCache->second;
    rCache.InitFromDoc(mpDoc, rRange);
    std::set<ScDPObject*> aRefs(rCache.GetAllReferences());
    rRefs.swap(aRefs);
}

void ScDPCollection::SheetCaches::removeCache(const ScRange& rRange)
{
    RangeIndexType::iterator it = std::find(maRanges.begin(), maRanges.end(), rRange);
    if (it == maRanges.end())
        // Not cached.  Nothing to do.
        return;

    size_t nIndex = std::distance(maRanges.begin(), it);
    CachesType::iterator itCache = maCaches.find(nIndex);
    if (itCache == maCaches.end())
    {
        OSL_FAIL("Cache pool and index pool out-of-sync !!!");
        return;
    }

    it->SetInvalid(); // Make this slot available for future caches.
    maCaches.erase(itCache);
}

bool ScDPCollection::SheetCaches::remove(const ScDPCache* p)
{
    CachesType::iterator it = maCaches.begin(), itEnd = maCaches.end();
    for (; it != itEnd; ++it)
    {
        if (it->second == p)
        {
            size_t idx = it->first;
            maCaches.erase(it);
            maRanges[idx].SetInvalid();
            return true;
        }
    }
    return false;
}

ScDPCollection::NameCaches::NameCaches(ScDocument* pDoc) : mpDoc(pDoc) {}

bool ScDPCollection::NameCaches::hasCache(const OUString& rName) const
{
    return maCaches.count(rName) != 0;
}

const ScDPCache* ScDPCollection::NameCaches::getCache(const OUString& rName, const ScRange& rRange)
{
    CachesType::const_iterator itr = maCaches.find(rName);
    if (itr != maCaches.end())
        // already cached.
        return itr->second;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScDPCache> pCache(new ScDPCache(mpDoc));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    pCache->InitFromDoc(mpDoc, rRange);
    const ScDPCache* p = pCache.get();
    maCaches.insert(rName, pCache);
    return p;
}

size_t ScDPCollection::NameCaches::size() const
{
    return maCaches.size();
}

void ScDPCollection::NameCaches::updateCache(const OUString& rName, const ScRange& rRange, std::set<ScDPObject*>& rRefs)
{
    CachesType::iterator itr = maCaches.find(rName);
    if (itr == maCaches.end())
    {
        rRefs.clear();
        return;
    }

    ScDPCache& rCache = *itr->second;
    rCache.InitFromDoc(mpDoc, rRange);
    std::set<ScDPObject*> aRefs(rCache.GetAllReferences());
    rRefs.swap(aRefs);
}

void ScDPCollection::NameCaches::removeCache(const OUString& rName)
{
    CachesType::iterator itr = maCaches.find(rName);
    if (itr != maCaches.end())
        maCaches.erase(itr);
}

bool ScDPCollection::NameCaches::remove(const ScDPCache* p)
{
    CachesType::iterator it = maCaches.begin(), itEnd = maCaches.end();
    for (; it != itEnd; ++it)
    {
        if (it->second == p)
        {
            maCaches.erase(it);
            return true;
        }
    }
    return false;
}

ScDPCollection::DBType::DBType(sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand) :
    mnSdbType(nSdbType), maDBName(rDBName), maCommand(rCommand) {}

bool ScDPCollection::DBType::less::operator() (const DBType& left, const DBType& right) const
{
    return left < right;
}

ScDPCollection::DBCaches::DBCaches(ScDocument* pDoc) : mpDoc(pDoc) {}

bool ScDPCollection::DBCaches::hasCache(sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand) const
{
    DBType aType(nSdbType, rDBName, rCommand);
    CachesType::const_iterator itr = maCaches.find(aType);
    return itr != maCaches.end();
}

const ScDPCache* ScDPCollection::DBCaches::getCache(sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand)
{
    DBType aType(nSdbType, rDBName, rCommand);
    CachesType::const_iterator itr = maCaches.find(aType);
    if (itr != maCaches.end())
        // already cached.
        return itr->second;

    uno::Reference<sdbc::XRowSet> xRowSet = createRowSet(nSdbType, rDBName, rCommand);
    if (!xRowSet.is())
        return NULL;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScDPCache> pCache(new ScDPCache(mpDoc));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    SvNumberFormatter aFormat(mpDoc->GetServiceManager(), ScGlobal::eLnge);
    pCache->InitFromDataBase(xRowSet, *aFormat.GetNullDate());
    ::comphelper::disposeComponent(xRowSet);
    const ScDPCache* p = pCache.get();
    maCaches.insert(aType, pCache);
    return p;
}

size_t ScDPCollection::DBCaches::size() const
{
    return maCaches.size();
}

uno::Reference<sdbc::XRowSet> ScDPCollection::DBCaches::createRowSet(
    sal_Int32 nSdbType, const ::rtl::OUString& rDBName, const ::rtl::OUString& rCommand)
{
    uno::Reference<sdbc::XRowSet> xRowSet;
    try
    {
        xRowSet = uno::Reference<sdbc::XRowSet>(
            comphelper::getProcessServiceFactory()->createInstance(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_SERVICE_ROWSET))),
            UNO_QUERY);

        uno::Reference<beans::XPropertySet> xRowProp(xRowSet, UNO_QUERY);
        OSL_ENSURE( xRowProp.is(), "can't get RowSet" );
        if (!xRowProp.is())
        {
            xRowSet.set(NULL);
            return xRowSet;
        }

        //
        //  set source parameters
        //
        uno::Any aAny;
        aAny <<= rDBName;
        xRowProp->setPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_DATASOURCENAME)), aAny );

        aAny <<= rCommand;
        xRowProp->setPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_COMMAND)), aAny );

        aAny <<= nSdbType;
        xRowProp->setPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DBPROP_COMMANDTYPE)), aAny );

        uno::Reference<sdb::XCompletedExecution> xExecute( xRowSet, uno::UNO_QUERY );
        if ( xExecute.is() )
        {
            uno::Reference<task::XInteractionHandler> xHandler(
                comphelper::getProcessServiceFactory()->createInstance(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_SERVICE_INTHANDLER )) ),
                uno::UNO_QUERY);
            xExecute->executeWithCompletion( xHandler );
        }
        else
            xRowSet->execute();

        return xRowSet;
    }
    catch ( const sdbc::SQLException& rError )
    {
        //! store error message
        InfoBox aInfoBox( 0, String(rError.Message) );
        aInfoBox.Execute();
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL("Unexpected exception in database");
    }

    xRowSet.set(NULL);
    return xRowSet;
}

void ScDPCollection::DBCaches::updateCache(
    sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand, std::set<ScDPObject*>& rRefs)
{
    DBType aType(nSdbType, rDBName, rCommand);
    CachesType::iterator it = maCaches.find(aType);
    if (it == maCaches.end())
    {
        // not cached.
        rRefs.clear();
        return;
    }

    ScDPCache& rCache = *it->second;

    uno::Reference<sdbc::XRowSet> xRowSet = createRowSet(nSdbType, rDBName, rCommand);
    if (!xRowSet.is())
    {
        rRefs.clear();
        return;
    }

    SvNumberFormatter aFormat(mpDoc->GetServiceManager(), ScGlobal::eLnge);
    if (!rCache.InitFromDataBase(xRowSet, *aFormat.GetNullDate()))
    {
        // initialization failed.
        rRefs.clear();
        return;
    }

    comphelper::disposeComponent(xRowSet);
    std::set<ScDPObject*> aRefs(rCache.GetAllReferences());
    aRefs.swap(rRefs);
}

void ScDPCollection::DBCaches::removeCache(sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand)
{
    DBType aType(nSdbType, rDBName, rCommand);
    CachesType::iterator itr = maCaches.find(aType);
    if (itr != maCaches.end())
        maCaches.erase(itr);
}

bool ScDPCollection::DBCaches::remove(const ScDPCache* p)
{
    CachesType::iterator it = maCaches.begin(), itEnd = maCaches.end();
    for (; it != itEnd; ++it)
    {
        if (it->second == p)
        {
            maCaches.erase(it);
            return true;
        }
    }
    return false;
}

ScDPCollection::ScDPCollection(ScDocument* pDocument) :
    pDoc( pDocument ),
    maSheetCaches(pDocument),
    maNameCaches(pDocument),
    maDBCaches(pDocument)
{
}

ScDPCollection::ScDPCollection(const ScDPCollection& r) :
    pDoc(r.pDoc),
    maSheetCaches(r.pDoc),
    maNameCaches(r.pDoc),
    maDBCaches(r.pDoc)
{
}

ScDPCollection::~ScDPCollection()
{
    maTables.clear();
}

namespace {

/**
 * Unary predicate to match DP objects by the table ID.
 */
class MatchByTable : public unary_function<ScDPObject, bool>
{
    SCTAB mnTab;
public:
    MatchByTable(SCTAB nTab) : mnTab(nTab) {}

    bool operator() (const ScDPObject& rObj) const
    {
        return rObj.GetOutRange().aStart.Tab() == mnTab;
    }
};

}

sal_uLong ScDPCollection::ReloadCache(ScDPObject* pDPObj, std::set<ScDPObject*>& rRefs)
{
    if (pDPObj->IsSheetData())
    {
        // data source is internal sheet.
        const ScSheetSourceDesc* pDesc = pDPObj->GetSheetDesc();
        if (!pDesc)
            return STR_ERR_DATAPILOTSOURCE;

        sal_uLong nErrId = pDesc->CheckSourceRange();
        if (nErrId)
            return nErrId;

        if (pDesc->HasRangeName())
        {
            // cache by named range
            ScDPCollection::NameCaches& rCaches = GetNameCaches();
            if (rCaches.hasCache(pDesc->GetRangeName()))
                rCaches.updateCache(pDesc->GetRangeName(), pDesc->GetSourceRange(), rRefs);
            else
            {
                // Not cached yet.  Collect all tables that use this named
                // range as data source.
                GetAllTables(pDesc->GetRangeName(), rRefs);
            }
        }
        else
        {
            // cache by cell range
            ScDPCollection::SheetCaches& rCaches = GetSheetCaches();
            if (rCaches.hasCache(pDesc->GetSourceRange()))
                rCaches.updateCache(pDesc->GetSourceRange(), rRefs);
            else
            {
                // Not cached yet.  Collect all tables that use this range as
                // data source.
                GetAllTables(pDesc->GetSourceRange(), rRefs);
            }
        }
    }
    else if (pDPObj->IsImportData())
    {
        // data source is external database.
        const ScImportSourceDesc* pDesc = pDPObj->GetImportSourceDesc();
        if (!pDesc)
            return STR_ERR_DATAPILOTSOURCE;

        ScDPCollection::DBCaches& rCaches = GetDBCaches();
        if (rCaches.hasCache(pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject))
            rCaches.updateCache(
                pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject, rRefs);
        else
        {
            // Not cached yet.  Collect all tables that use this range as
            // data source.
            GetAllTables(pDesc->GetCommandType(), pDesc->aDBName, pDesc->aObject, rRefs);
        }
    }
    return 0;
}

void ScDPCollection::DeleteOnTab( SCTAB nTab )
{
    maTables.erase_if(MatchByTable(nTab));
}

void ScDPCollection::UpdateReference( UpdateRefMode eUpdateRefMode,
                                         const ScRange& r, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    TablesType::iterator itr = maTables.begin(), itrEnd = maTables.end();
    for (; itr != itrEnd; ++itr)
        itr->UpdateReference(eUpdateRefMode, r, nDx, nDy, nDz);

    // Update the source ranges of the caches.
    maSheetCaches.updateReference(eUpdateRefMode, r, nDx, nDy, nDz);
}

void ScDPCollection::CopyToTab( SCTAB nOld, SCTAB nNew )
{
    TablesType aAdded;
    TablesType::const_iterator it = maTables.begin(), itEnd = maTables.end();
    for (; it != itEnd; ++it)
    {
        const ScDPObject& rObj = *it;
        ScRange aOutRange = rObj.GetOutRange();
        if (aOutRange.aStart.Tab() != nOld)
            continue;

        ScAddress& s = aOutRange.aStart;
        ScAddress& e = aOutRange.aEnd;
        s.SetTab(nNew);
        e.SetTab(nNew);
        std::auto_ptr<ScDPObject> pNew(new ScDPObject(rObj));
        pNew->SetOutRange(aOutRange);
        pDoc->ApplyFlagsTab(s.Col(), s.Row(), e.Col(), e.Row(), s.Tab(), SC_MF_DP_TABLE);
        aAdded.push_back(pNew);
    }

    maTables.transfer(maTables.end(), aAdded.begin(), aAdded.end(), aAdded);
}

bool ScDPCollection::RefsEqual( const ScDPCollection& r ) const
{
    if (maTables.size() != r.maTables.size())
        return false;

    TablesType::const_iterator itr = maTables.begin(), itr2 = r.maTables.begin(), itrEnd = maTables.end();
    for (; itr != itrEnd; ++itr, ++itr2)
        if (!itr->RefsEqual(*itr2))
            return false;

    return true;
}

void ScDPCollection::WriteRefsTo( ScDPCollection& r ) const
{
    if ( maTables.size() == r.maTables.size() )
    {
        //! assert equal names?
        TablesType::const_iterator itr = maTables.begin(), itrEnd = maTables.end();
        TablesType::iterator itr2 = r.maTables.begin();
        for (; itr != itrEnd; ++itr, ++itr2)
            itr->WriteRefsTo(*itr2);
    }
    else
    {
        // #i8180# If data pilot tables were deleted with their sheet,
        // this collection contains extra entries that must be restored.
        // Matching objects are found by their names.
        size_t nSrcSize = maTables.size();
        size_t nDestSize = r.maTables.size();
        OSL_ENSURE( nSrcSize >= nDestSize, "WriteRefsTo: missing entries in document" );
        for (size_t nSrcPos = 0; nSrcPos < nSrcSize; ++nSrcPos)
        {
            const ScDPObject& rSrcObj = maTables[nSrcPos];
            const OUString& aName = rSrcObj.GetName();
            bool bFound = false;
            for (size_t nDestPos = 0; nDestPos < nDestSize && !bFound; ++nDestPos)
            {
                ScDPObject& rDestObj = r.maTables[nDestPos];
                if (rDestObj.GetName() == aName)
                {
                    rSrcObj.WriteRefsTo(rDestObj);     // found object, copy refs
                    bFound = true;
                }
            }

            if (!bFound)
            {
                // none found, re-insert deleted object (see ScUndoDataPilot::Undo)

                ScDPObject* pDestObj = new ScDPObject(rSrcObj);
                pDestObj->SetAlive(true);
                r.InsertNewTable(pDestObj);
            }
        }
        OSL_ENSURE( maTables.size() == r.maTables.size(), "WriteRefsTo: couldn't restore all entries" );
    }
}

size_t ScDPCollection::GetCount() const
{
    return maTables.size();
}

ScDPObject* ScDPCollection::operator [](size_t nIndex)
{
    return &maTables[nIndex];
}

const ScDPObject* ScDPCollection::operator [](size_t nIndex) const
{
    return &maTables[nIndex];
}

const ScDPObject* ScDPCollection::GetByName(const OUString& rName) const
{
    TablesType::const_iterator itr = maTables.begin(), itrEnd = maTables.end();
    for (; itr != itrEnd; ++itr)
        if (itr->GetName() == rName)
            return &(*itr);

    return NULL;
}

OUString ScDPCollection::CreateNewName( sal_uInt16 nMin ) const
{
    OUString aBase(RTL_CONSTASCII_USTRINGPARAM("DataPilot"));

    size_t n = maTables.size();
    for (size_t nAdd = 0; nAdd <= n; ++nAdd)   //  nCount+1 tries
    {
        ::rtl::OUStringBuffer aBuf;
        aBuf.append(aBase);
        aBuf.append(static_cast<sal_Int32>(nMin + nAdd));
        OUString aNewName = aBuf.makeStringAndClear();
        bool bFound = false;
        TablesType::const_iterator itr = maTables.begin(), itrEnd = maTables.end();
        for (; itr != itrEnd; ++itr)
        {
            if (itr->GetName() == aNewName)
            {
                bFound = true;
                break;
            }
        }
        if (!bFound)
            return aNewName;            // found unused Name
    }
    return OUString();                    // should not happen
}

void ScDPCollection::FreeTable(ScDPObject* pDPObj)
{
    const ScRange& rOutRange = pDPObj->GetOutRange();
    const ScAddress& s = rOutRange.aStart;
    const ScAddress& e = rOutRange.aEnd;
    pDoc->RemoveFlagsTab(s.Col(), s.Row(), e.Col(), e.Row(), s.Tab(), SC_MF_DP_TABLE);
    TablesType::iterator itr = maTables.begin(), itrEnd = maTables.end();
    for (; itr != itrEnd; ++itr)
    {
        ScDPObject* p = &(*itr);
        if (p == pDPObj)
        {
            maTables.erase(itr);
            break;
        }
    }
}

bool ScDPCollection::InsertNewTable(ScDPObject* pDPObj)
{
    const ScRange& rOutRange = pDPObj->GetOutRange();
    const ScAddress& s = rOutRange.aStart;
    const ScAddress& e = rOutRange.aEnd;
    pDoc->ApplyFlagsTab(s.Col(), s.Row(), e.Col(), e.Row(), s.Tab(), SC_MF_DP_TABLE);

    maTables.push_back(pDPObj);
    return true;
}

bool ScDPCollection::HasDPTable(SCCOL nCol, SCROW nRow, SCTAB nTab) const
{
    const ScMergeFlagAttr* pMergeAttr = static_cast<const ScMergeFlagAttr*>(
            pDoc->GetAttr(nCol, nRow, nTab, ATTR_MERGE_FLAG));

    if (!pMergeAttr)
        return false;

    return pMergeAttr->HasDPTable();
}

ScDPCollection::SheetCaches& ScDPCollection::GetSheetCaches()
{
    return maSheetCaches;
}

ScDPCollection::NameCaches& ScDPCollection::GetNameCaches()
{
    return maNameCaches;
}

ScDPCollection::DBCaches& ScDPCollection::GetDBCaches()
{
    return maDBCaches;
}

void ScDPCollection::RemoveCache(const ScDPCache* pCache)
{
    if (maSheetCaches.remove(pCache))
        // sheet cache removed.
        return;

    if (maNameCaches.remove(pCache))
        // named range cache removed.
        return;

    if (maDBCaches.remove(pCache))
        // database cache removed.
        return;
}

void ScDPCollection::GetAllTables(const ScRange& rSrcRange, std::set<ScDPObject*>& rRefs) const
{
    std::set<ScDPObject*> aRefs;
    TablesType::const_iterator it = maTables.begin(), itEnd = maTables.end();
    for (; it != itEnd; ++it)
    {
        const ScDPObject& rObj = *it;
        if (!rObj.IsSheetData())
            // Source is not a sheet range.
            continue;

        const ScSheetSourceDesc* pDesc = rObj.GetSheetDesc();
        if (!pDesc)
            continue;

        if (pDesc->HasRangeName())
            // This table has a range name as its source.
            continue;

        if (pDesc->GetSourceRange() != rSrcRange)
            // Different source range.
            continue;

        aRefs.insert(const_cast<ScDPObject*>(&rObj));
    }

    rRefs.swap(aRefs);
}

void ScDPCollection::GetAllTables(const rtl::OUString& rSrcName, std::set<ScDPObject*>& rRefs) const
{
    std::set<ScDPObject*> aRefs;
    TablesType::const_iterator it = maTables.begin(), itEnd = maTables.end();
    for (; it != itEnd; ++it)
    {
        const ScDPObject& rObj = *it;
        if (!rObj.IsSheetData())
            // Source is not a sheet range.
            continue;

        const ScSheetSourceDesc* pDesc = rObj.GetSheetDesc();
        if (!pDesc)
            continue;

        if (!pDesc->HasRangeName())
            // This table probably has a sheet range as its source.
            continue;

        if (pDesc->GetRangeName() != rSrcName)
            // Different source name.
            continue;

        aRefs.insert(const_cast<ScDPObject*>(&rObj));
    }

    rRefs.swap(aRefs);
}

void ScDPCollection::GetAllTables(
    sal_Int32 nSdbType, const ::rtl::OUString& rDBName, const ::rtl::OUString& rCommand,
    std::set<ScDPObject*>& rRefs) const
{
    std::set<ScDPObject*> aRefs;
    TablesType::const_iterator it = maTables.begin(), itEnd = maTables.end();
    for (; it != itEnd; ++it)
    {
        const ScDPObject& rObj = *it;
        if (!rObj.IsImportData())
            // Source data is not a database.
            continue;

        const ScImportSourceDesc* pDesc = rObj.GetImportSourceDesc();
        if (!pDesc)
            continue;

        if (!pDesc->aDBName.equals(rDBName) || !pDesc->aObject.equals(rCommand) || pDesc->GetCommandType() != nSdbType)
            // Different database source.
            continue;

        aRefs.insert(const_cast<ScDPObject*>(&rObj));
    }

    rRefs.swap(aRefs);
}

bool operator<(const ScDPCollection::DBType& left, const ScDPCollection::DBType& right)
{
    if (left.mnSdbType != right.mnSdbType)
        return left.mnSdbType < right.mnSdbType;

    if (!left.maDBName.equals(right.maDBName))
        return left.maDBName < right.maDBName;

    return left.maCommand < right.maCommand;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
