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


#include <tools/resid.hxx>
#include <unotools/charclass.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <editeng/unolingu.hxx>
#include <txtfld.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <cntfrm.hxx>
#include <node.hxx>
#include <frmatr.hxx>
#include <pam.hxx>
#include <txttxmrk.hxx>
#include <frmfmt.hxx>
#include <fmtfld.hxx>
#include <txmsrt.hxx>
#include <ndtxt.hxx>
#include <txtatr.hxx>
#include <swtable.hxx>
#include <expfld.hxx>
#include <authfld.hxx>
#include <toxwrap.hxx>

#include <comcore.hrc>
#include <numrule.hxx>

extern sal_Bool IsFrameBehind( const SwTxtNode& rMyNd, xub_StrLen nMySttPos,
                           const SwTxtNode& rBehindNd, xub_StrLen nSttPos );

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;
/*--------------------------------------------------------------------
    Beschreibung: Strings initialisieren
 --------------------------------------------------------------------*/

sal_uInt16 SwTOXSortTabBase::nOpt = 0;

SV_IMPL_VARARR( SwTOXSources, SwTOXSource )


SwTOXInternational::SwTOXInternational( LanguageType nLang, sal_uInt16 nOpt,
                                        const String& rSortAlgorithm ) :
    eLang( nLang ),
    sSortAlgorithm(rSortAlgorithm),
    nOptions( nOpt )
{
    Init();
}

SwTOXInternational::SwTOXInternational( const SwTOXInternational& rIntl ) :
    eLang( rIntl.eLang ),
    sSortAlgorithm(rIntl.sSortAlgorithm),
    nOptions( rIntl.nOptions )
{
  Init();
}

void SwTOXInternational::Init()
{
    pIndexWrapper = new IndexEntrySupplierWrapper();

    const lang::Locale aLcl( SvxCreateLocale( eLang ) );
    pIndexWrapper->SetLocale( aLcl );

    if(!sSortAlgorithm.Len())
    {
        Sequence < OUString > aSeq( pIndexWrapper->GetAlgorithmList( aLcl ));
        if(aSeq.getLength())
            sSortAlgorithm = aSeq.getConstArray()[0];
    }

    if ( nOptions & nsSwTOIOptions::TOI_CASE_SENSITIVE )
        pIndexWrapper->LoadAlgorithm( aLcl, sSortAlgorithm, 0 );
    else
        pIndexWrapper->LoadAlgorithm( aLcl, sSortAlgorithm, SW_COLLATOR_IGNORES );

    pCharClass = new CharClass( aLcl );

}

SwTOXInternational::~SwTOXInternational()
{
    delete pCharClass;
    delete pIndexWrapper;
}

String SwTOXInternational::ToUpper( const String& rStr, xub_StrLen nPos ) const
{
    return pCharClass->toUpper( rStr, nPos, 1 );
}
inline sal_Bool SwTOXInternational::IsNumeric( const String& rStr ) const
{
    return pCharClass->isNumeric( rStr );
}

sal_Int32 SwTOXInternational::Compare( const String& rTxt1, const String& rTxtReading1,
                                       const lang::Locale& rLocale1,
                                       const String& rTxt2, const String& rTxtReading2,
                                       const lang::Locale& rLocale2 ) const
{
    return pIndexWrapper->CompareIndexEntry( rTxt1, rTxtReading1, rLocale1,
                                             rTxt2, rTxtReading2, rLocale2 );
}

String SwTOXInternational::GetIndexKey( const String& rTxt, const String& rTxtReading,
                                        const lang::Locale& rLocale ) const
{
    return pIndexWrapper->GetIndexKey( rTxt, rTxtReading, rLocale );
}

String SwTOXInternational::GetFollowingText( sal_Bool bMorePages ) const
{
    return pIndexWrapper->GetFollowingText( bMorePages );
}

/*--------------------------------------------------------------------
     Beschreibung:  SortierElement fuer Verzeichniseintraege
 --------------------------------------------------------------------*/


