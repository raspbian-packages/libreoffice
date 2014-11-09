/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_OPTIMISTICSET_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_OPTIMISTICSET_HXX

#include "KeySet.hxx"
#include <cppuhelper/implbase1.hxx>

#include <connectivity/sqlparse.hxx>
#include <connectivity/sqliterator.hxx>

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryAnalyzer.hpp>

namespace dbaccess
{
     // is used when the source supports keys
    class OptimisticSet : public OKeySet
    {
        ::connectivity::OSQLParser                              m_aSqlParser;
        ::connectivity::OSQLParseTreeIterator                   m_aSqlIterator;

        ::std::map<sal_Int32,sal_Int32>                         m_aJoinedColumns;
        ::std::map<sal_Int32,sal_Int32>                         m_aJoinedKeyColumns;

        mutable bool m_bResultSetChanged;

        void impl_convertValue_throw(const ORowSetRow& _rInsertRow,const SelectColumnDescription& i_aMetaData);

        void executeDelete(const ORowSetRow& _rDeleteRow,const OUString& i_sSQL,const OUString& i_sTableName);
        void fillJoinedColumns_throw(const ::std::vector< ::connectivity::TNodePair>& i_aJoinColumns);
        void fillJoinedColumns_throw(const OUString& i_sLeftColumn,const OUString& i_sRightColumn);
    protected:
        virtual void makeNewStatement( ) SAL_OVERRIDE;
        virtual ~OptimisticSet();
    public:
        OptimisticSet(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rContext,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& i_xConnection,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryAnalyzer >& _xComposer,
                      const ORowSetValueVector& _aParameterValueForCache,
                      sal_Int32 i_nMaxRows,
                      sal_Int32& o_nRowCount);

        // late ctor which can throw exceptions
        virtual void construct(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDriverSet,const OUString& i_sRowSetFilter) SAL_OVERRIDE;

        // ::com::sun::star::sdbcx::XDeleteRows
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows ,const connectivity::OSQLTable& _xTable) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
        // ::com::sun::star::sdbc::XResultSetUpdate
        virtual void SAL_CALL updateRow(const ORowSetRow& _rInsertRow,const ORowSetRow& _rOriginalRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
        virtual void SAL_CALL deleteRow(const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
        virtual void SAL_CALL insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

        // CacheSet
        virtual bool isResultSetChanged() const SAL_OVERRIDE;
        virtual void mergeColumnValues(sal_Int32 i_nColumnIndex,ORowSetValueVector::Vector& io_aInsertRow,ORowSetValueVector::Vector& io_aRow,::std::vector<sal_Int32>& o_aChangedColumns) SAL_OVERRIDE;
        virtual bool columnValuesUpdated(ORowSetValueVector::Vector& o_aCachedRow,const ORowSetValueVector::Vector& i_aRow) SAL_OVERRIDE;
        virtual bool updateColumnValues(const ORowSetValueVector::Vector& io_aCachedRow,ORowSetValueVector::Vector& io_aRow,const ::std::vector<sal_Int32>& i_aChangedColumns) SAL_OVERRIDE;
        virtual void fillMissingValues(ORowSetValueVector::Vector& io_aRow) const SAL_OVERRIDE;

        bool isReadOnly() const { return m_aJoinedKeyColumns.empty(); }
        const ::std::map<sal_Int32,sal_Int32>& getJoinedColumns() const { return m_aJoinedColumns; }
        const ::std::map<sal_Int32,sal_Int32>& getJoinedKeyColumns() const { return m_aJoinedKeyColumns; }
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_OPTIMISTICSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
