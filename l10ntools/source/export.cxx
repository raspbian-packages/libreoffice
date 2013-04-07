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

#include <stdio.h>
#include <stdlib.h>
#include <tools/fsys.hxx>
#include "export.hxx"
#include "tokens.h"
#include <iostream>
#include <vector>
#include <rtl/strbuf.hxx>
#include <comphelper/string.hxx>

using comphelper::string::getToken;

extern "C" { int yyerror( const char * ); }
extern "C" { int YYWarning( const char * ); }

Export *pExport = 0L;

// defines to parse command line
#define STATE_NON       0x0001
#define STATE_INPUT     0x0002
#define STATE_OUTPUT    0x0003
#define STATE_PRJ       0x0004
#define STATE_ROOT      0x0005
#define STATE_MERGESRC  0x0006
#define STATE_ERRORLOG  0x0007
#define STATE_BREAKHELP 0x0008
#define STATE_UNMERGE   0x0009
#define STATE_LANGUAGES 0X000A

// set of global variables
typedef ::std::vector< ByteString* > FileList;
FileList aInputFileList;
sal_Bool bEnableExport;
sal_Bool bMergeMode;
sal_Bool bErrorLog;
sal_Bool bBreakWhenHelpText;
sal_Bool bUnmerge;
sal_Bool bUTF8;
rtl::OString sPrj;
rtl::OString sPrjRoot;
rtl::OString sActFileName;
rtl::OString sOutputFile;
rtl::OString sMergeSrc;
rtl::OString sTempFile;
rtl::OString sFile;
MergeDataFile *pMergeDataFile;
FILE *pTempFile;

extern "C" {
// the whole interface to lexer is in this extern "C" section


/*****************************************************************************/
extern char *GetOutputFile( int argc, char* argv[])
/*****************************************************************************/
{
    bEnableExport = sal_False;
    bMergeMode = sal_False;
    bErrorLog = sal_True;
    bBreakWhenHelpText = sal_False;
    bUnmerge = sal_False;
    bUTF8 = sal_True;
    sPrj = "";
    sPrjRoot = "";
    sActFileName = "";
    Export::sLanguages = "";
    Export::sForcedLanguages = "";
    sTempFile = "";
    pTempFile = NULL;
    sal_uInt16 nState = STATE_NON;
    sal_Bool bInput = sal_False;

    // parse command line
    for( int i = 1; i < argc; i++ ) {
        ByteString sSwitch( argv[ i ] );

        if (sSwitch == "-i"  || sSwitch == "-I" ) {
            nState = STATE_INPUT; // next tokens specifies source files
        }
        else if (sSwitch == "-o"  || sSwitch == "-O" ) {
            nState = STATE_OUTPUT; // next token specifies the dest file
        }
        else if (sSwitch == "-p"  || sSwitch == "-P" ) {
            nState = STATE_PRJ; // next token specifies the cur. project
        }

        else if (sSwitch == "-r"  || sSwitch == "-R" ) {
            nState = STATE_ROOT; // next token specifies path to project root
        }
        else if (sSwitch == "-m"  || sSwitch == "-M" ) {
            nState = STATE_MERGESRC; // next token specifies the merge database
        }
        else if (sSwitch == "-e"  || sSwitch == "-E" ) {
            nState = STATE_ERRORLOG;
            bErrorLog = sal_False;
        }
        else if (sSwitch == "-b"  || sSwitch == "-B" ) {
            nState = STATE_BREAKHELP;
            bBreakWhenHelpText = sal_True;
        }
        else if (sSwitch == "-u"  || sSwitch == "-U" ) {
            nState = STATE_UNMERGE;
            bUnmerge = sal_True;
            bMergeMode = sal_True;
        }
        else if ( sSwitch == "-l"  || sSwitch == "-L" ) {
            nState = STATE_LANGUAGES;
        }
        else {
            switch ( nState ) {
                case STATE_NON: {
                    return NULL;    // no valid command line
                }
                case STATE_INPUT: {
                    aInputFileList.push_back( new ByteString( argv[ i ] ) );
                    bInput = sal_True; // min. one source file found
                }
                break;
                case STATE_OUTPUT: {
                    sOutputFile = rtl::OString(argv[i]); // the dest. file
                }
                break;
                case STATE_PRJ: {
                    sPrj = rtl::OString(argv[i]);
                }
                break;
                case STATE_ROOT: {
                    sPrjRoot = rtl::OString(argv[i]); // path to project root
                }
                break;
                case STATE_MERGESRC: {
                    sMergeSrc = rtl::OString(argv[i]);
                    bMergeMode = sal_True; // activate merge mode, cause merge database found
                }
                break;
                case STATE_LANGUAGES: {
                    Export::sLanguages = rtl::OString(argv[i]);
                }
                break;
            }
        }
    }
    if( bUnmerge )
        sMergeSrc = rtl::OString();
    if ( bInput ) {
        // command line is valid
        bEnableExport = sal_True;
        char *pReturn = new char[ sOutputFile.getLength() + 1 ];
        strcpy( pReturn, sOutputFile.getStr());  // #100211# - checked
        return pReturn;
    }

    // command line is not valid
    return NULL;
}
/*****************************************************************************/
int InitExport( char *pOutput , char* pFilename )
/*****************************************************************************/
{
    // instanciate Export
    rtl::OString sOutput( pOutput );
    rtl::OString sFilename( pFilename );

    if ( bMergeMode && !bUnmerge )
    {
        // merge mode enabled, so read database
        pExport = new Export(sOutput, bEnableExport, sPrj, sPrjRoot,
            sMergeSrc , sFilename );
    }
    else
    {
        // no merge mode, only export
        pExport = new Export(sOutput, bEnableExport, sPrj, sPrjRoot,
            sFilename );
    }
    return 1;
}

/*****************************************************************************/
int EndExport()
/*****************************************************************************/
{
    delete pExport;
    return 1;
}

extern const char* getFilename()
{
    return (*(aInputFileList[ 0 ])).GetBuffer();
}
/*****************************************************************************/
extern FILE *GetNextFile()
/*****************************************************************************/
{
    // look for next valid filename in input file list
    if ( sTempFile.getLength())
    {
        fclose( pTempFile );
        String sTemp(rtl::OStringToOUString(sTempFile,
            RTL_TEXTENCODING_ASCII_US));
        DirEntry aTemp( sTemp );
        aTemp.Kill();
    }

    while ( !aInputFileList.empty() )
    {
        ByteString sFileName( *(aInputFileList[ 0 ]) );

        rtl::OString sOrigFile( sFileName );

        sFileName = Export::GetNativeFile( sFileName );
        delete aInputFileList[ 0 ];
        aInputFileList.erase( aInputFileList.begin() );

        if ( sFileName == "" ) {
            fprintf( stderr, "ERROR: Could not precompile File %s\n",
                sOrigFile.getStr());
            return GetNextFile();
        }

        sTempFile = sFileName;
        Export::RemoveUTF8ByteOrderMarkerFromFile( sFileName );

        // able to open file?
        FILE *pFile = fopen( sFileName.GetBuffer(), "r" );
        if ( !pFile )
            fprintf( stderr, "Error: Could not open File %s\n",
                sFileName.GetBuffer());
        else {
            pTempFile = pFile;

            // this is a valid file which can be opened, so
            // create path to project root
            DirEntry aEntry(rtl::OStringToOUString(sOrigFile,
                RTL_TEXTENCODING_ASCII_US));
            aEntry.ToAbs();
            rtl::OString sFullEntry(rtl::OUStringToOString(aEntry.GetFull(),
                RTL_TEXTENCODING_ASCII_US));
            aEntry += DirEntry( String( "..", RTL_TEXTENCODING_ASCII_US ));
            aEntry += DirEntry( sPrjRoot );
            rtl::OString sPrjEntry(rtl::OUStringToOString(aEntry.GetFull(),
                RTL_TEXTENCODING_ASCII_US));

            // create file name, beginnig with project root
            // (e.g.: source\ui\src\menue.src)
            sActFileName = sFullEntry.copy(sPrjEntry.getLength() + 1);


            sActFileName = sActFileName.replace('/', '\\');
            sFile = sActFileName;

            if ( pExport ) {
                // create instance of class export
                pExport->Init();
            }
            // return the valid file handle
            return pFile;
        }
    }
    // this means the file could not be opened
    return NULL;
}

int Parse( int nTyp, const char *pTokenText ){
    pExport->Execute( nTyp , pTokenText );
    return 1;
}
void Close(){
    pExport->pParseQueue->Close();
}
/*****************************************************************************/
int WorkOnTokenSet( int nTyp, char *pTokenText )
/*****************************************************************************/
{

    pExport->pParseQueue->Push( QueueEntry( nTyp , ByteString( pTokenText ) ) );
    return 1;
}

} // extern

extern "C" {
/*****************************************************************************/
int SetError()
/*****************************************************************************/
{
    // set error at global instance of class Export
    pExport->SetError();
    return 1;
}
}

extern "C" {
/*****************************************************************************/
int GetError()
/*****************************************************************************/
{
    // get error at global instance of class Export
    if ( pExport->GetError())
        return 1;
    return sal_False;
}
}

//
// class ResData
//

void ResData::Dump(){
    printf("**************\nResData\n");
    printf("sPForm = %s , sResTyp = %s , sId = %s , sGId = %s , sHelpId = %s\n",sPForm.GetBuffer()
        ,sResTyp.GetBuffer(),sId.GetBuffer(),sGId.GetBuffer(),sHelpId.GetBuffer());

    rtl::OString a("*pStringList");
    rtl::OString b("*pUIEntries");
    rtl::OString c("*pFilterList");
    rtl::OString d("*pItemList");
    rtl::OString e("*pPairedList");
    rtl::OString f("sText");

    Export::DumpMap( f , sText );

    if( pStringList )   Export::DumpExportList( a , *pStringList );
    if( pUIEntries )    Export::DumpExportList( b , *pUIEntries );
    if( pFilterList )   Export::DumpExportList( c , *pFilterList );
    if( pItemList )     Export::DumpExportList( d , *pItemList );
    if( pPairedList )   Export::DumpExportList( e , *pPairedList );
    printf("\n");
}

