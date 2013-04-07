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
#ifndef _STREAM_HXX
#define _STREAM_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>
#include <tools/string.hxx>
#include <tools/errinf.hxx>
#include <tools/ref.hxx>
#include <tools/rtti.hxx>
#include <rtl/string.hxx>

class FileCopier;
class StreamData;

// ------------------------
// - FileFormat-Functions -
// ------------------------

inline rtl_TextEncoding GetStoreCharSet( rtl_TextEncoding eEncoding )
{
    if ( eEncoding == RTL_TEXTENCODING_ISO_8859_1 )
        return RTL_TEXTENCODING_MS_1252;
    else
        return eEncoding;
}

// ---------------
// - StreamTypes -
// ---------------

typedef sal_uInt16 StreamMode;

// read, write, create,... options
#define STREAM_READ                     0x0001  // allow read accesses
#define STREAM_WRITE                    0x0002  // allow write accesses
// file i/o
#define STREAM_NOCREATE                 0x0004  // 1 == Dont create file
#define STREAM_TRUNC                    0x0008  // Truncate _existing_ file to zero length
#define STREAM_COPY_ON_SYMLINK          0x0010  // copy-on-write for symlinks (UNX)

#define STREAM_READWRITEBITS            (STREAM_READ | STREAM_WRITE | \
                                         STREAM_NOCREATE | STREAM_TRUNC)

// sharing options
#define STREAM_SHARE_DENYNONE           0x0100
#define STREAM_SHARE_DENYREAD           0x0200  // overrides denynone
#define STREAM_SHARE_DENYWRITE      0x0400  // overrides denynone
#define STREAM_SHARE_DENYALL            0x0800  // overrides denyread,write,none

#define STREAM_SHAREBITS                (STREAM_SHARE_DENYNONE | STREAM_SHARE_DENYREAD |\
                                         STREAM_SHARE_DENYWRITE | STREAM_SHARE_DENYALL)

#define STREAM_READWRITE                (STREAM_READ | STREAM_WRITE)
#define STREAM_SHARE_DENYREADWRITE      (STREAM_SHARE_DENYREAD | STREAM_SHARE_DENYWRITE)

#define STREAM_STD_READ                 (STREAM_READ | STREAM_SHARE_DENYNONE | STREAM_NOCREATE)
#define STREAM_STD_WRITE                (STREAM_WRITE | STREAM_SHARE_DENYALL)
#define STREAM_STD_READWRITE            (STREAM_READWRITE | STREAM_SHARE_DENYALL)

#define STREAM_SEEK_TO_BEGIN            0L
#define STREAM_SEEK_TO_END              ULONG_MAX

#define NUMBERFORMAT_INT_BIGENDIAN      (sal_uInt16)0x0000
#define NUMBERFORMAT_INT_LITTLEENDIAN   (sal_uInt16)0xFFFF

#define COMPRESSMODE_FULL           (sal_uInt16)0xFFFF
#define COMPRESSMODE_NONE           (sal_uInt16)0x0000
#define COMPRESSMODE_ZBITMAP            (sal_uInt16)0x0001
#define COMPRESSMODE_NATIVE             (sal_uInt16)0x0010

#define STREAM_IO_DONTKNOW          0
#define STREAM_IO_READ              1
#define STREAM_IO_WRITE                 2

#define ID_STREAM                       1
#define ID_FILESTREAM                   2
#define ID_MEMORYSTREAM                 3
#define ID_SHAREDMEMORYSTREAM           4
#define ID_STORAGESTREAM                5
#define ID_PERSISTSTREAM                6

class SvStream;
typedef SvStream& (*SvStrPtr)( SvStream& );

// forward declaration with internal linkage
inline SvStream& operator<<( SvStream& rStr, SvStrPtr f );

// ---------------
// - SvLockBytes -
// ---------------

enum LockType {};

struct SvLockBytesStat
{
    sal_Size nSize;

    SvLockBytesStat(): nSize(0) {}
};

enum SvLockBytesStatFlag { SVSTATFLAG_DEFAULT };

class TOOLS_DLLPUBLIC SvLockBytes: public virtual SvRefBase
{
    SvStream * m_pStream;
    sal_Bool m_bOwner;
    sal_Bool m_bSync;

protected:
    void close();

public:
    TYPEINFO();

