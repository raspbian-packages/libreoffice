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

#include <rtl/strbuf.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include "drawdoc.hxx"
#include <unotools/streamwrap.hxx>
#include <svx/xmlgrhlp.hxx>

#include "../../ui/inc/DrawDocShell.hxx"

#include "sdxmlwrp.hxx"
#include "strmname.h"
#include <svx/xmleohlp.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>

#include <com/sun/star/xml/sax/XErrorHandler.hpp>
#include <com/sun/star/xml/sax/XEntityResolver.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDTDHandler.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <unotools/saveopt.hxx>

// include necessary for XML progress bar at load time
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svtools/sfxecode.hxx>

#include "sderror.hxx"
#include "sdresid.hxx"
#include "sdtransform.hxx"
#include "glob.hrc"

#include <sfx2/frame.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::document;
using namespace comphelper;

#define SD_XML_READERROR 1234

char const sXML_metaStreamName[] = "meta.xml";
char const sXML_styleStreamName[] = "styles.xml";
char const sXML_contentStreamName[] = "content.xml";
char const sXML_settingsStreamName[] = "settings.xml";

char const sXML_export_impress_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisExporter";
char const sXML_export_impress_meta_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisMetaExporter";
char const sXML_export_impress_styles_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisStylesExporter";
char const sXML_export_impress_content_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisContentExporter";
char const sXML_export_impress_settings_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisSettingsExporter";

char const sXML_export_draw_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisExporter";
char const sXML_export_draw_meta_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisMetaExporter";
char const sXML_export_draw_styles_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisStylesExporter";
char const sXML_export_draw_content_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisContentExporter";
char const sXML_export_draw_settings_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisSettingsExporter";

char const sXML_import_impress_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisImporter";
char const sXML_import_impress_meta_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisMetaImporter";
char const sXML_import_impress_styles_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisStylesImporter";
char const sXML_import_impress_content_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisContentImporter";
char const sXML_import_impress_settings_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisSettingsImporter";

char const sXML_import_draw_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisImporter";
char const sXML_import_draw_meta_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisMetaImporter";
char const sXML_import_draw_styles_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisStylesImporter";
char const sXML_import_draw_content_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisContentImporter";
char const sXML_import_draw_settings_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisSettingsImporter";

// OOo
char const sXML_export_impress_ooo_service[] = "com.sun.star.comp.Impress.XMLExporter";
char const sXML_export_impress_meta_ooo_service[] = "com.sun.star.comp.Impress.XMLMetaExporter";
char const sXML_export_impress_styles_ooo_service[] = "com.sun.star.comp.Impress.XMLStylesExporter";
char const sXML_export_impress_content_ooo_service[] = "com.sun.star.comp.Impress.XMLContentExporter";
char const sXML_export_impress_settings_ooo_service[] = "com.sun.star.comp.Impress.XMLSettingsExporter";

char const sXML_export_draw_ooo_service[] = "com.sun.star.comp.Draw.XMLExporter";
char const sXML_export_draw_meta_ooo_service[] = "com.sun.star.comp.Draw.XMLMetaExporter";
char const sXML_export_draw_styles_ooo_service[] = "com.sun.star.comp.Draw.XMLStylesExporter";
char const sXML_export_draw_content_ooo_service[] = "com.sun.star.comp.Draw.XMLContentExporter";
char const sXML_export_draw_settings_ooo_service[] = "com.sun.star.comp.Draw.XMLSettingsExporter";

char const sXML_import_impress_ooo_service[] = "com.sun.star.comp.Impress.XMLImporter";
char const sXML_import_impress_meta_ooo_service[] = "com.sun.star.comp.Impress.XMLMetaImporter";
char const sXML_import_impress_styles_ooo_service[] = "com.sun.star.comp.Impress.XMLStylesImporter";
char const sXML_import_impress_content_ooo_service[] = "com.sun.star.comp.Impress.XMLContentImporter";
char const sXML_import_impress_settings_ooo_service[] = "com.sun.star.comp.Impress.XMLSettingsImporter";

char const sXML_import_draw_ooo_service[] = "com.sun.star.comp.Draw.XMLImporter";
char const sXML_import_draw_meta_ooo_service[] = "com.sun.star.comp.Draw.XMLMetaImporter";
char const sXML_import_draw_styles_ooo_service[] = "com.sun.star.comp.Draw.XMLStylesImporter";
char const sXML_import_draw_content_ooo_service[] = "com.sun.star.comp.Draw.XMLContentImporter";
char const sXML_import_draw_settings_ooo_service[] = "com.sun.star.comp.Draw.XMLSettingsImporter";

struct XML_SERVICEMAP
{
    const sal_Char* mpService;
    const sal_Char* mpStream;
};

