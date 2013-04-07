/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009.
 * Copyright 2009 by Sun Microsystems, Inc.
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
#ifndef SC_DPTABLECACHE_HXX
#define SC_DPTABLECACHE_HXX

#include "global.hxx"
#include <svl/zforlist.hxx>
#include "dpglobal.hxx"

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

struct ScQueryParam;

/**
 * This class represents the cached data part of the datapilot cache table
 * implementation.
 */
class SC_DLLPUBLIC ScDPCache
{
public:
    typedef ::boost::ptr_vector<ScDPItemData>           DataListType;
    typedef std::set<ScDPObject*> ObjectSetType;
private:
    typedef ::boost::ptr_vector<DataListType>           DataGridType;
    typedef ::boost::ptr_vector< ::std::vector<SCROW> > RowGridType;

    ScDocument* mpDoc;
    long mnColumnCount;

    /**
     * All pivot table objects that references this cache.
     */
    mutable ObjectSetType maRefObjects;

    /**
     * This container stores only the unique instances of item data in each
     * column. Duplicates are not allowed.
     */
    DataGridType maTableDataValues;

    /**
     * This container stores indices within maTableDataValues pointing to the
     * data.  The order of data are exactly as they appear in the original
     * data source.
     */
    RowGridType maSourceData;

    /**
     * This container stores indices within maTableDataValues.  The order of
     * indices in each column represents ascending order of the actual data.
     */
    RowGridType maGlobalOrder;

    /**
     * This container stores the ranks of each unique data represented by
     * their index.
     */
    mutable RowGridType maIndexOrder;

    DataListType maLabelNames;    // Stores dimension names.
    std::vector<bool> mbEmptyRow; // Keeps track of empty rows.

    mutable ScDPItemDataPool    maAdditionalData;

    bool mbDisposing;

public:
    void AddReference(ScDPObject* pObj) const;
    void RemoveReference(ScDPObject* pObj) const;
    const ObjectSetType& GetAllReferences() const;

    SCROW GetIdByItemData( long nDim, const String& sItemData ) const;
    SCROW GetIdByItemData( long nDim, const ScDPItemData& rData ) const;

    SCROW GetAdditionalItemID ( const String& sItemData ) const;
    SCROW GetAdditionalItemID( const ScDPItemData& rData ) const;

    SCCOL GetDimensionIndex( String sName) const;
    const ScDPItemData* GetSortedItemData( SCCOL nDim, SCROW nOrder ) const;
    sal_uLong GetNumType ( sal_uLong nFormat ) const;
    sal_uLong GetNumberFormat( long nDim ) const;
    bool  IsDateDimension( long nDim ) const ;
    sal_uLong GetDimNumType( SCCOL nDim) const;
    SCROW GetDimMemberCount( SCCOL nDim ) const;
    SCROW GetOrder( long nDim, SCROW nIndex ) const;

    SCROW GetSortedItemDataId( SCCOL nDim, SCROW nOrder ) const;
    const DataListType& GetDimMemberValues( SCCOL nDim ) const;
    bool InitFromDoc(ScDocument* pDoc, const ScRange& rRange);
    bool InitFromDataBase(const  ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& xRowSet, const Date& rNullDate);

    SCROW  GetRowCount() const;
    SCROW  GetItemDataId( sal_uInt16 nDim, SCROW nRow, bool bRepeatIfEmpty ) const;
    String GetDimensionName( sal_uInt16 nColumn ) const;
    bool IsEmptyMember( SCROW nRow, sal_uInt16 nColumn ) const;
    bool IsRowEmpty( SCROW nRow ) const;
    bool IsValid() const;
    bool ValidQuery(SCROW nRow, const ScQueryParam& rQueryParam) const;

    ScDocument* GetDoc() const;//ms-cache-core
    long GetColumnCount() const;

    const ScDPItemData* GetItemDataById( long nDim, SCROW nId ) const;

    bool operator== ( const ScDPCache& r ) const;

    ScDPCache(ScDocument* pDoc);
    ~ScDPCache();

private:
    void AddLabel( ScDPItemData* pData);
    bool AddData(long nDim, ScDPItemData* pData);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
