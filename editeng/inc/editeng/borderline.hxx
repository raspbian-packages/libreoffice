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

#ifndef SVX_BORDERLINE_HXX
#define SVX_BORDERLINE_HXX

#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>
#include <svtools/ctrlbox.hxx>

// Line defaults in twips (former Writer defaults):

#define DEF_LINE_WIDTH_0        1
#define DEF_LINE_WIDTH_1        20
#define DEF_LINE_WIDTH_2        50
#define DEF_LINE_WIDTH_3        80
#define DEF_LINE_WIDTH_4        100
#define DEF_LINE_WIDTH_5        10

// ============================================================================

namespace editeng {

    enum SvxBorderStyle
    {
        SOLID,
        DOTTED,
        DASHED,
        DOUBLE,
        THINTHICK_SMALLGAP,
        THINTHICK_MEDIUMGAP,
        THINTHICK_LARGEGAP,
        THICKTHIN_SMALLGAP,
        THICKTHIN_MEDIUMGAP,
        THICKTHIN_LARGEGAP,
        EMBOSSED,
        ENGRAVED,
        OUTSET,
        INSET,
        NO_STYLE = -1
    };

    class EDITENG_DLLPUBLIC SvxBorderLine
    {
    protected:
        Color  aColor;

        long m_nWidth;
        bool m_bMirrorWidths;
        BorderWidthImpl m_aWidthImpl;
        long m_nMult;
        long m_nDiv;

        SvxBorderStyle   m_nStyle;
        sal_uInt16 nOutWidth;
        sal_uInt16 nInWidth;
        sal_uInt16 nDistance;

        bool             m_bUseLeftTop;
        Color            (*m_pColorOutFn)( Color );
        Color            (*m_pColorInFn)( Color );
        Color            (*m_pColorGapFn)( Color );

    public:
        SvxBorderLine( const Color *pCol = 0,
                long nWidth = 0, SvxBorderStyle nStyle = SOLID,
                bool bUseLeftTop = false,
                Color (*pColorOutFn)( Color ) = &darkColor,
                Color (*pColorInFn)( Color ) = &darkColor,
                Color (*pColorGapFn)( Color ) = NULL );
        SvxBorderLine( const SvxBorderLine& r );

        SvxBorderLine& operator=( const SvxBorderLine& r );

        const Color&    GetColor() const { return aColor; }
        Color           GetColorOut( bool bLeftOrTop = true ) const;
        Color           GetColorIn( bool bLeftOrTop = true ) const;
        bool            HasGapColor() const { return m_pColorGapFn != NULL; }
        Color           GetColorGap() const;

        void            SetWidth( long nWidth = 0 ) { m_nWidth = nWidth; }
        /** Guess the style and width from the three lines widths values.

            When the value of nStyle is SvxBorderLine::DOUBLE, the style set will be guessed
            using the three values to match the best possible style among the following:
                - SvxBorderLine::DOUBLE
                - SvxBorderLine::THINTHICK_SMALLGAP
                - SvxBorderLine::THINTHICK_MEDIUMGAP
                - SvxBorderLine::THINTHICK_LARGEGAP
                - SvxBorderLine::THICKTHIN_SMALLGAP
                - SvxBorderLine::THICKTHIN_MEDIUMGAP
                - SvxBorderLine::THICKTHIN_LARGEGAP

            If no styles matches the width, then the width is set to 0.

            There is one known case that could fit several styles: \a nIn = \a nDist = 0.75 pt,
            \a nOut = 1.5 pt. This case fits SvxBorderLine::THINTHICK_SMALLGAP and
            SvxBorderLine::THINTHICK_MEDIUMGAP with a 1.5 pt width and
            SvxBorderLine::THINTHICK_LARGEGAP with a 0.75 pt width. The same case happens
            also for thick-thin styles.

            \param nStyle the border style used to guess the width.
            \param nIn the width of the inner line in 1th pt
            \param nOut the width of the outer line in 1th pt
            \param nDist the width of the gap between the lines in 1th pt
         */
        void            GuessLinesWidths( SvxBorderStyle nStyle, sal_uInt16 nOut, sal_uInt16 nIn = 0, sal_uInt16 nDist = 0 );

        // TODO Hacky method to mirror lines in only a few cases
        void            SetMirrorWidths( bool bMirror = true ) { m_bMirrorWidths = bMirror; }
        long            GetWidth( ) const { return m_nWidth; }
        sal_uInt16      GetOutWidth() const;
        sal_uInt16      GetInWidth() const;
        sal_uInt16      GetDistance() const;

        SvxBorderStyle  GetStyle() const { return m_nStyle; }

        void            SetColor( const Color &rColor ) { aColor = rColor; }
        void            SetColorOutFn( Color (*pColorOutFn)( Color ) ) { m_pColorOutFn = pColorOutFn; }
        void            SetColorInFn( Color (*pColorInFn)( Color ) ) { m_pColorInFn = pColorInFn; }
        void            SetColorGapFn( Color (*pColorGapFn)( Color ) ) { m_pColorGapFn = pColorGapFn; }
        void            SetUseLeftTop( bool bUseLeftTop ) { m_bUseLeftTop = bUseLeftTop; }
        void            SetStyle( SvxBorderStyle nNew );
        void            ScaleMetrics( long nMult, long nDiv );

        sal_Bool            operator==( const SvxBorderLine &rCmp ) const;

        String          GetValueString( SfxMapUnit eSrcUnit, SfxMapUnit eDestUnit,
                                        const IntlWrapper* pIntl,
                                        sal_Bool bMetricStr = sal_False ) const;

        bool            HasPriority( const SvxBorderLine& rOtherLine ) const;

        bool isEmpty() const { return m_aWidthImpl.IsEmpty( ) || m_nStyle == NO_STYLE || m_nWidth == 0; }
        bool isDouble() const { return m_aWidthImpl.IsDouble(); }
        sal_uInt16 GetScaledWidth() const { return GetOutWidth() + GetInWidth() + GetDistance(); }

        static Color darkColor( Color aMain );
        static Color lightColor( Color aMain );

        static Color threeDLightColor( Color aMain );
        static Color threeDMediumColor( Color aMain );
        static Color threeDDarkColor( Color aMain );

        static BorderWidthImpl getWidthImpl( SvxBorderStyle nStyle );
    };

// ============================================================================

} // namespace editeng

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