struct XML_SERVICES
{
    const sal_Char* mpAll;
    const sal_Char* mpMeta;
    const sal_Char* mpStyles;
    const sal_Char* mpContent;
    const sal_Char* mpSettings;
};

XML_SERVICES* getServices( bool bImport, bool bDraw, sal_uLong nStoreVer )
{
    static XML_SERVICES gServices[] =
    {
        { sXML_export_impress_oasis_service, sXML_export_impress_meta_oasis_service, sXML_export_impress_styles_oasis_service, sXML_export_impress_content_oasis_service, sXML_export_impress_settings_oasis_service },
        { sXML_export_draw_oasis_service, sXML_export_draw_meta_oasis_service, sXML_export_draw_styles_oasis_service, sXML_export_draw_content_oasis_service, sXML_export_draw_settings_oasis_service },
        { sXML_import_impress_oasis_service, sXML_import_impress_meta_oasis_service, sXML_import_impress_styles_oasis_service, sXML_import_impress_content_oasis_service, sXML_import_impress_settings_oasis_service },
        { sXML_import_draw_oasis_service, sXML_import_draw_meta_oasis_service, sXML_import_draw_styles_oasis_service, sXML_import_draw_content_oasis_service, sXML_import_draw_settings_oasis_service },

        { sXML_export_impress_ooo_service, sXML_export_impress_meta_ooo_service, sXML_export_impress_styles_ooo_service, sXML_export_impress_content_ooo_service, sXML_export_impress_settings_ooo_service },
        { sXML_export_draw_ooo_service, sXML_export_draw_meta_ooo_service, sXML_export_draw_styles_ooo_service, sXML_export_draw_content_ooo_service, sXML_export_draw_settings_ooo_service },
        { sXML_import_impress_ooo_service, sXML_import_impress_meta_ooo_service, sXML_import_impress_styles_ooo_service, sXML_import_impress_content_ooo_service, sXML_import_impress_settings_ooo_service },
        { sXML_import_draw_ooo_service, sXML_import_draw_meta_ooo_service, sXML_import_draw_styles_ooo_service, sXML_import_draw_content_ooo_service, sXML_import_draw_settings_ooo_service },
    };

    return &gServices[ (bImport ? 2 : 0) + ((nStoreVer == SOFFICE_FILEFORMAT_60) ? 4 : 0) + (bDraw ? 1 : 0 ) ];
}



// - SdXMLWrapper -


SdXMLFilter::SdXMLFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, bool bShowProgress, SdXMLFilterMode eFilterMode, sal_uLong nStoreVer ) :
    SdFilter( rMedium, rDocShell, bShowProgress ), meFilterMode( eFilterMode ), mnStoreVer( nStoreVer )
{
}

SdXMLFilter::~SdXMLFilter(void)
{
}

