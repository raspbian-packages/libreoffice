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


#include "ostreamcontainer.hxx"


using namespace ::com::sun::star;

//-----------------------------------------------
OFSStreamContainer::OFSStreamContainer( const uno::Reference < io::XStream >& xStream )
: m_bDisposed( sal_False )
, m_bInputClosed( sal_False )
, m_bOutputClosed( sal_False )
, m_pListenersContainer( NULL )
, m_pTypeCollection( NULL )
{
    try
    {
        m_xStream = xStream;
        if ( !m_xStream.is() )
            throw uno::RuntimeException();

        m_xSeekable = uno::Reference< io::XSeekable >( xStream, uno::UNO_QUERY );
        m_xInputStream = xStream->getInputStream();
        m_xOutputStream = xStream->getOutputStream();
        m_xTruncate = uno::Reference< io::XTruncate >( m_xOutputStream, uno::UNO_QUERY );
        m_xAsyncOutputMonitor = uno::Reference< io::XAsyncOutputMonitor >( m_xOutputStream, uno::UNO_QUERY );
    }
    catch( uno::Exception& )
    {
        m_xStream = uno::Reference< io::XStream >();
        m_xSeekable = uno::Reference< io::XSeekable >();
        m_xInputStream = uno::Reference< io::XInputStream >();
        m_xOutputStream = uno::Reference< io::XOutputStream >();
        m_xTruncate = uno::Reference< io::XTruncate >();
        m_xAsyncOutputMonitor = uno::Reference< io::XAsyncOutputMonitor >();
    }
}

//-----------------------------------------------
OFSStreamContainer::~OFSStreamContainer()
{
    if ( m_pListenersContainer )
    {
        delete m_pListenersContainer;
        m_pListenersContainer = NULL;
    }
}

// XInterface
//-----------------------------------------------
uno::Any SAL_CALL OFSStreamContainer::queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException )
{
    uno::Any aReturn;

    aReturn <<= ::cppu::queryInterface
                (   rType
                    ,   static_cast<lang::XTypeProvider*> ( this )
                    ,   static_cast<io::XStream*> ( this )
                    ,   static_cast<embed::XExtendedStorageStream*> ( this )
                    ,   static_cast<lang::XComponent*> ( this ) );

    if ( aReturn.hasValue() == sal_True )
        return aReturn ;

    if ( m_xSeekable.is() )
    {
        aReturn <<= ::cppu::queryInterface
                (   rType
                    ,   static_cast<io::XSeekable*> ( this ) );

        if ( aReturn.hasValue() == sal_True )
            return aReturn ;
    }

    if ( m_xInputStream.is() )
    {
        aReturn <<= ::cppu::queryInterface
                (   rType
                    ,   static_cast<io::XInputStream*> ( this ) );

        if ( aReturn.hasValue() == sal_True )
            return aReturn ;
    }
    if ( m_xOutputStream.is() )
    {
        aReturn <<= ::cppu::queryInterface
                (   rType
                    ,   static_cast<io::XOutputStream*> ( this ) );

        if ( aReturn.hasValue() == sal_True )
            return aReturn ;
    }
    if ( m_xTruncate.is() )
    {
        aReturn <<= ::cppu::queryInterface
                (   rType
                    ,   static_cast<io::XTruncate*> ( this ) );

        if ( aReturn.hasValue() == sal_True )
            return aReturn ;
    }
    if ( m_xAsyncOutputMonitor.is() )
    {
        aReturn <<= ::cppu::queryInterface
                (   rType
                    ,   static_cast<io::XAsyncOutputMonitor*> ( this ) );

        if ( aReturn.hasValue() == sal_True )
            return aReturn ;
    }

    return OWeakObject::queryInterface( rType );
}

//-----------------------------------------------
void SAL_CALL OFSStreamContainer::acquire()
        throw()
{
    OWeakObject::acquire();
}

//-----------------------------------------------
void SAL_CALL OFSStreamContainer::release()
        throw()
{
    OWeakObject::release();
}

//  XTypeProvider
//-----------------------------------------------
uno::Sequence< uno::Type > SAL_CALL OFSStreamContainer::getTypes()
        throw( uno::RuntimeException )
{
    if ( m_pTypeCollection == NULL )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_pTypeCollection == NULL )
        {
            ::cppu::OTypeCollection aTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< lang::XTypeProvider >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XExtendedStorageStream >* )NULL ) );

            if ( m_xSeekable.is() )
                aTypeCollection = ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< io::XSeekable >* )NULL ),
                                        aTypeCollection.getTypes() );
            if ( m_xInputStream.is() )
                aTypeCollection = ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< io::XInputStream >* )NULL ),
                                        aTypeCollection.getTypes() );

            if ( m_xOutputStream.is() )
                aTypeCollection = ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< io::XOutputStream >* )NULL ),
                                        aTypeCollection.getTypes() );
            if ( m_xTruncate.is() )
                aTypeCollection = ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< io::XTruncate >* )NULL ),
                                        aTypeCollection.getTypes() );
            if ( m_xAsyncOutputMonitor.is() )
                aTypeCollection = ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< io::XAsyncOutputMonitor >* )NULL ),
                                        aTypeCollection.getTypes() );

            m_pTypeCollection = new ::cppu::OTypeCollection( aTypeCollection );
        }
    }
    return m_pTypeCollection->getTypes() ;
}

