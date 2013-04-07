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

#ifndef OOX_DRAWINGML_SHAPECONTEXT_HXX
#define OOX_DRAWINGML_SHAPECONTEXT_HXX

#include <com/sun/star/drawing/XShapes.hpp>

#include "oox/core/contexthandler.hxx"
#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/shapepropertiescontext.hxx"

namespace oox { namespace drawingml {

class ShapeContext : public ::oox::core::ContextHandler
{
public:
    ShapeContext( ::oox::core::ContextHandler& rParent, ShapePtr pMasterShapePtr, ShapePtr pShapePtr );
    virtual ~ShapeContext();

    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

    ShapePtr getShape();

protected:

    ShapePtr mpMasterShapePtr;
    ShapePtr mpShapePtr;
};

} }

#endif  //  OOX_DRAWINGML_SHAPEGROUPCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
