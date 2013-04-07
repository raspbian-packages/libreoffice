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



#include <vcl/svapp.hxx>
#include <sal/macros.h>
#include "cfgitem.hxx"

#include "starmath.hrc"
#include "smdll.hxx"
#include "format.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

using ::rtl::OUString;


static const char* aRootName = "Office.Math";

#define SYMBOL_LIST         "SymbolList"
#define FONT_FORMAT_LIST    "FontFormatList"

/////////////////////////////////////////////////////////////////


static Sequence< OUString > lcl_GetFontPropertyNames()
{
    static const char * aPropNames[] =
    {
        "Name",
        "CharSet",
        "Family",
        "Pitch",
        "Weight",
        "Italic",
        0
    };

    const char** ppPropName = aPropNames;

    Sequence< OUString > aNames( 6 );
    OUString *pNames = aNames.getArray();
    for( sal_Int32 i = 0; *ppPropName;  ++i, ++ppPropName )
    {
        pNames[i] = A2OU( *ppPropName );
    }
    return aNames;
}

/////////////////////////////////////////////////////////////////


static Sequence< OUString > lcl_GetSymbolPropertyNames()
{
    static const char * aPropNames[] =
    {
        "Char",
        "Set",
        "Predefined",
        "FontFormatId",
        0
    };

    const char** ppPropName = aPropNames;

    Sequence< OUString > aNames( 4 );
    OUString *pNames = aNames.getArray();
    for( sal_Int32 i = 0; *ppPropName;  ++i, ++ppPropName )
    {
        pNames[i] = A2OU( *ppPropName );
    }
    return aNames;
}

/////////////////////////////////////////////////////////////////

static const char * aMathPropNames[] =
{
    "Print/Title",
    "Print/FormulaText",
    "Print/Frame",
    "Print/Size",
    "Print/ZoomFactor",
    "LoadSave/IsSaveOnlyUsedSymbols",
    //"Misc/NoSymbolsWarning",  @deprecated
    "Misc/IgnoreSpacesRight",
    "View/ToolboxVisible",
    "View/AutoRedraw",
    "View/FormulaCursor"
};


//! Beware of order according to *_BEGIN *_END defines in format.hxx !
//! see respective load/save routines here
static const char * aFormatPropNames[] =
{
    "StandardFormat/Textmode",
    "StandardFormat/GreekCharStyle",
    "StandardFormat/ScaleNormalBracket",
    "StandardFormat/HorizontalAlignment",
    "StandardFormat/BaseSize",
    "StandardFormat/TextSize",
    "StandardFormat/IndexSize",
    "StandardFormat/FunctionSize",
    "StandardFormat/OperatorSize",
    "StandardFormat/LimitsSize",
    "StandardFormat/Distance/Horizontal",
    "StandardFormat/Distance/Vertical",
    "StandardFormat/Distance/Root",
    "StandardFormat/Distance/SuperScript",
    "StandardFormat/Distance/SubScript",
    "StandardFormat/Distance/Numerator",
    "StandardFormat/Distance/Denominator",
    "StandardFormat/Distance/Fraction",
    "StandardFormat/Distance/StrokeWidth",
    "StandardFormat/Distance/UpperLimit",
    "StandardFormat/Distance/LowerLimit",
    "StandardFormat/Distance/BracketSize",
    "StandardFormat/Distance/BracketSpace",
    "StandardFormat/Distance/MatrixRow",
    "StandardFormat/Distance/MatrixColumn",
    "StandardFormat/Distance/OrnamentSize",
    "StandardFormat/Distance/OrnamentSpace",
    "StandardFormat/Distance/OperatorSize",
    "StandardFormat/Distance/OperatorSpace",
    "StandardFormat/Distance/LeftSpace",
    "StandardFormat/Distance/RightSpace",
    "StandardFormat/Distance/TopSpace",
    "StandardFormat/Distance/BottomSpace",
    "StandardFormat/Distance/NormalBracketSize",
    "StandardFormat/VariableFont",
    "StandardFormat/FunctionFont",
    "StandardFormat/NumberFont",
    "StandardFormat/TextFont",
    "StandardFormat/SerifFont",
    "StandardFormat/SansFont",
    "StandardFormat/FixedFont"
};


static Sequence< OUString > lcl_GetPropertyNames(
        const char * aPropNames[], sal_uInt16 nCount )
{

    const char** ppPropName = aPropNames;

    Sequence< OUString > aNames( nCount );
    OUString *pNames = aNames.getArray();
    for (sal_Int32 i = 0;  i < nCount;  ++i, ++ppPropName)
    {
        pNames[i] = A2OU( *ppPropName );
    }
    return aNames;
}


