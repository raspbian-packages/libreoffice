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

#ifndef _DBA_CORE_QUERYCONTAINER_HXX_
#define _DBA_CORE_QUERYCONTAINER_HXX_

#include <cppuhelper/implbase5.hxx>
#include <comphelper/stl_types.hxx>
#include <cppuhelper/interfacecontainer.hxx>

/** === begin UNO includes == **/
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerApproveListener.hpp>
/** === end UNO includes === **/

#include "definitioncontainer.hxx"
#include "apitools.hxx"

namespace dbtools
{
    class IWarningsContainer;
}

namespace dbaccess
{

    typedef ::cppu::ImplHelper5 <   ::com::sun::star::container::XContainerListener
                                ,   ::com::sun::star::container::XContainerApproveListener
                                ,   ::com::sun::star::sdbcx::XDataDescriptorFactory
                                ,   ::com::sun::star::sdbcx::XAppend
                                ,   ::com::sun::star::sdbcx::XDrop
                                >   OQueryContainer_Base;

    //==========================================================================
    //= OQueryContainer
    //==========================================================================
    class OQueryContainer;
    class OContainerListener;
    class OQuery;
    class OQueryContainer   : public ODefinitionContainer
                            , public OQueryContainer_Base
    {
    protected:
        ::dbtools::IWarningsContainer*  m_pWarnings;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                                        m_xCommandDefinitions;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                                        m_xConnection;
        // possible actions on our "aggregate"
        enum AGGREGATE_ACTION { NONE, INSERTING, FLUSHING };
        AGGREGATE_ACTION        m_eDoingCurrently;

        OContainerListener*     m_pCommandsListener;

        /** a class which automatically resets m_eDoingCurrently in it's destructor
        */
        class OAutoActionReset; // just for the following friend declaration
        friend class OAutoActionReset;
        class OAutoActionReset
        {
            OQueryContainer*        m_pActor;
        public:
            OAutoActionReset(OQueryContainer* _pActor) : m_pActor(_pActor) { }
            ~OAutoActionReset() { m_pActor->m_eDoingCurrently = NONE; }
        };

        // ODefinitionContainer
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > createObject( const ::rtl::OUString& _rName);
        virtual sal_Bool checkExistence(const ::rtl::OUString& _rName);

        // helper
        virtual void SAL_CALL disposing();
        virtual ~OQueryContainer();
    public:
        /** ctor of the container. The parent has to support the <type scope="com::sun::star::sdbc">XConnection</type>
            interface.<BR>

            @param _pWarnings
                specifies a warnings container (May be <NULL/>)

                Any errors which occur during the lifetime of the query container,
                which cannot be reported as exceptionts (for instance in methods where throwing an SQLException is
                not allowed) will be appended to this container.</p>
                <p>The caller is responsible for ensuring the lifetime of the object pointed to by this parameter.
        */
        OQueryContainer(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxCommandDefinitions,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            ::dbtools::IWarningsContainer* _pWarnings
            );

        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )
        DECLARE_SERVICE_INFO();

    // ::com::sun::star::container::XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

        // XContainerApproveListener
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveInsertElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveReplaceElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveRemoveElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XDataDescriptorFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XAppend
        virtual void SAL_CALL appendByDescriptor( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XDrop
        virtual void SAL_CALL dropByName( const ::rtl::OUString& elementName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL dropByIndex( sal_Int32 index ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XElementAccess
        virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::container::XIndexAccess
        virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::container::XNameAccess
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);

    protected:
        // OContentHelper overridables
        virtual ::rtl::OUString determineContentType() const;

    private:
        // helper
        /** create a query object wrapping a CommandDefinition given by name. To retrieve the object, the CommandDescription
            container will be asked for the given name.<BR>
            The returned object is acquired once.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > implCreateWrapper(const ::rtl::OUString& _rName);
        /// create a query object wrapping a CommandDefinition. The returned object is acquired once.
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > implCreateWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _rxCommandDesc);

    };
}   // namespace dbaccess

#endif // _DBA_CORE_QUERYCONTAINER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
