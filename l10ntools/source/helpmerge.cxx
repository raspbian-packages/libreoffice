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

#include <tools/fsys.hxx>
#include <osl/file.hxx>
// local includes
#include <stdio.h>
#include <stdlib.h>
#include "helpmerge.hxx"
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <rtl/strbuf.hxx>
#include <comphelper/string.hxx>
#ifdef WNT
#include <windows.h>
#undef CopyFile
#include <direct.h>
#endif

#if OSL_DEBUG_LEVEL > 2
void HelpParser::Dump(XMLHashMap* rElem_in)
{
    for(XMLHashMap::iterator pos = rElem_in->begin();pos != rElem_in->end(); ++pos)
    {
        Dump(pos->second,pos->first);
    }
}

void HelpParser::Dump(LangHashMap* rElem_in,const ByteString sKey_in)
{
    ByteString x;
    OString y;
    fprintf(stdout,"+------------%s-----------+\n",sKey_in.GetBuffer() );
    for(LangHashMap::iterator posn=rElem_in->begin();posn!=rElem_in->end();++posn)
    {
        x=posn->first;
        y=posn->second->ToOString();
        fprintf(stdout,"key=%s value=%s\n",x.GetBuffer(),y.getStr());
    }
    fprintf(stdout,"+--------------------------+\n");
}
#endif

HelpParser::HelpParser( const ByteString &rHelpFile, bool rUTF8 , bool rHasInputList  )
        : sHelpFile( rHelpFile ),
          bUTF8    ( rUTF8     ),
          bHasInputList( rHasInputList )
          {};

