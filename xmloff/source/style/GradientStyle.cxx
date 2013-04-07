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


#include "xmloff/GradientStyle.hxx"

#include <com/sun/star/awt/Gradient.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>



using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::xmloff::token;

enum SvXMLTokenMapAttrs
{
    XML_TOK_GRADIENT_NAME,
    XML_TOK_GRADIENT_DISPLAY_NAME,
    XML_TOK_GRADIENT_STYLE,
    XML_TOK_GRADIENT_CX,
    XML_TOK_GRADIENT_CY,
    XML_TOK_GRADIENT_STARTCOLOR,
    XML_TOK_GRADIENT_ENDCOLOR,
    XML_TOK_GRADIENT_STARTINT,
    XML_TOK_GRADIENT_ENDINT,
    XML_TOK_GRADIENT_ANGLE,
    XML_TOK_GRADIENT_BORDER,
    XML_TOK_TABSTOP_END=XML_TOK_UNKNOWN
};


SvXMLEnumMapEntry const pXML_GradientStyle_Enum[] =
{
    { XML_GRADIENTSTYLE_LINEAR,         awt::GradientStyle_LINEAR },
    { XML_GRADIENTSTYLE_AXIAL,          awt::GradientStyle_AXIAL },
    { XML_GRADIENTSTYLE_RADIAL,         awt::GradientStyle_RADIAL },
    { XML_GRADIENTSTYLE_ELLIPSOID,      awt::GradientStyle_ELLIPTICAL },
    { XML_GRADIENTSTYLE_SQUARE,         awt::GradientStyle_SQUARE },
    { XML_GRADIENTSTYLE_RECTANGULAR,    awt::GradientStyle_RECT },
    { XML_TOKEN_INVALID, 0 }
};

//-------------------------------------------------------------
// Import
//-------------------------------------------------------------
XMLGradientStyleImport::XMLGradientStyleImport(
    SvXMLImport& rImp )
    : rImport(rImp)
{
}

XMLGradientStyleImport::~XMLGradientStyleImport()
{
}

sal_Bool XMLGradientStyleImport::importXML(
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    uno::Any& rValue,
    OUString& rStrName )
{
    sal_Bool bRet           = sal_False;
    sal_Bool bHasName       = sal_False;
    sal_Bool bHasStyle      = sal_False;
    sal_Bool bHasStartColor = sal_False;
    sal_Bool bHasEndColor   = sal_False;
    OUString aDisplayName;

    awt::Gradient aGradient;
    aGradient.XOffset = 0;
    aGradient.YOffset = 0;
    aGradient.StartIntensity = 100;
    aGradient.EndIntensity = 100;
    aGradient.Angle = 0;
    aGradient.Border = 0;

    {
        static SvXMLTokenMapEntry aGradientAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW, XML_NAME, XML_TOK_GRADIENT_NAME },
    { XML_NAMESPACE_DRAW, XML_DISPLAY_NAME, XML_TOK_GRADIENT_DISPLAY_NAME },
    { XML_NAMESPACE_DRAW, XML_STYLE, XML_TOK_GRADIENT_STYLE },
    { XML_NAMESPACE_DRAW, XML_CX, XML_TOK_GRADIENT_CX },
    { XML_NAMESPACE_DRAW, XML_CY, XML_TOK_GRADIENT_CY },
    { XML_NAMESPACE_DRAW, XML_START_COLOR, XML_TOK_GRADIENT_STARTCOLOR },
    { XML_NAMESPACE_DRAW, XML_END_COLOR, XML_TOK_GRADIENT_ENDCOLOR },
    { XML_NAMESPACE_DRAW, XML_START_INTENSITY, XML_TOK_GRADIENT_STARTINT },
    { XML_NAMESPACE_DRAW, XML_END_INTENSITY, XML_TOK_GRADIENT_ENDINT },
    { XML_NAMESPACE_DRAW, XML_GRADIENT_ANGLE, XML_TOK_GRADIENT_ANGLE },
    { XML_NAMESPACE_DRAW, XML_GRADIENT_BORDER, XML_TOK_GRADIENT_BORDER },
    XML_TOKEN_MAP_END
};

    SvXMLTokenMap aTokenMap( aGradientAttrTokenMap );
    SvXMLNamespaceMap& rNamespaceMap = rImport.GetNamespaceMap();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
        OUString aStrAttrName;
        sal_uInt16 nPrefix = rNamespaceMap.GetKeyByAttrName( rFullAttrName, &aStrAttrName );
        const OUString& rStrValue = xAttrList->getValueByIndex( i );

        sal_Int32 nTmpValue;

        switch( aTokenMap.Get( nPrefix, aStrAttrName ) )
        {
        case XML_TOK_GRADIENT_NAME:
            {
                rStrName = rStrValue;
                bHasName = sal_True;
            }
            break;
        case XML_TOK_GRADIENT_DISPLAY_NAME:
            {
                aDisplayName = rStrValue;
            }
            break;
        case XML_TOK_GRADIENT_STYLE:
            {
                sal_uInt16 eValue;
                if( SvXMLUnitConverter::convertEnum( eValue, rStrValue, pXML_GradientStyle_Enum ) )
                {
                    aGradient.Style = (awt::GradientStyle) eValue;
                    bHasStyle = sal_True;
                }
            }
            break;
        case XML_TOK_GRADIENT_CX:
            ::sax::Converter::convertPercent( nTmpValue, rStrValue );
            aGradient.XOffset = static_cast< sal_Int16 >( nTmpValue );
            break;
        case XML_TOK_GRADIENT_CY:
            ::sax::Converter::convertPercent( nTmpValue, rStrValue );
            aGradient.YOffset = static_cast< sal_Int16 >( nTmpValue );
            break;
        case XML_TOK_GRADIENT_STARTCOLOR:
            {
                bHasStartColor = ::sax::Converter::convertColor(
                        aGradient.StartColor, rStrValue);
            }
            break;
        case XML_TOK_GRADIENT_ENDCOLOR:
            {
                bHasStartColor = ::sax::Converter::convertColor(
                        aGradient.EndColor, rStrValue);
            }
            break;
        case XML_TOK_GRADIENT_STARTINT:
            ::sax::Converter::convertPercent( nTmpValue, rStrValue );
            aGradient.StartIntensity = static_cast< sal_Int16 >( nTmpValue );
            break;
        case XML_TOK_GRADIENT_ENDINT:
            ::sax::Converter::convertPercent( nTmpValue, rStrValue );
            aGradient.EndIntensity = static_cast< sal_Int16 >( nTmpValue );
            break;
        case XML_TOK_GRADIENT_ANGLE:
            {
                sal_Int32 nValue;
                ::sax::Converter::convertNumber( nValue, rStrValue, 0, 3600 );
                aGradient.Angle = sal_Int16( nValue );
            }
            break;
        case XML_TOK_GRADIENT_BORDER:
            ::sax::Converter::convertPercent( nTmpValue, rStrValue );
            aGradient.Border = static_cast< sal_Int16 >( nTmpValue );
            break;

        default:
            DBG_WARNING( "Unknown token at import gradient style" )
            ;
        }
    }

    rValue <<= aGradient;

    if( aDisplayName.getLength() )
    {
        rImport.AddStyleDisplayName( XML_STYLE_FAMILY_SD_GRADIENT_ID, rStrName,
                                     aDisplayName );
        rStrName = aDisplayName;
    }

    bRet = bHasName && bHasStyle && bHasStartColor && bHasEndColor;

    }

    return bRet;
}