static Sequence< OUString > GetFormatPropertyNames()
{
    sal_uInt16 nCnt = SAL_N_ELEMENTS(aFormatPropNames);
    return lcl_GetPropertyNames( aFormatPropNames, nCnt );
}


static Sequence< OUString > GetOtherPropertyNames()
{
    sal_uInt16 nCnt = SAL_N_ELEMENTS(aMathPropNames);
    return lcl_GetPropertyNames( aMathPropNames, nCnt );
}

/////////////////////////////////////////////////////////////////

struct SmCfgOther
{
    SmPrintSize     ePrintSize;
    sal_uInt16      nPrintZoomFactor;
    bool            bPrintTitle;
    bool            bPrintFormulaText;
    bool            bPrintFrame;
    bool            bIsSaveOnlyUsedSymbols;
    bool            bIgnoreSpacesRight;
    bool            bToolboxVisible;
    bool            bAutoRedraw;
    bool            bFormulaCursor;
    //sal_Bool            bNoSymbolsWarning;

    SmCfgOther();
};


SmCfgOther::SmCfgOther()
{
    ePrintSize          = PRINT_SIZE_NORMAL;
    nPrintZoomFactor    = 100;
    bPrintTitle         = bPrintFormulaText   =
    bPrintFrame         = bIgnoreSpacesRight  =
    bToolboxVisible     = bAutoRedraw         =
    bFormulaCursor      = bIsSaveOnlyUsedSymbols = true;
}

/////////////////////////////////////////////////////////////////


SmFontFormat::SmFontFormat()
{
    aName.AssignAscii( FONTNAME_MATH );
    nCharSet    = RTL_TEXTENCODING_UNICODE;
    nFamily     = FAMILY_DONTKNOW;
    nPitch      = PITCH_DONTKNOW;
    nWeight     = WEIGHT_DONTKNOW;
    nItalic     = ITALIC_NONE;
}


SmFontFormat::SmFontFormat( const Font &rFont )
{
    aName       = rFont.GetName();
    nCharSet    = (sal_Int16) rFont.GetCharSet();
    nFamily     = (sal_Int16) rFont.GetFamily();
    nPitch      = (sal_Int16) rFont.GetPitch();
    nWeight     = (sal_Int16) rFont.GetWeight();
    nItalic     = (sal_Int16) rFont.GetItalic();
}


const Font SmFontFormat::GetFont() const
{
    Font aRes;
    aRes.SetName( aName );
    aRes.SetCharSet( (rtl_TextEncoding) nCharSet );
    aRes.SetFamily( (FontFamily) nFamily );
    aRes.SetPitch( (FontPitch) nPitch );
    aRes.SetWeight( (FontWeight) nWeight );
    aRes.SetItalic( (FontItalic) nItalic );
    return aRes;
}


bool SmFontFormat::operator == ( const SmFontFormat &rFntFmt ) const
{
    return  aName    == rFntFmt.aName       &&
            nCharSet == rFntFmt.nCharSet    &&
            nFamily  == rFntFmt.nFamily     &&
            nPitch   == rFntFmt.nPitch      &&
            nWeight  == rFntFmt.nWeight     &&
            nItalic  == rFntFmt.nItalic;
}


/////////////////////////////////////////////////////////////////

SmFntFmtListEntry::SmFntFmtListEntry( const String &rId, const SmFontFormat &rFntFmt ) :
    aId     (rId),
    aFntFmt (rFntFmt)
{
}


SmFontFormatList::SmFontFormatList()
{
    bModified = false;
}


void SmFontFormatList::Clear()
{
    if (!aEntries.empty())
    {
        aEntries.clear();
        SetModified( true );
    }
}


void SmFontFormatList::AddFontFormat( const String &rFntFmtId,
        const SmFontFormat &rFntFmt )
{
    const SmFontFormat *pFntFmt = GetFontFormat( rFntFmtId );
    OSL_ENSURE( !pFntFmt, "FontFormatId already exists" );
    if (!pFntFmt)
    {
        SmFntFmtListEntry aEntry( rFntFmtId, rFntFmt );
        aEntries.push_back( aEntry );
        SetModified( true );
    }
}


void SmFontFormatList::RemoveFontFormat( const String &rFntFmtId )
{

    // search for entry
    for (size_t i = 0;  i < aEntries.size();  ++i)
    {
        if (aEntries[i].aId == rFntFmtId)
        {
            // remove entry if found
            aEntries.erase( aEntries.begin() + i );
            SetModified( true );
            break;
        }
    }
}


