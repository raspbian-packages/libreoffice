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


#ifdef DBG_UTIL

/*
 * Und hier die Beschreibung:
 *
 * Durch die PROTOCOL-Makros wird es ermoeglicht, Ereignisse im Frame-Methoden zu protokollieren.
 * In protokollwuerdigen Stellen in Frame-Methoden muss entweder ein PROTOCOL(...) oder bei Methoden,
 * bei denen auch das Verlassen der Methode mitprotokolliert werden soll, ein PROTOCOL_ENTER(...)-Makro
 * stehen.
 * Die Parameter der PROTOCOL-Makros sind
 * 1.   Ein Pointer auf einen SwFrm, also meist "this" oder "rThis"
 * 2.   Die Funktionsgruppe z.B. PROT_MAKEALL, hierueber wird (inline) entschieden, ob dies
 *      zur Zeit protokolliert werden soll oder nicht.
 * 3.   Die Aktion, im Normalfall 0, aber z.B. ein ACT_START bewirkt eine Einrueckung in der
 *      Ausgabedatei, ein ACT_END nimmt dies wieder zurueck. Auf diese Art wird z.B. durch
 *      PROTOCOL_ENTER am Anfang einer Methode eingerueckt und beim Verlassen wieder zurueck.
 * 4.   Der vierte Parameter ist ein void-Pointer, damit man irgendetwas uebergeben kann,
 *      was in das Protokoll einfliessen kann, typesches Beispiel bei PROT_GROW muss man
 *      einen Pointer auf den Wert, um den gegrowt werden soll, uebergeben.
 *
 *
 * Das Protokoll ist die Datei "dbg_lay.out" im aktuellen (BIN-)Verzeichnis.
 * Es enthaelt Zeilen mit FrmId, Funktionsgruppe sowie weiteren Infos.
 *
 * Was genau protokolliert wird, kann auf folgende Arten eingestellt werden:
 * 1.   Die statische Variable SwProtokoll::nRecord enthaelt die Funktionsgruppen,
 *      die aufgezeichnet werden sollen.
 *      Ein Wert von z.B. PROT_GROW bewirkt, das Aufrufe von SwFrm::Grow dokumentiert werden,
 *      PROT_MAKEALL protokolliert Aufrufe von xxx::MakeAll.
 *      Die PROT_XY-Werte koennen oderiert werden.
 *      Default ist Null, es wird keine Methode aufgezeichnet.
 * 2.   In der SwImplProtocol-Klasse gibt es einen Filter fuer Frame-Typen,
 *      nur die Methodenaufrufe von Frame-Typen, die dort gesetzt sind, werden protokolliert.
 *      Der Member nTypes kann auf Werte wie FRM_PAGE, FRM_SECTION gesetzt und oderiert werden.
 *      Default ist 0xFFFF, d.h. alle Frame-Typen.
 * 3.   In der SwImplProtocol-Klasse gibt es einen ArrayPointer auf FrmIds, die zu ueberwachen sind.
 *      Ist der Pointer Null, so werden alle Frames protokolliert, ansonsten nur Frames,
 *      die in dem Array vermerkt sind.
 *
 * Eine Aufzeichnung in Gang zu setzen, erfordert entweder Codemanipulation, z.B. in
 * SwProtocol::Init() einen anderen Default fuer nRecord setzen oder Debuggermanipulation.
 * Im Debugger gibt verschiedene, sich anbietende Stellen:
 * 1.   In SwProtocol::Init() einen Breakpoint setzen und dort nRecord manipulieren, ggf.
 *      FrmIds eintragen, dann beginnt die Aufzeichnung bereits beim Programmstart.
 * 2.   Waehrend des Programmlaufs einen Breakpoint vor irgendein PROTOCOL oder PROTOCOL_ENTER-
 *      Makro setzen, dann am SwProtocol::nRecord das unterste Bit setzen (PROT_INIT). Dies
 *      bewirkt, dass die Funktionsgruppe des folgenden Makros aktiviert und in Zukunft
 *      protokolliert wird.
 * 3.   Spezialfall von 2.: Wenn man 2. in der Methode SwRootFrm::Paint(..) anwendet, werden
 *      die Aufzeichnungseinstellung aus der Datei "dbg_lay.ini" ausgelesen!
 *      In dieser INI-Datei kann es Kommentarzeilen geben, diese beginnen mit '#', dann
 *      sind die Sektionen "[frmid]", "[frmtype]" und "[record]" relevant.
 *      Nach [frmid] koennen die FrameIds der zu protokollierenden Frames folgen. Gibt es
 *      dort keine Eintraege, werden alle Frames aufgezeichnet.
 *      Nach [frmtype] koennen FrameTypen folgen, die aufgezeichnet werden sollen, da der
 *      Default hier allerdings USHRT_MAX ist, werden sowieso alle aufgezeichnet. Man kann
 *      allerdings auch Typen entfernen, in dem man ein '!' vor den Wert setzt, z.B.
 *      !0xC000 nimmt die SwCntntFrms aus der Aufzeichnung heraus.
 *      Nach [record] folgen die Funktionsgruppen, die aufgezeichnet werden sollen, Default
 *      ist hier 0, also keine. Auch hier kann man mit einem vorgestellten '!' Funktionen
 *      wieder entfernen.
 *      Hier mal ein Beispiel fuer eine INI-Datei:
 *      ------------------------------------------
 *          #Funktionen: Alle, ausser PRTAREA
 *          [record] 0xFFFFFFE !0x200
 *          [frmid]
 *          #folgende FrmIds:
 *          1 2 12 13 14 15
 *          #keine Layoutframes ausser ColumnFrms
 *          [frmtype] !0x3FFF 0x4
 *      ------------------------------------------
 *
 * Wenn die Aufzeichnung erstmal laeuft, kann man in SwImplProtocol::_Record(...) mittels
 * Debugger vielfaeltige Manipulationen vornehmen, z.B. bezueglich FrameTypen oder FrmIds.
 *
 * --------------------------------------------------*/

