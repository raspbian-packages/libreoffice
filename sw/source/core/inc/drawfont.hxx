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
#ifndef _DRAWFONT_HXX
#define _DRAWFONT_HXX

#include <tools/solar.h>
#include <tools/string.hxx>

class SwTxtFrm;
class OutputDevice;
class ViewShell;
class SwScriptInfo;
class Point;
class SwWrongList;
class Size;
class SwFont;
class Font;
class SwUnderlineFont;

/*************************************************************************
 *                      class SwDrawTextInfo
 *
 * encapsulates information for drawing text
 *************************************************************************/

class SwDrawTextInfo
{
    const SwTxtFrm* pFrm;
    OutputDevice* pOut;
    ViewShell* pSh;
    const SwScriptInfo* pScriptInfo;
    const Point* pPos;
    const XubString* pText;
    const SwWrongList* pWrong;
    const SwWrongList* pGrammarCheck;
    const SwWrongList* pSmartTags; // SMARTTAGS
    const Size* pSize;
    SwFont *pFnt;
    SwUnderlineFont* pUnderFnt;
    xub_StrLen* pHyphPos;
    long nLeft;
    long nRight;
    long nKanaDiff;
    xub_StrLen nIdx;
    xub_StrLen nLen;
    xub_StrLen nOfst;
    sal_uInt16 nWidth;
    sal_uInt16 nAscent;
    sal_uInt16 nCompress;
    long nSperren;
    long nSpace;
    long nKern;
    xub_StrLen nNumberOfBlanks;
    sal_uInt8 nCursorBidiLevel;
    sal_Bool bBullet : 1;
    sal_Bool bUpper : 1;        // Fuer Kapitaelchen: Grossbuchstaben-Flag
    sal_Bool bDrawSpace : 1;    // Fuer Kapitaelchen: Unter/Durchstreichung
    sal_Bool bGreyWave  : 1;    // Graue Wellenlinie beim extended TextInput
    sal_Bool bSpaceStop : 1;    // For underlining we need to know, if a portion
                            // is right in front of a hole portion or a
                            // fix margin portion.
    sal_Bool bSnapToGrid : 1;   // Does paragraph snap to grid?
    sal_Bool bIgnoreFrmRTL : 1; // Paint text as if text has LTR direction, used for
                            // line numbering
    sal_Bool bPosMatchesBounds :1;  // GetCrsrOfst should not return the next
                                // position if screen position is inside second
                                // half of bound rect, used for Accessibility

    SwDrawTextInfo();       // nicht zulaessig
public:

#ifdef DBG_UTIL
    bool m_bPos   : 1;    // These flags should control that the appropriate
    bool m_bWrong : 1;    // Set-function has been called before calling
    bool m_bGrammarCheck : 1;     //  the Get-function of a member
    bool m_bSize  : 1;
    bool m_bFnt   : 1;
    bool m_bHyph  : 1;
    bool m_bLeft  : 1;
    bool m_bRight : 1;
    bool m_bKana  : 1;
    bool m_bOfst  : 1;
    bool m_bAscent: 1;
    bool m_bSperr : 1;
    bool m_bSpace : 1;
    bool m_bNumberOfBlanks : 1;
    bool m_bUppr  : 1;
    bool m_bDrawSp: 1;
#endif

    SwDrawTextInfo( ViewShell *pS, OutputDevice &rO, const SwScriptInfo* pSI,
                    const XubString &rSt, xub_StrLen nI, xub_StrLen nL,
                    sal_uInt16 nW = 0, sal_Bool bB = sal_False )
    {
        pFrm = NULL;
        pSh = pS;
        pOut = &rO;
        pScriptInfo = pSI;
        pText = &rSt;
        nIdx = nI;
        nLen = nL;
        nKern = 0;
        nCompress = 0;
        nWidth = nW;
        nNumberOfBlanks = 0;
        nCursorBidiLevel = 0;
        bBullet = bB;
        pUnderFnt = 0;
        bGreyWave = sal_False;
        bSpaceStop = sal_False;
        bSnapToGrid = sal_False;
        bIgnoreFrmRTL = sal_False;
        bPosMatchesBounds = sal_False;

        // These values are initialized but, they have to be
        // set explicitly via their Set-function before they may
        // be accessed by their Get-function:
        pPos = 0;
        pWrong = 0;
        pGrammarCheck = 0;
        pSmartTags = 0;
        pSize = 0;
        pFnt = 0;
        pHyphPos = 0;
        nLeft = 0;
        nRight = 0;
        nKanaDiff = 0;
        nOfst = 0;
        nAscent = 0;
        nSperren = 0;
        nSpace = 0;
        bUpper = sal_False;
        bDrawSpace = sal_False;

#ifdef DBG_UTIL
        // these flags control, whether the matching member variables have
        // been set by using the Set-function before they may be accessed
        // by their Get-function:
        m_bPos = m_bWrong = m_bGrammarCheck = m_bSize = m_bFnt = m_bAscent =
        m_bSpace = m_bNumberOfBlanks = m_bUppr =
        m_bDrawSp = m_bLeft = m_bRight = m_bKana = m_bOfst = m_bHyph =
        m_bSperr = false;
#endif
    }

