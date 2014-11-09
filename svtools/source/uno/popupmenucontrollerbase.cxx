/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svtools/popupmenucontrollerbase.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

namespace svt
{

struct PopupMenuControllerBaseDispatchInfo
{
    Reference< XDispatch > mxDispatch;
    const URL maURL;
    const Sequence< PropertyValue > maArgs;

    PopupMenuControllerBaseDispatchInfo( const Reference< XDispatch >& xDispatch, const URL& rURL, const Sequence< PropertyValue >& rArgs )
        : mxDispatch( xDispatch ), maURL( rURL ), maArgs( rArgs ) {}
};

PopupMenuControllerBase::PopupMenuControllerBase( const Reference< XComponentContext >& xContext ) :
    ::comphelper::OBaseMutex(),
    PopupMenuControllerBaseType(m_aMutex),
    m_bInitialized( false )
{
    if ( xContext.is() )
        m_xURLTransformer.set( util::URLTransformer::create( xContext ) );
}

PopupMenuControllerBase::~PopupMenuControllerBase()
{
}

// protected function
void PopupMenuControllerBase::throwIfDisposed() throw ( RuntimeException )
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        throw com::sun::star::lang::DisposedException();
}

// protected function
void PopupMenuControllerBase::resetPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu )
{
    if ( rPopupMenu.is() && rPopupMenu->getItemCount() > 0 )
    {
        rPopupMenu->clear();
    }
}

void SAL_CALL PopupMenuControllerBase::disposing()
{
    // Reset our members and set disposed flag
    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xPopupMenu.clear();
}

// XServiceInfo
sal_Bool SAL_CALL PopupMenuControllerBase::supportsService( const OUString& ServiceName ) throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

// XEventListener
void SAL_CALL PopupMenuControllerBase::disposing( const EventObject& ) throw ( RuntimeException, std::exception )
{
    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xPopupMenu.clear();
}

// XMenuListener
void SAL_CALL PopupMenuControllerBase::itemHighlighted( const awt::MenuEvent& ) throw (RuntimeException, std::exception)
{
}

void PopupMenuControllerBase::impl_select(const Reference< XDispatch >& _xDispatch,const URL& aURL)
{
    Sequence<PropertyValue>      aArgs;
    OSL_ENSURE(_xDispatch.is(),"PopupMenuControllerBase::impl_select: No dispatch");
    if ( _xDispatch.is() )
        _xDispatch->dispatch( aURL, aArgs );
}

void SAL_CALL PopupMenuControllerBase::itemSelected( const awt::MenuEvent& rEvent ) throw (RuntimeException, std::exception)
{
    throwIfDisposed();

    osl::MutexGuard aLock( m_aMutex );

    if( m_xPopupMenu.is() )
    {
        Sequence<PropertyValue> aArgs;
        dispatchCommand( m_xPopupMenu->getCommand( rEvent.MenuId ), aArgs );
    }
}

void PopupMenuControllerBase::dispatchCommand( const OUString& sCommandURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs )
{
    osl::MutexGuard aLock( m_aMutex );

    throwIfDisposed();

    try
    {
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY_THROW );
        URL aURL;
        aURL.Complete = sCommandURL;
        m_xURLTransformer->parseStrict( aURL );

        Reference< XDispatch > xDispatch( xDispatchProvider->queryDispatch( aURL, OUString(), 0 ), UNO_QUERY_THROW );

        Application::PostUserEvent( STATIC_LINK(0, PopupMenuControllerBase, ExecuteHdl_Impl), new PopupMenuControllerBaseDispatchInfo( xDispatch, aURL, rArgs ) );

    }
    catch( Exception& )
    {
    }

}

IMPL_STATIC_LINK_NOINSTANCE( PopupMenuControllerBase, ExecuteHdl_Impl, PopupMenuControllerBaseDispatchInfo*, pDispatchInfo )
{
    pDispatchInfo->mxDispatch->dispatch( pDispatchInfo->maURL, pDispatchInfo->maArgs );
    delete pDispatchInfo;
    return 0;
}

void SAL_CALL PopupMenuControllerBase::itemActivated( const awt::MenuEvent& ) throw (RuntimeException, std::exception)
{
}

void SAL_CALL PopupMenuControllerBase::itemDeactivated( const awt::MenuEvent& ) throw (RuntimeException, std::exception)
{
}

void SAL_CALL PopupMenuControllerBase::updatePopupMenu() throw ( ::com::sun::star::uno::RuntimeException, std::exception )
{
    osl::ClearableMutexGuard aLock( m_aMutex );
    throwIfDisposed();
    aLock.clear();

    updateCommand( m_aCommandURL );
}

void SAL_CALL PopupMenuControllerBase::updateCommand( const OUString& rCommandURL )
{
    osl::ClearableMutexGuard aLock( m_aMutex );
    Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    Reference< XDispatch > xDispatch( m_xDispatch );
    URL aTargetURL;
    aTargetURL.Complete = rCommandURL;
    m_xURLTransformer->parseStrict( aTargetURL );
    aLock.clear();

    // Add/remove status listener to get a status update once
    if ( xDispatch.is() )
    {
        xDispatch->addStatusListener( xStatusListener, aTargetURL );
        xDispatch->removeStatusListener( xStatusListener, aTargetURL );
    }
}


