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
#ifndef _CHART_POLARCOORDINATESYSTEM_HXX
#define _CHART_POLARCOORDINATESYSTEM_HXX

#include "ServiceMacros.hxx"
#include "BaseCoordinateSystem.hxx"

namespace chart
{

class PolarCoordinateSystem : public BaseCoordinateSystem
{
public:
    explicit PolarCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        sal_Int32 nDimensionCount = 2,
        sal_Bool bSwapXAndYAxis = sal_False );
    explicit PolarCoordinateSystem( const PolarCoordinateSystem & rSource );
    virtual ~PolarCoordinateSystem();

    // ____ XCoordinateSystem ____
    virtual ::rtl::OUString SAL_CALL getCoordinateSystemType()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getViewServiceName()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceInfo ____
    APPHELPER_XSERVICEINFO_DECL()
};

class PolarCoordinateSystem2d : public PolarCoordinateSystem
{
public:
    explicit PolarCoordinateSystem2d(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~PolarCoordinateSystem2d();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( PolarCoordinateSystem2d )
    // ____ XServiceInfo ____
    APPHELPER_XSERVICEINFO_DECL()
};

class PolarCoordinateSystem3d : public PolarCoordinateSystem
{
public:
    explicit PolarCoordinateSystem3d(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~PolarCoordinateSystem3d();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( PolarCoordinateSystem3d )
    // ____ XServiceInfo ____
    APPHELPER_XSERVICEINFO_DECL()
};

}  // namespace chart

// _CHART_POLARCOORDINATESYSTEM_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
