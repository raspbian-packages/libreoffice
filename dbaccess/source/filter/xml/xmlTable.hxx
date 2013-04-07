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
#ifndef DBA_XMLTABLE_HXX
#define DBA_XMLTABLE_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

namespace dbaxml
{
    class ODBFilter;
    class OXMLTable : public SvXMLImportContext
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xParentContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xTable;
        ::rtl::OUString m_sFilterStatement;
        ::rtl::OUString m_sOrderStatement;
        ::rtl::OUString m_sName;
        ::rtl::OUString m_sSchema;
        ::rtl::OUString m_sCatalog;
        ::rtl::OUString m_sStyleName;
        ::rtl::OUString m_sServiceName;
        sal_Bool        m_bApplyFilter;
        sal_Bool        m_bApplyOrder;


        ODBFilter& GetOwnImport();

        void fillAttributes(    sal_uInt16 nPrfx
                                ,const ::rtl::OUString& _sLocalName
                                ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                                , ::rtl::OUString& _rsCommand
                                ,::rtl::OUString& _rsTableName
                                ,::rtl::OUString& _rsTableSchema
                                ,::rtl::OUString& _rsTableCatalog
                            );

        virtual void setProperties(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _xProp);
    public:

        OXMLTable( ODBFilter& rImport
                    , sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xParentContainer
                    ,const ::rtl::OUString& _sServiceName
                    );
        virtual ~OXMLTable();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
        virtual void EndElement();
    };
// -----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

#endif // DBA_XMLTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