#include "dbg_lay.hxx"
#include <tools/stream.hxx>

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#endif

#include <stdio.h>

#include "frame.hxx"
#include "layfrm.hxx"
#include "flyfrm.hxx"
#include "txtfrm.hxx"
#include "ndtxt.hxx"
#include "dflyobj.hxx"
#include <fntcache.hxx>
#include <sortedobjs.hxx> // #i28701#

#include <rtl/strbuf.hxx>
#include <comphelper/string.hxx>

sal_uLong SwProtocol::nRecord = 0;
SwImplProtocol* SwProtocol::pImpl = NULL;

sal_uLong lcl_GetFrameId( const SwFrm* pFrm )
{
#if OSL_DEBUG_LEVEL > 1
    static sal_Bool bFrameId = sal_False;
    if( bFrameId )
        return pFrm->GetFrmId();
#endif
    if( pFrm && pFrm->IsTxtFrm() )
        return ((SwTxtFrm*)pFrm)->GetTxtNode()->GetIndex();
    return 0;
}

class SwImplProtocol
{
    SvFileStream *pStream;      // Ausgabestream
    std::set<sal_uInt16> *pFrmIds;      // welche FrmIds sollen aufgezeichnet werden ( NULL == alle )
    std::vector<long> aVars;    // Variables
    rtl::OStringBuffer aLayer;          // Einrueckung der Ausgabe ("  " pro Start/End)
    sal_uInt16 nTypes;              // welche Typen sollen aufgezeichnet werden
    sal_uInt16 nLineCount;          // Ausgegebene Zeilen
    sal_uInt16 nMaxLines;           // Maximal auszugebende Zeilen
    sal_uInt8 nInitFile;                // Bereich (FrmId,FrmType,Record) beim Einlesen der INI-Datei
    sal_uInt8 nTestMode;                // Special fuer Testformatierung, es wird ggf. nur
                                // innerhalb einer Testformatierung aufgezeichnet.
    void _Record( const SwFrm* pFrm, sal_uLong nFunction, sal_uLong nAct, void* pParam );
    sal_Bool NewStream();
    void CheckLine( rtl::OString& rLine );
    void SectFunc( rtl::OStringBuffer& rOut, const SwFrm* pFrm, sal_uLong nAct, void* pParam );
public:
    SwImplProtocol();
    ~SwImplProtocol();
    // Aufzeichnen
    void Record( const SwFrm* pFrm, sal_uLong nFunction, sal_uLong nAct, void* pParam )
        { if( pStream ) _Record( pFrm, nFunction, nAct, pParam ); }
    sal_Bool InsertFrm( sal_uInt16 nFrmId );    // FrmId aufnehmen zum Aufzeichnen
    sal_Bool DeleteFrm( sal_uInt16 nFrmId );    // FrmId entfernen, diesen nicht mehr Aufzeichnen
    void FileInit();                    // Auslesen der INI-Datei
    void ChkStream() { if( !pStream ) NewStream(); }
    void SnapShot( const SwFrm* pFrm, sal_uLong nFlags );
    void GetVar( const sal_uInt16 nNo, long& rVar )
        { if( nNo < aVars.size() ) rVar = aVars[ nNo ]; }
};