    SvLockBytes(): m_pStream(0), m_bOwner(sal_False), m_bSync(sal_False) {}

    SvLockBytes(SvStream * pTheStream, sal_Bool bTheOwner = sal_False):
        m_pStream(pTheStream), m_bOwner(bTheOwner), m_bSync(sal_False) {}

    virtual ~SvLockBytes() { close(); }

    virtual const SvStream * GetStream() const { return m_pStream; }

    virtual void SetSynchronMode(sal_Bool bTheSync = sal_True) { m_bSync = bTheSync; }

    virtual sal_Bool IsSynchronMode() const { return m_bSync; }

    virtual ErrCode ReadAt(sal_Size nPos, void * pBuffer, sal_Size nCount,
                           sal_Size * pRead) const;

    virtual ErrCode WriteAt(sal_Size nPos, const void * pBuffer, sal_Size nCount,
                            sal_Size * pWritten);

    virtual ErrCode Flush() const;

    virtual ErrCode SetSize(sal_Size nSize);

    virtual ErrCode LockRegion(sal_Size, sal_Size, LockType);

    virtual ErrCode UnlockRegion(sal_Size, sal_Size, LockType);

    virtual ErrCode Stat(SvLockBytesStat * pStat, SvLockBytesStatFlag) const;
};

SV_DECL_IMPL_REF(SvLockBytes);

// -------------------
// - SvOpenLockBytes -
// -------------------

class TOOLS_DLLPUBLIC SvOpenLockBytes: public SvLockBytes
{
public:
    TYPEINFO();

    SvOpenLockBytes(): SvLockBytes(0, sal_False) {}

    SvOpenLockBytes(SvStream * pStream, sal_Bool bOwner):
        SvLockBytes(pStream, bOwner) {}

    virtual ErrCode FillAppend(const void * pBuffer, sal_Size nCount,
                               sal_Size * pWritten) = 0;

    virtual sal_Size Tell() const = 0;

    virtual sal_Size Seek(sal_Size nPos) = 0;

    virtual void Terminate() = 0;
};

SV_DECL_IMPL_REF(SvOpenLockBytes);

// --------------------
// - SvAsyncLockBytes -
// --------------------

class SvAsyncLockBytes: public SvOpenLockBytes
{
    sal_Size m_nSize;
    sal_Bool m_bTerminated;

public:
    TYPEINFO();

    SvAsyncLockBytes(SvStream * pStream, sal_Bool bOwner):
        SvOpenLockBytes(pStream, bOwner), m_nSize(0), m_bTerminated(sal_False) {}

    virtual ErrCode ReadAt(sal_Size nPos, void * pBuffer, sal_Size nCount,
                           sal_Size * pRead) const;

    virtual ErrCode WriteAt(sal_Size nPos, const void * pBuffer, sal_Size nCount,
                            sal_Size * pWritten);

    virtual ErrCode FillAppend(const void * pBuffer, sal_Size nCount,
                               sal_Size * pWritten);

    virtual sal_Size Tell() const { return m_nSize; }

    virtual sal_Size Seek(sal_Size nPos);

    virtual void Terminate() { m_bTerminated = sal_True; }
};

SV_DECL_IMPL_REF(SvAsyncLockBytes);

// ----------
// - Stream -
// ----------

class TOOLS_DLLPUBLIC SvStream
{
private:
    // LockBytes Interface
    void*       pImp;           // unused
    SvLockBytesRef  xLockBytes;     // Default Implmentierung
    sal_Size        nActPos;        //

    // Puffer-Verwaltung
    sal_uInt8*  pRWBuf;         // Zeigt auf Read/Write-Puffer
    sal_uInt8*  pBufPos;        // pRWBuf + nBufActualPos
    sal_uInt16  nBufSize;       // Allozierte Groesse des Puffers
    sal_uInt16  nBufActualLen;  // Laenge des beschriebenen Teils des Puffers
                                    // Entspricht nBufSize, wenn EOF nicht
                                    // ueberschritten wurde
    sal_uInt16  nBufActualPos;  // aktuelle Position im Puffer (0..nBufSize-1)
    sal_uInt16  nBufFree;       // freier Platz im Puffer fuer IO vom Typ eIOMode
    unsigned int    eIOMode:2;      // STREAM_IO_*

