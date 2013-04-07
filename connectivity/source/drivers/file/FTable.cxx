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


#include <stdio.h>
#include "file/FTable.hxx"
#include "file/FColumns.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/debug.hxx>
#include <rtl/logfile.hxx>

using namespace connectivity;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

DBG_NAME( file_OFileTable )
OFileTable::OFileTable(sdbcx::OCollection* _pTables,OConnection* _pConnection)
: OTable_TYPEDEF(_pTables,_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers())
                ,m_pConnection(_pConnection)
                ,m_pFileStream(NULL)
                ,m_nFilePos(0)
                ,m_pBuffer(NULL)
                ,m_nBufferSize(0)
                ,m_bWriteable(sal_False)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::OFileTable" );
    DBG_CTOR( file_OFileTable, NULL );
    construct();
    TStringVector aVector;
    //  m_pColumns  = new OColumns(this,m_aMutex,aVector);
    m_aColumns = new OSQLColumns();
}
// -------------------------------------------------------------------------
OFileTable::OFileTable( sdbcx::OCollection* _pTables,OConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description ,
                    const ::rtl::OUString& _SchemaName,
                    const ::rtl::OUString& _CatalogName
                ) : OTable_TYPEDEF(_pTables,_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers(),
                                  _Name,
                                  _Type,
                                  _Description,
                                  _SchemaName,
                                  _CatalogName)
                ,m_pConnection(_pConnection)
                ,m_pFileStream(NULL)
                ,m_nFilePos(0)
                ,m_pBuffer(NULL)
                ,m_nBufferSize(0)
                ,m_bWriteable(sal_False)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::OFileTable" );
    DBG_CTOR( file_OFileTable, NULL );
    m_aColumns = new OSQLColumns();
    construct();
    //  refreshColumns();
}
// -------------------------------------------------------------------------
OFileTable::~OFileTable( )
{
    DBG_DTOR( file_OFileTable, NULL );
}
// -------------------------------------------------------------------------
void OFileTable::refreshColumns()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::refreshColumns" );
    TStringVector aVector;
        Reference< XResultSet > xResult = m_pConnection->getMetaData()->getColumns(Any(),
                                                    m_SchemaName,m_Name,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")));

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
            aVector.push_back(xRow->getString(4));
    }

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = new OColumns(this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void OFileTable::refreshKeys()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::refreshKeys" );
}
// -------------------------------------------------------------------------
void OFileTable::refreshIndexes()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::refreshIndexes" );
}
// -------------------------------------------------------------------------
Any SAL_CALL OFileTable::queryInterface( const Type & rType ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::queryInterface" );
    if( rType == ::getCppuType((const Reference<XKeysSupplier>*)0) ||
        rType == ::getCppuType((const Reference<XRename>*)0) ||
        rType == ::getCppuType((const Reference<XAlterTable>*)0) ||
        rType == ::getCppuType((const Reference<XIndexesSupplier>*)0) ||
        rType == ::getCppuType((const Reference<XDataDescriptorFactory>*)0))
        return Any();

    return OTable_TYPEDEF::queryInterface(rType);
}
// -------------------------------------------------------------------------
void SAL_CALL OFileTable::disposing(void)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::disposing" );
    OTable::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    FileClose();
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OFileTable::getUnoTunnelImplementationId()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::getUnoTunnelImplementationId" );
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OFileTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::getSomething" );
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : OTable_TYPEDEF::getSomething(rId);
}
// -----------------------------------------------------------------------------
void OFileTable::FileClose()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::FileClose" );
    ::osl::MutexGuard aGuard(m_aMutex);

    if (m_pFileStream && m_pFileStream->IsWritable())
        m_pFileStream->Flush();

    delete m_pFileStream;
    m_pFileStream = NULL;

    if (m_pBuffer)
    {
        delete[] m_pBuffer;
        m_pBuffer = NULL;
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OFileTable::acquire() throw()
{
    OTable_TYPEDEF::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFileTable::release() throw()
{
    OTable_TYPEDEF::release();
}
// -----------------------------------------------------------------------------
sal_Bool OFileTable::InsertRow(OValueRefVector& /*rRow*/, sal_Bool /*bFlush*/,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& /*_xCols*/)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::InsertRow" );
    return sal_False;
}
// -----------------------------------------------------------------------------
sal_Bool OFileTable::DeleteRow(const OSQLColumns& /*_rCols*/)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::DeleteRow" );
    return sal_False;
}
// -----------------------------------------------------------------------------
sal_Bool OFileTable::UpdateRow(OValueRefVector& /*rRow*/, OValueRefRow& /*pOrgRow*/,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& /*_xCols*/)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::UpdateRow" );
    return sal_False;
}
// -----------------------------------------------------------------------------
void OFileTable::addColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& /*descriptor*/)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::addColumn" );
    OSL_FAIL( "OFileTable::addColumn: not implemented!" );
}
// -----------------------------------------------------------------------------
void OFileTable::dropColumn(sal_Int32 /*_nPos*/)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::dropColumn" );
    OSL_FAIL( "OFileTable::addColumn: not implemented!" );
}

// -----------------------------------------------------------------------------
SvStream* OFileTable::createStream_simpleError( const String& _rFileName, StreamMode _eOpenMode)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::createStream_simpleError" );
    utl::UcbLockBytesHandler* p_null_dummy=NULL;
    SvStream* pReturn = ::utl::UcbStreamHelper::CreateStream( _rFileName, _eOpenMode, (_eOpenMode & STREAM_NOCREATE) == STREAM_NOCREATE ,p_null_dummy);
    if (pReturn && (ERRCODE_NONE != pReturn->GetErrorCode()))
    {
        delete pReturn;
        pReturn = NULL;
    }
    return pReturn;
}

// -----------------------------------------------------------------------------
void OFileTable::refreshHeader()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileTable::refreshHeader" );
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
