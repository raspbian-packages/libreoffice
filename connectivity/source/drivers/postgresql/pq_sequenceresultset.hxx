/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    Version: MPL 1.1 / GPLv3+ / LGPLv2.1+
 *
 *    The contents of this file are subject to the Mozilla Public License Version
 *    1.1 (the "License"); you may not use this file except in compliance with
 *    the License or as specified alternatively below. You may obtain a copy of
 *    the License at http://www.mozilla.org/MPL/
 *
 *    Software distributed under the License is distributed on an "AS IS" basis,
 *    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *    for the specific language governing rights and limitations under the
 *    License.
 *
 *    Major Contributor(s):
 *    [ Copyright (C) 2011 Lionel Elie Mamane <lionel@mamane.lu> ]
 *
 *    All Rights Reserved.
 *
 *    For minor contributions see the git repository.
 *
 *    Alternatively, the contents of this file may be used under the terms of
 *    either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 *    the GNU Lesser General Public License Version 2.1 or later (the "LGPLv2.1+"),
 *    in which case the provisions of the GPLv3+ or the LGPLv2.1+ are applicable
 *    instead of those above.
 *
 ************************************************************************/

#ifndef _PG_SEQUENCERESULTSET_HXX_
#define _PG_SEQUENCERESULTSET_HXX_

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/component.hxx>

#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include "pq_connection.hxx"
#include "pq_baseresultset.hxx"
#include "pq_statics.hxx"

namespace pq_sdbc_driver
{

// static const sal_Int32 RESULTSET_CURSOR_NAME = 0;
// static const sal_Int32 RESULTSET_ESCAPE_PROCESSING = 1;
// static const sal_Int32 RESULTSET_FETCH_DIRECTION = 2;
// static const sal_Int32 RESULTSET_FETCH_SIZE = 3;
// static const sal_Int32 RESULTSET_RESULT_SET_CONCURRENCY = 4;
// static const sal_Int32 RESULTSET_RESULT_SET_TYPE = 5;

//#define RESULTSET_SIZE 6
class ResultSetGuard;

class SequenceResultSet : public BaseResultSet
{
protected:
    ::com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > > m_data;

    ::com::sun::star::uno::Sequence< ::rtl::OUString > m_columnNames;
    ::com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSetMetaData > m_meta;

protected:
    /** mutex should be locked before called
     */
    virtual void checkClosed()
        throw ( com::sun::star::sdbc::SQLException, com::sun::star::uno::RuntimeException );

    /** unchecked, acquire mutex before calling
     */
    virtual ::com::sun::star::uno::Any getValue( sal_Int32 columnIndex );

public:
    SequenceResultSet(
        const ::rtl::Reference< RefCountedMutex > & mutex,
        const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > &owner,
        const com::sun::star::uno::Sequence< rtl::OUString > &colNames,
        const com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< com::sun::star::uno::Any > > &data,
        const com::sun::star::uno::Reference< com::sun::star::script::XTypeConverter > &tc,
        const ColumnMetaDataVector *pVec = 0);
    ~SequenceResultSet();

public: // XCloseable
    virtual void SAL_CALL close(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

public: // XResultSetMetaDataSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

public: // XColumnLocate
    virtual sal_Int32 SAL_CALL findColumn( const ::rtl::OUString& columnName )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
};

}
#endif