    // Error-Codes, Konvertierung, Komprimierung, ...
    int             bIsDirty:1;     // sal_True: Stream != Pufferinhalt
    int             bIsConsistent:1;// sal_False: Buffer enthaelt Daten, die NICHT
                                    // per PutData in den abgeleiteten Stream
                                    // geschrieben werden duerfen (siehe PutBack)
    int             bSwap:1;
    int             bIsEof:1;
    sal_uInt32  nError;
    sal_uInt16  nNumberFormatInt;
    sal_uInt16  nCompressMode;
    LineEnd         eLineDelimiter;
    CharSet         eStreamCharSet;

    // Verschluesselung
    rtl::OString m_aCryptMaskKey;           // aCryptMaskKey.getLength != 0  -> Verschluesselung
    unsigned char   nCryptMask;

    // Userdata
    long            nVersion;       // for external use

    // Hilfsmethoden
    TOOLS_DLLPRIVATE void           ImpInit();

                     SvStream ( const SvStream& rStream ); // not implemented
    SvStream&       operator=( const SvStream& rStream ); // not implemented

protected:
    sal_Size            nBufFilePos;    // Fileposition von pBuf[0]
    sal_uInt16          eStreamMode;
    sal_Bool            bIsWritable;

    virtual sal_Size    GetData( void* pData, sal_Size nSize );
    virtual sal_Size    PutData( const void* pData, sal_Size nSize );
    virtual sal_Size    SeekPos( sal_Size nPos );
    virtual void    FlushData();
    virtual void    SetSize( sal_Size nSize );

    void            ClearError();
    void            ClearBuffer();

    // verschluesselt & schreibt blockweise
    sal_Size            CryptAndWriteBuffer( const void* pStart, sal_Size nLen );
    sal_Bool            EncryptBuffer( void* pStart, sal_Size nLen );

    void            SyncSvStream( sal_Size nNewStreamPos ); // SvStream <- Medium
    void            SyncSysStream(); // SvStream -> Medium

public:
                    SvStream();
                    SvStream( SvLockBytes *pLockBytes);
    virtual         ~SvStream();

    SvLockBytes*    GetLockBytes() const { return xLockBytes; }

    sal_uInt32  GetError() const { return ERRCODE_TOERROR(nError); }
    sal_uInt32  GetErrorCode() const { return nError; }

    void            SetError( sal_uInt32 nErrorCode );
    virtual void    ResetError();

    void            SetNumberFormatInt( sal_uInt16 nNewFormat );
    sal_uInt16          GetNumberFormatInt() const { return nNumberFormatInt; }
                    /// Enable/disable swapping of endians, may be needed for Unicode import/export
    inline void     SetEndianSwap( sal_Bool bVal );
                    // returns status of endian swap flag
    sal_Bool            IsEndianSwap() const { return 0 != bSwap; }

    void            SetCompressMode( sal_uInt16 nNewMode )
                        { nCompressMode = nNewMode; }
    sal_uInt16          GetCompressMode() const { return nCompressMode; }

    void SetCryptMaskKey(const rtl::OString& rCryptMaskKey);
    const rtl::OString& GetCryptMaskKey() const { return m_aCryptMaskKey; }

    void            SetStreamCharSet( CharSet eCharSet )
                        { eStreamCharSet = eCharSet; }
    CharSet         GetStreamCharSet() const { return eStreamCharSet; }

    void            SetLineDelimiter( LineEnd eLineEnd )
                        { eLineDelimiter = eLineEnd; }
    LineEnd         GetLineDelimiter() const { return eLineDelimiter; }

    SvStream&       operator>>( sal_uInt16& rUInt16 );
    SvStream&       operator>>( sal_uInt32& rUInt32 );
    SvStream&       operator>>( sal_uInt64& rUInt64 );
    SvStream&       operator>>( long& rLong );
    SvStream&       operator>>( short& rShort );
    SvStream&       operator>>( int& rInt );
    SvStream&       operator>>( signed char& rChar );
    SvStream&       operator>>( char& rChar );
    SvStream&       operator>>( unsigned char& rChar );
    SvStream&       operator>>( float& rFloat );
    SvStream&       operator>>( double& rDouble );
    SvStream&       operator>>( SvStream& rStream );

