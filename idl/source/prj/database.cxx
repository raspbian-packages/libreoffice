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


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <tools/fsys.hxx>
#include <tools/debug.hxx>
#include <database.hxx>
#include <globals.hxx>
#include <rtl/strbuf.hxx>

SvIdlDataBase::SvIdlDataBase( const SvCommand& rCmd )
    : bExport( sal_False )
    , nUniqueId( 0 )
    , nVerbosity( rCmd.nVerbosity )
    , bIsModified( sal_False )
    , aPersStream( *IDLAPP->pClassMgr, NULL )
    , pIdTable( NULL )
{
}

SvIdlDataBase::~SvIdlDataBase()
{
    for ( size_t i = 0, n = aIdFileList.size(); i < n; ++i )
        delete aIdFileList[ i ];
    aIdFileList.clear();

    delete pIdTable;
}

#define ADD_TYPE( Name, OdlName, ParserChar, CName, BasName, BasPost )            \
    aTypeList.Append( new SvMetaType( SvHash_##Name()->GetName(),   \
                     BasName, OdlName, ParserChar, CName, BasName, BasPost ) );

SvMetaTypeMemberList & SvIdlDataBase::GetTypeList()
{
    if( aTypeList.Count() == 0 )
    { // fill initially
        aTypeList.Append( new SvMetaTypeString() );
        aTypeList.Append( new SvMetaTypevoid() );

        // MI: IDispatch::Invoke can not unsigned
        ADD_TYPE( UINT16,    "long", 'h', "unsigned short", "Long", "&" );
        ADD_TYPE( INT16,     "short", 'h', "short", "Integer", "%" );
        ADD_TYPE( UINT32,    "long", 'l', "unsigned long", "Long", "&" );
        ADD_TYPE( INT32,     "long", 'l', "long", "Long", "&" );
        ADD_TYPE( int,       "int", 'i', "int", "Integer", "%" );
        ADD_TYPE( BOOL,      "boolean", 'b', "unsigned char", "Boolean", "" );
        ADD_TYPE( char,      "char", 'c', "char", "Integer", "%" );
        ADD_TYPE( BYTE,      "char", 'c', "unsigned char", "Integer", "%" );
        ADD_TYPE( float,     "float", 'f', "float", "Single", "!" );
        ADD_TYPE( double,    "double", 'F', "double", "Double", "#" );
        ADD_TYPE( SbxObject, "VARIANT", 'o', "C_Object", "Object", "" );

        // Attention! When adding types all binary data bases get incompatible

    }
    return aTypeList;
}

SvMetaModule * SvIdlDataBase::GetModule( const rtl::OString& rName )
{
    for( sal_uLong n = 0; n < aModuleList.Count(); n++ )
        if( aModuleList.GetObject( n )->GetName().Equals(rName) )
            return aModuleList.GetObject( n );
    return NULL;
}

#define DATABASE_SIGNATURE  (sal_uInt32)0x13B799F2
#define DATABASE_VER 0x0006
sal_Bool SvIdlDataBase::IsBinaryFormat( SvStream & rStm )
{
    sal_uInt32  nSig = 0;
    sal_uLong   nPos = rStm.Tell();
    rStm >> nSig;
    rStm.Seek( nPos );

    return nSig == DATABASE_SIGNATURE;
}

void SvIdlDataBase::Load( SvStream & rStm )
{
    DBG_ASSERT( aTypeList.Count() == 0, "type list already initialized" );
    SvPersistStream aPStm( *IDLAPP->pClassMgr, &rStm );

    sal_uInt16  nVersion = 0;
    sal_uInt32  nSig = 0;

    aPStm >> nSig;
    aPStm >> nVersion;
    if( nSig != DATABASE_SIGNATURE )
    {
        aPStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        return;
    }
    if( nVersion != DATABASE_VER )
    {
        aPStm.SetError( SVSTREAM_WRONGVERSION );
        return;
    }
    aPStm >> aClassList;
    aPStm >> aTypeList;
    aPStm >> aAttrList;
    aPStm >> aModuleList;
    aPStm >> nUniqueId;

    if( aPStm.IsEof() )
        aPStm.SetError( SVSTREAM_GENERALERROR );
}