SwTOXSortTabBase::SwTOXSortTabBase( TOXSortType nTyp, const SwCntntNode* pNd,
                                    const SwTxtTOXMark* pMark,
                                    const SwTOXInternational* pInter,
                                    const lang::Locale* pLocale )
    : pTOXNd( 0 ), pTxtMark( pMark ), pTOXIntl( pInter ),
    nPos( 0 ), nCntPos( 0 ), nType( static_cast<sal_uInt16>(nTyp) ), bValidTxt( sal_False )
{
    if ( pLocale )
        aLocale = *pLocale;

    if( pNd )
    {
        xub_StrLen n = 0;
        if( pTxtMark )
            n = *pTxtMark->GetStart();
        SwTOXSource aTmp( pNd, n,
                    pTxtMark ? pTxtMark->GetTOXMark().IsMainEntry() : sal_False );
        aTOXSources.Insert( aTmp, aTOXSources.Count() );

        nPos = pNd->GetIndex();

        switch( nTyp )
        {
        case TOX_SORT_CONTENT:
        case TOX_SORT_PARA:
        case TOX_SORT_TABLE:
            // falls sie in Sonderbereichen stehen, sollte man die
            // Position im Body besorgen
            if( nPos < pNd->GetNodes().GetEndOfExtras().GetIndex() )
            {
                // dann die "Anker" (Body) Position holen.
                Point aPt;
                const SwCntntFrm* pFrm = pNd->getLayoutFrm( pNd->GetDoc()->GetCurrentLayout(), &aPt, 0, sal_False );
                if( pFrm )
                {
                    SwPosition aPos( *pNd );
                    const SwDoc& rDoc = *pNd->GetDoc();
                    bool const bResult = GetBodyTxtNode( rDoc, aPos, *pFrm );
                    OSL_ENSURE(bResult, "where is the text node");
                    (void) bResult; // unused in non-debug
                    nPos = aPos.nNode.GetIndex();
                    nCntPos = aPos.nContent.GetIndex();
                }
            }
            else
                nCntPos = n;
            break;
        default: break;
        }
    }
}


String SwTOXSortTabBase::GetURL() const
{
    return aEmptyStr;
}

void SwTOXSortTabBase::FillText( SwTxtNode& rNd, const SwIndex& rInsPos,
                                    sal_uInt16 ) const
{
    String sMyTxt;
    String sMyTxtReading;

    GetTxt( sMyTxt, sMyTxtReading );

    rNd.InsertText( sMyTxt, rInsPos );
}

sal_Bool SwTOXSortTabBase::operator==( const SwTOXSortTabBase& rCmp )
{
    sal_Bool bRet = nPos == rCmp.nPos && nCntPos == rCmp.nCntPos &&
            (!aTOXSources[0].pNd || !rCmp.aTOXSources[0].pNd ||
            aTOXSources[0].pNd == rCmp.aTOXSources[0].pNd );

    if( TOX_SORT_CONTENT == nType )
    {
        bRet = bRet && pTxtMark && rCmp.pTxtMark &&
                *pTxtMark->GetStart() == *rCmp.pTxtMark->GetStart();

        if( bRet )
        {
            // beide Pointer vorhanden -> vergleiche Text
            // beide Pointer nicht vorhanden -> vergleiche AlternativText
            const xub_StrLen *pEnd  = pTxtMark->GetEnd(),
                                *pEndCmp = rCmp.pTxtMark->GetEnd();

            String sMyTxt;
            String sMyTxtReading;
            GetTxt( sMyTxt, sMyTxtReading );

            String sOtherTxt;
            String sOtherTxtReading;
            rCmp.GetTxt( sOtherTxt, sOtherTxtReading );

            bRet = ( ( pEnd && pEndCmp ) || ( !pEnd && !pEndCmp ) ) &&
                    pTOXIntl->IsEqual( sMyTxt, sMyTxtReading, GetLocale(),
                                       sOtherTxt, sOtherTxtReading, rCmp.GetLocale() );
        }
    }
    return bRet;
}