    SvStream&       operator<<( sal_uInt16 nUInt16 );
    SvStream&       operator<<( sal_uInt32 nUInt32 );
    SvStream&       operator<<( sal_uInt64 nuInt64 );
    SvStream&       operator<<( long nLong );
    SvStream&       operator<<( short nShort );
    SvStream&       operator<<( int nInt );
    SvStream&       operator<<( signed char nChar );
    SvStream&       operator<<( char nChar );
    SvStream&       operator<<( unsigned char nChar );
    SvStream&       operator<<( float nFloat );
    SvStream&       operator<<( const double& rDouble );
    SvStream&       operator<<( const char* pBuf );
    SvStream&       operator<<( const unsigned char* pBuf );
    SvStream&       operator<<( SvStream& rStream );

    SvStream&       ReadByteString( UniString& rStr, rtl_TextEncoding eSrcCharSet );
    SvStream&       ReadByteString( UniString& rStr ) { return ReadByteString( rStr, GetStreamCharSet() ); }
    SvStream&       ReadByteString( ByteString& rStr );
    SvStream&       WriteByteString( const UniString& rStr, rtl_TextEncoding eDestCharSet );
    SvStream&       WriteByteString( const UniString& rStr ) { return WriteByteString( rStr, GetStreamCharSet() ); }
    SvStream&       WriteByteString( const ByteString& rStr );

    SvStream&       WriteNumber( sal_uInt32 nUInt32 );
    SvStream&       WriteNumber( sal_Int32 nInt32 );

    sal_Size        Read( void* pData, sal_Size nSize );
    sal_Size        Write( const void* pData, sal_Size nSize );
    sal_Size        Seek( sal_Size nPos );
    sal_Size        SeekRel( sal_sSize nPos );
    sal_Size        Tell() const { return nBufFilePos+nBufActualPos;  }
    //length between current (Tell()) pos and end of stream
    virtual sal_Size remainingSize();
    void            Flush();
    sal_Bool        IsEof() const { return bIsEof; }
    // next Tell() <= nSize
    sal_Bool        SetStreamSize( sal_Size nSize );

                /// Read in the stream to a zero character and put all
                /// read chracters in the Bytestring. The String interface
                /// convert the BytString with the given encoding to a String
    sal_Bool        ReadCString( ByteString& rStr );
    sal_Bool        ReadCString( String& rStr, rtl_TextEncoding eToEncode );
    sal_Bool        ReadCString( String& rStr ) { return ReadCString( rStr, GetStreamCharSet()); }

    sal_Bool        ReadLine( ByteString& rStr );
    sal_Bool        ReadLine( rtl::OString& rStr );
    sal_Bool        WriteLine( const ByteString& rStr );
    sal_Bool        WriteLines( const ByteString& rStr );

    sal_Bool        ReadByteStringLine( String& rStr, rtl_TextEncoding eSrcCharSet );
    sal_Bool        ReadByteStringLine( String& rStr ) { return ReadByteStringLine( rStr, GetStreamCharSet()); }
    sal_Bool        WriteByteStringLine( const String& rStr, rtl_TextEncoding eDestCharSet );
    sal_Bool        WriteByteStringLine( const String& rStr ) { return WriteByteStringLine( rStr, GetStreamCharSet()); }

                /// Switch to no endian swapping and write 0xfeff
    sal_Bool        StartWritingUnicodeText();

                /** If eReadBomCharSet==RTL_TEXTENCODING_DONTKNOW: read 16bit,
                    if 0xfeff do nothing (UTF-16), if 0xfffe switch endian
                    swapping (UTF-16), if 0xefbb or 0xbbef read another byte
                    and check for UTF-8. If no UTF-* BOM was detected put all
                    read bytes back. This means that if 2 bytes were read it
                    was an UTF-16 BOM, if 3 bytes were read it was an UTF-8
                    BOM. There is no UTF-7, UTF-32 or UTF-EBCDIC BOM detection!

                    If eReadBomCharSet!=RTL_TEXTENCODING_DONTKNOW: only read a
                    BOM of that encoding and switch endian swapping if UTF-16
                    and 0xfffe.
                 */
    sal_Bool        StartReadingUnicodeText( rtl_TextEncoding eReadBomCharSet );

