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
************************************************************************/

#ifndef INCLUDED_TEST_OFFICECONNECTION_HXX
#define INCLUDED_TEST_OFFICECONNECTION_HXX

#include "sal/config.h"

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "osl/process.h"
#include "unotest/detail/unotestdllapi.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace test {

// Start up and shut down an OOo instance (details about the OOo instance are
// tunneled in via "arg-..." bootstrap variables):
class OOO_DLLPUBLIC_UNOTEST OfficeConnection: private boost::noncopyable {
public:
    OfficeConnection();

    ~OfficeConnection();

    void setUp();

    void tearDown();

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
    getComponentContext() const;

    // Must not be called before setUp or after tearDown:
    bool isStillAlive() const;

private:
    oslProcess process_;
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
        context_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
