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





// INCLUDE ---------------------------------------------------------------
#include "xmloff/VisAreaContext.hxx"
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlimp.hxx>
#include <sax/tools/converter.hxx>
#include <tools/gen.hxx>

using namespace com::sun::star;
using namespace ::xmloff::token;

//------------------------------------------------------------------

XMLVisAreaContext::XMLVisAreaContext( SvXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            Rectangle& rRect,
                                            const sal_Int16 eMeasureUnit )
:
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    awt::Rectangle rAwtRect( rRect.getX(), rRect.getY(), rRect.getWidth(), rRect.getHeight() );
    process( xAttrList, rAwtRect, eMeasureUnit );

    rRect.setX( rAwtRect.X );
    rRect.setY( rAwtRect.Y );
    rRect.setWidth( rAwtRect.Width );
    rRect.setHeight( rAwtRect.Height );
}

XMLVisAreaContext::~XMLVisAreaContext()
{
}

void XMLVisAreaContext::process( const uno::Reference< xml::sax::XAttributeList>& xAttrList, awt::Rectangle& rRect, const sal_Int16 nMeasureUnit )
{
    sal_Int32 nX(0);
    sal_Int32 nY(0);
    sal_Int32 nWidth(0);
    sal_Int32 nHeight(0);
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_OFFICE)
        {
            if (IsXMLToken( aLocalName, XML_X ))
            {
                ::sax::Converter::convertMeasure(nX, sValue, nMeasureUnit);
                rRect.X = nX;
            }
            else if (IsXMLToken( aLocalName, XML_Y ))
            {
                ::sax::Converter::convertMeasure(nY, sValue, nMeasureUnit);
                rRect.Y = nY;
            }
            else if (IsXMLToken( aLocalName, XML_WIDTH ))
            {
                ::sax::Converter::convertMeasure(nWidth, sValue, nMeasureUnit);
                rRect.Width = nWidth;
            }
            else if (IsXMLToken( aLocalName, XML_HEIGHT ))
            {
                ::sax::Converter::convertMeasure(nHeight, sValue, nMeasureUnit);
                rRect.Height = nHeight;
            }
        }
    }
}

SvXMLImportContext *XMLVisAreaContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& )
{
    // here is no context
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void XMLVisAreaContext::EndElement()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