/*****************************************************************************/
sal_Bool ResData::SetId( const rtl::OString& rId, sal_uInt16 nLevel )
/*****************************************************************************/
{
    if ( nLevel > nIdLevel )
    {
        nIdLevel = nLevel;
        sId = rId;

        if ( bChild && bChildWithText )
        {
            rtl::OString sError(RTL_CONSTASCII_STRINGPARAM("ResId after child definition"));
            yyerror(sError.getStr());
            SetError();
        }

        if ( sId.Len() > 255 )
        {
            YYWarning( "LocalId > 255 chars, truncating..." );
            sId.Erase( 255 );
            sId = comphelper::string::stripEnd(sId, ' ');
            sId = comphelper::string::stripEnd(sId, '\t');
        }

        return sal_True;
    }

    return sal_False;
}

//
// class Export
//

Export::Export(const ByteString &rOutput, sal_Bool bWrite,
    const ByteString &rPrj, const ByteString &rPrjRoot,
    const ByteString& rFile)
                :
                pWordTransformer( NULL ),
                bDefine( sal_False ),
                bNextMustBeDefineEOL( sal_False ),
                nLevel( 0 ),
                nList( LIST_NON ),
                nListIndex( 0 ),
                nListLevel( 0 ),
                bSkipFile( false ),
                sProject( sPrj ),
                sRoot( sPrjRoot ),
                bEnableExport( bWrite ),
                bMergeMode( bUnmerge ),
                bError( sal_False ),
                bReadOver( sal_False ),
                bDontWriteOutput( sal_False ),
                sFilename( rFile )
{
    pParseQueue = new ParserQueue( *this );
    (void) rPrj;
    (void) rPrjRoot;
    (void) rFile;

    if( !isInitialized ) InitLanguages();
    // used when export is enabled

    // open output stream
    if ( bEnableExport ) {
        aOutput.Open( String( rOutput, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_WRITE | STREAM_TRUNC );
        if( !aOutput.IsOpen() ) {
            fprintf(stderr, "ERROR : Can't open file %s\n",rOutput.GetBuffer());
            exit ( -1 );
        }
        aOutput.SetStreamCharSet( RTL_TEXTENCODING_UTF8 );

        aOutput.SetLineDelimiter( LINEEND_CRLF );
    }
}

/*****************************************************************************/
Export::Export( const ByteString &rOutput, sal_Bool bWrite,
                const ByteString &rPrj, const ByteString &rPrjRoot,
                const ByteString &rMergeSource , const ByteString& rFile )
/*****************************************************************************/
                :
                pWordTransformer( NULL ),
                bDefine( sal_False ),
                bNextMustBeDefineEOL( sal_False ),
                nLevel( 0 ),
                nList( LIST_NON ),
                nListIndex( 0 ),
                nListLevel( 0 ),
                bSkipFile( false ),
                sProject( sPrj ),
                sRoot( sPrjRoot ),
                bEnableExport( bWrite ),
                bMergeMode( sal_True ),
                sMergeSrc( rMergeSource ),
                bError( sal_False ),
                bReadOver( sal_False ),
                bDontWriteOutput( sal_False ),
                sFilename( rFile )
{
    (void) rPrj;
    (void) rPrjRoot;
    (void) rFile;
    pParseQueue = new ParserQueue( *this );
    if( !isInitialized ) InitLanguages( bMergeMode );
    // used when merge is enabled

    // open output stream
    if ( bEnableExport ) {
        aOutput.Open( String( rOutput, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_WRITE | STREAM_TRUNC );
        aOutput.SetStreamCharSet( RTL_TEXTENCODING_UTF8 );
        aOutput.SetLineDelimiter( LINEEND_CRLF );
    }

}

/*****************************************************************************/
void Export::Init()
/*****************************************************************************/
{
    // resets the internal status, used before parseing another file
    sActPForm = "";
    bDefine = sal_False;
    bNextMustBeDefineEOL = sal_False;
    nLevel = 0;
    nList = LIST_NON;
    m_sListLang = rtl::OString();
    nListIndex = 0;
    for ( size_t i = 0, n = aResStack.size(); i < n;  ++i )
        delete aResStack[ i ];
    aResStack.clear();
}

/*****************************************************************************/
Export::~Export()
/*****************************************************************************/
{
    if( pParseQueue )
        delete pParseQueue;
    // close output stream
    if ( bEnableExport )
        aOutput.Close();
    for ( size_t i = 0, n = aResStack.size(); i < n;  ++i )
        delete aResStack[ i ];
    aResStack.clear();

    if ( bMergeMode && !bUnmerge ) {
        if ( !pMergeDataFile )
            pMergeDataFile = new MergeDataFile(sMergeSrc, sFile, bErrorLog);

        delete pMergeDataFile;
    }
}

/*****************************************************************************/
int Export::Execute( int nToken, const char * pToken )
/*****************************************************************************/
{

    ByteString sToken( pToken );
    ByteString sOrig( sToken );
    sal_Bool bWriteToMerged = bMergeMode;

    if ( nToken == CONDITION )
    {
        rtl::OString sTestToken(pToken);
        sTestToken = comphelper::string::remove(sTestToken, '\t');
        sTestToken = comphelper::string::remove(sTestToken, ' ');
        if (( !bReadOver ) && ( comphelper::string::indexOfL(sTestToken, RTL_CONSTASCII_STRINGPARAM("#ifndef__RSC_PARSER")) == 0 ))
            bReadOver = sal_True;
        else if (( bReadOver ) && ( comphelper::string::indexOfL(sTestToken, RTL_CONSTASCII_STRINGPARAM("#endif")) == 0 ))
            bReadOver = sal_False;
    }
    if ((( nToken < FILTER_LEVEL ) || ( bReadOver )) &&
        (!(( bNextMustBeDefineEOL ) && ( sOrig == "\n" )))) {
        // this tokens are not mandatory for parsing, so ignore them ...
        if ( bMergeMode )
            WriteToMerged( sOrig , false ); // ... ore whrite them directly to dest.
        return 0;
    }

    ResData *pResData = NULL;
    if ( nLevel ) {
        // res. exists at cur. level
        pResData = ( (nLevel-1) < aResStack.size() ) ? aResStack[ nLevel-1 ] : NULL;
    }
    else if (( nToken != RESSOURCE ) &&
            ( nToken != RESSOURCEEXPR ) &&
            ( nToken != SMALRESSOURCE ) &&
            ( nToken != LEVELUP ) &&
            ( nToken != NORMDEFINE ) &&
            ( nToken != RSCDEFINE ) &&
            ( nToken != CONDITION ) &&
            ( nToken != PRAGMA ))
    {
        // no res. exists at cur. level so return
        if ( bMergeMode )
            WriteToMerged( sOrig , false );
        return 0;
    }
    // #define NO_LOCALIZE_EXPORT
    if( bSkipFile ){
        if ( bMergeMode ) {
            WriteToMerged( sOrig , false );
        }
        return 1;
    }


    if ( bDefine ) {
        if (( nToken != EMPTYLINE ) && ( nToken != LEVELDOWN ) && ( nToken != LEVELUP )) {
            // cur. res. defined in macro
            if ( bNextMustBeDefineEOL ) {
                if ( nToken != RSCDEFINELEND ) {
                    // end of macro found, so destroy res.
                    bDefine = sal_False;
                    if ( bMergeMode ) {
                        MergeRest( pResData );
                    }
                    bNextMustBeDefineEOL = sal_False;
                    Execute( LEVELDOWN, "" );
                }
                else {
                    // next line also in macro definition
                    bNextMustBeDefineEOL = sal_False;
                    if ( bMergeMode )
                        WriteToMerged( sOrig , false );
                    return 1;
                }
            }
        }
    }

    sal_Bool bExecuteDown = sal_False;
    if ( nToken != LEVELDOWN ) {
        sal_uInt16 nOpen = 0;
        sal_uInt16 nClose = 0;
        sal_Bool bReadOver1 = sal_False;
        sal_uInt16 i = 0;
        for ( i = 0; i < sToken.Len(); i++ ) {
            if ( sToken.GetChar( i ) == '\"' )
                bReadOver1 = !bReadOver1;
            if ( !bReadOver1 && ( sToken.GetChar( i ) == '{' ))
                nOpen++;
        }

        bReadOver1 = sal_False;
        for ( i = 0; i < sToken.Len(); i++ ) {
            if ( sToken.GetChar( i ) == '\"' )
                bReadOver1 = !bReadOver1;
            if ( !bReadOver1 && ( sToken.GetChar( i ) == '}' ))
                nClose++;
        }

        if ( nOpen < nClose )
            bExecuteDown = sal_True;
    }
    switch ( nToken ) {

        case NORMDEFINE:
                        while( sToken.SearchAndReplace( "\r", " " ) != STRING_NOTFOUND ) {};
                        while( sToken.SearchAndReplace( "\t", " " ) != STRING_NOTFOUND ) {};
                        while( sToken.SearchAndReplace( "  ", " " ) != STRING_NOTFOUND ) {};
                        if( sToken.EqualsIgnoreCaseAscii( "#define NO_LOCALIZE_EXPORT" ) ){
                            bSkipFile = true;
                            return 0;
                        }
                        if ( bMergeMode )
                          WriteToMerged( sOrig , false );

                        return 0;


        case RSCDEFINE:
            bDefine = sal_True; // res. defined in macro

        case RESSOURCE:
        case RESSOURCEEXPR: {
            bDontWriteOutput = sal_False;
            if ( nToken != RSCDEFINE )
                bNextMustBeDefineEOL = sal_False;
            // this is the beginning of a new res.
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack[ nLevel - 2 ]->bChild = sal_True;
            }

            // create new instance for this res. and fill mandatory fields

            pResData = new ResData( sActPForm, FullId() , sFilename );
            aResStack.push_back( pResData );
            ByteString sBackup( sToken );
            sToken = comphelper::string::remove(sToken, '\n');
            sToken = comphelper::string::remove(sToken, '\r');
            sToken = comphelper::string::remove(sToken, '{');
            while( sToken.SearchAndReplace( "\t", " " ) != STRING_NOTFOUND ) {};
            sToken = comphelper::string::stripEnd(sToken, ' ');
            rtl::OString sTLower = getToken(sToken, 0, ' ').toAsciiLowerCase();
            pResData->sResTyp = sTLower;
            ByteString sId( sToken.Copy( pResData->sResTyp.Len() + 1 ));
            ByteString sCondition;
            if ( sId.Search( "#" ) != STRING_NOTFOUND )
            {
                // between ResTyp, Id and paranthes is a precomp. condition
                sCondition = "#";
                sCondition += ByteString(getToken(sId, 1, '#'));
                sId = getToken(sId, 0, '#');
            }
            sId = getToken(sId, 0, '/');
            CleanValue( sId );
            sId = comphelper::string::remove(sId, '\t');
            pResData->SetId( sId, ID_LEVEL_IDENTIFIER );
            if ( sCondition.Len())
            {
                Execute( CONDITION, "");  // execute the precomp. condition
            }
        }
        break;
        case SMALRESSOURCE: {
            bDontWriteOutput = sal_False;
            // this is the beginning of a new res.
            bNextMustBeDefineEOL = sal_False;
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack[ nLevel - 2 ]->bChild = sal_True;
            }

            // create new instance for this res. and fill mandatory fields

            pResData = new ResData( sActPForm, FullId() , sFilename );
            aResStack.push_back( pResData );
            sToken = comphelper::string::remove(sToken, '\n');
            sToken = comphelper::string::remove(sToken, '\r');
            sToken = comphelper::string::remove(sToken, '{');
            sToken = comphelper::string::remove(sToken, '\t');
            sToken = comphelper::string::remove(sToken, ' ');
            sToken = comphelper::string::remove(sToken, '\\');
            pResData->sResTyp = sToken.ToLowerAscii();
        }
        break;
        case LEVELUP: {
            // push
            if ( nList )
                nListLevel++;
            if ( nList )
                break;

            bDontWriteOutput = sal_False;
            ByteString sLowerTyp;
            if ( pResData )
                sLowerTyp = "unknown";
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack[ nLevel - 2 ]->bChild = sal_True;
            }

            ResData *pNewData = new ResData( sActPForm, FullId() , sFilename );
            pNewData->sResTyp = sLowerTyp;
            aResStack.push_back( pNewData );
        }
        break;
        case LEVELDOWN: {
            // pop
            if ( !nList  ) {
                bDontWriteOutput = sal_False;
                if ( nLevel ) {
                    if ( bDefine && (nLevel == 1 )) {
                        bDefine = sal_False;
                        bNextMustBeDefineEOL = sal_False;
                    }
                    WriteData( pResData );
                    ResStack::iterator it = aResStack.begin();
                    ::std::advance( it, nLevel-1 );
                    delete *it;
                    aResStack.erase( it );
                    nLevel--;
                }
            }
            else {
                if ( bDefine )
                    bNextMustBeDefineEOL = sal_True;
                if ( !nListLevel ) {
                    if ( bMergeMode )
                        MergeRest( pResData, MERGE_MODE_LIST );
                    nList = LIST_NON;
                }
                else
                    nListLevel--;
            }
        }
        break;
        case ASSIGNMENT:
        {
            bDontWriteOutput = sal_False;
            // interpret different types of assignement
            rtl::OString sKey = getToken(sToken, 0, '=');
            sKey = comphelper::string::remove(sKey, ' ');
            sKey = comphelper::string::remove(sKey, '\t');
            ByteString sValue = getToken(sToken, 1, '=');
            CleanValue( sValue );
            sKey = sKey.toAsciiUpperCase();
            if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("IDENTIFIER")))
            {
                ByteString sId(comphelper::string::remove(sValue, '\t'));
                sId = comphelper::string::remove(sId, ' ');
                pResData->SetId(sId, ID_LEVEL_IDENTIFIER);
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("HELPID")))
            {
                pResData->sHelpId = sValue;
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("STRINGLIST")))
            {
                pResData->bList = sal_True;
                nList = LIST_STRING;
                m_sListLang = SOURCE_LANGUAGE;
                nListIndex = 0;
                nListLevel = 0;
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("FILTERLIST")))
            {
                pResData->bList = sal_True;
                nList = LIST_FILTER;
                m_sListLang = SOURCE_LANGUAGE;
                nListIndex = 0;
                nListLevel = 0;
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("UIENTRIES")))
            {
                pResData->bList = sal_True;
                nList = LIST_UIENTRIES;
                m_sListLang = SOURCE_LANGUAGE;
                nListIndex = 0;
                nListLevel = 0;
            }
            if (( sToken.Search( "{" ) != STRING_NOTFOUND ) &&
                ( sToken.GetTokenCount( '{' ) > sToken.GetTokenCount( '}' )))
            {
                Parse( LEVELUP, "" );
            }
         }
        break;
        case UIENTRIES:
        case LISTASSIGNMENT:
        {
            bDontWriteOutput = sal_False;
            ByteString sTmpToken(comphelper::string::remove(sToken, ' '));
            sal_uInt16 nPos = 0;
            nPos = sTmpToken.ToLowerAscii().Search("[en-us]=");
            if( nPos != STRING_NOTFOUND ) {
                rtl::OString sKey = sTmpToken.Copy( 0 , nPos );
                sKey = comphelper::string::remove(sKey, ' ');
                sKey = comphelper::string::remove(sKey, '\t');
                ByteString sValue = getToken(sToken, 1, '=');
                CleanValue( sValue );
                sKey = sKey.toAsciiUpperCase();
                if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("STRINGLIST")))
                {
                    pResData->bList = sal_True;
                    nList = LIST_STRING;
                    m_sListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("FILTERLIST")))
                {
                    pResData->bList = sal_True;
                    nList = LIST_FILTER;
                    m_sListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                // PairedList
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("PAIREDLIST")))
                {
                    pResData->bList = sal_True;
                    nList = LIST_PAIRED;
                    m_sListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("ITEMLIST")))
                {
                    pResData->bList = sal_True;
                    nList = LIST_ITEM;
                    m_sListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("UIENTRIES")))
                {
                    pResData->bList = sal_True;
                    nList = LIST_UIENTRIES;
                    m_sListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }
            }
            else
            {
                // new res. is a String- or FilterList
                rtl::OString sKey = getToken(sToken, 0, '[');
                sKey = comphelper::string::remove(sKey, ' ');
                sKey = comphelper::string::remove(sKey, '\t');
                sKey = sKey.toAsciiUpperCase();
                if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("STRINGLIST")))
                    nList = LIST_STRING;
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("FILTERLIST")))
                    nList = LIST_FILTER;
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("PAIREDLIST")))
                    nList = LIST_PAIRED;                // abcd
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("ITEMLIST")))
                    nList = LIST_ITEM;
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("UIENTRIES")))
                    nList = LIST_UIENTRIES;
                if ( nList ) {
                    ByteString sLang = getToken(getToken(sToken, 1, '['), 0, ']');
                    CleanValue( sLang );
                    m_sListLang = sLang;
                    nListIndex = 0;
                    nListLevel = 0;
                }
            }
        }
        break;
        case TEXT:
        case _LISTTEXT:
        case LISTTEXT: {
            // this is an entry for a String- or FilterList
            if ( nList ) {
                SetChildWithText();
                ByteString sEntry(getToken(sToken, 1, '\"'));
                if ( sToken.GetTokenCount( '\"' ) > 3 )
                    sEntry += "\"";
                if ( sEntry == "\\\"" )
                    sEntry = "\"";
                InsertListEntry( sEntry, sOrig );
                if ( bMergeMode && ( sEntry != "\"" ))
                {
                    PrepareTextToMerge( sOrig, nList, m_sListLang, pResData );
                }
            }
        }
        break;
        case LONGTEXTLINE:
        case TEXTLINE:
            bDontWriteOutput = sal_False;
            if ( nLevel )
            {
                CutComment( sToken );

                // this is a text line!!!
                rtl::OString sKey = getToken(getToken(sToken, 0, '='), 0, '[');
                sKey = comphelper::string::remove(sKey, ' ');
                sKey = comphelper::string::remove(sKey, '\t');
                ByteString sText( GetText( sToken, nToken ));
                ByteString sLang;
                if ( getToken(sToken, 0, '=').indexOf('[') != -1 )
                {
                    sLang = getToken(getToken(getToken(sToken, 0, '='), 1, '['), 0, ']');
                    CleanValue( sLang );
                }
                rtl::OString sLangIndex = sLang;
                ByteString sOrigKey = sKey;
                if ( sText.Len() && sLang.Len() )
                {
                    sKey = sKey.toAsciiUpperCase();
                    if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("TEXT")) ||
                        sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("MESSAGE"))  ||
                        sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("CUSTOMUNITTEXT"))  ||
                        sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("SLOTNAME"))  ||
                        sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("UINAME")))
                    {
                        SetChildWithText();
                        if ( Export::isSourceLanguage( sLangIndex ) )
                            pResData->SetId( sText, ID_LEVEL_TEXT );

                        pResData->bText = sal_True;
                        pResData->sTextTyp = sOrigKey;
                        if ( bMergeMode ) {
                            PrepareTextToMerge( sOrig, STRING_TYP_TEXT, sLangIndex, pResData );
                        }
                        else
                        {
                            if (pResData->sText[ sLangIndex ].getLength())
                            {
                                rtl::OStringBuffer sError(RTL_CONSTASCII_STRINGPARAM("Language "));
                                sError.append(sLangIndex);
                                sError.append(RTL_CONSTASCII_STRINGPARAM("defined twice"));
                                yyerror(sError.getStr());
                            }
                            pResData->sText[ sLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "HELPTEXT" ) {
                        SetChildWithText();
                        pResData->bHelpText = sal_True;
                        if ( bBreakWhenHelpText )
                        {
                            YYWarning("\"HelpText\" found in source");
                            SetError();
                        }
                        if ( bMergeMode )
                            PrepareTextToMerge( sOrig, STRING_TYP_HELPTEXT, sLangIndex, pResData );
                        else
                        {
                            if (pResData->sHelpText[ sLangIndex ].getLength())
                            {
                                rtl::OStringBuffer sError(RTL_CONSTASCII_STRINGPARAM("Language "));
                                sError.append(sLangIndex);
                                sError.append(" defined twice");
                                YYWarning(sError.getStr());
                            }
                            pResData->sHelpText[ sLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "QUICKHELPTEXT" ) {
                        SetChildWithText();
                        pResData->bQuickHelpText = sal_True;
                        if ( bMergeMode )
                            PrepareTextToMerge( sOrig, STRING_TYP_QUICKHELPTEXT, sLangIndex, pResData );
                        else
                        {
                            if (pResData->sQuickHelpText[ sLangIndex ].getLength())
                            {
                                rtl::OStringBuffer sError(RTL_CONSTASCII_STRINGPARAM("Language "));
                                sError.append(sLangIndex);
                                sError.append(RTL_CONSTASCII_STRINGPARAM(" defined twice"));
                                YYWarning(sError.getStr());
                            }
                            pResData->sQuickHelpText[ sLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "TITLE" ) {
                        SetChildWithText();
                        pResData->bTitle = sal_True;
                        if ( bMergeMode )
                            PrepareTextToMerge( sOrig, STRING_TYP_TITLE, sLangIndex, pResData );
                        else
                        {
                            if ( pResData->sTitle[ sLangIndex ].getLength())
                            {
                                rtl::OStringBuffer sError(RTL_CONSTASCII_STRINGPARAM("Language "));
                                sError.append(sLangIndex);
                                sError.append(RTL_CONSTASCII_STRINGPARAM(" defined twice"));
                                YYWarning(sError.getStr());
                            }
                            pResData->sTitle[ sLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "ACCESSPATH" ) {
                        pResData->SetId( sText, ID_LEVEL_ACCESSPATH );
                    }
                    else if ( sKey == "FIELDNAME" ) {
                        pResData->SetId( sText, ID_LEVEL_FIELDNAME );
                    }
                }
            }
        break;
        case NEWTEXTINRES: {
            bDontWriteOutput = sal_True;
        }
        break;
        case APPFONTMAPPING:
        {
            using comphelper::string::replace;

            bDontWriteOutput = sal_False;
            // this is a AppfontMapping, so look if its a definition
            // of field size
            rtl::OString sKey = getToken(sToken, 0, '=');
            sKey = comphelper::string::remove(sKey, ' ');
            sKey = comphelper::string::remove(sKey, '\t');
            rtl::OString sMapping = getToken(sToken, 1, '=');
            sMapping = getToken(sMapping, 1, '(');
            sMapping = getToken(sMapping, 0, ')');
            sMapping = replace(sMapping, rtl::OString(' '), rtl::OString());
            sMapping = replace(sMapping, rtl::OString('\t'), rtl::OString());
            sKey = sKey.toAsciiUpperCase();
            if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("SIZE")))
                pResData->nWidth = ( sal_uInt16 ) getToken(sMapping, 0, ',').toInt32();
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("POSSIZE")))
                pResData->nWidth = ( sal_uInt16 ) getToken(sMapping, 2, ',').toInt32();
        }
        break;
        case RSCDEFINELEND:
            bDontWriteOutput = sal_False;
        break;
        case CONDITION: {
            bDontWriteOutput = sal_False;
            while( sToken.SearchAndReplace( "\r", " " ) != STRING_NOTFOUND ) {};
            while( sToken.SearchAndReplace( "\t", " " ) != STRING_NOTFOUND ) {};
            while( sToken.SearchAndReplace( "  ", " " ) != STRING_NOTFOUND ) {};
            ByteString sCondition = getToken(sToken, 0, ' ');
            if ( sCondition == "#ifndef" ) {
                sActPForm = "!defined ";
                sActPForm += ByteString(getToken(sToken, 1, ' '));
            }
            else if ( sCondition == "#ifdef" ) {
                sActPForm = "defined ";
                sActPForm += ByteString(getToken(sToken, 1, ' '));
            }
            else if ( sCondition == "#if" ) {
                sActPForm = sToken.Copy( 4 );
                while ( sActPForm.SearchAndReplace( "||", "\\or" ) != STRING_NOTFOUND ) {};
            }
            else if ( sCondition == "#elif" ) {
                sActPForm = sToken.Copy( 6 );
                while ( sActPForm.SearchAndReplace( "||", "\\or" ) != STRING_NOTFOUND ) {};
            }
            else if ( sCondition == "#else" ) {
                sActPForm = sCondition;
            }
            else if ( sCondition == "#endif" ) {
                sActPForm = "";
            }
            else break;
            if ( nLevel ) {
                WriteData( pResData, sal_True );
                pResData->sPForm = sActPForm;
            }
        }
        break;
        case EMPTYLINE : {
            bDontWriteOutput = sal_False;
            if ( bDefine ) {
                bNextMustBeDefineEOL = sal_False;
                bDefine = sal_False;
                while ( nLevel )
                    Parse( LEVELDOWN, "" );
            }
        }
        break;
        case PRAGMA : {
            bDontWriteOutput = sal_False;
            fprintf(stderr, "ERROR: archaic PRAGMA %s\n", sToken.GetBuffer());
            exit(-1);
        }
        break;
        case TEXTREFID : {
            bDontWriteOutput = sal_True;
        }
        }
    if ( bWriteToMerged ) {
        // the current token must be written to dest. without merging

        if( bDefine && sOrig.Len() > 2 ){
            for( sal_uInt16 n = 0 ; n < sOrig.Len() ; n++ ){
                if( sOrig.GetChar( n ) == '\n' && sOrig.GetChar( n-1 ) != '\\'){
                    sOrig.Insert('\\' , n++ );
                }
            }
        }
        WriteToMerged( sOrig , false);
    }

    if ( bExecuteDown ) {
        Parse( LEVELDOWN, "" );
    }

    return 1;
}

