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

#ifndef SC_CONDITIO_HXX
#define SC_CONDITIO_HXX

#include "global.hxx"
#include "address.hxx"
#include "formula/grammar.hxx"
#include <svl/svarray.hxx>
#include "scdllapi.h"
#include "rangelst.hxx"

class ScBaseCell;
class ScFormulaCell;
class ScTokenArray;


#define SC_COND_GROW 16

//  nOptions Flags
#define SC_COND_NOBLANKS    1


// ordering of ScConditionMode and ScQueryOp is equal,
// to facilitate the merging of both in the future

enum ScConditionMode
{
    SC_COND_EQUAL,
    SC_COND_LESS,
    SC_COND_GREATER,
    SC_COND_EQLESS,
    SC_COND_EQGREATER,
    SC_COND_NOTEQUAL,
    SC_COND_BETWEEN,
    SC_COND_NOTBETWEEN,
    SC_COND_DUPLICATE,
    SC_COND_NOTDUPLICATE,
    SC_COND_DIRECT,
    SC_COND_NONE
};

enum ScConditionValType
{
    SC_VAL_VALUE,
    SC_VAL_STRING,
    SC_VAL_FORMULA
};

class ScConditionalFormat;

class SC_DLLPUBLIC ScConditionEntry
{
                                        // stored data:
    ScConditionMode     eOp;
    sal_uInt16              nOptions;
    double              nVal1;          // input or calculated
    double              nVal2;
    String              aStrVal1;       // input or calculated
    String              aStrVal2;
    String              aStrNmsp1;      // namespace to be used on (re)compilation, e.g. in XML import
    String              aStrNmsp2;      // namespace to be used on (re)compilation, e.g. in XML import
    formula::FormulaGrammar::Grammar eTempGrammar1;  // grammar to be used on (re)compilation, e.g. in XML import
    formula::FormulaGrammar::Grammar eTempGrammar2;  // grammar to be used on (re)compilation, e.g. in XML import
    sal_Bool                bIsStr1;        // for recognition of empty strings
    sal_Bool                bIsStr2;
    ScTokenArray*       pFormula1;      // entered formula
    ScTokenArray*       pFormula2;
    ScAddress           aSrcPos;        // source position for formulas
                                        // temporary data:
    String              aSrcString;     // formula source position as text during XML import
    ScFormulaCell*      pFCell1;
    ScFormulaCell*      pFCell2;
    ScDocument*         pDoc;
    sal_Bool                bRelRef1;
    sal_Bool                bRelRef2;
    sal_Bool                bFirstRun;

    void    MakeCells( const ScAddress& rPos );
    void    Compile( const String& rExpr1, const String& rExpr2,
                        const String& rExprNmsp1, const String& rExprNmsp2,
                        formula::FormulaGrammar::Grammar eGrammar1,
                        formula::FormulaGrammar::Grammar eGrammar2,
                        sal_Bool bTextToReal );
    void    Interpret( const ScAddress& rPos );

    sal_Bool    IsValid( double nArg, const ScAddress& rAddr ) const;
    sal_Bool    IsValidStr( const String& rArg, const ScAddress& rAddr ) const;

public:
            ScConditionEntry( ScConditionMode eOper,
                                const String& rExpr1, const String& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const String& rExprNmsp1, const String& rExprNmsp2,
                                formula::FormulaGrammar::Grammar eGrammar1,
                                formula::FormulaGrammar::Grammar eGrammar2 );
            ScConditionEntry( ScConditionMode eOper,
                                const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                ScDocument* pDocument, const ScAddress& rPos );
            ScConditionEntry( const ScConditionEntry& r );  // flat copy of formulas
            // true copy of formulas (for Ref-Undo):
            ScConditionEntry( ScDocument* pDocument, const ScConditionEntry& r );
    virtual ~ScConditionEntry();

    int             operator== ( const ScConditionEntry& r ) const;

    void            SetParent( ScConditionalFormat* pNew )  { pCondFormat = pNew; }

    sal_Bool            IsCellValid( ScBaseCell* pCell, const ScAddress& rPos ) const;

    ScConditionMode GetOperation() const        { return eOp; }
    sal_Bool            IsIgnoreBlank() const       { return ( nOptions & SC_COND_NOBLANKS ) == 0; }
    void            SetIgnoreBlank(sal_Bool bSet);
    ScAddress       GetSrcPos() const           { return aSrcPos; }

    ScAddress       GetValidSrcPos() const;     // adjusted to allow textual representation of expressions

    void            SetSrcString( const String& rNew );     // for XML import

    void            SetFormula1( const ScTokenArray& rArray );
    void            SetFormula2( const ScTokenArray& rArray );

    String          GetExpression( const ScAddress& rCursor, sal_uInt16 nPos, sal_uLong nNumFmt = 0,
                                    const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;

    ScTokenArray*   CreateTokenArry( sal_uInt16 nPos ) const;

    void            CompileAll();
    void            CompileXML();
    void            UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void            UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos );

    void            SourceChanged( const ScAddress& rChanged );

