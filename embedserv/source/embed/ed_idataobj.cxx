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
#if defined(_MSC_VER) && (_MSC_VER > 1310)
#pragma warning(disable : 4917 4555)
#endif

// actually this workaround should be in presys.h!
//#define UINT64 USE_WIN_UINT64
//#define INT64 USE_WIN_INT64
//#define UINT32 USE_WIN_UINT32
//#define INT32 USE_WIN_INT32

#include "embeddoc.hxx"

//#undef UINT64
//#undef INT64
//#undef UINT32
//#undef INT32


#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>


#include <osl/thread.h>

using namespace ::com::sun::star;

//===============================================================================
// EmbedDocument_Impl
//===============================================================================

sal_uInt64 EmbedDocument_Impl::getMetaFileHandle_Impl( sal_Bool isEnhMeta )
{
    sal_uInt64 pResult = NULL;

    uno::Reference< datatransfer::XTransferable > xTransferable( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
    if ( xTransferable.is() )
    {
        uno::Sequence< sal_Int8 > aMetaBuffer;
        datatransfer::DataFlavor aFlavor;

        if ( isEnhMeta )
        {
            aFlavor.MimeType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                "application/x-openoffice-emf;windows_formatname=\"Image EMF\"" ) );
            aFlavor.HumanPresentableName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Enhanced Windows MetaFile" ) );
        }
        else
        {
            aFlavor.MimeType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"" ) );
            aFlavor.HumanPresentableName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Windows GDIMetaFile" ) );
        }

        aFlavor.DataType = getCppuType( (const sal_uInt64*) 0 );

        uno::Any aAny = xTransferable->getTransferData( aFlavor );
        aAny >>= pResult;
    }

    return pResult;
}

//-------------------------------------------------------------------------------
// IDataObject

STDMETHODIMP EmbedDocument_Impl::GetData( FORMATETC * pFormatetc, STGMEDIUM * pMedium )
{
    if ( !pFormatetc )
        return DV_E_FORMATETC;

    if ( !pMedium )
        return STG_E_MEDIUMFULL;

    if ( pFormatetc->dwAspect == DVASPECT_THUMBNAIL
      || pFormatetc->dwAspect == DVASPECT_ICON
      || pFormatetc->dwAspect == DVASPECT_DOCPRINT )
        return DV_E_DVASPECT;

    if ( pFormatetc->cfFormat == CF_ENHMETAFILE )
    {
        if ( !( pFormatetc->tymed & TYMED_ENHMF ) )
            return DV_E_TYMED;

        HENHMETAFILE hMeta = reinterpret_cast<HENHMETAFILE>( getMetaFileHandle_Impl( sal_True ) );

        if ( hMeta )
        {
            pMedium->tymed = TYMED_ENHMF;
            pMedium->hEnhMetaFile = hMeta;
            pMedium->pUnkForRelease = NULL;

            return S_OK;
        }

        return STG_E_MEDIUMFULL;
    }
    else if ( pFormatetc->cfFormat == CF_METAFILEPICT )
    {
          if ( !( pFormatetc->tymed & TYMED_MFPICT ) )
            return DV_E_TYMED;

        HGLOBAL hMeta = reinterpret_cast<HGLOBAL>( getMetaFileHandle_Impl( sal_False ) );

        if ( hMeta )
        {
            pMedium->tymed = TYMED_MFPICT;
            pMedium->hMetaFilePict = hMeta;
            pMedium->pUnkForRelease = NULL;

            return S_OK;
        }

        return STG_E_MEDIUMFULL;
    }
    else
    {
        CLIPFORMAT cf_embSource = (CLIPFORMAT)RegisterClipboardFormatA( "Embed Source" );
        CLIPFORMAT cf_embObj = (CLIPFORMAT)RegisterClipboardFormatA( "Embedded Object" );
        if ( pFormatetc->cfFormat == cf_embSource || pFormatetc->cfFormat == cf_embObj )
        {
            if ( !( pFormatetc->tymed & TYMED_ISTORAGE ) )
                return DV_E_TYMED;

            CComPtr< IStorage > pNewStg;
            HRESULT hr = StgCreateDocfile( NULL, STGM_CREATE | STGM_READWRITE | STGM_DELETEONRELEASE, 0, &pNewStg );
            if ( FAILED( hr ) || !pNewStg ) return STG_E_MEDIUMFULL;

            hr = SaveTo_Impl( pNewStg );
            if ( FAILED( hr ) ) return STG_E_MEDIUMFULL;

            pMedium->tymed = TYMED_ISTORAGE;
            pMedium->pstg = pNewStg;
            pMedium->pstg->AddRef();
            pMedium->pUnkForRelease = ( IUnknown* )pNewStg;

            return S_OK;
        }
    }

    return DV_E_FORMATETC;
}

STDMETHODIMP EmbedDocument_Impl::GetDataHere( FORMATETC * pFormatetc, STGMEDIUM * pMedium )
{
    if ( !pFormatetc )
        return DV_E_FORMATETC;

    if ( !pMedium )
        return STG_E_MEDIUMFULL;

    if ( pFormatetc->dwAspect == DVASPECT_THUMBNAIL
      || pFormatetc->dwAspect == DVASPECT_ICON
      || pFormatetc->dwAspect == DVASPECT_DOCPRINT )
        return DV_E_DVASPECT;

    CLIPFORMAT cf_embSource = (CLIPFORMAT)RegisterClipboardFormatA( "Embed Source" );
    CLIPFORMAT cf_embObj = (CLIPFORMAT)RegisterClipboardFormatA( "Embedded Object" );

    if ( pFormatetc->cfFormat == cf_embSource || pFormatetc->cfFormat == cf_embObj )
    {
        if ( !( pFormatetc->tymed & TYMED_ISTORAGE ) )
            return DV_E_TYMED;

        if ( !pMedium->pstg ) return STG_E_MEDIUMFULL;

        HRESULT hr = SaveTo_Impl( pMedium->pstg );
        if ( FAILED( hr ) ) return STG_E_MEDIUMFULL;

        pMedium->tymed = TYMED_ISTORAGE;
        pMedium->pUnkForRelease = NULL;

        return S_OK;
    }

    return DV_E_FORMATETC;
}