/* --------------------------------------------------
 * Durch das PROTOCOL_ENTER-Makro wird ein SwEnterLeave-Objekt erzeugt,
 * wenn die aktuelle Funktion aufgezeichnet werden soll, wird ein
 * SwImplEnterLeave-Objekt angelegt. Der Witz dabei ist, das der Ctor
 * des Impl-Objekt am Anfang der Funktion und automatisch der Dtor beim
 * Verlassen der Funktion gerufen wird. In der Basis-Implementierung ruft
 * der Ctor lediglich ein PROTOCOL(..) mit ACT_START und im Dtor ein
 * PROTOCOL(..) mit ACT_END.
 * Es lassen sich Ableitungen der Klasse bilden, um z.B. beim Verlassen
 * einer Funktion Groessenaenderungen des Frames zu dokumentieren u.v.a.m.
 * Dazu braucht dann nur noch in SwEnterLeave::Ctor(...) die gewuenschte
 * SwImplEnterLeave-Klasse angelegt zu werden.
 *
 * --------------------------------------------------*/

class SwImplEnterLeave
{
protected:
    const SwFrm* pFrm;              // Der Frame,
    sal_uLong nFunction, nAction;       // die Funktion, ggf. die Aktion
    void* pParam;                   // und weitere Parameter
public:
    SwImplEnterLeave( const SwFrm* pF, sal_uLong nFunct, sal_uLong nAct, void* pPar )
        : pFrm( pF ), nFunction( nFunct ), nAction( nAct ), pParam( pPar ) {}
    virtual void Enter();           // Ausgabe beim Eintritt
    virtual void Leave();           // Ausgabe beim Verlassen
};

class SwSizeEnterLeave : public SwImplEnterLeave
{
    long nFrmHeight;
public:
    SwSizeEnterLeave( const SwFrm* pF, sal_uLong nFunct, sal_uLong nAct, void* pPar )
        : SwImplEnterLeave( pF, nFunct, nAct, pPar ), nFrmHeight( pF->Frm().Height() ) {}
    virtual void Leave();           // Ausgabe der Groessenaenderung
};

class SwUpperEnterLeave : public SwImplEnterLeave
{
    sal_uInt16 nFrmId;
public:
    SwUpperEnterLeave( const SwFrm* pF, sal_uLong nFunct, sal_uLong nAct, void* pPar )
        : SwImplEnterLeave( pF, nFunct, nAct, pPar ), nFrmId( 0 ) {}
    virtual void Enter();           // Ausgabe
    virtual void Leave();           // Ausgabe der FrmId des Uppers
};

class SwFrmChangesLeave : public SwImplEnterLeave
{
    SwRect aFrm;
public:
    SwFrmChangesLeave( const SwFrm* pF, sal_uLong nFunct, sal_uLong nAct, void* pPar )
        : SwImplEnterLeave( pF, nFunct, nAct, pPar ), aFrm( pF->Frm() ) {}
    virtual void Enter();           // keine Ausgabe
    virtual void Leave();           // Ausgabe bei Aenderung der Frm-Area
};

void SwProtocol::Record( const SwFrm* pFrm, sal_uLong nFunction, sal_uLong nAct, void* pParam )
{
    if( Start() )
    {   // Hier landen wir, wenn im Debugger SwProtocol::nRecord mit PROT_INIT(0x1) oderiert wurde
        sal_Bool bFinit = sal_False; // Dies bietet im Debugger die Moeglichkeit,
        if( bFinit )         // die Aufzeichnung dieser Action zu beenden
        {
            nRecord &= ~nFunction;  // Diese Funktion nicht mehr aufzeichnen
            nRecord &= ~PROT_INIT;  // PROT_INIT stets zuruecksetzen
            return;
        }
        nRecord |= nFunction;       // Aufzeichnung dieser Funktion freischalten
        nRecord &= ~PROT_INIT;      // PROT_INIT stets zuruecksetzen
        if( pImpl )
            pImpl->ChkStream();
    }
    if( !pImpl )                        // Impl-Object anlegen, wenn noetig
        pImpl = new SwImplProtocol();
    pImpl->Record( pFrm, nFunction, nAct, pParam ); // ...und Aufzeichnen
}

// Die folgende Funktion wird beim Anziehen der Writer-DLL durch TxtInit(..) aufgerufen
// und ermoeglicht dem Debuggenden Funktionen und/oder FrmIds freizuschalten

void SwProtocol::Init()
{
    nRecord = 0;
    XubString aName( "dbg_lay.go", RTL_TEXTENCODING_MS_1252 );
    SvFileStream aStream( aName, STREAM_READ );
    if( aStream.IsOpen() )
    {
        pImpl = new SwImplProtocol();
        pImpl->FileInit();
    }
    aStream.Close();
}

// Ende der Aufzeichnung

void SwProtocol::Stop()
{
     if( pImpl )
     {
        delete pImpl;
        pImpl = NULL;
        if( pFntCache )
            pFntCache->Flush();
     }
     nRecord = 0;
}

// Creates a more or less detailed snapshot of the layout structur

void SwProtocol::SnapShot( const SwFrm* pFrm, sal_uLong nFlags )
{
    if( pImpl )
        pImpl->SnapShot( pFrm, nFlags );
}

void SwProtocol::GetVar( const sal_uInt16 nNo, long& rVar )
{
    if( pImpl )
        pImpl->GetVar( nNo, rVar );
}

