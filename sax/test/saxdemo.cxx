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

//------------------------------------------------------
// testcomponent - Loads a service and its testcomponent from dlls performs a test.
// Expands the dll-names depending on the actual environment.
// Example : testcomponent stardiv.uno.io.Pipe stm
//
// Therefor the testcode must exist in teststm and the testservice must be named test.stardiv.uno.io.Pipe
//

#include <stdio.h>
#include <vector>
#include <cstring>

#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>

#include <osl/diagnose.h>

using namespace ::rtl;
using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;


/************
 * Sequence of bytes -> InputStream
 ************/
class OInputStream : public WeakImplHelper1 < XInputStream >
{
public:
    OInputStream( const Sequence< sal_Int8 >&seq ) :
        m_seq( seq ),
        nPos( 0 )
        {}

public:
    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
        {
            nBytesToRead = (nBytesToRead > m_seq.getLength() - nPos ) ?
                m_seq.getLength() - nPos :
                nBytesToRead;
            aData = Sequence< sal_Int8 > ( &(m_seq.getConstArray()[nPos]) , nBytesToRead );
            nPos += nBytesToRead;
            return nBytesToRead;
        }
    virtual sal_Int32 SAL_CALL readSomeBytes(
        ::com::sun::star::uno::Sequence< sal_Int8 >& aData,
        sal_Int32 nMaxBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
        {
            return readBytes( aData, nMaxBytesToRead );
        }
    virtual void SAL_CALL skipBytes( sal_Int32 /* nBytesToSkip */ )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
        {
            // not implemented
        }
    virtual sal_Int32 SAL_CALL available(  )
        throw(NotConnectedException, IOException, RuntimeException)
        {
            return m_seq.getLength() - nPos;
        }
    virtual void SAL_CALL closeInput(  )
        throw(NotConnectedException, IOException, RuntimeException)
        {
            // not needed
        }
    Sequence< sal_Int8> m_seq;
    sal_Int32 nPos;
};

//-------------------------------
// Helper : create an input stream from a file
//------------------------------
Reference< XInputStream > createStreamFromFile(
    const char *pcFile )
{
    FILE *f = fopen( pcFile , "rb" );
    Reference<  XInputStream >  r;

    if( f ) {
        fseek( f , 0 , SEEK_END );
        int nLength = ftell( f );
        fseek( f , 0 , SEEK_SET );

        Sequence<sal_Int8> seqIn(nLength);
        fread( seqIn.getArray() , nLength , 1 , f );

        r = Reference< XInputStream > ( new OInputStream( seqIn ) );
        fclose( f );
    }
    return r;
}

//-----------------------------------------
// The document handler, which is needed for the saxparser
// The Documenthandler for reading sax
//-----------------------------------------
class TestDocumentHandler :
    public WeakImplHelper3< XExtendedDocumentHandler , XEntityResolver , XErrorHandler >
{
public:
    TestDocumentHandler(  )
    {
    }

public: // Error handler
    virtual void SAL_CALL error(const Any& aSAXParseException) throw (SAXException, RuntimeException)
    {
        printf( "Error !\n" );
        throw  SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("error from error handler")) ,
            Reference < XInterface >() ,
            aSAXParseException );
    }
    virtual void SAL_CALL fatalError(const Any& /* aSAXParseException */) throw (SAXException, RuntimeException)
    {
        printf( "Fatal Error !\n" );
    }
    virtual void SAL_CALL warning(const Any& /* aSAXParseException */) throw (SAXException, RuntimeException)
    {
        printf( "Warning !\n" );
    }