//-------------------------------------------------------------
// Export
//-------------------------------------------------------------

XMLGradientStyleExport::XMLGradientStyleExport(
    SvXMLExport& rExp )
    : rExport(rExp)
{
}

XMLGradientStyleExport::~XMLGradientStyleExport()
{
}

sal_Bool XMLGradientStyleExport::exportXML(
    const OUString& rStrName,
    const uno::Any& rValue )
{
    sal_Bool bRet = sal_False;
    awt::Gradient aGradient;

    if( rStrName.getLength() )
    {
        if( rValue >>= aGradient )
        {
            OUString aStrValue;
            OUStringBuffer aOut;

            // Style
            if( !SvXMLUnitConverter::convertEnum( aOut, aGradient.Style, pXML_GradientStyle_Enum ) )
            {
                bRet = sal_False;
            }
            else
            {
                // Name
                sal_Bool bEncoded = sal_False;
                OUString aStrName( rStrName );
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME,
                                      rExport.EncodeStyleName( aStrName,
                                                                &bEncoded ) );
                if( bEncoded )
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY_NAME,
                                            aStrName );


                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE, aStrValue );

                // Center x/y
                if( aGradient.Style != awt::GradientStyle_LINEAR &&
                    aGradient.Style != awt::GradientStyle_AXIAL   )
                {
                    ::sax::Converter::convertPercent(aOut, aGradient.XOffset);
                    aStrValue = aOut.makeStringAndClear();
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CX, aStrValue );
                    ::sax::Converter::convertPercent(aOut, aGradient.YOffset);
                    aStrValue = aOut.makeStringAndClear();
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CY, aStrValue );
                }

                // Color start
                ::sax::Converter::convertColor(aOut, aGradient.StartColor);
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_START_COLOR, aStrValue );

                // Color end
                ::sax::Converter::convertColor(aOut, aGradient.EndColor);
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_END_COLOR, aStrValue );

                // Intensity start
                ::sax::Converter::convertPercent(aOut, aGradient.StartIntensity);
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_START_INTENSITY, aStrValue );

                // Intensity end
                ::sax::Converter::convertPercent(aOut, aGradient.EndIntensity);
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_END_INTENSITY, aStrValue );

                // Angle
                if( aGradient.Style != awt::GradientStyle_RADIAL )
                {
                    ::sax::Converter::convertNumber(aOut, sal_Int32(aGradient.Angle));
                    aStrValue = aOut.makeStringAndClear();
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_GRADIENT_ANGLE, aStrValue );
                }

                // Border
                ::sax::Converter::convertPercent( aOut, aGradient.Border );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_GRADIENT_BORDER, aStrValue );

                // Do Write
                SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_GRADIENT,
                                      sal_True, sal_False );
            }
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
