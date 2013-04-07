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

#include "PieChartType.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "PolarCoordinateSystem.hxx"
#include "AxisHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "ContainerHelper.hxx"
#include "AxisIndexDefines.hxx"
#include "AxisHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/AxisType.hpp>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

enum
{
    PROP_PIECHARTTYPE_USE_RINGS
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "UseRings" ),
                  PROP_PIECHARTTYPE_USE_RINGS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticPieChartTypeDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        lcl_AddDefaultsToMap( aStaticDefaults );
        return &aStaticDefaults;
    }
private:
    void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_PIECHARTTYPE_USE_RINGS, false );
    }
};

struct StaticPieChartTypeDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticPieChartTypeDefaults_Initializer >
{
};

struct StaticPieChartTypeInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

};

struct StaticPieChartTypeInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticPieChartTypeInfoHelper_Initializer >
{
};

struct StaticPieChartTypeInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticPieChartTypeInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticPieChartTypeInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticPieChartTypeInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

PieChartType::PieChartType(
    const uno::Reference< uno::XComponentContext > & xContext,
    sal_Bool bUseRings  /* = sal_False */) :
        ChartType( xContext )
{
    if( bUseRings )
        setFastPropertyValue_NoBroadcast( PROP_PIECHARTTYPE_USE_RINGS, uno::makeAny( bUseRings ));
}

PieChartType::PieChartType( const PieChartType & rOther ) :
        ChartType( rOther )
{
}

PieChartType::~PieChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL PieChartType::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new PieChartType( *this ));
}

// ____ XChartType ____
::rtl::OUString SAL_CALL PieChartType::getChartType()
    throw (uno::RuntimeException)
{
    return CHART2_SERVICE_NAME_CHARTTYPE_PIE;
}

Reference< chart2::XCoordinateSystem > SAL_CALL
    PieChartType::createCoordinateSystem( ::sal_Int32 DimensionCount )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    Reference< chart2::XCoordinateSystem > xResult(
        new PolarCoordinateSystem(
            GetComponentContext(), DimensionCount, /* bSwapXAndYAxis */ sal_False ));

    for( sal_Int32 i=0; i<DimensionCount; ++i )
    {
        Reference< chart2::XAxis > xAxis( xResult->getAxisByDimension( i, MAIN_AXIS_INDEX ) );
        if( !xAxis.is() )
        {
            OSL_FAIL("a created coordinate system should have an axis for each dimension");
            continue;
        }

        //hhhh todo make axis invisible

        chart2::ScaleData aScaleData = xAxis->getScaleData();
        aScaleData.Scaling = AxisHelper::createLinearScaling();
        aScaleData.AxisType = chart2::AxisType::REALNUMBER;

        if( i == 0 )
            aScaleData.Orientation = chart2::AxisOrientation_REVERSE;
        else
            aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;

        //remove explicit scalings from all axes
        AxisHelper::removeExplicitScaling( aScaleData );

        xAxis->setScaleData( aScaleData );
    }

    return xResult;
}

// ____ OPropertySet ____
uno::Any PieChartType::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticPieChartTypeDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL PieChartType::getInfoHelper()
{
    return *StaticPieChartTypeInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL PieChartType::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticPieChartTypeInfo::get();
}

uno::Sequence< ::rtl::OUString > PieChartType::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 3 );
    aServices[ 0 ] = CHART2_SERVICE_NAME_CHARTTYPE_PIE;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.ChartType" );
    aServices[ 2 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( PieChartType,
                             C2U( "com.sun.star.comp.chart.PieChartType" ));

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
