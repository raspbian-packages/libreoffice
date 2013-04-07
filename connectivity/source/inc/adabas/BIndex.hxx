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

#ifndef _CONNECTIVITY_ADABAS_INDEX_HXX_
#define _CONNECTIVITY_ADABAS_INDEX_HXX_

#include "connectivity/sdbcx/VIndex.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>

namespace connectivity
{
    namespace adabas
    {
        class OAdabasTable;
        class OAdabasIndex : public sdbcx::OIndex
        {
            OAdabasTable*   m_pTable;
        public:
            virtual void refreshColumns();
        public:
            OAdabasIndex(OAdabasTable* _pTable);
            OAdabasIndex(   OAdabasTable* _pTable,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Catalog,
                    sal_Bool _isUnique,
                    sal_Bool _isPrimaryKeyIndex,
                    sal_Bool _isClustered
                );
            OAdabasTable* getTable() const { return m_pTable; }
        };
    }
}
#endif // _CONNECTIVITY_ADABAS_INDEX_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