    const SwTxtFrm* GetFrm() const
    {
        return pFrm;
    }

    void SetFrm( const SwTxtFrm* pNewFrm )
    {
        pFrm = pNewFrm;
    }

    ViewShell *GetShell() const
    {
        return pSh;
    }

    OutputDevice& GetOut() const
    {
        return *pOut;
    }

    OutputDevice *GetpOut() const
    {
        return pOut;
    }

    const SwScriptInfo* GetScriptInfo() const
    {
        return pScriptInfo;
    }

    const Point &GetPos() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bPos, "DrawTextInfo: Undefined Position" );
#endif
        return *pPos;
    }

    xub_StrLen *GetHyphPos() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bHyph, "DrawTextInfo: Undefined Hyph Position" );
#endif
        return pHyphPos;
    }

    const XubString &GetText() const
    {
        return *pText;
    }

    const SwWrongList* GetWrong() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bWrong, "DrawTextInfo: Undefined WrongList" );
#endif
        return pWrong;
    }

    const SwWrongList* GetGrammarCheck() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bGrammarCheck, "DrawTextInfo: Undefined GrammarCheck List" );
#endif
        return pGrammarCheck;
    }

    const SwWrongList* GetSmartTags() const
    {
        return pSmartTags;
    }

    const Size &GetSize() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bSize, "DrawTextInfo: Undefined Size" );
#endif
        return *pSize;
    }

    SwFont* GetFont() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bFnt, "DrawTextInfo: Undefined Font" );
#endif
        return pFnt;
    }

    SwUnderlineFont* GetUnderFnt() const
    {
        return pUnderFnt;
    }

    xub_StrLen GetIdx() const
    {
        return nIdx;
    }

    xub_StrLen GetLen() const
    {
        return nLen;
    }

    xub_StrLen GetOfst() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bOfst, "DrawTextInfo: Undefined Offset" );
#endif
        return nOfst;
    }

    xub_StrLen GetEnd() const
    {
        return nIdx + nLen;
    }

    long GetLeft() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bLeft, "DrawTextInfo: Undefined left range" );
#endif
        return nLeft;
    }

    long GetRight() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bRight, "DrawTextInfo: Undefined right range" );
#endif
        return nRight;
    }

    long GetKanaDiff() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bKana, "DrawTextInfo: Undefined kana difference" );
#endif
        return nKanaDiff;
    }

    sal_uInt16 GetWidth() const
    {
        return nWidth;
    }

    sal_uInt16 GetAscent() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bAscent, "DrawTextInfo: Undefined Ascent" );
#endif
        return nAscent;
    }

    sal_uInt16 GetKanaComp() const
    {
        return nCompress;
    }

    long GetSperren() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bSperr, "DrawTextInfo: Undefined >Sperren<" );
#endif
        return nSperren;
    }

    long GetKern() const
    {
        return nKern;
    }

    long GetSpace() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bSpace, "DrawTextInfo: Undefined Spacing" );
#endif
        return nSpace;
    }

    xub_StrLen GetNumberOfBlanks() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bNumberOfBlanks, "DrawTextInfo::Undefined NumberOfBlanks" );
#endif
        return nNumberOfBlanks;
    }

    sal_uInt8 GetCursorBidiLevel() const
    {
        return nCursorBidiLevel;
    }

    sal_Bool GetBullet() const
    {
        return bBullet;
    }

    sal_Bool GetUpper() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bUppr, "DrawTextInfo: Undefined Upperflag" );
#endif
        return bUpper;
    }

    sal_Bool GetDrawSpace() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bDrawSp, "DrawTextInfo: Undefined DrawSpaceflag" );
