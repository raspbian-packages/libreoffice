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
#ifndef _FRMTOOL_HXX
#define _FRMTOOL_HXX
#include "swtypes.hxx"
#include "layfrm.hxx"
#include "frmatr.hxx"
#include "swcache.hxx"
// #i96772#
#include <editeng/lrspitem.hxx>

class SwPageFrm;
class SwFlyFrm;
class SwCntntFrm;
class SwRootFrm;
class SwDoc;
class SwAttrSet;
class SdrObject;
class BigInt;
class SvxBrushItem;
class SdrMarkList;
class SwNodeIndex;
class OutputDevice;
class SwPageDesc;
class SwTableBox;
struct SwFindRowSpanCacheObj;

#if defined(MSC)
#define MA_FASTCALL __fastcall
#else
#define MA_FASTCALL
#endif

#define WEIT_WECH       LONG_MAX - 20000        //Initale Position der Flys.
#define BROWSE_HEIGHT   56700L * 10L               //10 Meter

#define GRFNUM_NO 0
#define GRFNUM_YES 1
#define GRFNUM_REPLACE 2

//Painten des Hintergrunds. Mit Brush oder Graphic.
// - add 6th parameter to indicate that method should
//     consider background transparency, saved in the color of the brush item
void MA_FASTCALL DrawGraphic( const SvxBrushItem *, OutputDevice *,
      const SwRect &rOrg, const SwRect &rOut, const sal_uInt8 nGrfNum = GRFNUM_NO,
      const sal_Bool bConsiderBackgroundTransparency = sal_False );

// - method to align rectangle
// Created declaration here to avoid <extern> declarations
void MA_FASTCALL SwAlignRect( SwRect &rRect, const ViewShell *pSh );

// - method to align graphic rectangle
// Created declaration here to avoid <extern> declarations
void SwAlignGrfRect( SwRect *pGrfRect, const OutputDevice &rOut );

//Fly besorgen, wenn keine List hineingereicht wird, wir die der aktuellen
//Shell benutzt.
//Implementierung in feshview.cxx
SwFlyFrm *GetFlyFromMarked( const SdrMarkList *pLst, ViewShell *pSh );

//Nicht gleich die math.lib anziehen.
sal_uLong MA_FASTCALL SqRt( BigInt nX );

SwFrm *SaveCntnt( SwLayoutFrm *pLay, SwFrm *pStart );
void RestoreCntnt( SwFrm *pSav, SwLayoutFrm *pParent, SwFrm *pSibling, bool bGrow );

//CntntNodes besorgen, CntntFrms erzeugen und in den LayFrm haengen.
void MA_FASTCALL _InsertCnt( SwLayoutFrm *pLay, SwDoc *pDoc, sal_uLong nIndex,
                 sal_Bool bPages = sal_False, sal_uLong nEndIndex = 0,
                 SwFrm *pPrv = 0 );

//Erzeugen der Frames fuer einen bestimmten Bereich, verwendet _InsertCnt
void MakeFrms( SwDoc *pDoc, const SwNodeIndex &rSttIdx,
                            const SwNodeIndex &rEndIdx );

//Um z.B. fuer Tabelleheadlines das Erzeugen der Flys in _InsertCnt zu unterbinden.
extern sal_Bool bDontCreateObjects;

//Fuer FlyCnts, siehe SwFlyAtCntFrm::MakeAll()
extern sal_Bool bSetCompletePaintOnInvalidate;

//Fuer Tabelleneinstellung per Tastatur.
long MA_FASTCALL CalcRowRstHeight( SwLayoutFrm *pRow );
long MA_FASTCALL CalcHeightWidthFlys( const SwFrm *pFrm );  //MA_FLY_HEIGHT

//Neue Seite einsetzen
SwPageFrm * MA_FASTCALL InsertNewPage( SwPageDesc &rDesc, SwFrm *pUpper,
                          sal_Bool bOdd, sal_Bool bInsertEmpty, sal_Bool bFtn,
                          SwFrm *pSibling );

//Flys bei der Seite anmelden.
void RegistFlys( SwPageFrm*, const SwLayoutFrm* );

//Benachrichtung des Fly Hintergrundes wenn Notwendig.
void Notify( SwFlyFrm *pFly, SwPageFrm *pOld, const SwRect &rOld,
             const SwRect* pOldRect = 0 );