sal_Bool SwTOXSortTabBase::operator<( const SwTOXSortTabBase& rCmp )
{
    if( nPos < rCmp.nPos )
        return sal_True;

    if( nPos == rCmp.nPos )
    {
        if( nCntPos < rCmp.nCntPos )
            return sal_True;

        if( nCntPos == rCmp.nCntPos )
        {
            const SwNode* pFirst = aTOXSources[0].pNd;
            const SwNode* pNext = rCmp.aTOXSources[0].pNd;

            if( pFirst && pFirst == pNext )
            {
                if( TOX_SORT_CONTENT == nType && pTxtMark && rCmp.pTxtMark )
                {
                    if( *pTxtMark->GetStart() < *rCmp.pTxtMark->GetStart() )
                        return sal_True;

                    if( *pTxtMark->GetStart() == *rCmp.pTxtMark->GetStart() )
                    {
                        const xub_StrLen *pEnd = pTxtMark->GetEnd(),
                                            *pEndCmp = rCmp.pTxtMark->GetEnd();

                        String sMyTxt;
                        String sMyTxtReading;
                        GetTxt( sMyTxt, sMyTxtReading );

                        String sOtherTxt;
                        String sOtherTxtReading;
                        rCmp.GetTxt( sOtherTxt, sOtherTxtReading );

                        // beide Pointer vorhanden -> vergleiche Text
                        // beide Pointer nicht vorhanden -> vergleiche AlternativText
                        if( ( pEnd && pEndCmp ) || ( !pEnd && !pEndCmp ) )
                            pTOXIntl->IsEqual( sMyTxt, sMyTxtReading, GetLocale(),
                                               sOtherTxt, sOtherTxtReading, rCmp.GetLocale() );

                        if( pEnd && !pEndCmp )
                            return sal_True;
                    }
                }
            }
            else if( pFirst && pFirst->IsTxtNode() &&
                     pNext && pNext->IsTxtNode() )
                    return ::IsFrameBehind( *(SwTxtNode*)pNext, nCntPos,
                                            *(SwTxtNode*)pFirst, nCntPos );
        }
    }
    return sal_False;
}

/*--------------------------------------------------------------------
     Beschreibung: sortierter Stichworteintrag
 --------------------------------------------------------------------*/


SwTOXIndex::SwTOXIndex( const SwTxtNode& rNd,
                        const SwTxtTOXMark* pMark, sal_uInt16 nOptions,
                        sal_uInt8 nKyLevel,
                        const SwTOXInternational& rIntl,
                        const lang::Locale& rLocale )
    : SwTOXSortTabBase( TOX_SORT_INDEX, &rNd, pMark, &rIntl, &rLocale ),
    nKeyLevel(nKyLevel)
{
    nPos = rNd.GetIndex();
    nOpt = nOptions;
}

//
// Stichworte vergleichen. Bezieht sich nur auf den Text
//


sal_Bool SwTOXIndex::operator==( const SwTOXSortTabBase& rCmpBase )
{
    SwTOXIndex& rCmp = (SwTOXIndex&)rCmpBase;

    // In Abhaengigkeit von den Optionen Grosskleinschreibung beachten
    if(GetLevel() != rCmp.GetLevel() || nKeyLevel != rCmp.nKeyLevel)
        return sal_False;

    OSL_ENSURE(pTxtMark, "pTxtMark == 0, Kein Stichwort");

    String sMyTxt;
    String sMyTxtReading;
    GetTxt( sMyTxt, sMyTxtReading );

    String sOtherTxt;
    String sOtherTxtReading;
    rCmp.GetTxt( sOtherTxt, sOtherTxtReading );

    sal_Bool bRet = pTOXIntl->IsEqual( sMyTxt, sMyTxtReading, GetLocale(),
                                   sOtherTxt, sOtherTxtReading, rCmp.GetLocale() );

    // Wenn nicht zusammengefasst wird muss die Pos aus gewertet werden
    if(bRet && !(GetOptions() & nsSwTOIOptions::TOI_SAME_ENTRY))
        bRet = nPos == rCmp.nPos;

    return bRet;
}