void SvIdlDataBase::Save( SvStream & rStm, sal_uInt32 nFlags )
{
    SvPersistStream aPStm( *IDLAPP->pClassMgr, &rStm );
    aPStm.SetContextFlags( nFlags );

    aPStm << (sal_uInt32)DATABASE_SIGNATURE;
    aPStm << (sal_uInt16)DATABASE_VER;

    sal_Bool bOnlyStreamedObjs = sal_False;
    if( nFlags & IDL_WRITE_CALLING )
        bOnlyStreamedObjs = sal_True;

    if( bOnlyStreamedObjs )
    {
        SvMetaClassMemberList aList;
        for( sal_uLong n = 0; n < GetModuleList().Count(); n++ )
        {
            SvMetaModule * pModule = GetModuleList().GetObject( n );
            if( !pModule->IsImported() )
                aList.Append( pModule->GetClassList() );
        }
        aPStm << aList;
    }
    else
        aPStm << aClassList;

    aTypeList.WriteObjects( aPStm, bOnlyStreamedObjs );
    aAttrList.WriteObjects( aPStm, bOnlyStreamedObjs );
    aModuleList.WriteObjects( aPStm, bOnlyStreamedObjs );
    aPStm << nUniqueId;
}

void SvIdlDataBase::SetError( const rtl::OString& rError, SvToken * pTok )
{
    if( pTok->GetLine() > 10000 )
        aError.SetText( "hgchcg" );

    if( aError.nLine < pTok->GetLine()
      || (aError.nLine == pTok->GetLine() && aError.nColumn < pTok->GetColumn()) )
    {
        aError = SvIdlError( pTok->GetLine(), pTok->GetColumn() );
        aError.SetText( rError );
    }
}

void SvIdlDataBase::Push( SvMetaObject * pObj )
{
    GetStack().Push( pObj );
}

#ifdef IDL_COMPILER
sal_Bool SvIdlDataBase::FindId( const rtl::OString& rIdName, sal_uLong * pVal )
{
    if( pIdTable )
    {
        sal_uInt32 nHash;
        if( pIdTable->Test( rIdName, &nHash ) )
        {
            *pVal = pIdTable->Get( nHash )->GetValue();
            return sal_True;
        }
    }
    return sal_False;
}

sal_Bool SvIdlDataBase::InsertId( const rtl::OString& rIdName, sal_uLong nVal )
{
    if( !pIdTable )
        pIdTable = new SvStringHashTable( 20003 );

    sal_uInt32 nHash;
    if( pIdTable->Insert( rIdName, &nHash ) )
    {
        pIdTable->Get( nHash )->SetValue( nVal );
        return sal_True;
    }
    return sal_False;
}