                /// Read a line of Unicode
    sal_Bool        ReadUniStringLine( String& rStr );
                /// Read a line of Unicode if eSrcCharSet==RTL_TEXTENCODING_UNICODE,
                /// otherwise read a line of Bytecode and convert from eSrcCharSet
    sal_Bool        ReadUniOrByteStringLine( String& rStr, rtl_TextEncoding eSrcCharSet );
    sal_Bool        ReadUniOrByteStringLine( String& rStr )
                    { return ReadUniOrByteStringLine( rStr, GetStreamCharSet() ); }
                /// Write a sequence of Unicode characters
    sal_Bool        WriteUnicodeText( const String& rStr );
                /// Write a sequence of Unicode characters if eDestCharSet==RTL_TEXTENCODING_UNICODE,
                /// otherwise write a sequence of Bytecodes converted to eDestCharSet
    sal_Bool        WriteUnicodeOrByteText( const String& rStr, rtl_TextEncoding eDestCharSet );
    sal_Bool        WriteUnicodeOrByteText( const String& rStr )
                    { return WriteUnicodeOrByteText( rStr, GetStreamCharSet() ); }

                /// Write a Unicode character if eDestCharSet==RTL_TEXTENCODING_UNICODE,
                /// otherwise write as Bytecode converted to eDestCharSet.
                /// This may result in more than one byte being written
                /// if a multi byte encoding (e.g. UTF7, UTF8) is chosen.
    sal_Bool        WriteUniOrByteChar( sal_Unicode ch, rtl_TextEncoding eDestCharSet );
    sal_Bool        WriteUniOrByteChar( sal_Unicode ch )
                    { return WriteUniOrByteChar( ch, GetStreamCharSet() ); }

                /** Read a CSV (comma separated values) data line using
                    ReadUniOrByteStringLine().

                    @param bEmbeddedLineBreak
                    If sal_True and a line-break occurs inside a field of data,
                    a line feed LF '\n' and the next line are appended. Repeats
                    until a line-break is not in a field. A field is determined
                    by delimiting rFieldSeparators and optionally surrounded by
                    a pair of cFieldQuote characters. For a line-break to be
                    within a field, the field content MUST be surrounded by
                    cFieldQuote characters, and the opening cFieldQuote MUST be
                    at the very start of a line or follow right behind a field
                    separator with no extra characters in between. Anything,
                    including field separators and escaped quotes (by doubling
                    them, or preceding them with a backslash if
                    bAllowBackslashEscape==sal_True) may appear in a quoted
                    field.

                    If bEmbeddedLineBreak==sal_False, nothing is parsed and the
                    string returned is simply one ReadUniOrByteStringLine().

                    @param rFieldSeparators
                    A list of characters that each may act as a field separator.

                    @param cFieldQuote
                    The quote character used.

                    @param bAllowBackslashEscape
                    If sal_True, an embedded quote character inside a quoted
                    field may also be escaped with a preceding backslash.
                    Normally, quotes are escaped by doubling them.

                    @return
                    sal_True if no stream error.

                    @ATTENTION
                    Note that the string returned may be truncated even inside
                    a quoted field if STRING_MAXLEN was reached. There
                    currently is no way to exactly determine the conditions,
                    whether this was at a line end, or whether open quotes
                    would have closed the field before the line end, as even a
                    ReadUniOrByteStringLine() may return prematurely but the
                    stream was positioned ahead until the real end of line.
                    Additionally, due to character encoding conversions, string
                    length and bytes read don't necessarily match, and
                    resyncing to a previous position matching the string's
                    length isn't always possible. As a result, a logical line
                    with embedded line breaks and more than STRING_MAXLEN
                    characters will be spoiled, and a subsequent ReadCsvLine()
                    may start under false preconditions.
                  */
    sal_Bool        ReadCsvLine( String& rStr, sal_Bool bEmbeddedLineBreak,
                        const String& rFieldSeparators, sal_Unicode cFieldQuote,
                        sal_Bool bAllowBackslashEscape = sal_False);

