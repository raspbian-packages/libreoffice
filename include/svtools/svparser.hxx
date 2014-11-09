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

#ifndef INCLUDED_SVTOOLS_SVPARSER_HXX
#define INCLUDED_SVTOOLS_SVPARSER_HXX

#include <svtools/svtdllapi.h>
#include <tools/link.hxx>
#include <tools/ref.hxx>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/utility.hpp>
#include <vector>

struct SvParser_Impl;
class SvStream;

enum SvParserState
{
    SVPAR_ACCEPTED = 0,
    SVPAR_NOTSTARTED,
    SVPAR_WORKING,
    SVPAR_PENDING,
    SVPAR_WAITFORDATA,
    SVPAR_ERROR
};

class SVT_DLLPUBLIC SvParser : public SvRefBase
{
    DECL_STATIC_LINK( SvParser, NewDataRead, void* );

protected:
    SvStream&           rInput;
    OUString            aToken;             // gescanntes Token
    sal_uLong           nlLineNr;           // akt. Zeilen Nummer
    sal_uLong           nlLinePos;          // akt. Spalten Nummer

    SvParser_Impl       *pImplData;         // interne Daten
    long                nTokenValue;        // zusaetzlicher Wert (RTF)
    bool                bTokenHasValue;     // indicates whether nTokenValue is valid
    SvParserState       eState;             // Status auch in abgl. Klassen

    rtl_TextEncoding    eSrcEnc;        // Source encoding

    sal_uLong nNextChPos;
    sal_Unicode nNextCh;                // Akt. Zeichen fuer die "lex"


    bool                bDownloadingFile : 1;// sal_True: Es wird gerade ein externes
                                        //       File geladen. d.h. alle
                                        //       DataAvailable Links muessen
                                        //       ignoriert werden.
                                        // Wenn keibes der folgenden
                                        // Flags gesetzt ist, wird der
                                        // Stream als ANSI gelesen,
                                        // aber als CharSet DONTKNOW
                                        // zurueckgegeben.
    bool                bUCS2BSrcEnc : 1;   // oder als big-endian UCS2
    bool                bSwitchToUCS2 : 1;  // Umschalten des ist erlaubt

    bool                bRTF_InTextRead : 1;  // only for RTF-Parser!!!

    struct TokenStackType
    {
        OUString    sToken;
        long        nTokenValue;
        bool        bTokenHasValue;
        int         nTokenId;

        TokenStackType()
            : nTokenValue(0)
            , bTokenHasValue(false)
            , nTokenId(0)
        {
        }
        ~TokenStackType() { }
    };

    // Methoden fuer Token-Stack
    int SkipToken( short nCnt = -1 );       // n Tokens zurueck "skippen"
    TokenStackType* GetStackPtr( short nCnt );
    inline sal_uInt8 GetStackPos() const;

    // scanne das naechste Token:
    //  Tokenstack abarbeiten und ggfs. _GetNextToken() rufen. Diese
    //  ist fuers erkennen von neuen Token verantwortlich
    int GetNextToken();
    virtual int _GetNextToken() = 0;

    // wird fuer jedes Token gerufen, das in CallParser erkannt wird
    virtual void NextToken( int nToken );

    // zu Zeiten der SvRefBase-Ableitung darf nicht jeder loeschen
    virtual ~SvParser();

    void ClearTxtConvContext();

private:
    TokenStackType* pTokenStack;
    TokenStackType *pTokenStackPos;
    sal_uInt8 nTokenStackSize, nTokenStackPos;

public:
    // Konstruktor
    SvParser( SvStream& rIn, sal_uInt8 nStackSize = 3 );

    virtual  SvParserState CallParser() = 0;    // Aufruf des Parsers

    inline SvParserState GetStatus() const  { return eState; }  // StatusInfo

    inline sal_uLong    GetLineNr() const       { return nlLineNr; }
    inline sal_uLong    GetLinePos() const      { return nlLinePos; }
    inline sal_uLong    IncLineNr()             { return ++nlLineNr; }
    inline sal_uLong    IncLinePos()            { return ++nlLinePos; }
    inline sal_uLong    SetLineNr( sal_uLong nlNum );           // inline unten
    inline sal_uLong    SetLinePos( sal_uLong nlPos );          // inline unten

    sal_Unicode GetNextChar();
    void RereadLookahead();

    inline bool IsParserWorking() const { return SVPAR_WORKING == eState; }

    Link GetAsynchCallLink() const
        { return STATIC_LINK( this, SvParser, NewDataRead ); }

    long CallAsyncCallLink() { return NewDataRead( this, 0 ); }