/*****************************************************************************/
void Export::CutComment( ByteString &rText )
/*****************************************************************************/
{
    if ( rText.Search( "//" ) != STRING_NOTFOUND ) {
        ByteString sWork( rText );
        sWork.SearchAndReplaceAll( "\\\"", "XX" );
        sal_uInt16 i = 0;
        sal_Bool bInner = sal_False;

        while ( i < sWork.Len() - 1 ) {
            if ( sWork.GetChar( i ) == '\"' )
                bInner = !bInner;
            else if
                (( sWork.GetChar( i ) == '/' ) &&
                ( !bInner ) &&
                ( sWork.GetChar( i + 1 ) == '/' ))
            {
                rText.Erase( i );
                return;
            }
            i++;
        }
    }
}

/*****************************************************************************/
sal_Bool Export::WriteData( ResData *pResData, sal_Bool bCreateNew )
/*****************************************************************************/
{
    if ( bMergeMode ) {
        MergeRest( pResData );
        return sal_True;
    }

    if ( bUnmerge )
        return sal_True;

       // mandatory to export: en-US

     if (( pResData->sText[ SOURCE_LANGUAGE ].getLength())
        ||
        (  pResData->sHelpText[ SOURCE_LANGUAGE ].getLength())
        ||
        (  pResData->sQuickHelpText[ SOURCE_LANGUAGE ].getLength())
         ||
        (  pResData->sTitle[ SOURCE_LANGUAGE ].getLength()))

       {
        ByteString sGID = pResData->sGId;
        ByteString sLID;
        if ( !sGID.Len())
            sGID = pResData->sId;
        else
            sLID = pResData->sId;

        ByteString sXText;
        ByteString sXHText;
        ByteString sXQHText;
        ByteString sXTitle;

        ByteString sTimeStamp( Export::GetTimeStamp());
        ByteString sCur;

        for( unsigned int n = 0; n < aLanguages.size(); n++ ){
            sCur = aLanguages[ n ];
                if ( !sCur.EqualsIgnoreCaseAscii("x-comment") ){
                    if (pResData->sText[ sCur ].getLength())
                        sXText = pResData->sText[ sCur ];
                    else {
                        sXText = pResData->sText[ SOURCE_LANGUAGE ];
                    }

                    if (pResData->sHelpText[ sCur ].getLength())
                        sXHText = pResData->sHelpText[ sCur ];
                    else {
                        sXHText = pResData->sHelpText[ SOURCE_LANGUAGE ];
                    }

                    if (pResData->sQuickHelpText[ sCur ].getLength())
                        sXQHText = pResData->sQuickHelpText[ sCur ];
                    else {
                        sXQHText = pResData->sQuickHelpText[ SOURCE_LANGUAGE ];
                    }

                    if (pResData->sTitle[ sCur ].getLength())
                        sXTitle = pResData->sTitle[ sCur ];
                    else
                        sXTitle = pResData->sTitle[ SOURCE_LANGUAGE ];

                    if ( !sXText.Len())
                        sXText = "-";

                    if ( !sXHText.Len())
                    {
                        if (pResData->sHelpText[ SOURCE_LANGUAGE ].getLength())
                            sXHText = pResData->sHelpText[ SOURCE_LANGUAGE ];
                    }
                }
                else
                    sXText = pResData->sText[ sCur ];

                if ( bEnableExport ) {
                    ByteString sOutput( sProject ); sOutput += "\t";
                    if ( sRoot.Len())
                        sOutput += sActFileName;
                    sOutput += "\t0\t";
                    sOutput += pResData->sResTyp; sOutput += "\t";
                    sOutput += sGID; sOutput += "\t";
                    sOutput += sLID; sOutput += "\t";
                    sOutput += pResData->sHelpId; sOutput   += "\t";
                    sOutput += pResData->sPForm; sOutput    += "\t";
                    sOutput += ByteString(rtl::OString::valueOf(static_cast<sal_Int64>(pResData->nWidth))); sOutput += "\t";
                    sOutput += sCur; sOutput += "\t";


                    sOutput += sXText; sOutput  += "\t";
                    sOutput += sXHText; sOutput += "\t";
                    sOutput += sXQHText; sOutput+= "\t";
                    sOutput += sXTitle; sOutput += "\t";
                    sOutput += sTimeStamp;

                    aOutput.WriteLine( sOutput );
                }

                if ( bCreateNew ) {
                    pResData->sText[ sCur ]         = "";
                    pResData->sHelpText[ sCur ]     = "";
                    pResData->sQuickHelpText[ sCur ]= "";
                    pResData->sTitle[ sCur ]        = "";
                }
            }
    }
    if ( pResData->pStringList ) {
        ByteString sList( "stringlist" );
        WriteExportList( pResData, pResData->pStringList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pStringList = 0;
    }
    if ( pResData->pFilterList ) {
        ByteString sList( "filterlist" );
        WriteExportList( pResData, pResData->pFilterList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pFilterList = 0;
    }
    if ( pResData->pItemList ) {
        ByteString sList( "itemlist" );
        WriteExportList( pResData, pResData->pItemList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pItemList = 0;
    }
    if ( pResData->pPairedList ) {
        ByteString sList( "pairedlist" );
        WriteExportList( pResData, pResData->pPairedList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pItemList = 0;
    }
    if ( pResData->pUIEntries ) {
        ByteString sList( "uientries" );
        WriteExportList( pResData, pResData->pUIEntries, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pUIEntries = 0;
    }
    return sal_True;
}
ByteString Export::GetPairedListID( const ByteString& sText ){
// < "STRING" ; IDENTIFIER ; > ;
    ByteString sIdent = getToken(sText, 1, ';');
    sIdent.ToUpperAscii();
    while( sIdent.SearchAndReplace( "\t", " " ) != STRING_NOTFOUND ) {};
    sIdent = comphelper::string::stripEnd(sIdent, ' ');
    sIdent = comphelper::string::stripStart(sIdent, ' ');
    return sIdent;
}
ByteString Export::GetPairedListString( const ByteString& sText ){
// < "STRING" ; IDENTIFIER ; > ;
    ByteString sString = getToken(sText, 0, ';');
    while( sString.SearchAndReplace( "\t", " " ) != STRING_NOTFOUND ) {};
    sString = comphelper::string::stripEnd(sString, ' ');
    ByteString s1 = sString.Copy( sString.Search( '\"' )+1 );
    sString = s1.Copy( 0 , s1.SearchBackward( '\"' ) );
    sString = comphelper::string::stripEnd(sString, ' ');
    sString = comphelper::string::stripStart(sString, ' ');
    return sString;
}
ByteString Export::StripList( const ByteString& sText ){
    ByteString s1 = sText.Copy( sText.Search( '\"' ) + 1 );
    return s1.Copy( 0 , s1.SearchBackward( '\"' ) );
}

/*****************************************************************************/
sal_Bool Export::WriteExportList( ResData *pResData, ExportList *pExportList,
                        const ByteString &rTyp, sal_Bool bCreateNew )
/*****************************************************************************/
{
    ByteString sGID = pResData->sGId;
    if ( !sGID.Len())
        sGID = pResData->sId;
    else {
        sGID += ".";
        sGID += pResData->sId;
        sGID = comphelper::string::stripEnd(sGID, '.');
    }

    ByteString sTimeStamp( Export::GetTimeStamp());
    ByteString sCur;
    for ( size_t i = 0; pExportList != NULL && i < pExportList->size(); i++ )
    {
        ExportListEntry *pEntry = (*pExportList)[  i ];

        ByteString sLID(rtl::OString::valueOf(static_cast<sal_Int64>(i + 1)));
        for (unsigned int n = 0; n < aLanguages.size(); ++n)
        {
            sCur = aLanguages[ n ];
            if ((*pEntry)[ SOURCE_LANGUAGE ].getLength())
            {
                if ( bEnableExport )
                {
                    ByteString sText((*pEntry)[ SOURCE_LANGUAGE ] );

                    // Strip PairList Line String
                    if( rTyp.EqualsIgnoreCaseAscii("pairedlist") )
                    {
                        sLID = GetPairedListID( sText );
                        if ((*pEntry)[ sCur ].getLength())
                            sText = (*pEntry)[ sCur ];
                        sText = GetPairedListString( sText );
                    }
                    else
                    {
                        sText = StripList( (*pEntry)[ sCur ] );
                        if( sText == "\\\"" )
                            sText = "\"";
                    }

                    ByteString sOutput( sProject ); sOutput += "\t";
                    if ( sRoot.Len())
                        sOutput += sActFileName;
                    sOutput += "\t0\t";
                    sOutput += rTyp; sOutput += "\t";
                    sOutput += sGID; sOutput += "\t";
                    sOutput += sLID; sOutput += "\t\t";
                    sOutput += pResData->sPForm; sOutput += "\t0\t";
                    sOutput += sCur; sOutput += "\t";

                    sOutput += sText; sOutput += "\t\t\t\t";
                    sOutput += sTimeStamp;

                    aOutput.WriteLine( sOutput );

                }
            }
        }
        if ( bCreateNew )
            delete [] pEntry;
    }
    if ( bCreateNew )
        delete pExportList;

    return sal_True;
}

/*****************************************************************************/
ByteString Export::FullId()
/*****************************************************************************/
{
    ByteString sFull;
    if ( nLevel > 1 ) {
        sFull = aResStack[ 0 ]->sId;
        for ( size_t i = 1; i < nLevel - 1; i++ ) {
            ByteString sToAdd = aResStack[ i ]->sId;
            if ( sToAdd.Len()) {
                sFull += ".";
                sFull += sToAdd;
            }
        }
    }
    if ( sFull.Len() > 255 )
    {
        rtl::OString sError(RTL_CONSTASCII_STRINGPARAM("GroupId > 255 chars"));
        printf("GroupID = %s\n", sFull.GetBuffer());
        yyerror(sError.getStr());
    }

    return sFull;
}

/*****************************************************************************/
void Export::InsertListEntry( const ByteString &rText, const ByteString &rLine )
/*****************************************************************************/
{
    ResData *pResData = ( nLevel-1 < aResStack.size() ) ? aResStack[ nLevel-1 ] : NULL;

    ExportList *pList = NULL;
    if ( nList == LIST_STRING ) {
        pList = pResData->pStringList;
        if ( !pList ) {
            pResData->pStringList = new ExportList();
            pList = pResData->pStringList;
            nListIndex = 0;
        }
    }
    else if ( nList == LIST_FILTER ) {
        pList = pResData->pFilterList;
        if ( !pList ) {
            pResData->pFilterList = new ExportList();
            pList = pResData->pFilterList;
            nListIndex = 0;
        }
    }
    else if ( nList == LIST_ITEM ) {
        pList = pResData->pItemList;
        if ( !pList ) {
            pResData->pItemList = new ExportList();
            pList = pResData->pItemList;
            nListIndex = 0;
        }
    }
    else if ( nList == LIST_PAIRED ) {
        pList = pResData->pPairedList;
        if ( !pList ) {
            pResData->pPairedList = new ExportList();
            pList = pResData->pPairedList;
            nListIndex = 0;
        }
    }
    else if ( nList == LIST_UIENTRIES ) {
        pList = pResData->pUIEntries;
        if ( !pList ) {
            pResData->pUIEntries = new ExportList();
            pList = pResData->pUIEntries;
            nListIndex = 0;
        }
    }
    else
        return;

    if ( nListIndex + 1 > pList->size())
    {
        ExportListEntry *pNew = new ExportListEntry();
        (*pNew)[LIST_REFID] = rtl::OString::valueOf(static_cast<sal_Int32>(REFID_NONE));
        pList->push_back(pNew);
    }
    ExportListEntry *pCurEntry = (*pList)[ nListIndex ];

    // For paired list use the line to set proper lid
    if( nList == LIST_PAIRED ){
        (*pCurEntry)[ m_sListLang ] = rLine;
    }else
        (*pCurEntry)[ m_sListLang ] = rText;

    if ( Export::isSourceLanguage( m_sListLang ) ) {
        if( nList == LIST_PAIRED ){
            (*pCurEntry)[ SOURCE_LANGUAGE ] = rLine;
        }
        else
            (*pCurEntry)[ SOURCE_LANGUAGE ] = rLine;

        pList->NewSourceLanguageListEntry();
    }

    nListIndex++;
}

/*****************************************************************************/
void Export::CleanValue( ByteString &rValue )
/*****************************************************************************/
{
    while ( rValue.Len()) {
        if (( rValue.GetChar( 0 ) == ' ' ) || ( rValue.GetChar( 0 ) == '\t' ))
            rValue = rValue.Copy( 1 );
        else
            break;
    }

    if ( rValue.Len()) {
        for ( sal_uInt16 i = rValue.Len() - 1; i > 0; i-- ) {
            if (( rValue.GetChar( i ) == ' ' ) || ( rValue.GetChar( i ) == '\t' ) ||
                ( rValue.GetChar( i ) == '\n' ) || ( rValue.GetChar( i ) == ';' ) ||
                ( rValue.GetChar( i ) == '{' ) || ( rValue.GetChar( i ) == '\\' ) ||
                ( rValue.GetChar( i ) == '\r' ))
                rValue.Erase( i );
            else
                break;
        }
    }
}


/*****************************************************************************/
ByteString Export::GetText( const ByteString &rSource, int nToken )
/*****************************************************************************/
#define TXT_STATE_NON   0x000
#define TXT_STATE_TEXT  0x001
#define TXT_STATE_MACRO 0x002
{
    ByteString sReturn;
    switch ( nToken )
    {
        case TEXTLINE:
        case LONGTEXTLINE:
        {
            ByteString sTmp( rSource.Copy( rSource.Search( "=" )));
            CleanValue( sTmp );
            sTmp = comphelper::string::remove(sTmp, '\n');
            sTmp = comphelper::string::remove(sTmp, '\r');

            while ( sTmp.SearchAndReplace( "\\\\\"", "-=<[BSlashBSlashHKom]>=-\"" )
                != STRING_NOTFOUND ) {};
            while ( sTmp.SearchAndReplace( "\\\"", "-=<[Hochkomma]>=-" )
                != STRING_NOTFOUND ) {};
            while ( sTmp.SearchAndReplace( "\\", "-=<[0x7F]>=-" )
                != STRING_NOTFOUND ) {};
            while ( sTmp.SearchAndReplace( "\\0x7F", "-=<[0x7F]>=-" )
                != STRING_NOTFOUND ) {};

            sal_uInt16 nStart = 0;
            sal_uInt16 nState = TXT_STATE_MACRO;

            nState = TXT_STATE_TEXT;
            nStart = 1;


            for ( sal_uInt16 i = nStart; i < sTmp.GetTokenCount( '\"' ); i++ ) {
                ByteString sToken = getToken(sTmp, i, '\"');
                if ( sToken.Len()) {
                    if ( nState == TXT_STATE_TEXT ) {
                        sReturn += sToken;
                        nState = TXT_STATE_MACRO;
                    }
                    else {
                        while( sToken.SearchAndReplace( "\t", " " ) !=
                            STRING_NOTFOUND ) {};
                        while( sToken.SearchAndReplace( "  ", " " ) !=
                            STRING_NOTFOUND ) {};
                        sToken = comphelper::string::stripStart(sToken, ' ');
                        sToken = comphelper::string::stripEnd(sToken, ' ');
                        if ( sToken.Len()) {
                            sReturn += "\\\" ";
                            sReturn += sToken;
                            sReturn += " \\\"";
                        }
                        nState = TXT_STATE_TEXT;
                    }
                }
            }

            while ( sReturn.SearchAndReplace( "-=<[0x7F]>=-", "" )
                != STRING_NOTFOUND ) {};
            while ( sReturn.SearchAndReplace( "-=<[Hochkomma]>=-", "\"" )
                != STRING_NOTFOUND ) {};
            while ( sReturn.SearchAndReplace( "-=<[BSlashBSlashHKom]>=-", "\\\\" )
                != STRING_NOTFOUND ) {};


            while ( sReturn.SearchAndReplace( "\\\\", "-=<[BSlashBSlash]>=-" )
                != STRING_NOTFOUND ) {};
            while ( sReturn.SearchAndReplace( "-=<[BSlashBSlash]>=-", "\\" )
                != STRING_NOTFOUND ) {};

        }
        break;
    }
    return sReturn;
}

/*****************************************************************************/
void Export::WriteToMerged( const ByteString &rText , bool bSDFContent )
/*****************************************************************************/
{
    if ( !bDontWriteOutput || !bUnmerge ) {
        ByteString sText( rText );
        while ( sText.SearchAndReplace( " \n", "\n" ) != STRING_NOTFOUND ) {};
        if( pParseQueue->bNextIsM && bSDFContent && sText.Len() > 2 ){
            for( sal_uInt16 n = 0 ; n < sText.Len() ; n++ ){
                if( sText.GetChar( n ) == '\n' && sText.GetChar( n-1 ) != '\\'){
                    sText.Insert('\\' , n++ );

                }
            }
        }
        else if( pParseQueue->bLastWasM && sText.Len() > 2 ){
            for( sal_uInt16 n = 0 ; n < sText.Len() ; n++ ){
                if( sText.GetChar( n ) == '\n' && sText.GetChar( n-1 ) != '\\'){
                    sText.Insert('\\' , n++ );
                }
                if( sText.GetChar( n ) == '\n' )pParseQueue->bMflag=true;
            }
        }
        else if( pParseQueue->bCurrentIsM && bSDFContent && sText.Len() > 2 ){
            for( sal_uInt16 n = 0 ; n < sText.Len() ; n++ ){
                if( sText.GetChar( n ) == '\n' && sText.GetChar( n-1 ) != '\\'){
                    sText.Insert('\\' , n++ );
                    pParseQueue->bMflag=true;
                }
            }
        }
        else if( pParseQueue->bMflag ){
            for( sal_uInt16 n = 1 ; n < sText.Len() ; n++ ){
                if( sText.GetChar( n ) == '\n' && sText.GetChar( n-1 ) != '\\'){
                    sText.Insert('\\' , n++ );
                }
            }
        }
        for (sal_uInt16 i = 0; i < sText.Len(); ++i)
        {
            if ( sText.GetChar( i ) != '\n' )
            {
                sal_Char cChar = sText.GetChar(i);
                aOutput.Write(&cChar, 1);

            }
            else
            {
                aOutput.WriteLine( ByteString());
            }
        }
    }
}

/*****************************************************************************/
void Export::ConvertMergeContent( ByteString &rText )
/*****************************************************************************/
{
    sal_Bool bNoOpen = ( rText.Search( "\\\"" ) != 0 );
    ByteString sClose( rText.Copy( rText.Len() - 2 ));
    sal_Bool bNoClose = ( sClose != "\\\"" );
    rtl::OStringBuffer sNew;
    for ( sal_uInt16 i = 0; i < rText.Len(); i++ )
    {
        rtl::OString sChar( rText.GetChar( i ));
        if (sChar.equalsL(RTL_CONSTASCII_STRINGPARAM("\\")))
        {
            if (( i + 1 ) < rText.Len())
            {
                sal_Char cNext = rText.GetChar(i + 1);
                if ( cNext == '\"' )
                {
                    sChar = rtl::OString('\"');
                    i++;
                }
                else if ( cNext == 'n' )
                {
                    sChar = rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\n"));
                    i++;
                }
                else if ( cNext == 't' )
                {
                    sChar = rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\t"));
                    i++;
                }
                else if ( cNext == '\'' )
                {
                    sChar = rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\\'"));
                    i++;
                }
                else
                {
                    sChar = rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\\\"));
                }
            }
            else
            {
                sChar = rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\\\"));
            }
        }
        else if (sChar.equalsL(RTL_CONSTASCII_STRINGPARAM("\"")))
        {
            sChar = rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\\""));
        }
        else if (sChar.equalsL(RTL_CONSTASCII_STRINGPARAM("")))
        {
            sChar = rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\0x7F"));
        }
        sNew.append(sChar);
    }

    rText = sNew.makeStringAndClear();

    if ( bNoOpen ) {
        ByteString sTmp( rText );
        rText = "\"";
        rText += sTmp;
    }
    if ( bNoClose )
        rText += "\"";
}

sal_Bool Export::PrepareTextToMerge(ByteString &rText, sal_uInt16 nTyp,
    rtl::OString &rLangIndex, ResData *pResData)
{
    // position to merge in:
    sal_uInt16 nStart = 0;
    sal_uInt16 nEnd = 0;
    ByteString sOldId = pResData->sId;
    ByteString sOldGId = pResData->sGId;
    ByteString sOldTyp = pResData->sResTyp;

    ByteString sOrigText( rText );

    switch ( nTyp ) {
        case LIST_STRING :
        case LIST_UIENTRIES :
        case LIST_FILTER :
        case LIST_PAIRED:
        case LIST_ITEM :
        {
            if ( bUnmerge )
                return sal_True;

            ExportList *pList = NULL;
            switch ( nTyp ) {
                case LIST_STRING : {
                    pResData->sResTyp = "stringlist";
                    pList = pResData->pStringList;
                }
                break;
                case LIST_UIENTRIES : {
                    pResData->sResTyp = "uientries";
                    pList = pResData->pUIEntries;
                }
                break;
                case LIST_FILTER : {
                    pResData->sResTyp = "filterlist";
                    pList = pResData->pFilterList;
                }
                break;
                case LIST_ITEM : {
                    pResData->sResTyp = "itemlist";
                    pList = pResData->pItemList;
                }
                break;
                case LIST_PAIRED : {
                    pResData->sResTyp = "pairedlist";
                    pList = pResData->pPairedList;
                }
                break;

            }
            if (pList)
            {
                ExportListEntry *pCurEntry = (*pList)[ nListIndex - 1 ];
                if ( pCurEntry )
                    rText = (*pCurEntry)[ SOURCE_LANGUAGE ];
            }

            nStart = rText.Search( "\"" );
            if ( nStart == STRING_NOTFOUND ) {
                rText = sOrigText;
                return sal_False;
            }

            sal_Bool bFound = sal_False;
            for ( nEnd = nStart + 1; nEnd < rText.Len() && !bFound; nEnd++ ) {
                if ( rText.GetChar( nEnd ) == '\"' )
                    bFound = sal_True;
            }
            if ( !bFound ) {
                rText = sOrigText;
                return sal_False;
            }

            nEnd --;
            sLastListLine = rText;
            if (( sLastListLine.Search( ">" ) != STRING_NOTFOUND ) &&
                ( sLastListLine.Search( "<" ) == STRING_NOTFOUND ))
            {
                ByteString sTmp = sLastListLine;
                sLastListLine = "<";
                sLastListLine += sTmp;
            }
            if ( pResData->sResTyp.EqualsIgnoreCaseAscii( "pairedlist" ) ){
               pResData->sId = GetPairedListID( sLastListLine );
            }
            else pResData->sId = rtl::OString::valueOf(static_cast<sal_Int32>(nListIndex));

            if ( pResData->sGId.Len())
                pResData->sGId += ".";
            pResData->sGId += sOldId;
            nTyp = STRING_TYP_TEXT;
        }
        break;
        case STRING_TYP_TEXT :
        case STRING_TYP_HELPTEXT :
        case STRING_TYP_QUICKHELPTEXT :
        case STRING_TYP_TITLE :
        {
            nStart = rText.Search( "=" );
            if ( nStart == STRING_NOTFOUND ) {
                rText = sOrigText;
                return sal_False;
            }

            nStart++;
            sal_Bool bFound = sal_False;
            while(( nStart < rText.Len()) && !bFound ) {
                if (( rText.GetChar( nStart ) != ' ' ) && ( rText.GetChar( nStart ) != '\t' ))
                    bFound = sal_True;
                else
                    nStart ++;
            }

            // no start position found
            if ( !bFound ) {
                rText = sOrigText;
                return sal_False;
            }

            // position to end mergeing in
            nEnd = rText.Len() - 1;
            bFound = sal_False;

            while (( nEnd > nStart ) && !bFound ) {
                if (( rText.GetChar( nEnd ) != ' ' ) && ( rText.GetChar( nEnd ) != '\t' ) &&
                    ( rText.GetChar( nEnd ) != '\n' ) && ( rText.GetChar( nEnd ) != ';' ) &&
                    ( rText.GetChar( nEnd ) != '{' ) && ( rText.GetChar( nEnd ) != '\\' ))
                {
                    bFound = sal_True;
                }
                else
                    nEnd --;
            }
        }
        break;
    }

    // search for merge data
    if ( !pMergeDataFile ){
        pMergeDataFile = new MergeDataFile( sMergeSrc, sFile, bErrorLog );

        // Init Languages
        ByteString sTmp = Export::sLanguages;
        if( sTmp.ToUpperAscii().Equals("ALL") )
            SetLanguages( pMergeDataFile->GetLanguages() );
        else if( !isInitialized )InitLanguages();

    }

    PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
    pResData->sId = sOldId;
    pResData->sGId = sOldGId;
    pResData->sResTyp = sOldTyp;

    if ( !pEntrys ) {
        rText = sOrigText;
        return sal_False; // no data found
    }

    ByteString sContent;
    pEntrys->GetTransex3Text(sContent, nTyp, rLangIndex);
    if (!sContent.Len() && (!Export::isSourceLanguage(rLangIndex)))
    {
        rText = sOrigText;
        return sal_False; // no data found
    }

    if (Export::isSourceLanguage(rLangIndex))
        return sal_False;

    ByteString sPostFix( rText.Copy( ++nEnd ));
    rText.Erase( nStart );

    ConvertMergeContent( sContent );



    // merge new res. in text line
    rText += sContent;
    rText += sPostFix;

    return sal_True;
}

/*****************************************************************************/
void Export::MergeRest( ResData *pResData, sal_uInt16 nMode )
/*****************************************************************************/
{
    if ( !pMergeDataFile ){
        pMergeDataFile = new MergeDataFile( sMergeSrc, sFile, bErrorLog );

        // Init Languages
        ByteString sTmp = Export::sLanguages;
        if( sTmp.ToUpperAscii().Equals("ALL") )
            SetLanguages( pMergeDataFile->GetLanguages() );
        else if( !isInitialized )InitLanguages();

    }
    switch ( nMode ) {
        case MERGE_MODE_NORMAL : {
            PFormEntrys *pEntry = pMergeDataFile->GetPFormEntrys( pResData );

            bool bWriteNoSlash = false;
            if ( pEntry && pResData->bText ) {

                sal_Bool bAddSemikolon = sal_False;
                sal_Bool bFirst = sal_True;
                ByteString sCur;
                ByteString sTmp = Export::sLanguages;

                for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                    sCur = aLanguages[ n ];

                    ByteString sText;
                    sal_Bool bText = pEntry->GetTransex3Text( sText, STRING_TYP_TEXT, sCur , sal_True );
                    if ( bText && sText.Len() && sText != "-" ) {
                        ByteString sOutput;
                        if ( bNextMustBeDefineEOL)  {
                            if ( bFirst )
                                sOutput += "\t\\\n";
                            else
                                sOutput += ";\t\\\n";
                        }
                        bFirst=sal_False;
                        sOutput += "\t";
                        sOutput += pResData->sTextTyp;
                        if ( ! Export::isSourceLanguage( sCur ) ) {
                            sOutput += "[ ";
                            sOutput += sCur;
                            sOutput += " ] ";
                        }
                        sOutput += "= ";
                        ConvertMergeContent( sText );
                        sOutput += sText;

                        if ( bDefine && bWriteNoSlash )
                            sOutput += ";\n";

                        if ( bDefine )
                            sOutput += ";\\\n";
                        else if ( !bNextMustBeDefineEOL )
                            sOutput += ";\n";
                        else
                            bAddSemikolon = sal_True;
                        for ( sal_uInt16 j = 1; j < nLevel; j++ )
                            sOutput += "\t";
                        WriteToMerged( sOutput , true );
                    }
                }


                if ( bAddSemikolon ) {
                    ByteString sOutput( ";" );
                    WriteToMerged( sOutput , false );
                }
            }

            if ( pEntry && pResData->bQuickHelpText ) {
                sal_Bool bAddSemikolon = sal_False;
                sal_Bool bFirst = sal_True;
                ByteString sCur;

                for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                    sCur = aLanguages[ n ];

                    ByteString sText;
                    sal_Bool bText = pEntry->GetTransex3Text( sText, STRING_TYP_QUICKHELPTEXT, sCur, sal_True );
                    if ( bText && sText.Len() && sText != "-" ) {
                        ByteString sOutput;
                        if ( bNextMustBeDefineEOL)  {
                            if ( bFirst )
                                sOutput += "\t\\\n";
                            else
                                sOutput += ";\t\\\n";
                        }
                        bFirst=sal_False;
                        sOutput += "\t";
                        sOutput += "QuickHelpText";
                        if ( ! Export::isSourceLanguage( sCur ) ) {
                            sOutput += "[ ";
                            sOutput += sCur;
                            sOutput += " ] ";
                        }
                        sOutput += "= ";
                        ConvertMergeContent( sText );
                        sOutput += sText;
                        if ( bDefine )
                            sOutput += ";\\\n";
                        else if ( !bNextMustBeDefineEOL )
                            sOutput += ";\n";
                        else
                            bAddSemikolon = sal_True;
                        for ( sal_uInt16 j = 1; j < nLevel; j++ )
                            sOutput += "\t";
                        WriteToMerged( sOutput ,true );
                    }
                }
                if ( bAddSemikolon ) {
                    ByteString sOutput( ";" );
                    WriteToMerged( sOutput , false );
                }
            }

            if ( pEntry && pResData->bTitle ) {
                sal_Bool bAddSemikolon = sal_False;
                sal_Bool bFirst = sal_True;
                ByteString sCur;

                for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                    sCur = aLanguages[ n ];

                ByteString sText;
                    sal_Bool bText = pEntry->GetTransex3Text( sText, STRING_TYP_TITLE, sCur, sal_True );
                    if ( bText && sText.Len() && sText != "-" ) {
                        ByteString sOutput;
                        if ( bNextMustBeDefineEOL)  {
                            if ( bFirst )
                                sOutput += "\t\\\n";
                            else
                                sOutput += ";\t\\\n";
                        }
                        bFirst=sal_False;
                        sOutput += "\t";
                        sOutput += "Title";
                        if ( ! Export::isSourceLanguage( sCur ) ) {
                            sOutput += "[ ";
                            sOutput += sCur;
                            sOutput += " ] ";
                        }
                        sOutput += "= ";
                        ConvertMergeContent( sText );
                        sOutput += sText;
                        if ( bDefine )
                            sOutput += ";\\\n";
                        else if ( !bNextMustBeDefineEOL )
                            sOutput += ";\n";
                        else
                            bAddSemikolon = sal_True;
                        for ( sal_uInt16 j = 1; j < nLevel; j++ )
                            sOutput += "\t";
                        WriteToMerged( sOutput ,true );
                    }
                }
                if ( bAddSemikolon ) {
                    ByteString sOutput( ";" );
                    WriteToMerged( sOutput ,false);
                }
            }
            // Merge Lists

            if ( pResData->bList ) {
                bool bPairedList = false;
                ByteString sOldId = pResData->sId;
                ByteString sOldGId = pResData->sGId;
                ByteString sOldTyp = pResData->sResTyp;
                if ( pResData->sGId.Len())
                    pResData->sGId += ".";
                pResData->sGId += sOldId;
                ByteString sSpace;
                for ( sal_uInt16 i = 1; i < nLevel-1; i++ )
                    sSpace += "\t";
                for ( sal_uInt16 nT = LIST_STRING; nT <= LIST_UIENTRIES; nT++ ) {
                    ExportList *pList = NULL;
                    switch ( nT ) {
                        case LIST_STRING : pResData->sResTyp = "stringlist"; pList = pResData->pStringList; bPairedList = false; break;
                        case LIST_FILTER : pResData->sResTyp = "filterlist"; pList = pResData->pFilterList; bPairedList = false; break;
                        case LIST_UIENTRIES : pResData->sResTyp = "uientries"; pList = pResData->pUIEntries;bPairedList = false; break;
                        case LIST_ITEM : pResData->sResTyp = "itemlist"; pList = pResData->pItemList;       bPairedList = false; break;
                        case LIST_PAIRED : pResData->sResTyp = "pairedlist"; pList = pResData->pPairedList; bPairedList = true;  break;
                    }
                    rtl::OString sCur;
                    for( unsigned int n = 0; n < aLanguages.size(); n++ )
                    {
                        sCur = aLanguages[ n ];
                        sal_uInt16 nIdx = 1;

                        // Set matching pairedlist identifier
                        if( bPairedList && pResData->pPairedList && ( nIdx == 1 ) ){
                            ExportListEntry* pListE = ( ExportListEntry* ) (*pResData->pPairedList)[ nIdx-1 ];
                            pResData->sId = GetPairedListID ( (*pListE)[ SOURCE_LANGUAGE ] );
                        }
                        else
                            pResData->sId = ByteString("1");

                        PFormEntrys *pEntrys;
                        sal_uLong nLIndex = 0;
                        sal_uLong nMaxIndex = 0;
                        if ( pList )
                            nMaxIndex = pList->GetSourceLanguageListEntryCount();
                        pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
                        while( pEntrys  && ( nLIndex < nMaxIndex )) {
                            ByteString sText;
                            sal_Bool bText;
                            bText = pEntrys->GetTransex3Text( sText, STRING_TYP_TEXT, sCur, sal_True );
                            if( !bText )
                                bText = pEntrys->GetTransex3Text( sText , STRING_TYP_TEXT, SOURCE_LANGUAGE , sal_False );

                            if ( bText && sText.Len())
                            {
                                if ( nIdx == 1 )
                                {
                                    rtl::OStringBuffer sHead;
                                    if ( bNextMustBeDefineEOL )
                                        sHead.append(RTL_CONSTASCII_STRINGPARAM("\\\n\t"));
                                    sHead.append(sSpace);
                                    switch ( nT )
                                    {
                                        case LIST_STRING:
                                            sHead.append(RTL_CONSTASCII_STRINGPARAM("StringList "));
                                            break;
                                        case LIST_FILTER:
                                            sHead.append(RTL_CONSTASCII_STRINGPARAM("FilterList "));
                                            break;
                                        case LIST_ITEM:
                                            sHead.append(RTL_CONSTASCII_STRINGPARAM("ItemList "));
                                            break;
                                        case LIST_PAIRED:
                                            sHead.append(RTL_CONSTASCII_STRINGPARAM("PairedList "));
                                            break;
                                        case LIST_UIENTRIES:
                                            sHead.append(RTL_CONSTASCII_STRINGPARAM("UIEntries "));
                                            break;
                                    }
                                    sHead.append(RTL_CONSTASCII_STRINGPARAM("[ "));
                                    sHead.append(sCur);
                                    sHead.append(RTL_CONSTASCII_STRINGPARAM(" ] "));
                                    //}
                                    if ( bDefine || bNextMustBeDefineEOL )
                                    {
                                        sHead.append(RTL_CONSTASCII_STRINGPARAM("= \\\n"));
                                        sHead.append(sSpace);
                                        sHead.append(RTL_CONSTASCII_STRINGPARAM("\t{\\\n\t"));
                                    }
                                    else
                                    {
                                        sHead.append(RTL_CONSTASCII_STRINGPARAM("= \n"));
                                        sHead.append(sSpace);
                                        sHead.append(RTL_CONSTASCII_STRINGPARAM("\t{\n\t"));
                                    }
                                    WriteToMerged(sHead.makeStringAndClear() , true);
                                }
                                ByteString sLine;
                                if ( pList && (*pList)[ nLIndex ] )
                                    sLine = ( *(*pList)[ nLIndex ])[ SOURCE_LANGUAGE ];
                                if ( !sLine.Len())
                                    sLine = sLastListLine;

                                if ( sLastListLine.Search( "<" ) != STRING_NOTFOUND ) {
                                    if (( nT != LIST_UIENTRIES ) &&
                                        (( sLine.Search( "{" ) == STRING_NOTFOUND ) ||
                                        ( sLine.Search( "{" ) >= sLine.Search( "\"" ))) &&
                                        (( sLine.Search( "<" ) == STRING_NOTFOUND ) ||
                                        ( sLine.Search( "<" ) >= sLine.Search( "\"" ))))
                                    {
                                        sLine.SearchAndReplace( "\"", "< \"" );
                                    }
                                }

                                sal_uInt16 nStart, nEnd;
                                nStart = sLine.Search( "\"" );

                                ByteString sPostFix;
                                if( !bPairedList ){
                                    nEnd = sLine.SearchBackward( '\"' );
                                    sPostFix = ByteString( sLine.Copy( ++nEnd ));
                                    sLine.Erase( nStart );
                                }


                                ConvertMergeContent( sText );

                                // merge new res. in text line
                                if( bPairedList ){
                                    sLine = MergePairedList( sLine , sText );
                                }
                                else{
                                    sLine += sText;
                                    sLine += sPostFix;
                                }

                                ByteString sText1( "\t" );
                                sText1 += sLine;
                                if ( bDefine || bNextMustBeDefineEOL )
                                    sText1 += " ;\\\n";
                                else
                                    sText1 += " ;\n";
                                sText1 += sSpace;
                                sText1 += "\t";
                                WriteToMerged( sText1 ,true );

                                // Set matching pairedlist identifier
                                if ( bPairedList ){
                                    nIdx++;
                                    ExportListEntry* pListE = ( ExportListEntry* )(*pResData->pPairedList)[ ( nIdx ) -1 ];
                                    if( pListE ){
                                        pResData->sId = GetPairedListID ( (*pListE)[ SOURCE_LANGUAGE ] );
                                    }
                                }
                                else
                                    pResData->sId = rtl::OString::valueOf(static_cast<sal_Int32>(++nIdx));
                            }
                            else
                                break;
                            nLIndex ++;
                            PFormEntrys *oldEntry = pEntrys;
                            pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
                            if( !pEntrys )
                                pEntrys = oldEntry;
                        }
                        if ( nIdx > 1 ) {
                            ByteString sFooter( sSpace.Copy( 1 ));
                            if ( bNextMustBeDefineEOL )
                                sFooter += "};";
                            else if ( !bDefine )
                                sFooter += "};\n\t";
                            else
                                sFooter += "\n\n";
                            WriteToMerged( sFooter ,true );
                        }
                    }
                }

                pResData->sId = sOldId;
                pResData->sGId = sOldGId;
                pResData->sResTyp = sOldTyp;
            }
        }
        break;
        case MERGE_MODE_LIST : {
            ExportList *pList = NULL;
            switch ( nList ) {
                // PairedList
                case LIST_STRING : pList = pResData->pStringList; break;
                case LIST_FILTER : pList = pResData->pFilterList; break;
                case LIST_UIENTRIES : pList = pResData->pUIEntries; break;
                case LIST_ITEM : pList = pResData->pItemList; break;
                case LIST_PAIRED : pList = pResData->pPairedList; break;

            }

            nListIndex++;
            sal_uLong nMaxIndex = 0;
            if ( pList )
                nMaxIndex = pList->GetSourceLanguageListEntryCount();
            ByteString sLine;
            if ( pList && (*pList)[ nListIndex ] )
                sLine = ( *(*pList)[ nListIndex ])[ SOURCE_LANGUAGE ];
            if ( !sLine.Len())
                sLine = sLastListLine;

            if ( sLastListLine.Search( "<" ) != STRING_NOTFOUND ) {
                if (( nList != LIST_UIENTRIES ) &&
                    (( sLine.Search( "{" ) == STRING_NOTFOUND ) ||
                    ( sLine.Search( "{" ) >= sLine.Search( "\"" ))) &&
                    (( sLine.Search( "<" ) == STRING_NOTFOUND ) ||
                    ( sLine.Search( "<" ) >= sLine.Search( "\"" ))))
                {
                    sLine.SearchAndReplace( "\"", "< \"" );
                }
            }

            while( PrepareTextToMerge( sLine, nList, m_sListLang, pResData ) && ( nListIndex <= nMaxIndex )) {
                ByteString sText( "\t" );
                sText += sLine;
                sText += " ;";
                sText += "\n";
                for ( sal_uInt16 i = 0; i < nLevel; i++ )
                    sText += "\t";
                WriteToMerged( sText ,false );
                nListIndex++;
                if ( pList && (*pList)[ nListIndex ])
                    sLine = ( *(*pList)[ nListIndex ])[ SOURCE_LANGUAGE ];
                if ( !sLine.Len())
                    sLine = sLastListLine;
                sLine += " ;";
            }
        }
        break;
    }
    pParseQueue->bMflag = false;
}

