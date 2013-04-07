/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Tor Lillqvist <tlillqvist@suse.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "oox/mathml/importutils.hxx"

#include <assert.h>
#include <stdio.h>

#include <oox/token/namespacemap.hxx>
#include <oox/token/tokenmap.hxx>
#include <oox/token/tokens.hxx>
#include <oox/token/namespaces.hxx>
#include <rtl/string.hxx>

// *sigh*
#define STR( str ) OUString( RTL_CONSTASCII_USTRINGPARAM( str ))
#define CSTR( str ) ( rtl::OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr())

#define OPENING( token ) XML_STREAM_OPENING( token )
#define CLOSING( token ) XML_STREAM_CLOSING( token )

// HACK - TODO convert to the real debug stuff
#undef SAL_LOG_LEVEL
#define SAL_LOG_LEVEL 2


using namespace com::sun::star;
using rtl::OUString;

namespace oox
{

namespace formulaimport
{

namespace
{
// a class that inherits from AttributeList, builds the internal data and then will be sliced off
// during conversion to the base class
class AttributeListBuilder
    : public XmlStream::AttributeList
{
public:
    AttributeListBuilder( const uno::Reference< xml::sax::XFastAttributeList >& a );
};

AttributeListBuilder::AttributeListBuilder( const uno::Reference< xml::sax::XFastAttributeList >& a )
{
    if( a.get() == NULL )
        return;
    uno::Sequence< xml::FastAttribute > aFastAttrSeq = a->getFastAttributes();
    const xml::FastAttribute* pFastAttr = aFastAttrSeq.getConstArray();
    sal_Int32 nFastAttrLength = aFastAttrSeq.getLength();
    for( int i = 0;
         i < nFastAttrLength;
         ++i )
    {
        attrs[ pFastAttr[ i ].Token ] = pFastAttr[ i ].Value;
    }
}

#if 0
static OUString tokenToString( int token )
{
    OUString tokenname = StaticTokenMap::get().getUnicodeTokenName( token & TOKEN_MASK );
    if( tokenname.isEmpty())
        tokenname = STR( "???" );
    int nmsp = ( token & NMSP_MASK & ~( TAG_OPENING | TAG_CLOSING ));
#if 0 // this is awfully long
    OUString namespacename = StaticNamespaceMap::get().count( nmsp ) != 0
        ? StaticNamespaceMap::get()[ nmsp ] : STR( "???" );
#else
    OUString namespacename;
    // only few are needed actually
    switch( nmsp )
    {
        case NMSP_officeMath:
            namespacename = STR( "m" );
            break;
        case NMSP_doc:
            namespacename = STR( "w" );
            break;
        default:
            namespacename = STR( "?" );
            break;
    }
#endif
    if( token == OPENING( token ))
        return STR( "<" ) + namespacename + STR( ":" ) + tokenname + STR ( ">" );
    if( token == CLOSING( token ))
        return STR( "</" ) + namespacename + STR( ":" ) + tokenname + STR ( ">" );
    // just the name itself, not specified whether opening or closing
    return namespacename + STR( ":" ) + tokenname;
}
#endif

} // namespace

bool XmlStream::AttributeList::hasAttribute( int token ) const
{
    return attrs.find( token ) != attrs.end();
}

rtl::OUString XmlStream::AttributeList::attribute( int token, const rtl::OUString& def ) const
{
    std::map< int, rtl::OUString >::const_iterator find = attrs.find( token );
    if( find != attrs.end())
        return find->second;
    return def;
}

bool XmlStream::AttributeList::attribute( int token, bool def ) const
{
    std::map< int, rtl::OUString >::const_iterator find = attrs.find( token );
    if( find != attrs.end())
    {
        if( find->second.equalsIgnoreAsciiCaseAscii( "true" ) || find->second.equalsIgnoreAsciiCaseAscii( "on" )
            || find->second.equalsIgnoreAsciiCaseAscii( "t" ) || find->second.equalsIgnoreAsciiCaseAscii( "1" ))
            return true;
        if( find->second.equalsIgnoreAsciiCaseAscii( "false" ) || find->second.equalsIgnoreAsciiCaseAscii( "off" )
            || find->second.equalsIgnoreAsciiCaseAscii( "f" ) || find->second.equalsIgnoreAsciiCaseAscii( "0" ))
            return false;
//        fprintf( stderr, "Cannot convert \'%s\' to bool.\n", CSTR( find->second ));
    }
    return def;
}

sal_Unicode XmlStream::AttributeList::attribute( int token, sal_Unicode def ) const
{
    std::map< int, rtl::OUString >::const_iterator find = attrs.find( token );
    if( find != attrs.end())
    {
        if( find->second.getLength() >= 1 )
        {
//            if( find->second.getLength() != 1 )
//                fprintf( stderr, "Cannot convert \'%s\' to sal_Unicode, stripping.\n", CSTR( find->second ));
            return find->second[ 0 ];
        }
    }
    return def;
}

XmlStream::Tag::Tag( int t, const uno::Reference< xml::sax::XFastAttributeList >& a, const rtl::OUString& txt )
: token( t )
, attributes( AttributeListBuilder( a ))
, text( txt )
{
}


XmlStream::Tag::operator bool() const
{
    return token != XML_TOKEN_INVALID;
}

XmlStream::XmlStream()
: pos( 0 )
{
    // make sure our extra bit does not conflict with values used by oox
    assert( TAG_OPENING > ( 1024 << NMSP_SHIFT ));
}

bool XmlStream::atEnd() const
{
    return pos >= tags.size();
}

XmlStream::Tag XmlStream::currentTag() const
{
    if( pos >= tags.size())
        return Tag();
    return tags[ pos ];
}

int XmlStream::currentToken() const
{
    if( pos >= tags.size())
        return XML_TOKEN_INVALID;
    return tags[ pos ].token;
}

void XmlStream::moveToNextTag()
{
    if( pos < tags.size())
        ++pos;
}

XmlStream::Tag XmlStream::ensureOpeningTag( int token )
{
    return checkTag( OPENING( token ), false );
}

XmlStream::Tag XmlStream::checkOpeningTag( int token )
{
    return checkTag( OPENING( token ), true );
}

void XmlStream::ensureClosingTag( int token )
{
    checkTag( CLOSING( token ), false );
}

XmlStream::Tag XmlStream::checkTag( int token, bool optional )
{
    // either it's the following tag, or find it
    int savedPos = pos;
#if SAL_LOG_LEVEL >= 2
    if( optional )
    { // avoid printing debug messages about skipping tags if the optional one
      // will not be found and the position will be reset back
        if( currentToken() != token && !findTagInternal( token, true ))
        {
            pos = savedPos;
            return Tag();
        }
    }
#endif
    if( currentToken() == token || findTag( token ))
    {
        Tag ret = currentTag();
        moveToNextTag();
        return ret; // ok
    }
    if( optional )
    { // not a problem, just rewind
        pos = savedPos;
        return Tag();
    }
//    fprintf( stderr, "Expected tag %s not found.\n", CSTR( tokenToString( token )));
    return Tag();
}

bool XmlStream::findTag( int token )
{
    return findTagInternal( token, false );
}

bool XmlStream::findTagInternal( int token, bool /*silent*/ )
{
    int depth = 0;
    for(;
         !atEnd();
         moveToNextTag())
    {
        if( depth > 0 ) // we're inside a nested element, skip those
        {
            if( currentToken() == OPENING( currentToken()))
            {
//                if( !silent )
//                    fprintf( stderr, "Skipping tag %s\n", CSTR( tokenToString( currentToken())));
                ++depth;
            }
            else if( currentToken() == CLOSING( currentToken()))
            {
//                if( !silent )
//                    fprintf( stderr, "Skipping tag %s\n", CSTR( tokenToString( currentToken())));
                --depth;
            }
            else
            {
//                if( !silent )
//                    fprintf( stderr, "Malformed token %d (%s)\n", currentToken(), CSTR( tokenToString( currentToken())));
                abort();
            }
            continue;
        }
        if( currentToken() == token )
            return true; // ok, found
        if( currentToken() == CLOSING( currentToken()))
            return false; // that would be leaving current element, so not found
        if( currentToken() == OPENING( currentToken()))
        {
//            if( !silent )
//                fprintf( stderr, "Skipping tag %s\n", CSTR( tokenToString( currentToken())));
            ++depth;
        }
        else
            abort();
    }
//    if( !silent )
//        fprintf( stderr, "Unexpected end of stream reached.\n" );
    return false;
}

void XmlStream::skipElement( int token )
{
    return skipElementInternal( token, true ); // no debug about skipping if called from outside
}

void XmlStream::skipElementInternal( int token, bool /*silent*/ )
{
    int closing = ( token & ~TAG_OPENING ) | TAG_CLOSING; // make it a closing tag
    assert( currentToken() == OPENING( token ));
//    if( !silent )
//        fprintf( stderr, "Skipping unexpected element %s\n", CSTR( tokenToString( currentToken())));
    moveToNextTag();
    // and just find the matching closing tag
    if( findTag( closing ))
    {
//        if( !silent )
//            fprintf( stderr, "Skipped unexpected element %s\n", CSTR( tokenToString( token )));
        moveToNextTag(); // and skip it too
        return;
    }
    // this one is an unexpected problem, do not silent it
//    fprintf( stderr, "Expected end of element %s not found.\n", CSTR( tokenToString( token )));
}

void XmlStream::handleUnexpectedTag()
{
    if( atEnd())
        return;
    if( currentToken() == CLOSING( currentToken()))
    {
//        fprintf( stderr, "Skipping unexpected tag %s\n", CSTR( tokenToString( currentToken())));
        moveToNextTag(); // just skip it
        return;
    }
    skipElementInternal( currentToken(), false ); // otherwise skip the entire element
}


void XmlStreamBuilder::appendOpeningTag( int token, const uno::Reference< xml::sax::XFastAttributeList >& attrs )
{
    tags.push_back( Tag( OPENING( token ), attrs ));
}

void XmlStreamBuilder::appendClosingTag( int token )
{
    tags.push_back( Tag( CLOSING( token )));
}

void XmlStreamBuilder::appendCharacters( const rtl::OUString& chars )
{
    assert( !tags.empty());
    tags.back().text += chars;
}

} // namespace
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