public: // ExtendedDocumentHandler

    virtual void SAL_CALL startDocument(void) throw (SAXException, RuntimeException)
    {
        m_iElementCount = 0;
        m_iAttributeCount = 0;
        m_iWhitespaceCount =0;
        m_iCharCount=0;
           printf( "document started\n" );
    }
    virtual void SAL_CALL endDocument(void) throw (SAXException, RuntimeException)
    {
        printf( "document finished\n" );
        printf( "(ElementCount %d),(AttributeCount %d),(WhitespaceCount %d),(CharCount %d)\n",
                m_iElementCount, m_iAttributeCount, m_iWhitespaceCount , m_iCharCount );

    }
    virtual void SAL_CALL startElement(const OUString& /* aName */,
                              const Reference< XAttributeList > & xAttribs)
        throw (SAXException,RuntimeException)
    {
        m_iElementCount ++;
        m_iAttributeCount += xAttribs->getLength();
    }

    virtual void SAL_CALL endElement(const OUString& /* aName */) throw (SAXException,RuntimeException)
    {
        // ignored
    }

    virtual void SAL_CALL characters(const OUString& aChars) throw (SAXException,RuntimeException)
    {
        m_iCharCount += aChars.getLength();
    }
    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces) throw (SAXException,RuntimeException)
    {
        m_iWhitespaceCount += aWhitespaces.getLength();
    }

    virtual void SAL_CALL processingInstruction(const OUString& /* aTarget */, const OUString& /* aData */) throw (SAXException,RuntimeException)
    {
        // ignored
    }

    virtual void SAL_CALL setDocumentLocator(const Reference< XLocator> & /* xLocator */)
        throw (SAXException,RuntimeException)
    {
        // ignored
    }

    virtual InputSource SAL_CALL resolveEntity(
        const OUString& sPublicId,
        const OUString& sSystemId)
        throw (RuntimeException)
    {
        InputSource source;
        source.sSystemId = sSystemId;
        source.sPublicId = sPublicId;

        source.aInputStream = createStreamFromFile(
            OUStringToOString( sSystemId, RTL_TEXTENCODING_ASCII_US).getStr() );

        return source;
    }

    virtual void SAL_CALL startCDATA(void) throw (SAXException,RuntimeException)
    {
    }
    virtual void SAL_CALL endCDATA(void) throw (RuntimeException)
    {
    }
    virtual void SAL_CALL comment(const OUString& /* sComment */) throw (SAXException,RuntimeException)
    {
    }
    virtual void SAL_CALL unknown(const OUString& /* sString */) throw (SAXException,RuntimeException)
    {
    }

    virtual void SAL_CALL allowLineBreak( void) throw (SAXException, RuntimeException )
    {

    }

public:
    int m_iElementCount;
    int m_iAttributeCount;
    int m_iWhitespaceCount;
    int m_iCharCount;
};

//--------------------------------------
// helper implementation for writing
// implements an XAttributeList
//-------------------------------------
struct AttributeListImpl_impl;
class AttributeListImpl : public WeakImplHelper1< XAttributeList >
{
public:
    AttributeListImpl();
    AttributeListImpl( const AttributeListImpl & );
    ~AttributeListImpl();

public:
    virtual sal_Int16 SAL_CALL getLength(void) throw  (RuntimeException);
    virtual OUString SAL_CALL getNameByIndex(sal_Int16 i) throw  (RuntimeException);
    virtual OUString SAL_CALL getTypeByIndex(sal_Int16 i) throw  (RuntimeException);
    virtual OUString SAL_CALL getTypeByName(const OUString& aName) throw  (RuntimeException);
    virtual OUString SAL_CALL getValueByIndex(sal_Int16 i) throw  (RuntimeException);
    virtual OUString SAL_CALL getValueByName(const OUString& aName) throw  (RuntimeException);

public:
    void addAttribute( const OUString &sName ,
                       const OUString &sType ,
                       const OUString &sValue );
    void clear();

private:
    struct AttributeListImpl_impl *m_pImpl;
};


struct TagAttribute
{
    TagAttribute(){}
    TagAttribute( const OUString &s_Name,
                  const OUString &s_Type ,
                  const OUString &s_Value )
    {
        this->sName     = s_Name;
        this->sType     = s_Type;
        this->sValue    = s_Value;
    }

    OUString sName;
    OUString sType;
    OUString sValue;
};

struct AttributeListImpl_impl
{
    AttributeListImpl_impl()
    {
        // performance improvement during adding
        vecAttribute.reserve(20);
    }
    vector<struct TagAttribute> vecAttribute;
};



sal_Int16 AttributeListImpl::getLength(void) throw  (RuntimeException)
{
    return (sal_Int16) m_pImpl->vecAttribute.size();
}


AttributeListImpl::AttributeListImpl( const AttributeListImpl &r )
{
    m_pImpl = new AttributeListImpl_impl;
    *m_pImpl = *(r.m_pImpl);
}

OUString AttributeListImpl::getNameByIndex(sal_Int16 i) throw  (RuntimeException)
{
    if( i < sal::static_int_cast<sal_Int16>(m_pImpl->vecAttribute.size()) ) {
        return m_pImpl->vecAttribute[i].sName;
    }
    return OUString();
}


OUString AttributeListImpl::getTypeByIndex(sal_Int16 i) throw  (RuntimeException)
{
    if( i < sal::static_int_cast<sal_Int16>(m_pImpl->vecAttribute.size()) ) {
        return m_pImpl->vecAttribute[i].sType;
    }
    return OUString();
}

