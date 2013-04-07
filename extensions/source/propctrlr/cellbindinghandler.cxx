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

#include "cellbindinghandler.hxx"
#include "formstrings.hxx"
#include "formmetadata.hxx"
#include "cellbindinghelper.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
/** === end UNO includes === **/
#include <tools/debug.hxx>

//------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_CellBindingPropertyHandler()
{
    ::pcr::CellBindingPropertyHandler::registerImplementation();
}

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::table;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::inspection;
    using namespace ::com::sun::star::form::binding;
    using namespace ::comphelper;

    //====================================================================
    //= CellBindingPropertyHandler
    //====================================================================
    DBG_NAME( CellBindingPropertyHandler )
    //--------------------------------------------------------------------
    CellBindingPropertyHandler::CellBindingPropertyHandler( const Reference< XComponentContext >& _rxContext )
        :CellBindingPropertyHandler_Base( _rxContext )
        ,m_pCellExchangeConverter( new DefaultEnumRepresentation( *m_pInfoService, ::getCppuType( static_cast< sal_Int16* >( NULL ) ), PROPERTY_ID_CELL_EXCHANGE_TYPE ) )
    {
        DBG_CTOR( CellBindingPropertyHandler, NULL );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL CellBindingPropertyHandler::getImplementationName_static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.extensions.CellBindingPropertyHandler" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL CellBindingPropertyHandler::getSupportedServiceNames_static(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported( 1 );
        aSupported[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.inspection.CellBindingPropertyHandler" ) );
        return aSupported;
    }

    //--------------------------------------------------------------------
    void CellBindingPropertyHandler::onNewComponent()
    {
        PropertyHandlerComponent::onNewComponent();

        Reference< XModel > xDocument( impl_getContextDocument_nothrow() );
        DBG_ASSERT( xDocument.is(), "CellBindingPropertyHandler::onNewComponent: no document!" );
        if ( CellBindingHelper::isSpreadsheetDocument( xDocument ) )
            m_pHelper.reset( new CellBindingHelper( m_xComponent, xDocument ) );
    }

    //--------------------------------------------------------------------
    CellBindingPropertyHandler::~CellBindingPropertyHandler( )
    {
        DBG_DTOR( CellBindingPropertyHandler, NULL );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL CellBindingPropertyHandler::getActuatingProperties( ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aInterestingProperties( 3 );
        aInterestingProperties[0] = PROPERTY_LIST_CELL_RANGE;
        aInterestingProperties[1] = PROPERTY_BOUND_CELL;
        aInterestingProperties[2] = PROPERTY_CONTROLSOURCE;
        return aInterestingProperties;
    }

    //--------------------------------------------------------------------
    void SAL_CALL CellBindingPropertyHandler::actuatingPropertyChanged( const ::rtl::OUString& _rActuatingPropertyName, const Any& _rNewValue, const Any& /*_rOldValue*/, const Reference< XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (NullPointerException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nActuatingPropId( impl_getPropertyId_throw( _rActuatingPropertyName ) );
        OSL_PRECOND( m_pHelper.get(), "CellBindingPropertyHandler::actuatingPropertyChanged: inconsistentcy!" );
            // if we survived impl_getPropertyId_throw, we should have a helper, since no helper implies no properties

        OSL_PRECOND( _rxInspectorUI.is(), "FormComponentPropertyHandler::actuatingPropertyChanged: no access to the UI!" );
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        ::std::vector< PropertyId > aDependentProperties;

        switch ( nActuatingPropId )
        {
        // ----- BoundCell -----
        case PROPERTY_ID_BOUND_CELL:
        {
            // the SQL-data-binding related properties need to be enabled if and only if
            // there is *no* valid cell binding
            Reference< XValueBinding > xBinding;
            _rNewValue >>= xBinding;

            if ( impl_isSupportedProperty_nothrow( PROPERTY_ID_CELL_EXCHANGE_TYPE ) )
                _rxInspectorUI->enablePropertyUI( PROPERTY_CELL_EXCHANGE_TYPE, xBinding.is() );
            if ( impl_componentHasProperty_throw( PROPERTY_CONTROLSOURCE ) )
                _rxInspectorUI->enablePropertyUI( PROPERTY_CONTROLSOURCE, !xBinding.is() );

            if ( impl_isSupportedProperty_nothrow( PROPERTY_ID_FILTERPROPOSAL ) )
                _rxInspectorUI->enablePropertyUI( PROPERTY_FILTERPROPOSAL, !xBinding.is() );
            if ( impl_isSupportedProperty_nothrow( PROPERTY_ID_EMPTY_IS_NULL ) )
                _rxInspectorUI->enablePropertyUI( PROPERTY_EMPTY_IS_NULL, !xBinding.is() );

            aDependentProperties.push_back( PROPERTY_ID_BOUNDCOLUMN );

            if ( !xBinding.is() && m_pHelper->getCurrentBinding().is() )
            {
                // ensure that the "transfer selection as" property is reset. Since we can't remember
                // it at the object itself, but derive it from the binding only, we have to normalize
                // it now that there *is* no binding anymore.
                setPropertyValue( PROPERTY_CELL_EXCHANGE_TYPE, makeAny( (sal_Int16) 0 ) );
            }
        }
        break;

        // ----- CellRange -----
        case PROPERTY_ID_LIST_CELL_RANGE:
        {
            // the list source related properties need to be enabled if and only if
            // there is *no* valid external list source for the control
            Reference< XListEntrySource > xSource;
            _rNewValue >>= xSource;

            _rxInspectorUI->enablePropertyUI( PROPERTY_STRINGITEMLIST, !xSource.is() );
            _rxInspectorUI->enablePropertyUI( PROPERTY_LISTSOURCE, !xSource.is() );
            _rxInspectorUI->enablePropertyUI( PROPERTY_LISTSOURCETYPE, !xSource.is() );

            aDependentProperties.push_back( PROPERTY_ID_BOUNDCOLUMN );

            // also reset the list entries if the cell range is reset
            // #i28319#
            if ( !_bFirstTimeInit )
            {
                try
                {
                    if ( !xSource.is() )
                        setPropertyValue( PROPERTY_STRINGITEMLIST, makeAny( Sequence< ::rtl::OUString >() ) );
                }
                catch( const Exception& )
                {
                    OSL_FAIL( "OPropertyBrowserController::actuatingPropertyChanged( ListCellRange ): caught an exception while resetting the string items!" );
                }
            }
        }
        break;  // case PROPERTY_ID_LIST_CELL_RANGE

        // ----- DataField -----
        case PROPERTY_ID_CONTROLSOURCE:
        {
            ::rtl::OUString sControlSource;
            _rNewValue >>= sControlSource;
            if ( impl_isSupportedProperty_nothrow( PROPERTY_ID_BOUND_CELL ) )
                _rxInspectorUI->enablePropertyUI( PROPERTY_BOUND_CELL, sControlSource.getLength() == 0 );
        }
        break;  // case PROPERTY_ID_CONTROLSOURCE

        default:
            OSL_FAIL( "CellBindingPropertyHandler::actuatingPropertyChanged: did not register for this property!" );
        }

        for ( ::std::vector< PropertyId >::const_iterator loopAffected = aDependentProperties.begin();
              loopAffected != aDependentProperties.end();
              ++loopAffected
            )
        {
            impl_updateDependentProperty_nothrow( *loopAffected, _rxInspectorUI );
        }
    }

    //--------------------------------------------------------------------
    void CellBindingPropertyHandler::impl_updateDependentProperty_nothrow( PropertyId _nPropId, const Reference< XObjectInspectorUI >& _rxInspectorUI ) const
    {
        try
        {
            switch ( _nPropId )
            {
            // ----- BoundColumn -----
            case PROPERTY_ID_BOUNDCOLUMN:
            {
                CellBindingPropertyHandler* pNonConstThis = const_cast< CellBindingPropertyHandler* >( this );
                Reference< XValueBinding > xBinding( pNonConstThis->getPropertyValue( PROPERTY_BOUND_CELL ), UNO_QUERY );
                Reference< XListEntrySource > xListSource( pNonConstThis->getPropertyValue( PROPERTY_LIST_CELL_RANGE ), UNO_QUERY );

                if ( impl_isSupportedProperty_nothrow( PROPERTY_ID_BOUNDCOLUMN ) )
                    _rxInspectorUI->enablePropertyUI( PROPERTY_BOUNDCOLUMN, !xBinding.is() && !xListSource.is() );
            }
            break;  // case PROPERTY_ID_BOUNDCOLUMN

            }   // switch

        }
        catch( const Exception& )
        {
            OSL_FAIL( "CellBindingPropertyHandler::impl_updateDependentProperty_nothrow: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    Any SAL_CALL CellBindingPropertyHandler::getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        OSL_ENSURE( m_pHelper.get(), "CellBindingPropertyHandler::getPropertyValue: inconsistency!" );
            // if we survived impl_getPropertyId_throw, we should have a helper, since no helper implies no properties

        Any aReturn;
        switch ( nPropId )
        {
        case PROPERTY_ID_BOUND_CELL:
        {
            Reference< XValueBinding > xBinding( m_pHelper->getCurrentBinding() );
            if ( !m_pHelper->isCellBinding( xBinding ) )
                xBinding.clear();

            aReturn <<= xBinding;
        }
        break;

        case PROPERTY_ID_LIST_CELL_RANGE:
        {
            Reference< XListEntrySource > xSource( m_pHelper->getCurrentListSource() );
            if ( !m_pHelper->isCellRangeListSource( xSource ) )
                xSource.clear();

            aReturn <<= xSource;
        }
        break;

        case PROPERTY_ID_CELL_EXCHANGE_TYPE:
        {
            Reference< XValueBinding > xBinding( m_pHelper->getCurrentBinding() );
            aReturn <<= (sal_Int16)( m_pHelper->isCellIntegerBinding( xBinding ) ? 1 : 0 );
        }
        break;

        default:
            OSL_FAIL( "CellBindingPropertyHandler::getPropertyValue: cannot handle this!" );
            break;
        }
        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL CellBindingPropertyHandler::setPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rValue ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        OSL_ENSURE( m_pHelper.get(), "CellBindingPropertyHandler::setPropertyValue: inconsistency!" );
            // if we survived impl_getPropertyId_throw, we should have a helper, since no helper implies no properties

        try
        {
            Any aOldValue = getPropertyValue( _rPropertyName );

            switch ( nPropId )
            {
            case PROPERTY_ID_BOUND_CELL:
            {
                Reference< XValueBinding > xBinding;
                _rValue >>= xBinding;
                m_pHelper->setBinding( xBinding );
            }
            break;

            case PROPERTY_ID_LIST_CELL_RANGE:
            {
                Reference< XListEntrySource > xSource;
                _rValue >>= xSource;
                m_pHelper->setListSource( xSource );
            }
            break;

            case PROPERTY_ID_CELL_EXCHANGE_TYPE:
            {
                sal_Int16 nExchangeType = 0;
                OSL_VERIFY( _rValue >>= nExchangeType );

                Reference< XValueBinding > xBinding = m_pHelper->getCurrentBinding( );
                if ( xBinding.is() )
                {
                    sal_Bool bNeedIntegerBinding = ( nExchangeType == 1 );
                    if ( (bool)bNeedIntegerBinding != m_pHelper->isCellIntegerBinding( xBinding ) )
                    {
                        CellAddress aAddress;
                        if ( m_pHelper->getAddressFromCellBinding( xBinding, aAddress ) )
                        {
                            xBinding = m_pHelper->createCellBindingFromAddress( aAddress, bNeedIntegerBinding );
                            m_pHelper->setBinding( xBinding );
                        }
                    }
                }
            }
            break;

            default:
                OSL_FAIL( "CellBindingPropertyHandler::setPropertyValue: cannot handle this!" );
                break;
            }

            impl_setContextDocumentModified_nothrow();

            Any aNewValue( getPropertyValue( _rPropertyName ) );
            firePropertyChange( _rPropertyName, nPropId, aOldValue, aNewValue );
            // TODO/UNOize: can't we make this a part of the base class, for all those "virtual"
            // properties? Base class'es |setPropertyValue| could call some |doSetPropertyValue|,
            // and handle the listener notification itself
        }
        catch( const Exception& )
        {
            OSL_FAIL( "CellBindingPropertyHandler::setPropertyValue: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    Any SAL_CALL CellBindingPropertyHandler::convertToPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rControlValue ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        Any aPropertyValue;

        OSL_ENSURE( m_pHelper.get(), "CellBindingPropertyHandler::convertToPropertyValue: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return aPropertyValue;

        PropertyId nPropId( m_pInfoService->getPropertyId( _rPropertyName ) );

        ::rtl::OUString sControlValue;
        OSL_VERIFY( _rControlValue >>= sControlValue );
        switch( nPropId )
        {
            case PROPERTY_ID_LIST_CELL_RANGE:
                aPropertyValue <<= m_pHelper->createCellListSourceFromStringAddress( sControlValue );
                break;

            case PROPERTY_ID_BOUND_CELL:
            {
                // if we have the possibility of an integer binding, then we must preserve
                // this property's value (e.g. if the current binding is an integer binding, then
                // the newly created one must be, too)
                bool bIntegerBinding = false;
                if ( m_pHelper->isCellIntegerBindingAllowed() )
                {
                    sal_Int16 nCurrentBindingType = 0;
                    getPropertyValue( PROPERTY_CELL_EXCHANGE_TYPE ) >>= nCurrentBindingType;
                    bIntegerBinding = ( nCurrentBindingType != 0 );
                }
                aPropertyValue <<= m_pHelper->createCellBindingFromStringAddress( sControlValue, bIntegerBinding );
            }
            break;

            case PROPERTY_ID_CELL_EXCHANGE_TYPE:
                m_pCellExchangeConverter->getValueFromDescription( sControlValue, aPropertyValue );
                break;

            default:
                OSL_FAIL( "CellBindingPropertyHandler::convertToPropertyValue: cannot handle this!" );
                break;
        }

        return aPropertyValue;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL CellBindingPropertyHandler::convertToControlValue( const ::rtl::OUString& _rPropertyName,
        const Any& _rPropertyValue, const Type& /*_rControlValueType*/ ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        Any aControlValue;

        OSL_ENSURE( m_pHelper.get(), "CellBindingPropertyHandler::convertToControlValue: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return aControlValue;

        PropertyId nPropId( m_pInfoService->getPropertyId( _rPropertyName ) );

        switch ( nPropId )
        {
            case PROPERTY_ID_BOUND_CELL:
            {
                Reference< XValueBinding > xBinding;
#if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
#endif
                _rPropertyValue >>= xBinding;
                OSL_ENSURE( bSuccess, "CellBindingPropertyHandler::convertToControlValue: invalid value (1)!" );

                // the only value binding we support so far is linking to spreadsheet cells
                aControlValue <<= m_pHelper->getStringAddressFromCellBinding( xBinding );
            }
            break;

            case PROPERTY_ID_LIST_CELL_RANGE:
            {
                Reference< XListEntrySource > xSource;
#if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
#endif
                _rPropertyValue >>= xSource;
                OSL_ENSURE( bSuccess, "CellBindingPropertyHandler::convertToControlValue: invalid value (2)!" );

                // the only value binding we support so far is linking to spreadsheet cells
                aControlValue <<= m_pHelper->getStringAddressFromCellListSource( xSource );
            }
            break;

            case PROPERTY_ID_CELL_EXCHANGE_TYPE:
                aControlValue <<= m_pCellExchangeConverter->getDescriptionForValue( _rPropertyValue );
                break;

            default:
                OSL_FAIL( "CellBindingPropertyHandler::convertToControlValue: cannot handle this!" );
                break;
        }

        return aControlValue;
    }

    //--------------------------------------------------------------------
    Sequence< Property > SAL_CALL CellBindingPropertyHandler::doDescribeSupportedProperties() const
    {
        ::std::vector< Property > aProperties;

        bool bAllowCellLinking      = m_pHelper.get() && m_pHelper->isCellBindingAllowed();
        bool bAllowCellIntLinking   = m_pHelper.get() && m_pHelper->isCellIntegerBindingAllowed();
        bool bAllowListCellRange    = m_pHelper.get() && m_pHelper->isListCellRangeAllowed();
        if ( bAllowCellLinking || bAllowListCellRange || bAllowCellIntLinking )
        {
            sal_Int32 nPos =  ( bAllowCellLinking    ? 1 : 0 )
                            + ( bAllowListCellRange  ? 1 : 0 )
                            + ( bAllowCellIntLinking ? 1 : 0 );
            aProperties.resize( nPos );

            if ( bAllowCellLinking )
            {
                aProperties[ --nPos ] = Property( PROPERTY_BOUND_CELL, PROPERTY_ID_BOUND_CELL,
                    ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ), 0 );
            }
            if ( bAllowCellIntLinking )
            {
                aProperties[ --nPos ] = Property( PROPERTY_CELL_EXCHANGE_TYPE, PROPERTY_ID_CELL_EXCHANGE_TYPE,
                    ::getCppuType( static_cast< sal_Int16* >( NULL ) ), 0 );
            }
            if ( bAllowListCellRange )
            {
                aProperties[ --nPos ] = Property( PROPERTY_LIST_CELL_RANGE, PROPERTY_ID_LIST_CELL_RANGE,
                    ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ), 0 );
            }
        }

        if ( aProperties.empty() )
            return Sequence< Property >();
        return Sequence< Property >( &(*aProperties.begin()), aProperties.size() );
    }

//........................................................................
}   // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
