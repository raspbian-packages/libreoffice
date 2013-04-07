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



#define SMDLL 1
#include "tools/rcid.h"
#include <comphelper/string.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/app.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/ctrltool.hxx>
#include <sfx2/printer.hxx>
#include <vcl/sound.hxx>
#include <vcl/sndstyle.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/settings.hxx>
#include <vcl/wall.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfx.hrc>
#include <tools/string.hxx>
#include <osl/diagnose.h>
#include <svx/ucsubset.hxx>


#include "dialog.hxx"
#include "starmath.hrc"
#include "config.hxx"
#include "dialog.hrc"
#include "smmod.hxx"
#include "symbol.hxx"
#include "view.hxx"
#include "document.hxx"
#include "unomodel.hxx"


using ::rtl::OUString;

////////////////////////////////////////
//
// Da der FontStyle besser ueber die Attribute gesetzt/abgefragt wird als ueber
// den StyleName bauen wir uns hier unsere eigene Uebersetzung
// Attribute <-> StyleName
//

class SmFontStyles
{
    String  aNormal;
    String  aBold;
    String  aItalic;
    String  aBoldItalic;
    String  aEmpty;

public:
    SmFontStyles();

    sal_uInt16          GetCount() const    { return 4; }
    const String &  GetStyleName( const Font &rFont ) const;
    const String &  GetStyleName( sal_uInt16 nIdx ) const;
};


SmFontStyles::SmFontStyles() :
    aNormal ( ResId( RID_FONTREGULAR, *SM_MOD()->GetResMgr() ) ),
    aBold   ( ResId( RID_FONTBOLD,    *SM_MOD()->GetResMgr() ) ),
    aItalic ( ResId( RID_FONTITALIC,  *SM_MOD()->GetResMgr() ) )
{

    aBoldItalic = aBold;
    aBoldItalic.AppendAscii( ", " );
    aBoldItalic += aItalic;
}


const String & SmFontStyles::GetStyleName( const Font &rFont ) const
{
    //! compare also SmSpecialNode::Prepare
    bool bBold   = IsBold( rFont ),
         bItalic = IsItalic( rFont );

    if (bBold && bItalic)
        return aBoldItalic;
    else if (bItalic)
        return aItalic;
    else if (bBold)
        return aBold;
    else
        return aNormal;
}


const String & SmFontStyles::GetStyleName( sal_uInt16 nIdx ) const
{
    // 0 = "normal",  1 = "italic",
    // 2 = "bold",    3 = "bold italic"

#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE( nIdx < GetCount(), "index out of range" );
#endif
    switch (nIdx)
    {
        case 0 : return aNormal;
        case 1 : return aItalic;
        case 2 : return aBold;
        case 3 : return aBoldItalic;
    }
    return aEmpty;
}


const SmFontStyles & GetFontStyles()
{
    static const SmFontStyles aImpl;
    return aImpl;
}

/////////////////////////////////////////////////////////////////

void SetFontStyle(const XubString &rStyleName, Font &rFont)
{
    // finden des Index passend zum StyleName fuer den leeren StyleName wird
    // 0 (nicht bold nicht italic) angenommen.
    sal_uInt16  nIndex = 0;
    if (rStyleName.Len())
    {
        sal_uInt16 i;
        const SmFontStyles &rStyles = GetFontStyles();
        for (i = 0;  i < rStyles.GetCount();  i++)
            if (rStyleName.CompareTo( rStyles.GetStyleName(i) ) == COMPARE_EQUAL)
                break;
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE(i < rStyles.GetCount(), "style-name unknown");
#endif
        nIndex = i;
    }

    rFont.SetItalic((nIndex & 0x1) ? ITALIC_NORMAL : ITALIC_NONE);
    rFont.SetWeight((nIndex & 0x2) ? WEIGHT_BOLD : WEIGHT_NORMAL);
}


/**************************************************************************/

IMPL_LINK_INLINE_START( SmPrintOptionsTabPage, SizeButtonClickHdl, Button *, EMPTYARG/*pButton*/ )
{
    aZoom.Enable(aSizeZoomed.IsChecked());
    return 0;
}
IMPL_LINK_INLINE_END( SmPrintOptionsTabPage, SizeButtonClickHdl, Button *, pButton )


SmPrintOptionsTabPage::SmPrintOptionsTabPage(Window *pParent, const SfxItemSet &rOptions)
    : SfxTabPage(pParent, SmResId(RID_PRINTOPTIONPAGE), rOptions),
    aFixedLine1     (this, SmResId( FL_PRINTOPTIONS )),
    aTitle          (this, SmResId( CB_TITLEROW )),
    aText           (this, SmResId( CB_EQUATION_TEXT )),
    aFrame          (this, SmResId( CB_FRAME )),
    aFixedLine2     (this, SmResId( FL_PRINT_FORMAT )),
    aSizeNormal     (this, SmResId( RB_ORIGINAL_SIZE )),
    aSizeScaled     (this, SmResId( RB_FIT_TO_PAGE )),
    aSizeZoomed     (this, SmResId( RB_ZOOM )),
    aZoom           (this, SmResId( MF_ZOOM )),
    aFixedLine3     (this, SmResId( FL_MISC_OPTIONS )),
    aNoRightSpaces  (this, SmResId( CB_IGNORE_SPACING )),
    aSaveOnlyUsedSymbols  (this, SmResId( CB_SAVE_ONLY_USED_SYMBOLS ))
{
    FreeResource();

    aSizeNormal.SetClickHdl(LINK(this, SmPrintOptionsTabPage, SizeButtonClickHdl));
    aSizeScaled.SetClickHdl(LINK(this, SmPrintOptionsTabPage, SizeButtonClickHdl));
    aSizeZoomed.SetClickHdl(LINK(this, SmPrintOptionsTabPage, SizeButtonClickHdl));

    Reset(rOptions);
}


sal_Bool SmPrintOptionsTabPage::FillItemSet(SfxItemSet& rSet)
{
    sal_uInt16  nPrintSize;
    if (aSizeNormal.IsChecked())
        nPrintSize = PRINT_SIZE_NORMAL;
    else if (aSizeScaled.IsChecked())
        nPrintSize = PRINT_SIZE_SCALED;
    else
        nPrintSize = PRINT_SIZE_ZOOMED;

    rSet.Put(SfxUInt16Item(GetWhich(SID_PRINTSIZE), (sal_uInt16) nPrintSize));
    rSet.Put(SfxUInt16Item(GetWhich(SID_PRINTZOOM), (sal_uInt16) aZoom.GetValue()));
    rSet.Put(SfxBoolItem(GetWhich(SID_PRINTTITLE), aTitle.IsChecked()));
    rSet.Put(SfxBoolItem(GetWhich(SID_PRINTTEXT), aText.IsChecked()));
    rSet.Put(SfxBoolItem(GetWhich(SID_PRINTFRAME), aFrame.IsChecked()));
    rSet.Put(SfxBoolItem(GetWhich(SID_NO_RIGHT_SPACES), aNoRightSpaces.IsChecked()));
    rSet.Put(SfxBoolItem(GetWhich(SID_SAVE_ONLY_USED_SYMBOLS), aSaveOnlyUsedSymbols.IsChecked()));

    return true;
}


void SmPrintOptionsTabPage::Reset(const SfxItemSet& rSet)
{
    SmPrintSize ePrintSize = (SmPrintSize)((const SfxUInt16Item &)rSet.Get(GetWhich(SID_PRINTSIZE))).GetValue();

    aSizeNormal.Check(ePrintSize == PRINT_SIZE_NORMAL);
    aSizeScaled.Check(ePrintSize == PRINT_SIZE_SCALED);
    aSizeZoomed.Check(ePrintSize == PRINT_SIZE_ZOOMED);

    aZoom.Enable(aSizeZoomed.IsChecked());

    aZoom.SetValue(((const SfxUInt16Item &)rSet.Get(GetWhich(SID_PRINTZOOM))).GetValue());

    aTitle.Check(((const SfxBoolItem &)rSet.Get(GetWhich(SID_PRINTTITLE))).GetValue());
    aText.Check(((const SfxBoolItem &)rSet.Get(GetWhich(SID_PRINTTEXT))).GetValue());
    aFrame.Check(((const SfxBoolItem &)rSet.Get(GetWhich(SID_PRINTFRAME))).GetValue());
    aNoRightSpaces.Check(((const SfxBoolItem &)rSet.Get(GetWhich(SID_NO_RIGHT_SPACES))).GetValue());
    aSaveOnlyUsedSymbols.Check(((const SfxBoolItem &)rSet.Get(GetWhich(SID_SAVE_ONLY_USED_SYMBOLS))).GetValue());
}


SfxTabPage* SmPrintOptionsTabPage::Create(Window* pWindow, const SfxItemSet& rSet)
{
    return (new SmPrintOptionsTabPage(pWindow, rSet));
}

/**************************************************************************/


void SmShowFont::Paint(const Rectangle& rRect )
{
    Control::Paint( rRect );

    XubString   Text (GetFont().GetName());
    Size    TextSize(GetTextWidth(Text), GetTextHeight());

    DrawText(Point((GetOutputSize().Width()  - TextSize.Width())  / 2,
                   (GetOutputSize().Height() - TextSize.Height()) / 2), Text);
}


void SmShowFont::SetFont(const Font& rFont)
{
    Color aTxtColor( GetTextColor() );
    Font aFont (rFont);

    Invalidate();
    aFont.SetSize(Size(0, 24));
    aFont.SetAlign(ALIGN_TOP);
    Control::SetFont(aFont);

    // keep old text color (new font may have different color)
    SetTextColor( aTxtColor );
}


IMPL_LINK_INLINE_START( SmFontDialog, FontSelectHdl, ComboBox *, pComboBox )
{
    Face.SetName(pComboBox->GetText());
    aShowFont.SetFont(Face);
    return 0;
}
IMPL_LINK_INLINE_END( SmFontDialog, FontSelectHdl, ComboBox *, pComboBox )


IMPL_LINK( SmFontDialog, FontModifyHdl, ComboBox *, pComboBox )
{
    // if font is available in list then use it
    sal_uInt16 nPos = pComboBox->GetEntryPos( pComboBox->GetText() );
    if (COMBOBOX_ENTRY_NOTFOUND != nPos)
    {
        FontSelectHdl( pComboBox );
    }
    return 0;
}


IMPL_LINK( SmFontDialog, AttrChangeHdl, CheckBox *, EMPTYARG /*pCheckBox*/ )
{
    if (aBoldCheckBox.IsChecked())
        Face.SetWeight(FontWeight(WEIGHT_BOLD));
    else
        Face.SetWeight(FontWeight(WEIGHT_NORMAL));

    if (aItalicCheckBox.IsChecked())
        Face.SetItalic(ITALIC_NORMAL);
    else
        Face.SetItalic(ITALIC_NONE);

    aShowFont.SetFont(Face);
    return 0;
}


void SmFontDialog::SetFont(const Font &rFont)
{
    Face = rFont;

    aFontBox.SetText( Face.GetName() );
    aBoldCheckBox.Check( IsBold( Face ) );
    aItalicCheckBox.Check( IsItalic( Face ) );

    aShowFont.SetFont(Face);
}


