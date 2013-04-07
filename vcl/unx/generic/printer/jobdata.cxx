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


#include "vcl/jobdata.hxx"
#include "vcl/printerinfomanager.hxx"

#include "tools/stream.hxx"

#include <sal/alloca.h>
#include <rtl/strbuf.hxx>
#include <comphelper/string.hxx>

using namespace psp;

JobData& JobData::operator=(const JobData& rRight)
{
    m_nCopies               = rRight.m_nCopies;
    m_nLeftMarginAdjust     = rRight.m_nLeftMarginAdjust;
    m_nRightMarginAdjust    = rRight.m_nRightMarginAdjust;
    m_nTopMarginAdjust      = rRight.m_nTopMarginAdjust;
    m_nBottomMarginAdjust   = rRight.m_nBottomMarginAdjust;
    m_nColorDepth           = rRight.m_nColorDepth;
    m_eOrientation          = rRight.m_eOrientation;
    m_aPrinterName          = rRight.m_aPrinterName;
    m_pParser               = rRight.m_pParser;
    m_aContext              = rRight.m_aContext;
    m_nPSLevel              = rRight.m_nPSLevel;
    m_nPDFDevice            = rRight.m_nPDFDevice;
    m_nColorDevice          = rRight.m_nColorDevice;

    if( ! m_pParser && m_aPrinterName.getLength() )
    {
        PrinterInfoManager& rMgr = PrinterInfoManager::get();
        rMgr.setupJobContextData( *this );
    }
    return *this;
}

void JobData::setCollate( bool bCollate )
{
    const PPDParser* pParser = m_aContext.getParser();
    if( pParser )
    {
        const PPDKey* pKey = pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Collate" ) ) );
        if( pKey )
        {
            const PPDValue* pVal = NULL;
            if( bCollate )
                pVal = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "True" ) ) );
            else
            {
                pVal = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "False" ) ) );
                if( ! pVal )
                    pVal = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "None" ) ) );
            }
            m_aContext.setValue( pKey, pVal );
        }
    }
}

bool JobData::setPaper( int i_nWidth, int i_nHeight )
{
    bool bSuccess = false;
    if( m_pParser )
    {
        rtl::OUString aPaper( m_pParser->matchPaper( i_nWidth, i_nHeight ) );

        const PPDKey*   pKey = m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) );
        const PPDValue* pValue = pKey ? pKey->getValueCaseInsensitive( aPaper ) : NULL;

        bSuccess = pKey && pValue && m_aContext.setValue( pKey, pValue, false );
    }
    return bSuccess;
}

bool JobData::setPaperBin( int i_nPaperBin )
{
    bool bSuccess = false;
    if( m_pParser )
    {
        const PPDKey*   pKey = m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) );
        const PPDValue* pValue = pKey ? pKey->getValue( i_nPaperBin ) : NULL;

        bSuccess = pKey && pValue && m_aContext.setValue( pKey, pValue, false );
    }
    return bSuccess;
}

bool JobData::getStreamBuffer( void*& pData, int& bytes )
{
    // consistency checks
    if( ! m_pParser )
        m_pParser = m_aContext.getParser();
    if( m_pParser != m_aContext.getParser() ||
        ! m_pParser )
        return false;

    SvMemoryStream aStream;

    // write header job data
    aStream.WriteLine( "JobData 1" );

    rtl::OStringBuffer aLine;

    aLine.append(RTL_CONSTASCII_STRINGPARAM("printer="));
    aLine.append(rtl::OUStringToOString(m_aPrinterName, RTL_TEXTENCODING_UTF8));
    aStream.WriteLine(aLine.makeStringAndClear());

    aLine.append(RTL_CONSTASCII_STRINGPARAM("orientation="));
    if (m_eOrientation == orientation::Landscape)
        aLine.append(RTL_CONSTASCII_STRINGPARAM("Landscape"));
    else
        aLine.append(RTL_CONSTASCII_STRINGPARAM("Portrait"));
    aStream.WriteLine(aLine.makeStringAndClear());

    aLine.append(RTL_CONSTASCII_STRINGPARAM("copies="));
    aLine.append(static_cast<sal_Int32>(m_nCopies));
    aStream.WriteLine(aLine.makeStringAndClear());

    aLine.append(RTL_CONSTASCII_STRINGPARAM("margindajustment="));
    aLine.append(static_cast<sal_Int32>(m_nLeftMarginAdjust));
    aLine.append(',');
    aLine.append(static_cast<sal_Int32>(m_nRightMarginAdjust));
    aLine.append(',');
    aLine.append(static_cast<sal_Int32>(m_nTopMarginAdjust));
    aLine.append(',');
    aLine.append(static_cast<sal_Int32>(m_nBottomMarginAdjust));
    aStream.WriteLine(aLine.makeStringAndClear());

    aLine.append(RTL_CONSTASCII_STRINGPARAM("colordepth="));
    aLine.append(static_cast<sal_Int32>(m_nColorDepth));
    aStream.WriteLine(aLine.makeStringAndClear());

    aLine.append(RTL_CONSTASCII_STRINGPARAM("pslevel="));
    aLine.append(static_cast<sal_Int32>(m_nPSLevel));
    aStream.WriteLine(aLine.makeStringAndClear());

    aLine.append(RTL_CONSTASCII_STRINGPARAM("pdfdevice="));
    aLine.append(static_cast<sal_Int32>(m_nPDFDevice));
    aStream.WriteLine(aLine.makeStringAndClear());

    aLine.append(RTL_CONSTASCII_STRINGPARAM("colordevice="));
    aLine.append(static_cast<sal_Int32>(m_nColorDevice));
    aStream.WriteLine(aLine.makeStringAndClear());

    // now append the PPDContext stream buffer
    aStream.WriteLine( "PPDContexData" );
    sal_uLong nBytes;
    char* pContextBuffer = m_aContext.getStreamableBuffer( nBytes );
    if( nBytes )
        aStream.Write( pContextBuffer, nBytes );
    delete [] pContextBuffer;

    // success
    pData = rtl_allocateMemory( bytes = aStream.Tell() );
    memcpy( pData, aStream.GetData(), bytes );
    return true;
}