OUString AttributeListImpl::getValueByIndex(sal_Int16 i) throw  (RuntimeException)
{
    if( i < sal::static_int_cast<sal_Int16>(m_pImpl->vecAttribute.size()) ) {
        return m_pImpl->vecAttribute[i].sValue;
    }
    return OUString();

}

OUString AttributeListImpl::getTypeByName( const OUString& sName ) throw  (RuntimeException)
{
    vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ++ii ) {
        if( (*ii).sName == sName ) {
            return (*ii).sType;
        }
    }
    return OUString();
}

OUString AttributeListImpl::getValueByName(const OUString& sName) throw  (RuntimeException)
{
    vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ++ii ) {
        if( (*ii).sName == sName ) {
            return (*ii).sValue;
        }
    }
    return OUString();
}



AttributeListImpl::AttributeListImpl()
{
    m_pImpl = new AttributeListImpl_impl;
}



AttributeListImpl::~AttributeListImpl()
{
    delete m_pImpl;
}


void AttributeListImpl::addAttribute(   const OUString &sName ,
                                        const OUString &sType ,
                                        const OUString &sValue )
{
    m_pImpl->vecAttribute.push_back( TagAttribute( sName , sType , sValue ) );
}

void AttributeListImpl::clear()
{
    m_pImpl->vecAttribute.clear();
}


//--------------------------------------
// helper function for writing
// ensures that linebreaks are inserted
// when writing a long text.
// Note: this implementation may be a bit slow,
// but it shows, how the SAX-Writer handles the allowLineBreak calls.
//--------------------------------------
void writeParagraphHelper(
    const  Reference< XExtendedDocumentHandler > &r ,
    const OUString & s)
{
    int nMax = s.getLength();
    int nStart = 0;
    int n = 1;

    Sequence<sal_uInt16> seq( s.getLength() );
    memcpy( seq.getArray() , s.getStr() , s.getLength() * sizeof( sal_uInt16 ) );

    for( n = 1 ; n < nMax ; n++ ){
        if( 32 == seq.getArray()[n] ) {
            r->allowLineBreak();
            r->characters( s.copy( nStart , n - nStart ) );
            nStart = n;
        }
    }
    r->allowLineBreak();
    r->characters( s.copy( nStart , n - nStart ) );
}