sal_Bool SvIdlDataBase::ReadIdFile( const String & rFileName )
{
    DirEntry aFullName( rFileName );
    aFullName.Find( GetPath() );

    for ( size_t i = 0, n = aIdFileList.size(); i < n; ++i )
        if ( *aIdFileList[ i ] == rFileName )
            return sal_True;

    aIdFileList.push_back( new String( rFileName ) );

    this->AddDepFile(aFullName.GetFull());
    SvTokenStream aTokStm( aFullName.GetFull() );
    if( aTokStm.GetStream().GetError() == SVSTREAM_OK )
    {
        SvToken * pTok = aTokStm.GetToken_Next();

        while( !pTok->IsEof() )
        {
            if( pTok->IsChar() && pTok->GetChar() == '#' )
            {
                pTok = aTokStm.GetToken_Next();
                if( pTok->Is( SvHash_define() ) )
                {
                    pTok = aTokStm.GetToken_Next();
                    ByteString aDefName;
                    if( pTok->IsIdentifier() )
                        aDefName = pTok->GetString();
                    else
                    {
                        ByteString aStr( "unexpected token after define" );
                        // set error
                        SetError( aStr, pTok );
                        WriteError( aTokStm );
                        return sal_False;
                    }

                    sal_uLong nVal = 0;
                    sal_Bool bOk = sal_True;
                    while( bOk )
                    {
                        pTok = aTokStm.GetToken_Next();
                        if( pTok->IsIdentifier() )
                        {
                            sal_uLong n;
                            if( FindId( pTok->GetString(), &n ) )
                                nVal += n;
                            else
                                bOk = sal_False;
                        }
                        else if( pTok->IsChar() )
                        {
                            if( pTok->GetChar() == '-'
                              || pTok->GetChar() == '/'
                              || pTok->GetChar() == '*'
                              || pTok->GetChar() == '&'
                              || pTok->GetChar() == '|'
                              || pTok->GetChar() == '^'
                              || pTok->GetChar() == '~' )
                            {
                                rtl::OStringBuffer aStr("unknown operator '");
                                aStr.append(pTok->GetChar());
                                aStr.append("'in define");
                                // set error
                                SetError( aStr.makeStringAndClear(), pTok );
                                WriteError( aTokStm );
                                return sal_False;
                            }
                            if( pTok->GetChar() != '+'
                              && pTok->GetChar() != '('
                              && pTok->GetChar() != ')' )
                                // only + is allowed, parentheses are immaterial
                                // because + is commutative
                                break;
                        }
                        else if( pTok->IsInteger() )
                        {
                            nVal += pTok->GetNumber();
                        }
                        else
                            break;
                    }
                    if( bOk )
                    {
                        if( !InsertId( aDefName, nVal ) )
                        {
                            ByteString aStr = "hash table overflow: ";
                            SetError( aStr, pTok );
                            WriteError( aTokStm );
                            return sal_False;
                        }
                    }
                }
                else if( pTok->Is( SvHash_include() ) )
                {
                    pTok = aTokStm.GetToken_Next();
                    rtl::OStringBuffer aName;
                    if( pTok->IsString() )
                        aName.append(pTok->GetString());
                    else if( pTok->IsChar() && pTok->GetChar() == '<' )
                    {
                        pTok = aTokStm.GetToken_Next();
                        while( !pTok->IsEof()
                          && !(pTok->IsChar() && pTok->GetChar() == '>') )
                        {
                            aName.append(pTok->GetTokenAsString());
                            pTok = aTokStm.GetToken_Next();
                        }
                        if( pTok->IsEof() )
                        {
                            rtl::OString aStr(RTL_CONSTASCII_STRINGPARAM(
                                "unexpected eof in #include"));
                            // set error
                            SetError(aStr, pTok);
                            WriteError( aTokStm );
                            return sal_False;
                        }
                    }
                    if (!ReadIdFile(rtl::OStringToOUString(aName.toString(),
                        RTL_TEXTENCODING_ASCII_US)))
                    {
                        rtl::OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
                            "cannot read file: "));
                        aStr.append(aName.makeStringAndClear());
                        SetError(aStr.makeStringAndClear(), pTok);
                        WriteError( aTokStm );
                        return sal_False;
                    }
                }
            }
            else
                pTok = aTokStm.GetToken_Next();
        }
    }
    else
        return sal_False;
    return sal_True;
}

SvMetaType * SvIdlDataBase::FindType( const SvMetaType * pPType,
                                    SvMetaTypeMemberList & rList )
{
    SvMetaType * pType = rList.First();
    while( pType && pPType != pType )
        pType = rList.Next();
    return pType;
}

SvMetaType * SvIdlDataBase::FindType( const rtl::OString& rName )
{
    SvMetaType * pType = aTypeList.First();
    while( pType && !rName.equals(pType->GetName()) )
        pType = aTypeList.Next();
    return pType;
}