SwImplProtocol::SwImplProtocol()
    : pStream( NULL ), pFrmIds( NULL ), nTypes( 0xffff ),
      nLineCount( 0 ), nMaxLines( USHRT_MAX ), nTestMode( 0 )
{
    NewStream();
}

sal_Bool SwImplProtocol::NewStream()
{
    XubString aName( "dbg_lay.out", RTL_TEXTENCODING_MS_1252 );
    nLineCount = 0;
    pStream = new SvFileStream( aName, STREAM_WRITE | STREAM_TRUNC );
    if( pStream->GetError() )
    {
        delete pStream;
        pStream = NULL;
    }
    return 0 != pStream;
}

SwImplProtocol::~SwImplProtocol()
{
    if( pStream )
    {
        pStream->Close();
        delete pStream;
    }
    pFrmIds->clear();
    delete pFrmIds;
    aVars.clear();
}

/* --------------------------------------------------
 * SwImplProtocol::CheckLine analysiert eine Zeile der INI-Datei
 * --------------------------------------------------*/

void SwImplProtocol::CheckLine( rtl::OString& rLine )
{
    rLine = rLine.toAsciiLowerCase(); // Gross/Kleinschreibung ist einerlei
    rLine = rLine.replace( '\t', ' ' );
    if( '#' == rLine[0] )   // Kommentarzeilen beginnen mit '#'
        return;
    if( '[' == rLine[0] )   // Bereiche: FrmIds, Typen oder Funktionen
    {
        rtl::OString aTmp = comphelper::string::getToken(rLine, 0, ']');
        if (aTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("[frmid")))      // Bereich FrmIds
        {
            nInitFile = 1;
            pFrmIds->clear();
            delete pFrmIds;
            pFrmIds = NULL;         // Default: Alle Frames aufzeichnen
        }
        else if (aTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("[frmtype")))// Bereich Typen
        {
            nInitFile = 2;
            nTypes = USHRT_MAX;     // Default: Alle FrmaeTypen aufzeichnen
        }
        else if (aTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("[record")))// Bereich Funktionen
        {
            nInitFile = 3;
            SwProtocol::SetRecord( 0 );// Default: Keine Funktion wird aufgezeichnet
        }
        else if (aTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("[test")))// Bereich Funktionen
        {
            nInitFile = 4; // Default:
            nTestMode = 0; // Ausserhalb der Testformatierung wird aufgezeichnet
        }
        else if (aTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("[max")))// maximale Zeilenzahl
        {
            nInitFile = 5; // Default:
            nMaxLines = USHRT_MAX;
        }
        else if (aTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("[var")))// variables
        {
            nInitFile = 6;
        }
        else
            nInitFile = 0;          // Nanu: Unbekannter Bereich?
        rLine = rLine.copy(aTmp.getLength() + 1);
    }

    // Blanks (oder Tabs) sind die Trenner
    sal_Int32 nIndex = 0;
    do
    {
        rtl::OString aTok = rLine.getToken( 0, ' ', nIndex );
        sal_Bool bNo = sal_False;
        if( '!' == aTok[0] )
        {
            bNo = sal_True;                 // Diese(n) Funktion/Typ entfernen
            aTok = aTok.copy(1);
        }
        if( aTok.getLength() )
        {
            sal_uLong nVal;
            sscanf( aTok.getStr(), "%li", &nVal );
            switch ( nInitFile )
            {
                case 1: InsertFrm( sal_uInt16( nVal ) );    // FrmId aufnehmen
                        break;
                case 2: {
                            sal_uInt16 nNew = (sal_uInt16)nVal;
                            if( bNo )
                                nTypes &= ~nNew;    // Typ entfernen
                            else
                                nTypes |= nNew;     // Typ aufnehmen
                        }
                        break;
                case 3: {
                            sal_uLong nOld = SwProtocol::Record();
                            if( bNo )
                                nOld &= ~nVal;      // Funktion entfernen
                            else
                                nOld |= nVal;       // Funktion aufnehmen
                            SwProtocol::SetRecord( nOld );
                        }
                        break;
                case 4: {
                            sal_uInt8 nNew = (sal_uInt8)nVal;
                            if( bNo )
                                nTestMode &= ~nNew; // TestMode zuruecksetzen
                            else
                                nTestMode |= nNew;      // TestMode setzen
                        }
                        break;
                case 5: nMaxLines = (sal_uInt16)nVal;
                        break;
                case 6: aVars.push_back( (long)nVal );
                        break;
            }
        }
    }
    while ( nIndex >= 0 );
}

/* --------------------------------------------------
 * SwImplProtocol::FileInit() liest die Datei "dbg_lay.ini"
 * im aktuellen Verzeichnis und wertet sie aus.
 * --------------------------------------------------*/