SmFontDialog::SmFontDialog(Window * pParent,
        OutputDevice *pFntListDevice, bool bHideCheckboxes, bool bFreeRes)
    : ModalDialog(pParent,SmResId(RID_FONTDIALOG)),
    aFixedText1     (this, SmResId(1)),
    aFontBox        (this, SmResId(1)),
    aBoldCheckBox   (this, SmResId(1)),
    aItalicCheckBox (this, SmResId(2)),
    aOKButton1      (this, SmResId(1)),
    aCancelButton1  (this, SmResId(1)),
    aShowFont       (this, SmResId(1)),
    aFixedText2     (this, SmResId(2))
{
    if (bFreeRes)
        FreeResource();

    {
        WaitObject( this );

        FontList aFontList( pFntListDevice );

        sal_uInt16  nCount = aFontList.GetFontNameCount();
        for (sal_uInt16 i = 0;  i < nCount;  i++)
            aFontBox.InsertEntry( aFontList.GetFontName(i).GetName() );

        Face.SetSize(Size(0, 24));
        Face.SetWeight(WEIGHT_NORMAL);
        Face.SetItalic(ITALIC_NONE);
        Face.SetFamily(FAMILY_DONTKNOW);
        Face.SetPitch(PITCH_DONTKNOW);
        Face.SetCharSet(RTL_TEXTENCODING_DONTKNOW);
        Face.SetTransparent(true);

        InitColor_Impl();

        // preview like controls should have a 2D look
        aShowFont.SetBorderStyle( WINDOW_BORDER_MONO );
    }

    aFontBox.SetSelectHdl(LINK(this, SmFontDialog, FontSelectHdl));
    aFontBox.SetModifyHdl(LINK(this, SmFontDialog, FontModifyHdl));
    aBoldCheckBox.SetClickHdl(LINK(this, SmFontDialog, AttrChangeHdl));
    aItalicCheckBox.SetClickHdl(LINK(this, SmFontDialog, AttrChangeHdl));

    if (bHideCheckboxes)
    {
        aBoldCheckBox.Check( false );
        aBoldCheckBox.Enable( false );
        aBoldCheckBox.Show( false );
        aItalicCheckBox.Check( false );
        aItalicCheckBox.Enable( false );
        aItalicCheckBox.Show( false );
        aFixedText2.Show( false );

        Size  aSize( aFontBox.GetSizePixel() );
        long nComboBoxBottom = aFontBox.GetPosPixel().Y() + aFontBox.GetSizePixel().Height();
        long nCheckBoxBottom = aItalicCheckBox.GetPosPixel().Y() + aItalicCheckBox.GetSizePixel().Height();
        aSize.Height() += nCheckBoxBottom - nComboBoxBottom;
        aFontBox.SetSizePixel( aSize );
    }
}

void SmFontDialog::InitColor_Impl()
{
#if OSL_DEBUG_LEVEL > 1
    Color aBC( GetDisplayBackground().GetColor() );
#endif
    ColorData   nBgCol  = COL_WHITE,
                nTxtCol = COL_BLACK;
    const StyleSettings &rS = GetSettings().GetStyleSettings();
    if (rS.GetHighContrastMode())
    {
        nBgCol  = rS.GetFieldColor().GetColor();
        nTxtCol = rS.GetFieldTextColor().GetColor();
    }

    Color aTmpColor( nBgCol );
    Wallpaper aWall( aTmpColor );
    Color aTxtColor( nTxtCol );
    aShowFont.SetBackground( aWall );
    aShowFont.SetTextColor( aTxtColor );
}


void SmFontDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DATACHANGED_SETTINGS  &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            InitColor_Impl();

    ModalDialog::DataChanged( rDCEvt );
}

/**************************************************************************/


IMPL_LINK( SmFontSizeDialog, DefaultButtonClickHdl, Button *, EMPTYARG /*pButton*/ )
{
    QueryBox *pQueryBox = new QueryBox(this, SmResId(RID_DEFAULTSAVEQUERY));

    if (pQueryBox->Execute() == RET_YES)
    {
        SmModule *pp = SM_MOD();
        SmFormat aFmt( pp->GetConfig()->GetStandardFormat() );
        WriteTo( aFmt );
        pp->GetConfig()->SetStandardFormat( aFmt );
    }

    delete pQueryBox;
    return 0;
}


SmFontSizeDialog::SmFontSizeDialog(Window * pParent, bool bFreeRes)
    : ModalDialog(pParent, SmResId(RID_FONTSIZEDIALOG)),
    aFixedText1(this, SmResId(1)),
    aBaseSize(this, SmResId(1)),
    aFixedText4(this, SmResId(4)),
    aTextSize(this, SmResId(4)),
    aFixedText5(this, SmResId(5)),
    aIndexSize(this, SmResId(5)),
    aFixedText6(this, SmResId(6)),
    aFunctionSize(this, SmResId(6)),
    aFixedText7(this, SmResId(7)),
    aOperatorSize(this, SmResId(7)),
    aFixedText8(this, SmResId(8)),
    aBorderSize(this, SmResId(8)),
    aFixedLine1(this, SmResId(1)),
    aOKButton1(this, SmResId(1)),
    aCancelButton1(this, SmResId(1)),
    aDefaultButton(this, SmResId(1))
{
    if (bFreeRes)
        FreeResource();

    aDefaultButton.SetClickHdl(LINK(this, SmFontSizeDialog, DefaultButtonClickHdl));
}


void SmFontSizeDialog::ReadFrom(const SmFormat &rFormat)
{
    //! aufpassen: richtig runden!
    aBaseSize.SetValue( SmRoundFraction(
        Sm100th_mmToPts( rFormat.GetBaseSize().Height() ) ) );

    aTextSize    .SetValue( rFormat.GetRelSize(SIZ_TEXT) );
    aIndexSize   .SetValue( rFormat.GetRelSize(SIZ_INDEX) );
    aFunctionSize.SetValue( rFormat.GetRelSize(SIZ_FUNCTION) );
    aOperatorSize.SetValue( rFormat.GetRelSize(SIZ_OPERATOR) );
    aBorderSize  .SetValue( rFormat.GetRelSize(SIZ_LIMITS) );
}


void SmFontSizeDialog::WriteTo(SmFormat &rFormat) const
{
    rFormat.SetBaseSize( Size(0, SmPtsTo100th_mm( static_cast< long >(aBaseSize.GetValue()))) );

    rFormat.SetRelSize(SIZ_TEXT,     (sal_uInt16) aTextSize    .GetValue());
    rFormat.SetRelSize(SIZ_INDEX,    (sal_uInt16) aIndexSize   .GetValue());
    rFormat.SetRelSize(SIZ_FUNCTION, (sal_uInt16) aFunctionSize.GetValue());
    rFormat.SetRelSize(SIZ_OPERATOR, (sal_uInt16) aOperatorSize.GetValue());
    rFormat.SetRelSize(SIZ_LIMITS,   (sal_uInt16) aBorderSize  .GetValue());

    const Size aTmp (rFormat.GetBaseSize());
    for (sal_uInt16  i = FNT_BEGIN;  i <= FNT_END;  i++)
        rFormat.SetFontSize(i, aTmp);

    rFormat.RequestApplyChanges();
}


/**************************************************************************/


IMPL_LINK( SmFontTypeDialog, MenuSelectHdl, Menu *, pMenu )
{
    SmFontPickListBox *pActiveListBox;

    bool bHideCheckboxes = false;
    switch (pMenu->GetCurItemId())
    {
        case 1: pActiveListBox = &aVariableFont; break;
        case 2: pActiveListBox = &aFunctionFont; break;
        case 3: pActiveListBox = &aNumberFont;   break;
        case 4: pActiveListBox = &aTextFont;     break;
        case 5: pActiveListBox = &aSerifFont; bHideCheckboxes = true;   break;
        case 6: pActiveListBox = &aSansFont;  bHideCheckboxes = true;   break;
        case 7: pActiveListBox = &aFixedFont; bHideCheckboxes = true;   break;
        default:pActiveListBox = NULL;
    }

    if (pActiveListBox)
    {
        SmFontDialog *pFontDialog = new SmFontDialog(this, pFontListDev, bHideCheckboxes);

        pActiveListBox->WriteTo(*pFontDialog);
        if (pFontDialog->Execute() == RET_OK)
            pActiveListBox->ReadFrom(*pFontDialog);
        delete pFontDialog;
    }
    return 0;
}


IMPL_LINK_INLINE_START( SmFontTypeDialog, DefaultButtonClickHdl, Button *, EMPTYARG /*pButton*/ )
{
    QueryBox *pQueryBox = new QueryBox(this, SmResId(RID_DEFAULTSAVEQUERY));
    if (pQueryBox->Execute() == RET_YES)
    {
        SmModule *pp = SM_MOD();
        SmFormat aFmt( pp->GetConfig()->GetStandardFormat() );
        WriteTo( aFmt );
        pp->GetConfig()->SetStandardFormat( aFmt, true );
    }

    delete pQueryBox;
    return 0;
}
IMPL_LINK_INLINE_END( SmFontTypeDialog, DefaultButtonClickHdl, Button *, pButton )


SmFontTypeDialog::SmFontTypeDialog(Window * pParent, OutputDevice *pFntListDevice, bool bFreeRes)
    : ModalDialog(pParent, SmResId(RID_FONTTYPEDIALOG)),
    aFixedText1    (this, SmResId(1)),
    aVariableFont  (this, SmResId(1)),
    aFixedText2    (this, SmResId(2)),
    aFunctionFont  (this, SmResId(2)),
    aFixedText3    (this, SmResId(3)),
    aNumberFont    (this, SmResId(3)),
    aFixedText4    (this, SmResId(4)),
    aTextFont      (this, SmResId(4)),
    aFixedText5    (this, SmResId(5)),
    aSerifFont     (this, SmResId(5)),
    aFixedText6    (this, SmResId(6)),
    aSansFont      (this, SmResId(6)),
    aFixedText7    (this, SmResId(7)),
    aFixedFont     (this, SmResId(7)),
    aFixedLine1    (this, SmResId(1)),
    aFixedLine2    (this, SmResId(2)),
    aOKButton1     (this, SmResId(1)),
    aCancelButton1 (this, SmResId(1)),
    aMenuButton    (this, SmResId(1)),
    aDefaultButton (this, SmResId(2)),
    pFontListDev    (pFntListDevice)
{
    if (bFreeRes)
        FreeResource();

    aDefaultButton.SetClickHdl(LINK(this, SmFontTypeDialog, DefaultButtonClickHdl));

    aMenuButton.GetPopupMenu()->SetSelectHdl(LINK(this, SmFontTypeDialog, MenuSelectHdl));
}

void SmFontTypeDialog::ReadFrom(const SmFormat &rFormat)
{
    SmModule *pp = SM_MOD();

    aVariableFont = pp->GetConfig()->GetFontPickList(FNT_VARIABLE);
    aFunctionFont = pp->GetConfig()->GetFontPickList(FNT_FUNCTION);
    aNumberFont   = pp->GetConfig()->GetFontPickList(FNT_NUMBER);
    aTextFont     = pp->GetConfig()->GetFontPickList(FNT_TEXT);
    aSerifFont    = pp->GetConfig()->GetFontPickList(FNT_SERIF);
    aSansFont     = pp->GetConfig()->GetFontPickList(FNT_SANS);
    aFixedFont    = pp->GetConfig()->GetFontPickList(FNT_FIXED);

    aVariableFont.Insert( rFormat.GetFont(FNT_VARIABLE) );
    aFunctionFont.Insert( rFormat.GetFont(FNT_FUNCTION) );
    aNumberFont  .Insert( rFormat.GetFont(FNT_NUMBER) );
    aTextFont    .Insert( rFormat.GetFont(FNT_TEXT) );
    aSerifFont   .Insert( rFormat.GetFont(FNT_SERIF) );
    aSansFont    .Insert( rFormat.GetFont(FNT_SANS) );
    aFixedFont   .Insert( rFormat.GetFont(FNT_FIXED) );
}


