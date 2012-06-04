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

#include <stdio.h>
#include <osl/diagnose.h>
#include <uno/threadpool.h>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <rtl/instance.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>

#include "thread.hxx"
#include "jobqueue.hxx"
#include "threadpool.hxx"

namespace {

namespace css = com::sun::star;

}

using namespace osl;
extern "C" {

void SAL_CALL cppu_requestThreadWorker( void *pVoid )
{
    ::cppu_threadpool::ORequestThread *pThread = ( ::cppu_threadpool::ORequestThread * ) pVoid;

    pThread->run();
    pThread->onTerminated();
}

}

namespace cppu_threadpool {

// ----------------------------------------------------------------------------------
    ThreadAdmin::ThreadAdmin(): m_disposed(false) {}

    ThreadAdmin::~ThreadAdmin()
    {
#if OSL_DEBUG_LEVEL > 1
        if( m_lst.size() )
        {
            fprintf( stderr, "%lu Threads left\n" , static_cast<unsigned long>(m_lst.size()) );
        }
#endif
    }

    void ThreadAdmin::add( ORequestThread *p )
    {
        MutexGuard aGuard( m_mutex );
        if( m_disposed )
        {
            throw css::lang::DisposedException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "cppu_threadpool::ORequestThread created after"
                        " cppu_threadpool::ThreadAdmin has been disposed")),
                css::uno::Reference< css::uno::XInterface >());
        }
        m_lst.push_back( p );
    }

    void ThreadAdmin::remove( ORequestThread * p )
    {
        MutexGuard aGuard( m_mutex );
        ::std::list< ORequestThread * >::iterator ii = ::std::find( m_lst.begin(), m_lst.end(), p );
        OSL_ASSERT( ii != m_lst.end() );
        m_lst.erase( ii );
    }

    void ThreadAdmin::join()
    {
        {
            MutexGuard aGuard( m_mutex );
            m_disposed = true;
        }
        ORequestThread *pCurrent;
        do
        {
            pCurrent = 0;
            {
                MutexGuard aGuard( m_mutex );
                if( ! m_lst.empty() )
                {
                    pCurrent = m_lst.front();
                    pCurrent->setDeleteSelf( sal_False );
                }
            }
            if ( pCurrent )
            {
                pCurrent->join();
                delete pCurrent;
            }
        } while( pCurrent );
    }

    struct theThreadAdmin : public rtl::StaticWithInit< ThreadAdminHolder, theThreadAdmin >
    {
        ThreadAdminHolder operator () () {
            ThreadAdminHolder aRet(new ThreadAdmin());
            return aRet;
        }
    };

    ThreadAdminHolder& ThreadAdmin::getInstance()
    {
        return theThreadAdmin::get();
    }

// ----------------------------------------------------------------------------------
    ORequestThread::ORequestThread( JobQueue *pQueue,
                                    const ByteSequence &aThreadId,
                                    sal_Bool bAsynchron )
        : m_thread( 0 )
        , m_aThreadAdmin( ThreadAdmin::getInstance() )
        , m_pQueue( pQueue )
        , m_aThreadId( aThreadId )
        , m_bAsynchron( bAsynchron )
        , m_bDeleteSelf( sal_True )
    {
        m_aThreadAdmin->add( this );
    }


    ORequestThread::~ORequestThread()
    {
        if (m_thread != 0)
        {
            osl_destroyThread(m_thread);
        }
    }


    void ORequestThread::setTask( JobQueue *pQueue,
                                  const ByteSequence &aThreadId,
                                  sal_Bool bAsynchron )
    {
        m_pQueue = pQueue;
        m_aThreadId = aThreadId;
        m_bAsynchron = bAsynchron;
    }

    sal_Bool ORequestThread::create()
    {
        OSL_ASSERT(m_thread == 0);  // only one running thread per instance

        m_thread = osl_createSuspendedThread( cppu_requestThreadWorker, (void*)this);
        if ( m_thread )
        {
            osl_resumeThread( m_thread );
        }

        return m_thread != 0;
    }

    void ORequestThread::join()
    {
        osl_joinWithThread( m_thread );
    }

    void ORequestThread::onTerminated()
    {
        m_aThreadAdmin->remove( this );
        if( m_bDeleteSelf )
        {
            delete this;
        }
    }

    void ORequestThread::run()
    {
        ThreadPoolHolder theThreadPool = cppu_threadpool::ThreadPool::getInstance();

        while ( m_pQueue )
        {
            if( ! m_bAsynchron )
            {
                if ( !uno_bindIdToCurrentThread( m_aThreadId.getHandle() ) )
                {
                    OSL_ASSERT( false );
                }
            }

            while( ! m_pQueue->isEmpty() )
            {
                // Note : Oneways should not get a disposable disposeid,
                //        It does not make sense to dispose a call in this state.
                //        That's way we put it an disposeid, that can't be used otherwise.
                m_pQueue->enter(
                    sal::static_int_cast< sal_Int64 >(
                        reinterpret_cast< sal_IntPtr >(this)),
                    sal_True );

                if( m_pQueue->isEmpty() )
                {
                    theThreadPool->revokeQueue( m_aThreadId , m_bAsynchron );
                    // Note : revokeQueue might have failed because m_pQueue.isEmpty()
                    //        may be false (race).
                }
            }

            delete m_pQueue;
            m_pQueue = 0;

            if( ! m_bAsynchron )
            {
                uno_releaseIdFromCurrentThread();
            }

            theThreadPool->waitInPool( this );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