#endif
        return bDrawSpace;
    }

    sal_Bool GetGreyWave() const
    {
        return bGreyWave;
    }

    sal_Bool IsSpaceStop() const
    {
        return bSpaceStop;
    }

    sal_Bool SnapToGrid() const
    {
        return bSnapToGrid;
    }

    sal_Bool IsIgnoreFrmRTL() const
    {
        return bIgnoreFrmRTL;
    }

    sal_Bool IsPosMatchesBounds() const
    {
        return bPosMatchesBounds;
    }

    void SetOut( OutputDevice &rNew )
    {
        pOut = &rNew;
    }

    void SetPos( const Point &rNew )
    {
        pPos = &rNew;
#ifdef DBG_UTIL
        m_bPos = true;
#endif
    }

    void SetHyphPos( xub_StrLen *pNew )
    {
        pHyphPos = pNew;
#ifdef DBG_UTIL
        m_bHyph = true;
#endif
    }

    void SetText( const XubString &rNew )
    {
        pText = &rNew;
    }

    void SetWrong( const SwWrongList* pNew )
    {
        pWrong = pNew;
#ifdef DBG_UTIL
        m_bWrong = true;
#endif
    }

    void SetGrammarCheck( const SwWrongList* pNew )
    {
        pGrammarCheck = pNew;
#ifdef DBG_UTIL
        m_bGrammarCheck = true;
#endif
    }

    void SetSmartTags( const SwWrongList* pNew )
    {
        pSmartTags = pNew;
    }

    void SetSize( const Size &rNew )
    {
        pSize = &rNew;
#ifdef DBG_UTIL
        m_bSize = true;
#endif
    }

    void SetFont( SwFont* pNew )
    {
        pFnt = pNew;
#ifdef DBG_UTIL
        m_bFnt = sal_True;
#endif
    }

    void SetIdx( xub_StrLen nNew )
    {
        nIdx = nNew;
    }

    void SetLen( xub_StrLen nNew )
    {
        nLen = nNew;
    }

    void SetOfst( xub_StrLen nNew )
    {
        nOfst = nNew;
#ifdef DBG_UTIL
        m_bOfst = true;
#endif
    }

    void SetLeft( long nNew )
    {
        nLeft = nNew;
#ifdef DBG_UTIL
        m_bLeft = true;
#endif
    }

    void SetRight( long nNew )
    {
        nRight = nNew;
#ifdef DBG_UTIL
        m_bRight = true;
#endif
    }

    void SetKanaDiff( long nNew )
    {
        nKanaDiff = nNew;
#ifdef DBG_UTIL
        m_bKana = true;
#endif
    }

    void SetWidth( sal_uInt16 nNew )
    {
        nWidth = nNew;
    }

    void SetAscent( sal_uInt16 nNew )
    {
        nAscent = nNew;
#ifdef DBG_UTIL
        m_bAscent = true;
#endif
    }

    void SetKern( long nNew )
    {
        nKern = nNew;
    }

    void SetSpace( long nNew )
    {
        if( nNew < 0 )
        {
            nSperren = -nNew;
            nSpace = 0;
        }
        else
        {
            nSpace = nNew;
            nSperren = 0;
        }
#ifdef DBG_UTIL
        m_bSpace = true;
        m_bSperr = true;
#endif
    }

    void SetNumberOfBlanks( xub_StrLen nNew )
    {
#ifdef DBG_UTIL
        m_bNumberOfBlanks = true;
#endif
        nNumberOfBlanks = nNew;
    }

    void SetCursorBidiLevel( sal_uInt8 nNew )
    {
        nCursorBidiLevel = nNew;
    }

    void SetKanaComp( short nNew )
    {
        nCompress = nNew;
    }

    void SetBullet( sal_Bool bNew )
    {
        bBullet = bNew;
    }

    void SetUnderFnt( SwUnderlineFont* pULFnt )
    {
        pUnderFnt = pULFnt;
    }

    void SetUpper( sal_Bool bNew )
    {
        bUpper = bNew;
#ifdef DBG_UTIL
        m_bUppr = true;
#endif
    }

    void SetDrawSpace( sal_Bool bNew )
    {
        bDrawSpace = bNew;
#ifdef DBG_UTIL
        m_bDrawSp = true;
#endif
    }

    void SetGreyWave( sal_Bool bNew )
    {
        bGreyWave = bNew;
    }

    void SetSpaceStop( sal_Bool bNew )
    {
        bSpaceStop = bNew;
    }

    void SetSnapToGrid( sal_Bool bNew )
    {
        bSnapToGrid = bNew;
    }

    void SetIgnoreFrmRTL( sal_Bool bNew )
    {
        bIgnoreFrmRTL = bNew;
    }

    void SetPosMatchesBounds( sal_Bool bNew )
    {
        bPosMatchesBounds = bNew;
    }

    void Shift( sal_uInt16 nDir );

    // sets a new color at the output device if necessary
    // if a font is passed as argument, the change if made to the font
    // otherwise the font at the output device is changed
    // returns if the font has been changed
    sal_Bool ApplyAutoColor( Font* pFnt = 0 );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