void SmFontTypeDialog::WriteTo(SmFormat &rFormat) const
{
    SmModule *pp = SM_MOD();

    pp->GetConfig()->GetFontPickList(FNT_VARIABLE) = aVariableFont;
    pp->GetConfig()->GetFontPickList(FNT_FUNCTION) = aFunctionFont;
    pp->GetConfig()->GetFontPickList(FNT_NUMBER)   = aNumberFont;
    pp->GetConfig()->GetFontPickList(FNT_TEXT)     = aTextFont;
    pp->GetConfig()->GetFontPickList(FNT_SERIF)    = aSerifFont;
    pp->GetConfig()->GetFontPickList(FNT_SANS)     = aSansFont;
    pp->GetConfig()->GetFontPickList(FNT_FIXED)    = aFixedFont;

    rFormat.SetFont( FNT_VARIABLE, aVariableFont.Get(0) );
    rFormat.SetFont( FNT_FUNCTION, aFunctionFont.Get(0) );
    rFormat.SetFont( FNT_NUMBER,   aNumberFont  .Get(0) );
    rFormat.SetFont( FNT_TEXT,     aTextFont    .Get(0) );
    rFormat.SetFont( FNT_SERIF,    aSerifFont   .Get(0) );
    rFormat.SetFont( FNT_SANS,     aSansFont    .Get(0) );
    rFormat.SetFont( FNT_FIXED,    aFixedFont   .Get(0) );

    rFormat.RequestApplyChanges();
}

/**************************************************************************/

struct FieldMinMax
{
    sal_uInt16 nMin, nMax;
};

// Data for min and max values of the 4 metric fields
// for each of the 10 categories
static const FieldMinMax pMinMaxData[10][4] =
{
    // 0
    {{ 0, 200 },    { 0, 200 },     { 0, 100 },     { 0, 0 }},
    // 1
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 2
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 3
    {{ 0, 100 },    { 1, 100 },     { 0, 0 },       { 0, 0 }},
    // 4
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 5
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 100 }},
    // 6
    {{ 0, 300 },    { 0, 300 },     { 0, 0 },       { 0, 0 }},
    // 7
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 8
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 9
    {{ 0, 10000 },  { 0, 10000 },   { 0, 10000 },   { 0, 10000 }}
};

SmCategoryDesc::SmCategoryDesc(const ResId& rResId, sal_uInt16 nCategoryIdx) :
    Resource(rResId)
{
    if (IsAvailableRes(ResId(1,*rResId.GetResMgr()).SetRT(RSC_STRING)))
    {
        Name = XubString(ResId(1,*rResId.GetResMgr()));

        int i;
        for (i = 0; i < 4; i++)
        {
            int nI2 = i + 2;

            if (IsAvailableRes(ResId(nI2,*rResId.GetResMgr()).SetRT(RSC_STRING)))
            {
                Strings  [i] = new XubString(ResId(nI2,*rResId.GetResMgr()));
                Graphics [i] = new Bitmap(ResId(10*nI2,*rResId.GetResMgr()));
            }
            else
            {
                Strings  [i] = 0;
                Graphics [i] = 0;
            }
        }

        for (i = 0; i < 4; i++)
        {
            const FieldMinMax &rMinMax = pMinMaxData[ nCategoryIdx ][i];
            Value[i] = Minimum[i] = rMinMax.nMin;
            Maximum[i] = rMinMax.nMax;
        }
    }

    FreeResource();
}


SmCategoryDesc::~SmCategoryDesc()
{
    for (int i = 0; i < 4; i++)
    {
        delete Strings  [i];
        delete Graphics [i];
    }
}

/**************************************************************************/

IMPL_LINK( SmDistanceDialog, GetFocusHdl, Control *, pControl )
{
    if (Categories[nActiveCategory])
    {
        sal_uInt16  i;

        if (pControl == &aMetricField1)
            i = 0;
        else if (pControl == &aMetricField2)
            i = 1;
        else if (pControl == &aMetricField3)
            i = 2;
        else if (pControl == &aMetricField4)
            i = 3;
        else
            return 0;
        aBitmap.SetBitmap(*(Categories[nActiveCategory]->GetGraphic(i)));
    }
    return 0;
}

IMPL_LINK( SmDistanceDialog, MenuSelectHdl, Menu *, pMenu )
{
    SetCategory(pMenu->GetCurItemId() - 1);
    return 0;
}


IMPL_LINK( SmDistanceDialog, DefaultButtonClickHdl, Button *, EMPTYARG /*pButton*/ )
{
    QueryBox *pQueryBox = new QueryBox(this, SmResId(RID_DEFAULTSAVEQUERY));

    if (pQueryBox->Execute() == RET_YES)
    {
        SmModule *pp = SM_MOD();
        SmFormat aFmt( pp->GetConfig()->GetStandardFormat() );
        WriteTo( aFmt );
        pp->GetConfig()->SetStandardFormat( aFmt );
    }
    delete pQueryBox;
    return 0;
}


IMPL_LINK( SmDistanceDialog, CheckBoxClickHdl, CheckBox *, pCheckBox )
{
    if (pCheckBox == &aCheckBox1)
    {
        aCheckBox1.Toggle();

        bool bChecked = aCheckBox1.IsChecked();
        aFixedText4  .Enable( bChecked );
        aMetricField4.Enable( bChecked );
    }
    return 0;
}


void SmDistanceDialog::SetHelpId(MetricField &rField, const rtl::OString& sHelpId)
{
    //! HelpID's die auf diese Weise explizit gesetzt werden, muessen im
    //! util Verzeichnis im File "hidother.src" mit Hilfe von "hidspecial"
    //! definiert werden!

    const XubString aEmptyText;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(aEmptyText.Len() == 0, "Sm: Ooops...");
#endif

    rField.SetHelpId(sHelpId);
    rField.SetHelpText(aEmptyText);

    // since MetricField inherits from SpinField which has a sub Edit field
    // (which is actually the one we modify) we have to set the help-id
    // for it too.
    Edit *pSubEdit = rField.GetSubEdit();
    if (pSubEdit)
    {
        pSubEdit->SetHelpId(sHelpId);
        pSubEdit->SetHelpText(aEmptyText);
    }
}


void SmDistanceDialog::SetCategory(sal_uInt16 nCategory)
{
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(/*0 <= nCategory  &&*/  nCategory < NOCATEGORIES,
        "Sm: wrong category number in SmDistanceDialog");
#endif

    // array to convert category- and metricfield-number in help ids.
    // 0 is used in case of unused combinations.
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(NOCATEGORIES == 10, "Sm : array doesn't fit into the number of categories");
#endif
    static const char * aCatMf2Hid[10][4] =
    {
        { HID_SMA_DEFAULT_DIST,         HID_SMA_LINE_DIST,          HID_SMA_ROOT_DIST, 0 },
        { HID_SMA_SUP_DIST,             HID_SMA_SUB_DIST ,          0, 0 },
        { HID_SMA_NUMERATOR_DIST,       HID_SMA_DENOMINATOR_DIST,   0, 0 },
        { HID_SMA_FRACLINE_EXCWIDTH,    HID_SMA_FRACLINE_LINEWIDTH, 0, 0 },
        { HID_SMA_UPPERLIMIT_DIST,      HID_SMA_LOWERLIMIT_DIST,    0, 0 },
        { HID_SMA_BRACKET_EXCHEIGHT,    HID_SMA_BRACKET_DIST,       0, HID_SMA_BRACKET_EXCHEIGHT2 },
        { HID_SMA_MATRIXROW_DIST,       HID_SMA_MATRIXCOL_DIST,     0, 0 },
        { HID_SMA_ATTRIBUT_DIST,        HID_SMA_INTERATTRIBUT_DIST, 0, 0 },
        { HID_SMA_OPERATOR_EXCHEIGHT,   HID_SMA_OPERATOR_DIST,      0, 0 },
        { HID_SMA_LEFTBORDER_DIST,      HID_SMA_RIGHTBORDER_DIST,   HID_SMA_UPPERBORDER_DIST, HID_SMA_LOWERBORDER_DIST }
    };

    // array to help iterate over the controls
    Window * const  aWin[4][2] =
    {
        { &aFixedText1,  &aMetricField1 },
        { &aFixedText2,  &aMetricField2 },
        { &aFixedText3,  &aMetricField3 },
        { &aFixedText4,  &aMetricField4 }
    };

    SmCategoryDesc *pCat;

    // merken der (evtl neuen) Einstellungen der aktiven SmCategoryDesc
    // bevor zu der neuen gewechselt wird.
    if (nActiveCategory != CATEGORY_NONE)
    {
        pCat = Categories[nActiveCategory];
        pCat->SetValue(0, (sal_uInt16) aMetricField1.GetValue());
        pCat->SetValue(1, (sal_uInt16) aMetricField2.GetValue());
        pCat->SetValue(2, (sal_uInt16) aMetricField3.GetValue());
        pCat->SetValue(3, (sal_uInt16) aMetricField4.GetValue());

        if (nActiveCategory == 5)
            bScaleAllBrackets = aCheckBox1.IsChecked();

        aMenuButton.GetPopupMenu()->CheckItem(nActiveCategory + 1, false);
    }

    // aktivieren/deaktivieren der zugehoerigen Controls in Abhaengigkeit von der
    // gewaehlten Kategorie.
    bool  bActive;
    for (sal_uInt16 i = 0;  i < 4;  i++)
    {
        FixedText   *pFT = (FixedText * const)   aWin[i][0];
        MetricField *pMF = (MetricField * const) aWin[i][1];

        // Um feststellen welche Controls aktiv sein sollen wird das
        // vorhandensein einer zugehoerigen HelpID ueberprueft.
        bActive = aCatMf2Hid[nCategory][i] != 0;

        pFT->Show(bActive);
        pFT->Enable(bActive);
        pMF->Show(bActive);
        pMF->Enable(bActive);

        // setzen von Masseinheit und Anzahl der Nachkommastellen
        FieldUnit  eUnit;
        sal_uInt16     nDigits;
        if (nCategory < 9)
        {
            eUnit   = FUNIT_CUSTOM;
            nDigits = 0;
            pMF->SetCustomUnitText( '%' );
        }
        else
        {
            eUnit   = FUNIT_100TH_MM;
            nDigits = 2;
        }
        pMF->SetUnit(eUnit);            //! veraendert den Wert
        pMF->SetDecimalDigits(nDigits);

        if (bActive)
        {
            pCat = Categories[nCategory];
            pFT->SetText(*pCat->GetString(i));

            pMF->SetMin(pCat->GetMinimum(i));
            pMF->SetMax(pCat->GetMaximum(i));
            pMF->SetValue(pCat->GetValue(i));

            SetHelpId(*pMF, aCatMf2Hid[nCategory][i]);
        }
    }
    // nun noch die CheckBox und das zugehoerige MetricField genau dann aktivieren,
    // falls es sich um das Klammer Menu handelt.
    bActive = nCategory == 5;
    aCheckBox1.Show(bActive);
    aCheckBox1.Enable(bActive);
    if (bActive)
    {
        aCheckBox1.Check( bScaleAllBrackets );

        bool bChecked = aCheckBox1.IsChecked();
        aFixedText4  .Enable( bChecked );
        aMetricField4.Enable( bChecked );
    }

    aMenuButton.GetPopupMenu()->CheckItem(nCategory + 1, true);
    aFixedLine.SetText(Categories[nCategory]->GetName());

    nActiveCategory = nCategory;

    aMetricField1.GrabFocus();
    Invalidate();
    Update();
}