void Notify_Background( const SdrObject* pObj,
                        SwPageFrm* pPage,
                        const SwRect& rRect,
                        const PrepareHint eHint,
                        const sal_Bool bInva );

const SwFrm* GetVirtualUpper( const SwFrm* pFrm, const Point& rPos );

sal_Bool Is_Lower_Of( const SwFrm *pCurrFrm, const SdrObject* pObj );

const SwFrm *FindKontext( const SwFrm *pFrm, sal_uInt16 nAdditionalKontextTyp );

sal_Bool IsFrmInSameKontext( const SwFrm *pInnerFrm, const SwFrm *pFrm );

const SwFrm * MA_FASTCALL FindPage( const SwRect &rRect, const SwFrm *pPage );

// wird von SwCntntNode::GetFrm und von SwFlyFrm::GetFrm
//              gerufen
SwFrm* GetFrmOfModify( const SwRootFrm* pLayout, SwModify const&, sal_uInt16 const nFrmType, const Point* = 0,
                        const SwPosition *pPos = 0,
                        const sal_Bool bCalcFrm = sal_False );

//Sollen ExtraDaten (Reline-Strich, Zeilennummern) gepaintet werden?
sal_Bool IsExtraData( const SwDoc *pDoc );

// #i11760# - method declaration <CalcCntnt(..)>
void CalcCntnt( SwLayoutFrm *pLay,
                bool bNoColl = false,
                bool bNoCalcFollow = false );


//Die Notify-Klassen merken sich im CTor die aktuellen Groessen und fuehren
//im DTor ggf. die notwendigen Benachrichtigungen durch.

class SwFrmNotify
{
protected:
    SwFrm *pFrm;
    const SwRect aFrm;
    const SwRect aPrt;
    SwTwips mnFlyAnchorOfst;
    SwTwips mnFlyAnchorOfstNoWrap;
    sal_Bool     bHadFollow;
    sal_Bool     bInvaKeep;
    sal_Bool     bValidSize;
protected:
    // #i49383#
    bool mbFrmDeleted;

public:
    SwFrmNotify( SwFrm *pFrm );
    ~SwFrmNotify();

    const SwRect &Frm() const { return aFrm; }
    const SwRect &Prt() const { return aPrt; }
    void SetInvaKeep() { bInvaKeep = sal_True; }
    // #i49383#
    void FrmDeleted()
    {
        mbFrmDeleted = true;
    }
};

class SwLayNotify : public SwFrmNotify
{
    sal_Bool bLowersComplete;

    SwLayoutFrm *GetLay() { return (SwLayoutFrm*)pFrm; }
public:
    SwLayNotify( SwLayoutFrm *pLayFrm );
    ~SwLayNotify();

    void SetLowersComplete( sal_Bool b ) { bLowersComplete = b; }
    sal_Bool IsLowersComplete()          { return bLowersComplete; }
};

class SwFlyNotify : public SwLayNotify
{
    SwPageFrm *pOldPage;
    const SwRect aFrmAndSpace;
    SwFlyFrm *GetFly() { return (SwFlyFrm*)pFrm; }
public:
    SwFlyNotify( SwFlyFrm *pFlyFrm );
    ~SwFlyNotify();

    SwPageFrm *GetOldPage() const { return pOldPage; }
};

class SwCntntNotify : public SwFrmNotify
{
private:
    // #i11859#
    bool    mbChkHeightOfLastLine;
    SwTwips mnHeightOfLastLine;

    // #i25029#
    bool        mbInvalidatePrevPrtArea;
    bool        mbBordersJoinedWithPrev;

    SwCntntFrm *GetCnt() { return (SwCntntFrm*)pFrm; }

public:
    SwCntntNotify( SwCntntFrm *pCntFrm );
    ~SwCntntNotify();

    // #i25029#
    void SetInvalidatePrevPrtArea()
    {
        mbInvalidatePrevPrtArea = true;
    }

    void SetBordersJoinedWithPrev()
    {
        mbBordersJoinedWithPrev = true;
    }
};

//SwBorderAttrs kapselt die Berechnung fuer die Randattribute inclusive
//Umrandung. Die Attribute und die errechneten Werte werden gecached.
//Neu: Die gesammte Klasse wird gecached.