//
// kleiner haengt nur vom Text ab

sal_Bool SwTOXIndex::operator<( const SwTOXSortTabBase& rCmpBase )
{
    SwTOXIndex& rCmp = (SwTOXIndex&)rCmpBase;

    OSL_ENSURE(pTxtMark, "pTxtMark == 0, Kein Stichwort");

    String sMyTxt;
    String sMyTxtReading;
    GetTxt( sMyTxt, sMyTxtReading );

    String sOtherTxt;
    String sOtherTxtReading;
    rCmp.GetTxt( sOtherTxt, sOtherTxtReading );

    sal_Bool bRet = GetLevel() == rCmp.GetLevel() &&
                pTOXIntl->IsLess( sMyTxt, sMyTxtReading, GetLocale(),
                                  sOtherTxt, sOtherTxtReading, rCmp.GetLocale() );

    // Wenn nicht zusammengefasst wird muss die Pos aus gewertet werden
    if( !bRet && !(GetOptions() & nsSwTOIOptions::TOI_SAME_ENTRY) )
    {
        bRet = pTOXIntl->IsEqual( sMyTxt, sMyTxtReading, GetLocale(),
                                   sOtherTxt, sOtherTxtReading, rCmp.GetLocale() ) &&
               nPos < rCmp.nPos;
    }

    return bRet;
}

//
// Das Stichwort selbst

void SwTOXIndex::GetText_Impl( String& rTxt, String& rTxtReading ) const
{
    OSL_ENSURE(pTxtMark, "pTxtMark == 0, Kein Stichwort");
    const SwTOXMark& rTOXMark = pTxtMark->GetTOXMark();
    switch(nKeyLevel)
    {
        case FORM_PRIMARY_KEY    :
        {
            rTxt = rTOXMark.GetPrimaryKey();
            rTxtReading = rTOXMark.GetPrimaryKeyReading();
        }
        break;
        case FORM_SECONDARY_KEY  :
        {
            rTxt = rTOXMark.GetSecondaryKey();
            rTxtReading = rTOXMark.GetSecondaryKeyReading();
        }
        break;
        case FORM_ENTRY          :
        {
            rTxt = rTOXMark.GetText();
            rTxtReading = rTOXMark.GetTextReading();
        }
        break;
    }
    // if TOI_INITIAL_CAPS is set, first character is to be capitalized
    if( nsSwTOIOptions::TOI_INITIAL_CAPS & nOpt && pTOXIntl )
    {
        String sUpper( pTOXIntl->ToUpper( rTxt, 0 ));
        rTxt.Erase( 0, 1 ).Insert( sUpper, 0 );
    }
}

void SwTOXIndex::FillText( SwTxtNode& rNd, const SwIndex& rInsPos, sal_uInt16 ) const
{
    const xub_StrLen* pEnd = pTxtMark->GetEnd();
    String sTmp;
    String sTmpReading;
    if( pEnd && !pTxtMark->GetTOXMark().IsAlternativeText() &&
            0 == (GetOptions() & nsSwTOIOptions::TOI_KEY_AS_ENTRY))
    {
        sTmp = ((SwTxtNode*)aTOXSources[0].pNd)->GetExpandTxt(
                            *pTxtMark->GetStart(),
                            *pEnd - *pTxtMark->GetStart());
        if(nsSwTOIOptions::TOI_INITIAL_CAPS&nOpt && pTOXIntl)
        {
            String sUpper( pTOXIntl->ToUpper( sTmp, 0 ));
            sTmp.Erase( 0, 1 ).Insert( sUpper, 0 );
        }
    }
    else
        GetTxt( sTmp, sTmpReading );

    rNd.InsertText( sTmp, rInsPos );
}



