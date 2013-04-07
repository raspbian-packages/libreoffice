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


#include <svl/inetdef.hxx>
#include "svtools/htmlkywd.hxx"

#include <rtl/tencinfo.h>

#include <unotools/configmgr.hxx>
#include "svl/urihelper.hxx"
#include <tools/datetime.hxx>

#include <sfx2/frmhtmlw.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/sfx.hrc>
#include "bastyp.hrc"

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>


// -----------------------------------------------------------------------

using namespace ::com::sun::star;

static sal_Char const sHTML_SC_yes[] =  "YES";
static sal_Char const sHTML_SC_no[] =       "NO";
static sal_Char const sHTML_SC_auto[] = "AUTO";
static sal_Char const sHTML_MIME_text_html[] =  "text/html; charset=";

#if defined(UNX)
const sal_Char SfxFrameHTMLWriter::sNewLine[] = "\012";
#else
const sal_Char SfxFrameHTMLWriter::sNewLine[] = "\015\012";
#endif

void SfxFrameHTMLWriter::OutMeta( SvStream& rStrm,
                                  const sal_Char *pIndent,
                                  const String& rName,
                                  const String& rContent, sal_Bool bHTTPEquiv,
                                     rtl_TextEncoding eDestEnc,
                                  String *pNonConvertableChars  )
{
    rStrm << sNewLine;
    if( pIndent )
        rStrm << pIndent;

    rtl::OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_meta).append(' ')
        .append(bHTTPEquiv ? OOO_STRING_SVTOOLS_HTML_O_httpequiv : OOO_STRING_SVTOOLS_HTML_O_name).append("=\"");
    rStrm << sOut.makeStringAndClear().getStr();

    HTMLOutFuncs::Out_String( rStrm, rName, eDestEnc, pNonConvertableChars );

    sOut.append("\" ").append(OOO_STRING_SVTOOLS_HTML_O_content).append("=\"");
    rStrm << sOut.makeStringAndClear().getStr();

    HTMLOutFuncs::Out_String( rStrm, rContent, eDestEnc, pNonConvertableChars ) << "\">";
}

void SfxFrameHTMLWriter::Out_DocInfo( SvStream& rStrm, const String& rBaseURL,
        const uno::Reference<document::XDocumentProperties> & i_xDocProps,
        const sal_Char *pIndent,
        rtl_TextEncoding eDestEnc,
        String *pNonConvertableChars    )
{
    const sal_Char *pCharSet =
                rtl_getBestMimeCharsetFromTextEncoding( eDestEnc );

    if( pCharSet )
    {
        String aContentType = String::CreateFromAscii( sHTML_MIME_text_html );
        aContentType.AppendAscii( pCharSet );
        OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_content_type, aContentType, sal_True,
                 eDestEnc, pNonConvertableChars );
    }

    // Titel (auch wenn er leer ist)
    rStrm << sNewLine;
    if( pIndent )
        rStrm << pIndent;
    HTMLOutFuncs::Out_AsciiTag( rStrm, OOO_STRING_SVTOOLS_HTML_title );
    if( i_xDocProps.is() )
    {
        const String& rTitle = i_xDocProps->getTitle();
        if( rTitle.Len() )
            HTMLOutFuncs::Out_String( rStrm, rTitle, eDestEnc, pNonConvertableChars );
    }
    HTMLOutFuncs::Out_AsciiTag( rStrm, OOO_STRING_SVTOOLS_HTML_title, sal_False );

    // Target-Frame
    if( i_xDocProps.is() )
    {
        const String& rTarget = i_xDocProps->getDefaultTarget();
        if( rTarget.Len() )
        {
            rStrm << sNewLine;
            if( pIndent )
                rStrm << pIndent;

            rtl::OStringBuffer sOut;
            sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_base).append(' ')
                .append(OOO_STRING_SVTOOLS_HTML_O_target).append("=\"");
            rStrm << sOut.makeStringAndClear().getStr();
            HTMLOutFuncs::Out_String( rStrm, rTarget, eDestEnc, pNonConvertableChars )
                << "\">";
        }
    }

    // Who we are
    String sGenerator( SfxResId( STR_HTML_GENERATOR ) );
    ::rtl::OUString os( RTL_CONSTASCII_USTRINGPARAM("$_OS") );
    ::rtl::Bootstrap::expandMacros(os);
    sGenerator.SearchAndReplaceAscii( "%1", os );
    OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_generator, sGenerator, sal_False, eDestEnc, pNonConvertableChars );

    if( i_xDocProps.is() )
    {
        // Reload
        if( (i_xDocProps->getAutoloadSecs() != 0) ||
            i_xDocProps->getAutoloadURL().getLength() )
        {
            String sContent = String::CreateFromInt32(
                                i_xDocProps->getAutoloadSecs() );

            const String &rReloadURL = i_xDocProps->getAutoloadURL();
            if( rReloadURL.Len() )
            {
                sContent.AppendAscii( ";URL=" );
                sContent += String(
                    URIHelper::simpleNormalizedMakeRelative(
                        rBaseURL, rReloadURL));
            }

            OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_refresh, sContent, sal_True,
                     eDestEnc, pNonConvertableChars );
        }

        // Author
        const String& rAuthor = i_xDocProps->getAuthor();
        if( rAuthor.Len() )
            OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_author, rAuthor, sal_False,
                     eDestEnc, pNonConvertableChars );

        // created
        ::util::DateTime uDT = i_xDocProps->getCreationDate();
        Date aD(uDT.Day, uDT.Month, uDT.Year);
        Time aT(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.HundredthSeconds);
        String sOut = String::CreateFromInt32(aD.GetDate());
        sOut += ';';
        sOut += String::CreateFromInt32(aT.GetTime());
        OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_created, sOut, sal_False,
                 eDestEnc, pNonConvertableChars );

        // changedby
        const String& rChangedBy = i_xDocProps->getModifiedBy();
        if( rChangedBy.Len() )
            OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_changedby, rChangedBy, sal_False,
                     eDestEnc, pNonConvertableChars );

        // changed
        uDT = i_xDocProps->getModificationDate();
        Date aD2(uDT.Day, uDT.Month, uDT.Year);
        Time aT2(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.HundredthSeconds);
        sOut = String::CreateFromInt32(aD2.GetDate());
        sOut += ';';
        sOut += String::CreateFromInt32(aT2.GetTime());
        OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_changed, sOut, sal_False,
                 eDestEnc, pNonConvertableChars );

        // Subject
        const String& rTheme = i_xDocProps->getSubject();
        if( rTheme.Len() )
            OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_classification, rTheme, sal_False,
                     eDestEnc, pNonConvertableChars );

        // Description
        const String& rComment = i_xDocProps->getDescription();
        if( rComment.Len() )
            OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_description, rComment, sal_False,
                     eDestEnc, pNonConvertableChars);

        // Keywords
        String Keywords = ::comphelper::string::convertCommaSeparated(
            i_xDocProps->getKeywords());
        if( Keywords.Len() )
            OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_keywords, Keywords, sal_False,
                     eDestEnc, pNonConvertableChars);

        uno::Reference < script::XTypeConverter > xConverter(
            ::comphelper::getProcessServiceFactory()->createInstance(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter"))),
                uno::UNO_QUERY_THROW );
        uno::Reference<beans::XPropertySet> xUserDefinedProps(
            i_xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
        DBG_ASSERT(xUserDefinedProps.is(), "UserDefinedProperties is null");
        uno::Reference<beans::XPropertySetInfo> xPropInfo =
            xUserDefinedProps->getPropertySetInfo();
        DBG_ASSERT(xPropInfo.is(), "UserDefinedProperties Info is null");
        uno::Sequence<beans::Property> props = xPropInfo->getProperties();
        for (sal_Int32 i = 0; i < props.getLength(); ++i)
        {
            try
            {
                ::rtl::OUString name = props[i].Name;
                uno::Any aStr = xConverter->convertToSimpleType(
                        xUserDefinedProps->getPropertyValue(name),
                        uno::TypeClass_STRING);
                ::rtl::OUString str;
                aStr >>= str;
                String valstr(comphelper::string::stripEnd(str, ' '));
                OutMeta( rStrm, pIndent, name, valstr, sal_False,
                         eDestEnc, pNonConvertableChars );
            }
            catch (const uno::Exception&)
            {
                // may happen with concurrent modification...
                DBG_WARNING("SfxFrameHTMLWriter::Out_DocInfo: exception");
            }
        }
    }
}