STDMETHODIMP EmbedDocument_Impl::QueryGetData( FORMATETC * pFormatetc )
{
    if ( pFormatetc )
    {
        if ( pFormatetc->dwAspect == DVASPECT_THUMBNAIL
          || pFormatetc->dwAspect == DVASPECT_ICON
          || pFormatetc->dwAspect == DVASPECT_DOCPRINT )
            return DV_E_DVASPECT;

        if ( pFormatetc->cfFormat == CF_ENHMETAFILE )
        {
            if ( !( pFormatetc->tymed & TYMED_ENHMF ) )
                return DV_E_TYMED;

            return S_OK;
        }
        else if ( pFormatetc->cfFormat == CF_METAFILEPICT )
        {
              if ( !( pFormatetc->tymed & TYMED_MFPICT ) )
                return DV_E_TYMED;

            return S_OK;
        }
        else
        {
            CLIPFORMAT cf_embSource = (CLIPFORMAT)RegisterClipboardFormatA( "Embed Source" );
            CLIPFORMAT cf_embObj = (CLIPFORMAT)RegisterClipboardFormatA( "Embedded Object" );
            if ( pFormatetc->cfFormat == cf_embSource || pFormatetc->cfFormat == cf_embObj )
            {
                if ( !( pFormatetc->tymed & TYMED_ISTORAGE ) )
                    return DV_E_TYMED;

                return S_OK;
            }
        }
    }

    return DV_E_FORMATETC;

}

STDMETHODIMP EmbedDocument_Impl::GetCanonicalFormatEtc( FORMATETC * pFormatetcIn, FORMATETC * pFormatetcOut )
{
    if ( !pFormatetcIn || !pFormatetcOut )
        return DV_E_FORMATETC;

    pFormatetcOut->ptd = NULL;
    pFormatetcOut->cfFormat = pFormatetcIn->cfFormat;
    pFormatetcOut->dwAspect = DVASPECT_CONTENT;

    if ( pFormatetcIn->cfFormat == CF_ENHMETAFILE )
    {
        pFormatetcOut->tymed = TYMED_ENHMF;
        return S_OK;
    }
    else if ( pFormatetcIn->cfFormat == CF_METAFILEPICT )
    {
          pFormatetcOut->tymed = TYMED_MFPICT;
        return S_OK;
    }
    else
    {
        CLIPFORMAT cf_embSource = (CLIPFORMAT)RegisterClipboardFormatA( "Embed Source" );
        CLIPFORMAT cf_embObj = (CLIPFORMAT)RegisterClipboardFormatA( "Embedded Object" );
        if ( pFormatetcIn->cfFormat == cf_embSource || pFormatetcIn->cfFormat == cf_embObj )
        {
            pFormatetcOut->tymed = TYMED_ISTORAGE;
            return S_OK;
        }
    }

    return DV_E_FORMATETC;
}

STDMETHODIMP EmbedDocument_Impl::SetData( FORMATETC * /*pFormatetc*/, STGMEDIUM * /*pMedium*/, BOOL /*fRelease*/ )
{
    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC ** /*ppFormatetc*/ )
{
    if ( dwDirection == DATADIR_GET )
        return OLE_S_USEREG;

    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::DAdvise( FORMATETC * pFormatetc, DWORD advf, IAdviseSink * pAdvSink, DWORD * pdwConnection )
{
    if ( !m_pDAdviseHolder )
        if ( !SUCCEEDED( CreateDataAdviseHolder( &m_pDAdviseHolder ) ) || !m_pDAdviseHolder )
            return E_OUTOFMEMORY;

    return m_pDAdviseHolder->Advise( (IDataObject*)this, pFormatetc, advf, pAdvSink, pdwConnection );
}

STDMETHODIMP EmbedDocument_Impl::DUnadvise( DWORD dwConnection )
{
    if ( !m_pDAdviseHolder )
        if ( !SUCCEEDED( CreateDataAdviseHolder( &m_pDAdviseHolder ) ) || !m_pDAdviseHolder )
            return E_OUTOFMEMORY;

    return m_pDAdviseHolder->Unadvise( dwConnection );
}

STDMETHODIMP EmbedDocument_Impl::EnumDAdvise( IEnumSTATDATA ** ppenumAdvise )
{
    if ( !m_pDAdviseHolder )
        if ( !SUCCEEDED( CreateDataAdviseHolder( &m_pDAdviseHolder ) ) || !m_pDAdviseHolder )
            return E_OUTOFMEMORY;

    return m_pDAdviseHolder->EnumAdvise( ppenumAdvise );
}

// Fix strange warnings about some
// ATL::CAxHostWindow::QueryInterface|AddRef|Releae functions.
// warning C4505: 'xxx' : unreferenced local function has been removed
#if defined(_MSC_VER)
#pragma warning(disable: 4505)
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