SvMetaType * SvIdlDataBase::ReadKnownType( SvTokenStream & rInStm )
{
    sal_Bool bIn    = sal_False;
    sal_Bool bOut   = sal_False;
    int nCall0  = CALL_VALUE;
    int nCall1  = CALL_VALUE;
    sal_Bool bSet   = sal_False; // any attribute set

    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->HasHash() )
    {
        sal_uInt32 nBeginPos = 0; // can not happen with Tell
        while( nBeginPos != rInStm.Tell() )
        {
            nBeginPos = rInStm.Tell();
            if( pTok->Is( SvHash_in() ) )
            {
                bIn  = sal_True;
                pTok = rInStm.GetToken_Next();
                bSet = sal_True;
            }
            if( pTok->Is( SvHash_out() ) )
            {
                bOut = sal_True;
                pTok = rInStm.GetToken_Next();
                bSet = sal_True;
            }
            if( pTok->Is( SvHash_inout() ) )
            {
                bIn  = sal_True;
                bOut = sal_True;
                pTok = rInStm.GetToken_Next();
                bSet = sal_True;
            }
        }
    }

    if( pTok->IsIdentifier() )
    {
        ByteString aName = pTok->GetString();
        SvMetaTypeMemberList & rList = GetTypeList();
        SvMetaType * pType = rList.First();
        while( pType )
        {
            if( pType->GetName() == aName )
                break;
            pType = rList.Next();
        }
        if( pType )
        {
            pTok = rInStm.GetToken();
            if( pTok->IsChar() )
            {
                if( pTok->GetChar() == '&' || pTok->GetChar() == '*' )
                {
                    nCall0 = (pTok->GetChar() == '&') ? CALL_REFERENCE :
                                                        CALL_POINTER;
                    rInStm.GetToken_Next();
                    pTok = rInStm.GetToken();
                    if( pTok->GetChar() == '&' || pTok->GetChar() == '*' )
                    {
                        nCall1 = (pTok->GetChar() == '&') ? CALL_REFERENCE :
                                                            CALL_POINTER;
                        rInStm.GetToken_Next();
                    }
                    bSet = sal_True;
                }
            }

            if( !bSet )
                // is exactly this type
                return pType;

            DBG_ASSERT( aTmpTypeList.First(), "mindestens ein Element" );
            SvMetaTypeRef xType = new SvMetaType( pType->GetName(), 'h', "dummy" );
            xType->SetRef( pType );
            xType->SetIn( bIn );
            xType->SetOut( bOut );
            xType->SetCall0( nCall0 );
            xType->SetCall1( nCall1 );

            aTmpTypeList.Append( xType );
            return xType;
        }
    }
    rInStm.Seek( nTokPos );
    return NULL;
}

SvMetaAttribute * SvIdlDataBase::ReadKnownAttr
(
    SvTokenStream & rInStm,
    SvMetaType *    pType   /* If pType == NULL, then the type has
                               still to be read. */
)
{
    sal_uInt32  nTokPos = rInStm.Tell();

    if( !pType )
        pType = ReadKnownType( rInStm );

    if( !pType )
    {
        // otherwise SlotId?
        SvToken * pTok = rInStm.GetToken_Next();
        if( pTok->IsIdentifier() )
        {
            sal_uLong n;
            if( FindId( pTok->GetString(), &n ) )
            {
                for( sal_uLong i = 0; i < aAttrList.Count(); i++ )
                {
                    SvMetaAttribute * pAttr = aAttrList.GetObject( i );
                    if( pAttr->GetSlotId() == pTok->GetString() )
                        return pAttr;
                }
            }

            rtl::OStringBuffer aStr("Nicht gefunden : ");
            aStr.append(pTok->GetString());
            OSL_FAIL(aStr.getStr());
        }
    }

    rInStm.Seek( nTokPos );
    return NULL;
}