const SmFontFormat * SmFontFormatList::GetFontFormat( const String &rFntFmtId ) const
{
    const SmFontFormat *pRes = 0;

    for (size_t i = 0;  i < aEntries.size();  ++i)
    {
        if (aEntries[i].aId == rFntFmtId)
        {
            pRes = &aEntries[i].aFntFmt;
            break;
        }
    }

    return pRes;
}



const SmFontFormat * SmFontFormatList::GetFontFormat( size_t nPos ) const
{
    const SmFontFormat *pRes = 0;
    if (nPos < aEntries.size())
        pRes = &aEntries[nPos].aFntFmt;
    return pRes;
}


const String SmFontFormatList::GetFontFormatId( const SmFontFormat &rFntFmt ) const
{
    String aRes;

    for (size_t i = 0;  i < aEntries.size();  ++i)
    {
        if (aEntries[i].aFntFmt == rFntFmt)
        {
            aRes = aEntries[i].aId;
            break;
        }
    }

    return aRes;
}


const String SmFontFormatList::GetFontFormatId( const SmFontFormat &rFntFmt, bool bAdd )
{
    String aRes( GetFontFormatId( rFntFmt) );
    if (0 == aRes.Len()  &&  bAdd)
    {
        aRes = GetNewFontFormatId();
        AddFontFormat( aRes, rFntFmt );
    }
    return aRes;
}


const String SmFontFormatList::GetFontFormatId( size_t nPos ) const
{
    String aRes;
    if (nPos < aEntries.size())
        aRes = aEntries[nPos].aId;
    return aRes;
}


const String SmFontFormatList::GetNewFontFormatId() const
{
    // returns first unused FormatId

    String aRes;

    String aPrefix( RTL_CONSTASCII_USTRINGPARAM( "Id" ) );
    sal_Int32 nCnt = GetCount();
    for (sal_Int32 i = 1;  i <= nCnt + 1;  ++i)
    {
        String aTmpId( aPrefix );
        aTmpId += String::CreateFromInt32( i );
        if (!GetFontFormat( aTmpId ))
        {
            aRes = aTmpId;
            break;
        }
    }
    OSL_ENSURE( 0 != aRes.Len(), "failed to create new FontFormatId" );

    return aRes;
}

/////////////////////////////////////////////////////////////////

SmMathConfig::SmMathConfig() :
    ConfigItem( String::CreateFromAscii( aRootName ))
{
    pFormat         = 0;
    pOther          = 0;
    pFontFormatList = 0;
    pSymbolMgr      = 0;

    bIsOtherModified = bIsFormatModified = false;
}


SmMathConfig::~SmMathConfig()
{
    Save();
    delete pFormat;
    delete pOther;
    delete pFontFormatList;
    delete pSymbolMgr;
}


void SmMathConfig::SetOtherModified( bool bVal )
{
    bIsOtherModified = bVal;
}


void SmMathConfig::SetFormatModified( bool bVal )
{
    bIsFormatModified = bVal;
}


void SmMathConfig::SetFontFormatListModified( bool bVal )
{
    if (pFontFormatList)
        pFontFormatList->SetModified( bVal );
}


