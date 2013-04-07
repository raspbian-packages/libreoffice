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
#ifndef SDTAKITM_HXX
#define SDTAKITM_HXX

#include <svl/eitem.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

//------------------------------
// class SdrTextAniKindItem
//------------------------------

enum SdrTextAniKind {SDRTEXTANI_NONE,
                     SDRTEXTANI_BLINK,
                     SDRTEXTANI_SCROLL,
                     SDRTEXTANI_ALTERNATE,
                     SDRTEXTANI_SLIDE};

// - SDRTEXTANI_BLINK:
//   Einfach nur Blinken. Direction und Amount ohne Wirkung.
//   Frequenz siehe Delay, 0=0.5Hz (Delay=250).
//   Count=Anzahl der blinker. 0=Endlos.
//   StartInside: sal_False=beginnend mit Pause, sal_True=Beginnend mit Puls
//   StopInside:  sal_False=Nach Count blinkern nicht sichtbar, sal_True=sichtbar
//                (nur wenn Count!=0)
// - SDRTEXTANI_SCROLL:
//   Schrift laeuft rein, laeuft vollstaendig durch. Wenn ganz verschwunden
//   geht's wieder von vorne los.
//   Delay in ms, Sonderfall Delay=0 wird auf 50ms defaulted (20Hz)
//   Count=Durchlaufanzahl (0=Endlos)
//   Direction: ist eben die Richtung in die gescrollt wird.
//   StartInside: sal_False=Schrift wird auch beim ersten Durchlauf reingescrollt
//                sal_True=Der linke Teil der Schrift steht beim ersten Durchlauf
//                     bereits da (im sichtbaren "Scrollbereich")
//   StopInside:  sal_False=Schrift wird auch beim letzten Durchlauf vollstaendig
//                      rausgescrollt
//                sal_True=Schrift wird beim letzten Durchlauf nicht rausgescrollt
//                (nur wenn Count!=0)
//   Amount: Schrittweite in logischen Einheiten. Negative Werte bedeuten
//           Pixel statt log. Einheiten. Wenn Amount=0, dann wird auf
//           1 Pixel defaulted.
// - SDRTEXTANI_ALTERNATE:
//   Wie SDRTEXTANI_SCROLL, nur wird nicht gescrollt bis Schrift ganz
//   verschwunden, sondern bis der letzte Teil vollstaendig sichtbar ist. Dann
//   wird die Richtung umgekehrt und zuruekgescrollt.
//   Count=Anzahl der Durchlaeufe=Anzahl der Richtungswechsel-1
//   Bei Count=1 ist SDRTEXTANI_ALTERNATE also identisch mit SDRTEXTANI_SCROLL
//   Direction ist die Startrichtung
//   Alle andern Parameter wie SDRTEXTANI_SCROLL
// - SDRTEXTANI_SLIDE:
//   Schrift wird nur reingeschoben bis zur Originalposition.
//   -> also wie SCROLL mit StartInside=sal_False, StopInside=TRUE
//   und Count=1. Count=0 wird als Count=1 interpretiert.
//   Fuer jeden Count>1 wird der Text jedoch ruekwaerts wieder vollstaendig
//   rausgeschoben (aehnlich wie ALTERNATE) und dann wieder reingeschoben.
//   StopInside wird nicht ausgewertet, weil immer Inside gestoppt wird.
//   StartInside wird nicht ausgewertet, weil immer Outside gestartet wird.
//   Alle andern Parameter wie SDRTEXTANI_SCROLL
// StartInside,StopInside: Bei sal_True ist die anfaengliche/entgueltige
// Textposition abhaengig von der Textverankerung am Zeichenobjekt. Sie
// entspricht der Textposition bei normalem Paint (ohne Laufschrift).

class SVX_DLLPUBLIC SdrTextAniKindItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrTextAniKindItem(SdrTextAniKind eKind=SDRTEXTANI_NONE): SfxEnumItem(SDRATTR_TEXT_ANIKIND,(sal_uInt16)eKind) {}
    SdrTextAniKindItem(SvStream& rIn)                       : SfxEnumItem(SDRATTR_TEXT_ANIKIND,rIn)  {}
    virtual SfxPoolItem*      Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*      Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual sal_uInt16            GetValueCount() const; // { return 5; }
            SdrTextAniKind GetValue() const      { return (SdrTextAniKind)SfxEnumItem::GetValue(); }

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual String  GetValueTextByPos(sal_uInt16 nPos) const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