    bool            MarkUsedExternalReferences() const;

protected:
    virtual void    DataChanged( const ScRange* pModified ) const;
    ScDocument*     GetDocument() const     { return pDoc; }
    ScConditionalFormat*    pCondFormat;
};

//
//  single entry for conditional formatting
//

class SC_DLLPUBLIC ScCondFormatEntry : public ScConditionEntry
{
    String                  aStyleName;

    using ScConditionEntry::operator==;

public:
            ScCondFormatEntry( ScConditionMode eOper,
                                const String& rExpr1, const String& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const String& rStyle,
                                const String& rExprNmsp1 = EMPTY_STRING,
                                const String& rExprNmsp2 = EMPTY_STRING,
                                formula::FormulaGrammar::Grammar eGrammar1 = formula::FormulaGrammar::GRAM_DEFAULT,
                                formula::FormulaGrammar::Grammar eGrammar2 = formula::FormulaGrammar::GRAM_DEFAULT );
            ScCondFormatEntry( ScConditionMode eOper,
                                const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const String& rStyle );
            ScCondFormatEntry( const ScCondFormatEntry& r );
            ScCondFormatEntry( ScDocument* pDocument, const ScCondFormatEntry& r );
    virtual ~ScCondFormatEntry();

    int             operator== ( const ScCondFormatEntry& r ) const;

    const String&   GetStyle() const        { return aStyleName; }
    void            UpdateStyleName(const String& rNew)  { aStyleName=rNew; }

protected:
    virtual void    DataChanged( const ScRange* pModified ) const;
};

//
//  complete conditional formatting
//

class SC_DLLPUBLIC ScConditionalFormat
{
    ScDocument*         pDoc;
    ScRangeList*        pAreas;             // area for Paint
    sal_uInt32          nKey;               // Index in attributes
    ScCondFormatEntry** ppEntries;
    sal_uInt16              nEntryCount;
    sal_Bool                bIsUsed;            // temporary at Save
    ScRangeListRef      pRanges;            // Ranges for conditional format

public:
            ScConditionalFormat(sal_uInt32 nNewKey, ScDocument* pDocument);
            ScConditionalFormat(const ScConditionalFormat& r);
            ~ScConditionalFormat();

    // true copy of formulas (for Ref-Undo / between documents)
    ScConditionalFormat* Clone(ScDocument* pNewDoc = NULL) const;

    void            AddEntry( const ScCondFormatEntry& rNew );
    void            AddRangeInfo( const ScRangeListRef& rRanges );
    const ScRangeListRef&  GetRangeInfo() const  { return pRanges; }

    sal_Bool            IsEmpty() const         { return (nEntryCount == 0); }
    sal_uInt16          Count() const           { return nEntryCount; }

    void            CompileAll();
    void            CompileXML();
    void            UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void            UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos );
    void            RenameCellStyle( const String& rOld, const String& rNew );

    void            SourceChanged( const ScAddress& rAddr );

    const ScCondFormatEntry* GetEntry( sal_uInt16 nPos ) const;

    const String&   GetCellStyle( ScBaseCell* pCell, const ScAddress& rPos ) const;

    sal_Bool            EqualEntries( const ScConditionalFormat& r ) const;

    void            DoRepaint( const ScRange* pModified );
    void            InvalidateArea();

    sal_uInt32      GetKey() const          { return nKey; }
    void            SetKey(sal_uInt32 nNew) { nKey = nNew; }    // only if not inserted!

    void            SetUsed(sal_Bool bSet)      { bIsUsed = bSet; }
    sal_Bool            IsUsed() const          { return bIsUsed; }

    bool            MarkUsedExternalReferences() const;

    //  sorted (via PTRARR) by Index
    //  operator== only for sorting
    sal_Bool operator ==( const ScConditionalFormat& r ) const  { return nKey == r.nKey; }
    sal_Bool operator < ( const ScConditionalFormat& r ) const  { return nKey <  r.nKey; }
};

//
//  List of areas and formats:
//

typedef ScConditionalFormat* ScConditionalFormatPtr;

SV_DECL_PTRARR_SORT(ScConditionalFormats_Impl, ScConditionalFormatPtr,
                        SC_COND_GROW, SC_COND_GROW)

class ScConditionalFormatList : public ScConditionalFormats_Impl
{
public:
        ScConditionalFormatList() {}
        ScConditionalFormatList(const ScConditionalFormatList& rList);
        ScConditionalFormatList(ScDocument* pNewDoc, const ScConditionalFormatList& rList);
        ~ScConditionalFormatList() {}

    void    InsertNew( ScConditionalFormat* pNew )
                { if (!Insert(pNew)) delete pNew; }

    ScConditionalFormat* GetFormat( sal_uInt32 nKey );

    void    CompileAll();
    void    CompileXML();
    void    UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void    RenameCellStyle( const String& rOld, const String& rNew );
    void    UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos );

    void    SourceChanged( const ScAddress& rAddr );

    /** Temporarily during save, returns RefManager's decision whether ALL
     *  references are marked now. */
    bool    MarkUsedExternalReferences() const;

    sal_Bool    operator==( const ScConditionalFormatList& r ) const;       // for Ref-Undo
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