/*****************************************************************************/
bool HelpParser::CreateSDF(
/*****************************************************************************/
    const ByteString &rSDFFile_in, const ByteString &rPrj_in,const ByteString &rRoot_in,
    const ByteString &sHelpFile, XMLFile *pXmlFile, const ByteString &rGsi1){
    // GSI File constants
    static const String GSI_SEQUENCE1( String::CreateFromAscii("\t0\t") );
    static const String GSI_SEQUENCE2( String::CreateFromAscii("\t\t\t0\t")     );
    static const String GSI_TAB      ( String::CreateFromAscii("\t")            );
    static const String GSI_SEQUENCE4( String::CreateFromAscii("\t\t\t\t")      );
    static const String ret          ( String::CreateFromAscii("\n")            );
    static const String ret_char     ( String::CreateFromAscii("")              );
    static const String tab          ( String::CreateFromAscii("\t")            );
    static const String tab_char     ( String::CreateFromAscii("")              );

    SimpleXMLParser aParser;
    String sUsedTempFile;
    String sXmlFile;

    if( Export::fileHasUTF8ByteOrderMarker( sHelpFile ) )
    {
        DirEntry aTempFile = Export::GetTempFile();
        DirEntry aSourceFile( String( sHelpFile , RTL_TEXTENCODING_ASCII_US ) );
        aSourceFile.CopyTo( aTempFile , FSYS_ACTION_COPYFILE );
        String sTempFile = aTempFile.GetFull();
        Export::RemoveUTF8ByteOrderMarkerFromFile(rtl::OUStringToOString(sTempFile , RTL_TEXTENCODING_ASCII_US));
        sUsedTempFile = sTempFile;
        sXmlFile = sTempFile;
    }
    else
    {
        sUsedTempFile = String::CreateFromAscii("");
        sXmlFile = String( sHelpFile , RTL_TEXTENCODING_ASCII_US );
    }

//    ByteString fullFilePath;
    //DirEntry aFile( sXmlFile );
    //makeAbsolutePath( sHelpFile , rRoot_in);
    ByteString fullFilePath = rPrj_in;
    fullFilePath.Append( "\\" );
    fullFilePath.Append( makeAbsolutePath( sHelpFile , rRoot_in ) );
    fullFilePath.SearchAndReplaceAll( "\\", "/" );

    String strFullPath( fullFilePath.GetBuffer() , RTL_TEXTENCODING_ASCII_US );

    //printf( "%s\n", fullFilePath.GetBuffer() );
    std::auto_ptr <XMLFile> file ( aParser.Execute( strFullPath , sXmlFile, pXmlFile ) );

    if(file.get() == NULL)
    {
        printf("%s\n", rtl::OUStringToOString(aParser.GetError().sMessage, RTL_TEXTENCODING_ASCII_US).getStr());
        exit(-1);
    }
    file->Extract();
    if( !file->CheckExportStatus() ){
        return true;
    }
    SvFileStream aSDFStream( String( rSDFFile_in, RTL_TEXTENCODING_ASCII_US ),
        STREAM_STD_WRITE | STREAM_TRUNC );

    if ( !aSDFStream.IsOpen()) {
        fprintf(stdout,"Can't open file %s\n",rSDFFile_in.GetBuffer());
        return false;
    }

    ByteString sActFileName = makeAbsolutePath( sHelpFile , rRoot_in );

    XMLHashMap*  aXMLStrHM   = file->GetStrings();
    LangHashMap* pElem;
    XMLElement*  pXMLElement  = NULL;

    ByteString sTimeStamp( Export::GetTimeStamp() );
    OUString sOUTimeStamp( sTimeStamp.GetBuffer() , sTimeStamp.Len() , RTL_TEXTENCODING_ASCII_US );

    OUStringBuffer sBuffer;
    const OUString sOUPrj( rPrj_in.GetBuffer() , rPrj_in.Len() , RTL_TEXTENCODING_ASCII_US );
    const OUString sOUActFileName(sActFileName.GetBuffer() , sActFileName.Len() , RTL_TEXTENCODING_ASCII_US );
    const OUString sOUGsi1( rGsi1.GetBuffer() , rGsi1.Len() , RTL_TEXTENCODING_ASCII_US );

    Export::InitLanguages( false );
    std::vector<ByteString> aLanguages = Export::GetLanguages();

    std::vector<ByteString> order = file->getOrder();
    std::vector<ByteString>::iterator pos;
    XMLHashMap::iterator posm;

    for( pos = order.begin(); pos != order.end() ; ++pos )
    {
        posm = aXMLStrHM->find( *pos );
        pElem = posm->second;
        ByteString sCur;

        for( unsigned int n = 0; n < aLanguages.size(); n++ )
        {
            sCur = aLanguages[ n ];
            pXMLElement = (*pElem)[ sCur ];

            if( pXMLElement != NULL )
            {
                OUString data = pXMLElement->ToOUString();
                String sTmp = String(data.getStr());
                sTmp.SearchAndReplaceAll(ret,ret_char);    // Remove \n
                sTmp.SearchAndReplaceAll(tab,tab_char);    // Remove \t

                data = OUString( sTmp );
                sBuffer.append( sOUPrj );
                sBuffer.append( GSI_TAB );              //"\t";
                if ( rRoot_in.Len())
                    sBuffer.append( sOUActFileName );
                   sBuffer.append( GSI_SEQUENCE1 );     //"\t0\t";
                   sBuffer.append( sOUGsi1 );               //"help";
                   sBuffer.append( GSI_TAB );              //"\t";
                ByteString sID = posm->first;           // ID
                sBuffer.append( OUString( sID.GetBuffer() , sID.Len() , RTL_TEXTENCODING_UTF8 ) );
                sBuffer.append( GSI_TAB ); //"\t";
                   ByteString sOldRef = pXMLElement->GetOldref(); // oldref
                sBuffer.append( OUString(sOldRef.GetBuffer() , sOldRef.Len() , RTL_TEXTENCODING_UTF8 ) );
                   sBuffer.append( GSI_SEQUENCE2 );     //"\t\t\t0\t";
                sBuffer.append( OUString( sCur.GetBuffer() , sCur.Len() , RTL_TEXTENCODING_UTF8 ) );
                   sBuffer.append( GSI_TAB );               //"\t";
                sBuffer.append( data );
                  sBuffer.append( GSI_SEQUENCE4 );      //"\t\t\t\t";
                sBuffer.append( sOUTimeStamp );
                rtl::OString sOut(rtl::OUStringToOString(sBuffer.makeStringAndClear().getStr() , RTL_TEXTENCODING_UTF8));
                if( data.getLength() > 0 )
                    aSDFStream.WriteLine( sOut );
                pXMLElement=NULL;
            }else fprintf(stdout,"\nDBG: NullPointer in HelpParser::CreateSDF , Language %s\n",sCur.GetBuffer() );
        }

    }
    aSDFStream.Close();

    if( !sUsedTempFile.EqualsIgnoreCaseAscii( "" ) ){
        DirEntry aTempFile( sUsedTempFile );
        aTempFile.Kill();
    }
    return sal_True;
}

