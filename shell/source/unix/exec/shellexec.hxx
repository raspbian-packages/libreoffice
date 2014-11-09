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

#ifndef INCLUDED_SHELL_SOURCE_UNIX_EXEC_SHELLEXEC_HXX
#define INCLUDED_SHELL_SOURCE_UNIX_EXEC_SHELLEXEC_HXX

#include <cppuhelper/implbase2.hxx>
#include <osl/mutex.hxx>
#include <rtl/strbuf.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <com/sun/star/system/XSystemShellExecute.hpp>


// class declaration


class ShellExec : public ::cppu::WeakImplHelper2< com::sun::star::system::XSystemShellExecute, com::sun::star::lang::XServiceInfo >
{
    OString m_aDesktopEnvironment;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
    m_xContext;

public:
    ShellExec(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext);


    // XSystemShellExecute


    virtual void SAL_CALL execute( const OUString& aCommand, const OUString& aParameter, sal_Int32 nFlags )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::system::SystemShellExecuteException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    // XServiceInfo


    virtual OUString SAL_CALL getImplementationName(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


// helper function - needed for urltest
void escapeForShell( OStringBuffer & rBuffer, const OString & rURL);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