bool JobData::constructFromStreamBuffer( void* pData, int bytes, JobData& rJobData )
{
    SvMemoryStream aStream( pData, bytes, STREAM_READ );
    rtl::OString aLine;
    bool bVersion       = false;
    bool bPrinter       = false;
    bool bOrientation   = false;
    bool bCopies        = false;
    bool bContext       = false;
    bool bMargin        = false;
    bool bColorDepth    = false;
    bool bColorDevice   = false;
    bool bPSLevel       = false;
    bool bPDFDevice     = false;

    const char printerEquals[] = "printer=";
    const char orientatationEquals[] = "orientation=";
    const char copiesEquals[] = "copies=";
    const char margindajustmentEquals[] = "margindajustment=";
    const char colordepthEquals[] = "colordepth=";
    const char colordeviceEquals[] = "colordevice=";
    const char pslevelEquals[] = "pslevel=";
    const char pdfdeviceEquals[] = "pdfdevice=";

    using comphelper::string::matchL;

    while( ! aStream.IsEof() )
    {
        aStream.ReadLine( aLine );
        if (matchL(aLine, RTL_CONSTASCII_STRINGPARAM("JobData")))
            bVersion = true;
        else if (matchL(aLine, RTL_CONSTASCII_STRINGPARAM(printerEquals)))
        {
            bPrinter = true;
            rJobData.m_aPrinterName = rtl::OStringToOUString(aLine.copy(RTL_CONSTASCII_LENGTH(printerEquals)), RTL_TEXTENCODING_UTF8);
        }
        else if (matchL(aLine, RTL_CONSTASCII_STRINGPARAM(orientatationEquals)))
        {
            bOrientation = true;
            rJobData.m_eOrientation = aLine.copy(RTL_CONSTASCII_LENGTH(orientatationEquals)).equalsIgnoreAsciiCase("landscape") ? orientation::Landscape : orientation::Portrait;
        }
        else if (matchL(aLine, RTL_CONSTASCII_STRINGPARAM(copiesEquals)))
        {
            bCopies = true;
            rJobData.m_nCopies = aLine.copy(RTL_CONSTASCII_LENGTH(copiesEquals)).toInt32();
        }
        else if (matchL(aLine, RTL_CONSTASCII_STRINGPARAM(margindajustmentEquals)))
        {
            bMargin = true;
            rtl::OString aValues(aLine.copy(RTL_CONSTASCII_LENGTH(margindajustmentEquals)));
            using comphelper::string::getToken;
            rJobData.m_nLeftMarginAdjust = getToken(aValues, 0, ',').toInt32();
            rJobData.m_nRightMarginAdjust = getToken(aValues, 1, ',').toInt32();
            rJobData.m_nTopMarginAdjust = getToken(aValues, 2, ',').toInt32();
            rJobData.m_nBottomMarginAdjust = getToken(aValues, 3, ',').toInt32();
        }
        else if (matchL(aLine, RTL_CONSTASCII_STRINGPARAM(colordepthEquals)))
        {
            bColorDepth = true;
            rJobData.m_nColorDepth = aLine.copy(RTL_CONSTASCII_LENGTH(colordepthEquals)).toInt32();
        }
        else if (matchL(aLine, RTL_CONSTASCII_STRINGPARAM(colordeviceEquals)))
        {
            bColorDevice = true;
            rJobData.m_nColorDevice = aLine.copy(RTL_CONSTASCII_LENGTH(colordeviceEquals)).toInt32();
        }
        else if (matchL(aLine, RTL_CONSTASCII_STRINGPARAM(pslevelEquals)))
        {
            bPSLevel = true;
            rJobData.m_nPSLevel = aLine.copy(RTL_CONSTASCII_LENGTH(pslevelEquals)).toInt32();
        }
        else if (matchL(aLine, RTL_CONSTASCII_STRINGPARAM(pdfdeviceEquals)))
        {
            bPDFDevice = true;
            rJobData.m_nPDFDevice = aLine.copy(RTL_CONSTASCII_LENGTH(pdfdeviceEquals)).toInt32();
        }
        else if (aLine.equalsL(RTL_CONSTASCII_STRINGPARAM("PPDContexData")))
        {
            if( bPrinter )
            {
                PrinterInfoManager& rManager = PrinterInfoManager::get();
                const PrinterInfo& rInfo = rManager.getPrinterInfo( rJobData.m_aPrinterName );
                rJobData.m_pParser = PPDParser::getParser( rInfo.m_aDriverName );
                if( rJobData.m_pParser )
                {
                    rJobData.m_aContext.setParser( rJobData.m_pParser );
                    int nBytes = bytes - aStream.Tell();
                    char* pRemain = (char*)alloca( bytes - aStream.Tell() );
                    aStream.Read( pRemain, nBytes );
                    rJobData.m_aContext.rebuildFromStreamBuffer( pRemain, nBytes );
                    bContext = true;
                }
            }
        }
    }

    return bVersion && bPrinter && bOrientation && bCopies && bContext && bMargin && bPSLevel && bPDFDevice && bColorDevice && bColorDepth;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