// XDispatchProvider
Reference< XDispatch > SAL_CALL
PopupMenuControllerBase::queryDispatch(
    const URL& /*aURL*/,
    const OUString& /*sTarget*/,
    sal_Int32 /*nFlags*/ )
throw( RuntimeException, std::exception )
{
    // must be implemented by subclass
    osl::MutexGuard aLock( m_aMutex );
    throwIfDisposed();

    return Reference< XDispatch >();
}

Sequence< Reference< XDispatch > > SAL_CALL PopupMenuControllerBase::queryDispatches( const Sequence< DispatchDescriptor >& lDescriptor ) throw( RuntimeException, std::exception )
{
    // Create return list - which must have same size then the given descriptor
    // It's not allowed to pack it!
    osl::ClearableMutexGuard aLock( m_aMutex );
    throwIfDisposed();
    aLock.clear();

    sal_Int32                                                          nCount = lDescriptor.getLength();
    uno::Sequence< uno::Reference< frame::XDispatch > > lDispatcher( nCount );

    // Step over all descriptors and try to get any dispatcher for it.
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = queryDispatch( lDescriptor[i].FeatureURL  ,
                                        lDescriptor[i].FrameName   ,
                                        lDescriptor[i].SearchFlags );
    }

    return lDispatcher;
}

// XDispatch
void SAL_CALL
PopupMenuControllerBase::dispatch(
    const URL& /*aURL*/,
    const Sequence< PropertyValue >& /*seqProperties*/ )
throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
    // must be implemented by subclass
    osl::MutexGuard aLock( m_aMutex );
    throwIfDisposed();
}

void SAL_CALL
PopupMenuControllerBase::addStatusListener(
    const Reference< XStatusListener >& xControl,
    const URL& aURL )
throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
    osl::ResettableMutexGuard aLock( m_aMutex );
    throwIfDisposed();
    aLock.clear();

    bool bStatusUpdate( false );
    rBHelper.addListener( ::getCppuType( &xControl ), xControl );

    aLock.reset();
    if ( aURL.Complete.startsWith( m_aBaseURL ) )
        bStatusUpdate = true;
    aLock.clear();

    if ( bStatusUpdate )
    {
        // Dummy update for popup menu controllers
        FeatureStateEvent aEvent;
        aEvent.FeatureURL = aURL;
        aEvent.IsEnabled  = sal_True;
        aEvent.Requery    = sal_False;
        aEvent.State      = Any();
        xControl->statusChanged( aEvent );
    }
}

void SAL_CALL PopupMenuControllerBase::removeStatusListener(
    const Reference< XStatusListener >& xControl,
    const URL& /*aURL*/ )
throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
    rBHelper.removeListener( ::getCppuType( &xControl ), xControl );
}

OUString PopupMenuControllerBase::determineBaseURL( const OUString& aURL )
{
    // Just use the main part of the URL for popup menu controllers
    sal_Int32     nQueryPart( 0 );
    sal_Int32     nSchemePart( 0 );
    OUString aMainURL( "vnd.sun.star.popup:" );

    nSchemePart = aURL.indexOf( ':' );
    if (( nSchemePart > 0 ) &&
        ( aURL.getLength() > ( nSchemePart+1 )))
    {
        nQueryPart  = aURL.indexOf( '?', nSchemePart );
        if ( nQueryPart > 0 )
            aMainURL += aURL.copy( nSchemePart, nQueryPart-nSchemePart );
        else if ( nQueryPart == -1 )
            aMainURL += aURL.copy( nSchemePart+1 );
    }

    return aMainURL;
}

// XInitialization
void SAL_CALL PopupMenuControllerBase::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException, std::exception )
{
    osl::MutexGuard aLock( m_aMutex );

    bool bInitalized( m_bInitialized );
    if ( !bInitalized )
    {
        PropertyValue       aPropValue;
        OUString       aCommandURL;
        Reference< XFrame > xFrame;

        for ( int i = 0; i < aArguments.getLength(); i++ )
        {
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name == "Frame" )
                    aPropValue.Value >>= xFrame;
                else if ( aPropValue.Name == "CommandURL" )
                    aPropValue.Value >>= aCommandURL;
                else if ( aPropValue.Name == "ModuleName" )
                    aPropValue.Value >>= m_aModuleName;
            }
        }

        if ( xFrame.is() && !aCommandURL.isEmpty() )
        {
            m_xFrame        = xFrame;
            m_aCommandURL   = aCommandURL;
            m_aBaseURL      = determineBaseURL( aCommandURL );
            m_bInitialized  = true;
        }
    }
}
// XPopupMenuController
void SAL_CALL PopupMenuControllerBase::setPopupMenu( const Reference< awt::XPopupMenu >& xPopupMenu ) throw ( RuntimeException, std::exception )
{
    osl::MutexGuard aLock( m_aMutex );
    throwIfDisposed();

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        SolarMutexGuard aSolarMutexGuard;

        m_xPopupMenu = xPopupMenu;
        m_xPopupMenu->addMenuListener( Reference< awt::XMenuListener >( (OWeakObject*)this, UNO_QUERY ));

        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

        URL aTargetURL;
        aTargetURL.Complete = m_aCommandURL;
        m_xURLTransformer->parseStrict( aTargetURL );
        m_xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );

        impl_setPopupMenu();

        updatePopupMenu();
    }
}
void PopupMenuControllerBase::impl_setPopupMenu()
{
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