SmDistanceDialog::SmDistanceDialog(Window *pParent, bool bFreeRes)
    : ModalDialog(pParent, SmResId(RID_DISTANCEDIALOG)),
    aFixedText1    (this, SmResId(1)),
    aMetricField1  (this, SmResId(1)),
    aFixedText2    (this, SmResId(2)),
    aMetricField2  (this, SmResId(2)),
    aFixedText3    (this, SmResId(3)),
    aMetricField3  (this, SmResId(3)),
    aCheckBox1     (this, SmResId(1)),
    aFixedText4    (this, SmResId(4)),
    aMetricField4  (this, SmResId(4)),
    aOKButton1     (this, SmResId(1)),
    aCancelButton1 (this, SmResId(1)),
    aMenuButton    (this, SmResId(1)),
    aDefaultButton (this, SmResId(1)),
    aBitmap        (this, SmResId(1)),
    aFixedLine     (this, SmResId(1))
{
    for (sal_uInt16 i = 0; i < NOCATEGORIES; i++)
        Categories[i] = new SmCategoryDesc(SmResId(i + 1), i);
    nActiveCategory   = CATEGORY_NONE;
    bScaleAllBrackets = false;

    if (bFreeRes)
        FreeResource();

    // preview like controls should have a 2D look
    aBitmap.SetBorderStyle( WINDOW_BORDER_MONO );

    aMetricField1.SetGetFocusHdl(LINK(this, SmDistanceDialog, GetFocusHdl));
    aMetricField2.SetGetFocusHdl(LINK(this, SmDistanceDialog, GetFocusHdl));
    aMetricField3.SetGetFocusHdl(LINK(this, SmDistanceDialog, GetFocusHdl));
    aMetricField4.SetGetFocusHdl(LINK(this, SmDistanceDialog, GetFocusHdl));
    aCheckBox1.SetClickHdl(LINK(this, SmDistanceDialog, CheckBoxClickHdl));

    aMenuButton.GetPopupMenu()->SetSelectHdl(LINK(this, SmDistanceDialog, MenuSelectHdl));

    aDefaultButton.SetClickHdl(LINK(this, SmDistanceDialog, DefaultButtonClickHdl));
}


SmDistanceDialog::~SmDistanceDialog()
{
    for (int i = 0; i < NOCATEGORIES; i++)
        DELETEZ(Categories[i]);
}

void SmDistanceDialog::DataChanged( const DataChangedEvent &rEvt )
{
    ModalDialog::DataChanged( rEvt );
}

void SmDistanceDialog::ReadFrom(const SmFormat &rFormat)
{
    Categories[0]->SetValue(0, rFormat.GetDistance(DIS_HORIZONTAL));
    Categories[0]->SetValue(1, rFormat.GetDistance(DIS_VERTICAL));
    Categories[0]->SetValue(2, rFormat.GetDistance(DIS_ROOT));
    Categories[1]->SetValue(0, rFormat.GetDistance(DIS_SUPERSCRIPT));
    Categories[1]->SetValue(1, rFormat.GetDistance(DIS_SUBSCRIPT));
    Categories[2]->SetValue(0, rFormat.GetDistance(DIS_NUMERATOR));
    Categories[2]->SetValue(1, rFormat.GetDistance(DIS_DENOMINATOR));
    Categories[3]->SetValue(0, rFormat.GetDistance(DIS_FRACTION));
    Categories[3]->SetValue(1, rFormat.GetDistance(DIS_STROKEWIDTH));
    Categories[4]->SetValue(0, rFormat.GetDistance(DIS_UPPERLIMIT));
    Categories[4]->SetValue(1, rFormat.GetDistance(DIS_LOWERLIMIT));
    Categories[5]->SetValue(0, rFormat.GetDistance(DIS_BRACKETSIZE));
    Categories[5]->SetValue(1, rFormat.GetDistance(DIS_BRACKETSPACE));
    Categories[5]->SetValue(3, rFormat.GetDistance(DIS_NORMALBRACKETSIZE));
    Categories[6]->SetValue(0, rFormat.GetDistance(DIS_MATRIXROW));
    Categories[6]->SetValue(1, rFormat.GetDistance(DIS_MATRIXCOL));
    Categories[7]->SetValue(0, rFormat.GetDistance(DIS_ORNAMENTSIZE));
    Categories[7]->SetValue(1, rFormat.GetDistance(DIS_ORNAMENTSPACE));
    Categories[8]->SetValue(0, rFormat.GetDistance(DIS_OPERATORSIZE));
    Categories[8]->SetValue(1, rFormat.GetDistance(DIS_OPERATORSPACE));
    Categories[9]->SetValue(0, rFormat.GetDistance(DIS_LEFTSPACE));
    Categories[9]->SetValue(1, rFormat.GetDistance(DIS_RIGHTSPACE));
    Categories[9]->SetValue(2, rFormat.GetDistance(DIS_TOPSPACE));
    Categories[9]->SetValue(3, rFormat.GetDistance(DIS_BOTTOMSPACE));

    bScaleAllBrackets = rFormat.IsScaleNormalBrackets();

    // force update (even of category 0) by setting nActiveCategory to a
    // non-existent category number
    nActiveCategory = CATEGORY_NONE;
    SetCategory(0);
}


void SmDistanceDialog::WriteTo(SmFormat &rFormat) /*const*/
{
    // hmm... koennen die tatsaechlich unterschiedlich sein?
    // wenn nicht kann oben naemlich das const stehen!
    SetCategory(nActiveCategory);

    rFormat.SetDistance( DIS_HORIZONTAL,        Categories[0]->GetValue(0) );
    rFormat.SetDistance( DIS_VERTICAL,          Categories[0]->GetValue(1) );
    rFormat.SetDistance( DIS_ROOT,              Categories[0]->GetValue(2) );
    rFormat.SetDistance( DIS_SUPERSCRIPT,       Categories[1]->GetValue(0) );
    rFormat.SetDistance( DIS_SUBSCRIPT,         Categories[1]->GetValue(1) );
    rFormat.SetDistance( DIS_NUMERATOR,         Categories[2]->GetValue(0) );
    rFormat.SetDistance( DIS_DENOMINATOR,       Categories[2]->GetValue(1) );
    rFormat.SetDistance( DIS_FRACTION,          Categories[3]->GetValue(0) );
    rFormat.SetDistance( DIS_STROKEWIDTH,       Categories[3]->GetValue(1) );
    rFormat.SetDistance( DIS_UPPERLIMIT,        Categories[4]->GetValue(0) );
    rFormat.SetDistance( DIS_LOWERLIMIT,        Categories[4]->GetValue(1) );
    rFormat.SetDistance( DIS_BRACKETSIZE,       Categories[5]->GetValue(0) );
    rFormat.SetDistance( DIS_BRACKETSPACE,      Categories[5]->GetValue(1) );
    rFormat.SetDistance( DIS_MATRIXROW,         Categories[6]->GetValue(0) );
    rFormat.SetDistance( DIS_MATRIXCOL,         Categories[6]->GetValue(1) );
    rFormat.SetDistance( DIS_ORNAMENTSIZE,      Categories[7]->GetValue(0) );
    rFormat.SetDistance( DIS_ORNAMENTSPACE,     Categories[7]->GetValue(1) );
    rFormat.SetDistance( DIS_OPERATORSIZE,      Categories[8]->GetValue(0) );
    rFormat.SetDistance( DIS_OPERATORSPACE,     Categories[8]->GetValue(1) );
    rFormat.SetDistance( DIS_LEFTSPACE,         Categories[9]->GetValue(0) );
    rFormat.SetDistance( DIS_RIGHTSPACE,        Categories[9]->GetValue(1) );
    rFormat.SetDistance( DIS_TOPSPACE,          Categories[9]->GetValue(2) );
    rFormat.SetDistance( DIS_BOTTOMSPACE,       Categories[9]->GetValue(3) );
    rFormat.SetDistance( DIS_NORMALBRACKETSIZE, Categories[5]->GetValue(3) );

    rFormat.SetScaleNormalBrackets( bScaleAllBrackets );

    rFormat.RequestApplyChanges();
}


/**************************************************************************/


IMPL_LINK( SmAlignDialog, DefaultButtonClickHdl, Button *, EMPTYARG /*pButton*/ )
{
   QueryBox *pQueryBox = new QueryBox(this, SmResId(RID_DEFAULTSAVEQUERY));

    if (pQueryBox->Execute() == RET_YES)
    {
        SmModule *pp = SM_MOD();
        SmFormat aFmt( pp->GetConfig()->GetStandardFormat() );
        WriteTo( aFmt );
        pp->GetConfig()->SetStandardFormat( aFmt );
    }

    delete pQueryBox;
    return 0;
}


SmAlignDialog::SmAlignDialog(Window * pParent, bool bFreeRes)
    : ModalDialog(pParent, SmResId(RID_ALIGNDIALOG)),
    aLeft          (this, SmResId(1)),
    aCenter        (this, SmResId(2)),
    aRight         (this, SmResId(3)),
    aFixedLine1    (this, SmResId(1)),
    aOKButton1     (this, SmResId(1)),
    aCancelButton1 (this, SmResId(1)),
    aDefaultButton (this, SmResId(1))
{
    if (bFreeRes)
        FreeResource();

    aDefaultButton.SetClickHdl(LINK(this, SmAlignDialog, DefaultButtonClickHdl));
}


void SmAlignDialog::ReadFrom(const SmFormat &rFormat)
{
    switch (rFormat.GetHorAlign())
    {
        case AlignLeft:
            aLeft  .Check(true);
            aCenter.Check(false);
            aRight .Check(false);
            break;

        case AlignCenter:
            aLeft  .Check(false);
            aCenter.Check(true);
            aRight .Check(false);
            break;

        case AlignRight:
            aLeft  .Check(false);
            aCenter.Check(false);
            aRight .Check(true);
            break;
    }
}


void SmAlignDialog::WriteTo(SmFormat &rFormat) const
{
    if (aLeft.IsChecked())
        rFormat.SetHorAlign(AlignLeft);
    else if (aRight.IsChecked())
        rFormat.SetHorAlign(AlignRight);
    else
        rFormat.SetHorAlign(AlignCenter);

    rFormat.RequestApplyChanges();
}


/**************************************************************************/


void SmShowSymbolSet::Paint(const Rectangle&)
{
    Push(PUSH_MAPMODE);

    // MapUnit einstellen fuer die 'nLen' berechnet wurde
    SetMapMode(MapMode(MAP_PIXEL));

    sal_uInt16 v        = sal::static_int_cast< sal_uInt16 >((aVScrollBar.GetThumbPos() * nColumns));
    size_t nSymbols = aSymbolSet.size();

    Color aTxtColor( GetTextColor() );
    for (sal_uInt16 i = v; i < nSymbols ; i++)
    {
        SmSym    aSymbol (*aSymbolSet[i]);
        Font     aFont   (aSymbol.GetFace());
        aFont.SetAlign(ALIGN_TOP);

        // etwas kleinere FontSize nehmen (als nLen) um etwas Luft zu haben
        // (hoffentlich auch genug fuer links und rechts!)
        aFont.SetSize(Size(0, nLen - (nLen / 3)));
        SetFont(aFont);
        // keep text color
        SetTextColor( aTxtColor );

        int   nIV   = i - v;
        sal_UCS4 cChar = aSymbol.GetCharacter();
        String aText( OUString( &cChar, 1 ) );
        Size  aSize( GetTextWidth( aText ), GetTextHeight());

        DrawText(Point((nIV % nColumns) * nLen + (nLen - aSize.Width()) / 2,
                       (nIV / nColumns) * nLen + (nLen - aSize.Height()) / 2),
                 aText);
    }

    if (nSelectSymbol != SYMBOL_NONE)
    {
        Invert(Rectangle(Point(((nSelectSymbol - v) % nColumns) * nLen,
                                 ((nSelectSymbol - v) / nColumns) * nLen),
                           Size(nLen, nLen)));
    }

    Pop();
}


