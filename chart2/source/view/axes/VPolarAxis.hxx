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
#ifndef _CHART2_VPOLARAXIS_HXX
#define _CHART2_VPOLARAXIS_HXX

#include "VAxisBase.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class PolarPlottingPositionHelper;

class VPolarAxis : public VAxisBase
{
public:
    static VPolarAxis* createAxis( const AxisProperties& rAxisProperties
           , const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier
           , sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount );

    void setIncrements( const std::vector< ExplicitIncrementData >& rIncrements );

    virtual sal_Bool isAnythingToDraw();

    virtual ~VPolarAxis();

protected:
    VPolarAxis( const AxisProperties& rAxisProperties
           , const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier
           , sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount );

protected: //member
    PolarPlottingPositionHelper* m_pPosHelper;
    ::std::vector< ExplicitIncrementData >   m_aIncrements;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