//-----------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL OFSStreamContainer::getImplementationId()
        throw( uno::RuntimeException )
{
    static ::cppu::OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() ) ;

        if ( pID == NULL )
        {
            static ::cppu::OImplementationId aID( sal_False ) ;
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

// XStream
//-----------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OFSStreamContainer::getInputStream()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() )
        throw uno::RuntimeException();

    if ( m_xInputStream.is() )
        return uno::Reference< io::XInputStream >( static_cast< io::XInputStream* >( this ) );

    return uno::Reference< io::XInputStream >();
}

//-----------------------------------------------
uno::Reference< io::XOutputStream > SAL_CALL OFSStreamContainer::getOutputStream()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() )
        throw uno::RuntimeException();

    if ( m_xOutputStream.is() )
        return uno::Reference< io::XOutputStream >( static_cast< io::XOutputStream* >( this ) );

    return uno::Reference< io::XOutputStream >();
}

// XComponent
//-----------------------------------------------
void SAL_CALL OFSStreamContainer::dispose()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() )
        throw uno::RuntimeException();

    if ( m_xInputStream.is() && !m_bInputClosed )
    {
        m_xInputStream->closeInput();
        m_bInputClosed = sal_True;
    }

    if ( m_xOutputStream.is() && !m_bOutputClosed )
    {
        m_xOutputStream->closeOutput();
        m_bOutputClosed = sal_True;
    }

    if ( m_pListenersContainer )
    {
        lang::EventObject aSource( static_cast< ::cppu::OWeakObject*>( this ) );
        m_pListenersContainer->disposeAndClear( aSource );
    }

    m_bDisposed = sal_True;
}

//-----------------------------------------------
void SAL_CALL OFSStreamContainer::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_pListenersContainer )
        m_pListenersContainer = new ::cppu::OInterfaceContainerHelper( m_aMutex );

    m_pListenersContainer->addInterface( xListener );
}

//-----------------------------------------------
void SAL_CALL OFSStreamContainer::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( m_pListenersContainer )
        m_pListenersContainer->removeInterface( xListener );
}


// XSeekable
//-----------------------------------------------
void SAL_CALL OFSStreamContainer::seek( sal_Int64 location )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() || !m_xSeekable.is() )
        throw uno::RuntimeException();

    m_xSeekable->seek( location );
}

//-----------------------------------------------
sal_Int64 SAL_CALL OFSStreamContainer::getPosition()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() || !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getPosition();
}

//-----------------------------------------------
sal_Int64 SAL_CALL OFSStreamContainer::getLength()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() || !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getLength();
}


// XInputStream
//-----------------------------------------------
sal_Int32 SAL_CALL OFSStreamContainer::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() || !m_xInputStream.is() )
        throw uno::RuntimeException();

    return m_xInputStream->readBytes( aData, nBytesToRead );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OFSStreamContainer::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() || !m_xInputStream.is() )
        throw uno::RuntimeException();

    return m_xInputStream->readSomeBytes( aData, nMaxBytesToRead );
}

//-----------------------------------------------
void SAL_CALL OFSStreamContainer::skipBytes( sal_Int32 nBytesToSkip )
        throw( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() || !m_xInputStream.is() )
        throw uno::RuntimeException();

    m_xInputStream->skipBytes( nBytesToSkip );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OFSStreamContainer::available()
        throw( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() || !m_xInputStream.is() )
        throw uno::RuntimeException();

    return m_xInputStream->available();
}

//-----------------------------------------------
void SAL_CALL OFSStreamContainer::closeInput()
        throw( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() || !m_xInputStream.is() )
        throw uno::RuntimeException();

    if ( m_xInputStream.is() )
    {
        m_xInputStream->closeInput();
        m_bInputClosed = sal_True;
    }

    if ( m_bOutputClosed )
        dispose();
}

// XOutputStream
//-----------------------------------------------
void SAL_CALL OFSStreamContainer::writeBytes( const uno::Sequence< sal_Int8 >& aData )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() || !m_xOutputStream.is() )
        throw uno::RuntimeException();

    return m_xOutputStream->writeBytes( aData );
}

//-----------------------------------------------
void SAL_CALL OFSStreamContainer::flush()
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() || !m_xOutputStream.is() )
        throw uno::RuntimeException();

    return m_xOutputStream->flush();
}

//-----------------------------------------------
void SAL_CALL OFSStreamContainer::closeOutput()
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() || !m_xOutputStream.is() )
        throw uno::RuntimeException();

    if ( m_xOutputStream.is() )
    {
        m_xOutputStream->closeOutput();
        m_bOutputClosed = sal_True;
    }

    if ( m_bInputClosed )
        dispose();
}


// XTruncate
//-----------------------------------------------
void SAL_CALL OFSStreamContainer::truncate()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() || !m_xTruncate.is() )
        throw uno::RuntimeException();

    m_xTruncate->truncate();
}


// XAsyncOutputMonitor
//-----------------------------------------------
void SAL_CALL OFSStreamContainer::waitForCompletion()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xStream.is() || !m_xAsyncOutputMonitor.is() )
        throw uno::RuntimeException();

    m_xAsyncOutputMonitor->waitForCompletion();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
