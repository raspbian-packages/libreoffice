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
#ifndef CHART_COLUMNLINECHARTTYPETEMPLATE_HXX
#define CHART_COLUMNLINECHARTTYPETEMPLATE_HXX

#include "ChartTypeTemplate.hxx"
#include "StackMode.hxx"

#include "OPropertySet.hxx"
#include "MutexContainer.hxx"
#include <comphelper/uno3.hxx>

namespace chart
{

class ColumnLineChartTypeTemplate :
        public MutexContainer,
        public ChartTypeTemplate,
        public ::property::OPropertySet
{
public:
    explicit ColumnLineChartTypeTemplate(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext,
        const ::rtl::OUString & rServiceName,
        StackMode eStackMode,
        sal_Int32 nNumberOfLines );
    virtual ~ColumnLineChartTypeTemplate();

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XChartTypeTemplate ____
    virtual sal_Bool SAL_CALL matchesTemplate(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >& xDiagram,
        sal_Bool bAdaptProperties )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType > SAL_CALL
        getChartTypeForNewSeries( const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartType > >& aFormerlyUsedChartTypes )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL applyStyle(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& xSeries,
        ::sal_Int32 nChartTypeGroupIndex,
        ::sal_Int32 nSeriesIndex,
        ::sal_Int32 nSeriesCount )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataInterpreter > SAL_CALL getDataInterpreter()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ ChartTypeTemplate ____
    virtual void createChartTypes(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Sequence<
                    ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XDataSeries > > > & aSeriesSeq,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XCoordinateSystem > > & rCoordSys,
            const ::com::sun::star::uno::Sequence<
                  ::com::sun::star::uno::Reference<
                      ::com::sun::star::chart2::XChartType > > & aOldChartTypesSeq
            );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >
                getChartTypeForIndex( sal_Int32 nChartTypeIndex );

    virtual StackMode getStackMode( sal_Int32 nChartTypeIndex ) const;

private:
    StackMode          m_eStackMode;
};

} //  namespace chart

// CHART_COLUMNLINECHARTTYPETEMPLATE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
