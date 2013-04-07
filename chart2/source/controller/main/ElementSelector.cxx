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


#include "ElementSelector.hxx"
#include "macros.hxx"
#include "ObjectNameProvider.hxx"
#include "ObjectHierarchy.hxx"
#include "servicenames.hxx"
#include <chartview/ExplicitValueProvider.hxx>
#include "DrawViewWrapper.hxx"
#include "ResId.hxx"
#include "Strings.hrc"

#include <toolkit/helper/vclunohelper.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

namespace chart
{

using namespace com::sun::star;
using namespace com::sun::star::uno;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.ElementSelectorToolbarController" ));
}

//------------------------------------------------------------------------------

SelectorListBox::SelectorListBox( Window* pParent, WinBits nStyle )
    : ListBox( pParent, nStyle )
    , m_bReleaseFocus( true )
{
}

SelectorListBox::~SelectorListBox()
{
}

void lcl_addObjectsToList( const ObjectHierarchy& rHierarchy, const ObjectHierarchy::tOID & rParent, std::vector< ListBoxEntryData >& rEntries
                          , const sal_Int32 nHierarchyDepth, const Reference< chart2::XChartDocument >& xChartDoc )
{
    ObjectHierarchy::tChildContainer aChildren( rHierarchy.getChildren(rParent) );
    ObjectHierarchy::tChildContainer::const_iterator aIt( aChildren.begin());
    while( aIt != aChildren.end() )
    {
        ObjectHierarchy::tOID aOID = *aIt;
        ::rtl::OUString aCID = aOID.getObjectCID();
        ListBoxEntryData aEntry;
        aEntry.OID = aOID;
        aEntry.UIName += ObjectNameProvider::getNameForCID( aCID, xChartDoc );
        aEntry.nHierarchyDepth = nHierarchyDepth;
        rEntries.push_back(aEntry);
        lcl_addObjectsToList( rHierarchy, aOID, rEntries, nHierarchyDepth+1, xChartDoc );
        ++aIt;
    }
}

void SelectorListBox::SetChartController( const Reference< frame::XController >& xChartController )
{
    m_xChartController = xChartController;
}

void SelectorListBox::UpdateChartElementsListAndSelection()
{
    Clear();
    m_aEntries.clear();

    Reference< frame::XController > xChartController( m_xChartController );
    if( xChartController.is() )
    {
        Reference< view::XSelectionSupplier > xSelectionSupplier( xChartController, uno::UNO_QUERY);
        ObjectHierarchy::tOID aSelectedOID;
        rtl::OUString aSelectedCID;
        if( xSelectionSupplier.is() )
        {
            aSelectedOID = ObjectIdentifier( xSelectionSupplier->getSelection() );
            aSelectedCID = aSelectedOID.getObjectCID();
        }

        Reference< chart2::XChartDocument > xChartDoc( xChartController->getModel(), uno::UNO_QUERY );
        ObjectType eType( aSelectedOID.getObjectType() );
        bool bAddSelectionToList = false;
        if ( eType == OBJECTTYPE_DATA_POINT || eType == OBJECTTYPE_DATA_LABEL || eType == OBJECTTYPE_SHAPE )
            bAddSelectionToList = true;

        Reference< uno::XInterface > xChartView;
        Reference< lang::XMultiServiceFactory > xFact( xChartController->getModel(), uno::UNO_QUERY );
        if( xFact.is() )
            xChartView = xFact->createInstance( CHART_VIEW_SERVICE_NAME );
        ExplicitValueProvider* pExplicitValueProvider = 0;//ExplicitValueProvider::getExplicitValueProvider(xChartView); dies erzeugt alle sichtbaren datenpinkte, das ist zu viel
        ObjectHierarchy aHierarchy( xChartDoc, pExplicitValueProvider, true /*bFlattenDiagram*/, true /*bOrderingForElementSelector*/ );
        lcl_addObjectsToList( aHierarchy, aHierarchy.getRootNodeOID(), m_aEntries, 0, xChartDoc );

        std::vector< ListBoxEntryData >::iterator aIt( m_aEntries.begin() );
        if( bAddSelectionToList )
        {
            if ( aSelectedOID.isAutoGeneratedObject() )
            {
                rtl::OUString aSeriesCID = ObjectIdentifier::createClassifiedIdentifierForParticle( ObjectIdentifier::getSeriesParticleFromCID( aSelectedCID ) );
                for( aIt = m_aEntries.begin(); aIt != m_aEntries.end(); ++aIt )
                {
                    if( aIt->OID.getObjectCID().match( aSeriesCID ) )
                    {
                        ListBoxEntryData aEntry;
                        aEntry.UIName = ObjectNameProvider::getNameForCID( aSelectedCID, xChartDoc );
                        aEntry.OID = aSelectedOID;
                        ++aIt;
                        if( aIt != m_aEntries.end() )
                            m_aEntries.insert(aIt, aEntry);
                        else
                            m_aEntries.push_back( aEntry );
                        break;
                    }
                }
            }
            else if ( aSelectedOID.isAdditionalShape() )
            {
                ListBoxEntryData aEntry;
                SdrObject* pSelectedObj = DrawViewWrapper::getSdrObject( aSelectedOID.getAdditionalShape() );
                ::rtl::OUString aName( pSelectedObj ? pSelectedObj->GetName() : String() );
                aEntry.UIName = ( aName.getLength() > 0 ? aName : ::rtl::OUString( String( SchResId( STR_OBJECT_SHAPE ) ) ) );
                aEntry.OID = aSelectedOID;
                m_aEntries.push_back( aEntry );
            }
        }

        sal_uInt16 nEntryPosToSelect = 0; bool bSelectionFound = false;
        aIt = m_aEntries.begin();
        for( sal_uInt16 nN=0; aIt != m_aEntries.end(); ++aIt, ++nN )
        {
            InsertEntry( aIt->UIName );
            if ( !bSelectionFound && aSelectedOID == aIt->OID )
            {
                nEntryPosToSelect = nN;
                bSelectionFound = true;
            }
        }

        if( bSelectionFound )
            SelectEntryPos(nEntryPosToSelect);

        sal_uInt16 nEntryCount = GetEntryCount();
        if( nEntryCount > 100 )
            nEntryCount = 100;
        SetDropDownLineCount( nEntryCount );
    }
    SaveValue();//remind current selection pos
}