void SmShowSymbolSet::MouseButtonDown(const MouseEvent& rMEvt)
{
    GrabFocus();

    if (rMEvt.IsLeft() && Rectangle(Point(0, 0), aOutputSize).IsInside(rMEvt.GetPosPixel()))
    {
        long nPos = (rMEvt.GetPosPixel().Y() / nLen) * nColumns + (rMEvt.GetPosPixel().X() / nLen) +
                      aVScrollBar.GetThumbPos() * nColumns;
        SelectSymbol( sal::static_int_cast< sal_uInt16 >(nPos) );

        aSelectHdlLink.Call(this);

        if (rMEvt.GetClicks() > 1) aDblClickHdlLink.Call(this);
    }
    else Control::MouseButtonDown (rMEvt);
}


void SmShowSymbolSet::KeyInput(const KeyEvent& rKEvt)
{
    sal_uInt16 n = nSelectSymbol;

    if (n != SYMBOL_NONE)
    {
        switch (rKEvt.GetKeyCode().GetCode())
        {
            case KEY_DOWN:      n = n + nColumns;   break;
            case KEY_UP:        n = n - nColumns;   break;
            case KEY_LEFT:      n -= 1; break;
            case KEY_RIGHT:     n += 1; break;
            case KEY_HOME:      n  = 0; break;
            case KEY_END:       n  = static_cast< sal_uInt16 >(aSymbolSet.size() - 1);   break;
            case KEY_PAGEUP:    n -= nColumns * nRows;  break;
            case KEY_PAGEDOWN:  n += nColumns * nRows;  break;

            default:
                Control::KeyInput(rKEvt);
                return;
        }
    }
    else
        n = 0;

    if (n >= aSymbolSet.size())
        n = nSelectSymbol;

    // adjust scrollbar
    if ((n < (sal_uInt16) (aVScrollBar.GetThumbPos() * nColumns)) ||
        (n >= (sal_uInt16) ((aVScrollBar.GetThumbPos() + nRows) * nColumns)))
    {
        aVScrollBar.SetThumbPos(n / nColumns);
        Invalidate();
        Update();
    }

    SelectSymbol(n);
    aSelectHdlLink.Call(this);
}


SmShowSymbolSet::SmShowSymbolSet(Window *pParent, const ResId& rResId) :
    Control(pParent, rResId),
    aVScrollBar(this, WinBits(WB_VSCROLL))
{
    nSelectSymbol = SYMBOL_NONE;

    aOutputSize = GetOutputSizePixel();
    long nScrollBarWidth = aVScrollBar.GetSizePixel().Width(),
         nUseableWidth   = aOutputSize.Width() - nScrollBarWidth;

    // Hoehe von 16pt in Pixeln (passend zu 'aOutputSize')
    nLen = (sal_uInt16) LogicToPixel(Size(0, 16), MapMode(MAP_POINT)).Height();

    nColumns = sal::static_int_cast< sal_uInt16 >(nUseableWidth / nLen);
    if (nColumns > 2  && nColumns % 2 != 0)
        nColumns--;
    nRows    = sal::static_int_cast< sal_uInt16 >(aOutputSize.Height() / nLen);
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(nColumns > 0, "Sm : no columns");
    OSL_ENSURE(nRows > 0, "Sm : no rows");
#endif

    // genau passend machen
    aOutputSize.Width()  = nColumns * nLen;
    aOutputSize.Height() = nRows * nLen;

    aVScrollBar.SetPosSizePixel(Point(aOutputSize.Width() + 1, -1),
                                Size(nScrollBarWidth, aOutputSize.Height() + 2));
    aVScrollBar.Enable(false);
    aVScrollBar.Show();
    aVScrollBar.SetScrollHdl(LINK(this, SmShowSymbolSet, ScrollHdl));

    Size WindowSize (aOutputSize);
    WindowSize.Width() += nScrollBarWidth;
    SetOutputSizePixel(WindowSize);

}


void SmShowSymbolSet::SetSymbolSet(const SymbolPtrVec_t& rSymbolSet)
{
    aSymbolSet = rSymbolSet;

    if (static_cast< sal_uInt16 >(aSymbolSet.size()) > (nColumns * nRows))
    {
        aVScrollBar.SetRange(Range(0, ((aSymbolSet.size() + (nColumns - 1)) / nColumns) - nRows));
        aVScrollBar.Enable(true);
    }
    else
    {
        aVScrollBar.SetRange(Range(0,0));
        aVScrollBar.Enable (false);
    }

    Invalidate();
}


void SmShowSymbolSet::SelectSymbol(sal_uInt16 nSymbol)
{
    int v = (int) (aVScrollBar.GetThumbPos() * nColumns);

    if (nSelectSymbol != SYMBOL_NONE)
        Invalidate(Rectangle(Point(((nSelectSymbol - v) % nColumns) * nLen,
                                   ((nSelectSymbol - v) / nColumns) * nLen),
                             Size(nLen, nLen)));

    if (nSymbol < aSymbolSet.size())
        nSelectSymbol = nSymbol;

    if (aSymbolSet.size() == 0)
        nSelectSymbol = SYMBOL_NONE;

    if (nSelectSymbol != SYMBOL_NONE)
        Invalidate(Rectangle(Point(((nSelectSymbol - v) % nColumns) * nLen,
                                   ((nSelectSymbol - v) / nColumns) * nLen),
                             Size(nLen, nLen)));

    Update();
}


IMPL_LINK( SmShowSymbolSet, ScrollHdl, ScrollBar*, EMPTYARG /*pScrollBar*/)
{
    Invalidate();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

void SmShowSymbol::Paint(const Rectangle &rRect)
{
    Control::Paint( rRect );

    const XubString &rText = GetText();
    Size            aTextSize(GetTextWidth(rText), GetTextHeight());

    DrawText(Point((GetOutputSize().Width()  - aTextSize.Width())  / 2,
                   (GetOutputSize().Height() * 7/10)), rText);
}


void SmShowSymbol::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (rMEvt.GetClicks() > 1)
        aDblClickHdlLink.Call(this);
    else
        Control::MouseButtonDown (rMEvt);
}


void SmShowSymbol::SetSymbol(const SmSym *pSymbol)
{
    if (pSymbol)
    {
        Font aFont (pSymbol->GetFace());
        aFont.SetSize(Size(0, GetOutputSize().Height() - GetOutputSize().Height() / 3));
        aFont.SetAlign(ALIGN_BASELINE);
        SetFont(aFont);

        sal_UCS4 cChar = pSymbol->GetCharacter();
        String aText( OUString( &cChar, 1 ) );
        SetText( aText );
    }

    // 'Invalidate' fuellt den background mit der background-Farbe.
    // Falls der NULL pointer uebergeben wurde reicht dies also zum loeschen
    // der Anzeige
    Invalidate();
}


////////////////////////////////////////////////////////////////////////////////

void SmSymbolDialog::FillSymbolSets(bool bDeleteText)
    // fuellt die Eintraege der moeglichen 'SymbolsSet's im Dialog mit den
    // aktuellen Werten des SymbolSet Managers, selektiert aber keinen.
{
    aSymbolSets.Clear();
    if (bDeleteText)
        aSymbolSets.SetNoSelection();

    std::set< String >  aSybolSetNames( rSymbolMgr.GetSymbolSetNames() );
    std::set< String >::const_iterator aIt( aSybolSetNames.begin() );
    for ( ; aIt != aSybolSetNames.end(); ++aIt)
        aSymbolSets.InsertEntry( *aIt );
}


IMPL_LINK( SmSymbolDialog, SymbolSetChangeHdl, ListBox *, EMPTYARG pListBox )
{
    (void) pListBox;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pListBox == &aSymbolSets, "Sm : wrong argument");
#endif

    SelectSymbolSet(aSymbolSets.GetSelectEntry());
    return 0;
}


IMPL_LINK( SmSymbolDialog, SymbolChangeHdl, SmShowSymbolSet *, EMPTYARG pShowSymbolSet )
{
    (void) pShowSymbolSet;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pShowSymbolSet == &aSymbolSetDisplay, "Sm : wrong argument");
#endif

    SelectSymbol(aSymbolSetDisplay.GetSelectSymbol());
    return 0;
}

IMPL_LINK( SmSymbolDialog, EditClickHdl, Button *, EMPTYARG pButton )
{
    (void) pButton;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pButton == &aEditBtn, "Sm : wrong argument");
#endif

    SmSymDefineDialog *pDialog = new SmSymDefineDialog(this, pFontListDev, rSymbolMgr);

    // aktuelles Symbol und SymbolSet am neuen Dialog setzen
    const XubString  aSymSetName (aSymbolSets.GetSelectEntry()),
                    aSymName    (aSymbolName.GetText());
    pDialog->SelectOldSymbolSet(aSymSetName);
    pDialog->SelectOldSymbol(aSymName);
    pDialog->SelectSymbolSet(aSymSetName);
    pDialog->SelectSymbol(aSymName);

    // altes SymbolSet merken
    XubString  aOldSymbolSet (aSymbolSets.GetSelectEntry());

    sal_uInt16 nSymPos = GetSelectedSymbol();

    // Dialog an evtl geaenderte Daten des SymbolSet Manager anpassen
    if (pDialog->Execute() == RET_OK  &&  rSymbolMgr.IsModified())
    {
        rSymbolMgr.Save();
        FillSymbolSets();
    }

    // wenn das alte SymbolSet nicht mehr existiert zum ersten gehen
    // (soweit eines vorhanden ist)
    if (!SelectSymbolSet(aOldSymbolSet)  &&  aSymbolSets.GetEntryCount() > 0)
        SelectSymbolSet(aSymbolSets.GetEntry(0));
    else
    {
        // just update display of current symbol set
        OSL_ENSURE( aSymSetName == aSymSetName, "unexpected change in symbol set name" );
           aSymbolSet      = rSymbolMgr.GetSymbolSet( aSymbolSetName );
        aSymbolSetDisplay.SetSymbolSet( aSymbolSet );
    }

    if (nSymPos >= aSymbolSet.size())
        nSymPos = static_cast< sal_uInt16 >(aSymbolSet.size()) - 1;
    SelectSymbol( nSymPos );

    delete pDialog;
    return 0;
}


IMPL_LINK( SmSymbolDialog, SymbolDblClickHdl, SmShowSymbolSet *, EMPTYARG pShowSymbolSet )
{
    (void) pShowSymbolSet;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pShowSymbolSet == &aSymbolSetDisplay, "Sm : wrong argument");
#endif

    GetClickHdl(&aGetBtn);
    EndDialog(RET_OK);
    return 0;
}


IMPL_LINK( SmSymbolDialog, GetClickHdl, Button *, EMPTYARG pButton )
{
    (void) pButton;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pButton == &aGetBtn, "Sm : wrong button");
#endif

    const SmSym *pSym = GetSymbol();
    if (pSym)
    {
        String  aText ('%');
        aText += pSym->GetName();
        aText += (sal_Unicode)' ';

        rViewSh.GetViewFrame()->GetDispatcher()->Execute(
                SID_INSERTSYMBOL, SFX_CALLMODE_STANDARD,
                new SfxStringItem(SID_INSERTSYMBOL, aText), 0L);
    }

    return 0;
}


IMPL_LINK_INLINE_START( SmSymbolDialog, CloseClickHdl, Button *, EMPTYARG pButton )
{
    (void) pButton;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pButton == &aCloseBtn, "Sm : wrong button");