sal_uInt16 SwTOXIndex::GetLevel() const
{
    OSL_ENSURE(pTxtMark, "pTxtMark == 0, Kein Stichwort");

    sal_uInt16 nForm = FORM_PRIMARY_KEY;

    if( 0 == (GetOptions() & nsSwTOIOptions::TOI_KEY_AS_ENTRY)&&
        pTxtMark->GetTOXMark().GetPrimaryKey().Len() )
    {
        nForm = FORM_SECONDARY_KEY;
        if( pTxtMark->GetTOXMark().GetSecondaryKey().Len() )
            nForm = FORM_ENTRY;
    }
    return nForm;
}

/*--------------------------------------------------------------------
     Beschreibung: Schluessel und Trennzeichen
 --------------------------------------------------------------------*/


SwTOXCustom::SwTOXCustom(const String& rStr, const String& rReading,
                         sal_uInt16 nLevel,
                         const SwTOXInternational& rIntl,
                         const lang::Locale& rLocale )
    : SwTOXSortTabBase( TOX_SORT_CUSTOM, 0, 0, &rIntl, &rLocale ),
    aKey(rStr), sReading(rReading), nLev(nLevel)
{
}


sal_Bool SwTOXCustom::operator==(const SwTOXSortTabBase& rCmpBase)
{
    String sMyTxt;
    String sMyTxtReading;
    GetTxt( sMyTxt, sMyTxtReading );

    String sOtherTxt;
    String sOtherTxtReading;
    rCmpBase.GetTxt( sOtherTxt, sOtherTxtReading );

    return GetLevel() == rCmpBase.GetLevel() &&
           pTOXIntl->IsEqual( sMyTxt, sMyTxtReading, GetLocale(),
                              sOtherTxt, sOtherTxtReading, rCmpBase.GetLocale() );
}


sal_Bool SwTOXCustom::operator < (const SwTOXSortTabBase& rCmpBase)
{
    String sMyTxt;
    String sMyTxtReading;
    GetTxt( sMyTxt, sMyTxtReading );

    String sOtherTxt;
    String sOtherTxtReading;
    rCmpBase.GetTxt( sOtherTxt, sOtherTxtReading );

    return  GetLevel() <= rCmpBase.GetLevel() &&
            pTOXIntl->IsLess( sMyTxt, sMyTxtReading, GetLocale(),
                              sOtherTxt, sOtherTxtReading, rCmpBase.GetLocale() );
}


sal_uInt16 SwTOXCustom::GetLevel() const
{
    return nLev;
}


void SwTOXCustom::GetText_Impl( String& rTxt, String &rTxtReading ) const
{
    rTxt = aKey;
    rTxtReading = sReading;
}


/*--------------------------------------------------------------------
     Beschreibung: sortierter Inhaltsverz. Eintrag
 --------------------------------------------------------------------*/


SwTOXContent::SwTOXContent( const SwTxtNode& rNd, const SwTxtTOXMark* pMark,
                        const SwTOXInternational& rIntl)
    : SwTOXSortTabBase( TOX_SORT_CONTENT, &rNd, pMark, &rIntl )
{
}


//  Der Text des Inhalts
//

void SwTOXContent::GetText_Impl( String& rTxt, String& rTxtReading ) const
{
    const xub_StrLen* pEnd = pTxtMark->GetEnd();
    if( pEnd && !pTxtMark->GetTOXMark().IsAlternativeText() )
    {
        rTxt = ((SwTxtNode*)aTOXSources[0].pNd)->GetExpandTxt(
                                     *pTxtMark->GetStart(),
                                     *pEnd - *pTxtMark->GetStart() );

        rTxtReading = pTxtMark->GetTOXMark().GetTextReading();
    }
    else
        rTxt = pTxtMark->GetTOXMark().GetAlternativeText();
}