void SmMathConfig::ReadSymbol( SmSym &rSymbol,
                        const rtl::OUString &rSymbolName,
                        const rtl::OUString &rBaseNode ) const
{
    Sequence< OUString > aNames = lcl_GetSymbolPropertyNames();
    sal_Int32 nProps = aNames.getLength();

    OUString aDelim( OUString::valueOf( (sal_Unicode) '/' ) );
    OUString *pName = aNames.getArray();
    for (sal_Int32 i = 0;  i < nProps;  ++i)
    {
        OUString &rName = pName[i];
        OUString aTmp( rName );
        rName = rBaseNode;
        rName += aDelim;
        rName += rSymbolName;
        rName += aDelim;
        rName += aTmp;
    }

    const Sequence< Any > aValues = ((SmMathConfig*) this)->GetProperties( aNames );

    if (nProps  &&  aValues.getLength() == nProps)
    {
        const Any * pValue = aValues.getConstArray();
        Font        aFont;
        sal_UCS4    cChar = '\0';
        String      aSet;
        bool        bPredefined = false;

        OUString    aTmpStr;
        sal_Int32       nTmp32 = 0;
        bool        bTmp = false;

        bool bOK = true;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp32))
            cChar = static_cast< sal_UCS4 >( nTmp32 );
        else
            bOK = false;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= aTmpStr))
            aSet = aTmpStr;
        else
            bOK = false;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= bTmp))
            bPredefined = bTmp;
        else
            bOK = false;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= aTmpStr))
        {
            const SmFontFormat *pFntFmt = GetFontFormatList().GetFontFormat( aTmpStr );
            OSL_ENSURE( pFntFmt, "unknown FontFormat" );
            if (pFntFmt)
                aFont = pFntFmt->GetFont();
        }
        else
            bOK = false;
        ++pValue;

        if (bOK)
        {
            String aUiName( rSymbolName );
            String aUiSetName( aSet );
            if (bPredefined)
            {
                String aTmp;
                aTmp = GetUiSymbolName( rSymbolName );
                OSL_ENSURE( aTmp.Len(), "localized symbol-name not found" );
                if (aTmp.Len())
                    aUiName = aTmp;
                aTmp = GetUiSymbolSetName( aSet );
                OSL_ENSURE( aTmp.Len(), "localized symbolset-name not found" );
                if (aTmp.Len())
                    aUiSetName = aTmp;
            }

            rSymbol = SmSym( aUiName, aFont, cChar, aUiSetName, bPredefined );
            if (aUiName != String(rSymbolName))
                rSymbol.SetExportName( rSymbolName );
        }
        else
        {
            OSL_FAIL( "symbol read error" );
        }
    }
}


SmSymbolManager & SmMathConfig::GetSymbolManager()
{
    if (!pSymbolMgr)
    {
        pSymbolMgr = new SmSymbolManager;
        pSymbolMgr->Load();
    }
    return *pSymbolMgr;
}


void SmMathConfig::Commit()
{
    Save();
}


void SmMathConfig::Save()
{
    SaveOther();
    SaveFormat();
    SaveFontFormatList();
}


void SmMathConfig::GetSymbols( std::vector< SmSym > &rSymbols ) const
{
    Sequence< OUString > aNodes( ((SmMathConfig*) this)->GetNodeNames( A2OU( SYMBOL_LIST ) ) );
    const OUString *pNode = aNodes.getConstArray();
    sal_Int32 nNodes = aNodes.getLength();

    rSymbols.resize( nNodes );
    std::vector< SmSym >::iterator aIt( rSymbols.begin() );
    std::vector< SmSym >::iterator aEnd( rSymbols.end() );
    while (aIt != aEnd)
    {
        ReadSymbol( *aIt++, *pNode++, A2OU( SYMBOL_LIST ) );
    }
}


void SmMathConfig::SetSymbols( const std::vector< SmSym > &rNewSymbols )
{
    sal_uIntPtr nCount = rNewSymbols.size();

    Sequence< OUString > aNames = lcl_GetSymbolPropertyNames();
    const OUString *pNames = aNames.getConstArray();
    sal_uIntPtr nSymbolProps = sal::static_int_cast< sal_uInt32 >(aNames.getLength());

    Sequence< PropertyValue > aValues( nCount * nSymbolProps );
    PropertyValue *pValues = aValues.getArray();

    PropertyValue *pVal = pValues;
    OUString aDelim( OUString::valueOf( (sal_Unicode) '/' ) );
    std::vector< SmSym >::const_iterator aIt( rNewSymbols.begin() );
    std::vector< SmSym >::const_iterator aEnd( rNewSymbols.end() );
    while (aIt != aEnd)
    {
        const SmSym &rSymbol = *aIt++;
        OUString  aNodeNameDelim( A2OU( SYMBOL_LIST ) );
        aNodeNameDelim += aDelim;
        aNodeNameDelim += rSymbol.GetExportName();
        aNodeNameDelim += aDelim;

        const OUString *pName = pNames;

        // Char
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= static_cast< sal_UCS4 >( rSymbol.GetCharacter() );
        pVal++;
        // Set
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        OUString aTmp( rSymbol.GetSymbolSetName() );
        if (rSymbol.IsPredefined())
            aTmp = GetExportSymbolSetName( aTmp );
        pVal->Value <<= aTmp;
        pVal++;
        // Predefined
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= (sal_Bool) rSymbol.IsPredefined();
        pVal++;
        // FontFormatId
        SmFontFormat aFntFmt( rSymbol.GetFace() );
        String aFntFmtId( GetFontFormatList().GetFontFormatId( aFntFmt, true ) );
        OSL_ENSURE( aFntFmtId.Len(), "FontFormatId not found" );
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= OUString( aFntFmtId );
        pVal++;
    }
    OSL_ENSURE( pVal - pValues == sal::static_int_cast< ptrdiff_t >(nCount * nSymbolProps), "properties missing" );
    ReplaceSetProperties( A2OU( SYMBOL_LIST ) , aValues );

    StripFontFormatList( rNewSymbols );
    SaveFontFormatList();
}