ByteString Export::MergePairedList( ByteString& sLine , ByteString& sText ){
// < "xy" ; IDENTIFIER ; >
    ByteString sPre  = sLine.Copy( 0 , sLine.Search('\"') );
    ByteString sPost = sLine.Copy( sLine.SearchBackward('\"') + 1 , sLine.Len() );
    sPre.Append( sText );
    sPre.Append( sPost );
    return sPre;
}

/*****************************************************************************/
void Export::SetChildWithText()
/*****************************************************************************/
{
    if ( aResStack.size() > 1 ) {
        for ( size_t i = 0; i < aResStack.size() - 1; i++ ) {
            aResStack[ i ]->bChildWithText = sal_True;
        }
    }
}

void ParserQueue::Push( const QueueEntry& aEntry ){
    sal_uInt16 nLen = aEntry.sLine.Len();

    if( !bStart ){
        aQueueCur->push( aEntry );
        if( nLen > 1 && aEntry.sLine.GetChar( nLen-1 ) == '\n' )
            bStart = true;
        else if ( aEntry.nTyp != IGNOREDTOKENS ){
            if( nLen > 1 && ( aEntry.sLine.GetChar( nLen-1 ) == '\\') ){
                // Next is Macro
                bCurrentIsM = true;
             }else{
                // Next is no Macro
                bCurrentIsM = false;
             }
        }
    }
    else{
        aQueueNext->push( aEntry );
        if( nLen > 1 && aEntry.sLine.GetChar( nLen-1 ) != '\n' ){
            if( nLen > 1 && ( aEntry.sLine.GetChar( nLen-1  ) == '\\') ){
                // Next is Macro
                bNextIsM = true;
            }
            else{
                // Next is no Macro
                bNextIsM = false;
            }
        }else if( nLen > 2 && aEntry.sLine.GetChar( nLen-1 ) == '\n' ){
            if( aEntry.nTyp != IGNOREDTOKENS ){
                if( nLen > 2 && ( aEntry.sLine.GetChar( nLen-2  ) == '\\') ){
                    // Next is Macro
                    bNextIsM = true;
                }
                else{
                    // Next is no Macro
                    bNextIsM = false;
                }
            }
            // Pop current
            Pop( *aQueueCur );
            bLastWasM = bCurrentIsM;
            // next -> current
            bCurrentIsM = bNextIsM;
            aQref = aQueueCur;
            aQueueCur = aQueueNext;
            aQueueNext = aQref;

        }

        else{
            // Pop current
            Pop( *aQueueCur );
            bLastWasM = bCurrentIsM;
            // next -> current
            bCurrentIsM = bNextIsM;
            aQref = aQueueCur;
            aQueueCur = aQueueNext;
            aQueueNext = aQref;
        }
    }
}

void ParserQueue::Close(){
    // Pop current
    Pop( *aQueueCur );
    // next -> current
    bLastWasM = bCurrentIsM;
    bCurrentIsM = bNextIsM;
    aQref = aQueueCur;
    aQueueCur = aQueueNext;
    aQueueNext = aQref;
    bNextIsM = false;
    Pop( *aQueueNext );
};
void ParserQueue::Pop( std::queue<QueueEntry>& aQueue ){
    while( !aQueue.empty() ){
        QueueEntry aEntry = aQueue.front();
        aQueue.pop();
        aExport.Execute( aEntry.nTyp , (char*) aEntry.sLine.GetBuffer() );
    }
}
ParserQueue::ParserQueue( Export& aExportObj )
        :
          bCurrentIsM( false ),
          bNextIsM( false ) ,
          bLastWasM( false ),
          bMflag( false ) ,
          aExport( aExportObj ) ,
          bStart( false ) ,
          bStartNext( false )
{
          aQueueNext = new std::queue<QueueEntry>;
          aQueueCur  = new std::queue<QueueEntry>;
}


ParserQueue::~ParserQueue(){
    if( aQueueNext )    delete aQueueNext;
    if( aQueueCur )     delete aQueueCur;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