void SwTOXContent::FillText( SwTxtNode& rNd, const SwIndex& rInsPos, sal_uInt16 ) const
{
    const xub_StrLen* pEnd = pTxtMark->GetEnd();
    if( pEnd && !pTxtMark->GetTOXMark().IsAlternativeText() )
        ((SwTxtNode*)aTOXSources[0].pNd)->GetExpandTxt( rNd, &rInsPos,
                                    *pTxtMark->GetStart(),
                                    *pEnd - *pTxtMark->GetStart() );
    else
    {
        String sTmp, sTmpReading;
        GetTxt( sTmp, sTmpReading );
        rNd.InsertText( sTmp, rInsPos );
    }
}

//
// Die Ebene fuer Anzeige
//


sal_uInt16 SwTOXContent::GetLevel() const
{
    return pTxtMark->GetTOXMark().GetLevel();
}

/*--------------------------------------------------------------------
     Beschreibung: Verzeichnis aus Absaetzen zusammengesammelt
 --------------------------------------------------------------------*/

// bei Sortierung von OLE/Grafiken aufpassen !!!
// Die Position darf nicht die im Dokument,
// sondern muss die vom "Henkel" sein  !!


SwTOXPara::SwTOXPara( const SwCntntNode& rNd, SwTOXElement eT, sal_uInt16 nLevel )
    : SwTOXSortTabBase( TOX_SORT_PARA, &rNd, 0, 0 ),
    eType( eT ),
    m_nLevel(nLevel),
    nStartIndex(0),
    nEndIndex(STRING_LEN)
{
}


void SwTOXPara::GetText_Impl( String& rTxt, String& ) const
{
    const SwCntntNode* pNd = aTOXSources[0].pNd;
    switch( eType )
    {
    case nsSwTOXElement::TOX_SEQUENCE:
    case nsSwTOXElement::TOX_TEMPLATE:
    case nsSwTOXElement::TOX_OUTLINELEVEL:
        {
            xub_StrLen nStt = nStartIndex;
            rTxt = ((SwTxtNode*)pNd)->GetExpandTxt(
                    nStt,
                    STRING_NOTFOUND == nEndIndex ? STRING_LEN : nEndIndex - nStt);
        }
        break;

    case nsSwTOXElement::TOX_OLE:
    case nsSwTOXElement::TOX_GRAPHIC:
    case nsSwTOXElement::TOX_FRAME:
        {
            // suche das FlyFormat, dort steht der Object/Grafik-Name
            SwFrmFmt* pFly = pNd->GetFlyFmt();
            if( pFly )
                rTxt = pFly->GetName();
            else
            {
                OSL_ENSURE( !this, "Grafik/Object ohne Namen" );
                sal_uInt16 nId = nsSwTOXElement::TOX_OLE == eType
                                ? STR_OBJECT_DEFNAME
                                : nsSwTOXElement::TOX_GRAPHIC == eType
                                    ? STR_GRAPHIC_DEFNAME
                                    : STR_FRAME_DEFNAME;
                rTxt = SW_RESSTR( nId );
            }
        }
        break;
    default: break;
    }
}

void SwTOXPara::FillText( SwTxtNode& rNd, const SwIndex& rInsPos, sal_uInt16 ) const
{
    if( nsSwTOXElement::TOX_TEMPLATE == eType || nsSwTOXElement::TOX_SEQUENCE == eType  || nsSwTOXElement::TOX_OUTLINELEVEL == eType)
    {
        SwTxtNode* pSrc = (SwTxtNode*)aTOXSources[0].pNd;
        xub_StrLen nStt = nStartIndex;
        pSrc->GetExpandTxt( rNd, &rInsPos, nStt,
                nEndIndex == STRING_LEN ? STRING_LEN : nEndIndex - nStt,
                sal_False, sal_False, sal_True );
    }
    else
    {
        String sTmp, sTmpReading;
        GetTxt( sTmp, sTmpReading );
        sTmp.SearchAndReplaceAll('\t', ' ');
        rNd.InsertText( sTmp, rInsPos );
    }
}