//!!!Achtung: Wenn weitere Attribute gecached werden muss unbedingt die
//Methode Modify::Modify mitgepflegt werden!!!

// - delete old method <SwBorderAttrs::CalcRight()> and
// the stuff that belongs to it.
class SwBorderAttrs : public SwCacheObj
{
    const SwAttrSet      &rAttrSet;
    const SvxULSpaceItem &rUL;
    // #i96772#
    SvxLRSpaceItem rLR;
    const SvxBoxItem     &rBox;
    const SvxShadowItem  &rShadow;
    const Size            aFrmSize;     //Die FrmSize

    sal_Bool bBorderDist    :1;             //Ist's ein Frm der auch ohne Linie
                                        //einen Abstand haben kann?

    //Mit den Folgenden Bools werden die gecache'ten Werte fuer UNgueltig
    //erklaert - bis sie einmal berechnet wurden.
    sal_Bool bTopLine       :1;
    sal_Bool bBottomLine    :1;
    sal_Bool bLeftLine      :1;
    sal_Bool bRightLine     :1;
    sal_Bool bTop           :1;
    sal_Bool bBottom        :1;
    sal_Bool bLine          :1;

    sal_Bool bIsLine        :1; //Umrandung an mind. einer Kante?

    sal_Bool bCacheGetLine        :1; //GetTopLine(), GetBottomLine() cachen?
    sal_Bool bCachedGetTopLine    :1; //GetTopLine() gecached?
    sal_Bool bCachedGetBottomLine :1; //GetBottomLine() gecached?
    // - booleans indicating, if values <bJoinedWithPrev>
    //          and <bJoinedWithNext> are cached and valid.
    //          Caching depends on value of <bCacheGetLine>.
    mutable sal_Bool bCachedJoinedWithPrev :1;
    mutable sal_Bool bCachedJoinedWithNext :1;
    // - booleans indicating, if borders are joined
    //          with previous/next frame.
    sal_Bool bJoinedWithPrev :1;
    sal_Bool bJoinedWithNext :1;

    //Die gecache'ten Werte, undefiniert bis sie einmal berechnet wurden.
    sal_uInt16 nTopLine,
           nBottomLine,
           nLeftLine,
           nRightLine,
           nTop,
           nBottom,
           nGetTopLine,
           nGetBottomLine;

    //Nur die Lines + Shadow errechnen.
    void _CalcTopLine();
    void _CalcBottomLine();
    void _CalcLeftLine();
    void _CalcRightLine();

    //Lines + Shadow + Abstaende
    void _CalcTop();
    void _CalcBottom();

    void _IsLine();

    // #i25029# - add optional 2nd parameter <_pPrevFrm>
    // If set, its value is taken for testing, if borders/shadow have to joined
    // with previous frame.
    void _GetTopLine   ( const SwFrm& _rFrm,
                         const SwFrm* _pPrevFrm = 0L );
    void _GetBottomLine( const SwFrm& _rFrm );

    // - private methods to calculate cached values
    // <bJoinedWithPrev> and <bJoinedWithNext>.
    // #i25029# - add optional 2nd parameter <_pPrevFrm>
    // If set, its value is taken for testing, if borders/shadow have to joined
    // with previous frame.
    void _CalcJoinedWithPrev( const SwFrm& _rFrm,
                              const SwFrm* _pPrevFrm = 0L );
    void _CalcJoinedWithNext( const SwFrm& _rFrm );

    // - internal helper method for methods
    // <_CalcJoinedWithPrev> and <_CalcJoinedWithNext>.
    sal_Bool _JoinWithCmp( const SwFrm& _rCallerFrm,
                       const SwFrm& _rCmpFrm ) const;