SvMetaAttribute* SvIdlDataBase::SearchKnownAttr
(
    const SvNumberIdentifier& rId
)
{
    sal_uLong n;
    if( FindId( rId, &n ) )
    {
        for( sal_uLong i = 0; i < aAttrList.Count(); i++ )
        {
            SvMetaAttribute * pAttr = aAttrList.GetObject( i );
            if( pAttr->GetSlotId() == rId )
                return pAttr;
        }
    }

    return NULL;
}

SvMetaClass * SvIdlDataBase::ReadKnownClass( SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->IsIdentifier() )
        for( sal_uLong n = 0; n < aClassList.Count(); n++ )
        {
            SvMetaClass * pClass = aClassList.GetObject( n );
            if( pClass->GetName() == pTok->GetString() )
                return pClass;
        }

    rInStm.Seek( nTokPos );
    return NULL;
}

void SvIdlDataBase::Write(const rtl::OString& rText)
{
    if( nVerbosity != 0 )
        fprintf( stdout, "%s", rText.getStr() );
}

void SvIdlDataBase::WriteError( const rtl::OString& rErrWrn,
                                const rtl::OString& rFileName,
                                const rtl::OString& rErrorText,
                                sal_uLong nRow, sal_uLong nColumn ) const
{
    // error treatment
    fprintf( stderr, "\n%s --- %s: ( %ld, %ld )\n",
             rFileName.getStr(), rErrWrn.getStr(), nRow, nColumn );

    if( !rErrorText.isEmpty() )
    { // error set
        fprintf( stderr, "\t%s\n", rErrorText.getStr() );
    }
}

void SvIdlDataBase::WriteError( SvTokenStream & rInStm )
{
    // error treatment
    String aFileName( rInStm.GetFileName() );
    rtl::OStringBuffer aErrorText;
    sal_uLong   nRow = 0, nColumn = 0;

    rInStm.SeekEnd();
    SvToken *pTok = rInStm.GetToken();

    // error position
    nRow    = pTok->GetLine();
    nColumn = pTok->GetColumn();

    if( aError.IsError() )
    { // error set
        // search error token
        // error text
        if( aError.GetText().getLength() )
        {
            aErrorText.append(RTL_CONSTASCII_STRINGPARAM("may be <"));
            aErrorText.append(aError.GetText());
        }
        SvToken * pPrevTok = NULL;
        while( pTok != pPrevTok )
        {
            pPrevTok = pTok;
            if( pTok->GetLine() == aError.nLine
              && pTok->GetColumn() == aError.nColumn )
                break;
            pTok = rInStm.GetToken_PrevAll();
        }

        // error position
        aErrorText.append(RTL_CONSTASCII_STRINGPARAM("> at ( "));
        aErrorText.append(static_cast<sal_Int64>(aError.nLine));
        aErrorText.append(RTL_CONSTASCII_STRINGPARAM(", "));
        aErrorText.append(static_cast<sal_Int64>(aError.nColumn));
        aErrorText.append(RTL_CONSTASCII_STRINGPARAM(" )"));

        // reset error
        aError = SvIdlError();
    }

    WriteError("error", rtl::OUStringToOString(aFileName,
        RTL_TEXTENCODING_UTF8), aErrorText.makeStringAndClear(), nRow, nColumn);

    DBG_ASSERT( pTok, "token must be found" );
    if( !pTok )
        return;

    // look for identifier close by
    if( !pTok->IsIdentifier() )
    {
        rInStm.GetToken_PrevAll();
        pTok = rInStm.GetToken();
    }
    if( pTok && pTok->IsIdentifier() )
    {
        ByteString aN = IDLAPP->pHashTable->GetNearString( pTok->GetString() );
        if( aN.Len() )
            fprintf( stderr, "%s versus %s\n", pTok->GetString().GetBuffer(), aN.GetBuffer() );
    }
}

SvIdlWorkingBase::SvIdlWorkingBase(const SvCommand& rCmd) : SvIdlDataBase(rCmd)
{
}