    void            SetBufferSize( sal_uInt16 nBufSize );
    sal_uInt16  GetBufferSize() const { return nBufSize; }

    void            RefreshBuffer();
    SvStream&       PutBack( char aCh );

    sal_Bool            IsWritable() const { return bIsWritable; }
    StreamMode      GetStreamMode() const { return eStreamMode; }
    virtual sal_uInt16  IsA() const;

    long            GetVersion() { return nVersion; }
    void            SetVersion( long n ) { nVersion = n; }

    friend SvStream& operator<<( SvStream& rStr, SvStrPtr f ); // fuer Manips

    //end of input seen during previous i/o operation
    bool eof() const { return bIsEof; }

    // stream is broken
    bool bad() const { return GetError() != 0; }

    //If the state is good() the previous i/o operation succeeded.
    //
    //If the state is good(), the next input operation might succeed;
    //otherwise, it will fail.
    //
    //Applying an input operation to a stream that is not in the good() state
    //is a null operation as far as the variable being read into is concerned.
    //
    //If we try to read into a variable v and the operation fails, the value of
    //v should be unchanged,
    bool good() const { return !(eof() || bad()); }
};

inline SvStream& operator<<( SvStream& rStr, SvStrPtr f )
{
    (*f)(rStr);
    return rStr;
}

inline void SvStream::SetEndianSwap( sal_Bool bVal )
{
#ifdef OSL_BIGENDIAN
    SetNumberFormatInt( bVal ? NUMBERFORMAT_INT_LITTLEENDIAN : NUMBERFORMAT_INT_BIGENDIAN );
#else
    SetNumberFormatInt( bVal ? NUMBERFORMAT_INT_BIGENDIAN : NUMBERFORMAT_INT_LITTLEENDIAN );
#endif
}

TOOLS_DLLPUBLIC SvStream& endl( SvStream& rStr );
/// same as endl() but Unicode
TOOLS_DLLPUBLIC SvStream& endlu( SvStream& rStr );
/// call endlu() if eStreamCharSet==RTL_TEXTECODING_UNICODE otherwise endl()
TOOLS_DLLPUBLIC SvStream& endlub( SvStream& rStr );

//Attempt to read nLen 8bit units to an OString, returned rtl::OString's length
//is number of units successfully read
TOOLS_DLLPUBLIC rtl::OString read_uInt8s_AsOString(SvStream& rStr, sal_Size nLen);

//Attempt to read nLen little endian 16bit units to an OUString, returned
//rtl::OUString's length is number of units successfully read
TOOLS_DLLPUBLIC rtl::OUString read_LEuInt16s_AsOUString(SvStream& rStr, sal_Size nLen);

// --------------
// - FileStream -
// --------------

class TOOLS_DLLPUBLIC SvFileStream : public SvStream
{
    friend class ImpEaMgr;
    friend class CORmFileStream;
    friend class FileCopier;

private:
    StreamData*         pInstanceData;
    String          aFilename;
    sal_uInt16      nLockCounter;
    sal_Bool            bIsOpen;
    sal_uInt32      GetFileHandle() const;

    // Forbidden and not implemented.
    SvFileStream (const SvFileStream&);
    SvFileStream & operator= (const SvFileStream&);

protected:

    virtual sal_Size    GetData( void* pData, sal_Size nSize );
    virtual sal_Size    PutData( const void* pData, sal_Size nSize );
    virtual sal_Size    SeekPos( sal_Size nPos );
    virtual void    SetSize( sal_Size nSize );
    virtual void    FlushData();

public:
                    // Schaltet bei fehlgeschlagenem Schreiboeffnen auf Lesen zurueck
                    SvFileStream( const String& rFileName, StreamMode eOpenMode );
                    SvFileStream();
                    ~SvFileStream();

    virtual void    ResetError();

    sal_Bool            LockRange( sal_Size nByteOffset, sal_Size nBytes );
    sal_Bool            UnlockRange( sal_Size nByteOffset, sal_Size nBytes );
    sal_Bool            LockFile();
    sal_Bool            UnlockFile();

