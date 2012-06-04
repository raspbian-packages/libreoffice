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

#ifndef _SVX_UNOFORBIDDENCHARSTABLE_HXX_
#define _SVX_UNOFORBIDDENCHARSTABLE_HXX_

#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>

#include <rtl/ref.hxx>

#include <cppuhelper/implbase2.hxx>
namespace binfilter {

class SvxForbiddenCharactersTable;

class SvxUnoForbiddenCharsTable : public cppu::WeakImplHelper2<
                                        ::com::sun::star::i18n::XForbiddenCharacters,
                                        ::com::sun::star::linguistic2::XSupportedLocales>
{
protected:
    /** this virtual function is called if the forbidden characters are changed */
    virtual void onChange();

    rtl::Reference<SvxForbiddenCharactersTable> mxForbiddenChars;

public:
    SvxUnoForbiddenCharsTable(rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars);
    ~SvxUnoForbiddenCharsTable();

    // XForbiddenCharacters
    virtual ::com::sun::star::i18n::ForbiddenCharacters SAL_CALL getForbiddenCharacters( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasForbiddenCharacters( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setForbiddenCharacters( const ::com::sun::star::lang::Locale& rLocale, const ::com::sun::star::i18n::ForbiddenCharacters& rForbiddenCharacters ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeForbiddenCharacters( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);

    // XSupportedLocales
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getLocales(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasLocale( const ::com::sun::star::lang::Locale& aLocale ) throw (::com::sun::star::uno::RuntimeException);
};

}//end of namespace binfilter
#endif // _SVX_UNOFORBIDDENCHARSTABLE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
