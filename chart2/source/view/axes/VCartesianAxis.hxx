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
#ifndef _CHART2_VCARTESIANAXIS_HXX
#define _CHART2_VCARTESIANAXIS_HXX

#include "VAxisBase.hxx"
#include <basegfx/vector/b2dvector.hxx>

#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class VCartesianAxis : public VAxisBase
{
    //-------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------
public:
    VCartesianAxis( const AxisProperties& rAxisProperties
           , const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier
           , sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount
           , PlottingPositionHelper* pPosHelper = NULL //takes ownership
           );

    virtual ~VCartesianAxis();

    virtual void createMaximumLabels();
    virtual void createLabels();
    virtual void updatePositions();

    virtual void createShapes();

    virtual sal_Int32 estimateMaximumAutoMainIncrementCount();
    virtual void createAllTickInfos( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos );
    void createAllTickInfosFromComplexCategories( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos, bool bShiftedPosition );

    ::std::auto_ptr< TickIter > createLabelTickIterator( sal_Int32 nTextLevel );
    ::std::auto_ptr< TickIter > createMaximumLabelTickIterator( sal_Int32 nTextLevel );
    sal_Int32 getTextLevelCount() const;

    //-------------------------------------------------------------------------
    virtual TickFactory* createTickFactory();

    //-------------------------------------------------------------------------
    double      getLogicValueWhereMainLineCrossesOtherAxis() const;
    double      getLogicValueWhereLabelLineCrossesOtherAxis() const;
    bool        getLogicValueWhereExtraLineCrossesOtherAxis( double& fCrossesOtherAxis) const;
    void        get2DAxisMainLine( ::basegfx::B2DVector& rStart, ::basegfx::B2DVector& rEnd, double fCrossesOtherAxis );

    //-------------------------------------------------------------------------
    //Layout interface for cartesian axes:

    //the returned value describes the minimum size that is necessary
    //for the text labels in the direction orthogonal to the axis
    //(for an y-axis a width is returned; in case of an x-axis the value describes a height)
    //the return value is measured in screen dimension
    //As an example the MinimumOrthogonalSize of an x-axis equals the
    //Font Height if the label properties allow for labels parallel to the axis.
//    sal_Int32 calculateMinimumOrthogonalSize( /*... parallel...*/ );
    //Minimum->Preferred

    //returns true if the MinimumOrthogonalSize can be calculated
    //with the creation of at most one text shape
    //(this is e.g. true if the parameters allow for labels parallel to the axis.)
//    sal_bool  canQuicklyCalculateMinimumOrthogonalSize();


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

    struct ScreenPosAndLogicPos
    {
        double fLogicX;
        double fLogicY;
        double fLogicZ;

        B2DVector aScreenPos;
    };

protected: //methods
    bool    createTextShapes( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::drawing::XShapes >& xTarget
                     , TickIter& rTickIter
                     , AxisLabelProperties& rAxisLabelProperties
                     , TickFactory_2D* pTickFactory
                     , sal_Int32 nScreenDistanceBetweenTicks );

    void    createTickMarkLineShapes( ::std::vector< TickInfo >& rTickInfos, const TickmarkProperties& rTickmarkProperties, TickFactory_2D& rTickFactory2D, bool bOnlyAtLabels );

    TickFactory_2D* createTickFactory2D();
    void    hideIdenticalScreenValues(  ::std::vector< ::std::vector< TickInfo > >& rTickInfos ) const;

    void    doStaggeringOfLabels( const AxisLabelProperties& rAxisLabelProperties
                            , TickFactory_2D* pTickFactory2D );
    bool    isAutoStaggeringOfLabelsAllowed( const AxisLabelProperties& rAxisLabelProperties
                            , bool bIsHorizontalAxis, bool bIsVerticalAxis );
    bool    isBreakOfLabelsAllowed( const AxisLabelProperties& rAxisLabelProperties, bool bIsHorizontalAxis );

    ::basegfx::B2DVector getScreenPosition( double fLogicX, double fLogicY, double fLogicZ ) const;
    ScreenPosAndLogicPos getScreenPosAndLogicPos( double fLogicX, double fLogicY, double fLogicZ ) const;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