     //Rechte und linke Linie sowie LRSpace gleich?
    // - change name of 1st parameter - "rAttrs" -> "rCmpAttrs".
    sal_Bool CmpLeftRight( const SwBorderAttrs &rCmpAttrs,
                       const SwFrm *pCaller,
                       const SwFrm *pCmp ) const;

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwBorderAttrs)

    SwBorderAttrs( const SwModify *pOwner, const SwFrm *pConstructor );
    ~SwBorderAttrs();

    inline const SwAttrSet      &GetAttrSet() const { return rAttrSet;  }
    inline const SvxULSpaceItem &GetULSpace() const { return rUL;       }
    inline const SvxLRSpaceItem &GetLRSpace() const { return rLR;       }
    inline const SvxBoxItem     &GetBox()     const { return rBox;      }
    inline const SvxShadowItem  &GetShadow()  const { return rShadow;   }

    inline sal_uInt16 CalcTopLine() const;
    inline sal_uInt16 CalcBottomLine() const;
    inline sal_uInt16 CalcLeftLine() const;
    inline sal_uInt16 CalcRightLine() const;
    inline sal_uInt16 CalcTop() const;
    inline sal_uInt16 CalcBottom() const;
           long CalcLeft( const SwFrm *pCaller ) const;
           long CalcRight( const SwFrm *pCaller ) const;

    inline sal_Bool IsLine() const;

    inline const Size &GetSize()     const { return aFrmSize; }

    inline sal_Bool IsBorderDist() const { return bBorderDist; }

    //Sollen obere bzw. untere Umrandung fuer den Frm ausgewertet werden?
    // #i25029# - add optional 2nd parameter <_pPrevFrm>
    // If set, its value is taken for testing, if borders/shadow have to joined
    // with previous frame.
    inline sal_uInt16 GetTopLine   ( const SwFrm& _rFrm,
                                 const SwFrm* _pPrevFrm = 0L ) const;
    inline sal_uInt16 GetBottomLine( const SwFrm& _rFrm ) const;
    inline void   SetGetCacheLine( sal_Bool bNew ) const;
    // - accessors for cached values <bJoinedWithPrev>
    // and <bJoinedWithPrev>
    // #i25029# - add optional 2nd parameter <_pPrevFrm>
    // If set, its value is taken for testing, if borders/shadow have to joined
    // with previous frame.
    sal_Bool JoinedWithPrev( const SwFrm& _rFrm,
                         const SwFrm* _pPrevFrm = 0L ) const;
    sal_Bool JoinedWithNext( const SwFrm& _rFrm ) const;
};

class SwBorderAttrAccess : public SwCacheAccess
{
    const SwFrm *pConstructor;      //opt: Zur weitergabe an SwBorderAttrs
protected:
    virtual SwCacheObj *NewObj();

public:
    SwBorderAttrAccess( SwCache &rCache, const SwFrm *pOwner );

    SwBorderAttrs *Get();
};

//---------------------------------------------------------------------
//Iterator fuer die DrawObjecte einer Seite. Die Objecte werden Nach ihrer
//Z-Order iteriert.
//Das iterieren ist nicht eben billig, denn fuer alle Operationen muss jeweils
//ueber das gesamte SortArray iteriert werden.
class SwOrderIter
{
    const SwPageFrm *pPage;
    const SdrObject *pCurrent;
    const sal_Bool bFlysOnly;
public:
    SwOrderIter( const SwPageFrm *pPage, sal_Bool bFlysOnly = sal_True );

    void             Current( const SdrObject *pNew ) { pCurrent = pNew; }
    const SdrObject *Current()    const { return pCurrent; }
    const SdrObject *operator()() const { return pCurrent; }
    const SdrObject *Top();
    const SdrObject *Bottom();
    const SdrObject *Next();
    const SdrObject *Prev();
};


class StackHack
{
    static sal_uInt8 nCnt;
    static sal_Bool bLocked;
public:
    StackHack()
    {
        if ( ++StackHack::nCnt > 50 )
            StackHack::bLocked = sal_True;
    }
    ~StackHack()
    {
        if ( --StackHack::nCnt < 5 )
            StackHack::bLocked = sal_False;
    }

    static sal_Bool IsLocked()  { return StackHack::bLocked; }
    static sal_uInt8 Count()        { return StackHack::nCnt; }
};


