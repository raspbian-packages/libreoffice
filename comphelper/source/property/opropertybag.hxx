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

#ifndef COMPHELPER_OPROPERTYBAG_HXX
#define COMPHELPER_OPROPERTYBAG_HXX

/** === begin UNO includes === **/
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/container/XSet.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase6.hxx>
#include <comphelper/propstate.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertybag.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/uno3.hxx>

#include <map>
#include <set>
#include <memory>

//........................................................................
namespace comphelper
{
//........................................................................

    struct SAL_DLLPRIVATE UnoTypeLess : public ::std::unary_function< ::com::sun::star::uno::Type, bool >
    {
        inline bool operator()( const ::com::sun::star::uno::Type& _rLHS, const ::com::sun::star::uno::Type& _rRHS ) const
        {
            return rtl_ustr_compare(
                _rLHS.getTypeLibType()->pTypeName->buffer,
                _rRHS.getTypeLibType()->pTypeName->buffer
            ) < 0;
        }
    };

    typedef ::std::map< sal_Int32, ::com::sun::star::uno::Any >     MapInt2Any;
    typedef ::std::set< ::com::sun::star::uno::Type, UnoTypeLess >  TypeBag;

    //====================================================================
    //= OPropertyBag
    //====================================================================
    typedef ::cppu::WeakAggImplHelper6  <   ::com::sun::star::beans::XPropertyContainer
                                        ,   ::com::sun::star::beans::XPropertyAccess
                                        ,   ::com::sun::star::util::XModifiable
                                        ,   ::com::sun::star::lang::XServiceInfo
                                        ,   ::com::sun::star::lang::XInitialization
                                        ,   ::com::sun::star::container::XSet
                                        >   OPropertyBag_Base;
    typedef ::comphelper::OPropertyStateHelper  OPropertyBag_PBase;

    class OPropertyBag  :public ::comphelper::OMutexAndBroadcastHelper  // must be before OPropertyBag_PBase
                        ,public OPropertyBag_PBase
                        ,public OPropertyBag_Base
                        ,public ::cppu::IEventNotificationHook
    {
    private:
        ::comphelper::ComponentContext
                        m_aContext;

        /// our IPropertyArrayHelper implementation
        ::std::auto_ptr< ::cppu::OPropertyArrayHelper >
                        m_pArrayHelper;
        ::comphelper::PropertyBag
                        m_aDynamicProperties;
        /// set of allowed property types
        TypeBag         m_aAllowedTypes;
        /// should we automatically add properties which are tried to set, if they don't exist previously?
        bool            m_bAutoAddProperties;

        /// for notification
        ::cppu::OInterfaceContainerHelper m_NotifyListeners;
        /// modify flag
        bool            m_isModified;

    public:
        OPropertyBag( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext );

        // XServiceInfo - static versions
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::rtl::OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    protected:
        virtual ~OPropertyBag();
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        /** === begin UNO interface implementations == **/
        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XModifiable:
        virtual ::sal_Bool SAL_CALL isModified(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setModified( ::sal_Bool bModified )
            throw (::com::sun::star::beans::PropertyVetoException,
                    ::com::sun::star::uno::RuntimeException);

        // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener(
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::util::XModifyListener > & xListener)
            throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeModifyListener(
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::util::XModifyListener > & xListener)
            throw (::com::sun::star::uno::RuntimeException);

        // XPropertyContainer
        virtual void SAL_CALL addProperty( const ::rtl::OUString& Name, ::sal_Int16 Attributes, const ::com::sun::star::uno::Any& DefaultValue ) throw (::com::sun::star::beans::PropertyExistException, ::com::sun::star::beans::IllegalTypeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeProperty( const ::rtl::OUString& Name ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::NotRemoveableException, ::com::sun::star::uno::RuntimeException);

        // XPropertyAccess
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPropertyValues(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

        // XSet
        virtual ::sal_Bool SAL_CALL has( const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL insert( const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL remove( const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

        // XEnumerationAccess (base of XSet)
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw (::com::sun::star::uno::RuntimeException);

        // XElementAccess (basf of XEnumerationAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);
        /** === UNO interface implementations == **/

        // XPropertyState
        virtual ::com::sun::star::uno::Any  getPropertyDefaultByHandle( sal_Int32 _nHandle ) const;

        // OPropertyStateHelper
        virtual ::com::sun::star::beans::PropertyState  getPropertyStateByHandle( sal_Int32 _nHandle );

        // OPropertySetHelper
        virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
        virtual sal_Bool SAL_CALL convertFastPropertyValue( ::com::sun::star::uno::Any & rConvertedValue, ::com::sun::star::uno::Any & rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // IEventNotificationHook
        virtual void fireEvents(
            sal_Int32 * pnHandles,
            sal_Int32 nCount,
            sal_Bool bVetoable,
            bool bIgnoreRuntimeExceptionsWhileFiring);

        void SAL_CALL setModifiedImpl( ::sal_Bool bModified,
            bool bIgnoreRuntimeExceptionsWhileFiring);

    private:
        /** finds a free property handle
            @precond
                our mutex is locked
        */
        sal_Int32   findFreeHandle() const;

        /** implements the setPropertyValues method
            @param _rProps
                the property values to set

            @throws PropertyVetoException
                if the XMultiPropertySet::setPropertyValues call does so

            @throws ::com::sun::star::lang::IllegalArgumentException
                if the XMultiPropertySet::setPropertyValues call does so

            @throws ::com::sun::star::lang::WrappedTargetException
                if the XMultiPropertySet::setPropertyValues call does so

            @throws ::com::sun::star::uno::RuntimeException
                if the XMultiPropertySet::setPropertyValues call does so

            @throws ::com::sun::star::beans::UnknownPropertyException
                if the XMultiPropertySet::setPropertyValues call does so, and <arg>_bTolerateUnknownProperties</arg>
                was set to <FALSE/>

            @throws ::com::sun::star::lang::WrappedTargetException
                if the XMultiPropertySet::setPropertyValues call did throw an exception not listed
                above
        */
        void impl_setPropertyValues_throw( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rProps );

    private:
        OPropertyBag();                                 // never implemented
        OPropertyBag( const OPropertyBag& );            // never implemented
        OPropertyBag& operator=( const OPropertyBag& ); // never implemented
    protected:
        using ::cppu::OPropertySetHelper::getPropertyValues;
        using ::cppu::OPropertySetHelper::setPropertyValues;
        using ::cppu::OPropertySetHelper::getFastPropertyValue;
    };

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_OPROPERTYBAG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
