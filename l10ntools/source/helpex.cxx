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

// local includes
#include "helpmerge.hxx"

// defines to parse command line
#define STATE_NON               0x0001
#define STATE_INPUT             0x0002
#define STATE_OUTPUT            0x0003
#define STATE_PRJ               0x0004
#define STATE_ROOT              0x0005
#define STATE_SDFFILE           0x0006
#define STATE_ERRORLOG          0x0007
#define STATE_BREAKHELP         0x0008
#define STATE_UNMERGE           0x0009
#define STATE_LANGUAGES         0x000A
#define STATE_FORCE_LANGUAGES   0x000B
#define STATE_OUTPUTX           0xfe
#define STATE_OUTPUTY           0xff

// set of global variables
ByteString sInputFile;
sal_Bool bEnableExport;
sal_Bool bMergeMode;
sal_Bool bErrorLog;
sal_Bool bUTF8;
ByteString sPrj;
ByteString sPrjRoot;
ByteString sOutputFile;
ByteString sOutputFileX;
ByteString sOutputFileY;
ByteString sSDFFile;

/*****************************************************************************/
sal_Bool ParseCommandLine( int argc, char* argv[])
/*****************************************************************************/
{
    bEnableExport = sal_False;
    bMergeMode = sal_False;
    bErrorLog = sal_True;
    bUTF8 = sal_True;
    sPrj = "";
    sPrjRoot = "";
    Export::sLanguages = "";
    Export::sForcedLanguages = "";

    sal_uInt16 nState = STATE_NON;
    sal_Bool bInput = sal_False;

    // parse command line
    for( int i = 1; i < argc; i++ ) {
        if ( ByteString( argv[ i ]).ToUpperAscii() == "-I" ) {
            nState = STATE_INPUT; // next tokens specifies source files
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii()  == "-O" ) {
            nState = STATE_OUTPUT; // next token specifies the dest file
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii()  == "-X" ) {
            nState = STATE_OUTPUTX; // next token specifies the dest file
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii()  == "-Y" ) {
            nState = STATE_OUTPUTY; // next token specifies the dest file
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii() == "-P" ) {
            nState = STATE_PRJ; // next token specifies the cur. project
        }
         else if ( ByteString( argv[ i ]).ToUpperAscii() == "-LF" ) {
            nState = STATE_FORCE_LANGUAGES;
        }

        else if ( ByteString( argv[ i ]).ToUpperAscii() == "-R" ) {
            nState = STATE_ROOT; // next token specifies path to project root
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii() == "-M" ) {
            nState = STATE_SDFFILE; // next token specifies the merge database
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii() == "-E" ) {
            nState = STATE_ERRORLOG;
            bErrorLog = sal_False;
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii() == "-L" ) {
            nState = STATE_LANGUAGES;
        }
        else {
            switch ( nState ) {
                case STATE_NON: {
                    return sal_False;   // no valid command line
                }
                //break;
                case STATE_INPUT: {
                    sInputFile = argv[ i ];
                    bInput = sal_True; // source file found
                }
                break;
                case STATE_OUTPUT: {
                    sOutputFile = argv[ i ]; // the dest. file
                }
                break;
                case STATE_OUTPUTX: {
                    sOutputFileX = argv[ i ]; // the dest. file
                }
                break;
                case STATE_OUTPUTY: {
                    sOutputFileY = argv[ i ]; // the dest. file
                }
                break;
                case STATE_PRJ: {
                    sPrj = argv[ i ];
                }
                break;
                case STATE_ROOT: {
                    sPrjRoot = argv[ i ]; // path to project root
                }
                break;
                case STATE_SDFFILE: {
                    sSDFFile = argv[ i ];
                    bMergeMode = sal_True; // activate merge mode, cause merge database found
                }
                break;
                case STATE_LANGUAGES: {
                    Export::sLanguages = argv[ i ];
                }
                case STATE_FORCE_LANGUAGES:{
                    Export::sForcedLanguages = argv[ i ];
                }
                break;
            }
        }
    }

    if ( bInput ) {
        // command line is valid
        bEnableExport = sal_True;
        return sal_True;
    }

    // command line is not valid
    return sal_False;
}