SmFontFormatList & SmMathConfig::GetFontFormatList()
{
    if (!pFontFormatList)
    {
        LoadFontFormatList();
    }
    return *pFontFormatList;
}


void SmMathConfig::LoadFontFormatList()
{
    if (!pFontFormatList)
        pFontFormatList = new SmFontFormatList;
    else
        pFontFormatList->Clear();

    Sequence< OUString > aNodes( GetNodeNames( A2OU( FONT_FORMAT_LIST ) ) );
    const OUString *pNode = aNodes.getConstArray();
    sal_Int32 nNodes = aNodes.getLength();

    for (sal_Int32 i = 0;  i < nNodes;  ++i)
    {
        SmFontFormat aFntFmt;
        ReadFontFormat( aFntFmt, pNode[i], A2OU( FONT_FORMAT_LIST ) );
        if (!pFontFormatList->GetFontFormat( pNode[i] ))
        {
            OSL_ENSURE( 0 == pFontFormatList->GetFontFormat( pNode[i] ),
                    "FontFormat ID already exists" );
            pFontFormatList->AddFontFormat( pNode[i], aFntFmt );
        }
    }
    pFontFormatList->SetModified( false );
}


void SmMathConfig::ReadFontFormat( SmFontFormat &rFontFormat,
        const OUString &rSymbolName, const OUString &rBaseNode ) const
{
    Sequence< OUString > aNames = lcl_GetFontPropertyNames();
    sal_Int32 nProps = aNames.getLength();

    OUString aDelim( OUString::valueOf( (sal_Unicode) '/' ) );
    OUString *pName = aNames.getArray();
    for (sal_Int32 i = 0;  i < nProps;  ++i)
    {
        OUString &rName = pName[i];
        OUString aTmp( rName );
        rName = rBaseNode;
        rName += aDelim;
        rName += rSymbolName;
        rName += aDelim;
        rName += aTmp;
    }

    const Sequence< Any > aValues = ((SmMathConfig*) this)->GetProperties( aNames );

    if (nProps  &&  aValues.getLength() == nProps)
    {
        const Any * pValue = aValues.getConstArray();

        OUString    aTmpStr;
        sal_Int16       nTmp16 = 0;

        bool bOK = true;
        if (pValue->hasValue()  &&  (*pValue >>= aTmpStr))
            rFontFormat.aName = aTmpStr;
        else
            bOK = false;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            rFontFormat.nCharSet = nTmp16; // 6.0 file-format GetSOLoadTextEncoding not needed
        else
            bOK = false;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            rFontFormat.nFamily = nTmp16;
        else
            bOK = false;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            rFontFormat.nPitch = nTmp16;
        else
            bOK = false;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            rFontFormat.nWeight = nTmp16;
        else
            bOK = false;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            rFontFormat.nItalic = nTmp16;
        else
            bOK = false;
        ++pValue;

        OSL_ENSURE( bOK, "read FontFormat failed" );
        (void)bOK;
    }
}


void SmMathConfig::SaveFontFormatList()
{
    SmFontFormatList &rFntFmtList = GetFontFormatList();

    if (!rFntFmtList.IsModified())
        return;

    Sequence< OUString > aNames = lcl_GetFontPropertyNames();
    sal_Int32 nSymbolProps = aNames.getLength();

    size_t nCount = rFntFmtList.GetCount();

    Sequence< PropertyValue > aValues( nCount * nSymbolProps );
    PropertyValue *pValues = aValues.getArray();

    PropertyValue *pVal = pValues;
    OUString aDelim( OUString::valueOf( (sal_Unicode) '/' ) );
    for (size_t i = 0;  i < nCount;  ++i)
    {
        String aFntFmtId( rFntFmtList.GetFontFormatId( i ) );
        const SmFontFormat aFntFmt( *rFntFmtList.GetFontFormat( aFntFmtId ) );

        OUString  aNodeNameDelim( A2OU( FONT_FORMAT_LIST ) );
        aNodeNameDelim += aDelim;
        aNodeNameDelim += aFntFmtId;
        aNodeNameDelim += aDelim;

        const OUString *pName = aNames.getConstArray();;

        // Name
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= OUString( aFntFmt.aName );
        pVal++;
        // CharSet
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= (sal_Int16) aFntFmt.nCharSet; // 6.0 file-format GetSOStoreTextEncoding not needed
        pVal++;
        // Family
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= (sal_Int16) aFntFmt.nFamily;
        pVal++;
        // Pitch
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= (sal_Int16) aFntFmt.nPitch;
        pVal++;
        // Weight
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= (sal_Int16) aFntFmt.nWeight;
        pVal++;
        // Italic
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= (sal_Int16) aFntFmt.nItalic;
        pVal++;
    }
    OSL_ENSURE( sal::static_int_cast<size_t>(pVal - pValues) == nCount * nSymbolProps, "properties missing" );
    ReplaceSetProperties( A2OU( FONT_FORMAT_LIST ) , aValues );

    rFntFmtList.SetModified( false );
}


