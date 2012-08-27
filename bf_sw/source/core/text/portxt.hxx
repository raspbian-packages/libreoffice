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
#ifndef _PORTXT_HXX
#define _PORTXT_HXX

#ifdef GCC
#include <sys/types.h>
#else
#include <new.h>    //fuer size_t, FIXEDMEM aus tools
#endif
#include <tools/mempool.hxx>

#include "porlin.hxx"
/*N*/ #include <tools/debug.hxx> //for stripping
namespace binfilter {
class SwTxtGuess;

/*************************************************************************
 *                      class SwTxtPortion
 *************************************************************************/

class SwTxtPortion : public SwLinePortion
{
    void BreakCut( SwTxtFormatInfo &rInf, const SwTxtGuess &rGuess );
    void BreakUnderflow( SwTxtFormatInfo &rInf );
    sal_Bool _Format( SwTxtFormatInfo &rInf );

public:
    inline SwTxtPortion(){ SetWhichPor( POR_TXT ); }
    SwTxtPortion( const SwLinePortion &rPortion );
    virtual void Paint( const SwTxtPaintInfo& /*rInf*/ ) const { DBG_BF_ASSERT(0, "STRIP"); }
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo& , XubString& ) const { DBG_BF_ASSERT(0, "STRIP"); return sal_False; }

    // zaehlt die Spaces fuer Blocksatz
    xub_StrLen GetSpaceCnt( const SwTxtSizeInfo &rInf, xub_StrLen& rCnt ) const;

    sal_Bool CreateHyphen( SwTxtFormatInfo &rInf, SwTxtGuess &rGuess );

    // Accessibility: pass information about this portion to the PortionHandler

     OUTPUT_OPERATOR
    DECL_FIXEDMEMPOOL_NEWDEL(SwTxtPortion)
};

/*************************************************************************
 *                      class SwHolePortion
 *************************************************************************/

class SwHolePortion : public SwLinePortion
{
    KSHORT nBlankWidth;
public:
            SwHolePortion( const SwTxtPortion &rPor );
    inline KSHORT GetBlankWidth( ) const { return nBlankWidth; }
    inline void SetBlankWidth( const KSHORT nNew ) { nBlankWidth = nNew; }
    virtual SwLinePortion *Compress();
    virtual void Paint( const SwTxtPaintInfo& /*rInf*/ ) const { DBG_BF_ASSERT(0, "STRIP"); }

    // Accessibility: pass information about this portion to the PortionHandler

    OUTPUT_OPERATOR
    DECL_FIXEDMEMPOOL_NEWDEL(SwHolePortion)
};

CLASSIO( SwTxtPortion )
CLASSIO( SwHolePortion )


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