ByteString HelpParser::makeAbsolutePath( const ByteString& sHelpFile , const ByteString& rRoot_in )
{
    DirEntry aEntry( String( sHelpFile, RTL_TEXTENCODING_ASCII_US ));
    aEntry.ToAbs();
    String sFullEntry = aEntry.GetFull();
    aEntry += DirEntry( String( "..", RTL_TEXTENCODING_ASCII_US ));
    aEntry += DirEntry( rRoot_in );
    rtl::OString sPrjEntry(rtl::OUStringToOString(aEntry.GetFull(), osl_getThreadTextEncoding()));
    rtl::OString sActFileName(rtl::OUStringToOString(
        sFullEntry.Copy(sPrjEntry.getLength() + 1), osl_getThreadTextEncoding()));

    return sActFileName.replace('/', '\\');
}

bool HelpParser::Merge( const ByteString &rSDFFile, const ByteString &rDestinationFile  ,
    const rtl::OString& rLanguage , MergeDataFile& aMergeDataFile )
{

    (void) rSDFFile;
    bool hasNoError = true;

    SimpleXMLParser aParser;

    String sUsedTempFile;
    String sXmlFile;

    if( Export::fileHasUTF8ByteOrderMarker( sHelpFile ) ){
        DirEntry aTempFile = Export::GetTempFile();
        DirEntry aSourceFile( String( sHelpFile , RTL_TEXTENCODING_ASCII_US ) );
        aSourceFile.CopyTo( aTempFile , FSYS_ACTION_COPYFILE );
        String sTempFile = aTempFile.GetFull();
        Export::RemoveUTF8ByteOrderMarkerFromFile(rtl::OUStringToOString(sTempFile, RTL_TEXTENCODING_ASCII_US));
        sUsedTempFile = sTempFile;
        sXmlFile = sTempFile;
    }else{
        sUsedTempFile = String::CreateFromAscii("");
        sXmlFile = String( sHelpFile , RTL_TEXTENCODING_ASCII_US );
    }

    OUString sOUHelpFile( sXmlFile );
    String fullFilePath;
    DirEntry aFile( sXmlFile );

    XMLFile* xmlfile = ( aParser.Execute( aFile.GetFull() , sOUHelpFile, new XMLFile( '0' ) ) );
    printf("Dest file %s\n",rDestinationFile.GetBuffer());
    hasNoError = MergeSingleFile( xmlfile , aMergeDataFile , rLanguage , rDestinationFile );
    delete xmlfile;
    if( !sUsedTempFile.EqualsIgnoreCaseAscii( "" ) ){
        DirEntry aTempFile( sUsedTempFile );
        aTempFile.Kill();
    }
    return hasNoError;
}

bool ByteStringEqual( const ByteString& rKey1, const ByteString& rKey2 )  {
    return rKey1.CompareTo( rKey2 )==COMPARE_EQUAL;
};
bool ByteStringLess( const ByteString& rKey1, const ByteString& rKey2 )  {
     return rKey1.CompareTo( rKey2 )==COMPARE_LESS;
}