void SmMathConfig::StripFontFormatList( const std::vector< SmSym > &rSymbols )
{
    size_t i;

    // build list of used font-formats only
    //!! font-format IDs may be different !!
    SmFontFormatList aUsedList;
    for (i = 0;  i < rSymbols.size();  ++i)
    {
        OSL_ENSURE( rSymbols[i].GetName().Len() > 0, "non named symbol" );
        aUsedList.GetFontFormatId( SmFontFormat( rSymbols[i].GetFace() ) , true );
    }
    const SmFormat & rStdFmt = GetStandardFormat();
    for (i = FNT_BEGIN;  i <= FNT_END;  ++i)
    {
        aUsedList.GetFontFormatId( SmFontFormat( rStdFmt.GetFont( i ) ) , true );
    }

    // remove unused font-formats from list
    SmFontFormatList &rFntFmtList = GetFontFormatList();
    size_t nCnt = rFntFmtList.GetCount();
    SmFontFormat *pTmpFormat = new SmFontFormat[ nCnt ];
    String       *pId     = new String      [ nCnt ];
    size_t k;
    for (k = 0;  k < nCnt;  ++k)
    {
        pTmpFormat[k] = *rFntFmtList.GetFontFormat( k );
        pId[k]     = rFntFmtList.GetFontFormatId( k );
    }
    for (k = 0;  k < nCnt;  ++k)
    {
        if (0 == aUsedList.GetFontFormatId( pTmpFormat[k] ).Len())
        {
            rFntFmtList.RemoveFontFormat( pId[k] );
        }
    }
    delete [] pId;
    delete [] pTmpFormat;
}


void SmMathConfig::LoadOther()
{
    if (!pOther)
        pOther = new SmCfgOther;

    Sequence< OUString > aNames( GetOtherPropertyNames() );
    sal_Int32 nProps = aNames.getLength();

    Sequence< Any > aValues( GetProperties( aNames ) );
    if (nProps  &&  aValues.getLength() == nProps)
    {
        const Any *pValues = aValues.getConstArray();
        const Any *pVal = pValues;

        sal_Int16   nTmp16 = 0;
        bool    bTmp = false;

        // Print/Title
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bPrintTitle = bTmp;
        ++pVal;
        // Print/FormulaText
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bPrintFormulaText = bTmp;
        ++pVal;
        // Print/Frame
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bPrintFrame = bTmp;
        ++pVal;
        // Print/Size
        if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
            pOther->ePrintSize = (SmPrintSize) nTmp16;
        ++pVal;
        // Print/ZoomFactor
        if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
            pOther->nPrintZoomFactor = nTmp16;
        ++pVal;
        // LoadSave/IsSaveOnlyUsedSymbols
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bIsSaveOnlyUsedSymbols = bTmp;
        ++pVal;
        // Misc/IgnoreSpacesRight
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bIgnoreSpacesRight = bTmp;
        ++pVal;
        // View/ToolboxVisible
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bToolboxVisible = bTmp;
        ++pVal;
        // View/AutoRedraw
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bAutoRedraw = bTmp;
        ++pVal;
        // View/FormulaCursor
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bFormulaCursor = bTmp;
        ++pVal;

        OSL_ENSURE( pVal - pValues == nProps, "property mismatch" );
        SetOtherModified( false );
    }
}