void SwImplProtocol::FileInit()
{
    XubString aName( "dbg_lay.ini", RTL_TEXTENCODING_MS_1252 );
    SvFileStream aStream( aName, STREAM_READ );
    if( aStream.IsOpen() )
    {
        rtl::OString aLine;
        nInitFile = 0;
        while( aStream.good() )
        {
            sal_Char c;
            aStream >> c;
            if( '\n' == c || '\r' == c )    // Zeilenende
            {
                aLine = aLine.trim();
                if( aLine.getLength() )
                    CheckLine( aLine );     // Zeile auswerten
                aLine = rtl::OString();
            }
            else
                aLine = rtl::OString(c);
        }
        if( aLine.getLength() )
            CheckLine( aLine );     // letzte Zeile auswerten
    }
    aStream.Close();
}

/* --------------------------------------------------
 * lcl_Start sorgt fuer Einrueckung um zwei Blanks bei ACT_START
 * und nimmt diese bei ACT_END wieder zurueck.
 * --------------------------------------------------*/
void lcl_Start(rtl::OStringBuffer& rOut, rtl::OStringBuffer& rLay, sal_uLong nAction)
{
    if( nAction == ACT_START )
    {
        rLay.append(RTL_CONSTASCII_STRINGPARAM("  "));
        rOut.append(RTL_CONSTASCII_STRINGPARAM(" On"));
    }
    else if( nAction == ACT_END )
    {
        if( rLay.getLength() > 1 )
        {
            rLay.remove(rLay.getLength() - 2, rLay.getLength());
            rOut.remove(0, 2);
        }
        rOut.append(RTL_CONSTASCII_STRINGPARAM(" Off"));
    }
}

/* --------------------------------------------------
 * lcl_Flags gibt das ValidSize-, ValidPos- und ValidPrtArea-Flag ("Sz","Ps","PA")
 * des Frames aus, "+" fuer valid, "-" fuer invalid.
 * --------------------------------------------------*/

void lcl_Flags(rtl::OStringBuffer& rOut, const SwFrm* pFrm)
{
    rOut.append(RTL_CONSTASCII_STRINGPARAM(" Sz"));
    rOut.append(pFrm->GetValidSizeFlag() ? '+' : '-');
    rOut.append(RTL_CONSTASCII_STRINGPARAM(" Ps"));
    rOut.append(pFrm->GetValidPosFlag() ? '+' : '-');
    rOut.append(RTL_CONSTASCII_STRINGPARAM(" PA"));
    rOut.append(pFrm->GetValidPrtAreaFlag() ? '+' : '-');
}

/* --------------------------------------------------
 * lcl_FrameType gibt den Typ des Frames in Klartext aus.
 * --------------------------------------------------*/

