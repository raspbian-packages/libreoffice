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

#include <accessibility/extended/accessibletabbar.hxx>
#include <svtools/tabbar.hxx>
#include <accessibility/extended/accessibletabbarpagelist.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/awt/vclxfont.hxx>
#include <toolkit/helper/convert.hxx>

#include <vector>


//.........................................................................
namespace accessibility
{
//.........................................................................

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::accessibility;
    using namespace ::comphelper;

    DBG_NAME( AccessibleTabBar )

    //  ----------------------------------------------------
    //  class AccessibleTabBar
    //  ----------------------------------------------------

    AccessibleTabBar::AccessibleTabBar( TabBar* pTabBar )
        :AccessibleTabBarBase( pTabBar )
    {
        DBG_CTOR( AccessibleTabBar, NULL );

        if ( m_pTabBar )
            m_aAccessibleChildren.assign( m_pTabBar->GetAccessibleChildWindowCount() + 1, Reference< XAccessible >() );
    }

    // -----------------------------------------------------------------------------

    AccessibleTabBar::~AccessibleTabBar()
    {
        DBG_DTOR( AccessibleTabBar, NULL );
    }

    // -----------------------------------------------------------------------------

    void AccessibleTabBar::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
    {
        Any aOldValue, aNewValue;

        switch ( rVclWindowEvent.GetId() )
        {
            case VCLEVENT_WINDOW_ENABLED:
            {
                aNewValue <<= AccessibleStateType::SENSITIVE;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
                aNewValue <<= AccessibleStateType::ENABLED;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
            }
            break;
            case VCLEVENT_WINDOW_DISABLED:
            {
                aOldValue <<= AccessibleStateType::ENABLED;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
                aOldValue <<= AccessibleStateType::SENSITIVE;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
            }
            break;
            case VCLEVENT_WINDOW_GETFOCUS:
            {
                aNewValue <<= AccessibleStateType::FOCUSED;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
            }
            break;
            case VCLEVENT_WINDOW_LOSEFOCUS:
            {
                aOldValue <<= AccessibleStateType::FOCUSED;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
            }
            break;
            case VCLEVENT_WINDOW_SHOW:
            {
                aNewValue <<= AccessibleStateType::SHOWING;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
            }
            break;
            case VCLEVENT_WINDOW_HIDE:
            {
                aOldValue <<= AccessibleStateType::SHOWING;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
            }
            break;
            default:
            {
                AccessibleTabBarBase::ProcessWindowEvent( rVclWindowEvent );
            }
            break;
        }
    }

    // -----------------------------------------------------------------------------

    void AccessibleTabBar::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
    {
        if ( m_pTabBar )
        {
            if ( m_pTabBar->IsEnabled() )
            {
                rStateSet.AddState( AccessibleStateType::ENABLED );
                rStateSet.AddState( AccessibleStateType::SENSITIVE );
            }

            rStateSet.AddState( AccessibleStateType::FOCUSABLE );

            if ( m_pTabBar->HasFocus() )
                rStateSet.AddState( AccessibleStateType::FOCUSED );

            rStateSet.AddState( AccessibleStateType::VISIBLE );

            if ( m_pTabBar->IsVisible() )
                rStateSet.AddState( AccessibleStateType::SHOWING );

            if ( m_pTabBar->GetStyle() & WB_SIZEABLE )
                rStateSet.AddState( AccessibleStateType::RESIZABLE );
        }
    }

    // -----------------------------------------------------------------------------
    // OCommonAccessibleComponent
    // -----------------------------------------------------------------------------

    awt::Rectangle AccessibleTabBar::implGetBounds() throw (RuntimeException)
    {
        awt::Rectangle aBounds;
        if ( m_pTabBar )
            aBounds = AWTRectangle( Rectangle( m_pTabBar->GetPosPixel(), m_pTabBar->GetSizePixel() ) );

        return aBounds;
    }

    // -----------------------------------------------------------------------------
    // XInterface
    // -----------------------------------------------------------------------------

    IMPLEMENT_FORWARD_XINTERFACE2( AccessibleTabBar, AccessibleExtendedComponentHelper_BASE, AccessibleTabBar_BASE )