void SmMathConfig::SaveOther()
{
    if (!pOther || !IsOtherModified())
        return;

    const Sequence< OUString > aNames( GetOtherPropertyNames() );
    sal_Int32 nProps = aNames.getLength();

    Sequence< Any > aValues( nProps );
    Any *pValues = aValues.getArray();
    Any *pValue  = pValues;

    // Print/Title
    *pValue++ <<= (sal_Bool) pOther->bPrintTitle;
    // Print/FormulaText
    *pValue++ <<= (sal_Bool) pOther->bPrintFormulaText;
    // Print/Frame
    *pValue++ <<= (sal_Bool) pOther->bPrintFrame;
    // Print/Size
    *pValue++ <<= (sal_Int16) pOther->ePrintSize;
    // Print/ZoomFactor
    *pValue++ <<= (sal_Int16) pOther->nPrintZoomFactor;
    // LoadSave/IsSaveOnlyUsedSymbols
    *pValue++ <<= (sal_Bool) pOther->bIsSaveOnlyUsedSymbols;
    // Misc/IgnoreSpacesRight
    *pValue++ <<= (sal_Bool) pOther->bIgnoreSpacesRight;
    // View/ToolboxVisible
    *pValue++ <<= (sal_Bool) pOther->bToolboxVisible;
    // View/AutoRedraw
    *pValue++ <<= (sal_Bool) pOther->bAutoRedraw;
    // View/FormulaCursor
    *pValue++ <<= (sal_Bool) pOther->bFormulaCursor;

    OSL_ENSURE( pValue - pValues == nProps, "property mismatch" );
    PutProperties( aNames , aValues );

    SetOtherModified( false );
}

void SmMathConfig::LoadFormat()
{
    if (!pFormat)
        pFormat = new SmFormat;


    Sequence< OUString > aNames( GetFormatPropertyNames() );
    sal_Int32 nProps = aNames.getLength();

    Sequence< Any > aValues( GetProperties( aNames ) );
    if (nProps  &&  aValues.getLength() == nProps)
    {
        const Any *pValues = aValues.getConstArray();
        const Any *pVal = pValues;

        OUString    aTmpStr;
        sal_Int16       nTmp16 = 0;
        bool        bTmp = false;

        // StandardFormat/Textmode
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pFormat->SetTextmode( bTmp );
        ++pVal;
        // StandardFormat/GreekCharStyle
        if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
            pFormat->SetGreekCharStyle( nTmp16 );
        ++pVal;
        // StandardFormat/ScaleNormalBracket
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pFormat->SetScaleNormalBrackets( bTmp );
        ++pVal;
        // StandardFormat/HorizontalAlignment
        if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
            pFormat->SetHorAlign( (SmHorAlign) nTmp16 );
        ++pVal;
        // StandardFormat/BaseSize
        if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
            pFormat->SetBaseSize( Size(0, SmPtsTo100th_mm( nTmp16 )) );
        ++pVal;

        sal_uInt16 i;
        for (i = SIZ_BEGIN;  i <= SIZ_END;  ++i)
        {
            if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
                pFormat->SetRelSize( i, nTmp16 );
            ++pVal;
        }

        for (i = DIS_BEGIN;  i <= DIS_END;  ++i)
        {
            if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
                pFormat->SetDistance( i, nTmp16 );
            ++pVal;
        }

        LanguageType nLang = Application::GetSettings().GetUILanguage();
        for (i = FNT_BEGIN;  i < FNT_END;  ++i)
        {
            Font aFnt;
            bool bUseDefaultFont = true;
            if (pVal->hasValue()  &&  (*pVal >>= aTmpStr))
            {
                bUseDefaultFont = 0 == aTmpStr.getLength();
                if (bUseDefaultFont)
                {
                    aFnt = pFormat->GetFont( i );
                    aFnt.SetName( GetDefaultFontName( nLang, i ) );
                }
                else
                {
                    const SmFontFormat *pFntFmt = GetFontFormatList().GetFontFormat( aTmpStr );
                    OSL_ENSURE( pFntFmt, "unknown FontFormat" );
                    if (pFntFmt)
                        aFnt = pFntFmt->GetFont();
                }
            }
            ++pVal;

            aFnt.SetSize( pFormat->GetBaseSize() );
            pFormat->SetFont( i, aFnt, bUseDefaultFont );
        }

        OSL_ENSURE( pVal - pValues == nProps, "property mismatch" );
        SetFormatModified( false );
    }
}