    // fuers asynchrone lesen aus dem SvStream
    /*virtual*/ void SaveState( int nToken );
    /*virtual*/ void RestoreState();
    virtual void Continue( int nToken );

    inline void SetDownloadingFile( bool bSet ) { bDownloadingFile = bSet; }
    inline bool IsDownloadingFile() const { return bDownloadingFile; }

    // Set/get source encoding. The UCS2BEncoding flag is valid if source
    // encoding is UCS2. It specifies a big endian encoding.
    void SetSrcEncoding( rtl_TextEncoding eSrcEnc );
    rtl_TextEncoding GetSrcEncoding() const { return eSrcEnc; }

    void SetSrcUCS2BEncoding( bool bSet ) { bUCS2BSrcEnc = bSet; }
    bool IsSrcUCS2BEncoding() const { return bUCS2BSrcEnc; }

    // Darf der Zeichensatz auf UCS/2 umgeschaltet werden, wenn
    // in den ersten beiden Zeichen im Stream eine BOM steht?
    void SetSwitchToUCS2( bool bSet ) { bSwitchToUCS2 = bSet; }
    bool IsSwitchToUCS2() const { return bSwitchToUCS2; }

    // Aus wie vielen Bytes betseht ein Zeichen
    inline sal_uInt16 GetCharSize() const;

    int GetSaveToken() const;

    // Aufbau einer Which-Map 'rWhichMap' aus einem Array von
    // 'pWhichIds' von Which-Ids. Es hat die Lange 'nWhichIds'.
    // Die Which-Map wird nicht geloescht.
    static void BuildWhichTbl( std::vector<sal_uInt16> &rWhichMap,
                               sal_uInt16 *pWhichIds,
                               sal_uInt16 nWhichIds );
};


#ifndef GOODIES_DECL_SVPARSER_DEFINED
#define GOODIES_DECL_SVPARSER_DEFINED
typedef tools::SvRef<SvParser> SvParserRef;
#endif

inline sal_uLong SvParser::SetLineNr( sal_uLong nlNum )
{   sal_uLong nlOld = nlLineNr; nlLineNr = nlNum; return nlOld; }

inline sal_uLong SvParser::SetLinePos( sal_uLong nlPos )
{   sal_uLong nlOld = nlLinePos; nlLinePos = nlPos; return nlOld; }

inline sal_uInt8 SvParser::GetStackPos() const
{   return nTokenStackPos; }

inline sal_uInt16 SvParser::GetCharSize() const
{
    return (RTL_TEXTENCODING_UCS2 == eSrcEnc) ? 2 : 1;
}


/*========================================================================
 *
 * SvKeyValue.
 *
 *======================================================================*/

class SvKeyValue
{
    /** Representation.
    */
    OUString m_aKey;
    OUString m_aValue;

public:
    /** Construction.
    */
    SvKeyValue (void)
    {}

    SvKeyValue (const OUString &rKey, const OUString &rValue)
        : m_aKey (rKey), m_aValue (rValue)
    {}

    SvKeyValue (const SvKeyValue &rOther)
        : m_aKey (rOther.m_aKey), m_aValue (rOther.m_aValue)
    {}

    /** Assignment.
    */
    SvKeyValue& operator= (SvKeyValue &rOther)
    {
        m_aKey   = rOther.m_aKey;
        m_aValue = rOther.m_aValue;
        return *this;
    }

    /** Operation.
    */
    const OUString& GetKey   (void) const { return m_aKey; }
    const OUString& GetValue (void) const { return m_aValue; }

    void SetKey   (const OUString &rKey  ) { m_aKey = rKey; }
    void SetValue (const OUString &rValue) { m_aValue = rValue; }
};

/*========================================================================
 *
 * SvKeyValueIterator.
 *
 *======================================================================*/

typedef boost::ptr_vector<SvKeyValue> SvKeyValueList_Impl;

class SVT_DLLPUBLIC SvKeyValueIterator : public SvRefBase,
    private boost::noncopyable
{
    /** Representation.
    */
    SvKeyValueList_Impl* m_pList;
    sal_uInt16               m_nPos;

public:
    /** Construction/Destruction.
    */
    SvKeyValueIterator (void);
    virtual ~SvKeyValueIterator (void);

    /** Operation.
    */
    virtual bool GetFirst (SvKeyValue &rKeyVal);
    virtual bool GetNext  (SvKeyValue &rKeyVal);
    virtual void Append   (const SvKeyValue &rKeyVal);
};

typedef tools::SvRef<SvKeyValueIterator> SvKeyValueIteratorRef;

#endif // INCLUDED_SVTOOLS_SVPARSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