sal_Int32 ReadThroughComponent(
    Reference<io::XInputStream> xInputStream,
    Reference<XComponent> xModelComponent,
    const OUString& rStreamName,
    Reference<uno::XComponentContext> & rxContext,
    const sal_Char* pFilterName,
    const Sequence<Any>& rFilterArguments,
    const OUString& rName,
    bool bMustBeSuccessfull,
    bool bEncrypted )
{
    DBG_ASSERT(xInputStream.is(), "input stream missing");
    DBG_ASSERT(xModelComponent.is(), "document missing");
    DBG_ASSERT(rxContext.is(), "factory missing");
    DBG_ASSERT(NULL != pFilterName,"I need a service name for the component!");

    SAL_INFO( "sd.filter", "ReadThroughComponent" );

    // prepare ParserInputSrouce
    xml::sax::InputSource aParserInput;
    aParserInput.sSystemId = rName;
    aParserInput.aInputStream = xInputStream;

    // get parser
    Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(rxContext);
    SAL_INFO( "sd.filter", "parser created" );

    // get filter
    OUString aFilterName(OUString::createFromAscii(pFilterName));
    Reference< xml::sax::XDocumentHandler > xFilter(
        rxContext->getServiceManager()->createInstanceWithArgumentsAndContext(aFilterName, rFilterArguments, rxContext),
        UNO_QUERY );
    SAL_WARN_IF(!xFilter.is(), "sd.filter", "Can't instantiate filter component: " << aFilterName);
    if( !xFilter.is() )
        return SD_XML_READERROR;
    SAL_INFO( "sd.filter", "" << pFilterName << " created" );

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    // connect model and filter
    Reference < XImporter > xImporter( xFilter, UNO_QUERY );
    xImporter->setTargetDocument( xModelComponent );
    // finally, parser the stream
    SAL_INFO( "sd.filter", "parsing stream" );
    try
    {
        xParser->parseStream( aParserInput );
    }
    catch (const xml::sax::SAXParseException& r)
    {
        // sax parser sends wrapped exceptions,
        // try to find the original one
        xml::sax::SAXException aSaxEx = *(xml::sax::SAXException*)(&r);
        bool bTryChild = true;

        while( bTryChild )
        {
            xml::sax::SAXException aTmp;
            if ( aSaxEx.WrappedException >>= aTmp )
                aSaxEx = aTmp;
            else
                bTryChild = false;
        }

        packages::zip::ZipIOException aBrokenPackage;
        if ( aSaxEx.WrappedException >>= aBrokenPackage )
            return ERRCODE_IO_BROKENPACKAGE;

        if( bEncrypted )
            return ERRCODE_SFX_WRONGPASSWORD;

#if OSL_DEBUG_LEVEL > 1
        SAL_WARN( "sd.filter", "SAX parse exception caught while importing:" << r.Message);
#endif

        OUString sErr( OUString::number( r.LineNumber ));
        sErr += ",";
        sErr += OUString::number( r.ColumnNumber );

        if (!rStreamName.isEmpty())
        {
            return *new TwoStringErrorInfo(
                            (bMustBeSuccessfull ? ERR_FORMAT_FILE_ROWCOL
                                                    : WARN_FORMAT_FILE_ROWCOL),
                            rStreamName, sErr,
                            ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
        }
        else
        {
            DBG_ASSERT( bMustBeSuccessfull, "Warnings are not supported" );
            return *new StringErrorInfo( ERR_FORMAT_ROWCOL, sErr,
                             ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
        }
    }
    catch (const xml::sax::SAXException& r)
    {
        packages::zip::ZipIOException aBrokenPackage;
        if ( r.WrappedException >>= aBrokenPackage )
            return ERRCODE_IO_BROKENPACKAGE;

        if( bEncrypted )
            return ERRCODE_SFX_WRONGPASSWORD;

#if OSL_DEBUG_LEVEL > 1
        SAL_WARN( "sd.filter", "SAX exception caught while importing:" << r.Message);
#endif
        return SD_XML_READERROR;
    }
    catch (const packages::zip::ZipIOException& r)
    {
#if OSL_DEBUG_LEVEL > 1
        SAL_WARN( "sd.filter", "Zip exception caught while importing:" << r.Message);
#else
        (void)r;
#endif
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch (const io::IOException& r)
    {
#if OSL_DEBUG_LEVEL > 1
        SAL_WARN( "sd.filter", "IO exception caught while importing:" << r.Message);
#else
        (void)r;
#endif
        return SD_XML_READERROR;
    }
    catch (const uno::Exception& r)
    {
#if OSL_DEBUG_LEVEL > 1
        SAL_WARN( "sd.filter", "uno exception caught while importing:" << r.Message);
#else
        (void)r;
#endif
        return SD_XML_READERROR;
    }

    // success!
    return 0;
}

sal_Int32 ReadThroughComponent(
    const uno::Reference < embed::XStorage >& xStorage,
    Reference<XComponent> xModelComponent,
    const sal_Char* pStreamName,
    const sal_Char* pCompatibilityStreamName,
    Reference<uno::XComponentContext> & rxContext,
    const sal_Char* pFilterName,
    const Sequence<Any>& rFilterArguments,
    const OUString& rName,
    bool bMustBeSuccessfull )
{
    DBG_ASSERT(xStorage.is(), "Need storage!");
    DBG_ASSERT(NULL != pStreamName, "Please, please, give me a name!");

    // open stream (and set parser input)
    OUString sStreamName = OUString::createFromAscii(pStreamName);
    bool bContainsStream = false;
    try
    {
        bContainsStream = xStorage->isStreamElement(sStreamName);
    }
    catch (const container::NoSuchElementException&)
    {
    }

    if (!bContainsStream )
    {
        // stream name not found! Then try the compatibility name.
        // if no stream can be opened, return immediately with OK signal

        // do we even have an alternative name?
        if ( NULL == pCompatibilityStreamName )
            return 0;

        // if so, does the stream exist?
        sStreamName = OUString::createFromAscii(pCompatibilityStreamName);
        try
        {
            bContainsStream = xStorage->isStreamElement(sStreamName);
        }
        catch (const container::NoSuchElementException&)
        {
        }

        if (! bContainsStream )
            return 0;
    }

    // set Base URL
    uno::Reference< beans::XPropertySet > xInfoSet;
    if( rFilterArguments.getLength() > 0 )
        rFilterArguments.getConstArray()[0] >>= xInfoSet;
    DBG_ASSERT( xInfoSet.is(), "missing property set" );
    if( xInfoSet.is() )
    {
        OUString sPropName( "StreamName" );
        xInfoSet->setPropertyValue( sPropName, makeAny( sStreamName ) );
    }

    try
    {
        // get input stream
        Reference <io::XStream> xStream =
                xStorage->openStreamElement( sStreamName, embed::ElementModes::READ );
        Reference <beans::XPropertySet > xProps( xStream, uno::UNO_QUERY );
        if ( !xStream.is() || ! xProps.is() )
            return SD_XML_READERROR;

        Any aAny = xProps->getPropertyValue( "Encrypted" );

        bool bEncrypted = aAny.getValueType() == ::getBooleanCppuType() &&
                *(sal_Bool *)aAny.getValue();

        Reference <io::XInputStream> xInputStream = xStream->getInputStream();

        // read from the stream
        return ReadThroughComponent(
            xInputStream, xModelComponent, sStreamName, rxContext,
            pFilterName, rFilterArguments,
            rName, bMustBeSuccessfull, bEncrypted );
    }
    catch (const packages::WrongPasswordException&)
    {
        return ERRCODE_SFX_WRONGPASSWORD;
    }
    catch (const packages::zip::ZipIOException&)
    {
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch (const uno::Exception&)
    {}

    return SD_XML_READERROR;
}



bool SdXMLFilter::Import( ErrCode& nError )
{
    sal_uInt32  nRet = 0;

    // Get service factory
    Reference< uno::XComponentContext > rxContext =
            comphelper::getProcessComponentContext();



    SdDrawDocument* pDoc = mrDocShell.GetDoc();
    pDoc->EnableUndo(false);
    pDoc->NewOrLoadCompleted( NEW_DOC );
    pDoc->CreateFirstPages();
    pDoc->StopWorkStartupDelay();



    mxModel->lockControllers();



    /** property map for import info set */
    PropertyMapEntry const aImportInfoMap[] =
    {
        // necessary properties for XML progress bar at load time
        { OUString("ProgressRange"),   0, ::cppu::UnoType<sal_Int32>::get(), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString("ProgressMax"),     0, ::cppu::UnoType<sal_Int32>::get(), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString("ProgressCurrent"), 0, ::cppu::UnoType<sal_Int32>::get(), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString("Preview"),         0, ::cppu::UnoType<sal_Bool>::get(),  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString("PageLayouts"), 0, cppu::UnoType<container::XNameAccess>::get(),  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,     0},
        { OUString("PrivateData"), 0,
              cppu::UnoType<XInterface>::get(),
              ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("BaseURI"), 0,
              ::cppu::UnoType<OUString>::get(),
              ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("StreamRelPath"), 0,
              ::cppu::UnoType<OUString>::get(),
              ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("StreamName"), 0,
              ::cppu::UnoType<OUString>::get(),
              ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("BuildId"), 0,
              ::cppu::UnoType<OUString>::get(),
              ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("OrganizerMode"), 0,
              ::getBooleanCppuType(),
              ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("SourceStorage"), 0, cppu::UnoType<embed::XStorage>::get(),
          ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    uno::Reference< beans::XPropertySet > xInfoSet( GenericPropertySet_CreateInstance( new PropertySetInfo( aImportInfoMap ) ) );
    xInfoSet->setPropertyValue( "Preview" , uno::makeAny( mrDocShell.GetDoc()->IsStarDrawPreviewMode() ) );

    // ---- get BuildId from parent container if available

    uno::Reference< container::XChild > xChild( mxModel, uno::UNO_QUERY );
    if( xChild.is() )
    {
        uno::Reference< beans::XPropertySet > xParentSet( xChild->getParent(), uno::UNO_QUERY );
        if( xParentSet.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xParentSet->getPropertySetInfo() );
            OUString sPropName( "BuildId" );
            if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(sPropName) )
            {
                xInfoSet->setPropertyValue( sPropName, xParentSet->getPropertyValue(sPropName) );
            }
        }
    }



    Reference< io::XActiveDataSource > xSource;
    Reference< XInterface > xPipe;
    Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = 0;
    Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

    Reference< lang::XComponent > xModelComp( mxModel, uno::UNO_QUERY );



    // try to get an XStatusIndicator from the Medium
    if( mbShowProgress )
    {
        SfxItemSet* pSet = mrMedium.GetItemSet();
        if(pSet)
        {
            const SfxUnoAnyItem* pItem = static_cast<const SfxUnoAnyItem*>(
                pSet->GetItem(SID_PROGRESS_STATUSBAR_CONTROL) );
            if (pItem)
            {
                pItem->GetValue() >>= mxStatusIndicator;
            }
        }

        if(mxStatusIndicator.is())
        {
            sal_Int32 nProgressRange(1000000);
            sal_Int32 nProgressCurrent(0);
            OUString aMsg(SD_RESSTR(STR_LOAD_DOC));
            mxStatusIndicator->start(aMsg, nProgressRange);

            // set ProgressRange
            uno::Any aProgRange;
            aProgRange <<= nProgressRange;
            xInfoSet->setPropertyValue( "ProgressRange" , aProgRange);

            // set ProgressCurrent
            uno::Any aProgCurrent;
            aProgCurrent <<= nProgressCurrent;
            xInfoSet->setPropertyValue( "ProgressCurrent" , aProgCurrent);
        }
    }


    // get the input stream (storage or stream)


    SvStorageStreamRef xDocStream;
    Reference<io::XInputStream> xInputStream;
    uno::Reference < embed::XStorage > xStorage = mrMedium.GetStorage();

    OUString sSourceStorage( "SourceStorage");
    xInfoSet->setPropertyValue( sSourceStorage, Any( xStorage ) );

    if( !xStorage.is() )
        nRet = SD_XML_READERROR;

    if( 0 == nRet )
    {
        pGraphicHelper = SvXMLGraphicHelper::Create( xStorage,
                                                     GRAPHICHELPER_MODE_READ,
                                                     false );
        xGraphicResolver = pGraphicHelper;
        pObjectHelper = SvXMLEmbeddedObjectHelper::Create(
                                    xStorage, *pDoc->GetPersist(),
                                    EMBEDDEDOBJECTHELPER_MODE_READ,
                                    false );
        xObjectResolver = pObjectHelper;
    }

    // Set base URI
    xInfoSet->setPropertyValue( "BaseURI" , makeAny( mrMedium.GetBaseURL() ) );

    if( 0 == nRet && SFX_CREATE_MODE_EMBEDDED == mrDocShell.GetCreateMode() )
    {
        OUString aName;
        if ( mrMedium.GetItemSet() )
        {
            const SfxStringItem* pDocHierarchItem = static_cast<const SfxStringItem*>(
                mrMedium.GetItemSet()->GetItem(SID_DOC_HIERARCHICALNAME) );
            if ( pDocHierarchItem )
                aName = pDocHierarchItem->GetValue();
        }
        else
            aName = "dummyObjectName" ;

        if( !aName.isEmpty() )
            xInfoSet->setPropertyValue( "StreamRelPath", Any( aName ) );
    }

    if (SDXMLMODE_Organizer == meFilterMode)
        xInfoSet->setPropertyValue("OrganizerMode", uno::makeAny(sal_True));



    if( 0 == nRet )
    {

        // prepare filter arguments
        Sequence<Any> aFilterArgs( 4 );
        Any *pArgs = aFilterArgs.getArray();
        *pArgs++ <<= xInfoSet;
        *pArgs++ <<= xGraphicResolver;
        *pArgs++ <<= xObjectResolver;
        *pArgs++ <<= mxStatusIndicator;

        Sequence<Any> aEmptyArgs( 2 );
        pArgs = aEmptyArgs.getArray();
        *pArgs++ <<= xInfoSet;
        *pArgs++ <<= mxStatusIndicator;

        const OUString aName( mrMedium.GetName() );

        XML_SERVICES* pServices = getServices( true, IsDraw(), mnStoreVer );

        sal_uInt32 nWarn = 0;
        sal_uInt32 nWarn2 = 0;
        // read storage streams
        // #i103539#: always read meta.xml for generator
        nWarn = ReadThroughComponent(
            xStorage, xModelComp, "meta.xml", "Meta.xml", rxContext,
            pServices->mpMeta,
            aEmptyArgs, aName, false );

        if( meFilterMode != SDXMLMODE_Organizer )
        {
            nWarn2 = ReadThroughComponent(
                xStorage, xModelComp, "settings.xml", NULL, rxContext,
                pServices->mpSettings,
                aFilterArgs, aName, false );
        }

        nRet = ReadThroughComponent(
            xStorage, xModelComp, "styles.xml", NULL, rxContext,
            pServices->mpStyles,
            aFilterArgs, aName, true );

        if( !nRet && (meFilterMode != SDXMLMODE_Organizer) )
            nRet = ReadThroughComponent(
               xStorage, xModelComp, "content.xml", "Content.xml", rxContext,
               pServices->mpContent,
               aFilterArgs, aName, true );

        if( !nRet )
        {
            if( nWarn )
                nRet = nWarn;
            else if( nWarn2 )
                nRet = nWarn2;
        }
    }


    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );
    xGraphicResolver = 0;
    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
    xObjectResolver = 0;

    if( mxStatusIndicator.is() )
        mxStatusIndicator->end();

    if( mxModel.is() )
        mxModel->unlockControllers();

    if( nRet == 0 )
        pDoc->UpdateAllLinks();

    switch( nRet )
    {
    case 0: break;
    case SD_XML_READERROR: break;
    case ERRCODE_IO_BROKENPACKAGE:
        if( xStorage.is() )
        {
            nError = ERRCODE_IO_BROKENPACKAGE;
            break;
        }
        // fall through intented
    default:
        {
            // TODO/LATER: this is completely wrong! Filter code should never call ErrorHandler directly!
            ErrorHandler::HandleError( nRet );
            if( IsWarning( nRet ) )
                nRet = 0;
        }
    }


    // clear unused named items from item pool

    uno::Reference< lang::XMultiServiceFactory> xModelFactory( mxModel, uno::UNO_QUERY );
    if( xModelFactory.is() )
    {
        try
        {
            const OUString aName("~clear~" );
            uno::Reference< container::XNameContainer > xGradient( xModelFactory->createInstance( "com.sun.star.drawing.GradientTable" ), uno::UNO_QUERY );
            if( xGradient.is() )
                xGradient->removeByName( aName );

            uno::Reference< container::XNameContainer > xHatch( xModelFactory->createInstance( "com.sun.star.drawing.HatchTable" ), uno::UNO_QUERY );
            if( xHatch.is() )
                xHatch->removeByName( aName );

            uno::Reference< container::XNameContainer > xBitmap( xModelFactory->createInstance( "com.sun.star.drawing.BitmapTable" ), uno::UNO_QUERY );
            if( xBitmap.is() )
                xBitmap->removeByName( aName );

            uno::Reference< container::XNameContainer > xTransGradient( xModelFactory->createInstance( "com.sun.star.drawing.TransparencyGradientTable" ), uno::UNO_QUERY );
            if( xTransGradient.is() )
                xTransGradient->removeByName( aName );

            uno::Reference< container::XNameContainer > xMarker( xModelFactory->createInstance( "com.sun.star.drawing.MarkerTable" ), uno::UNO_QUERY );
            if( xMarker.is() )
                xMarker->removeByName( aName );

            uno::Reference< container::XNameContainer > xDashes( xModelFactory->createInstance( "com.sun.star.drawing.DashTable" ), uno::UNO_QUERY );
            if( xDashes.is() )
                xDashes->removeByName( aName );
        }
        catch (const Exception&)
        {
            SAL_WARN( "sd.filter","sd::SdXMLFilter::Import(), exception during clearing of unused named items");
        }
    }

    // set BuildId on XModel for later OLE object loading
    if( xInfoSet.is() )
    {
        uno::Reference< beans::XPropertySet > xModelSet( mxModel, uno::UNO_QUERY );
        if( xModelSet.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xModelSetInfo( xModelSet->getPropertySetInfo() );
            const OUString sPropName( "BuildId" );

            OUString sBuildId;
            xInfoSet->getPropertyValue(sPropName) >>= sBuildId;

            if( xModelSetInfo.is() && xModelSetInfo->hasPropertyByName(sPropName) )
            {
                xModelSet->setPropertyValue( sPropName, Any( sBuildId ) );
            }

            bool bTransform = false;

            if( nRet == 0 )
            {
                if( !sBuildId.isEmpty() )
                {
                    sal_Int32 nIndex = sBuildId.indexOf('$');
                    if( nIndex != -1 )
                    {
                        sal_Int32 nUPD = sBuildId.copy( 0, nIndex ).toInt32();

                        if( nUPD == 300 )
                        {
                            sal_Int32 nBuildId = sBuildId.copy( nIndex+1 ).toInt32();
                            if( (nBuildId > 0) && (nBuildId < 9316) )
                                bTransform = true; // treat OOo 3.0 beta1 as OOo 2.x
                        }
                        else if( (nUPD == 680) || ( nUPD >= 640 && nUPD <= 645 ) )
                            bTransform = true;
                    }
                }
                else
                {
                    // check for binary formats
                     const SfxFilter * pFilter = mrMedium.GetFilter();
                    if( pFilter )
                    {
                        OUString typeName(pFilter->GetRealTypeName());
                        if( typeName.startsWith( "impress_StarImpress" ) ||
                            typeName.startsWith( "draw_StarDraw" ) )
                        {
                            bTransform = true;
                        }
                    }
                }
            }

            if( bTransform )
                TransformOOo2xDocument( pDoc );
        }
    }

    pDoc->EnableUndo(true);
    mrDocShell.ClearUndoBuffer();
    return nRet == 0;
}



bool SdXMLFilter::Export()
{
    SvXMLEmbeddedObjectHelper*  pObjectHelper = NULL;
    SvXMLGraphicHelper*         pGraphicHelper = NULL;
    bool                    bDocRet = false;

    if( !mxModel.is() )
    {
        SAL_WARN( "sd.filter","Got NO Model in XMLExport");
        return false;
    }

    bool bLocked = mxModel->hasControllersLocked();

    try
    {
        mxModel->lockControllers();

        uno::Reference< lang::XServiceInfo > xServiceInfo( mxModel, uno::UNO_QUERY );

        if( !xServiceInfo.is() || !xServiceInfo->supportsService( "com.sun.star.drawing.GenericDrawingDocument" ) )
        {
            SAL_WARN( "sd.filter", "Model is no DrawingDocument in XMLExport" );
            return false;
        }

        uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

        uno::Reference< xml::sax::XWriter > xWriter = xml::sax::Writer::create( xContext );

        /** property map for export info set */
        PropertyMapEntry const aExportInfoMap[] =
        {
            { OUString("ProgressRange"),   0, ::cppu::UnoType<sal_Int32>::get(), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { OUString("ProgressMax"),     0, ::cppu::UnoType<sal_Int32>::get(), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { OUString("ProgressCurrent"), 0, ::cppu::UnoType<sal_Int32>::get(), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { OUString("UsePrettyPrinting"),0, ::getBooleanCppuType(),             ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},

            { OUString("PageLayoutNames"), 0, ::cppu::UnoType<OUString>::get(),  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,     0},
            { OUString("BaseURI"), 0,
                  ::cppu::UnoType<OUString>::get(),
                  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString("StreamRelPath"), 0,
                  ::cppu::UnoType<OUString>::get(),
                  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString("StreamName"), 0,
                  ::cppu::UnoType<OUString>::get(),
                  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString("StyleNames"), 0,
                  ::getCppuType( (Sequence<OUString>*)0 ),
                  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString("StyleFamilies"), 0,
                  ::getCppuType( (Sequence<sal_Int32>*)0 ),
                  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString("TargetStorage"), 0, cppu::UnoType<embed::XStorage>::get(),
                  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString(), 0, css::uno::Type(), 0, 0 }
        };

        uno::Reference< beans::XPropertySet > xInfoSet( GenericPropertySet_CreateInstance( new PropertySetInfo( aExportInfoMap ) ) );


        SvtSaveOptions aSaveOpt;
        OUString sUsePrettyPrinting("UsePrettyPrinting");
        bool bUsePrettyPrinting( aSaveOpt.IsPrettyPrinting() );
        xInfoSet->setPropertyValue( sUsePrettyPrinting, makeAny( bUsePrettyPrinting ) );

        const uno::Reference < embed::XStorage >& xStorage = mrMedium.GetOutputStorage();

        // Set base URI
        OUString sPropName( "BaseURI" );
        xInfoSet->setPropertyValue( sPropName, makeAny( mrMedium.GetBaseURL( true ) ) );

        OUString sTargetStorage( "TargetStorage" );
        xInfoSet->setPropertyValue( sTargetStorage, Any( xStorage ) );

        if( SFX_CREATE_MODE_EMBEDDED == mrDocShell.GetCreateMode() )
        {
            OUString aName;
            if ( mrMedium.GetItemSet() )
            {
                const SfxStringItem* pDocHierarchItem = static_cast<const SfxStringItem*>(
                    mrMedium.GetItemSet()->GetItem(SID_DOC_HIERARCHICALNAME) );
                if ( pDocHierarchItem )
                    aName = pDocHierarchItem->GetValue();
            }

            if( !aName.isEmpty() )
            {
                sPropName = "StreamRelPath";
                xInfoSet->setPropertyValue( sPropName, makeAny( aName ) );
            }
        }

        // initialize descriptor
        uno::Sequence< beans::PropertyValue > aDescriptor( 1 );
        beans::PropertyValue* pProps = aDescriptor.getArray();

        pProps[0].Name = "FileName";
        pProps[0].Value <<= OUString( mrMedium.GetName() );

        {
            uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
            uno::Reference< document::XGraphicObjectResolver >  xGrfResolver;

            // create helper for graphic and ole export if we have a storage
            if( xStorage.is() )
            {
                pObjectHelper = SvXMLEmbeddedObjectHelper::Create( xStorage, *mrDocShell.GetDoc()->GetPersist(), EMBEDDEDOBJECTHELPER_MODE_WRITE, false );
                xObjectResolver = pObjectHelper;

                pGraphicHelper = SvXMLGraphicHelper::Create( xStorage, GRAPHICHELPER_MODE_WRITE, false );
                xGrfResolver = pGraphicHelper;
            }

            if(mbShowProgress)
            {
                CreateStatusIndicator();
                if(mxStatusIndicator.is())
                {
                    sal_Int32 nProgressRange(1000000);
                    sal_Int32 nProgressCurrent(0);
                    OUString aMsg(SD_RESSTR(STR_SAVE_DOC));
                    mxStatusIndicator->start(aMsg, nProgressRange);

                    // set ProgressRange
                    uno::Any aProgRange;
                    aProgRange <<= nProgressRange;
                    xInfoSet->setPropertyValue( "ProgressRange" , aProgRange);

                    // set ProgressCurrent
                    uno::Any aProgCurrent;
                    aProgCurrent <<= nProgressCurrent;
                    xInfoSet->setPropertyValue( "ProgressCurrent" , aProgCurrent);
                }
            }

            uno::Reference< lang::XComponent > xComponent( mxModel, uno::UNO_QUERY );

            XML_SERVICES* pServiceNames = getServices( false, IsDraw(), mnStoreVer );

            XML_SERVICEMAP aServices[5]; sal_uInt16 i = 0;
            aServices[i  ].mpService = pServiceNames->mpStyles;
            aServices[i++].mpStream  = sXML_styleStreamName;

            aServices[i  ].mpService = pServiceNames->mpContent;
            aServices[i++].mpStream  = sXML_contentStreamName;

            aServices[i  ].mpService = pServiceNames->mpSettings;
            aServices[i++].mpStream  = sXML_settingsStreamName;

            if( mrDocShell.GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
            {
                aServices[i  ].mpService = pServiceNames->mpMeta;
                aServices[i++].mpStream  = sXML_metaStreamName;
            };

            aServices[i].mpService = NULL;
            aServices[i].mpStream  = NULL;

            XML_SERVICEMAP* pServices = aServices;

            // doc export
            do
            {
                SAL_INFO( "sd.filter", "exporting substream " << pServices->mpStream );

                uno::Reference<io::XOutputStream> xDocOut;
                if( xStorage.is() )
                {
                    const OUString sDocName( OUString::createFromAscii( pServices->mpStream ) );
                    uno::Reference<io::XStream> xStream =
                            xStorage->openStreamElement( sDocName,
                            embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );

                    DBG_ASSERT(xStream.is(), "Can't create output stream in package!");
                    if( !xStream.is() )
                        return false;

                    xDocOut = xStream->getOutputStream();
                    Reference <beans::XPropertySet > xProps( xStream, uno::UNO_QUERY );
                    if( !xDocOut.is() || !xProps.is() )
                        return false;

                    uno::Any aAny;
                    aAny <<= OUString( "text/xml");
                    xProps->setPropertyValue( "MediaType" , aAny);

                    // encrypt all streams
                    xProps->setPropertyValue( "UseCommonStoragePasswordEncryption",
                                              uno::makeAny( true ) );

                    const OUString sStreamName( "StreamName");
                    xInfoSet->setPropertyValue( sStreamName, Any( sDocName ) );
                }

                uno::Reference< io::XActiveDataSource > xDocSrc( xWriter, uno::UNO_QUERY );
                xDocSrc->setOutputStream( xDocOut );

                uno::Sequence< uno::Any > aArgs( 2 + ( mxStatusIndicator.is() ? 1 : 0 ) + ( xGrfResolver.is() ? 1 : 0 ) + ( xObjectResolver.is() ? 1 : 0 ) );
                uno::Any* pArgs = aArgs.getArray();
                *pArgs++ <<= xInfoSet;
                if( xGrfResolver.is() )         *pArgs++ <<= xGrfResolver;
                if( xObjectResolver.is() )      *pArgs++ <<= xObjectResolver;
                if( mxStatusIndicator.is() )    *pArgs++ <<= mxStatusIndicator;

                *pArgs   <<= xWriter;

                uno::Reference< document::XFilter > xFilter( xContext->getServiceManager()->createInstanceWithArgumentsAndContext( OUString::createFromAscii( pServices->mpService ), aArgs, xContext ), uno::UNO_QUERY );
                if( xFilter.is() )
                {
                    uno::Reference< document::XExporter > xExporter( xFilter, uno::UNO_QUERY );
                    if( xExporter.is() )
                    {
                        xExporter->setSourceDocument( xComponent );
                        // outputstream will be closed by SAX parser
                        bDocRet = xFilter->filter( aDescriptor );
                    }
                }

                pServices++;
            }
            while( bDocRet && pServices->mpService );

            if(mbShowProgress)
            {
                if(mxStatusIndicator.is())
                    mxStatusIndicator->end();
            }
        }
    }
    catch (const uno::Exception &e)
    {
#if OSL_DEBUG_LEVEL > 1
        SAL_WARN( "sd.filter", "uno Exception caught while exporting:" << e.Message);
#else
        (void)e;
#endif
        bDocRet = false;
    }
    if ( !bLocked )
        mxModel->unlockControllers();

    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );

    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );

    return bDocRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