void lcl_FrameType( rtl::OStringBuffer& rOut, const SwFrm* pFrm )
{
    if( pFrm->IsTxtFrm() )
        rOut.append(RTL_CONSTASCII_STRINGPARAM("Txt "));
    else if( pFrm->IsLayoutFrm() )
    {
        if( pFrm->IsPageFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Page "));
        else if( pFrm->IsColumnFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Col "));
        else if( pFrm->IsBodyFrm() )
        {
            if( pFrm->GetUpper() && pFrm->IsColBodyFrm() )
                rOut.append(RTL_CONSTASCII_STRINGPARAM("(Col)"));
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Body "));
        }
        else if( pFrm->IsRootFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Root "));
        else if( pFrm->IsCellFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Cell "));
        else if( pFrm->IsTabFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Tab "));
        else if( pFrm->IsRowFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Row "));
        else if( pFrm->IsSctFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Sect "));
        else if( pFrm->IsHeaderFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Header "));
        else if( pFrm->IsFooterFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Footer "));
        else if( pFrm->IsFtnFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Ftn "));
        else if( pFrm->IsFtnContFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("FtnCont "));
        else if( pFrm->IsFlyFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Fly "));
        else
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Layout "));
    }
    else if( pFrm->IsNoTxtFrm() )
        rOut.append(RTL_CONSTASCII_STRINGPARAM("NoTxt "));
    else
        rOut.append(RTL_CONSTASCII_STRINGPARAM("Not impl. "));
}

/* --------------------------------------------------
 * SwImplProtocol::Record(..) wird nur gerufen, wenn das PROTOCOL-Makro
 * feststellt, dass die Funktion aufgezeichnet werden soll ( SwProtocol::nRecord ).
 * In dieser Methode werden noch die beiden weiteren Einschraenkungen ueberprueft,
 * ob die FrmId und der FrameType zu den aufzuzeichnenden gehoeren.
 * --------------------------------------------------*/

void SwImplProtocol::_Record( const SwFrm* pFrm, sal_uLong nFunction, sal_uLong nAct, void* pParam )
{
    sal_uInt16 nSpecial = 0;
    if( nSpecial )  // Debugger-Manipulationsmoeglichkeit
    {
        sal_uInt16 nId = sal_uInt16(lcl_GetFrameId( pFrm ));
        switch ( nSpecial )
        {
            case 1: InsertFrm( nId ); break;
            case 2: DeleteFrm( nId ); break;
            case 3: pFrmIds->clear(); delete pFrmIds; pFrmIds = NULL; break;
            case 4: delete pStream; pStream = NULL; break;
        }
        return;
    }
    if( !pStream && !NewStream() )
        return; // Immer noch kein Stream

    if( pFrmIds && !pFrmIds->count( sal_uInt16(lcl_GetFrameId( pFrm )) ) )
        return; // gehoert nicht zu den gewuenschten FrmIds

    if( !(pFrm->GetType() & nTypes) )
        return; // Der Typ ist unerwuenscht

    if( 1 == nTestMode && nFunction != PROT_TESTFORMAT )
        return; // Wir sollen nur innerhalb einer Testformatierung aufzeichnen
    sal_Bool bTmp = sal_False;
    rtl::OStringBuffer aOut(aLayer);
    aOut.append(static_cast<sal_Int64>(lcl_GetFrameId(pFrm)));
    aOut.append(' ');
    lcl_FrameType( aOut, pFrm );    // dann den FrameType
    switch ( nFunction )            // und die Funktion
    {
        case PROT_SNAPSHOT: lcl_Flags( aOut, pFrm );
                            break;
        case PROT_MAKEALL:  aOut.append(RTL_CONSTASCII_STRINGPARAM("MakeAll"));
                            lcl_Start( aOut, aLayer, nAct );
                            if( nAct == ACT_START )
                                lcl_Flags( aOut, pFrm );
                            break;
        case PROT_MOVE_FWD: bTmp = sal_True; // NoBreak
        case PROT_MOVE_BWD:
                            if (nFunction == bTmp)
                                aOut.append(RTL_CONSTASCII_STRINGPARAM("Fwd"));
                            else
                                aOut.append(RTL_CONSTASCII_STRINGPARAM("Bwd"));
                            lcl_Start( aOut, aLayer, nAct );
                            if( pParam )
                            {
                                aOut.append(' ');
                                aOut.append(static_cast<sal_Int32>(*((sal_uInt16*)pParam)));
                            }
                            break;
        case PROT_GROW_TST: if( ACT_START != nAct )
                                return;
                            aOut.append(RTL_CONSTASCII_STRINGPARAM("TestGrow"));
                            break;
        case PROT_SHRINK_TST: if( ACT_START != nAct )
                                return;
                            aOut.append(RTL_CONSTASCII_STRINGPARAM("TestShrink"));
                            break;
        case PROT_ADJUSTN :
        case PROT_SHRINK:   bTmp = sal_True; // NoBreak
        case PROT_GROW:
                            if (!bTmp)
                                aOut.append(RTL_CONSTASCII_STRINGPARAM("Grow"));
                            else
                            {
                                if (nFunction == PROT_SHRINK)
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM("Shrink"));
                                else
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM("AdjustNgbhd"));
                            }
                            lcl_Start( aOut, aLayer, nAct );
                            if( pParam )
                            {
                                aOut.append(' ');
                                aOut.append(static_cast<sal_Int64>(*((long*)pParam)));
                            }
                            break;
        case PROT_POS:      break;
        case PROT_PRTAREA:  aOut.append(RTL_CONSTASCII_STRINGPARAM("PrtArea"));
                            lcl_Start( aOut, aLayer, nAct );
                            break;
        case PROT_SIZE:     aOut.append(RTL_CONSTASCII_STRINGPARAM("Size"));
                            lcl_Start( aOut, aLayer, nAct );
                            aOut.append(' ');
                            aOut.append(static_cast<sal_Int64>(pFrm->Frm().Height()));
                            break;
        case PROT_LEAF:     aOut.append(RTL_CONSTASCII_STRINGPARAM("Prev/NextLeaf"));
                            lcl_Start( aOut, aLayer, nAct );
                            aOut.append(' ');
                            if( pParam )
                            {
                                aOut.append(' ');
                                aOut.append(static_cast<sal_Int64>(lcl_GetFrameId((SwFrm*)pParam)));
                            }
                            break;
        case PROT_FILE_INIT: FileInit();
                            aOut.append(RTL_CONSTASCII_STRINGPARAM("Initialize"));
                            break;
        case PROT_SECTION:  SectFunc(aOut, pFrm, nAct, pParam);
                            break;
        case PROT_CUT:      bTmp = sal_True; // NoBreak
        case PROT_PASTE:
                            if (bTmp)
                                aOut.append(RTL_CONSTASCII_STRINGPARAM("Cut from "));
                            else
                                aOut.append(RTL_CONSTASCII_STRINGPARAM("Paste to "));
                            aOut.append(static_cast<sal_Int64>(lcl_GetFrameId((SwFrm*)pParam)));
                            break;
        case PROT_TESTFORMAT:
                            aOut.append(RTL_CONSTASCII_STRINGPARAM("Test"));
                            lcl_Start( aOut, aLayer, nAct );
                            if( ACT_START == nAct )
                                nTestMode |= 2;
                            else
                                nTestMode &= ~2;
                            break;
        case PROT_FRMCHANGES:
                            {
                                SwRect& rFrm = *((SwRect*)pParam);
                                if( pFrm->Frm().Pos() != rFrm.Pos() )
                                {
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM("PosChg: ("));
                                    aOut.append(static_cast<sal_Int64>(rFrm.Left()));
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM(", "));
                                    aOut.append(static_cast<sal_Int64>(rFrm.Top()));
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM(") ("));
                                    aOut.append(static_cast<sal_Int64>(pFrm->Frm().Left()));
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM(", "));
                                    aOut.append(static_cast<sal_Int64>(pFrm->Frm().Top()));
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM(") "));
                                }
                                if( pFrm->Frm().Height() != rFrm.Height() )
                                {
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM("Height: "));
                                    aOut.append(static_cast<sal_Int64>(rFrm.Height()));
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM(" -> "));
                                    aOut.append(static_cast<sal_Int64>(pFrm->Frm().Height()));
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM(" "));
                                }
                                if( pFrm->Frm().Width() != rFrm.Width() )
                                {
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM("Width: "));
                                    aOut.append(static_cast<sal_Int64>(rFrm.Width()));
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM(" -> "));
                                    aOut.append(static_cast<sal_Int64>(pFrm->Frm().Width()));
                                    aOut.append(' ');
                                }
                                break;
                            }
    }
    *pStream << aOut.getStr() << endl;  // Ausgabe
    pStream->Flush();   // Gleich auf die Platte, damit man mitlesen kann
    if( ++nLineCount >= nMaxLines )     // Maximale Ausgabe erreicht?
        SwProtocol::SetRecord( 0 );        // => Ende der Aufzeichnung
}