    // -----------------------------------------------------------------------------
    // XTypeProvider
    // -----------------------------------------------------------------------------

    IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccessibleTabBar, AccessibleExtendedComponentHelper_BASE, AccessibleTabBar_BASE )

    // -----------------------------------------------------------------------------
    // XComponent
    // -----------------------------------------------------------------------------

    void AccessibleTabBar::disposing()
    {
        AccessibleTabBarBase::disposing();

        // dispose all children
        for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
        {
            Reference< XComponent > xComponent( m_aAccessibleChildren[i], UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        m_aAccessibleChildren.clear();
    }

    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    ::rtl::OUString AccessibleTabBar::getImplementationName() throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svtools.AccessibleTabBar" ));
    }

    // -----------------------------------------------------------------------------

    sal_Bool AccessibleTabBar::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
        const ::rtl::OUString* pNames = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pNames + aNames.getLength();
        for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
            ;

        return pNames != pEnd;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > AccessibleTabBar::getSupportedServiceNames() throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aNames(1);
        aNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.AccessibleTabBar" ));
        return aNames;
    }

    // -----------------------------------------------------------------------------
    // XAccessible
    // -----------------------------------------------------------------------------

    Reference< XAccessibleContext > AccessibleTabBar::getAccessibleContext(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return this;
    }

    // -----------------------------------------------------------------------------
    // XAccessibleContext
    // -----------------------------------------------------------------------------

    sal_Int32 AccessibleTabBar::getAccessibleChildCount() throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return m_aAccessibleChildren.size();
    }

    // -----------------------------------------------------------------------------

    Reference< XAccessible > AccessibleTabBar::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        if ( i < 0 || i >= getAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild = m_aAccessibleChildren[i];
        if ( !xChild.is() )
        {
            if ( m_pTabBar )
            {
                sal_Int32 nCount = m_pTabBar->GetAccessibleChildWindowCount();

                if ( i < nCount )
                {
                    Window* pChild = m_pTabBar->GetAccessibleChildWindow( (sal_uInt16)i );
                    if ( pChild )
                        xChild = pChild->GetAccessible();
                }
                else if ( i == nCount )
                {
                    xChild = new AccessibleTabBarPageList( m_pTabBar, i );
                }

                // insert into child list
                m_aAccessibleChildren[i] = xChild;
            }
        }

        return xChild;
    }

    // -----------------------------------------------------------------------------

    Reference< XAccessible > AccessibleTabBar::getAccessibleParent(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        Reference< XAccessible > xParent;
        if ( m_pTabBar )
        {
            Window* pParent = m_pTabBar->GetAccessibleParentWindow();
            if ( pParent )
                xParent = pParent->GetAccessible();
        }

        return xParent;
    }

    // -----------------------------------------------------------------------------

    sal_Int32 AccessibleTabBar::getAccessibleIndexInParent(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        sal_Int32 nIndexInParent = -1;
        if ( m_pTabBar )
        {
            Window* pParent = m_pTabBar->GetAccessibleParentWindow();
            if ( pParent )
            {
                for ( sal_uInt16 i = 0, nCount = pParent->GetAccessibleChildWindowCount(); i < nCount; ++i )
                {
                    Window* pChild = pParent->GetAccessibleChildWindow( i );
                    if ( pChild == static_cast< Window* >( m_pTabBar ) )
                    {
                        nIndexInParent = i;
                        break;
                    }
                }
            }
        }

        return nIndexInParent;
    }

    // -----------------------------------------------------------------------------

    sal_Int16 AccessibleTabBar::getAccessibleRole(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return AccessibleRole::PANEL;
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString AccessibleTabBar::getAccessibleDescription( ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        ::rtl::OUString sDescription;
        if ( m_pTabBar )
            sDescription = m_pTabBar->GetAccessibleDescription();

        return sDescription;
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString AccessibleTabBar::getAccessibleName(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        ::rtl::OUString sName;
        if ( m_pTabBar )
            sName = m_pTabBar->GetAccessibleName();

        return sName;
    }

    // -----------------------------------------------------------------------------

    Reference< XAccessibleRelationSet > AccessibleTabBar::getAccessibleRelationSet(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
        Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
        return xSet;
    }

    // -----------------------------------------------------------------------------

    Reference< XAccessibleStateSet > AccessibleTabBar::getAccessibleStateSet(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
        Reference< XAccessibleStateSet > xSet = pStateSetHelper;

        if ( !rBHelper.bDisposed && !rBHelper.bInDispose )
        {
            FillAccessibleStateSet( *pStateSetHelper );
        }
        else
        {
            pStateSetHelper->AddState( AccessibleStateType::DEFUNC );
        }

        return xSet;
    }

    // -----------------------------------------------------------------------------

    Locale AccessibleTabBar::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return Application::GetSettings().GetLocale();
    }

    // -----------------------------------------------------------------------------
    // XAccessibleComponent
    // -----------------------------------------------------------------------------

    Reference< XAccessible > AccessibleTabBar::getAccessibleAtPoint( const awt::Point& rPoint ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        Reference< XAccessible > xChild;
        for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
        {
            Reference< XAccessible > xAcc = getAccessibleChild( i );
            if ( xAcc.is() )
            {
                Reference< XAccessibleComponent > xComp( xAcc->getAccessibleContext(), UNO_QUERY );
                if ( xComp.is() )
                {
                    Rectangle aRect = VCLRectangle( xComp->getBounds() );
                    Point aPos = VCLPoint( rPoint );
                    if ( aRect.IsInside( aPos ) )
                    {
                        xChild = xAcc;
                        break;
                    }
                }
            }
        }

        return xChild;
    }

    // -----------------------------------------------------------------------------

    void AccessibleTabBar::grabFocus(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        if ( m_pTabBar )
            m_pTabBar->GrabFocus();
    }

    // -----------------------------------------------------------------------------

    sal_Int32 AccessibleTabBar::getForeground(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        sal_Int32 nColor = 0;
        if ( m_pTabBar )
        {
            if ( m_pTabBar->IsControlForeground() )
                nColor = m_pTabBar->GetControlForeground().GetColor();
            else
            {
                Font aFont;
                if ( m_pTabBar->IsControlFont() )
                    aFont = m_pTabBar->GetControlFont();
                else
                    aFont = m_pTabBar->GetFont();
                nColor = aFont.GetColor().GetColor();
            }
        }

        return nColor;
    }

    // -----------------------------------------------------------------------------

    sal_Int32 AccessibleTabBar::getBackground(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        sal_Int32 nColor = 0;
        if ( m_pTabBar )
        {
            if ( m_pTabBar->IsControlBackground() )
                nColor = m_pTabBar->GetControlBackground().GetColor();
            else
                nColor = m_pTabBar->GetBackground().GetColor().GetColor();
        }

        return nColor;
    }

    // -----------------------------------------------------------------------------
    // XAccessibleExtendedComponent
    // -----------------------------------------------------------------------------

    Reference< awt::XFont > AccessibleTabBar::getFont(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        Reference< awt::XFont > xFont;
        if ( m_pTabBar )
        {
            Reference< awt::XDevice > xDev( m_pTabBar->GetComponentInterface(), UNO_QUERY );
            if ( xDev.is() )
            {
                Font aFont;
                if ( m_pTabBar->IsControlFont() )
                    aFont = m_pTabBar->GetControlFont();
                else
                    aFont = m_pTabBar->GetFont();
                VCLXFont* pVCLXFont = new VCLXFont;
                pVCLXFont->Init( *xDev.get(), aFont );
                xFont = pVCLXFont;
            }
        }

        return xFont;
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString AccessibleTabBar::getTitledBorderText(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        ::rtl::OUString sText;
        if ( m_pTabBar )
            sText = m_pTabBar->GetText();

        return sText;
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString AccessibleTabBar::getToolTipText(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        ::rtl::OUString sText;
        if ( m_pTabBar )
            sText = m_pTabBar->GetQuickHelpText();

        return sText;
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace accessibility
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
