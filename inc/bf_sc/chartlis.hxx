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

#ifndef SC_CHARTLIS_HXX
#define SC_CHARTLIS_HXX

#include <bf_svtools/bf_solar.h>


#include <vcl/timer.hxx>

#include <bf_svtools/lstner.hxx>
#include "collect.hxx"
#include "rangelst.hxx"


#include <com/sun/star/chart/XChartData.hpp>
namespace binfilter {
class ScDocument;
class ScChartUnoData;

class ScChartListener : public StrData, public SfxListener
{
private:
    ScRangeListRef  aRangeListRef;
    ScChartUnoData* pUnoData;
    ScDocument*     pDoc;
    BOOL            bUsed;  // fuer ScChartListenerCollection::FreeUnused
    BOOL            bDirty;
    BOOL            bSeriesRangesScheduled;

                    // not implemented
    ScChartListener& operator=( const ScChartListener& );

public:
                    ScChartListener( const String& rName, ScDocument* pDoc,
                                    const ScRange& rRange );
                    ScChartListener( const String& rName, ScDocument* pDoc,
                                    const ScRangeListRef& rRangeListRef );
                    ScChartListener( const ScChartListener& );
    virtual         ~ScChartListener();
    virtual DataObject* Clone() const { return NULL; } // DBG_BF_ASSERT

    void            SetUno( const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& rListener,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartData >& rSource );
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >  GetUnoListener() const;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartData >                     GetUnoSource() const;

    BOOL            IsUno() const   { return (pUnoData != NULL); }

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    void            StartListeningTo();
    void            EndListeningTo();
    void            Update();
    const ScRangeListRef&   GetRangeList() const { return aRangeListRef; }
    void            SetRangeList( const ScRangeListRef& rNew ) { aRangeListRef = rNew; }
    void            SetRangeList( const ScRange& rNew );
    BOOL            IsUsed() const { return bUsed; }
    void            SetUsed( BOOL bFlg ) { bUsed = bFlg; }
    BOOL            IsDirty() const { return bDirty; }
    void            SetDirty( BOOL bFlg ) { bDirty = bFlg; }

    // if chart series ranges are to be updated later on (e.g. DeleteTab, InsertTab)
    void            ScheduleSeriesRanges()      { bSeriesRangesScheduled = TRUE; }

};

class ScChartListenerCollection : public StrCollection
{
private:
    Timer           aTimer;
    ScDocument*     pDoc;

                    DECL_LINK( TimerHdl, Timer* );

                    // not implemented
    ScChartListenerCollection& operator=( const ScChartListenerCollection& );

public:
                    ScChartListenerCollection( ScDocument* pDoc );
                    ScChartListenerCollection( const ScChartListenerCollection& );
    virtual DataObject* Clone() const { return NULL; } // DBG_BF_ASSERT

    virtual         ~ScChartListenerCollection();

    // FreeUnused nur wie in ScDocument::UpdateChartListenerCollection verwenden!
    void            FreeUnused();
    void            FreeUno( const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& rListener,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartData >& rSource );
    void            StartTimer();
    void            UpdateDirtyCharts();
    void            SetDirty();
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