void SelectorListBox::ReleaseFocus_Impl()
{
    if ( !m_bReleaseFocus )
    {
        m_bReleaseFocus = true;
        return;
    }

    Reference< frame::XController > xController( m_xChartController );
    Reference< frame::XFrame > xFrame( xController->getFrame() );
    if ( xFrame.is() && xFrame->getContainerWindow().is() )
        xFrame->getContainerWindow()->setFocus();
}

void SelectorListBox::Select()
{
    ListBox::Select();

    if ( !IsTravelSelect() )
    {
        sal_uInt16 nPos = GetSelectEntryPos();
        if( nPos < m_aEntries.size() )
        {
            ObjectHierarchy::tOID aOID = m_aEntries[nPos].OID;
            Reference< view::XSelectionSupplier > xSelectionSupplier( m_xChartController.get(), uno::UNO_QUERY );
            if( xSelectionSupplier.is() )
                xSelectionSupplier->select( aOID.getAny() );
        }
        ReleaseFocus_Impl();
    }
}

long SelectorListBox::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        sal_uInt16 nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

        switch ( nCode )
        {
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( KEY_TAB == nCode )
                    m_bReleaseFocus = false;
                else
                    nHandled = 1;
                Select();
                break;
            }

            case KEY_ESCAPE:
                SelectEntryPos( GetSavedValue() ); //restore saved selection
                ReleaseFocus_Impl();
                break;
        }
    }
    else if ( EVENT_LOSEFOCUS == rNEvt.GetType() )
    {
        if ( !HasFocus() )
            SelectEntryPos( GetSavedValue() );
    }

    return nHandled ? nHandled : ListBox::Notify( rNEvt );
}

Reference< ::com::sun::star::accessibility::XAccessible > SelectorListBox::CreateAccessible()
{
    UpdateChartElementsListAndSelection();
    return ListBox::CreateAccessible();
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ElementSelectorToolbarController, lcl_aServiceName );

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ElementSelectorToolbarController::getSupportedServiceNames_Static()
{
    Sequence< ::rtl::OUString > aServices(1);
    aServices[ 0 ] = C2U( "com.sun.star.frame.ToolbarController" );
    return aServices;
}
// -----------------------------------------------------------------------------
ElementSelectorToolbarController::ElementSelectorToolbarController( const uno::Reference< uno::XComponentContext > & xContext )
    : m_xCC( xContext )
{
}
// -----------------------------------------------------------------------------
ElementSelectorToolbarController::~ElementSelectorToolbarController()
{
}
// -----------------------------------------------------------------------------
// XInterface
Any SAL_CALL ElementSelectorToolbarController::queryInterface( const Type& _rType ) throw (RuntimeException)
{
    Any aReturn = ToolboxController::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ElementSelectorToolbarController_BASE::queryInterface(_rType);
    return aReturn;
}
// -----------------------------------------------------------------------------
void SAL_CALL ElementSelectorToolbarController::acquire() throw ()
{
    ToolboxController::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL ElementSelectorToolbarController::release() throw ()
{
    ToolboxController::release();
}
// -----------------------------------------------------------------------------
void SAL_CALL ElementSelectorToolbarController::initialize( const Sequence< Any >& rArguments ) throw (Exception, RuntimeException)
{
    ToolboxController::initialize(rArguments);
}
// -----------------------------------------------------------------------------
void SAL_CALL ElementSelectorToolbarController::statusChanged( const frame::FeatureStateEvent& rEvent ) throw ( RuntimeException )
{
    if( m_apSelectorListBox.get() )
    {
        SolarMutexGuard aSolarMutexGuard;
        if( rEvent.FeatureURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ChartElementSelector" ) ) )
        {
            Reference< frame::XController > xChartController;
            rEvent.State >>= xChartController;
            m_apSelectorListBox->SetChartController( xChartController );
            m_apSelectorListBox->UpdateChartElementsListAndSelection();
        }
    }
}
// -----------------------------------------------------------------------------
uno::Reference< awt::XWindow > SAL_CALL ElementSelectorToolbarController::createItemWindow( const uno::Reference< awt::XWindow >& xParent )
        throw (uno::RuntimeException)
{
    uno::Reference< awt::XWindow > xItemWindow;
    if( !m_apSelectorListBox.get() )
    {
        Window* pParent = VCLUnoHelper::GetWindow( xParent );
        if( pParent )
        {
            m_apSelectorListBox = ::std::auto_ptr< SelectorListBox >( new SelectorListBox( pParent, WB_DROPDOWN|WB_AUTOHSCROLL|WB_BORDER ) );
            ::Size aLogicalSize( 95, 160 );
            ::Size aPixelSize = m_apSelectorListBox->LogicToPixel( aLogicalSize, MAP_APPFONT );
            m_apSelectorListBox->SetSizePixel( aPixelSize );
            m_apSelectorListBox->SetDropDownLineCount( 5 );
        }
    }
    if( m_apSelectorListBox.get() )
        xItemWindow = VCLUnoHelper::GetInterface( m_apSelectorListBox.get() );
    return xItemWindow;
}

//..........................................................................
} // chart2
//..........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
