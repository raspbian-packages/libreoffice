/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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


#include "ctp_panel.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/WindowClass.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

//......................................................................................................................
namespace sd { namespace colortoolpanel
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::awt::XWindowPeer;
    using ::com::sun::star::lang::XMultiComponentFactory;
    using ::com::sun::star::awt::XToolkit;
    using ::com::sun::star::awt::WindowDescriptor;
    using ::com::sun::star::awt::WindowClass_SIMPLE;
    using ::com::sun::star::awt::Rectangle;
    using ::com::sun::star::awt::PaintEvent;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::awt::XDevice;
    using ::com::sun::star::awt::XGraphics;
    using ::com::sun::star::accessibility::XAccessible;
    using ::com::sun::star::frame::XFrame;
    /** === end UNO using === **/
    namespace WindowAttribute = ::com::sun::star::awt::WindowAttribute;
    namespace PosSize = ::com::sun::star::awt::PosSize;
    namespace UIElementType = ::com::sun::star::ui::UIElementType;

    //==================================================================================================================
    //= helpers
    //==================================================================================================================
    namespace
    {
        Reference< XWindow > lcl_createPlainWindow_nothrow( const Reference< XComponentContext >& i_rContext,
            const Reference< XWindowPeer >& i_rParentWindow )
        {
            try
            {
                ENSURE_OR_THROW( i_rContext.is(), "illegal component context" );
                Reference< XMultiComponentFactory > xFactory( i_rContext->getServiceManager(), UNO_SET_THROW );
                Reference< XToolkit > xToolkit( xFactory->createInstanceWithContext(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.Toolkit" ) ),
                    i_rContext
                ), UNO_QUERY_THROW );

                WindowDescriptor aWindow;
                aWindow.Type = WindowClass_SIMPLE;
                aWindow.WindowServiceName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "window" ) );
                aWindow.Parent = i_rParentWindow;
                aWindow.WindowAttributes = WindowAttribute::BORDER;

                Reference< XWindowPeer > xWindow( xToolkit->createWindow( aWindow ), UNO_SET_THROW );
                return Reference< XWindow >( xWindow, UNO_QUERY_THROW );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return NULL;
        }
    }
    //==================================================================================================================
    //= class SingleColorPanel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    SingleColorPanel::SingleColorPanel( const Reference< XComponentContext >& i_rContext, const Reference< XWindow >& i_rParentWindow, const ::sal_Int32 i_nPanelColor )
        :SingleColorPanel_Base( m_aMutex )
        ,m_xWindow()
        ,m_nPanelColor( i_nPanelColor )
    {
        // retrieve the parent window for our to-be-created pane window
        Reference< XWindowPeer > xParentPeer( i_rParentWindow, UNO_QUERY );

        osl_incrementInterlockedCount( &m_refCount );
        if ( xParentPeer.is() )
        {
            m_xWindow = lcl_createPlainWindow_nothrow( i_rContext, xParentPeer );
            m_xWindow->addPaintListener( this );
            if ( m_xWindow.is() )
            {
                const Rectangle aPanelAnchorSize( i_rParentWindow->getPosSize() );
                m_xWindow->setPosSize( 0, 0, aPanelAnchorSize.Width, aPanelAnchorSize.Height, PosSize::POSSIZE );
                m_xWindow->setVisible( sal_True );
            }
        }
        osl_decrementInterlockedCount( &m_refCount );
    }

    //------------------------------------------------------------------------------------------------------------------
    SingleColorPanel::~SingleColorPanel()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XWindow > SAL_CALL SingleColorPanel::getWindow() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xWindow.is() )
            throw DisposedException( ::rtl::OUString(), *this );
        return m_xWindow;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL SingleColorPanel::createAccessible( const Reference< XAccessible >& i_rParentAccessible ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xWindow.is() )
            throw DisposedException( ::rtl::OUString(), *this );

        // TODO: the following is wrong, since it doesn't respect i_rParentAccessible. In a real extension, you should
        // implement this correctly :)
        (void)i_rParentAccessible;
        return Reference< XAccessible >( getWindow(), UNO_QUERY );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SingleColorPanel::windowPaint( const PaintEvent& i_rEvent ) throw (RuntimeException)
    {
        try
        {
            const Reference< XDevice > xDevice( i_rEvent.Source, UNO_QUERY_THROW );
            const Reference< XGraphics > xGraphics( xDevice->createGraphics(), UNO_SET_THROW );
            xGraphics->setFillColor( m_nPanelColor );
            xGraphics->setLineColor( 0x00FFFFFF );

            const Reference< XWindow > xWindow( i_rEvent.Source, UNO_QUERY_THROW );
            const Rectangle aWindowRect( xWindow->getPosSize() );
            xGraphics->drawRect( 0, 0, aWindowRect.Width - 1, aWindowRect.Height - 1 );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SingleColorPanel::disposing( const EventObject& i_rSource ) throw (RuntimeException)
    {
        (void)i_rSource;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SingleColorPanel::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xWindow.is() )
            // already disposed
            return;
        m_xWindow->removePaintListener( this );
        try
        {
            Reference< XComponent > xWindowComp( m_xWindow, UNO_QUERY_THROW );
            xWindowComp->dispose();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        m_xWindow.clear();
    }

    //==================================================================================================================
    //= PanelUIElement
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    PanelUIElement::PanelUIElement( const Reference< XComponentContext >& i_rContext, const Reference< XWindow >& i_rParentWindow,
        const ::rtl::OUString& i_rResourceURL, const ::sal_Int32 i_nPanelColor )
        :PanelUIElement_Base( m_aMutex )
        ,m_sResourceURL( i_rResourceURL )
        ,m_xToolPanel( new SingleColorPanel( i_rContext, i_rParentWindow, i_nPanelColor ) )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    PanelUIElement::~PanelUIElement()
    {
    }

     //------------------------------------------------------------------------------------------------------------------
    Reference< XFrame > SAL_CALL PanelUIElement::getFrame() throw (RuntimeException)
    {
        // TODO
        return NULL;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL PanelUIElement::getResourceURL() throw (RuntimeException)
    {
        return m_sResourceURL;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int16 SAL_CALL PanelUIElement::getType() throw (RuntimeException)
    {
        return UIElementType::TOOLPANEL;
    }

     //------------------------------------------------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL PanelUIElement::getRealInterface(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xToolPanel.is() )
            throw DisposedException();
        return m_xToolPanel;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL PanelUIElement::disposing()
    {
        Reference< XComponent > xPanelComponent( m_xToolPanel, UNO_QUERY_THROW );
        m_xToolPanel.clear();
        xPanelComponent->dispose();
    }

//......................................................................................................................
} } // namespace sd::colortoolpanel
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