void HelpParser::parse_languages( std::vector<ByteString>& aLanguages , MergeDataFile& aMergeDataFile ){
    std::vector<ByteString> aTmp;

    const ByteString ENUS   ("en-US");

    Export::InitLanguages( false );

    if (Export::sLanguages.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("ALL")))
    {
        aLanguages = aMergeDataFile.GetLanguages();
        aLanguages.push_back( ENUS );

        if( !Export::sForcedLanguages.isEmpty() )
        {
            std::vector<ByteString> aFL = Export::GetForcedLanguages();
            std::copy( aFL.begin() ,
                       aFL.end() ,
                       back_inserter( aLanguages )
                     );
            std::sort(   aLanguages.begin() , aLanguages.end() , ByteStringLess );
            std::vector<ByteString>::iterator unique_iter =  std::unique( aLanguages.begin() , aLanguages.end() , ByteStringEqual );
            std::copy( aLanguages.begin() , unique_iter , back_inserter( aTmp ) );
            aLanguages = aTmp;
        }
    }
    else{
        aLanguages = Export::GetLanguages();
    }

}

bool HelpParser::Merge(
    const ByteString &rSDFFile, const ByteString &rPathX , const ByteString &rPathY , bool bISO ,
    const std::vector<ByteString>& aLanguages , MergeDataFile& aMergeDataFile , bool bCreateDir )
{


    (void) rSDFFile ;
    bool hasNoError = true;
    SimpleXMLParser aParser;
    String sUsedTempFile;
    String sXmlFile;

    if( Export::fileHasUTF8ByteOrderMarker( sHelpFile ) )
    {
        DirEntry aTempFile = Export::GetTempFile();
        DirEntry aSourceFile( String( sHelpFile , RTL_TEXTENCODING_ASCII_US ) );
        aSourceFile.CopyTo( aTempFile , FSYS_ACTION_COPYFILE );
        String sTempFile = aTempFile.GetFull();
        Export::RemoveUTF8ByteOrderMarkerFromFile(rtl::OUStringToOString(sTempFile , RTL_TEXTENCODING_ASCII_US));
        sUsedTempFile = sTempFile;
        sXmlFile = sTempFile;
    }
    else
    {
        sUsedTempFile = String::CreateFromAscii("");
        sXmlFile = String( sHelpFile , RTL_TEXTENCODING_ASCII_US );
    }


    OUString sOUHelpFile( sXmlFile );
      String fullFilePath;
    DirEntry aFile( sXmlFile );

    XMLFile* xmlfile = ( aParser.Execute( aFile.GetFull() , sOUHelpFile, new XMLFile( '0' ) ) );
    xmlfile->Extract();

    if( xmlfile == NULL)
    {
        printf("%s\n", rtl::OUStringToOString(aParser.GetError().sMessage, RTL_TEXTENCODING_UTF8).getStr());
        exit(-1);
    }


    ByteString sCur;
    for( unsigned int n = 0; n < aLanguages.size(); n++ ){
        sCur = aLanguages[ n ];

        ByteString sFilepath;
        if( bISO )  sFilepath = GetOutpath( rPathX , sCur , rPathY );
        else        sFilepath = rPathX;
        if( bCreateDir ) MakeDir( sFilepath );

        XMLFile* file = new XMLFile( *xmlfile );
        sFilepath.Append( sHelpFile );
        hasNoError = MergeSingleFile( file , aMergeDataFile , sCur , sFilepath );
        delete file;

        if( !hasNoError ) return false;         // Stop on error
     }

    if( !sUsedTempFile.EqualsIgnoreCaseAscii( "" ) )
    {
        DirEntry aTempFile( sUsedTempFile );
        aTempFile.Kill();
    }
    delete xmlfile;
    return hasNoError;
}