sal_uInt16 SwTOXPara::GetLevel() const
{
    sal_uInt16 nRet = m_nLevel;
    const SwCntntNode*  pNd = aTOXSources[0].pNd;

    if( nsSwTOXElement::TOX_OUTLINELEVEL == eType && pNd->GetTxtNode() )
    {
        const int nTmp = ((SwTxtNode*)pNd)->GetAttrOutlineLevel();//#outline level,zhaojianwei????
        if(nTmp != 0 )
            nRet = static_cast<sal_uInt16>(nTmp);
    }
    return nRet;
}


String SwTOXPara::GetURL() const
{
    String aTxt;
    const SwCntntNode* pNd = aTOXSources[0].pNd;
    switch( eType )
    {
    case nsSwTOXElement::TOX_TEMPLATE:
    case nsSwTOXElement::TOX_OUTLINELEVEL:
        {
            const SwTxtNode * pTxtNd = static_cast<const SwTxtNode *>(pNd);

            SwDoc* pDoc = const_cast<SwDoc*>( pTxtNd->GetDoc() );
            ::sw::mark::IMark const * const pMark = pDoc->getIDocumentMarkAccess()->getMarkForTxtNode(
                                *(pTxtNd),
                                IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK);
            aTxt = '#';
            const String aMarkName( pMark->GetName() );
            aTxt += aMarkName;
        }
        break;

    case nsSwTOXElement::TOX_OLE:
    case nsSwTOXElement::TOX_GRAPHIC:
    case nsSwTOXElement::TOX_FRAME:
        {
            // suche das FlyFormat, dort steht der Object/Grafik-Name
            SwFrmFmt* pFly = pNd->GetFlyFmt();
            if( pFly )
            {
                (( aTxt = '#' ) += pFly->GetName() ) += cMarkSeperator;
                const sal_Char* pStr;
                switch( eType )
                {
                case nsSwTOXElement::TOX_OLE:       pStr = pMarkToOLE; break;
                case nsSwTOXElement::TOX_GRAPHIC:   pStr = pMarkToGraphic; break;
                case nsSwTOXElement::TOX_FRAME:     pStr = pMarkToFrame; break;
                default:            pStr = 0;
                }
                if( pStr )
                    aTxt.AppendAscii( pStr );
            }
        }
        break;
    default: break;
    }
    return aTxt;
}


/*--------------------------------------------------------------------
    Beschreibung: Tabelle
 --------------------------------------------------------------------*/


SwTOXTable::SwTOXTable( const SwCntntNode& rNd )
    : SwTOXSortTabBase( TOX_SORT_TABLE, &rNd, 0, 0 ),
    nLevel(FORM_ALPHA_DELIMITTER)
{
}


void SwTOXTable::GetText_Impl( String& rTxt, String& ) const
{
    const SwNode* pNd = aTOXSources[0].pNd;
    if( pNd && 0 != ( pNd = pNd->FindTableNode() ) )
    {
        rTxt = ((SwTableNode*)pNd)->GetTable().GetFrmFmt()->GetName();
    }
    else
    {
        OSL_ENSURE( !this, "Wo ist meine Tabelle geblieben?" );
        rTxt = SW_RESSTR( STR_TABLE_DEFNAME );
    }
}

sal_uInt16 SwTOXTable::GetLevel() const
{
    return nLevel;
}


String SwTOXTable::GetURL() const
{
    String aTxt;
    const SwNode* pNd = aTOXSources[0].pNd;
    if( pNd && 0 != ( pNd = pNd->FindTableNode() ) )
    {
        aTxt = ((SwTableNode*)pNd)->GetTable().GetFrmFmt()->GetName();
        if( aTxt.Len() )
        {
            ( aTxt.Insert( '#', 0 ) += cMarkSeperator ).
                                            AppendAscii( pMarkToTable );
        }
    }
    return aTxt;
}

SwTOXAuthority::SwTOXAuthority( const SwCntntNode& rNd,
                SwFmtFld& rField, const SwTOXInternational& rIntl ) :
    SwTOXSortTabBase( TOX_SORT_AUTHORITY, &rNd, 0, &rIntl ),
    m_rField(rField)
{
    if(rField.GetTxtFld())
        nCntPos = *rField.GetTxtFld()->GetStart();
}