sal_Bool SvIdlWorkingBase::ReadSvIdl( SvTokenStream & rInStm, sal_Bool bImported, const String & rPath )
{
    aPath = rPath; // only valid for this iteration
    SvToken * pTok;
    sal_Bool bOk = sal_True;
        pTok = rInStm.GetToken();
        // only one import at the very beginning
        if( pTok->Is( SvHash_import() ) )
        {
            rInStm.GetToken_Next();
            rInStm.Read( '(' ); // optional
            pTok = rInStm.GetToken_Next();
            if( pTok->IsString() )
            {
                DirEntry aFullName( String::CreateFromAscii( pTok->GetString().GetBuffer() ) );
                if( aFullName.Find( rPath ) )
                {
                    this->AddDepFile(aFullName.GetFull());
                    SvFileStream aStm( aFullName.GetFull(),
                                        STREAM_STD_READ | STREAM_NOCREATE );
                    Load( aStm );
                    if( aStm.GetError() != SVSTREAM_OK )
                    {
                        if( aStm.GetError() == SVSTREAM_WRONGVERSION )
                        {
                            rtl::OStringBuffer aStr("wrong version, file ");
                            aStr.append(rtl::OUStringToOString(
                                aFullName.GetFull(), RTL_TEXTENCODING_UTF8));
                            SetError(aStr.makeStringAndClear(), pTok);
                            WriteError( rInStm );
                            bOk = sal_False;
                        }
                        else
                        {
                            aStm.Seek( 0 );
                            aStm.ResetError();
                            SvTokenStream aTokStm( aStm, aFullName.GetFull() );
                            bOk = ReadSvIdl( aTokStm, sal_True, rPath );
                        }
                    }
                }
                else
                    bOk = sal_False;
            }
            else
                bOk = sal_False;
        }

    sal_uInt32 nBeginPos = 0xFFFFFFFF; // can not happen with Tell

    while( bOk && nBeginPos != rInStm.Tell() )
    {
        nBeginPos = rInStm.Tell();
        pTok = rInStm.GetToken();
        if( pTok->IsEof() )
            return sal_True;
        if( pTok->IsEmpty() )
            bOk = sal_False;

        // only one import at the very beginning
        if( pTok->Is( SvHash_module() ) )
        {
            SvMetaModuleRef aModule = new SvMetaModule( rInStm.GetFileName(), bImported );
            if( aModule->ReadSvIdl( *this, rInStm ) )
                GetModuleList().Append( aModule );
            else
                bOk = sal_False;
        }
        else
            bOk = sal_False;
    }
    if( !bOk || !pTok->IsEof() )
    {
         // error treatment
         WriteError( rInStm );
         return sal_False;
    }
    return sal_True;
}

sal_Bool SvIdlWorkingBase::WriteSvIdl( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return sal_False;

    SvStringHashList aList;
    if( GetIdTable() )
    {
        GetIdTable()->FillHashList( &aList );
        for ( size_t i = 0, n = aList.size(); i < n; ++i )
        {
            SvStringHashEntry* pEntry = aList[ i ];
            rOutStm << "#define " << pEntry->GetName().GetBuffer()
                    << '\t'
                    << rtl::OString::valueOf(static_cast<sal_Int64>(
                        pEntry->GetValue())).getStr()
                    << endl;
        }
    }

    for( sal_uLong n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        pModule->WriteSvIdl( *this, rOutStm, 0 );
    }
    return sal_True;
}

sal_Bool SvIdlWorkingBase::WriteSfx( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return sal_False;

    // reset all tmp variables for writing
    WriteReset();
    SvMemoryStream aTmpStm( 256000, 256000 );
    sal_uLong n;
    for( n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        if( !pModule->IsImported() )
            pModule->WriteSfx( *this, aTmpStm );
        aTmpStm.Seek( 0 );
    }
    for( n = 0; n < aUsedTypes.Count(); n++ )
    {
        SvMetaType * pType = aUsedTypes.GetObject( n );
        pType->WriteSfx( *this, rOutStm );
    }
    aUsedTypes.Clear();
    rOutStm << aTmpStm;
    return sal_True;
}