/* --------------------------------------------------
 * SwImplProtocol::SectFunc(...) wird von SwImplProtocol::_Record(..) gerufen,
 * hier werden die Ausgaben rund um SectionFrms abgehandelt.
 * --------------------------------------------------*/

void SwImplProtocol::SectFunc(rtl::OStringBuffer &rOut, const SwFrm* , sal_uLong nAct, void* pParam)
{
    sal_Bool bTmp = sal_False;
    switch( nAct )
    {
        case ACT_MERGE:         rOut.append(RTL_CONSTASCII_STRINGPARAM("Merge Section "));
                                rOut.append(static_cast<sal_Int64>(lcl_GetFrameId((SwFrm*)pParam)));
                                break;
        case ACT_CREATE_MASTER: bTmp = sal_True; // NoBreak
        case ACT_CREATE_FOLLOW: rOut.append(RTL_CONSTASCII_STRINGPARAM("Create Section "));
                                if (bTmp)
                                    rOut.append(RTL_CONSTASCII_STRINGPARAM("Master to "));
                                else
                                    rOut.append(RTL_CONSTASCII_STRINGPARAM("Follow from "));
                                rOut.append(static_cast<sal_Int64>(lcl_GetFrameId((SwFrm*)pParam)));
                                break;
        case ACT_DEL_MASTER:    bTmp = sal_True; // NoBreak
        case ACT_DEL_FOLLOW:    rOut.append(RTL_CONSTASCII_STRINGPARAM("Delete Section "));
                                if (bTmp)
                                    rOut.append(RTL_CONSTASCII_STRINGPARAM("Master to "));
                                else
                                    rOut.append(RTL_CONSTASCII_STRINGPARAM("Follow from "));
                                rOut.append(static_cast<sal_Int64>(lcl_GetFrameId((SwFrm*)pParam)));
                                break;
    }
}

/* --------------------------------------------------
 * SwImplProtocol::InsertFrm(..) nimmt eine neue FrmId zum Aufzeichnen auf,
 * wenn pFrmIds==NULL, werden alle aufgezeichnet, sobald durch InsertFrm(..)
 * pFrmIds angelegt wird, werden nur noch die enthaltenen FrmIds aufgezeichnet.
 * --------------------------------------------------*/

sal_Bool SwImplProtocol::InsertFrm( sal_uInt16 nId )
{
    if( !pFrmIds )
        pFrmIds = new std::set<sal_uInt16>;
    if( pFrmIds->count( nId ) )
        return sal_False;
    pFrmIds->insert( nId );
    return sal_True;
}

/* --------------------------------------------------
 * SwImplProtocol::DeleteFrm(..) entfernt eine FrmId aus dem pFrmIds-Array,
 * so dass diese Frame nicht mehr aufgezeichnet wird.
 * --------------------------------------------------*/
