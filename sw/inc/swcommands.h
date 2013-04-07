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
#ifndef SW_SWCOMMANDS_HRC
#define SW_SWCOMMANDS_HRC

#define CMD_SID_CREATE_SW_DRAWVIEW                  ".uno:CreateSWDrawView"
#define CMD_FN_REDLINE_ACCEPT                       ".uno:AcceptTrackedChanges"
#define CMD_FN_REDLINE_ON                           ".uno:TrackChanges"
#define CMD_FN_ADD_UNKNOWN                          ".uno:AddAllUnknownWords"
#define CMD_FN_FRAME_ALIGN_VERT_BOTTOM              ".uno:AlignBottom"
#define CMD_FN_FRAME_ALIGN_VERT_CHAR_BOTTOM         ".uno:AlignCharBottom"
#define CMD_FN_FRAME_ALIGN_VERT_CHAR_TOP            ".uno:AlignCharTop"
#define CMD_FN_FRAME_ALIGN_HORZ_CENTER              ".uno:AlignHorizontalCenter"
#define CMD_FN_FRAME_ALIGN_HORZ_LEFT                ".uno:AlignLeft"
#define CMD_FN_FRAME_ALIGN_HORZ_RIGHT               ".uno:AlignRight"
#define CMD_FN_FRAME_ALIGN_VERT_ROW_BOTTOM          ".uno:AlignRowBottom"
#define CMD_FN_FRAME_ALIGN_VERT_ROW_TOP             ".uno:AlignRowTop"
#define CMD_FN_FRAME_ALIGN_VERT_TOP                 ".uno:AlignTop"
#define CMD_FN_FRAME_ALIGN_VERT_CENTER              ".uno:AlignVerticalCenter"
#define CMD_FN_FRAME_ALIGN_VERT_CHAR_CENTER         ".uno:AlignVerticalCharCenter"
#define CMD_FN_FRAME_ALIGN_VERT_ROW_CENTER          ".uno:AlignVerticalRowCenter"
#define CMD_FN_EDIT_AUTH_ENTRY_DLG                  ".uno:AuthoritiesEntryDialog"
#define CMD_FN_AUTOFORMAT_APPLY                     ".uno:AutoFormatApply"
#define CMD_FN_AUTOFORMAT_REDLINE_APPLY             ".uno:AutoFormatRedlineApply"
#define CMD_FN_TABLE_AUTOSUM                        ".uno:AutoSum"
#define CMD_SID_ATTR_CHAR_COLOR_BACKGROUND          ".uno:BackColor"
#define CMD_FN_FORMAT_BACKGROUND_DLG                ".uno:BackgroundDialog"
#define CMD_FN_BACKSPACE                            ".uno:SwBackspace"
#define CMD_FN_FORMAT_BORDER_DLG                    ".uno:BorderDialog"
#define CMD_FN_NUMBER_BULLETS                       ".uno:BulletsAndNumberingDialog"
#define CMD_FN_CALC_TABLE                           ".uno:Calc"
#define CMD_FN_CALCULATE                            ".uno:CalculateSel"
#define CMD_FN_TABLE_VERT_BOTTOM                    ".uno:CellVertBottom"
#define CMD_FN_TABLE_VERT_CENTER                    ".uno:CellVertCenter"
#define CMD_FN_TABLE_VERT_NONE                      ".uno:CellVertTop"
#define CMD_FN_FRAME_CHAIN                          ".uno:ChainFrames"
#define CMD_FN_CHANGE_DBFIELD                       ".uno:ChangeDatabaseField"
#define CMD_FN_NUMBERING_OUTLINE_DLG                ".uno:ChapterNumberingDialog"
#define CMD_SID_ATTR_CHAR_COLOR_BACKGROUND_EXT      ".uno:CharBackgroundExt"
#define CMD_SID_ATTR_CHAR_COLOR_EXT                 ".uno:CharColorExt"
#define CMD_FN_CHAR_LEFT_SEL                        ".uno:CharLeftSel"
#define CMD_FN_CHAR_RIGHT_SEL                       ".uno:CharRightSel"
#define CMD_FN_CLOSE_PAGEPREVIEW                    ".uno:ClosePreview"
#define CMD_FN_REDLINE_COMMENT                      ".uno:CommentChangeTracking"
#define CMD_FN_VIEW_META_CHARS                      ".uno:ControlCodes"
#define CMD_FN_CONVERT_TEXT_TABLE                   ".uno:ConvertTableText"
#define CMD_FN_ABSTRACT_NEWDOC                      ".uno:CreateAbstract"
#define CMD_FN_DEC_INDENT_OFFSET                    ".uno:DecrementIndentValue"
#define CMD_FN_NUM_BULLET_DOWN                      ".uno:DecrementLevel"
#define CMD_FN_NUM_BULLET_OUTLINE_DOWN              ".uno:DecrementSubLevels"
#define CMD_FN_TABLE_DELETE_COL                     ".uno:DeleteColumns"
#define CMD_FN_TABLE_DELETE_ROW                     ".uno:DeleteRows"
#define CMD_FN_DELETE_WHOLE_LINE                    ".uno:DelLine"
#define CMD_FN_DELETE_LINE                          ".uno:DelToEndOfLine"
#define CMD_FN_DELETE_PARA                          ".uno:DelToEndOfPara"
#define CMD_FN_DELETE_SENT                          ".uno:DelToEndOfSentence"
#define CMD_FN_DELETE_WORD                          ".uno:DelToEndOfWord"
#define CMD_FN_DELETE_BACK_LINE                     ".uno:DelToStartOfLine"
#define CMD_FN_DELETE_BACK_PARA                     ".uno:DelToStartOfPara"
#define CMD_FN_DELETE_BACK_SENT                     ".uno:DelToStartOfSentence"
#define CMD_FN_DELETE_BACK_WORD                     ".uno:DelToStartOfWord"
#define CMD_FN_TABLE_BALANCE_CELLS                  ".uno:DistributeColumns"
#define CMD_FN_TABLE_BALANCE_ROWS                   ".uno:DistributeRows"
#define CMD_FN_EDIT_CURRENT_TOX                     ".uno:EditCurIndex"
#define CMD_FN_GLOSSARY_DLG                         ".uno:EditGlossary"
#define CMD_FN_EDIT_HYPERLINK                       ".uno:EditHyperlink"
#define CMD_FN_EDIT_REGION                          ".uno:EditRegion"
#define CMD_FN_END_OF_DOCUMENT_SEL                  ".uno:EndOfDocumentSel"
#define CMD_FN_END_OF_LINE_SEL                      ".uno:EndOfLineSel"
#define CMD_FN_END_OF_PARA_SEL                      ".uno:EndOfParaSel"
#define CMD_FN_TABLE_SELECT_COL                     ".uno:EntireColumn"
#define CMD_FN_TABLE_SELECT_ROW                     ".uno:EntireRow"
#define CMD_FN_ESCAPE                               ".uno:Escape"
#define CMD_FN_EXECUTE_MACROFIELD                   ".uno:ExecuteMacroField"
#define CMD_FN_EXPAND_GLOSSARY                      ".uno:ExpandGlossary"
#define CMD_FN_EDIT_FIELD                           ".uno:FieldDialog"
#define CMD_FN_VIEW_FIELDNAME                       ".uno:Fieldnames"
#define CMD_FN_VIEW_FIELDS                          ".uno:Fields"
#define CMD_FN_FLIP_HORZ_GRAFIC                     ".uno:FlipHorizontal"
#define CMD_FN_FLIP_VERT_GRAFIC                     ".uno:FlipVertical"
#define CMD_SID_ATTR_CHAR_COLOR2                    ".uno:FontColor"
#define CMD_FN_FORMAT_FOOTNOTE_DLG                  ".uno:FootnoteDialog"
#define CMD_FN_EDIT_FOOTNOTE                        ".uno:EditFootnote"
#define CMD_FN_FORMAT_COLUMN                        ".uno:FormatColumns"
#define CMD_FN_FORMAT_DROPCAPS                      ".uno:FormatDropcap"
#define CMD_FN_FORMAT_FRAME_DLG                     ".uno:FrameDialog"
#define CMD_FN_LINE_DOWN                            ".uno:GoDown"
#define CMD_FN_CHAR_LEFT                            ".uno:GoLeft"
#define CMD_FN_CHAR_RIGHT                           ".uno:GoRight"
#define CMD_FN_FRAME_TO_ANCHOR                      ".uno:GoToAnchor"
#define CMD_FN_END_TABLE                            ".uno:GoToEnd"
#define CMD_FN_END_OF_COLUMN                        ".uno:GoToEndOfColumn"
#define CMD_FN_END_OF_DOCUMENT                      ".uno:GoToEndOfDoc"
#define CMD_FN_END_OF_LINE                          ".uno:GoToEndOfLine"
#define CMD_FN_END_OF_NEXT_COLUMN                   ".uno:GoToEndOfNextColumn"
#define CMD_FN_END_OF_NEXT_PAGE                     ".uno:GoToEndOfNextPage"
#define CMD_FN_END_OF_NEXT_PAGE_SEL                 ".uno:GoToEndOfNextPageSel"
#define CMD_FN_END_OF_PAGE                          ".uno:GoToEndOfPage"
#define CMD_FN_END_OF_PAGE_SEL                      ".uno:GoToEndOfPageSel"
#define CMD_FN_END_OF_PARA                          ".uno:GoToEndOfPara"
#define CMD_FN_END_OF_PREV_COLUMN                   ".uno:GoToEndOfPrevColumn"
#define CMD_FN_END_OF_PREV_PAGE                     ".uno:GoToEndOfPrevPage"
#define CMD_FN_END_OF_PREV_PAGE_SEL                 ".uno:GoToEndOfPrevPageSel"
#define CMD_FN_NEXT_TOXMARK                         ".uno:GotoNextIndexMark"
#define CMD_FN_GOTO_NEXT_INPUTFLD                   ".uno:GotoNextInputField"
#define CMD_FN_GOTO_NEXT_OBJ                        ".uno:GotoNextObject"
#define CMD_FN_NEXT_PARA                            ".uno:GoToNextPara"
#define CMD_FN_GOTO_NEXT_MARK                       ".uno:GotoNextPlacemarker"
#define CMD_FN_NEXT_SENT                            ".uno:GoToNextSentence"
#define CMD_FN_NEXT_SENT_SEL                        ".uno:GotoNextSentenceSel"
#define CMD_FN_NEXT_TBLFML                          ".uno:GotoNextTableFormula"
#define CMD_FN_NEXT_WORD                            ".uno:GoToNextWord"
#define CMD_FN_NEXT_TBLFML_ERR                      ".uno:GotoNextWrongTableFormula"
#define CMD_FN_NAVIGATION_PI_GOTO_PAGE              ".uno:GotoPage"
#define CMD_FN_PREV_TOXMARK                         ".uno:GotoPrevIndexMark"
#define CMD_FN_GOTO_PREV_INPUTFLD                   ".uno:GotoPrevInputField"
#define CMD_FN_GOTO_PREV_OBJ                        ".uno:GotoPrevObject"
#define CMD_FN_PREV_PARA                            ".uno:GoToPrevPara"
#define CMD_FN_GOTO_PREV_MARK                       ".uno:GotoPrevPlacemarker"
#define CMD_FN_PREV_SENT                            ".uno:GoToPrevSentence"
#define CMD_FN_PREV_SENT_SEL                        ".uno:GotoPrevSentenceSel"
#define CMD_FN_PREV_TBLFML                          ".uno:GotoPrevTableFormula"
#define CMD_FN_PREV_WORD                            ".uno:GoToPrevWord"
#define CMD_FN_PREV_TBLFML_ERR                      ".uno:GotoPrevWrongTableFormula"
#define CMD_FN_START_TABLE                          ".uno:GoToStartOfTable"
#define CMD_FN_START_OF_COLUMN                      ".uno:GoToStartOfColumn"
#define CMD_FN_START_OF_DOCUMENT                    ".uno:GoToStartOfDoc"
#define CMD_FN_START_OF_LINE                        ".uno:GoToStartOfLine"
#define CMD_FN_START_OF_NEXT_COLUMN                 ".uno:GoToStartOfNextColumn"
#define CMD_FN_START_OF_NEXT_PAGE                   ".uno:GoToStartOfNextPage"
#define CMD_FN_START_OF_NEXT_PAGE_SEL               ".uno:GoToStartOfNextPageSel"
#define CMD_FN_START_OF_PAGE                        ".uno:GoToStartOfPage"
#define CMD_FN_START_OF_PAGE_SEL                    ".uno:GoToStartOfPageSel"
#define CMD_FN_START_OF_PARA                        ".uno:GoToStartOfPara"
#define CMD_FN_START_OF_PREV_COLUMN                 ".uno:GoToStartOfPrevColumn"
#define CMD_FN_START_OF_PREV_PAGE                   ".uno:GoToStartOfPrevPage"
#define CMD_FN_START_OF_PREV_PAGE_SEL               ".uno:GoToStartOfPrevPageSel"
#define CMD_FN_LINE_UP                              ".uno:GoUp"
#define CMD_FN_VIEW_GRAPHIC                         ".uno:Graphic"
#define CMD_FN_FORMAT_GRAFIC_DLG                    ".uno:GraphicDialog"
#define CMD_FN_SAVE_GRAPHIC                         ".uno:SaveGraphic"
#define CMD_FN_EXTERNAL_EDIT                        ".uno:ExternalEdit"
#define CMD_FN_EXPORT_OLE_AS_PDF                    ".uno:ExportOleAsPDF"
#define CMD_FN_EXPORT_OLE_AS_GRAPHIC                ".uno:ExportOleAsGraphic"
#define CMD_FN_GROW_FONT_SIZE                       ".uno:Grow"
#define CMD_FN_HSCROLLBAR                           ".uno:HScroll"
#define CMD_SID_HTML_MODE                           ".uno:HtmlMode"
#define CMD_FN_HYPHENATE_OPT_DLG                    ".uno:Hyphenate"
#define CMD_FN_INC_INDENT_OFFSET                    ".uno:IncrementIndentValue"
#define CMD_FN_NUM_BULLET_UP                        ".uno:IncrementLevel"
#define CMD_FN_NUM_BULLET_OUTLINE_UP                ".uno:IncrementSubLevels"
#define CMD_FN_EDIT_IDX_ENTRY_DLG                   ".uno:IndexEntryDialog"
#define CMD_FN_IDX_MARK_TO_IDX                      ".uno:IndexMarkToIndex"
#define CMD_FN_POSTIT                               ".uno:InsertAnnotation"
#define CMD_FN_INSERT_FLD_AUTHOR                    ".uno:InsertAuthorField"
#define CMD_FN_INSERT_AUTH_ENTRY_DLG                ".uno:InsertAuthoritiesEntry"
#define CMD_FN_INSERT_BOOKMARK                      ".uno:InsertBookmark"
#define CMD_FN_INSERT_BREAK_DLG                     ".uno:InsertBreak"
#define CMD_FN_INSERT_CAPTION                       ".uno:InsertCaptionDialog"
#define CMD_FN_INSERT_COLUMN_BREAK                  ".uno:InsertColumnBreak"
#define CMD_FN_TABLE_INSERT_COL_DLG                 ".uno:InsertColumnDialog"
#define CMD_FN_TABLE_INSERT_COL                     ".uno:InsertColumns"
#define CMD_FN_INSERT_REGION                        ".uno:InsertSection"
#define CMD_FN_INSERT_CTRL                          ".uno:InsertCtrl"
#define CMD_FN_INSERT_FLD_DATE                      ".uno:InsertDateField"
#define CMD_FN_INSERT_ENDNOTE                       ".uno:InsertEndnote"
#define CMD_FN_ENVELOP                              ".uno:InsertEnvelope"
#define CMD_FN_INSERT_FIELD                         ".uno:InsertField"
#define CMD_FN_INSERT_FIELD_CTRL                    ".uno:InsertFieldCtrl"
#define CMD_FN_INSERT_FOOTNOTE_DLG                  ".uno:InsertFootnoteDialog"
#define CMD_FN_INSERT_FOOTNOTE                      ".uno:InsertFootnote"
#define CMD_FN_EDIT_FORMULA                         ".uno:InsertFormula"
#define CMD_FN_INSERT_FRAME                         ".uno:InsertFrame"
#define CMD_FN_INSERT_FRAME_INTERACT                ".uno:InsertFrameInteract"
#define CMD_FN_INSERT_FRAME_INTERACT_NOCOL          ".uno:InsertFrameInteractNoColumns"
#define CMD_FN_INSERT_HRULER                        ".uno:InsertGraphicRuler"
#define CMD_FN_INSERT_HYPERLINK                     ".uno:InsertHyperlinkDlg"
#define CMD_FN_INSERT_IDX_ENTRY_DLG                 ".uno:InsertIndexesEntry"
#define CMD_FN_INSERT_LINEBREAK                     ".uno:InsertLinebreak"
#define CMD_FN_INSERT_MULTI_TOX                     ".uno:InsertMultiIndex"
#define CMD_FN_NUM_BULLET_NONUM                     ".uno:InsertNeutralParagraph"
#define CMD_FN_INSERT_OBJ_CTRL                      ".uno:InsertObjCtrl"
#define CMD_FN_INSERT_OBJECT_DLG                    ".uno:InsertObjectDialog"
#define CMD_FN_INSERT_SMA                           ".uno:InsertObjectStarMath"
#define CMD_FN_INSERT_PAGEBREAK                     ".uno:InsertPagebreak"
#define CMD_FN_INSERT_FLD_PGCOUNT                   ".uno:InsertPageCountField"
#define CMD_FN_INSERT_PAGEFOOTER                    ".uno:InsertPageFooter"
#define CMD_FN_INSERT_PAGEHEADER                    ".uno:InsertPageHeader"
#define CMD_FN_INSERT_FLD_PGNUMBER                  ".uno:InsertPageNumberField"
#define CMD_FN_INSERT_BREAK                         ".uno:InsertPara"
#define CMD_FN_INSERT_REF_FIELD                     ".uno:InsertReferenceField"
#define CMD_FN_TABLE_INSERT_ROW_DLG                 ".uno:InsertRowDialog"
#define CMD_FN_TABLE_INSERT_ROW                     ".uno:InsertRows"
#define CMD_FN_JAVAEDIT                             ".uno:InsertScript"
#define CMD_FN_INSERT_TABLE                         ".uno:InsertTable"
#define CMD_FN_INSERT_FLD_TIME                      ".uno:InsertTimeField"
#define CMD_FN_INSERT_FLD_TITLE                     ".uno:InsertTitleField"
#define CMD_FN_INSERT_FLD_TOPIC                     ".uno:InsertTopicField"
#define CMD_FN_NUM_BULLET_NEXT                      ".uno:JumpDownThisLevel"
#define CMD_FN_END_DOC_DIRECT                       ".uno:JumpToEndOfDoc"
#define CMD_FN_TO_FOOTER                            ".uno:JumpToFooter"
#define CMD_FN_FOOTNOTE_TO_ANCHOR                   ".uno:JumpToFootnoteOrAnchor"
#define CMD_FN_TO_HEADER                            ".uno:JumpToHeader"
#define CMD_FN_NEXT_BOOKMARK                        ".uno:JumpToNextBookmark"
#define CMD_FN_NEXT_FOOTNOTE                        ".uno:JumpToNextFootnote"
#define CMD_FN_CNTNT_TO_NEXT_FRAME                  ".uno:JumpToNextFrame"
#define CMD_FN_GOTO_NEXT_REGION                     ".uno:JumpToNextRegion"
#define CMD_FN_NEXT_TABLE                           ".uno:JumpToNextTable"
#define CMD_FN_PREV_BOOKMARK                        ".uno:JumpToPrevBookmark"
#define CMD_FN_PREV_FOOTNOTE                        ".uno:JumpToPrevFootnote"
#define CMD_FN_GOTO_PREV_REGION                     ".uno:JumpToPrevRegion"
#define CMD_FN_PREV_TABLE                           ".uno:JumpToPrevTable"
#define CMD_FN_GOTO_REFERENCE                       ".uno:JumpToReference"
#define CMD_FN_START_DOC_DIRECT                     ".uno:JumpToStartOfDoc"
#define CMD_FN_NUM_BULLET_PREV                      ".uno:JumpUpThisLevel"
#define CMD_FN_LINE_DOWN_SEL                        ".uno:LineDownSel"
#define CMD_FN_LINE_NUMBERING_DLG                   ".uno:LineNumberingDialog"
#define CMD_FN_LINE_UP_SEL                          ".uno:LineUpSel"
#define CMD_FN_EDIT_LINK_DLG                        ".uno:LinkDialog"
#define CMD_SID_TEMPLATE_LOAD                       ".uno:LoadStyles"
#define CMD_FN_VIEW_MARKS                           ".uno:Marks"
#define CMD_FN_TABLE_MERGE_CELLS                    ".uno:MergeCells"
#define CMD_FN_QRY_MERGE                            ".uno:MergeDialog"
#define CMD_FN_MAILMERGE_WIZARD                     ".uno:MailMergeWizard"
#define CMD_FN_TABLE_MERGE_TABLE                    ".uno:MergeTable"
#define CMD_FN_FORMAT_APPLY_HEAD1                   ".uno:ApplyStyleHead1"
#define CMD_FN_FORMAT_APPLY_HEAD2                   ".uno:ApplyStyleHead2"
#define CMD_FN_FORMAT_APPLY_HEAD3                   ".uno:ApplyStyleHead3"
#define CMD_FN_FORMAT_APPLY_DEFAULT                 ".uno:ApplyStyleDefault"
#define CMD_FN_FORMAT_APPLY_TEXTBODY                ".uno:ApplyStyleTextbody"
#define CMD_FN_GRAPHIC_MIRROR_ON_EVEN_PAGES         ".uno:MirrorGraphicOnEvenPages"
#define CMD_FN_FRAME_MIRROR_ON_EVEN_PAGES           ".uno:MirrorOnEvenPages"
#define CMD_FN_NUM_BULLET_MOVEDOWN                  ".uno:MoveDown"
#define CMD_FN_NUM_BULLET_OUTLINE_MOVEDOWN          ".uno:MoveDownSubItems"
#define CMD_FN_NUM_BULLET_MOVEUP                    ".uno:MoveUp"
#define CMD_FN_NUM_BULLET_OUTLINE_MOVEUP            ".uno:MoveUpSubItems"
#define CMD_FN_TITLE_DESCRIPTION_SHAPE              ".uno:ObjectTitleDescription"
#define CMD_FN_NAME_SHAPE                           ".uno:NameGroup"
#define CMD_FN_NEW_GLOBAL_DOC                       ".uno:NewGlobalDoc"
#define CMD_FN_NEW_HTML_DOC                         ".uno:NewHtmlDoc"
#define CMD_FN_NUMBER_FORMAT                        ".uno:NumberFormat"
#define CMD_FN_NUMBER_CURRENCY                      ".uno:NumberFormatCurrency"
#define CMD_FN_NUMBER_DATE                          ".uno:NumberFormatDate"
#define CMD_FN_NUMBER_TWODEC                        ".uno:NumberFormatDecimal"
#define CMD_FN_NUMBER_PERCENT                       ".uno:NumberFormatPercent"
#define CMD_FN_NUMBER_SCIENTIFIC                    ".uno:NumberFormatScientific"
#define CMD_FN_NUMBER_STANDARD                      ".uno:NumberFormatStandard"
#define CMD_FN_NUMBER_TIME                          ".uno:NumberFormatTime"
#define CMD_FN_NUMBER_NEWSTART                      ".uno:NumberingStart"
#define CMD_FN_NUM_OR_NONUM                         ".uno:NumberOrNoNumber"
#define CMD_FN_FRAME_DOWN                           ".uno:ObjectBackOne"
#define CMD_FN_FRAME_UP                             ".uno:ObjectForwardOne"
#define CMD_FN_AUTOFORMAT_AUTO                      ".uno:OnlineAutoFormat"
#define CMD_FN_OPTIMIZE_TABLE                       ".uno:OptimizeTable"
#define CMD_FN_FORMAT_PAGE_COLUMN_DLG               ".uno:PageColumnDialog"
#define CMD_FN_FORMAT_PAGE_DLG                      ".uno:PageDialog"
#define CMD_FN_PAGEDOWN                             ".uno:PageDown"
#define CMD_FN_PAGEDOWN_SEL                         ".uno:PageDownSel"
#define CMD_FN_CHANGE_PAGENUM                       ".uno:PageOffset"
#define CMD_FN_SET_PAGE_STYLE                       ".uno:PageStyleApply"
#define CMD_FN_STAT_TEMPLATE                        ".uno:PageStyleName"
#define CMD_FN_PAGEUP                               ".uno:PageUp"
#define CMD_FN_PAGEUP_SEL                           ".uno:PageUpSel"
#define CMD_FN_PREVIEW_ZOOM                         ".uno:PreviewZoom"
#define CMD_FN_PRINT_LAYOUT                         ".uno:PrintLayout"
#define CMD_FN_PRINT_PAGEPREVIEW                    ".uno:PrintPagePreView"
#define CMD_FN_TABLE_SET_READ_ONLY_CELLS            ".uno:Protect"
#define CMD_FN_REDLINE_PROTECT                      ".uno:ProtectTraceChangeMode"
#define CMD_FN_REFRESH_VIEW                         ".uno:RefreshView"
#define CMD_FN_NUM_BULLET_OFF                       ".uno:RemoveBullets"
#define CMD_FN_REMOVE_CUR_TOX                       ".uno:RemoveTableOf"
#define CMD_FN_REPAGINATE                           ".uno:Repaginate"
#define CMD_FN_REPEAT_SEARCH                        ".uno:RepeatSearch"
#define CMD_FN_FORMAT_RESET                         ".uno:ResetAttributes"
#define CMD_FN_TABLE_UNSET_READ_ONLY                ".uno:ResetTableProtection"
#define CMD_FN_RULER                                ".uno:Ruler"
#define CMD_FN_QRY_INSERT                           ".uno:SbaInsert"
#define CMD_FN_QRY_INSERT_FIELD                     ".uno:SbaInsertField"
#define CMD_FN_QRY_MERGE_FIELD                      ".uno:SbaMerge"
#define CMD_FN_SCROLL_NAVIGATION                    ".uno:ScrollNavigation"
#define CMD_FN_SCROLL_NEXT_PREV                     ".uno:ScrollNextPrev"
#define CMD_FN_TABLE_SELECT_ALL                     ".uno:SelectTable"
#define CMD_FN_STAT_SELMODE                         ".uno:SelectionMode"
#define CMD_FN_SELECT_PARA                          ".uno:SelectText"
#define CMD_FN_SELECT_WORD                          ".uno:SelectWord"
#define CMD_FN_ABSTRACT_STARIMPRESS                 ".uno:SendAbstractToStarImpress"
#define CMD_FN_OUTLINE_TO_CLIPBOARD                 ".uno:SendOutlineToClipboard"
#define CMD_FN_OUTLINE_TO_IMPRESS                   ".uno:SendOutlineToStarImpress"
#define CMD_FN_TOOL_ANCHOR_AT_CHAR                  ".uno:SetAnchorAtChar"
#define CMD_FN_TOOL_ANCHOR_CHAR                     ".uno:SetAnchorToChar"
#define CMD_FN_TOOL_ANCHOR_FRAME                    ".uno:SetAnchorToFrame"
#define CMD_FN_TOOL_ANCHOR_PAGE                     ".uno:SetAnchorToPage"
#define CMD_FN_TOOL_ANCHOR_PARAGRAPH                ".uno:SetAnchorToPara"
#define CMD_FN_TABLE_SET_COL_WIDTH                  ".uno:SetColumnWidth"
#define CMD_FN_SET_EXT_MODE                         ".uno:SetExtSelection"
#define CMD_FN_SET_ADD_MODE                         ".uno:SetMultiSelection"
#define CMD_FN_TABLE_ADJUST_CELLS                   ".uno:SetOptimalColumnWidth"
#define CMD_FN_TABLE_OPTIMAL_HEIGHT                 ".uno:SetOptimalRowHeight"
#define CMD_FN_TABLE_SET_ROW_HEIGHT                 ".uno:SetRowHeight"
#define CMD_FN_SHADOWCURSOR                         ".uno:ShadowCursor"
#define CMD_FN_SHIFT_BACKSPACE                      ".uno:ShiftBackspace"
#define CMD_FN_SHOW_MULTIPLE_PAGES                  ".uno:ShowMultiplePages"
#define CMD_FN_VIEW_NOTES                           ".uno:ShowAnnotations"
#define CMD_FN_VIEW_HIDDEN_PARA                     ".uno:ShowHiddenParagraphs"
#define CMD_FN_REDLINE_SHOW                         ".uno:ShowTrackedChanges"
#define CMD_FN_SHOW_TWO_PAGES                       ".uno:ShowTwoPages"
#define CMD_FN_SHOW_BOOKVIEW                        ".uno:ShowBookview"
#define CMD_FN_SHRINK_FONT_SIZE                     ".uno:Shrink"
#define CMD_FN_SORTING_DLG                          ".uno:SortDialog"
#define CMD_FN_TABLE_SPLIT_CELLS                    ".uno:SplitCell"
#define CMD_FN_TABLE_SPLIT_TABLE                    ".uno:SplitTable"
#define CMD_FN_AUTO_CORRECT                         ".uno:StartAutoCorrect"
#define CMD_FN_START_OF_DOCUMENT_SEL                ".uno:StartOfDocumentSel"
#define CMD_FN_START_OF_LINE_SEL                    ".uno:StartOfLineSel"
#define CMD_FN_START_OF_PARA_SEL                    ".uno:StartOfParaSel"
#define CMD_FN_STAT_BOOKMARK                        ".uno:StateBookmark"
#define CMD_FN_STAT_PAGE                            ".uno:StatePageNumber"
#define CMD_FN_STAT_ZOOM                            ".uno:StateZoom"
#define CMD_FN_SET_SUB_SCRIPT                       ".uno:SubScript"
#define CMD_FN_SET_SUPER_SCRIPT                     ".uno:SuperScript"
#define CMD_FN_SBA_BRW_INSERT                       ".uno:SwBrwInsert"
#define CMD_FN_SBA_BRW_MERGE                        ".uno:SwMailMerge"
#define CMD_FN_SBA_BRW_UPDATE                       ".uno:SwUpdate"
#define CMD_FN_SYNC_LABELS                          ".uno:SynchronizeLabelsDlg"
#define CMD_FN_VIEW_TABLEGRID                       ".uno:TableBoundaries"
#define CMD_FN_FORMAT_TABLE_DLG                     ".uno:TableDialog"
#define CMD_FN_TABLE_MODE_FIX                       ".uno:TableModeFix"
#define CMD_FN_TABLE_MODE_FIX_PROP                  ".uno:TableModeFixProp"
#define CMD_FN_TABLE_MODE_VARIABLE                  ".uno:TableModeVariable"
#define CMD_FN_NUM_FORMAT_TABLE_DLG                 ".uno:TableNumberFormatDialog"
#define CMD_FN_SET_MODOPT_TBLNUMFMT                 ".uno:TableNumberRecognition"
#define CMD_FN_DRAWTEXT_ATTR_DLG                    ".uno:TextAttributes"
#define CMD_FN_DRAW_WRAP_DLG                        ".uno:TextWrap"
#define CMD_FN_THESAURUS_DLG                        ".uno:ThesaurusDialog"
#define CMD_FN_TOOL_ANCHOR                          ".uno:ToggleAnchorType"
#define CMD_FN_TOOL_HIERARCHIE                      ".uno:ToggleObjectLayer"
#define CMD_FN_UNDERLINE_DOUBLE                     ".uno:UnderlineDouble"
#define CMD_FN_FRAME_UNCHAIN                        ".uno:UnhainFrames"
#define CMD_FN_TABLE_UNSET_READ_ONLY_CELLS          ".uno:UnsetCellsReadOnly"
#define CMD_FN_UPDATE_ALL                           ".uno:UpdateAll"
#define CMD_FN_UPDATE_TOX                           ".uno:UpdateAllIndexes"
#define CMD_FN_UPDATE_ALL_LINKS                     ".uno:UpdateAllLinks"
#define CMD_FN_UPDATE_CHARTS                        ".uno:UpdateCharts"
#define CMD_FN_UPDATE_CUR_TOX                       ".uno:UpdateCurIndex"
#define CMD_FN_UPDATE_FIELDS                        ".uno:UpdateFields"
#define CMD_FN_UPDATE_INPUTFIELDS                   ".uno:UpdateInputFields"
#define CMD_FN_VIEW_BOUNDS                          ".uno:ViewBounds"
#define CMD_FN_VLINEAL                              ".uno:VRuler"
#define CMD_FN_VSCROLLBAR                           ".uno:VScroll"
#define CMD_FN_PREV_WORD_SEL                        ".uno:WordLeftSel"
#define CMD_FN_NEXT_WORD_SEL                        ".uno:WordRightSel"
#define CMD_FN_WRAP_ANCHOR_ONLY                     ".uno:WrapAnchorOnly"
#define CMD_FN_FRAME_WRAP_CONTOUR                   ".uno:WrapContour"
#define CMD_FN_FRAME_WRAP_IDEAL                     ".uno:WrapIdeal"
#define CMD_FN_FRAME_WRAP_LEFT                      ".uno:WrapLeft"
#define CMD_FN_FRAME_NOWRAP                         ".uno:WrapOff"
#define CMD_FN_FRAME_WRAP                           ".uno:WrapOn"
#define CMD_FN_FRAME_WRAP_RIGHT                     ".uno:WrapRight"
#define CMD_FN_FRAME_WRAPTHRU                       ".uno:WrapThrough"
#define CMD_FN_FRAME_WRAPTHRU_TRANSP                ".uno:WrapThroughTransparent"
#define CMD_FN_INSERT_STRING                        ".uno:InsertText"
#define CMD_SID_ATTR_PARA_NUMRULE                   ".uno:NumRule"
#define CMD_SID_ATTR_PARA_OUTLINE_LEVEL             ".uno:OutlineLevel"
#define CMD_FN_NUMBER_NEWSTART_AT                   ".uno:NumNewStartAt"
#define CMD_FN_FORMAT_LINENUMBER                    ".uno:LineNumber"
#define CMD_SID_ATTR_PARA_PAGENUM                   ".uno:PageNumber"
#define CMD_SID_ATTR_PARA_REGISTER                  ".uno:RegisterTrue"
#define CMD_FN_TO_FOOTNOTE_AREA                     ".uno:JumpToFootnoteArea"
#define CMD_FN_TXTATR_INET                          ".uno:HyperlinkAttributes"
#define CMD_FN_INSERT_GLOSSARY                      ".uno:InsertGlossary"
#define CMD_FN_NEW_GLOSSARY                         ".uno:NewGlossary"
#define CMD_FN_SET_ACT_GLOSSARY                     ".uno:SetActGlossaryGroup"
#define CMD_FN_DELETE_BOOKMARK                      ".uno:DeleteBookmark"
#define CMD_FN_DROP_TEXT                            ".uno:SetDropCapText"
#define CMD_FN_DROP_CHAR_STYLE_NAME                 ".uno:SetDropCapCharStyleName"
#define CMD_FN_READONLY_SELECTION_MODE              ".uno:SelectTextMode"
#define CMD_FN_INSERT_DBFIELD                       ".uno:InsertDBField"
#define CMD_FN_GOTO_NEXT_CELL                       ".uno:JumpToNextCell"
#define CMD_FN_GOTO_PREV_CELL                       ".uno:JumpToPrevCell"
#define CMD_FN_INSERT_FIELD_DATA_ONLY               ".uno:InsertFieldDataOnly"
#define CMD_SID_ATTR_BRUSH_ROW                      ".uno:TableRowBackground"
#define CMD_SID_ATTR_BRUSH_TABLE                    ".uno:TableBackground"
#define CMD_FN_TABLE_ROW_SPLIT                      ".uno:RowSplit"
#define CMD_FN_MAILMERGE_CHILDWINDOW                ".uno:MailmergeChildWindow"
#define CMD_FN_WORDCOUNT_DIALOG                     ".uno:WordCountDialog"
#define CMD_FN_MAILMERGE_SENDMAIL_CHILDWINDOW       ".uno:SendMailChildWindow"
#define CMD_FN_TABLE_DELETE_TABLE                   ".uno:DeleteTable"
#define CMD_FN_TABLE_SELECT_CELL                    ".uno:EntireCell"
#define CMD_FN_CONVERT_TABLE_TO_TEXT                ".uno:ConvertTableToText"
#define CMD_FN_CONVERT_TEXT_TO_TABLE                ".uno:ConvertTextToTable"
#define CMD_FN_TABLE_HEADLINE_REPEAT                ".uno:HeadingRowsRepeat"
#define CMD_FN_TABLE_SORT_DIALOG                    ".uno:TableSort"
#define CMD_FN_XFORMS_DESIGN_MODE                   ".uno:SwitchXFormsDesignMode"
#define CMD_FN_REMOVE_DIRECT_CHAR_FORMATS           ".uno:RemoveDirectCharFormats"
#define CMD_FN_NUM_CONTINUE                         ".uno:ContinueNumbering"
#define CMD_FN_SELECTION_MODE_DEFAULT               ".uno:SelectionModeDefault"
#define CMD_FN_DELETE_NOTE                          ".uno:DeleteNote"
#define CMD_FN_REPLY                                ".uno:ReplyNote"
#define CMD_FN_DELETE_COMMENT                       ".uno:DeleteComment"
#define CMD_FN_DELETE_NOTE_AUTHOR                   ".uno:DeleteAuthor"
#define CMD_FN_DELETE_ALL_NOTES                     ".uno:DeleteAllNotes"
#define CMD_FN_HIDE_NOTE                            ".uno:HideNote"
#define CMD_FN_HIDE_NOTE_AUTHOR                     ".uno:HideAuthor"
#define CMD_SID_ATTR_VIEWLAYOUT                     ".uno:ViewLayout"
#define CMD_FN_HIDE_ALL_NOTES                       ".uno:HideAllNotes"
#define CMD_FN_SELECTION_MODE_BLOCK                 ".uno:SelectionModeBlock"
#define CMD_FN_SPELL_GRAMMAR_DIALOG                 ".uno:SpellingAndGrammarDialog"
#define CMD_FN_REDLINE_ACCEPT_DIRECT                ".uno:AcceptTracedChange"
#define CMD_FN_REDLINE_REJECT_DIRECT                ".uno:RejectTracedChange"
#define CMD_FN_REMOVE_HYPERLINK                     ".uno:RemoveHyperlink"
#define CMD_FN_COPY_HYPERLINK_LOCATION              ".uno:CopyHyperlinkLocation"
#define CMD_FN_HEADERFOOTER_EDIT                    ".uno:HeaderFooterEdit"
#define CMD_FN_HEADERFOOTER_DELETE                  ".uno:HeaderFooterDelete"
#define CMD_FN_HEADERFOOTER_BORDERBACK              ".uno:HeaderFooterBorderBackground"
#define CMD_FN_PAGEBREAK_EDIT                       ".uno:PageBreakEdit"
#define CMD_FN_PAGEBREAK_DELETE                     ".uno:PageBreakDelete"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