sal_Bool SvIdlWorkingBase::WriteHelpIds( SvStream& rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return sal_False;

    Table aIdTable;
    sal_uLong n;
    for( n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        pModule->WriteHelpIds( *this, rOutStm, &aIdTable );
    }

    const SvMetaAttributeMemberList & rAttrList = GetAttrList();
    for( n = 0; n < rAttrList.Count(); n++ )
    {
        SvMetaAttribute * pAttr = rAttrList.GetObject( n );
        pAttr->WriteHelpId( *this, rOutStm, &aIdTable );
    }

    return sal_True;
}

sal_Bool SvIdlWorkingBase::WriteSfxItem( SvStream & )
{
    return sal_False;
}

void SvIdlDataBase::StartNewFile( const String& rName )
{
    bExport = ( aExportFile.EqualsIgnoreCaseAscii( rName ) );
}

void SvIdlDataBase::AppendAttr( SvMetaAttribute *pAttr )
{
    aAttrList.Append( pAttr );
    if ( bExport )
        pAttr->SetNewAttribute( sal_True );
}

sal_Bool SvIdlWorkingBase::WriteCSV( SvStream& rStrm )
{
    SvMetaAttributeMemberList &rList = GetAttrList();
    sal_uLong nCount = rList.Count();
    for ( sal_uLong n=0; n<nCount; n++ )
    {
        if ( rList.GetObject(n)->IsNewAttribute() )
        {
            rList.GetObject(n)->WriteCSV( *this, rStrm );
        }
    }

    if ( rStrm.GetError() != SVSTREAM_OK )
        return sal_False;
    else
        return sal_True;
}

sal_Bool SvIdlWorkingBase::WriteDocumentation( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return sal_False;

    for( sal_uLong n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        if( !pModule->IsImported() )
            pModule->Write( *this, rOutStm, 0, WRITE_DOCU );
    }
    return sal_True;
}

void SvIdlDataBase::AddDepFile(String const& rFileName)
{
    m_DepFiles.insert(rFileName);
}

#ifdef WNT
static ::rtl::OString
lcl_ConvertToCygwin(::rtl::OString const& rString)
{
    sal_Int32 i = 0;
    sal_Int32 const len = rString.getLength();
    ::rtl::OStringBuffer buf(len + 16);
    if ((2 <= len) && (':' == rString[1]))
    {
        buf.append("/cygdrive/");
        buf.append(static_cast<sal_Char>(tolower(rString[0])));
        i = 2;
    }
    for (; i < len; ++i)
    {
        sal_Char const c(rString[i]);
        switch (c)
        {
            case '\\':
                buf.append('/');
            break;
            case ' ':
                buf.append("\\ ");
            break;
            default:
                buf.append(c);
            break;
        }
    }
    return buf.makeStringAndClear();
}
#endif

static ::rtl::OString
lcl_Convert(::rtl::OUString const& rString)
{
    return
#ifdef WNT
        lcl_ConvertToCygwin
#endif
            (::rtl::OUStringToOString(rString, RTL_TEXTENCODING_UTF8));
}

struct WriteDep
{
    SvFileStream & m_rStream;
    explicit WriteDep(SvFileStream & rStream) : m_rStream(rStream) { }
    void operator() (::rtl::OUString const& rItem)
    {
        m_rStream << " \\\n ";
        m_rStream << lcl_Convert(rItem).getStr();
    }
};

bool SvIdlDataBase::WriteDepFile(
        SvFileStream & rStream, ::rtl::OUString const& rTarget)
{
    rStream << lcl_Convert(rTarget).getStr();
    rStream << " :";
    ::std::for_each(m_DepFiles.begin(), m_DepFiles.end(), WriteDep(rStream));
    return rStream.GetError() == SVSTREAM_OK;
}

#endif // IDL_COMPILER

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