    void            Open( const String& rFileName, StreamMode eOpenMode );
    void            Close();
    sal_Bool            IsOpen() const { return bIsOpen; }
    sal_Bool            IsLocked() const { return ( nLockCounter!=0 ); }
    virtual sal_uInt16  IsA() const;

    const String&   GetFileName() const { return aFilename; }
};

// ----------------
// - MemoryStream -
// ----------------

class TOOLS_DLLPUBLIC SvMemoryStream : public SvStream
{
    // Forbidden and not implemented.
    SvMemoryStream (const SvMemoryStream&);
    SvMemoryStream & operator= (const SvMemoryStream&);

    friend class SvCacheStream;
    sal_Size            GetSize() const { return nSize; }

protected:
    sal_Size            nSize;
    sal_Size            nResize;
    sal_Size            nPos;
    sal_Size            nEndOfData;
    sal_uInt8*          pBuf;
    sal_Bool            bOwnsData;

    virtual sal_Size    GetData( void* pData, sal_Size nSize );
    virtual sal_Size    PutData( const void* pData, sal_Size nSize );
    virtual sal_Size    SeekPos( sal_Size nPos );
    virtual void    SetSize( sal_Size nSize );
    virtual void    FlushData();

    // AllocateMemory muss folgende Variable mitpflegen:
    // - pBuf: Adresse des neuen Blocks
    virtual sal_Bool    AllocateMemory( sal_Size nSize );

    // ReAllocateMemory muss folgende Variablen mitpflegen:
    // - pBuf: Adresse des neuen Blocks
    // - nEndOfData: Muss auf nNewSize-1L gesetzt werden, wenn ausserhalb des Blocks
    //               Muss auf 0 gesetzt werden, wenn neuer Block 0 Byte gross
    // - nSize: Neue Groesse des Blocks
    // - nPos: Muss auf 0 gesetzt werden, wenn ausserhalb des Blocks
    virtual sal_Bool    ReAllocateMemory( long nDiff );

    // wird aufgerufen, wenn dem Stream der Speicher gehoert oder wenn
    // der Speicher in der Groesse veraendert wird.
    // FreeMemory muss folgende Variablen mitpflegen:
    // - in abgeleiteten Klassen muessen ggf. Handles genullt werden
    virtual void    FreeMemory();

                    SvMemoryStream(void*) { }   // Fuer unsere Subklassen

public:
                    SvMemoryStream( void* pBuf, sal_Size nSize, StreamMode eMode);
                    SvMemoryStream( sal_Size nInitSize=512, sal_Size nResize=64 );
                    ~SvMemoryStream();

    virtual void    ResetError();

    sal_Size        GetEndOfData() const { return nEndOfData; }
    const void*     GetData() { Flush(); return pBuf; }
    operator const  void*() { Flush(); return pBuf; }
    virtual sal_uInt16  IsA() const;

    void*           SwitchBuffer( sal_Size nInitSize=512, sal_Size nResize=64 );
    void*           SetBuffer( void* pBuf, sal_Size nSize,
                               sal_Bool bOwnsData=sal_True, sal_Size nEOF=0 );

    void            ObjectOwnsMemory( sal_Bool bOwn ) { bOwnsData = bOwn; }
    sal_Bool            IsObjectMemoryOwner() { return bOwnsData; }
    void            SetResizeOffset( sal_Size nNewResize ) { nResize = nNewResize; }
    sal_Size            GetResizeOffset() const { return nResize; }
    virtual sal_Size remainingSize() { return GetSize() - Tell(); }
};

// --------------------
// - SvDataCopyStream -
// --------------------

// AB 10.5.1996: Diese Klasse bildet die Basis fuer Klassen, die mittels
// SvData (SO2\DTRANS.HXX/CXX) transportiert werden sollen, z.B. Graphik
// Die abgeleiteten Klassen muessen die virtuellen Funktionen ueberladen.

class TOOLS_DLLPUBLIC SvDataCopyStream
{
public:
    // mehrfaches Aufrufen von Load und Assign erlaubt
                    TYPEINFO();
    virtual         ~SvDataCopyStream(){}
    virtual void    Load( SvStream & ) = 0;
    virtual void    Save( SvStream & ) = 0;
    virtual void    Assign( const SvDataCopyStream & );
};

#endif // _STREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