/*****************************************************************************/
void Help()
/*****************************************************************************/
{
    fprintf( stdout, "Syntax: HELPEX[-p Prj][-r PrjRoot]-i FileIn ( -o FileOut | -x path -y relfile )[-m DataBase][-e][-b][-u][-L l1,l2,...] -LF l1,l2 \n" );
    fprintf( stdout, " Prj:      Project\n" );
    fprintf( stdout, " PrjRoot:  Path to project root (..\\.. etc.)\n" );
    fprintf( stdout, " FileIn:   Source file (*.lng)\n" );
    fprintf( stdout, " FileOut:  Destination file (*.*)\n" );
    fprintf( stdout, " DataBase: Mergedata (*.sdf)\n" );
    fprintf( stdout, " -L: Restrict the handled languages. l1,l2,... are elements of (en-US,fr,de...)\n" );
    fprintf( stdout, " -LF: Force the creation of that languages\n" );

}

/*****************************************************************************/
#ifndef TESTDRIVER

#if defined(UNX)
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/*****************************************************************************/
{

    if ( !ParseCommandLine( argc, argv )) {
        Help();
        return 1;
    }
    //sal_uInt32 startfull = Export::startMessure();

    bool hasInputList = sInputFile.GetBuffer()[0]=='@';
//    printf("x = %s , y = %s , o = %s\n", sOutputFileX.GetBuffer(),  sOutputFileY.GetBuffer() , sOutputFile.GetBuffer() );
    bool hasNoError = true;

    if ( sOutputFile.Len() ){                                               // Merge single file ?
        //printf("DBG: Inputfile = %s\n",sInputFile.GetBuffer());
        HelpParser aParser( sInputFile, bUTF8 , false );

        if ( bMergeMode )
        {
            //sal_uInt64 startreadloc = Export::startMessure();
            MergeDataFile aMergeDataFile(sSDFFile, sInputFile, sal_False);
            //Export::stopMessure( ByteString("read localize.sdf") , startreadloc );

            hasNoError = aParser.Merge( sSDFFile, sOutputFile , Export::sLanguages , aMergeDataFile );
        }
        else
            hasNoError = aParser.CreateSDF( sOutputFile, sPrj, sPrjRoot, sInputFile, new XMLFile( '0' ), "help" );
    }else if ( sOutputFileX.Len() && sOutputFileY.Len() && hasInputList ) {  // Merge multiple files ?
        if ( bMergeMode ){

            ifstream aFStream( sInputFile.Copy( 1 , sInputFile.Len() ).GetBuffer() , ios::in );

            if( !aFStream ){
                cerr << "ERROR: - helpex - Can't open the file " << sInputFile.Copy( 1 , sInputFile.Len() ).GetBuffer() << "\n";
                exit(-1);
            }

            vector<ByteString> filelist;
            rtl::OStringBuffer filename;
            sal_Char aChar;
            while( aFStream.get( aChar ) )
            {
                if( aChar == ' ' || aChar == '\n')
                    filelist.push_back( ByteString( filename.makeStringAndClear().getStr() ) );
                else
                    filename.append( aChar );
            }
            if( filename.getLength() > 0 )
                filelist.push_back( ByteString ( filename.makeStringAndClear().getStr() ) );

            aFStream.close();
            ByteString sHelpFile(""); // dummy
            MergeDataFile aMergeDataFile( sSDFFile, sHelpFile, sal_False );

            //aMergeDataFile.Dump();
            std::vector<ByteString> aLanguages;
            HelpParser::parse_languages( aLanguages , aMergeDataFile );

            bool bCreateDir = true;
            for( vector<ByteString>::iterator pos = filelist.begin() ; pos != filelist.end() ; ++pos )
            {
                sHelpFile = *pos;
                cout << ".";cout.flush();

                HelpParser aParser( sHelpFile , bUTF8 , true );
                hasNoError = aParser.Merge( sSDFFile , sOutputFileX , sOutputFileY , true , aLanguages , aMergeDataFile , bCreateDir );
                bCreateDir = false;
            }
        }
    } else
        cerr << "helpex ERROR: Wrong input parameters!\n";

    //Export::stopMessure( ByteString("full cycle") , startfull );
    if( hasNoError )
        return 0;
    else
        return 1;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
