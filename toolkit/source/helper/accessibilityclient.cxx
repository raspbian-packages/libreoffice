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

#include <toolkit/helper/accessibilityclient.hxx>
#include <toolkit/helper/accessiblefactory.hxx>
#include <osl/module.h>
#include <osl/diagnose.h>
#include <tools/solar.h>

// #define UNLOAD_ON_LAST_CLIENT_DYING
    // this is not recommended currently. If enabled, the implementation will log
    // the number of active clients, and unload the acc library when the last client
    // goes away.
    // Sounds like a good idea, unfortunately, there's no guarantee that all objects
    // implemented in this library are already dead.
    // Iow, just because an object implementing an XAccessible (implemented in this lib
    // here) died, it's not said that everybody released all references to the
    // XAccessibleContext used by this component, and implemented in the acc lib.
    // So we cannot really unload the lib.
    //
    // Alternatively, if the lib would us own "usage counting", i.e. every component
    // implemented therein would affect a static ref count, the acc lib could care
    // for unloading itself.

//........................................................................
namespace toolkit
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::accessibility;

    namespace
    {
#ifdef UNLOAD_ON_LAST_CLIENT_DYING
        static oslInterlockedCount                      s_nAccessibilityClients = 0;
#endif // UNLOAD_ON_LAST_CLIENT_DYING
        static oslModule                                s_hAccessibleImplementationModule = NULL;
        static GetStandardAccComponentFactory           s_pAccessibleFactoryFunc = NULL;
        static ::rtl::Reference< IAccessibleFactory >   s_pFactory;
    }

    //====================================================================
    //= AccessibleDummyFactory
    //====================================================================
    class AccessibleDummyFactory : public IAccessibleFactory
    {
    public:
        AccessibleDummyFactory();

    protected:
        virtual ~AccessibleDummyFactory();

    private:
        AccessibleDummyFactory( const AccessibleDummyFactory& );            // never implemented
        AccessibleDummyFactory& operator=( const AccessibleDummyFactory& ); // never implemented

        oslInterlockedCount m_refCount;

    public:
        // IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

        // IAccessibleFactory
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXButton* /*_pXWindow*/ )
        {
            return NULL;
        }
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXCheckBox* /*_pXWindow*/ )
        {
            return NULL;
        }
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXRadioButton* /*_pXWindow*/ )
        {
            return NULL;
        }
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXListBox* /*_pXWindow*/ )
        {
            return NULL;
        }
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXFixedHyperlink* /*_pXWindow*/ )
        {
            return NULL;
        }
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXFixedText* /*_pXWindow*/ )
        {
            return NULL;
        }
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXScrollBar* /*_pXWindow*/ )
        {
            return NULL;
        }
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXEdit* /*_pXWindow*/ )
        {
            return NULL;
        }
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXComboBox* /*_pXWindow*/ )
        {
            return NULL;
        }
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXToolBox* /*_pXWindow*/ )
        {
            return NULL;
        }
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXWindow* /*_pXWindow*/ )
        {
            return NULL;
        }
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                createAccessible( Menu* /*_pMenu*/, sal_Bool /*_bIsMenuBar*/ )
        {
            return NULL;
        }
    };

    //--------------------------------------------------------------------
    AccessibleDummyFactory::AccessibleDummyFactory()
    {
    }

    //--------------------------------------------------------------------
    AccessibleDummyFactory::~AccessibleDummyFactory()
    {
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL AccessibleDummyFactory::acquire()
    {
        return osl_incrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL AccessibleDummyFactory::release()
    {
        if ( 0 == osl_decrementInterlockedCount( &m_refCount ) )
        {
            delete this;
            return 0;
        }
        return m_refCount;
    }

    //====================================================================
    //= AccessibilityClient
    //====================================================================
    //--------------------------------------------------------------------
    AccessibilityClient::AccessibilityClient()
        :m_bInitialized( false )
    {
    }

    //--------------------------------------------------------------------
    extern "C" { static void SAL_CALL thisModule() {} }

    void AccessibilityClient::ensureInitialized()
    {
        if ( m_bInitialized )
            return;

        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

#ifdef UNLOAD_ON_LAST_CLIENT_DYING
        if ( 1 == osl_incrementInterlockedCount( &s_nAccessibilityClients ) )
        {   // the first client
#endif // UNLOAD_ON_LAST_CLIENT_DYING
            // load the library implementing the factory
            if ( !s_pFactory.get() )
            {
                const ::rtl::OUString sModuleName(RTL_CONSTASCII_USTRINGPARAM(
                    SVLIBRARY( "acc" ))
                );
                s_hAccessibleImplementationModule = osl_loadModuleRelative( &thisModule, sModuleName.pData, 0 );
                if ( s_hAccessibleImplementationModule != NULL )
                {
                    const ::rtl::OUString sFactoryCreationFunc =
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getStandardAccessibleFactory"));
                    s_pAccessibleFactoryFunc = (GetStandardAccComponentFactory)
                        osl_getFunctionSymbol( s_hAccessibleImplementationModule, sFactoryCreationFunc.pData );

                }
                OSL_ENSURE( s_pAccessibleFactoryFunc, "AccessibilityClient::ensureInitialized: could not load the library, or not retrieve the needed symbol!" );

                // get a factory instance
                if ( s_pAccessibleFactoryFunc )
                {
                    IAccessibleFactory* pFactory = static_cast< IAccessibleFactory* >( (*s_pAccessibleFactoryFunc)() );
                    OSL_ENSURE( pFactory, "AccessibilityClient::ensureInitialized: no factory provided by the A11Y lib!" );
                    if ( pFactory )
                    {
                        s_pFactory = pFactory;
                        pFactory->release();
                    }
                }
            }

            if ( !s_pFactory.get() )
                // the attempt to load the lib, or to create the factory, failed
                // -> fall back to a dummy factory
                s_pFactory = new AccessibleDummyFactory;
#ifdef UNLOAD_ON_LAST_CLIENT_DYING
        }
#endif

        m_bInitialized = true;
    }

    //--------------------------------------------------------------------
    AccessibilityClient::~AccessibilityClient()
    {
        if ( m_bInitialized )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

#ifdef UNLOAD_ON_LAST_CLIENT_DYING
            if( 0 == osl_decrementInterlockedCount( &s_nAccessibilityClients ) )
            {
                s_pFactory = NULL;
                s_pAccessibleFactoryFunc = NULL;
                if ( s_hAccessibleImplementationModule )
                {
                    osl_unloadModule( s_hAccessibleImplementationModule );
                    s_hAccessibleImplementationModule = NULL;
                }
            }
#endif // UNLOAD_ON_LAST_CLIENT_DYING
        }
    }

    //--------------------------------------------------------------------
    IAccessibleFactory& AccessibilityClient::getFactory()
    {
        ensureInitialized();
        OSL_ENSURE( s_pFactory.is(), "AccessibilityClient::getFactory: at least a dummy factory should have been created!" );
        return *s_pFactory;
    }

//........................................................................
}   // namespace toolkit
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