sal_uInt16 SwTOXAuthority::GetLevel() const
{
    String sText(((SwAuthorityField*)m_rField.GetFld())->
                        GetFieldText(AUTH_FIELD_AUTHORITY_TYPE));
    //#i18655# the level '0' is the heading level therefor the values are incremented here
    sal_uInt16 nRet = 1;
    if( pTOXIntl->IsNumeric( sText ) )
    {
        nRet = (sal_uInt16)sText.ToInt32();
        nRet++;
    }
    //illegal values are also set to 'ARTICLE' as non-numeric values are
    if(nRet > AUTH_TYPE_END)
        nRet = 1;
    return nRet;
}

static String lcl_GetText(SwFmtFld const& rField)
{
    return rField.GetFld()->ExpandField(true);
}

void SwTOXAuthority::GetText_Impl( String& rTxt, String& ) const
{
    rTxt = lcl_GetText(m_rField);
}

void    SwTOXAuthority::FillText( SwTxtNode& rNd,
                        const SwIndex& rInsPos, sal_uInt16 nAuthField ) const
{
    SwAuthorityField* pField = (SwAuthorityField*)m_rField.GetFld();
    String sText;
    if(AUTH_FIELD_IDENTIFIER == nAuthField)
    {
        sText = lcl_GetText(m_rField);
        const SwAuthorityFieldType* pType = (const SwAuthorityFieldType*)pField->GetTyp();
        sal_Unicode cChar = pType->GetPrefix();
        if(cChar && cChar != ' ')
            sText.Erase(0, 1);
        cChar = pType->GetSuffix();
        if(cChar && cChar != ' ')
            sText.Erase(sText.Len() - 1, 1);
    }
    else if(AUTH_FIELD_AUTHORITY_TYPE == nAuthField)
    {
        sal_uInt16 nLevel = GetLevel();
        if(nLevel)
            sText = SwAuthorityFieldType::GetAuthTypeName((ToxAuthorityType) --nLevel);
    }
    else
        sText = (pField->GetFieldText((ToxAuthorityField) nAuthField));
    rNd.InsertText( sText, rInsPos );
}

sal_Bool    SwTOXAuthority::operator==( const SwTOXSortTabBase& rCmp)
{
    return nType == rCmp.nType &&
            ((SwAuthorityField*)m_rField.GetFld())->GetHandle() ==
                ((SwAuthorityField*)((SwTOXAuthority&)rCmp).m_rField.GetFld())->GetHandle();
}

sal_Bool    SwTOXAuthority::operator<( const SwTOXSortTabBase& rBase)
{
    sal_Bool bRet = sal_False;
    SwAuthorityField* pField = (SwAuthorityField*)m_rField.GetFld();
    SwAuthorityFieldType* pType = (SwAuthorityFieldType*)
                                                pField->GetTyp();
    if(pType->IsSortByDocument())
        bRet = SwTOXSortTabBase::operator<(rBase);
    else
    {
        SwAuthorityField* pCmpField = (SwAuthorityField*)
                        ((SwTOXAuthority&)rBase).m_rField.GetFld();


        for(sal_uInt16 i = 0; i < pType->GetSortKeyCount(); i++)
        {
            const SwTOXSortKey* pKey = pType->GetSortKey(i);
            String sMyTxt = pField->GetFieldText(pKey->eField);
            String sMyTxtReading;
            String sOtherTxt = pCmpField->GetFieldText(pKey->eField);
            String sOtherTxtReading;

            sal_Int32 nComp = pTOXIntl->Compare( sMyTxt, sMyTxtReading, GetLocale(),
                                                 sOtherTxt, sOtherTxtReading, rBase.GetLocale() );

            if( nComp )
            {
                bRet = (-1 == nComp) == pKey->bSortAscending;
                break;
            }
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