#endif

    EndDialog(true);
    return 0;
}
IMPL_LINK_INLINE_END( SmSymbolDialog, CloseClickHdl, Button *, pButton )


SmSymbolDialog::SmSymbolDialog(Window *pParent, OutputDevice *pFntListDevice,
        SmSymbolManager &rMgr, SmViewShell &rViewShell, bool bFreeRes) :
    ModalDialog         (pParent, SmResId(RID_SYMBOLDIALOG)),
    aSymbolSetText      (this, SmResId(1)),
    aSymbolSets         (this, SmResId(1)),
    aSymbolSetDisplay   (this, SmResId(1)),
    aSymbolName         (this, SmResId(2)),
    aSymbolDisplay      (this, SmResId(2)),
    aGetBtn             (this, SmResId(2)),
    aCloseBtn           (this, SmResId(3)),
    aEditBtn            (this, SmResId(1)),
    rViewSh             (rViewShell),
    rSymbolMgr          (rMgr),
    pFontListDev        (pFntListDevice)
{
    if (bFreeRes)
        FreeResource();

    aSymbolSetName = String();
    aSymbolSet.clear();
    FillSymbolSets();
    if (aSymbolSets.GetEntryCount() > 0)
        SelectSymbolSet(aSymbolSets.GetEntry(0));

    InitColor_Impl();

    // preview like controls should have a 2D look
    aSymbolDisplay.SetBorderStyle( WINDOW_BORDER_MONO );

    aSymbolSets      .SetSelectHdl  (LINK(this, SmSymbolDialog, SymbolSetChangeHdl));
    aSymbolSetDisplay.SetSelectHdl  (LINK(this, SmSymbolDialog, SymbolChangeHdl));
    aSymbolSetDisplay.SetDblClickHdl(LINK(this, SmSymbolDialog, SymbolDblClickHdl));
    aSymbolDisplay   .SetDblClickHdl(LINK(this, SmSymbolDialog, SymbolDblClickHdl));
    aCloseBtn        .SetClickHdl   (LINK(this, SmSymbolDialog, CloseClickHdl));
    aEditBtn         .SetClickHdl   (LINK(this, SmSymbolDialog, EditClickHdl));
    aGetBtn          .SetClickHdl   (LINK(this, SmSymbolDialog, GetClickHdl));
}


SmSymbolDialog::~SmSymbolDialog()
{
}


void SmSymbolDialog::InitColor_Impl()
{
#if OSL_DEBUG_LEVEL > 1
    Color aBC( GetDisplayBackground().GetColor() );
#endif
    ColorData   nBgCol  = COL_WHITE,
                nTxtCol = COL_BLACK;
    const StyleSettings &rS = GetSettings().GetStyleSettings();
    if (rS.GetHighContrastMode())
    {
        nBgCol  = rS.GetFieldColor().GetColor();
        nTxtCol = rS.GetFieldTextColor().GetColor();
    }

    Color aTmpColor( nBgCol );
    Wallpaper aWall( aTmpColor );
    Color aTxtColor( nTxtCol );
    aSymbolDisplay   .SetBackground( aWall );
    aSymbolDisplay   .SetTextColor( aTxtColor );
    aSymbolSetDisplay.SetBackground( aWall );
    aSymbolSetDisplay.SetTextColor( aTxtColor );
}


void SmSymbolDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DATACHANGED_SETTINGS  &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            InitColor_Impl();

    ModalDialog::DataChanged( rDCEvt );
}


bool SmSymbolDialog::SelectSymbolSet(const XubString &rSymbolSetName)
{
    bool    bRet = false;
    sal_uInt16  nPos = aSymbolSets.GetEntryPos(rSymbolSetName);

    aSymbolSetName = String();
    aSymbolSet.clear();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        aSymbolSets.SelectEntryPos(nPos);

        aSymbolSetName  = rSymbolSetName;
        aSymbolSet      = rSymbolMgr.GetSymbolSet( aSymbolSetName );

        // sort symbols by Unicode position (useful for displaying Greek characters alphabetically)
        std::sort( aSymbolSet.begin(), aSymbolSet.end(), lt_SmSymPtr() );

        aSymbolSetDisplay.SetSymbolSet( aSymbolSet );
        if (aSymbolSet.size() > 0)
            SelectSymbol(0);

        bRet = true;
    }
    else
        aSymbolSets.SetNoSelection();

    return bRet;
}


void SmSymbolDialog::SelectSymbol(sal_uInt16 nSymbolNo)
{
    const SmSym *pSym = NULL;
    if (aSymbolSetName.Len() > 0  &&  nSymbolNo < static_cast< sal_uInt16 >(aSymbolSet.size()))
        pSym = aSymbolSet[ nSymbolNo ];

    aSymbolSetDisplay.SelectSymbol(nSymbolNo);
    aSymbolDisplay.SetSymbol(pSym);
    aSymbolName.SetText(pSym ? pSym->GetName() : XubString());
}


const SmSym * SmSymbolDialog::GetSymbol() const
{
    sal_uInt16 nSymbolNo = aSymbolSetDisplay.GetSelectSymbol();
    bool bValid = aSymbolSetName.Len() > 0  &&  nSymbolNo < static_cast< sal_uInt16 >(aSymbolSet.size());
    return bValid ? aSymbolSet[ nSymbolNo ] : NULL;
}


////////////////////////////////////////////////////////////////////////////////


void SmShowChar::Paint(const Rectangle &rRect)
{
    Control::Paint( rRect );

    OUString aText( GetText() );
    if (aText.getLength() > 0)
    {
#if OSL_DEBUG_LEVEL > 1
        sal_Int32 nPos = 0;
        sal_UCS4 cChar = aText.iterateCodePoints( &nPos );
        (void) cChar;
#endif
        Size aTextSize(GetTextWidth(aText), GetTextHeight());

        DrawText(Point((GetOutputSize().Width()  - aTextSize.Width())  / 2,
                       (GetOutputSize().Height() * 7/10)), aText);
    }
}


void SmShowChar::SetSymbol( const SmSym *pSym )
{
    if (pSym)
        SetSymbol( pSym->GetCharacter(), pSym->GetFace() );
}


void SmShowChar::SetSymbol( sal_UCS4 cChar, const Font &rFont )
{
    Font aFont( rFont );
    aFont.SetSize( Size(0, GetOutputSize().Height() - GetOutputSize().Height() / 3) );
    aFont.SetAlign(ALIGN_BASELINE);
    SetFont(aFont);
    aFont.SetTransparent(true);

    String aText( OUString( &cChar, 1) );
    SetText( aText );

    Invalidate();
}


////////////////////////////////////////////////////////////////////////////////

void SmSymDefineDialog::FillSymbols(ComboBox &rComboBox, bool bDeleteText)
{
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(&rComboBox == &aOldSymbols  ||  &rComboBox == &aSymbols,
        "Sm : wrong ComboBox");
#endif

    rComboBox.Clear();
    if (bDeleteText)
        rComboBox.SetText(XubString());

    ComboBox &rBox = &rComboBox == &aOldSymbols ? aOldSymbolSets : aSymbolSets;
    SymbolPtrVec_t aSymSet( aSymbolMgrCopy.GetSymbolSet( rBox.GetText() ) );
    for (size_t i = 0;  i < aSymSet.size();  ++i)
        rComboBox.InsertEntry( aSymSet[i]->GetName() );
}


void SmSymDefineDialog::FillSymbolSets(ComboBox &rComboBox, bool bDeleteText)
{
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(&rComboBox == &aOldSymbolSets  ||  &rComboBox == &aSymbolSets,
        "Sm : falsche ComboBox");
#endif

    rComboBox.Clear();
    if (bDeleteText)
        rComboBox.SetText(XubString());

    const std::set< String >  aSymbolSetNames( aSymbolMgrCopy.GetSymbolSetNames() );
    std::set< String >::const_iterator aIt( aSymbolSetNames.begin() );
    for ( ;  aIt != aSymbolSetNames.end();  ++aIt)
        rComboBox.InsertEntry( *aIt );
}


void SmSymDefineDialog::FillFonts(bool bDelete)
{
    aFonts.Clear();
    if (bDelete)
        aFonts.SetNoSelection();

    // alle Fonts der 'FontList' in die Fontliste aufnehmen
    // von denen mit gleichen Namen jedoch nur einen (denn der Style wird
    // ueber die 'FontStyleBox' gewaehlt und nicht auch noch hier)
    if (pFontList)
    {
        sal_uInt16  nCount = pFontList->GetFontNameCount();
        for (sal_uInt16 i = 0;  i < nCount;  i++)
            aFonts.InsertEntry( pFontList->GetFontName(i).GetName() );
    }
}


void SmSymDefineDialog::FillStyles(bool bDeleteText)
{
    aStyles.Clear();
    if (bDeleteText)
        aStyles.SetText(XubString());

    XubString aText (aFonts.GetSelectEntry());
    if (aText.Len() != 0)
    {
        // eigene StyleName's verwenden
        const SmFontStyles &rStyles = GetFontStyles();
        for (sal_uInt16 i = 0;  i < rStyles.GetCount();  i++)
            aStyles.InsertEntry( rStyles.GetStyleName(i) );

#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE(aStyles.GetEntryCount() > 0, "Sm : no styles available");
#endif
        aStyles.SetText( aStyles.GetEntry(0) );
    }
}


SmSym * SmSymDefineDialog::GetSymbol(const ComboBox &rComboBox)
{
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(&rComboBox == &aOldSymbols  ||  &rComboBox == &aSymbols,
        "Sm : wrong combobox");
#endif
    return aSymbolMgrCopy.GetSymbolByName(rComboBox.GetText());
}


IMPL_LINK( SmSymDefineDialog, OldSymbolChangeHdl, ComboBox *, EMPTYARG pComboBox )
{
    (void) pComboBox;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pComboBox == &aOldSymbols, "Sm : wrong argument");
#endif
    SelectSymbol(aOldSymbols, aOldSymbols.GetText(), false);
    return 0;
}


IMPL_LINK( SmSymDefineDialog, OldSymbolSetChangeHdl, ComboBox *, EMPTYARG pComboBox )
{
    (void) pComboBox;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pComboBox == &aOldSymbolSets, "Sm : wrong argument");
#endif
    SelectSymbolSet(aOldSymbolSets, aOldSymbolSets.GetText(), false);
    return 0;
}


IMPL_LINK( SmSymDefineDialog, ModifyHdl, ComboBox *, pComboBox )
{
    // merken der Cursorposition zum wiederherstellen derselben
    Selection  aSelection (pComboBox->GetSelection());

    if (pComboBox == &aSymbols)
        SelectSymbol(aSymbols, aSymbols.GetText(), false);
    else if (pComboBox == &aSymbolSets)
        SelectSymbolSet(aSymbolSets, aSymbolSets.GetText(), false);
    else if (pComboBox == &aOldSymbols)
        // nur Namen aus der Liste erlauben
        SelectSymbol(aOldSymbols, aOldSymbols.GetText(), true);
    else if (pComboBox == &aOldSymbolSets)
        // nur Namen aus der Liste erlauben
        SelectSymbolSet(aOldSymbolSets, aOldSymbolSets.GetText(), true);
    else if (pComboBox == &aStyles)
        // nur Namen aus der Liste erlauben (ist hier eh immer der Fall)
        SelectStyle(aStyles.GetText(), true);
    else
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_FAIL("Sm : wrong combobox argument");
#endif
    }

    pComboBox->SetSelection(aSelection);

    UpdateButtons();

    return 0;
}


