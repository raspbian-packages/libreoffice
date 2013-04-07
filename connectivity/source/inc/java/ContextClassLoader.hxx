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

#ifndef CONNECTIVITY_CONTEXTCLASSLOADER_HXX
#define CONNECTIVITY_CONTEXTCLASSLOADER_HXX

#include "java/GlobalRef.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

namespace comphelper
{
    class ResourceBasedEventLogger;
}

//........................................................................
namespace connectivity { namespace jdbc
{
//........................................................................

    //====================================================================
    //= ContextClassLoaderScope
    //====================================================================
    /**
    */
    class ContextClassLoaderScope
    {
    public:
        /** creates the instance. If isActive returns <FALSE/> afterwards, then an exception
            happend in the JVM, which should be raised as UNO exception by the caller

            @param  environment
                the current JNI environment
            @param  newClassLoader
                the new class loader to set at the current thread
            @param  _rLoggerForErrors
                the logger which should be passed to java_lang_object::ThrowLoggedSQLException in case
                an error occurs
            @param  _rxErrorContext
                the context which should be passed to java_lang_object::ThrowLoggedSQLException in case
                an error occurs

        */
        ContextClassLoaderScope(
            JNIEnv& environment,
            const GlobalRef< jobject >& newClassLoader,
            const ::comphelper::ResourceBasedEventLogger& _rLoggerForErrors,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxErrorContext
        );

        ~ContextClassLoaderScope() { pop(true); }

        void pop() { pop(false); }

        bool isActive() const
        {
            return  ( m_currentThread.is() )
                &&  ( m_setContextClassLoaderMethod != NULL );
        }

    private:
        ContextClassLoaderScope(ContextClassLoaderScope &); // not defined
        void operator =(ContextClassLoaderScope &); // not defined

        void pop( bool clearExceptions );

        JNIEnv&                             m_environment;
        LocalRef< jobject >                 m_currentThread;
        LocalRef< jobject >                 m_oldContextClassLoader;
        jmethodID                           m_setContextClassLoaderMethod;
    };


//........................................................................
} } // namespace connectivity::jdbc
//........................................................................

#endif // CONNECTIVITY_CONTEXTCLASSLOADER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