bool HelpParser::MergeSingleFile( XMLFile* file , MergeDataFile& aMergeDataFile , const ByteString& sLanguage ,
                                  ByteString sPath )
{
    file->Extract();

       XMLHashMap*   aXMLStrHM     = file->GetStrings();
    LangHashMap*  aLangHM;
    static  ResData pResData( "","","");
    pResData.sResTyp   = "help";

    for(XMLHashMap::iterator pos=aXMLStrHM->begin();pos!=aXMLStrHM->end();++pos)    // Merge every l10n related string
    {

        aLangHM             = pos->second;
#if OSL_DEBUG_LEVEL > 2
        printf("*********************DUMPING HASHMAP***************************************");
        Dump(aXMLStrHM);
        printf("DBG: sHelpFile = %s\n",sHelpFile.GetBuffer() );
#endif

        pResData.sGId      =  pos->first;
        pResData.sFilename  =  sHelpFile;

        ProcessHelp( aLangHM , sLanguage, &pResData , aMergeDataFile );
     }


    // Init temp and target file
    ByteString sTempFile;
    ByteString sTargetFile( sPath );
    ByteString sTempFileCopy;

    static const ByteString INPATH = Export::GetEnv( "INPATH" );
    Export::getRandomName( sPath , sTempFile , INPATH );
    Export::getRandomName( sPath , sTempFileCopy , INPATH );
    // Write in the temp file
    bool hasNoError = file->Write ( sTempFile );
    if( !hasNoError )
    {
        cerr << "ERROR: file->Write failed\n";
        return false;
    }

    DirEntry aTmp( sTempFile );
    DirEntry aTmp2( sTempFileCopy );
    DirEntry aTar( sTargetFile );

    if( !Export::CopyFile( sTempFile , sTempFileCopy ) )
    {
#if defined(UNX)
        sleep( 3 );
#else
        Sleep( 3 );
#endif
        if( !Export::CopyFile( sTempFile , sTempFileCopy ) )
        {
            cerr << "ERROR: Can not copy file from " << sTempFile.GetBuffer() << " to " << sTempFileCopy.GetBuffer() << "\n";
            return false;
        }
    }

    FileStat aFSTest( aTar );
    if( aFSTest.GetSize() < 1 )
    {
        remove( sTargetFile.GetBuffer() );
    }
    int rc;
#if defined(UNX)
    rc = rename( sTempFile.GetBuffer() , sTargetFile.GetBuffer() );
#else
    rc = MoveFileEx( sTempFile.GetBuffer() , sTargetFile.GetBuffer(), MOVEFILE_REPLACE_EXISTING );
#endif
    FileStat aFS( aTar );

    //cout << "mv " << sTempFile.GetBuffer() << " " << sTargetFile.GetBuffer() << "\n";
    //cout << "rc -> " << rc << " filesize -> " << aFS.GetSize() << "\n";
// Windows rename returns -1 if the file already exits
//#ifdef UNX
    if( rc < 0 || aFS.GetSize() < 1 )
//#else
//    if( aFS.GetSize() < 1 )
//#endif
    {
#if defined(UNX)
        sleep( 3 );
#else
        Sleep( 3 );
#endif
        aFSTest.Update( aTar );
        if( aFSTest.GetSize() < 1 )
        {
            remove( sTargetFile.GetBuffer() );
        }
#if defined(UNX)
        rc = rename( sTempFileCopy.GetBuffer() , sTargetFile.GetBuffer() );
#else
        rc = MoveFileEx( sTempFileCopy.GetBuffer() , sTargetFile.GetBuffer() , MOVEFILE_REPLACE_EXISTING );
#endif
        aFS.Update( aTar );

        //cout << "mv2 " << sTempFileCopy.GetBuffer() << " " << sTargetFile.GetBuffer() << "\n";
        //cout << "rc -> " << rc << " filesize -> " << aFS.GetSize() << "\n";

// Windows rename returns -1 if the file already exits
//#ifdef WNT
//        if( aFS.GetSize() < 1 )
//#else
        if( rc < 0 || aFS.GetSize() < 1 )
//#endif
        {
            cerr << "ERROR: helpex Can't rename file " << sTempFileCopy.GetBuffer() << " to " << sTargetFile.GetBuffer() << " rename rc=" << rc << " filesize=" << aFS.GetSize() << "\n";
            aTmp.Kill();
            aTmp2.Kill();
            if( aFS.GetSize() < 1 )
                aTar.Kill();
            return false;
        }
    }
    aTmp.Kill();
    aTmp2.Kill();

    return true;
}