IMPL_LINK( SmSymDefineDialog, FontChangeHdl, ListBox *, EMPTYARG pListBox )
{
    (void) pListBox;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pListBox == &aFonts, "Sm : wrong argument");
#endif

    SelectFont(aFonts.GetSelectEntry());
    return 0;
}


IMPL_LINK( SmSymDefineDialog, SubsetChangeHdl, ListBox *, EMPTYARG pListBox )
{
    (void) pListBox;
    sal_uInt16 nPos = aFontsSubsetLB.GetSelectEntryPos();
    if (LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        const Subset* pSubset = reinterpret_cast<const Subset*> (aFontsSubsetLB.GetEntryData( nPos ));
        if (pSubset)
        {
            aCharsetDisplay.SelectCharacter( pSubset->GetRangeMin() );
        }
    }
    return 0;
}


IMPL_LINK( SmSymDefineDialog, StyleChangeHdl, ComboBox *, EMPTYARG pComboBox )
{
    (void) pComboBox;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pComboBox == &aStyles, "Sm : falsches Argument");
#endif

    SelectStyle(aStyles.GetText());
    return 0;
}


IMPL_LINK( SmSymDefineDialog, CharHighlightHdl, Control *, EMPTYARG )
{
   sal_UCS4 cChar = aCharsetDisplay.GetSelectCharacter();

#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE( pSubsetMap, "SubsetMap missing" );
#endif
    if (pSubsetMap)
    {
        const Subset* pSubset = pSubsetMap->GetSubsetByUnicode( cChar );
        if (pSubset)
            aFontsSubsetLB.SelectEntry( pSubset->GetName() );
        else
            aFontsSubsetLB.SetNoSelection();
    }

    aSymbolDisplay.SetSymbol( cChar, aCharsetDisplay.GetFont() );

    UpdateButtons();

    // display Unicode position as symbol name while iterating over characters
    const String aHex( String::CreateFromInt64( cChar, 16 ).ToUpperAscii() );
    const String aPattern( A2OU( aHex.Len() > 4 ? "Ux000000" : "Ux0000" ) );
    String aUnicodePos( aPattern.Copy( 0, aPattern.Len() - aHex.Len() ) );
    aUnicodePos += aHex;
    aSymbols.SetText( aUnicodePos );
    aSymbolName.SetText( aUnicodePos );

    return 0;
}


IMPL_LINK( SmSymDefineDialog, AddClickHdl, Button *, EMPTYARG pButton )
{
    (void) pButton;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pButton == &aAddBtn, "Sm : wrong argument");
    OSL_ENSURE(aAddBtn.IsEnabled(), "Sm : requirements met ??");
#endif

    // add symbol
    const SmSym aNewSymbol( aSymbols.GetText(), aCharsetDisplay.GetFont(),
            aCharsetDisplay.GetSelectCharacter(), aSymbolSets.GetText() );
    //OSL_ENSURE( aSymbolMgrCopy.GetSymbolByName(aTmpSymbolName) == NULL, "symbol already exists" );
    aSymbolMgrCopy.AddOrReplaceSymbol( aNewSymbol );

    // update display of new symbol
    aSymbolDisplay.SetSymbol( &aNewSymbol );
    aSymbolName.SetText( aNewSymbol.GetName() );
    aSymbolSetName.SetText( aNewSymbol.GetSymbolSetName() );

    // update list box entries
    FillSymbolSets(aOldSymbolSets, false);
    FillSymbolSets(aSymbolSets,    false);
    FillSymbols(aOldSymbols ,false);
    FillSymbols(aSymbols    ,false);

    UpdateButtons();

    return 0;
}


IMPL_LINK( SmSymDefineDialog, ChangeClickHdl, Button *, EMPTYARG pButton )
{
    (void) pButton;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pButton == &aChangeBtn, "Sm : wrong argument");
    OSL_ENSURE(aChangeBtn.IsEnabled(), "Sm : requirements met ??");
#endif

    // get new Sybol to use
    //! get font from symbol-disp lay since charset-display does not keep
    //! the bold attribut.
    const SmSym aNewSymbol( aSymbols.GetText(), aCharsetDisplay.GetFont(),
            aCharsetDisplay.GetSelectCharacter(), aSymbolSets.GetText() );

    // remove old symbol if the name was changed then add new one
    const bool bNameChanged       = aOldSymbols.GetText() != aSymbols.GetText();
    if (bNameChanged)
        aSymbolMgrCopy.RemoveSymbol( aOldSymbols.GetText() );
    aSymbolMgrCopy.AddOrReplaceSymbol( aNewSymbol, true );

    // clear display for original symbol if necessary
    if (bNameChanged)
        SetOrigSymbol(NULL, XubString());

    // update display of new symbol
    aSymbolDisplay.SetSymbol( &aNewSymbol );
    aSymbolName.SetText( aNewSymbol.GetName() );
    aSymbolSetName.SetText( aNewSymbol.GetSymbolSetName() );

    // update list box entries
    FillSymbolSets(aOldSymbolSets, false);
    FillSymbolSets(aSymbolSets,    false);
    FillSymbols(aOldSymbols ,false);
    FillSymbols(aSymbols    ,false);

    UpdateButtons();

    return 0;
}


IMPL_LINK( SmSymDefineDialog, DeleteClickHdl, Button *, EMPTYARG pButton )
{
    (void) pButton;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pButton == &aDeleteBtn, "Sm : wrong argument");
    OSL_ENSURE(aDeleteBtn.IsEnabled(), "Sm : requirements met ??");
#endif

    if (pOrigSymbol)
    {
        aSymbolMgrCopy.RemoveSymbol( pOrigSymbol->GetName() );

        // clear display for original symbol
        SetOrigSymbol(NULL, XubString());

        // update list box entries
        FillSymbolSets(aOldSymbolSets, false);
        FillSymbolSets(aSymbolSets,    false);
        FillSymbols(aOldSymbols ,false);
        FillSymbols(aSymbols    ,false);
    }

    UpdateButtons();

    return 0;
}


void SmSymDefineDialog::UpdateButtons()
{
    bool  bAdd    = false,
          bChange = false,
          bDelete = false;
    XubString aTmpSymbolName    (aSymbols.GetText()),
              aTmpSymbolSetName (aSymbolSets.GetText());

    if (aTmpSymbolName.Len() > 0  &&  aTmpSymbolSetName.Len() > 0)
    {
        // alle Einstellungen gleich?
        //! (Font-, Style- und SymbolSet Name werden nicht case sensitiv verglichen)
        bool bEqual = pOrigSymbol
                    && aTmpSymbolSetName.EqualsIgnoreCaseAscii(aOldSymbolSetName.GetText())
                    && aTmpSymbolName.Equals(pOrigSymbol->GetName())
                    && aFonts.GetSelectEntry().EqualsIgnoreCaseAscii(
                            pOrigSymbol->GetFace().GetName())
                    && aStyles.GetText().EqualsIgnoreCaseAscii(
                            GetFontStyles().GetStyleName(pOrigSymbol->GetFace()))
                    && aCharsetDisplay.GetSelectCharacter() == pOrigSymbol->GetCharacter();

        // hinzufuegen nur wenn es noch kein Symbol desgleichen Namens gibt
        bAdd    = aSymbolMgrCopy.GetSymbolByName(aTmpSymbolName) == NULL;

        // loeschen nur wenn alle Einstellungen gleich sind
        bDelete = pOrigSymbol != NULL;

        // aendern nur falls altes Symbol vorhanden und am neuen etwas anders ist
        bChange = pOrigSymbol && !bEqual;
    }

    aAddBtn   .Enable(bAdd);
    aChangeBtn.Enable(bChange);
    aDeleteBtn.Enable(bDelete);
}


SmSymDefineDialog::SmSymDefineDialog(Window * pParent,
        OutputDevice *pFntListDevice, SmSymbolManager &rMgr, bool bFreeRes) :
    ModalDialog         (pParent, SmResId(RID_SYMDEFINEDIALOG)),
    aOldSymbolText      (this, SmResId(1)),
    aOldSymbols         (this, SmResId(1)),
    aOldSymbolSetText   (this, SmResId(2)),
    aOldSymbolSets      (this, SmResId(2)),
    aCharsetDisplay     (this, SmResId(1)),
    aSymbolText         (this, SmResId(9)),
    aSymbols            (this, SmResId(4)),
    aSymbolSetText      (this, SmResId(10)),
    aSymbolSets         (this, SmResId(5)),
    aFontText           (this, SmResId(3)),
    aFonts              (this, SmResId(1)),
    aFontsSubsetFT      (this, SmResId( FT_FONTS_SUBSET )),
    aFontsSubsetLB      (this, SmResId( LB_FONTS_SUBSET )),
    aStyleText          (this, SmResId(4)),
    aStyles             (this, SmResId(3)),
    aOldSymbolName      (this, SmResId(7)),
    aOldSymbolDisplay   (this, SmResId(3)),
    aOldSymbolSetName   (this, SmResId(8)),
    aSymbolName         (this, SmResId(5)),
    aSymbolDisplay      (this, SmResId(2)),
    aSymbolSetName      (this, SmResId(6)),
    aOkBtn              (this, SmResId(1)),
    aCancelBtn          (this, SmResId(1)),
    aAddBtn             (this, SmResId(1)),
    aChangeBtn          (this, SmResId(2)),
    aDeleteBtn          (this, SmResId(3)),
    aRightArrow         (this, SmResId(1)),
    aRigthArrow_Im      (SmResId(1)),
    rSymbolMgr          (rMgr),
    pSubsetMap          (NULL),
    pFontList           (NULL)
{
    if (bFreeRes)
        FreeResource();

    pFontList = new FontList( pFntListDevice );

    pOrigSymbol = 0;

    // auto completion is troublesome since that symbols character also gets automatically selected in the
    // display and if the user previously selected a character to define/redefine that one this is bad
   aOldSymbols.EnableAutocomplete( false, true );
   aSymbols   .EnableAutocomplete( false, true );

    FillFonts();
    if (aFonts.GetEntryCount() > 0)
        SelectFont(aFonts.GetEntry(0));

    InitColor_Impl();

    SetSymbolSetManager(rSymbolMgr);

    aOldSymbols    .SetSelectHdl(LINK(this, SmSymDefineDialog, OldSymbolChangeHdl));
    aOldSymbolSets .SetSelectHdl(LINK(this, SmSymDefineDialog, OldSymbolSetChangeHdl));
    aSymbolSets    .SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    aOldSymbolSets .SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    aSymbols       .SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    aOldSymbols    .SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    aStyles        .SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    aFonts         .SetSelectHdl(LINK(this, SmSymDefineDialog, FontChangeHdl));
    aFontsSubsetLB .SetSelectHdl(LINK(this, SmSymDefineDialog, SubsetChangeHdl));
    aStyles        .SetSelectHdl(LINK(this, SmSymDefineDialog, StyleChangeHdl));
    aAddBtn        .SetClickHdl (LINK(this, SmSymDefineDialog, AddClickHdl));
    aChangeBtn     .SetClickHdl (LINK(this, SmSymDefineDialog, ChangeClickHdl));
    aDeleteBtn     .SetClickHdl (LINK(this, SmSymDefineDialog, DeleteClickHdl));
    aCharsetDisplay.SetHighlightHdl( LINK( this, SmSymDefineDialog, CharHighlightHdl ) );

    // preview like controls should have a 2D look
    aOldSymbolDisplay.SetBorderStyle( WINDOW_BORDER_MONO );
    aSymbolDisplay   .SetBorderStyle( WINDOW_BORDER_MONO );
}