void SmMathConfig::SaveFormat()
{
    if (!pFormat || !IsFormatModified())
        return;

    const Sequence< OUString > aNames( GetFormatPropertyNames() );
    sal_Int32 nProps = aNames.getLength();

    Sequence< Any > aValues( nProps );
    Any *pValues = aValues.getArray();
    Any *pValue  = pValues;

    // StandardFormat/Textmode
    *pValue++ <<= (sal_Bool) pFormat->IsTextmode();
    // StandardFormat/GreekCharStyle
    *pValue++ <<= (sal_Int16) pFormat->GetGreekCharStyle();
    // StandardFormat/ScaleNormalBracket
    *pValue++ <<= (sal_Bool) pFormat->IsScaleNormalBrackets();
    // StandardFormat/HorizontalAlignment
    *pValue++ <<= (sal_Int16) pFormat->GetHorAlign();
    // StandardFormat/BaseSize
    *pValue++ <<= (sal_Int16) SmRoundFraction( Sm100th_mmToPts(
                                    pFormat->GetBaseSize().Height() ) );

    sal_uInt16 i;
    for (i = SIZ_BEGIN;  i <= SIZ_END;  ++i)
        *pValue++ <<= (sal_Int16) pFormat->GetRelSize( i );

    for (i = DIS_BEGIN;  i <= DIS_END;  ++i)
        *pValue++ <<= (sal_Int16) pFormat->GetDistance( i );

    for (i = FNT_BEGIN;  i < FNT_END;  ++i)
    {
        OUString aFntFmtId;

        if (!pFormat->IsDefaultFont( i ))
        {
            SmFontFormat aFntFmt( pFormat->GetFont( i ) );
            aFntFmtId = GetFontFormatList().GetFontFormatId( aFntFmt, true );
            OSL_ENSURE( aFntFmtId.getLength(), "FontFormatId not found" );
        }

        *pValue++ <<= aFntFmtId;
    }

    OSL_ENSURE( pValue - pValues == nProps, "property mismatch" );
    PutProperties( aNames , aValues );

    SetFormatModified( false );
}


const SmFormat & SmMathConfig::GetStandardFormat() const
{
    if (!pFormat)
        ((SmMathConfig *) this)->LoadFormat();
    return *pFormat;
}


void SmMathConfig::SetStandardFormat( const SmFormat &rFormat, bool bSaveFontFormatList )
{
    if (!pFormat)
        LoadFormat();
    if (rFormat != *pFormat)
    {
        *pFormat = rFormat;
        SetFormatModified( true );
        SaveFormat();

        if (bSaveFontFormatList)
        {
            // needed for SmFontTypeDialog's DefaultButtonClickHdl
            SetFontFormatListModified( true );
            SaveFontFormatList();
        }
    }
}


SmPrintSize SmMathConfig::GetPrintSize() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->ePrintSize;
}


void SmMathConfig::SetPrintSize( SmPrintSize eSize )
{
    if (!pOther)
        LoadOther();
    if (eSize != pOther->ePrintSize)
    {
        pOther->ePrintSize = eSize;
        SetOtherModified( true );
    }
}


sal_uInt16 SmMathConfig::GetPrintZoomFactor() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->nPrintZoomFactor;
}


void SmMathConfig::SetPrintZoomFactor( sal_uInt16 nVal )
{
    if (!pOther)
        LoadOther();
    if (nVal != pOther->nPrintZoomFactor)
    {
        pOther->nPrintZoomFactor = nVal;
        SetOtherModified( true );
    }
}


void SmMathConfig::SetOtherIfNotEqual( bool &rbItem, bool bNewVal )
{
    if (bNewVal != rbItem)
    {
        rbItem = bNewVal;
        SetOtherModified( true );
    }
}


bool SmMathConfig::IsPrintTitle() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bPrintTitle;
}


void SmMathConfig::SetPrintTitle( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bPrintTitle, bVal );
}


bool SmMathConfig::IsPrintFormulaText() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bPrintFormulaText;
}


void SmMathConfig::SetPrintFormulaText( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bPrintFormulaText, bVal );
}

bool SmMathConfig::IsSaveOnlyUsedSymbols() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bIsSaveOnlyUsedSymbols;
}

bool SmMathConfig::IsPrintFrame() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bPrintFrame;
}


void SmMathConfig::SetPrintFrame( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bPrintFrame, bVal );
}


void SmMathConfig::SetSaveOnlyUsedSymbols( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bIsSaveOnlyUsedSymbols, bVal );
}


bool SmMathConfig::IsIgnoreSpacesRight() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bIgnoreSpacesRight;
}


void SmMathConfig::SetIgnoreSpacesRight( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bIgnoreSpacesRight, bVal );
}


bool SmMathConfig::IsAutoRedraw() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bAutoRedraw;
}


void SmMathConfig::SetAutoRedraw( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bAutoRedraw, bVal );
}


bool SmMathConfig::IsShowFormulaCursor() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bFormulaCursor;
}


void SmMathConfig::SetShowFormulaCursor( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bFormulaCursor, bVal );
}

void SmMathConfig::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& )
{}

/////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
