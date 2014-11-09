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
#ifndef INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBAPAGES_HXX
#define INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBAPAGES_HXX

#include <ooo/vba/office/MsoShapeType.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/msforms/XPages.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>

#include <vbahelper/vbacollectionimpl.hxx>
typedef CollTestImplHelper<
ov::msforms::XPages > ScVbaPages_BASE;

class ScVbaPages : public ScVbaPages_BASE
{
protected:
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
public:
    ScVbaPages( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xPages ) throw ( css::lang::IllegalArgumentException );
    virtual ~ScVbaPages() {}
        // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) SAL_OVERRIDE;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) SAL_OVERRIDE;
     // ScVbaPages_BASE
     virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) SAL_OVERRIDE;

};
#endif // INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBAPAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