ByteString HelpParser::GetOutpath( const ByteString& rPathX , const ByteString& sCur , const ByteString& rPathY )
{
    ByteString testpath = rPathX;
    static const rtl::OString sDelimiter(rtl::OUStringToOString(DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US));
    testpath = comphelper::string::stripEnd(testpath, '/');
    testpath = comphelper::string::stripEnd(testpath, '\\');
    testpath += sDelimiter;
    testpath += sCur;
    testpath += sDelimiter;
    ByteString sRelativePath( rPathY );
    sRelativePath = comphelper::string::stripStart(sRelativePath, '/');
    sRelativePath = comphelper::string::stripStart(sRelativePath, '\\');
    testpath += sRelativePath;
    testpath += sDelimiter;
    return testpath;
}

void HelpParser::MakeDir( const ByteString& sPath )
{
    ByteString sTPath( sPath );
    rtl::OString sDelimiter(rtl::OUStringToOString(DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US));
    sTPath.SearchAndReplaceAll( sDelimiter , rtl::OString('/') );
    sal_uInt16 cnt = sTPath.GetTokenCount( '/' );
    rtl::OStringBuffer sCreateDir;
    for (sal_uInt16 i = 0; i < cnt; ++i)
    {
        sCreateDir.append(comphelper::string::getToken(sTPath, i , '/'));
        sCreateDir.append(sDelimiter);
#ifdef WNT
        _mkdir( sCreateDir.getStr() );
#else
        mkdir( sCreateDir.getStr() , S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
#endif
    }
}


/* ProcessHelp Methode: search for en-US entry and replace it with the current language*/
void HelpParser::ProcessHelp( LangHashMap* aLangHM , const ByteString& sCur , ResData *pResData , MergeDataFile& aMergeDataFile ){

    XMLElement*   pXMLElement = NULL;
       PFormEntrys   *pEntrys    = NULL;
    XMLData       *data       = NULL;

    String        sNewdata;
    ByteString sLId;
    ByteString sGId;

    pEntrys = NULL;

    if( !sCur.EqualsIgnoreCaseAscii("en-US") ){
        pXMLElement = (*aLangHM)[ "en-US" ];
        if( pXMLElement == NULL )
        {
            printf("Error: Can't find en-US entry\n");
        }
        if( pXMLElement != NULL )
        {
            sLId    = pXMLElement->GetOldref();
            pResData->sId     =  sLId;

            pEntrys = aMergeDataFile.GetPFormEntrys( pResData );
            if( pEntrys != NULL)
            {
                ByteString sNewText;
                pEntrys->GetText( sNewText, STRING_TYP_TEXT, sCur , true );
                sNewdata = String(  sNewText , RTL_TEXTENCODING_UTF8 );
                if ( sNewdata.Len())
                {
                    if( pXMLElement != NULL )
                    {
                        data   = new XMLData( sNewdata , NULL , true ); // Add new one
                        pXMLElement->RemoveAndDeleteAllChilds();
                        pXMLElement->AddChild( data );
                        aLangHM->erase( sCur );
                    }
                }
            }else if( pResData == NULL ){fprintf(stdout,"Can't find GID=%s LID=%s TYP=%s\n",pResData->sGId.GetBuffer(),pResData->sId.GetBuffer(),pResData->sResTyp.GetBuffer());}
            pXMLElement->ChangeLanguageTag( String( sCur , RTL_TEXTENCODING_ASCII_US) );
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
