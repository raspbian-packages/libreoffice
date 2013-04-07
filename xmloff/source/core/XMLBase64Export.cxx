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


#include <rtl/ustrbuf.hxx>

#include <com/sun/star/io/XInputStream.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/xmlexp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include "XMLBase64Export.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

#define INPUT_BUFFER_SIZE 54
#define OUTPUT_BUFFER_SIZE 72

XMLBase64Export::XMLBase64Export( SvXMLExport& rExp ) :
    rExport( rExp ){
}

sal_Bool XMLBase64Export::exportXML( const Reference < XInputStream> & rIn )
{
    sal_Bool bRet = sal_True;
    try
    {
        Sequence < sal_Int8 > aInBuff( INPUT_BUFFER_SIZE );
        OUStringBuffer aOutBuff( OUTPUT_BUFFER_SIZE );
        sal_Int32 nRead;
        do
        {
            nRead = rIn->readBytes( aInBuff, INPUT_BUFFER_SIZE );
            if( nRead > 0 )
            {
				::sax::Converter::encodeBase64( aOutBuff, aInBuff );
                GetExport().Characters( aOutBuff.makeStringAndClear() );
                if( nRead == INPUT_BUFFER_SIZE )
                    GetExport().IgnorableWhitespace();
            }
        }
        while( nRead == INPUT_BUFFER_SIZE );
    }
    catch( ... )
    {
        bRet = sal_False;
    }

    return bRet;
}

sal_Bool XMLBase64Export::exportElement(
            const Reference < XInputStream > & rIn,
            sal_uInt16 nNamespace,
            enum ::xmloff::token::XMLTokenEnum eName )
{
    SvXMLElementExport aElem( GetExport(), nNamespace, eName, sal_True,
                              sal_True );
    return exportXML( rIn );
}

sal_Bool XMLBase64Export::exportOfficeBinaryDataElement(
            const Reference < XInputStream > & rIn )
{
    return exportElement( rIn, XML_NAMESPACE_OFFICE,
                             ::xmloff::token::XML_BINARY_DATA );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
