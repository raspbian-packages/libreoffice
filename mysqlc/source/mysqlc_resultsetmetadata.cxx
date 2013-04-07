/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2008 by Sun Microsystems, Inc.
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
************************************************************************/

#include "mysqlc_resultsetmetadata.hxx"
#include "mysqlc_general.hxx"
#include "cppconn/exception.h"

#include <rtl/ustrbuf.hxx>

using namespace connectivity::mysqlc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using ::rtl::OUString;

// -------------------------------------------------------------------------
OResultSetMetaData::~OResultSetMetaData()
{
}
/* }}} */


/* {{{ OResultSetMetaData::getColumnDisplaySize() -I- */
sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::getColumnDisplaySize");

    try {
        meta->getColumnDisplaySize(column);
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getColumnDisplaySize", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::getColumnType() -I- */
sal_Int32 SAL_CALL OResultSetMetaData::getColumnType(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::getColumnType");
    checkColumnIndex(column);

    try {
        return mysqlc_sdbc_driver::mysqlToOOOType(meta->getColumnType(column));
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}
/* }}} */

/*
  XXX: This method doesn't throw exceptions at all.
  Should it declare that it throws ?? What if throw() is removed?
  Does it change the API, the open-close principle?
*/
/* {{{ OResultSetMetaData::getColumnCount() -I- */
sal_Int32 SAL_CALL OResultSetMetaData::getColumnCount()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::getColumnCount");
    try {
        return meta->getColumnCount();
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::isCaseSensitive() -I- */
sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::isCaseSensitive");
    checkColumnIndex(column);

    try {
        return meta->isCaseSensitive(column);
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::getSchemaName() -I- */
OUString SAL_CALL OResultSetMetaData::getSchemaName(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::getSchemaName");
    checkColumnIndex(column);

    try {
        return convert(meta->getSchemaName(column));
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return OUString(); // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::getColumnName() -I- */
OUString SAL_CALL OResultSetMetaData::getColumnName(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::getColumnName");
    checkColumnIndex(column);

    try {
        return convert( meta->getColumnName( column ) );
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return OUString(); // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::getTableName() -I- */
OUString SAL_CALL OResultSetMetaData::getTableName(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::getTableName");
    checkColumnIndex(column);

    try {
        return convert(meta->getTableName(column));
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return OUString(); // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::getCatalogName() -I- */
OUString SAL_CALL OResultSetMetaData::getCatalogName(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::getCatalogName");
    checkColumnIndex(column);

    try {
        return convert(meta->getCatalogName(column));
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return OUString(); // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::getColumnTypeName() -I- */
OUString SAL_CALL OResultSetMetaData::getColumnTypeName(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::getColumnTypeName");
    checkColumnIndex(column);

    try {
        return convert(meta->getColumnTypeName(column));
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return OUString(); // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::getColumnLabel() -I- */
OUString SAL_CALL OResultSetMetaData::getColumnLabel(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::getColumnLabel");
    checkColumnIndex(column);

    try {
        return convert(meta->getColumnLabel(column));
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return OUString(); // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::getColumnServiceName() -I- */
OUString SAL_CALL OResultSetMetaData::getColumnServiceName(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::getColumnServiceName");
    checkColumnIndex(column);

    OUString aRet = OUString();
    return aRet;
}
/* }}} */


/* {{{ OResultSetMetaData::isCurrency() -I- */
sal_Bool SAL_CALL OResultSetMetaData::isCurrency(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::isCurrency");
    checkColumnIndex(column);

    try {
        return meta->isCurrency(column)? sal_True:sal_False;
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::isAutoIncrement() -I- */
sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::isAutoIncrement");
    checkColumnIndex(column);

    try {
        return meta->isAutoIncrement(column)? sal_True:sal_False;
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::isSigned() -I- */
sal_Bool SAL_CALL OResultSetMetaData::isSigned(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::isSigned");
    checkColumnIndex(column);

    try {
        return meta->isSigned(column)? sal_True:sal_False;
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::getPrecision() -I- */
sal_Int32 SAL_CALL OResultSetMetaData::getPrecision(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::getPrecision");
    checkColumnIndex(column);

    try {
        return meta->getPrecision(column);
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getPrecision", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::getScale() -I- */
sal_Int32 SAL_CALL OResultSetMetaData::getScale(sal_Int32 column)
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::getScale");
    checkColumnIndex(column);
    try {
        return meta->getScale(column);
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getScale", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::isNullable() -I- */
sal_Int32 SAL_CALL OResultSetMetaData::isNullable(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::isNullable");
    checkColumnIndex(column);

    try {
        return meta->isNullable(column)? sal_True:sal_False;
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::isSearchable() -I- */
sal_Bool SAL_CALL OResultSetMetaData::isSearchable(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::isSearchable");
    checkColumnIndex(column);

    try {
        return meta->isSearchable(column)? sal_True:sal_False;
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::isReadOnly() -I- */
sal_Bool SAL_CALL OResultSetMetaData::isReadOnly(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::isReadOnly");
    checkColumnIndex(column);

    try {
        return meta->isReadOnly(column)? sal_True:sal_False;
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::isDefinitelyWritable() -I- */
sal_Bool SAL_CALL OResultSetMetaData::isDefinitelyWritable(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::isDefinitelyWritable");
    checkColumnIndex(column);

    try {
        return meta->isDefinitelyWritable(column)? sal_True:sal_False;
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::isWritable() -I- */
sal_Bool SAL_CALL OResultSetMetaData::isWritable(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::isWritable");
    checkColumnIndex(column);

    try {
        return meta->isWritable(column)? sal_True:sal_False;
    } catch (sql::MethodNotImplementedException) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}
/* }}} */


/* {{{ OResultSetMetaData::checkColumnIndex() -I- */
void OResultSetMetaData::checkColumnIndex(sal_Int32 columnIndex)
    throw (SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::checkColumnIndex");
    if (columnIndex < 1 || columnIndex > (sal_Int32) meta->getColumnCount()) {

        ::rtl::OUStringBuffer buf;
        buf.appendAscii( "Column index out of range (expected 1 to " );
        buf.append( sal_Int32( meta->getColumnCount() ) );
        buf.appendAscii( ", got " );
        buf.append( sal_Int32( columnIndex ) );
        buf.append( sal_Unicode( '.' ) );
        throw SQLException( buf.makeStringAndClear(), *this, OUString(), 1, Any() );
    }
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