void SfxFrameHTMLWriter::Out_FrameDescriptor(
    SvStream& rOut, const String& rBaseURL, const uno::Reference < beans::XPropertySet >& xSet,
    rtl_TextEncoding eDestEnc, String *pNonConvertableChars )
{
    try
    {
        rtl::OStringBuffer sOut;
        ::rtl::OUString aStr;
        uno::Any aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameURL")) );
        if ( (aAny >>= aStr) && aStr.getLength() )
        {
            String aURL = INetURLObject( aStr ).GetMainURL( INetURLObject::DECODE_TO_IURI );
            if( aURL.Len() )
            {
                aURL = URIHelper::simpleNormalizedMakeRelative(
                    rBaseURL, aURL );
                sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_src)
                    .append(RTL_CONSTASCII_STRINGPARAM("=\""));
                rOut << sOut.makeStringAndClear().getStr();
                HTMLOutFuncs::Out_String( rOut, aURL, eDestEnc, pNonConvertableChars );
                sOut.append('\"');
            }
        }

        aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameName")) );
        if ( (aAny >>= aStr) && aStr.getLength() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_name)
                .append(RTL_CONSTASCII_STRINGPARAM("=\""));
            rOut << sOut.makeStringAndClear().getStr();
            HTMLOutFuncs::Out_String( rOut, aStr, eDestEnc, pNonConvertableChars );
            sOut.append('\"');
        }

        sal_Int32 nVal = SIZE_NOT_SET;
        aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameMarginWidth")) );
        if ( (aAny >>= nVal) && nVal != SIZE_NOT_SET )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_marginwidth)
                .append('=').append(nVal);
        }
        aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameMarginHeight")) );
        if ( (aAny >>= nVal) && nVal != SIZE_NOT_SET )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_marginheight)
                .append('=').append(nVal);
        }

        sal_Bool bVal = sal_True;
        aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameIsAutoScroll")) );
        if ( (aAny >>= bVal) && !bVal )
        {
            aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameIsScrollingMode")) );
            if ( aAny >>= bVal )
            {
                const sal_Char *pStr = bVal ? sHTML_SC_yes : sHTML_SC_no;
                sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_scrolling)
                    .append(pStr);
            }
        }

        // frame border (MS+Netscape-Extension)
        aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameIsAutoBorder")) );
        if ( (aAny >>= bVal) && !bVal )
        {
            aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameIsBorder")) );
            if ( aAny >>= bVal )
            {
                const char* pStr = bVal ? sHTML_SC_yes : sHTML_SC_no;
                sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_frameborder)
                    .append('=').append(pStr);
            }
        }
        rOut << sOut.getStr();
    }
    catch (const uno::Exception&)
    {
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