//Sollen obere bzw. untere Umrandung fuer den Frm ausgewertet werden?
// #i25029# - add optional 2nd parameter <_pPrevFrm>
// If set, its value is taken for testing, if borders/shadow have to joined
// with previous frame.
inline sal_uInt16 SwBorderAttrs::GetTopLine ( const SwFrm& _rFrm,
                                          const SwFrm* _pPrevFrm ) const
{
    if ( !bCachedGetTopLine || _pPrevFrm )
    {
        const_cast<SwBorderAttrs*>(this)->_GetTopLine( _rFrm, _pPrevFrm );
    }
    return nGetTopLine;
}
inline sal_uInt16 SwBorderAttrs::GetBottomLine( const SwFrm& _rFrm ) const
{
    if ( !bCachedGetBottomLine )
        const_cast<SwBorderAttrs*>(this)->_GetBottomLine( _rFrm );
    return nGetBottomLine;
}
inline void SwBorderAttrs::SetGetCacheLine( sal_Bool bNew ) const
{
    ((SwBorderAttrs*)this)->bCacheGetLine = bNew;
    ((SwBorderAttrs*)this)->bCachedGetBottomLine =
    ((SwBorderAttrs*)this)->bCachedGetTopLine = sal_False;
    // - invalidate cache for values <bJoinedWithPrev>
    // and <bJoinedWithNext>.
    bCachedJoinedWithPrev = sal_False;
    bCachedJoinedWithNext = sal_False;
}

inline sal_uInt16 SwBorderAttrs::CalcTopLine() const
{
    if ( bTopLine )
        ((SwBorderAttrs*)this)->_CalcTopLine();
    return nTopLine;
}
inline sal_uInt16 SwBorderAttrs::CalcBottomLine() const
{
    if ( bBottomLine )
        ((SwBorderAttrs*)this)->_CalcBottomLine();
    return nBottomLine;
}
inline sal_uInt16 SwBorderAttrs::CalcLeftLine() const
{
    if ( bLeftLine )
        ((SwBorderAttrs*)this)->_CalcLeftLine();
    return nLeftLine;
}
inline sal_uInt16 SwBorderAttrs::CalcRightLine() const
{
    if ( bRightLine )
        ((SwBorderAttrs*)this)->_CalcRightLine();
    return nRightLine;
}
inline sal_uInt16 SwBorderAttrs::CalcTop() const
{
    if ( bTop )
        ((SwBorderAttrs*)this)->_CalcTop();
    return nTop;
}
inline sal_uInt16 SwBorderAttrs::CalcBottom() const
{
    if ( bBottom )
        ((SwBorderAttrs*)this)->_CalcBottom();
    return nBottom;
}
inline sal_Bool SwBorderAttrs::IsLine() const
{
    if ( bLine )
        ((SwBorderAttrs*)this)->_IsLine();
    return bIsLine;
}

/** method to determine the spacing values of a frame

    #i28701#
    Values only provided for flow frames (table, section or text frames)
    Note: line spacing value is only determined for text frames
    #i102458#
    Add output parameter <obIsLineSpacingProportional>

    @param rFrm
    input parameter - frame, for which the spacing values are determined.

    @param onPrevLowerSpacing
    output parameter - lower spacing of the frame in SwTwips

    @param onPrevLineSpacing
    output parameter - line spacing of the frame in SwTwips

    @param obIsLineSpacingProportional
*/
void GetSpacingValuesOfFrm( const SwFrm& rFrm,
                            SwTwips& onLowerSpacing,
                            SwTwips& onLineSpacing,
                            bool& obIsLineSpacingProportional );

/** method to get the content of the table cell

    Content from any nested tables will be omitted.
    Note: line spacing value is only determined for text frames

    @param rCell_
    input parameter - the cell which should be searched for content.

    return
        pointer to the found content frame or 0
*/

const SwCntntFrm* GetCellCntnt( const SwLayoutFrm& rCell_ );


/** helper class to check if a frame has been deleted during an operation
 *  !!!WARNING!!! This should only be used as a last and desperate means
 *  to make the code robust.
 */

class SwDeletionChecker
{
    private:

    const SwFrm* mpFrm;
    const SwModify* mpRegIn;

    public:

    SwDeletionChecker( const SwFrm* pFrm )
            : mpFrm( pFrm ),
              mpRegIn( pFrm ? const_cast<SwFrm*>(pFrm)->GetRegisteredIn() : 0 )
    {
    }

    /**
     *  return
     *    true if mpFrm != 0 and mpFrm is not client of pRegIn
     *    false otherwise
     */
    bool HasBeenDeleted();
};

#endif  //_FRMTOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
