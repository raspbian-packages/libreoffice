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
#ifndef _FTP_FTPRESULTSETI_HXX_
#define _FTP_FTPRESULTSETI_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include "ftpresultsetbase.hxx"
#include "ftpdirp.hxx"


namespace ftp {

    class ResultSetI
        : public ResultSetBase
    {
    public:

        ResultSetI(
            const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory>& xMSF,
            const com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentProvider>& xProvider,
            sal_Int32 nOpenMode,
            const com::sun::star::uno::Sequence<
            com::sun::star::beans::Property >& seq,
            const com::sun::star::uno::Sequence<
            com::sun::star::ucb::NumberedSortingInfo >& seqSort,
            const std::vector<FTPDirentry>&  dirvec);

    private:
    };

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