sal_Bool SwImplProtocol::DeleteFrm( sal_uInt16 nId )
{
    if( !pFrmIds )
        return sal_False;
    if ( pFrmIds->erase(nId) )
        return sal_True;
    return sal_False;
}

/*--------------------------------------------------
 * SwProtocol::SnapShot(..)
 * creates a snapshot of the given frame and its content.
 * --------------------------------------------------*/
void SwImplProtocol::SnapShot( const SwFrm* pFrm, sal_uLong nFlags )
{
    while( pFrm )
    {
        _Record( pFrm, PROT_SNAPSHOT, 0, 0);
        if( pFrm->GetDrawObjs() && nFlags & SNAP_FLYFRAMES )
        {
            aLayer.append(RTL_CONSTASCII_STRINGPARAM("[ "));
            const SwSortedObjs &rObjs = *pFrm->GetDrawObjs();
            for ( sal_uInt16 i = 0; i < rObjs.Count(); ++i )
            {
                SwAnchoredObject* pObj = rObjs[i];
                if ( pObj->ISA(SwFlyFrm) )
                    SnapShot( static_cast<SwFlyFrm*>(pObj), nFlags );
            }
            if (aLayer.getLength() > 1)
                aLayer.remove(aLayer.getLength() - 2, aLayer.getLength());
        }
        if( pFrm->IsLayoutFrm() && nFlags & SNAP_LOWER &&
            ( !pFrm->IsTabFrm() || nFlags & SNAP_TABLECONT ) )
        {
            aLayer.append(RTL_CONSTASCII_STRINGPARAM("  "));
            SnapShot( ((SwLayoutFrm*)pFrm)->Lower(), nFlags );
            if (aLayer.getLength() > 1)
                aLayer.remove(aLayer.getLength() - 2, aLayer.getLength());
        }
        pFrm = pFrm->GetNext();
    }
}

/* --------------------------------------------------
 * SwEnterLeave::Ctor(..) wird vom eigentlichen (inline-)Kontruktor gerufen,
 * wenn die Funktion aufgezeichnet werden soll.
 * Die Aufgabe ist es abhaengig von der Funktion das richtige SwImplEnterLeave-Objekt
 * zu erzeugen, alles weitere geschieht dann in dessen Ctor/Dtor.
 * --------------------------------------------------*/
void SwEnterLeave::Ctor( const SwFrm* pFrm, sal_uLong nFunc, sal_uLong nAct, void* pPar )
{
    switch( nFunc )
    {
        case PROT_ADJUSTN :
        case PROT_GROW:
        case PROT_SHRINK : pImpl = new SwSizeEnterLeave( pFrm, nFunc, nAct, pPar ); break;
        case PROT_MOVE_FWD:
        case PROT_MOVE_BWD : pImpl = new SwUpperEnterLeave( pFrm, nFunc, nAct, pPar ); break;
        case PROT_FRMCHANGES : pImpl = new SwFrmChangesLeave( pFrm, nFunc, nAct, pPar ); break;
        default: pImpl = new SwImplEnterLeave( pFrm, nFunc, nAct, pPar ); break;
    }
    pImpl->Enter();
}

/* --------------------------------------------------
 * SwEnterLeave::Dtor() ruft lediglich den Destruktor des SwImplEnterLeave-Objekts,
 * ist nur deshalb nicht inline, damit die SwImplEnterLeave-Definition nicht
 * im dbg_lay.hxx zu stehen braucht.
 * --------------------------------------------------*/

void SwEnterLeave::Dtor()
{
    if( pImpl )
    {
        pImpl->Leave();
        delete pImpl;
    }
}

void SwImplEnterLeave::Enter()
{
    SwProtocol::Record( pFrm, nFunction, ACT_START, pParam );
}

void SwImplEnterLeave::Leave()
{
    SwProtocol::Record( pFrm, nFunction, ACT_END, pParam );
}

void SwSizeEnterLeave::Leave()
{
    nFrmHeight = pFrm->Frm().Height() - nFrmHeight;
    SwProtocol::Record( pFrm, nFunction, ACT_END, &nFrmHeight );
}

void SwUpperEnterLeave::Enter()
{
    nFrmId = pFrm->GetUpper() ? sal_uInt16(lcl_GetFrameId( pFrm->GetUpper() )) : 0;
    SwProtocol::Record( pFrm, nFunction, ACT_START, &nFrmId );
}

void SwUpperEnterLeave::Leave()
{
    nFrmId = pFrm->GetUpper() ? sal_uInt16(lcl_GetFrameId( pFrm->GetUpper() )) : 0;
    SwProtocol::Record( pFrm, nFunction, ACT_END, &nFrmId );
}

void SwFrmChangesLeave::Enter()
{
}

void SwFrmChangesLeave::Leave()
{
    if( pFrm->Frm() != aFrm )
        SwProtocol::Record( pFrm, PROT_FRMCHANGES, 0, &aFrm );
}

#endif // DBG_UTIL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