SmSymDefineDialog::~SmSymDefineDialog()
{
    delete pSubsetMap;
    delete pOrigSymbol;
}

void SmSymDefineDialog::InitColor_Impl()
{
#if OSL_DEBUG_LEVEL > 1
    Color aBC( GetDisplayBackground().GetColor() );
#endif
    ColorData   nBgCol  = COL_WHITE,
                nTxtCol = COL_BLACK;
    bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();
    if (bHighContrast)
    {
        const StyleSettings &rS = GetSettings().GetStyleSettings();
        nBgCol  = rS.GetFieldColor().GetColor();
        nTxtCol = rS.GetFieldTextColor().GetColor();
    }

    Color aTmpColor( nBgCol );
    Wallpaper aWall( aTmpColor );
    Color aTxtColor( nTxtCol );
    aCharsetDisplay  .SetBackground( aWall );
    aCharsetDisplay  .SetTextColor( aTxtColor );
    aOldSymbolDisplay.SetBackground( aWall );
    aOldSymbolDisplay.SetTextColor( aTxtColor );
    aSymbolDisplay   .SetBackground( aWall );
    aSymbolDisplay   .SetTextColor( aTxtColor );

    const Image &rArrowRight = aRigthArrow_Im;
    aRightArrow.SetImage( rArrowRight );
}


void SmSymDefineDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DATACHANGED_SETTINGS  &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            InitColor_Impl();

    ModalDialog::DataChanged( rDCEvt );
}


short SmSymDefineDialog::Execute()
{
    short nResult = ModalDialog::Execute();

    // Aenderungen uebernehmen falls Dialog mit OK beendet wurde
    if (aSymbolMgrCopy.IsModified()  &&  nResult == RET_OK)
        rSymbolMgr = aSymbolMgrCopy;

    return nResult;
}


void SmSymDefineDialog::SetSymbolSetManager(const SmSymbolManager &rMgr)
{
    aSymbolMgrCopy = rMgr;

    // Das modified Flag der Kopie auf false setzen, damit man spaeter damit
    // testen kann ob sich was geaendert hat.
    aSymbolMgrCopy.SetModified(false);

    FillSymbolSets(aOldSymbolSets);
    if (aOldSymbolSets.GetEntryCount() > 0)
        SelectSymbolSet(aOldSymbolSets.GetEntry(0));
    FillSymbolSets(aSymbolSets);
    if (aSymbolSets.GetEntryCount() > 0)
        SelectSymbolSet(aSymbolSets.GetEntry(0));
    FillSymbols(aOldSymbols);
    if (aOldSymbols.GetEntryCount() > 0)
        SelectSymbol(aOldSymbols.GetEntry(0));
    FillSymbols(aSymbols);
    if (aSymbols.GetEntryCount() > 0)
        SelectSymbol(aSymbols.GetEntry(0));

    UpdateButtons();
}


bool SmSymDefineDialog::SelectSymbolSet(ComboBox &rComboBox,
        const XubString &rSymbolSetName, bool bDeleteText)
{
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(&rComboBox == &aOldSymbolSets  ||  &rComboBox == &aSymbolSets,
        "Sm : wrong ComboBox");
#endif

    // 'Normalisieren' des SymbolNamens (ohne leading und trailing Leerzeichen)
    XubString  aNormName (rSymbolSetName);
    aNormName = comphelper::string::stripStart(aNormName, ' ');
    aNormName = comphelper::string::stripEnd(aNormName, ' ');
    // und evtl Abweichungen in der Eingabe beseitigen
    rComboBox.SetText(aNormName);

    bool   bRet = false;
    sal_uInt16 nPos = rComboBox.GetEntryPos(aNormName);

    if (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        rComboBox.SetText(rComboBox.GetEntry(nPos));
        bRet = true;
    }
    else if (bDeleteText)
        rComboBox.SetText(XubString());

    bool  bIsOld = &rComboBox == &aOldSymbolSets;

    // setzen des SymbolSet Namens an der zugehoerigen Darstellung
    FixedText &rFT = bIsOld ? aOldSymbolSetName : aSymbolSetName;
    rFT.SetText(rComboBox.GetText());

    // setzen der zum SymbolSet gehoerenden Symbol Namen an der zugehoerigen
    // Auswahbox
    ComboBox  &rCB = bIsOld ? aOldSymbols : aSymbols;
    FillSymbols(rCB, false);

    // bei Wechsel des SymbolSets fuer das alte Zeichen ein gueltiges
    // Symbol bzw keins zur Anzeige bringen
    if (bIsOld)
    {
        XubString  aTmpOldSymbolName;
        if (aOldSymbols.GetEntryCount() > 0)
            aTmpOldSymbolName = aOldSymbols.GetEntry(0);
        SelectSymbol(aOldSymbols, aTmpOldSymbolName, true);
    }

    UpdateButtons();

    return bRet;
}


void SmSymDefineDialog::SetOrigSymbol(const SmSym *pSymbol,
                                      const XubString &rSymbolSetName)
{
    // clear old symbol
    delete pOrigSymbol;
    pOrigSymbol = 0;

    XubString   aSymName,
                aSymSetName;
    if (pSymbol)
    {
        // set new symbol
        pOrigSymbol = new SmSym( *pSymbol );

        aSymName    = pSymbol->GetName();
        aSymSetName = rSymbolSetName;
        aOldSymbolDisplay.SetSymbol( pSymbol );
    }
    else
    {   // loeschen des angezeigten Symbols
        aOldSymbolDisplay.SetText(XubString());
        aOldSymbolDisplay.Invalidate();
    }
    aOldSymbolName   .SetText(aSymName);
    aOldSymbolSetName.SetText(aSymSetName);
}


bool SmSymDefineDialog::SelectSymbol(ComboBox &rComboBox,
        const XubString &rSymbolName, bool bDeleteText)
{
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(&rComboBox == &aOldSymbols  ||  &rComboBox == &aSymbols,
        "Sm : wrong ComboBox");
#endif

    // 'Normalisieren' des SymbolNamens (ohne Leerzeichen)
    XubString  aNormName(comphelper::string::remove(rSymbolName, ' '));
    // und evtl Abweichungen in der Eingabe beseitigen
    rComboBox.SetText(aNormName);

    bool   bRet = false;
    sal_uInt16 nPos = rComboBox.GetEntryPos(aNormName);

    bool  bIsOld = &rComboBox == &aOldSymbols;

    if (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        rComboBox.SetText(rComboBox.GetEntry(nPos));

        if (!bIsOld)
        {
            const SmSym *pSymbol = GetSymbol(aSymbols);
            if (pSymbol)
            {
                // Font und Style entsprechend waehlen
                const Font &rFont = pSymbol->GetFace();
                SelectFont(rFont.GetName(), false);
                SelectStyle(GetFontStyles().GetStyleName(rFont), false);

                // da das setzen des Fonts ueber den Style Namen des SymbolsFonts nicht
                // so gut klappt (er kann zB leer sein obwohl der Font selbst 'bold' und
                // 'italic' ist!). Setzen wir hier den Font wie er zum Symbol gehoert
                // zu Fuss.
                aCharsetDisplay.SetFont(rFont);
                aSymbolDisplay.SetFont(rFont);

                // das zugehoerige Zeichen auswaehlen
                SelectChar(pSymbol->GetCharacter());

                // since SelectChar will also set the unicode point as text in the
                // symbols box, we have to set the symbol name again to get that one displayed
                aSymbols.SetText( pSymbol->GetName() );
            }
        }

        bRet = true;
    }
    else if (bDeleteText)
        rComboBox.SetText(XubString());

    if (bIsOld)
    {
        // bei Wechsel des alten Symbols nur vorhandene anzeigen sonst keins
        const SmSym *pOldSymbol = NULL;
        XubString     aTmpOldSymbolSetName;
        if (nPos != COMBOBOX_ENTRY_NOTFOUND)
        {
            pOldSymbol        = aSymbolMgrCopy.GetSymbolByName(aNormName);
            aTmpOldSymbolSetName = aOldSymbolSets.GetText();
        }
        SetOrigSymbol(pOldSymbol, aTmpOldSymbolSetName);
    }
    else
        aSymbolName.SetText(rComboBox.GetText());

    UpdateButtons();

    return bRet;
}


void SmSymDefineDialog::SetFont(const XubString &rFontName, const XubString &rStyleName)
{
    // Font (FontInfo) passend zu Namen und Style holen
    FontInfo aFI;
    if (pFontList)
        aFI = pFontList->Get(rFontName, WEIGHT_NORMAL, ITALIC_NONE);
    SetFontStyle(rStyleName, aFI);

    aCharsetDisplay.SetFont(aFI);
    aSymbolDisplay.SetFont(aFI);

    // update subset listbox for new font's unicode subsets
    FontCharMap aFontCharMap;
    aCharsetDisplay.GetFontCharMap( aFontCharMap );
    if (pSubsetMap)
        delete pSubsetMap;
    pSubsetMap = new SubsetMap( &aFontCharMap );

    aFontsSubsetLB.Clear();
    bool bFirst = true;
    const Subset* pSubset;
    while( NULL != (pSubset = pSubsetMap->GetNextSubset( bFirst )) )
    {
        sal_uInt16 nPos = aFontsSubsetLB.InsertEntry( pSubset->GetName());
        aFontsSubsetLB.SetEntryData( nPos, (void *) pSubset );
        // subset must live at least as long as the selected font !!!
        if( bFirst )
            aFontsSubsetLB.SelectEntryPos( nPos );
        bFirst = false;
    }
    if( bFirst )
        aFontsSubsetLB.SetNoSelection();
    aFontsSubsetLB.Enable( !bFirst );
}


bool SmSymDefineDialog::SelectFont(const XubString &rFontName, bool bApplyFont)
{
    bool   bRet = false;
    sal_uInt16 nPos = aFonts.GetEntryPos(rFontName);

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        aFonts.SelectEntryPos(nPos);
        if (aStyles.GetEntryCount() > 0)
            SelectStyle(aStyles.GetEntry(0));
        if (bApplyFont)
        {
            SetFont(aFonts.GetSelectEntry(), aStyles.GetText());
        bRet = true;
            aSymbolDisplay.SetSymbol( aCharsetDisplay.GetSelectCharacter(), aCharsetDisplay.GetFont() );
        }
        bRet = sal_True;
    }
    else
        aFonts.SetNoSelection();
    FillStyles();

    UpdateButtons();

    return bRet;
}


bool SmSymDefineDialog::SelectStyle(const XubString &rStyleName, bool bApplyFont)
{
    bool   bRet = false;
    sal_uInt16 nPos = aStyles.GetEntryPos(rStyleName);

    // falls der Style nicht zur Auswahl steht nehmen wir den erst moeglichen
    // (sofern vorhanden)
    if (nPos == COMBOBOX_ENTRY_NOTFOUND  &&  aStyles.GetEntryCount() > 0)
        nPos = 0;

    if (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        aStyles.SetText(aStyles.GetEntry(nPos));
        if (bApplyFont)
        {
            SetFont(aFonts.GetSelectEntry(), aStyles.GetText());
        bRet = true;
            aSymbolDisplay.SetSymbol( aCharsetDisplay.GetSelectCharacter(), aCharsetDisplay.GetFont() );
        }
        bRet = sal_True;
    }
    else
        aStyles.SetText(XubString());

    UpdateButtons();

    return bRet;
}


void SmSymDefineDialog::SelectChar(xub_Unicode cChar)
{
    aCharsetDisplay.SelectCharacter( cChar );
    aSymbolDisplay.SetSymbol( cChar, aCharsetDisplay.GetFont() );

    UpdateButtons();
}


/**************************************************************************/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