//---------------------------------
// helper implementation for SAX-Writer
// writes data to a file
//--------------------------------
class OFileWriter :
        public WeakImplHelper1< XOutputStream >
{
public:
    OFileWriter( char *pcFile ) { strncpy( m_pcFile , pcFile, 256 - 1 ); m_f = 0; }


public:
    virtual void SAL_CALL writeBytes(const Sequence< sal_Int8 >& aData)
        throw  (NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual void SAL_CALL flush(void)
        throw  (NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual void SAL_CALL closeOutput(void)
        throw  (NotConnectedException, BufferSizeExceededException, RuntimeException);
private:
    char m_pcFile[256];
    FILE *m_f;
};


void OFileWriter::writeBytes(const Sequence< sal_Int8 >& aData)
    throw  (NotConnectedException, BufferSizeExceededException, RuntimeException)
{
    if( ! m_f ) {
        m_f = fopen( m_pcFile , "w" );
    }

    fwrite( aData.getConstArray() , 1 , aData.getLength() , m_f );
}


void OFileWriter::flush(void)
    throw  (NotConnectedException, BufferSizeExceededException, RuntimeException)
{
    fflush( m_f );
}

void OFileWriter::closeOutput(void)
    throw  (NotConnectedException, BufferSizeExceededException, RuntimeException)
{
    fclose( m_f );
    m_f = 0;
}



// Needed to switch on solaris threads
#ifdef SOLARIS
extern "C" void ChangeGlobalInit();
#endif
int main (int argc, char **argv)
{

    if( argc < 3) {
        printf( "usage : saxdemo inputfile outputfile\n" );
        exit( 0 );
    }
#ifdef SOLARIS
    // switch on threads in solaris
    ChangeGlobalInit();
#endif

    // create service manager
    Reference< XMultiServiceFactory > xSMgr = createRegistryServiceFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "applicat.rdb" )) );

    Reference < XImplementationRegistration > xReg;
    try
    {
        // Create registration service
        Reference < XInterface > x = xSMgr->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.ImplementationRegistration")) );
        xReg = Reference<  XImplementationRegistration > ( x , UNO_QUERY );
    }
    catch( Exception & ) {
        printf( "Couldn't create ImplementationRegistration service\n" );
        exit(1);
    }

    OString sTestName;
    try
    {
        // Load dll for the tested component
        OUString aDllName(RTL_CONSTASCII_USTRINGPARAM( "sax.uno" SAL_DLLEXTENSION ));
        xReg->registerImplementation(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary")),
            aDllName,
            Reference< XSimpleRegistry > ()  );
    }
    catch( Exception &e ) {
        printf( "Couldn't reach sax dll\n" );
        printf( "%s\n" , OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).getStr() );

        exit(1);
    }


    //--------------------------------
    // parser demo
    // read xml from a file and count elements
    //--------------------------------
    Reference< XInterface > x = xSMgr->createInstance(
        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser")) );
    if( x.is() )
    {
        Reference< XParser > rParser( x , UNO_QUERY );

        // create and connect the document handler to the parser
        TestDocumentHandler *pDocHandler = new TestDocumentHandler( );

        Reference < XDocumentHandler >  rDocHandler( (XDocumentHandler *) pDocHandler );
        Reference< XEntityResolver > rEntityResolver( (XEntityResolver *) pDocHandler );

        rParser->setDocumentHandler( rDocHandler );
        rParser->setEntityResolver( rEntityResolver );

        // create the input stream
        InputSource source;
        source.aInputStream = createStreamFromFile( argv[1] );
        source.sSystemId    = OUString::createFromAscii( argv[1] );

        try
        {
            // start parsing
            rParser->parseStream( source );
        }

        catch( Exception & e )
        {
            OString o1 = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8 );
            printf( "Exception during parsing : %s\n" ,  o1.getStr() );
        }
    }
    else
    {
        printf( "couldn't create sax-parser component\n" );
    }


    //----------------------
    // The SAX-Writer demo
    //----------------------
    x= xSMgr->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer")) );
    if( x.is() )
    {
        printf( "start writing to %s\n" , argv[2] );

        OFileWriter *pw = new OFileWriter( argv[2] );
        Reference< XActiveDataSource > source( x , UNO_QUERY );
        source->setOutputStream( Reference< XOutputStream> ( (XOutputStream*) pw ) );

        AttributeListImpl *pList = new AttributeListImpl;
        Reference< XAttributeList > rList( (XAttributeList *) pList );

        Reference< XExtendedDocumentHandler > r( x , UNO_QUERY );
        r->startDocument();

        pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM("Arg1" )),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("CDATA")) ,
                             OUString( RTL_CONSTASCII_USTRINGPARAM("foo\n   u")) );
        pList->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM("Arg2")) ,
                             OUString( RTL_CONSTASCII_USTRINGPARAM("CDATA")) ,
                             OUString( RTL_CONSTASCII_USTRINGPARAM("foo2")) );

        r->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM("tag1"))  , rList );
        // tells the writer to insert a linefeed
        r->ignorableWhitespace( OUString() );

        r->characters( OUString( RTL_CONSTASCII_USTRINGPARAM("huhu")) );
        r->ignorableWhitespace( OUString() );

        r->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM("hi")) , rList );
        r->ignorableWhitespace( OUString() );

        // the enpassant must be converted & -> &amp;
        r->characters( OUString( RTL_CONSTASCII_USTRINGPARAM("&#252;")) );
        r->ignorableWhitespace( OUString() );

        // '>' must not be converted
        r->startCDATA();
        r->characters( OUString( RTL_CONSTASCII_USTRINGPARAM(" > foo < "))  );
        r->endCDATA();
        r->ignorableWhitespace( OUString() );

        OUString testParagraph = OUString( RTL_CONSTASCII_USTRINGPARAM(
            "This is only a test to check, if the writer inserts line feeds "
            "if needed or if the writer puts the whole text into one line." ));
        writeParagraphHelper( r , testParagraph );

        r->ignorableWhitespace( OUString() );
        r->comment( OUString( RTL_CONSTASCII_USTRINGPARAM("This is a comment !")) );
        r->ignorableWhitespace( OUString() );

        r->startElement( OUString( RTL_CONSTASCII_USTRINGPARAM("emptytagtest"))  , rList );
        r->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM("emptytagtest")) );
        r->ignorableWhitespace( OUString() );

        r->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM("hi")) );
        r->ignorableWhitespace( OUString() );

        r->endElement( OUString( RTL_CONSTASCII_USTRINGPARAM("tag1")) );
        r->endDocument();

        printf( "finished writing\n" );
    }
    else
    {
        printf( "couldn't create sax-writer component\n" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
