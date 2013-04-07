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

/******************************************************************************
Attention : "In this file C++ style // comments are not allowed as it will be
included in c-context files, so c++ style stuff will cause problems.
******************************************************************************/

#ifndef INCLUDED_SW_CMDID_H
#define INCLUDED_SW_CMDID_H

#include <sfx2/sfx.hrc>

#ifndef SW_UNOMID_H
#include <unomid.h>
#endif

/* Flags that are being used in the slot definitions for the disable-features */
#define SW_DISABLE_ON_PROTECTED_CURSOR  0x00000001
#define SW_DISABLE_ON_MAILBOX_EDITOR    0x00000002

#define FN_FILE                 SID_SW_START
#define FN_EDIT                 (SID_SW_START +  100)
#define FN_VIEW                 (SID_SW_START +  200)
#define FN_INSERT               (SID_SW_START +  300)
#define FN_FORMAT               (SID_SW_START +  400)
#define FN_EXTRA                (SID_SW_START +  600)
#define FN_WINDOW               (SID_SW_START +  700)
#define FN_HELP                 (SID_SW_START +  800)
#define FN_SELECTION            (SID_SW_START +  900)
#define FN_QUERY                (SID_SW_START + 1000)
#define FN_ENVELP               (SID_SW_START + 1050)
#define FN_PARAM                (SID_SW_START + 1100)
#define FN_STAT                 (SID_SW_START + 1180)
#define FN_PRNOPT               (SID_SW_START + 1200)
#define FN_PGPREVIEW            (SID_SW_START + 1250)
#define FN_FRAME                (SID_SW_START + 1300)
#define FN_INSERT2              (SID_SW_START + 1400)
#define FN_FORMAT2              (SID_SW_START + 1600)
#define FN_EDIT2                (SID_SW_START + 1800)
#define FN_QUERY2               (SID_SW_START + 2000)
#define FN_EXTRA2               (SID_SW_START + 2200)
#define FN_PARAM2               (SID_SW_START + 2400)
#define FN_NOTES                (SID_SW_START + 2500)
#define FN_HEADERFOOTER         (SID_SW_START + 2600)
#define FN_PAGEBREAK            (SID_SW_START + 2610)

 /* More accurately, this range should be from FN_EXTRA2 to FN_PARAM2-1, but
 * FN_NUMBER_NEWSTART comes from FN_FORMAT2, and FN_PARAM_LINK_DISPLAY_NAME
 * comes from FN_PARAM2 so we need to include FORMAT2,
 * EDIT2 and QUERY2 and PARAM2 in the range...hopefully this will be fixed
 * soon */

#define FN_UNO_RANGE_BEGIN      FN_FORMAT2
#define FN_UNO_RANGE_END        (FN_PARAM2 + 199)

#define HELP_OFFSET                 1100
#define CMD_STR_OFFSET              2200
#define CMD_STR_OFFSET_MULTILANG    3300
#define CMDID_END                   5500

/*--------------------------------------------------------------------
    Region: File
 --------------------------------------------------------------------*/
#define FN_CLOSE_FILE           (FN_FILE + 1 )    /* Close */
#define FN_LAUNCH_REGISTRY      (FN_FILE + 2 )    /* Registry */

#define FN_NEW_GLOBAL_DOC       (FN_FILE + 4 )    /* Create Global Document */
#define FN_NEW_FILE             (FN_FILE + 5 )    /* New */
#define FN_NEW_FILE_DLG         (FN_FILE + 6 )    /* New dialog */
#define FN_OPEN_FILE            (FN_FILE + 7 )    /* Open */
#define FN_EDIT_FILE            (FN_FILE + 8 )    /* Open or ToTop */

#define FN_INETFILE             (FN_FILE + 9 )    /* Internet File-Dialog */

#define FN_PRINT_FILE           (FN_FILE + 10)    /* Print */
#define FN_PRINT_FILE_OPTIONS   (FN_FILE + 11)    /* Print file options */
#define FN_SAVE_FILE_AS         (FN_FILE + 12)    /* Save as */
#define FN_SAVE_FILE            (FN_FILE + 13)    /* Save */
#define FN_SETUP_PRINTER        (FN_FILE + 14)    /* Printer setup */
#define FN_SETUP_PRINTER_DLG    (FN_FILE + 15)    /* Printer setup dialog */
#define FN_SHOW_PREVIEW         (FN_FILE + 16)    /* Print preview */
#define FN_EXIT                 (FN_FILE + 17)    /* Close Writer */
#define FN_PRINT_CFG_DLG        (FN_FILE + 18)    /* Print configuration dialog */

#define FN_SAVE_ALL             (FN_FILE + 23)    /* Save all files */
#define FN_NEW_FILE_DEFAULT     (FN_FILE + 24)    /* New file with default template */
#define FN_PRINT_FILE_DEFAULT   (FN_FILE + 25)    /* Print with default settings */
#define FN_LAUNCH_EQ_EDITOR     (FN_FILE + 26)    /* Equation editor */
#define FN_CHANGE_PRINTER       (FN_FILE + 27)    /* Select printer */
#define FN_FAX_END              (FN_FILE + 29)    /* Ready to fax, Id for PostMessage */

#define FN_SELECT_DATABASE      (FN_FILE + 30)    /* Database selection */
#define FN_DOC_INFO_DLG         (FN_FILE + 31)    /* Document information dialog */
#define FN_DOC_MGR_DLG          (FN_FILE + 32)    /* Document-Manager */

#define FN_GET_DOCSTAT          (FN_FILE + 33)    /* Get document statistics */

#define FN_SAVE_SELECTION       (FN_FILE + 35)    /* Save selection */

#define FN_OUTLINE_TO_IMPRESS   (FN_FILE + 36)  /* Send outline to impress */
#define FN_OUTLINE_TO_CLIPBOARD (FN_FILE + 37)  /* Send outline to clipboard */

#define FN_NEW_HTML_DOC         (FN_FILE + 40 ) /* Create HTML document */

#define FN_APP_START            (FN_FILE + 98)  /* Macro for App.Start */
#define FN_APP_END              (FN_FILE + 99)  /* Macro for App.End */

/*--------------------------------------------------------------------
    Region: Edit
 --------------------------------------------------------------------*/
#define FN_CLEAR                (FN_EDIT + 1 )    /* Clear */
#define FN_COPY                 (FN_EDIT + 2 )    /* Copy */
#define FN_CUT                  (FN_EDIT + 3 )    /* Cut */
#define FN_EDIT_FIELD           (FN_EDIT + 4 )    /* Edit field */
#define FN_EDIT_FIELD_DLG       (FN_EDIT + 5 )    /* Edit field dialog */
#define FN_EDIT_FILE_INFO       (FN_EDIT + 6 )    /* Document information */
#define FN_EDIT_FILE_INFO_DLG   (FN_EDIT + 7 )    /* Document information Dialog */
#define FN_EDIT_LINK            (FN_EDIT + 8 )    /* Edit link */
#define FN_EDIT_LINK_DLG        (FN_EDIT + 9 )    /* Edit link dialog */
#define FN_GOTO                 (FN_EDIT + 11)    /* Go to */

#define FN_PASTE                (FN_EDIT + 13)    /* Paste */
#define FN_PASTESPECIAL_DLG     (FN_EDIT + 15)    /* Paste special dialog */
#define FN_NUMBER_BULLETS       (FN_EDIT + 21)    /* Bullets */
#define FN_REPEAT               (FN_EDIT + 22)    /* Repeat last */
#define FN_EDIT_IDX_ENTRY_DLG   (FN_EDIT + 23)    /* Edit Index-Entry */
#define FN_UPDATE_FIELDS        (FN_EDIT + 26)    /* Update fields */
#define FN_EXECUTE_MACROFIELD   (FN_EDIT + 27)    /* Execute macrofield */
#define FN_EDIT_FORMULA         (FN_EDIT + 28)    /* Edit formula in RibbonBar */
#define FN_CALC_TABLE           (FN_EDIT + 29)

/*--------------------------------------------------------------------
    Region: Bullets
 --------------------------------------------------------------------*/
#define FN_NUM_BULLET_DOWN      (FN_EDIT + 30)  /* Bullet down */
#define FN_NUM_BULLET_UP        (FN_EDIT + 31)  /* Bullet up */
#define FN_NUM_BULLET_PREV      (FN_EDIT + 32)  /* to previous bullet */
#define FN_NUM_BULLET_NEXT      (FN_EDIT + 33)  /* to next bullet */
#define FN_NUM_BULLET_MOVEUP    (FN_EDIT + 34)  /* move bullet up */
#define FN_NUM_BULLET_MOVEDOWN  (FN_EDIT + 35)  /* move bullet down */
#define FN_NUM_BULLET_NONUM     (FN_EDIT + 36)  /* bullet without number */
#ifndef FN_NUM_BULLET_OFF //in SVX already
#define FN_NUM_BULLET_OFF       (FN_EDIT + 37)  /* turn off numbering */
#endif

#define FN_NUM_BULLET_OUTLINE_DOWN      (FN_EDIT + 39)  /* reduce in rank with sub points */
#define FN_NUM_BULLET_OUTLINE_UP        (FN_EDIT + 40)  /* increase in rank with sub-items */
#define FN_NUM_BULLET_OUTLINE_MOVEUP    (FN_EDIT + 41)  /* Push up with sub-items */
#define FN_NUM_BULLET_OUTLINE_MOVEDOWN  (FN_EDIT + 42)  /* Push down with sub-items */
#define FN_UPDATE_INPUTFIELDS           (FN_EDIT + 43)  /* Update input fields */

#define FN_NUM_OR_NONUM         (FN_EDIT + 46)  /* Number on/off */

#define FN_GOTO_NEXT_INPUTFLD   (FN_EDIT + 47)  /* go to next inputfield */
#define FN_GOTO_PREV_INPUTFLD   (FN_EDIT + 48)  /* go to previous inputfield    */

#define FN_REPEAT_SEARCH        (FN_EDIT + 50)  /* Search again */
#define FN_REPEAT_REPLACE       (FN_EDIT + 51)  /* Repeat replace */
#define FN_UPDATE_GRAFIC        (FN_EDIT + 53)  /* Change graphic */
#define FN_SETTAB_ATCURPOS      (FN_EDIT + 54)  /* Set Tab at the current position */
#define FN_SET_LR_IND_ATCURPOS  (FN_EDIT + 55)  /* EZE and LR set */
#define FN_FRMCNT_TO_BODY       (FN_EDIT + 56)  /* Frame content to text */
#define FN_UPDATE_TOXBASE       (FN_EDIT + 57)  /* Next TOC list refresh */

#define FN_REPAGINATE           (FN_EDIT + 61)  /* Force repagination */
#define FN_EDIT_FOOTNOTE        (FN_EDIT + 62)  /* Edit footnote */

#define FN_EDIT_REGION          (FN_EDIT + 65)  /* Edit region */
#define FN_GOTO_REFERENCE       (FN_EDIT + 66)  /* From Refmark to the reference */

// already defined in SVX
#define FN_NEXT_BOOKMARK        (FN_EDIT + 68)  /*  */
#define FN_PREV_BOOKMARK        (FN_EDIT + 69)  /*  */

/*Navigator ToolBoxes*/
#define FN_SELECT_BOOKMARK           (FN_EDIT + 70)  /**/
#define FN_ITEM_DOWN                 (FN_EDIT + 71)  /**/
#define FN_ITEM_LEFT                 (FN_EDIT + 72)  /**/
#define FN_ITEM_RIGHT                (FN_EDIT + 73)  /**/
#define FN_ITEM_UP                   (FN_EDIT + 74)  /**/
#define FN_DOWN                      (FN_EDIT + 75)  /**/

#define FN_SELECT_FOOTER             (FN_EDIT + 77)  /**/
#define FN_SELECT_FRAME              (FN_EDIT + 78)  /**/
#define FN_SELECT_HEADER             (FN_EDIT + 79)  /**/
#define FN_PAGENUMBER                (FN_EDIT + 80)  /**/
#define FN_SELECT_AUTO_BOOKMARK      (FN_EDIT + 81)  /**/
#define FN_SELECT_FOOTNOTE           (FN_EDIT + 82)  /**/
#define FN_SELECT_SET_AUTO_BOOKMARK  (FN_EDIT + 83)  /**/
#define FN_SELECT_TABLE              (FN_EDIT + 84)  /**/
#define FN_SELECT_INDEX              (FN_EDIT + 85)  /**/
#define FN_UP                        (FN_EDIT + 86)  /**/

#define FN_SELECT_PARA               (FN_EDIT + 97) /* select paragraph*/

#define FN_SELECT_CONTENT            (FN_EDIT + 99) /* Navigator - Content Type */

#define FN_UPDATE_ALL_LINKS          (FN_EDIT2 + 24) /* update all links */

#define FN_REDLINE_SHOW              (FN_EDIT2 + 26) /* show Redlining */
#define FN_REDLINE_COMMENT           (FN_EDIT2 + 27) /* comment Redlining */

#define FN_UPDATE_ALL                (FN_EDIT2 + 28) /* FN_UPDATE_ALL_LINKS,
                                                        FN_UPDATE_FIELDS,
                                                        FN_UPDATE_TOX,
                                                        CalcLayout */

#define FN_REDLINE_ACCEPT            (FN_EDIT2 + 29) /* Redlining Accept/Reject */
#define FN_ATTR_COLUMNS              (FN_EDIT2 + 31) /* SlotId for SwFmtCol */
#define FN_EDIT_CURRENT_TOX          (FN_EDIT2 + 32) /* edit current index */
#define FN_EDIT_AUTH_ENTRY_DLG       (FN_EDIT2 + 33) /* edit authorities entry*/
#define FN_UPDATE_CHARTS             (FN_EDIT2 + 34) /* update all charts */

#define FN_EDIT_HYPERLINK            (FN_EDIT2 + 35)    /* edit hyperlink */
#define FN_NUM_CONTINUE              (FN_EDIT2 + 36)    /* continue previous numbering */
#define FN_REDLINE_ACCEPT_DIRECT     (FN_EDIT2 + 37)    /* accept redline at current position*/
#define FN_REDLINE_REJECT_DIRECT     (FN_EDIT2 + 38)    /* reject redline at current position*/
#define FN_REMOVE_HYPERLINK          (FN_EDIT2 + 39)    /* remove hyperlink attribute */
#define FN_COPY_HYPERLINK_LOCATION   (FN_EDIT2 + 40)    /* copy hyperlink URL to clipboard */

/*--------------------------------------------------------------------
    Region: Edit
 --------------------------------------------------------------------*/

#define FN_REFRESH_VIEW         (FN_VIEW + 1)   /* Refresh/Redraw */
#define FN_SHOW_OUTLINE_VIEW    (FN_VIEW + 2)   /* Show outline */

#define FN_DRAW_WRAP_DLG        (FN_VIEW + 3)   /* Draw Umlauf-Dlg */

#define FN_RULER                (FN_VIEW + 11)  /* Horizontal ruler */

#define FN_VIEW_GRAPHIC         (FN_VIEW + 13)  /* View graphic */
#define FN_VIEW_BOUNDS          (FN_VIEW + 14)  /* View bounds */
#define FN_VIEW_FIELDS          (FN_VIEW + 15)  /* View fields */
#define FN_VLINEAL              (FN_VIEW + 16)  /* Vertical Liner */
#define FN_VSCROLLBAR           (FN_VIEW + 17)  /* Vertical Scrollbar */
#define FN_HSCROLLBAR           (FN_VIEW + 18)  /* Horizontal Scrollbar */
#define FN_TOOLBOX_TOGGLE       (FN_VIEW + 19)  /* Toggle toolbox */
#define FN_VIEWOPTIONS_ON       (FN_VIEW + 20)  /* Viewoptions group on */
#define FN_VIEWOPTIONS_OFF      (FN_VIEW + 21)  /* Viewoptions group off */
#define FN_CONTROLS_ON          (FN_VIEW + 22)  /* Controls group on */
#define FN_CONTROLS_OFF         (FN_VIEW + 23)  /* Controls group off */

#define FN_VIEW_META_CHARS      (FN_VIEW + 24)  /* View meta chars */
#define FN_VIEW_MARKS           (FN_VIEW + 25)  /* View marks */
#define FN_VIEW_FIELDNAME       (FN_VIEW + 26)  /* View field names */
#define FN_VIEW_TABLEGRID       (FN_VIEW + 27)  /* View tablegrid */

#define FN_SET_PAGE             (FN_VIEW + 29)  /* Set page template to paragraph */

#define FN_TOOLBOX_SWITCH       (FN_VIEW + 30)  /* Shell switch to text */

// more Navigator
#define FN_CONTENT_LB           (FN_VIEW + 32)  /**/
#define FN_SHOW_CONTENT_BOX     (FN_VIEW + 33)  /**/
#define FN_SHOW_ROOT            (FN_VIEW + 34)  /**/
#define FN_DROP_REGION          (FN_VIEW + 35)  /**/
#define FN_OUTLINE_LEVEL        (FN_VIEW + 36)  /**/

#define FN_PRINT_LAYOUT         (FN_VIEW + 37)  /* print layout */

#define FN_DROP_REGION_LINK     (FN_VIEW + 38)  /**/
#define FN_DROP_REGION_COPY     (FN_VIEW + 39)  /**/

#define FN_SCROLL_NAVIGATION    (FN_VIEW + 40)  /* Navigation Controller in the scrollbar */
#define FN_SCROLL_NEXT_PREV     (FN_VIEW + 41)  /* processes MoveType */

#define FN_VIEW_HIDDEN_PARA     (FN_VIEW + 42)  /* View hidden paragraphs */
#define FN_VIEW_SMOOTH_SCROLL   (FN_VIEW + 43)

#define FN_GLOBAL_SWITCH        (FN_VIEW + 44)  /* Toggle between Global and Content */
#define FN_GLOBAL_EDIT          (FN_VIEW + 45)  /* edit */
#define FN_GLOBAL_UPDATE        (FN_VIEW + 46)  /* update */
#define FN_GLOBAL_OPEN          (FN_VIEW + 47)  /* open */
#define FN_GLOBAL_SAVE_CONTENT  (FN_VIEW + 48)  /* save content of link */
#define FN_CREATE_NAVIGATION    (FN_VIEW + 49)  /* create Navigations-Controller */
#define FN_PREVIEW_ZOOM          (FN_VIEW + 51)  /* create table controller for zoom */

#define FN_SET_MODOPT_TBLNUMFMT  (FN_VIEW + 52) /* Number recognition in tables */
#define FN_HSCROLL_METRIC        (FN_VIEW + 53) /* Metric horizontal scrollbar */
#define FN_VSCROLL_METRIC        (FN_VIEW + 54) /* Metric vertical scrollbar */
#define FN_VIEW_NOTES            (FN_VIEW + 55)

/*--------------------------------------------------------------------
    Region: Insert
 --------------------------------------------------------------------*/

#define FN_INSERT_BOOKMARK      (FN_INSERT + 2 )  /* Bookmark */

#define FN_INSERT_BREAK_DLG     (FN_INSERT + 4 )  /* Break */
#define FN_INSERT_COLUMN_BREAK  (FN_INSERT + 5 )  /* Column break */
#define FN_INSERT_DATE_TIME     (FN_INSERT + 6 )  /* Date / Time */
#define FN_INSERT_FIELD         (FN_INSERT + 8 )  /* Textbefehl */
#define FN_CHANGE_DBFIELD       (FN_INSERT + 9 )  /* Change the database field */

#define FN_INSERT_CAPTION       (FN_INSERT + 10)  /* Caption */
#define FN_INSERT_DBFIELD       (FN_INSERT + 11)  /* Insert database field - for recording */
#define FN_INSERT_FOOTNOTE_DLG  (FN_INSERT + 12)  /* Footnote Dialog */

#define FN_INSERT_REF_FIELD     (FN_INSERT + 13)  /* Insert Reference Field */

#define FN_INSERT_HYPERLINK     (FN_INSERT + 14)  /* Character dialogue / hyperlink page */

#define FN_INSERT_INDEX         (FN_INSERT + 16)  /* Index entry */
#define FN_INSERT_INDEX_DLG     (FN_INSERT + 17)    /* Index entry */
#define FN_INSERT_LINEBREAK     (FN_INSERT + 18)    /* Newline */
#define FN_INSERT_FIELD_DATA_ONLY (FN_INSERT + 19)  /* Field dialog for mail merge*/
#define FN_INSERT_OBJECT        (FN_INSERT + 21)    /* Object */
#define FN_INSERT_OBJECT_DLG    (FN_INSERT + 22)    /* Object */
#define FN_INSERT_PAGEBREAK     (FN_INSERT + 23)    /* Page break*/
#define FN_INSERT_PAGENO        (FN_INSERT + 24)    /* Page Number */
#define FN_INSERT_PARAGRAPH     (FN_INSERT + 25)    /* Paragraph */
#define FN_INSERT_RECORD        (FN_INSERT + 26)    /* Insert record */
#define FN_MERGE_RECORD         (FN_INSERT + 27)    /* Merge record */
#define FN_INSERT_SYMBOL        (FN_INSERT + 28)    /* Special characters */
#define FN_POSTIT               (FN_INSERT + 29)    /* Insert/edit PostIt */
#define FN_INSERT_TABLE         (FN_INSERT + 30)    /* Insert Table */
#define FN_INSERT_STRING        (FN_INSERT+31)
#define FN_INSERT_COLS          (FN_INSERT + 32)    /* Insert Columns */
#define FN_INSERT_FRAME_INTERACT (FN_INSERT + 33)   /* Insert interactive frame */
#define FN_INSERT_FRAME         (FN_INSERT + 34)    /* Insert Frame */

#define FN_INSERT_IDX_ENTRY_DLG (FN_INSERT + 35)    /* insert index entry */
#define FN_INSERT_FRAME_INTERACT_NOCOL (FN_INSERT + 36) /*insert interactive non column frame*/

#define FN_TOOL_GROUP           (FN_INSERT + 47)    /* Group objects */
#define FN_TOOL_UNGROUP         (FN_INSERT + 48)    /* Remove Grouping */

#define FN_TOOL_ANCHOR          (FN_INSERT + 49)    /* Draw Anchor for object */
#define FN_TOOL_ANCHOR_PAGE     (FN_INSERT + 50)    /* anchor Draw object to page */
#define FN_TOOL_ANCHOR_PARAGRAPH (FN_INSERT + 51)   /* anchor Draw object to paragraph */
#define FN_TOOL_HIERARCHIE      (FN_INSERT + 52)    /* change hierarchy */

#define FN_MAILMERGE_WIZARD     (FN_INSERT + 64)    /* mail merge wizard */
#define FN_QRY_OPEN_TABLE       (FN_INSERT + 65)    /* open database (Basic) */
#define FN_TOOL_ANCHOR_FRAME    (FN_INSERT + 66)    /* anchor Draw-Object to frame*/
#define FN_QRY_MERGE            (FN_INSERT + 67)    /* insert record (serial letter) */
#define FN_MAILMERGE_CHILDWINDOW (FN_INSERT + 68)   /* back-to-mail-merge-wizard child window*/
#define FN_INSERT_SMA           (FN_INSERT + 69)    /* insert StarMath  */

#define FN_QRY_GET_COLUMN_NAME  (FN_INSERT + 70)    /* number of columns (Basic) */
#define FN_QRY_GET_COLUMN_TYPE  (FN_INSERT + 71)    /* retrieve type of database field (Basic) */
#define FN_QRY_GET_COLUMN_TEXT  (FN_INSERT + 72)    /* content of current record (Basic) */
#define FN_QRY_DEL_DEFINED_DB   (FN_INSERT + 75)    /* delete database definition */

#define FN_DRAWTEXT_ATTR_DLG    (FN_INSERT + 76)    /* position DrawText */

#define FN_QRY_APPEND_RECORD    (FN_INSERT + 77)    /* append new record */
#define FN_QRY_UPDATE_RECORD    (FN_INSERT + 78)    /* change record row */
#define FN_QRY_DELETE_RECORD    (FN_INSERT + 79)    /* delete record */
#define FN_QRY_GET_PHYS_RECORD_ID (FN_INSERT + 80)  /* get physical (absolute) record number (Basic) */

#define FN_TOOL_ANCHOR_CHAR     (FN_INSERT + 84)    /* anchor Draw object to charater */

#define FN_QRY_INSERT           (FN_INSERT + 86)    /* insert record selection in to text */
#define FN_QRY_MERGE_FIELD      (FN_INSERT + 87)    /* insert record selection into fields */
#define FN_QRY_INSERT_FIELD     (FN_INSERT + 88)    /* insert database field */

#define FN_INSERT_CTRL          (FN_INSERT + 89)    /* toolbar controller insert*/
#define FN_INSERT_OBJ_CTRL      (FN_INSERT + 90)    /* toolbar controller (insert/object) */
#define FN_INSERT_FIELD_CTRL    (FN_INSERT + 91)    /* toolbar controller insert/field commands */

#define FN_INSERT_FLD_DATE      (FN_INSERT + 92)
#define FN_INSERT_FLD_TIME      (FN_INSERT + 93)
#define FN_INSERT_FLD_PGNUMBER  (FN_INSERT + 94)
#define FN_INSERT_FLD_PGCOUNT   (FN_INSERT + 95)
#define FN_INSERT_FLD_TOPIC     (FN_INSERT + 96)
#define FN_INSERT_FLD_TITLE     (FN_INSERT + 97)
#define FN_INSERT_FLD_AUTHOR    (FN_INSERT + 98)
#define FN_INSERT_FOOTNOTE      (FN_INSERT + 99)

/*--------------------------------------------------------------------
    Region: Paste (Part 2)
 --------------------------------------------------------------------*/

#define FN_QRY_GET_ALL_DBNAME   (FN_INSERT2 + 1)    /* retrieve all configured database names */
#define FN_QRY_GET_DBNAME_COUNT (FN_INSERT2 + 2)    /* retrieve number of all configured database names */
#define FN_QRY_GET_TABLE_NAME   (FN_INSERT2 + 3)    /* retrieve table name of a database */
#define FN_QRY_GET_TABLE_COUNT  (FN_INSERT2 + 4)    /* retrieve number of tables of a database */
#define FN_MAILMERGE_SENDMAIL_CHILDWINDOW (FN_INSERT2 + 5)  /* child window provided by mailmerge */

#define FN_SBA_BRW_UPDATE       (FN_INSERT2 + 7)    /* insert records into fields */
#define FN_SBA_BRW_INSERT       (FN_INSERT2 + 8)    /* insert records into text */
#define FN_SBA_BRW_MERGE        (FN_INSERT2 + 9)    /* serial letter print */
#define FN_JAVAEDIT             (FN_INSERT2 + 10)   /* edit script field */
#define FN_INSERT_HRULER        (FN_INSERT2 + 11)   /* insert horizontal ruler */
#define FN_TOOL_ANCHOR_AT_CHAR  (FN_INSERT2 + 12)   /* anchor object to character*/

#define FN_INSERT_PAGEHEADER    (FN_INSERT2 + 13)   /* insert default header */
#define FN_INSERT_PAGEFOOTER    (FN_INSERT2 + 14)   /* insert default footer */

#define FN_INSERT_ENDNOTE           (FN_INSERT2 + 18)   /* insert endnote*/
#define FN_INSERT_REGION            (FN_INSERT2 + 19)   /* Insert section */

#define FN_INSERT_MULTI_TOX         (FN_INSERT2 + 20)   /* insert any TOX */
#define FN_INSERT_AUTH_ENTRY_DLG    (FN_INSERT2 + 21)   /* insert entry for table of authorities*/

/*--------------------------------------------------------------------
    Region: Format
 --------------------------------------------------------------------*/

#define FN_AUTOFORMAT_APPLY     (FN_FORMAT + 1 ) /* apply autoformat options */
#define FN_AUTOFORMAT_AUTO      (FN_FORMAT + 2 ) /* apply autoformat during user input */
#define FN_GROW_FONT_SIZE       (FN_FORMAT + 3 ) /* size */
#define FN_SHRINK_FONT_SIZE     (FN_FORMAT + 4 ) /* size */
#define FN_UNDERLINE_DOUBLE     (FN_FORMAT + 5 ) /* double underline */
#define FN_AUTOFORMAT_REDLINE_APPLY (FN_FORMAT + 6 ) /* apply autoformat with Redlining */
#define FN_SET_SUPER_SCRIPT     (FN_FORMAT + 11) /* superscript */
#define FN_SET_SUB_SCRIPT       (FN_FORMAT + 12) /* subscript */

#define FN_SET_CASEMAP          (FN_FORMAT + 14) /* CaseMap */
#define FN_SET_LANGUAGE         (FN_FORMAT + 15) /* language */
#define FN_SET_KERNING          (FN_FORMAT + 16) /* Kerning */

#define FN_INDENT_TO_TABSTOP    (FN_FORMAT + 17) /* indent to tabstop */
#define FN_FLIP_HORZ_GRAFIC     (FN_FORMAT + 25) /* mirror horizontally */
#define FN_FLIP_VERT_GRAFIC     (FN_FORMAT + 26) /* mirror vertically */
#define FN_SET_LINE_SPACE       (FN_FORMAT + 27) /* parametrized line distance */
#define FN_SET_ADJUST           (FN_FORMAT + 28) /* parametrized alignemnt */
#define FN_SET_LRMARGIN         (FN_FORMAT + 29) /* left/right margin */
#define FN_SET_ULMARGIN         (FN_FORMAT + 30) /* upper/lower margin */
#define FN_UNINDENT_TO_TABSTOP  (FN_FORMAT + 31) /* indent to previous tabstop */

#define FN_SET_HYPHEN_ZONE      (FN_FORMAT + 32) /* hyphenation */
#define FN_SET_KEEP_TOGETHER    (FN_FORMAT + 34) /* no hyphenation */
#define FN_SET_KEEP_WITH_NEXT   (FN_FORMAT + 35) /* keep together with following */
#define FN_SET_WIDOW            (FN_FORMAT + 36) /* Widows */
#define FN_SET_ORPHAN           (FN_FORMAT + 37) /* Orphans */

#define FN_REGISTER_COLLECTION  (FN_FORMAT + 43)  /* Referenzvorlage an der Seite */
#define FN_REGISTER_MODE        (FN_FORMAT + 44)  /* Register mode on/off */
#define FN_NUM_FORMAT_TABLE_DLG     (FN_FORMAT + 45)  /* number format in table */
#define FN_FORMAT_BORDER_DLG        (FN_FORMAT + 48)  /* */
#define FN_FORMAT_PAGE_COLUMN_DLG   (FN_FORMAT + 49)  /* columns per page */
#define FN_FORMAT_BACKGROUND_DLG    (FN_FORMAT + 50)  /* background */
#define FN_FORMAT_PAGE              (FN_FORMAT + 51)  /* page */
#define FN_FORMAT_PAGE_DLG          (FN_FORMAT + 52)  /* page */
#define FN_FORMAT_COLUMN            (FN_FORMAT + 53)  /* columns */
#define FN_FORMAT_DROPCAPS          (FN_FORMAT + 54)  /* initials */
#define FN_FORMAT_FRAME             (FN_FORMAT + 55)  /* frame */
#define FN_FORMAT_FRAME_DLG         (FN_FORMAT + 56)  /* frame */
#define FN_FORMAT_GRAFIC            (FN_FORMAT + 57)  /* graphic */
#define FN_FORMAT_GRAFIC_DLG        (FN_FORMAT + 58)  /* graphic */
#define FN_FORMAT_TABLE             (FN_FORMAT + 59)  /* table */
#define FN_FORMAT_TABLE_DLG         (FN_FORMAT + 60)  /* table */
#define FN_FORMAT_OLE_DLG           (FN_FORMAT + 61)  /* edit OLE */

#define FN_NEW_STYLE_BY_EXAMPLE     (FN_FORMAT + 62)  /* create style by example */
#define FN_UPDATE_STYLE_BY_EXAMPLE  (FN_FORMAT + 63)  /* update style */
#define FN_STYLE_SHEET_FRAME_DLG    (FN_FORMAT + 64)  /* frame style */
#define FN_STYLE_SHEET_DOC_DLG      (FN_FORMAT + 65)  /* document style */
#define FN_STYLE_SHEET_GRAFIK_DLG   (FN_FORMAT + 66)  /* document style */
#define FN_FORMAT_FOOTNOTE          (FN_FORMAT + 67)  /* footnotes */
#define FN_FORMAT_FOOTNOTE_DLG      (FN_FORMAT + 68)  /* footnote dialog */

// already defined in Svx
//#define FN_FORMAT_RESET           (FN_FORMAT + 69)

#define FN_FRAME_TO_TOP             (FN_FORMAT + 70)
#define FN_FRAME_TO_BOTTOM          (FN_FORMAT + 71)

#define FN_FRAME_NOWRAP             (FN_FORMAT + 72)
#define FN_FRAME_WRAP               (FN_FORMAT + 73)
#define FN_FRAME_WRAPTHRU           (FN_FORMAT + 74)

#define FN_FRAME_ALIGN_HORZ_LEFT    (FN_FORMAT + 75)
#define FN_FRAME_ALIGN_HORZ_RIGHT   (FN_FORMAT + 76)
#define FN_FRAME_ALIGN_HORZ_CENTER  (FN_FORMAT + 77)
#define FN_FRAME_ALIGN_VERT_TOP     (FN_FORMAT + 78)
#define FN_FRAME_ALIGN_VERT_BOTTOM  (FN_FORMAT + 79)
#define FN_FRAME_ALIGN_VERT_CENTER  (FN_FORMAT + 80)

#define FN_SET_FRM_POSITION         (FN_FORMAT + 82)/* frame position -- " -- */
#define FN_SET_FRM_OPTIONS          (FN_FORMAT + 86)/* frame position -- " -- */

#define FN_SET_PAGE_STYLE           (FN_FORMAT + 93)  /* apply page style */

#define FN_FORMAT_TITLEPAGE_DLG     (FN_FORMAT + 98)  /* Title Page */

#define FN_TABLE_REP                (FN_FORMAT + 99)  /* TableRepresentation */
#define FN_CONVERT_TEXT_TABLE       (FN_FORMAT + 100)  /* convert text <-> table */
#define FN_TABLE_INSERT_ROW         (FN_FORMAT + 101)  /* */
#define FN_TABLE_INSERT_COL         (FN_FORMAT + 102)  /* */
#define FN_TABLE_DELETE_ROW         (FN_FORMAT + 103)  /* */
#define FN_TABLE_DELETE_COL         (FN_FORMAT + 104)  /* */
#define FN_TABLE_SPLIT_CELLS        (FN_FORMAT + 105)  /* */
#define FN_TABLE_MERGE_CELLS        (FN_FORMAT + 106)  /* */
#define FN_TABLE_SET_ROW_HEIGHT     (FN_FORMAT + 107)  /* */
#define FN_TABLE_SET_COL_WIDTH      (FN_FORMAT + 108)  /* */
#define FN_TABLE_SET_ULSPACE        (FN_FORMAT + 109)  /* table: set upper / lower offset */
#define FN_OPTIMIZE_TABLE           (FN_FORMAT + 110)  /* ToolBoxItem for optimizing tables */
#define FN_TABLE_SET_SHADOW         (FN_FORMAT + 111)  /* */
#define FN_TABLE_GOTO_CELL          (FN_FORMAT + 112)  /* */
#define FN_TABLE_SELECT_ROW         (FN_FORMAT + 113)  /* */
#define FN_TABLE_SELECT_COL         (FN_FORMAT + 114)  /* */
#define FN_TABLE_SELECT_ALL         (FN_FORMAT + 115)  /* */
#define FN_TABLE_SET_READ_ONLY      (FN_FORMAT + 116)  /* protect table */
#define FN_TABLE_SET_READ_ONLY_CELLS (FN_FORMAT + 117)  /* protect table cells */
#define FN_TABLE_UNSET_READ_ONLY_CELLS  (FN_FORMAT + 119)  /* undo table cell protection */
#define FN_TABLE_HEADLINE_REPEAT    (FN_FORMAT + 120)   /* also used in SwXTextTable*/
#define FN_TABLE_ADJUST_CELLS       (FN_FORMAT + 121)  /* */

#define FN_FRAME_UP                 (FN_FORMAT + 122)  /* frame by one level up */
#define FN_FRAME_DOWN               (FN_FORMAT + 123)  /* frame by one level down */

#define FN_TABLE_INSERT_CHART       (FN_FORMAT + 124)

#define FN_SET_FRM_LRSPACE          (FN_FORMAT + 125)  /* frame page offset */
#define FN_SET_FRM_ULSPACE          (FN_FORMAT + 126)  /* frame head-/foot offset */
#define FN_TABLE_SET_LRSPACE        (FN_FORMAT + 127)  /* Tabelle: linker / rechter Abstand einstellen */
#define FN_TABLE_OPTIMAL_HEIGHT     (FN_FORMAT + 128)  /* optimal cell height */
#define FN_TABLE_DELETE_TABLE       (FN_FORMAT + 129)  /* remove current table*/
#define FN_TABLE_SELECT_CELL        (FN_FORMAT + 130)  /* selects the current table cell */
#define FN_CONVERT_TEXT_TO_TABLE    (FN_FORMAT + 131)  /* convert selected text to table */
#define FN_CONVERT_TABLE_TO_TEXT    (FN_FORMAT + 132)  /* convert a table to text */
#define FN_TABLE_SORT_DIALOG        (FN_FORMAT + 133)  /* sorting in tables*/

/*--------------------------------------------------------------------
    Region: Page Template
 --------------------------------------------------------------------*/
#define FN_NEW_PAGE_STYLE           (FN_FORMAT + 129) /* create page style */
#define FN_PAGE_STYLE_SET_LR_MARGIN (FN_FORMAT + 130) /* left / right margin */
#define FN_PAGE_STYLE_SET_UL_MARGIN (FN_FORMAT + 131) /* upper / lower margin */
#define FN_PAGE_STYLE_SET_PAGE      (FN_FORMAT + 134) /* */
#define FN_PAGE_STYLE_SET_PAPER_SIZE (FN_FORMAT + 135) /* */
#define FN_PAGE_STYLE_SET_PAPER_BIN (FN_FORMAT + 136) /* paper tray */
#define FN_PAGE_STYLE_SET_NUMBER_FORMAT (FN_FORMAT + 137) /* */
#define FN_PAGE_STYLE_SET_COLS      (FN_FORMAT + 138) /* number of columns */

/* these Ids are only required by Help and are replaced by the Ids without _DLG
 * for the dialog */
#define FN_TABLE_INSERT_COL_DLG     (FN_FORMAT + 142)  /* */
#define FN_TABLE_INSERT_ROW_DLG     (FN_FORMAT + 143)  /* */

#define FN_GOTO_NEXT_CELL           (FN_FORMAT + 145)  /* Table: go to next cell */
#define FN_GOTO_PREV_CELL           (FN_FORMAT + 146)  /* Table: go to previous cell */

#define FN_TABLE_HEADLINE_COUNT     (FN_FORMAT + 147) /* number of repeated headlines of tables */

/*--------------------------------------------------------------------
    Region: Ribbon
 --------------------------------------------------------------------*/

#define FN_SET_BACKGROUND_COLOR     (FN_FORMAT + 150)  /* */

#define FN_SET_BORDER_POS           (FN_FORMAT + 152)  /* */
#define FN_SET_BORDER_LINES         (FN_FORMAT + 153)  /* */
#define FN_SET_BORDER_COLOR         (FN_FORMAT + 154)  /* */

#define FN_FORMULA_CALC             (FN_FORMAT + 156)  /* select formula */
#define FN_FORMULA_CANCEL           (FN_FORMAT + 157)  /* don't apply formula */
#define FN_FORMULA_APPLY            (FN_FORMAT + 158)  /* apply formula */

#define FN_TABLE_UNSET_READ_ONLY    (FN_FORMAT + 159)  /* undo cell protection for table */

#define FN_FORMULA_POSITION         (FN_FORMAT + 160)  /* position display*/
#define FN_FORMULA_EDIT             (FN_FORMAT + 161)  /* edit-field */

#define FN_FRAME_WRAP_IDEAL         (FN_FORMAT + 163)  /* frame ideal wrapping */
#define FN_FRAME_WRAPTHRU_TRANSP    (FN_FORMAT + 164)  /* frame transparent wrapping */

#define FN_FRAME_ALIGN_VERT_ROW_TOP     (FN_FORMAT + 165)  /* */
#define FN_FRAME_ALIGN_VERT_ROW_BOTTOM  (FN_FORMAT + 166)  /* */
#define FN_FRAME_ALIGN_VERT_ROW_CENTER  (FN_FORMAT + 167)  /* */

#define FN_FRAME_ALIGN_VERT_CHAR_TOP    (FN_FORMAT + 168)  /* */
#define FN_FRAME_ALIGN_VERT_CHAR_BOTTOM (FN_FORMAT + 169)  /* */
#define FN_FRAME_ALIGN_VERT_CHAR_CENTER (FN_FORMAT + 170)  /* */

#define FN_TABLE_SET_DEF_BORDER         (FN_FORMAT + 171)  /* set predefined border */

#define FN_FRAME_WRAP_LEFT              (FN_FORMAT + 172)  /* */
#define FN_FRAME_WRAP_RIGHT             (FN_FORMAT + 173)  /* */

#define FN_TABLE_SET_ROW_AUTOHEIGHT (FN_FORMAT + 174)  /* table: set minimal automatic row height */

#define FN_WRAP_ANCHOR_ONLY         (FN_FORMAT + 181)  /* wrap only for first paragraph */

#define FN_TABLE_BALANCE_CELLS      (FN_FORMAT + 182)  /* evenly distribute columns */
#define FN_TABLE_BALANCE_ROWS       (FN_FORMAT + 183)  /* evenly distribute rows */

#define FN_FRAME_WRAP_CONTOUR       (FN_FORMAT + 184)  /* */

#define FN_TABLE_VERT_NONE          (FN_FORMAT + 185)  /* vertical alignment in table cells */
#define FN_TABLE_VERT_CENTER        (FN_FORMAT + 186)  /*  -"-  */
#define FN_TABLE_VERT_BOTTOM        (FN_FORMAT + 187)  /*  -"-  */
#define FN_TABLE_SET_VERT_ALIGN     (FN_FORMAT + 188)  /*  -"-  */
#define FN_TABLE_MODE_FIX           (FN_FORMAT + 189)  /* table mode */
#define FN_TABLE_MODE_FIX_PROP      (FN_FORMAT + 190)  /*  -"-  */
#define FN_TABLE_MODE_VARIABLE      (FN_FORMAT + 191)  /*  -"-  */
#define FN_TABLE_BOX_TEXTORIENTATION (FN_FORMAT + 192)  /* text orientation of table cells */

#define FN_TABLE_AUTOSUM            (FN_FORMAT + 195)  /* */
#define FN_SET_CONTROL_HANDLER      (FN_FORMAT + 199)  /* set macro */

#define FN_IS_IMAGE                  (FN_FORMAT2 + 6)   /* numbering: with graphic? */

#define FN_GOTO_NEXT_REGION          (FN_FORMAT2 + 9)   /* */
#define FN_GOTO_PREV_REGION          (FN_FORMAT2 + 10)  /* */

#define FN_GET_SBXCONTROL            (FN_FORMAT2 + 11)  /* */
#define FN_ABSTRACT_NEWDOC           (FN_FORMAT2 + 12)  /* abstract in new document */
#define FN_ABSTRACT_STARIMPRESS      (FN_FORMAT2 + 13)  /* abstract to StarImpress */

#define FN_NAME_SHAPE                (FN_FORMAT2 + 14)  /* Name shapes */

#define FN_TITLE_DESCRIPTION_SHAPE   (FN_FORMAT2 + 15)  /* shape title and description */

#define FN_NUMBER_FORMAT                    (FN_FORMAT2 + 120) /* set Boxen/NumberFormatter */
#define FN_NUMBER_STANDARD                  (FN_FORMAT2 + 121)
#define FN_NUMBER_TWODEC                    (FN_FORMAT2 + 123)
#define FN_NUMBER_SCIENTIFIC                (FN_FORMAT2 + 124)
#define FN_NUMBER_DATE                      (FN_FORMAT2 + 125)
#define FN_NUMBER_TIME                      (FN_FORMAT2 + 126)
#define FN_NUMBER_CURRENCY                  (FN_FORMAT2 + 127)
#define FN_NUMBER_PERCENT                   (FN_FORMAT2 + 128)

#define FN_EXPORT_OLE                       (FN_FORMAT2 + 129)    /* Export menu item */
#define FN_EXPORT_OLE_AS_PDF                (FN_FORMAT2 + 130)    /* Export as PDF */
#define FN_EXPORT_OLE_AS_GRAPHIC            (FN_FORMAT2 + 131)    /* Export as JPG */

#define FN_FRAME_CHAIN                      (FN_FORMAT2 + 136)
#define FN_FRAME_UNCHAIN                    (FN_FORMAT2 + 137)

#define FN_NUMBER_NEWSTART                  (FN_FORMAT2 + 138)
#define FN_NUMBER_NEWSTART_AT               (FN_FORMAT2 + 139)

#define FN_FRAME_MIRROR_ON_EVEN_PAGES       (FN_FORMAT2 + 140)
#define FN_GRAPHIC_MIRROR_ON_EVEN_PAGES     (FN_FORMAT2 + 141)

#define FN_TABLE_SPLIT_TABLE                (FN_FORMAT2 + 142)
#define FN_SYNC_LABELS                      (FN_FORMAT2 + 143)

#define FN_FORMAT_LINENUMBER                (FN_FORMAT2 + 144)

#define FN_TABLE_RELATIVE_WIDTH             (FN_FORMAT2 + 147)  /* UNO */
#define FN_TABLE_WIDTH                      (FN_FORMAT2 + 148)  /* UNO */
#define FN_TABLE_IS_RELATIVE_WIDTH          (FN_FORMAT2 + 149)  /* UNO */

#define FN_INC_INDENT_OFFSET                (FN_FORMAT2 + 150)
#define FN_DEC_INDENT_OFFSET                (FN_FORMAT2 + 151)

#define FN_TABLE_MERGE_TABLE                (FN_FORMAT2 + 152)
#define FN_TABLE_ROW_SPLIT                  (FN_FORMAT2 + 153)

#define FN_FORMAT_APPLY_HEAD1               (FN_FORMAT2 + 154)
#define FN_FORMAT_APPLY_HEAD2               (FN_FORMAT2 + 155)
#define FN_FORMAT_APPLY_HEAD3               (FN_FORMAT2 + 156)
#define FN_FORMAT_APPLY_DEFAULT             (FN_FORMAT2 + 157)
#define FN_FORMAT_APPLY_TEXTBODY            (FN_FORMAT2 + 158)
#define FN_REMOVE_DIRECT_CHAR_FORMATS       (FN_FORMAT2 + 159)

#define FN_SAVE_GRAPHIC                     (FN_FORMAT2 + 160)

/*--------------------------------------------------------------------
    Region: Extras
 --------------------------------------------------------------------*/

#define FN_LINE_NUMBERING_DLG       (FN_EXTRA + 2 )   /* */
#define FN_HYPHENATE_OPT_DLG        (FN_EXTRA + 5 )   /* */
#define FN_ADD_UNKNOWN              (FN_EXTRA + 6 )   /* learn words */
#define FN_DICTIONARY_DLG           (FN_EXTRA + 8 )   /* dictionaries */
#define FN_NUMBERING                (FN_EXTRA + 9 )   /* numbering/bullets*/
#define FN_NUMBERING_DLG            (FN_EXTRA + 10)   /* numbering/bullets */
#define FN_NUMBERING_OUTLINE        (FN_EXTRA + 11)   /* */
#define FN_NUMBERING_OUTLINE_DLG    (FN_EXTRA + 12)   /* */
#define FN_SORTING_DLG              (FN_EXTRA + 14)   /* */
#define FN_CALCULATE                (FN_EXTRA + 15)   /* */
#define FN_GENERATE_TOC_DLG         (FN_EXTRA + 16)   /* table of contents */
#define FN_GENERATE_INDEX_DLG       (FN_EXTRA + 17)   /* index */
#define FN_GENERATE_DIR_DLG         (FN_EXTRA + 18)   /* */
#define FN_GLOSSARY_DLG             (FN_EXTRA + 20)   /* text building blocks */
#define FN_MACRO_CHOOSER            (FN_EXTRA + 21)   /* record macro */

#define FN_PLAY_MACRO               (FN_EXTRA + 23)   /* run macro */
#define FN_LAUNCH_BASIC             (FN_EXTRA + 25)   /* run macro */
#define FN_EXPAND_GLOSSARY          (FN_EXTRA + 28)   /* expand text building blocks */
#define FN_CONFIG_TOOLBOX           (FN_EXTRA + 29)   /* configure toolbox*/
#define FN_CONFIG_MENU              (FN_EXTRA + 30)   /* configure menu */
#define FN_CONFIG_KEY               (FN_EXTRA + 31)   /* configure keyboard */
#define FN_CHANGE_PAGENUM           (FN_EXTRA + 34)   /* change page numbers*/

#define FN_MACRO_POPUP              (FN_EXTRA + 37)   /* all macro functions*/
#define FN_BULLET                   (FN_EXTRA + 38)   /* bullet list */

//  Region: Glossary

#define FN_AUTO_CORRECT             (FN_EXTRA + 49 )  /* Autocorrect from Basic */

#define FN_UPDATE_TOX               (FN_EXTRA + 53)   /* update all indices */
#define FN_UPDATE_CUR_TOX           (FN_EXTRA + 54)   /* update current index */
#define FN_REMOVE_CUR_TOX           (FN_EXTRA + 55)  /* remove the current TOX*/

#define FN_NAVIGATION_PI_GOTO_PAGE  (FN_EXTRA + 59 )  /* goto page from navigation-PI */

#define FN_LETTER_WIZZARD           (FN_EXTRA + 60 )
#define FN_FAX_WIZZARD              (FN_EXTRA + 61 )
#define FN_MEMO_WIZZARD             (FN_EXTRA + 62 )
#define FN_AGENDA_WIZZARD           (FN_EXTRA + 63 )

#define FN_SET_BASIC_METRIC         (FN_EXTRA + 80) /* set default metric Basic */

#define FN_RESERVED_9               (FN_EXTRA + 86)   /* place holder */
#define FN_RESERVED_8               (FN_EXTRA + 87)   /* place holder */
#define FN_RESERVED_7               (FN_EXTRA + 88)   /* place holder */
#define FN_RESERVED_6               (FN_EXTRA + 89)   /* place holder */
#define FN_RESERVED_5               (FN_EXTRA + 90)   /* place holder */
#define FN_RESERVED_4               (FN_EXTRA + 91)   /* place holder */
#define FN_RESERVED_3               (FN_EXTRA + 92)   /* place holder */
#define FN_RESERVED_2               (FN_EXTRA + 93)   /* place holder */
#define FN_RESERVED_1               (FN_EXTRA + 94)   /* place holder */

#define FN_COLLECTION_GROUP_CNT     (FN_EXTRA + 96)
#define FN_COLLECTION_GROUP_IDX     (FN_EXTRA + 97)
#define FN_COLL_TYPE                (FN_EXTRA + 98)   /* type for GlobalDoc-Collection*/
#define FN_COLL_ADD                 (FN_EXTRA + 99)

#define FN_COLL_TITLE               (FN_EXTRA2 + 1)  /*  area name or index title */
#define FN_SHADOWCURSOR             (FN_EXTRA2 + 4)  /* enable/disable Shadow Cursor */

#define FN_VIEW_IN_FRAME            (FN_EXTRA2 + 8)  /* the view is in a FrameDoc */

#define FN_UNO_PARA_STYLE           (FN_EXTRA2 + 9)  // starting here are UNI-Ids for the PropertyMap listed
#define FN_UNO_PAGE_STYLE           (FN_EXTRA2 + 10)

#define FN_UNO_FRAME_STYLE_NAME     (FN_EXTRA2 + 12)
#define FN_UNO_NUM_START_VALUE      (FN_EXTRA2 + 13)
#define FN_UNO_NUM_LEVEL            (FN_EXTRA2 + 14)
#define FN_UNO_NUM_RULES            (FN_EXTRA2 + 15)
#define FN_UNO_DOCUMENT_INDEX_MARK  (FN_EXTRA2 + 16)
#define FN_UNO_DOCUMENT_INDEX       (FN_EXTRA2 + 17)
#define FN_UNO_TEXT_FIELD           (FN_EXTRA2 + 18)
#define FN_UNO_TEXT_TABLE           (FN_EXTRA2 + 19)
#define FN_UNO_CELL                 (FN_EXTRA2 + 20)
#define FN_UNO_TEXT_FRAME           (FN_EXTRA2 + 21)
#define FN_UNO_REFERENCE_MARK       (FN_EXTRA2 + 22)
#define FN_UNO_TEXT_SECTION         (FN_EXTRA2 + 23)
#define FN_UNO_FOOTNOTE             (FN_EXTRA2 + 24)
#define FN_UNO_ENDNOTE              (FN_EXTRA2 + 25)
#define FN_UNO_RANGE_COL_LABEL      (FN_EXTRA2 + 26)
#define FN_UNO_RANGE_ROW_LABEL      (FN_EXTRA2 + 27)
#define FN_UNO_TABLE_COLUMS         (FN_EXTRA2 + 28)
#define FN_UNO_TABLE_BORDER         (FN_EXTRA2 + 29)
#define FN_UNO_TABLE_COLUMN_SEPARATORS      (FN_EXTRA2 + 30)
#define FN_UNO_TABLE_COLUMN_RELATIVE_SUM    (FN_EXTRA2 + 31)
#define FN_UNO_TABLE_CELL_BACKGROUND        (FN_EXTRA2 + 32)
#define FN_UNO_ROW_HEIGHT                   (FN_EXTRA2 + 33)
#define FN_UNO_ROW_AUTO_HEIGHT              (FN_EXTRA2 + 34)
#define FN_UNO_HEADER                       (FN_EXTRA2 + 35)
#define FN_UNO_HEADER_LEFT                  (FN_EXTRA2 + 36)
#define FN_UNO_HEADER_RIGHT                 (FN_EXTRA2 + 37)
#define FN_UNO_FOOTER                       (FN_EXTRA2 + 38)
#define FN_UNO_FOOTER_LEFT                  (FN_EXTRA2 + 39)
#define FN_UNO_FOOTER_RIGHT                 (FN_EXTRA2 + 40)
#define FN_UNO_HEADER_BACKGROUND            (FN_EXTRA2 + 41)
#define FN_UNO_HEADER_BOX                   (FN_EXTRA2 + 42)
#define FN_UNO_HEADER_LR_SPACE              (FN_EXTRA2 + 43)
#define FN_UNO_HEADER_SHADOW                (FN_EXTRA2 + 44)
#define FN_UNO_FOOTER_BACKGROUND            (FN_EXTRA2 + 45)
#define FN_UNO_FOOTER_BOX                   (FN_EXTRA2 + 46)
#define FN_UNO_FOOTER_LR_SPACE              (FN_EXTRA2 + 47)
#define FN_UNO_FOOTER_SHADOW                (FN_EXTRA2 + 48)
#define FN_UNO_HEADER_BODY_DISTANCE         (FN_EXTRA2 + 49)
#define FN_UNO_HEADER_IS_DYNAMIC_DISTANCE   (FN_EXTRA2 + 50)
#define FN_UNO_FOOTER_BODY_DISTANCE         (FN_EXTRA2 + 51)
#define FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE   (FN_EXTRA2 + 52)
#define FN_UNO_HEADER_SHARE_CONTENT         (FN_EXTRA2 + 53)
#define FN_UNO_FOOTER_SHARE_CONTENT         (FN_EXTRA2 + 54)
#define FN_UNO_HEADER_HEIGHT                (FN_EXTRA2 + 55)
#define FN_UNO_FOOTER_HEIGHT                (FN_EXTRA2 + 56)
#define FN_UNO_HEADER_ON                    (FN_EXTRA2 + 57)
#define FN_UNO_FOOTER_ON                    (FN_EXTRA2 + 58)
#define FN_UNO_FOLLOW_STYLE                 (FN_EXTRA2 + 59)

#define FN_API_CALL                         (FN_EXTRA2 + 60)

#define FN_UNO_IS_PHYSICAL                  (FN_EXTRA2 + 61)
#define FN_UNO_IS_AUTO_UPDATE               (FN_EXTRA2 + 62)
#define FN_UNO_DISPLAY_NAME                 (FN_EXTRA2 + 63)

#define FN_UNO_WRAP                         (FN_EXTRA2 + 64)
#define FN_UNO_ANCHOR_TYPES                 (FN_EXTRA2 + 65)
#define FN_UNO_PARA_CHAPTER_NUMBERING_LEVEL (FN_EXTRA2 + 66)
#define FN_UNO_PARA_CONDITIONAL_STYLE_NAME  (FN_EXTRA2 + 67)

#define FN_UNO_CATEGORY                     (FN_EXTRA2 + 68)
#define FN_UNO_IS_NUMBER                    (FN_EXTRA2 + 69)
#define FN_UNO_TEXT_WRAP                    (FN_EXTRA2 + 70)
#define FN_UNO_ANCHOR_TYPE                  (FN_EXTRA2 + 71)
#define FN_SKIP_HIDDEN_TEXT                 (FN_EXTRA2 + 72)
#define FN_SKIP_PROTECTED_TEXT              (FN_EXTRA2 + 73)
#define FN_UNO_Z_ORDER                      (FN_EXTRA2 + 74)
#define FN_UNO_REDLINE_NODE_START           (FN_EXTRA2 + 75)
#define FN_UNO_REDLINE_NODE_END             (FN_EXTRA2 + 76)
#define FN_UNO_TEXT_PORTION_TYPE            (FN_EXTRA2 + 77)
#define FN_UNO_CONTROL_CHARACTER            (FN_EXTRA2 + 78)
#define FN_UNO_BOOKMARK                     (FN_EXTRA2 + 79)
#define FN_UNO_IS_COLLAPSED                 (FN_EXTRA2 + 80)
#define FN_UNO_IS_START                     (FN_EXTRA2 + 81)
#define FN_UNO_IS_AUTOMATIC_CONTOUR         (FN_EXTRA2 + 82)
#define FN_UNO_IS_PIXEL_CONTOUR             (FN_EXTRA2 + 83)
#define FN_UNO_ALTERNATIVE_TEXT             (FN_EXTRA2 + 84)
#define FN_UNO_ACTUAL_SIZE                  (FN_EXTRA2 + 85)
#define FN_UNO_GRAPHIC_U_R_L                (FN_EXTRA2 + 86)
#define FN_UNO_GRAPHIC_FILTER               (FN_EXTRA2 + 87)
#define FN_UNO_CELL_NAME                    (FN_EXTRA2 + 88)
#define FN_INSERT_GLOSSARY                  (FN_EXTRA2 + 89)
#define FN_NEW_GLOSSARY                     (FN_EXTRA2 + 90)
#define FN_SET_ACT_GLOSSARY                 (FN_EXTRA2 + 91)

#define FN_UNO_HEADER_EAT_SPACING           (FN_EXTRA2 + 92)
#define FN_UNO_FOOTER_EAT_SPACING           (FN_EXTRA2 + 93)
#define FN_UNO_CHARFMT_SEQUENCE             (FN_EXTRA2 + 94)
#define FN_UNO_CLSID                        (FN_EXTRA2 + 95)
#define FN_UNO_MODEL                        (FN_EXTRA2 + 96)
#define FN_UNO_COMPONENT                    (FN_EXTRA2 + 97)
#define FN_WORDCOUNT_DIALOG                 (FN_EXTRA2 + 98)

#define FN_XFORMS_DESIGN_MODE               (FN_EXTRA2 + 99)

#define FN_UNO_PARA_STYLE_CONDITIONS        (FN_EXTRA2 + 100)
#define FN_UNO_GRAPHIC                      (FN_EXTRA2 + 101)

#define FN_UNO_REPLACEMENT_GRAPHIC_URL      (FN_EXTRA2 + 102)
#define FN_UNO_CELL_ROW_SPAN                (FN_EXTRA2 + 103)
#define FN_UNO_TABLE_BORDER_DISTANCES       (FN_EXTRA2 + 104)
#define FN_SPELL_GRAMMAR_DIALOG             (FN_EXTRA2 + 105)
#define FN_UNO_STREAM_NAME                  (FN_EXTRA2 + 106)
#define FN_UNO_LIST_ID                      (FN_EXTRA2 + 107)
#define FN_UNO_REPLACEMENT_GRAPHIC          (FN_EXTRA2 + 108)

#define FN_UNO_PARA_CONT_PREV_SUBTREE       (FN_EXTRA2 + 109)
#define FN_UNO_PARA_NUM_STRING              (FN_EXTRA2 + 110)
#define FN_UNO_TABLE_NAME                   (FN_EXTRA2 + 111)
#define FN_UNO_META                         (FN_EXTRA2 + 112)
#define FN_UNO_NESTED_TEXT_CONTENT          (FN_EXTRA2 + 113)
/* Edit Graphic with External Tool */
#define FN_EXTERNAL_EDIT                    (FN_EXTRA2 + 114)

/* Navigation buttons */
#define FN_NAVIGATION_BACK                  (FN_EXTRA2 + 115)
#define FN_NAVIGATION_FORWARD               (FN_EXTRA2 + 116)


// #i972: bool items to be passed to SwFrmPage for evaluation
#define FN_OLE_IS_MATH                      (FN_EXTRA2 + 114)
#define FN_MATH_BASELINE_ALIGNMENT          (FN_EXTRA2 + 115)

#define FN_EMBEDDED_OBJECT                  (FN_EXTRA2 + 116)

/*------------------------------------------------ --------------------
    Area: Help
 -------------------------------------------------- ------------------*/
/*------------------------------------------------ --------------------
    Region: Traveling & Selection
 -------------------------------------------------- ------------------*/
#define FN_CHAR_LEFT                (FN_SELECTION + 1 ) /* */
#define FN_CHAR_RIGHT               (FN_SELECTION + 2 ) /* */
#define FN_LINE_UP                  (FN_SELECTION + 3 ) /* */
#define FN_LINE_DOWN                (FN_SELECTION + 4 ) /* */
#define FN_START_OF_LINE            (FN_SELECTION + 5 ) /* StartOfLine */
#define FN_END_OF_LINE              (FN_SELECTION + 6 ) /* EndOfLine */
#define FN_START_OF_DOCUMENT        (FN_SELECTION + 7 ) /* StartOfDocument */
#define FN_START_OF_NEXT_PAGE       (FN_SELECTION + 9 ) /* StartOfNextPage ??? */
#define FN_END_OF_NEXT_PAGE         (FN_SELECTION + 10) /* ??? */
#define FN_START_OF_PREV_PAGE       (FN_SELECTION + 11) /* StartOfPrevPage ??? */
#define FN_END_OF_PREV_PAGE         (FN_SELECTION + 12) /* ??? */
#define FN_START_OF_PAGE            (FN_SELECTION + 13) /* StartOfPage */
#define FN_END_OF_PAGE              (FN_SELECTION + 14) /* EndOfPage */
#define FN_START_OF_WINDOW          (FN_SELECTION + 15) /* StartOfWindow */
#define FN_END_OF_WINDOW            (FN_SELECTION + 16) /* EndOfWindow */
#define FN_START_OF_COLUMN          (FN_SELECTION + 17) /* StartOfColumn */
#define FN_END_OF_COLUMN            (FN_SELECTION + 18) /* EndOfColumn */
#define FN_START_OF_PARA            (FN_SELECTION + 19) /* StartOfPara */
#define FN_END_OF_PARA              (FN_SELECTION + 20) /* EndOfPara */
#define FN_NEXT_WORD                (FN_SELECTION + 21) /* NextWord */
#define FN_PREV_WORD                (FN_SELECTION + 22) /* PrevWord */
#define FN_NEXT_SENT                (FN_SELECTION + 23) /* NextSentence */
#define FN_PREV_SENT                (FN_SELECTION + 24) /* PrevSentence */
#define FN_DELETE                   (FN_SELECTION + 25) /* Delete */
#define FN_BACKSPACE                (FN_SELECTION + 26) /* Backspace */
#define FN_DELETE_SENT              (FN_SELECTION + 27) /* DeleteToEndOfSentence */
#define FN_DELETE_BACK_SENT         (FN_SELECTION + 28) /* DeleteToStartOfSentence */
#define FN_DELETE_WORD              (FN_SELECTION + 29) /* DeleteToEndOfWord */
#define FN_DELETE_BACK_WORD         (FN_SELECTION + 30) /* DeleteToStartOfWord */
#define FN_DELETE_LINE              (FN_SELECTION + 31) /* DeleteToEndOfLine */
#define FN_DELETE_BACK_LINE         (FN_SELECTION + 32) /* DeleteToStartOfLine */
#define FN_DELETE_PARA              (FN_SELECTION + 33) /* DeleteToEndOfPara */
#define FN_DELETE_BACK_PARA         (FN_SELECTION + 34) /* DeleteToStartOfPara */
#define FN_DELETE_WHOLE_LINE        (FN_SELECTION + 35) /* DeleteLine ??? */
#define FN_SET_INS_MODE             (FN_SELECTION + 36) /* Insertmode / Overwrite */
#define FN_PAGEUP                   (FN_SELECTION + 37) /* PageUpWithCursor */
#define FN_PAGEDOWN                 (FN_SELECTION + 38) /* PageDownWithCursor */
#define FN_SET_ADD_MODE             (FN_SELECTION + 39) /* Selektionsmodus */
#define FN_SET_EXT_MODE             (FN_SELECTION + 40) /* Selektionsmodus */
#define FN_ESCAPE                   (FN_SELECTION + 41) /* Normal */
#define FN_SHIFT_BACKSPACE          (FN_SELECTION + 42) /* like Backspace */
#define FN_SELECT_WORD              (FN_SELECTION + 43) /* select word*/

#define FN_GOTO_NEXT_OBJ            (FN_SELECTION + 44)   /* Goto next object */
#define FN_GOTO_PREV_OBJ            (FN_SELECTION + 45)   /* Go to previous object */

#define FN_START_TABLE              (FN_SELECTION + 47) /* to the beginning of the table */
#define FN_END_TABLE                (FN_SELECTION + 48) /* to the end of the table */
#define FN_NEXT_TABLE               (FN_SELECTION + 49) /* to the next table */
#define FN_PREV_TABLE               (FN_SELECTION + 50) /* to the previous table */

#define FN_START_OF_NEXT_COLUMN     (FN_SELECTION + 51) /* to the start of the next column */
#define FN_END_OF_NEXT_COLUMN       (FN_SELECTION + 52) /* to the end of the next column */
#define FN_START_OF_PREV_COLUMN     (FN_SELECTION + 53) /* to the start of the previous column */
#define FN_END_OF_PREV_COLUMN       (FN_SELECTION + 54) /* to the end of the previous column */

#define FN_FOOTNOTE_TO_ANCHOR       (FN_SELECTION + 55) /* from the footnote to the anchor */
#define FN_NEXT_FOOTNOTE            (FN_SELECTION + 56) /* to the next footnote */
#define FN_PREV_FOOTNOTE            (FN_SELECTION + 57) /* to the previous footnote */

#define FN_CNTNT_TO_NEXT_FRAME      (FN_SELECTION + 58) /* from content to the next frame */
#define FN_FRAME_TO_ANCHOR          (FN_SELECTION + 59) /* from frame to the anchor */

#define FN_TO_HEADER                (FN_SELECTION + 60) /* toggle between content and header */
#define FN_TO_FOOTER                (FN_SELECTION + 61) /* toggle between content and footer */

#define FN_IDX_MARK_TO_IDX          (FN_SELECTION + 62) /* from the index mark to the index */

#define FN_TO_FOOTNOTE_AREA         (FN_SELECTION + 63) /* jump into the appropriate footnote area */

#define FN_SET_BLOCK_MODE           (FN_SELECTION + 64) /* selection mode: block */

#define FN_PREV_PARA                (FN_SELECTION + 74)
#define FN_NEXT_PARA                (FN_SELECTION + 75)

#define FN_GOTO_NEXT_MARK           (FN_SELECTION + 76)   /* Go to next marker */
#define FN_GOTO_PREV_MARK           (FN_SELECTION + 77)   /* Go to previous marker */

#define FN_NEXT_TOXMARK             (FN_SELECTION + 83)   /* to the next index mark */
#define FN_PREV_TOXMARK             (FN_SELECTION + 84)   /* to the previous index mark */
#define FN_NEXT_TBLFML              (FN_SELECTION + 85)   /* to the next table formula */
#define FN_PREV_TBLFML              (FN_SELECTION + 86)   /* to the previous table formula */
#define FN_NEXT_TBLFML_ERR          (FN_SELECTION + 87)   /* to the next broken table formula */
#define FN_PREV_TBLFML_ERR          (FN_SELECTION + 88)   /* to the previous broken table formula */

#define FN_READONLY_SELECTION_MODE  (FN_SELECTION + 89)   /* switches text selection mode in readonly documents*/
#define FN_SELECTION_MODE_DEFAULT   (FN_SELECTION + 90)   /* use default selection mode - not block mode */
#define FN_SELECTION_MODE_BLOCK     (FN_SELECTION + 91)   /* switch on block selection */

/*--------------------------------------------------------------------
    QUERY-Block
 --------------------------------------------------------------------*/

#define FN_IS_MODIFIED              (FN_QUERY +16)      /* Was the document modified? */
#define FN_RESET_MODIFIED           (FN_QUERY +17)      /* Reset modified flag */

#define FN_GET_DOC_INFO             (FN_QUERY +22)      /* Wert Dokumentinfo erfragen */
#define FN_GET_ERRNO                (FN_QUERY +26)      /* Get error number */
#define FN_GET_DOCMAN_PATH          (FN_QUERY +27)      /* Get path to a group in Docmgr */

#define FN_TXTATR_INET              (FN_QUERY +29)      /* INet-Attribut */
#define FN_EXECUTE_DRAG             (FN_QUERY +30)      /* Start drag and drop */

#define FN_GET_PRINT_AREA           (FN_QUERY +32)      /* Get attribute for printable area of page */

#define FN_GET_PARAGRAPH            (FN_QUERY +48)      /* Get Paragraph-Object from the text shell to the view */

#define FN_DB_FORM_LETTER               (FN_QUERY2 + 0)     /* form letter */
#define FN_DB_INSERT_CONTENT            (FN_QUERY2 + 1)     /* merge data into fields*/
#define FN_DB_INSERT_COLUMNS            (FN_QUERY2 + 2)     /* insert data into text/table/as fields*/
#define FN_DB_CONNECTION_ANY            (FN_QUERY2 + 3)
#define FN_DB_COLUMN_ANY                (FN_QUERY2 + 4)
#define FN_DB_DATA_SOURCE_ANY           (FN_QUERY2 + 5)
#define FN_DB_DATA_COMMAND_ANY          (FN_QUERY2 + 6)
#define FN_DB_DATA_COMMAND_TYPE_ANY     (FN_QUERY2 + 7)
#define FN_DB_DATA_COLUMN_NAME_ANY      (FN_QUERY2 + 8)
#define FN_DB_DATA_SELECTION_ANY        (FN_QUERY2 + 9)
#define FN_DB_DATA_CURSOR_ANY           (FN_QUERY2 + 10)

/*--------------------------------------------------------------------
    Region: Envelope
 --------------------------------------------------------------------*/

#define FN_ENVELOP                  (FN_ENVELP    )


#define FN_DELETE_NOTE_AUTHOR       (FN_NOTES+1)
#define FN_DELETE_ALL_NOTES         (FN_NOTES+2)
#define FN_HIDE_NOTE                (FN_NOTES+3)
#define FN_HIDE_NOTE_AUTHOR         (FN_NOTES+4)
#define FN_HIDE_ALL_NOTES           (FN_NOTES+5)

#define FN_DELETE_COMMENT           (FN_NOTES+6)
#define FN_REPLY                    (FN_NOTES+7)


#define FN_HEADERFOOTER_EDIT        (FN_HEADERFOOTER+1)
#define FN_HEADERFOOTER_DELETE      (FN_HEADERFOOTER+2)
#define FN_HEADERFOOTER_BORDERBACK  (FN_HEADERFOOTER+3)

#define FN_PAGEBREAK_EDIT           (FN_PAGEBREAK+1)
#define FN_PAGEBREAK_DELETE         (FN_PAGEBREAK+2)

/*--------------------------------------------------------------------
    Region: Parameter
 --------------------------------------------------------------------*/

#define FN_PARAM_TABLE_COLS         (FN_PARAM)
#define FN_PARAM_TABLE_ROWS         (FN_PARAM+1)
#define FN_PARAM_MOVE_COUNT         (FN_PARAM+2)
#define FN_PARAM_MOVE_SELECTION     (FN_PARAM+3)

#define FN_PARAM_SEARCH_WORDONLY    (FN_PARAM+5)
#define FN_PARAM_SEARCH_EXACT       (FN_PARAM+6)
#define FN_PARAM_SEARCH_REGEXP      (FN_PARAM+7)
#define FN_PARAM_SEARCH_BACKWARDS   (FN_PARAM+8)
#define FN_PARAM_SEARCH_FINDALL     (FN_PARAM+9)
#define FN_PARAM_SEARCH_SELECTION   (FN_PARAM+10)
#define FN_PARAM_SEARCH_TEXT        (FN_PARAM+11)
#define FN_PARAM_SEARCH_TEXTREPLACE (FN_PARAM+12)

#define FN_PARAM_PARADLGLIMITS      (FN_PARAM+15)
#define FN_PARAM_FRMMODE            (FN_PARAM+16)

#define FN_PARAM_ADDPRINTER         (FN_PARAM+18)
#define FN_PARAM_GRID               (FN_PARAM+19)
#define FN_PARAM_DOCDISP            (FN_PARAM+20)
#define FN_PARAM_ELEM               (FN_PARAM+21)
#define FN_PARAM_SWTEST             (FN_PARAM+22)

#define FN_PARAM_FTN_INFO           (FN_PARAM+23)

#define FN_PARAM_REGION_NAME            (FN_PARAM+24)
#define FN_PARAM_REGION_CONDITION       (FN_PARAM+25)
#define FN_PARAM_REGION_HIDDEN          (FN_PARAM+26)
#define FN_PARAM_REGION_PROTECT         (FN_PARAM+27)
#define FN_PARAM_REGION_EDIT_IN_READONLY (FN_PARAM+28)

#define FN_PARAM_INSERT_AFTER           (FN_PARAM+29)
#define FN_PARAM_INDEX_NAME             (FN_PARAM+31)
#define FN_PARAM_INDEX_OPTIONS          (FN_PARAM+32)
#define FN_PARAM_INDEX_FORMAT           (FN_PARAM+33)
#define FN_PARAM_INDEX_LEVEL            (FN_PARAM+34)
#define FN_PARAM_INDEX_TEMPLATE         (FN_PARAM+35)
#define FN_PARAM_INDEX_ALTSTR           (FN_PARAM+36)
#define FN_PARAM_INDEX_PRIMKEY          (FN_PARAM+37)
#define FN_PARAM_INDEX_SECKEY           (FN_PARAM+38)
#define FN_PARAM_WIDTH                  (FN_PARAM+39)
#define FN_PARAM_LOCATION               (FN_PARAM+40)
#define FN_PARAM_DIR                    (FN_PARAM+41)
#define FN_PARAM_COUNT                  (FN_PARAM+42)
#define FN_PARAM_COLOR                  (FN_PARAM+43)

#define FN_PARAM_TABLE_NAME             (FN_PARAM+44)
#define FN_PARAM_TABLE_WIDTH            (FN_PARAM+45)
#define FN_PARAM_TABLE_ULSPACE          (FN_PARAM+46)
#define FN_PARAM_TABLE_LRSPACE          (FN_PARAM+47)
#define FN_PARAM_TABLE_SHADOW           (FN_PARAM+48)
#define FN_PARAM_TABLE_ALIGN            (FN_PARAM+49)
#define FN_PARAM_TABLE_HEADLINE         (FN_PARAM+50)
#define FN_PARAM_TABLE_SPACE            (FN_PARAM+51)
#define FN_PARAM_TABLE_COLUMNS          (FN_PARAM+52)

#define FN_PARAM_GRF_CONNECT            (FN_PARAM+53)
#define FN_PARAM_FIELD_TYPE             (FN_PARAM+54)
#define FN_PARAM_FIELD_SUBTYPE          (FN_PARAM+56)
#define FN_PARAM_FIELD_FORMAT           (FN_PARAM+57)

#define FN_PARAM_EVENT                  (FN_PARAM+68)
#define FN_PARAM_URL                    (FN_PARAM+69)
#define FN_PARAM_GRF_REALSIZE           (FN_PARAM+70)
#define FN_PARAM_GRF_DIALOG             (FN_PARAM+71)
#define FN_PARAM_GRF_CLIENTMAP          (FN_PARAM+74) /* ClientMap */
#define FN_PARAM_GRF_SERVERMAP          (FN_PARAM+75) /* ServerMap */
#define FN_PARAM_GRF_TARGETFRAME        (FN_PARAM+76) /* TargetFrame */
#define FN_INET_FIELD_MACRO             (FN_PARAM+77) /* Id for URL-Field-Macros*/

#define FN_PARAM_PRINTER                (FN_PARAM+78) /* Printer */
#define FN_PARAM_STDFONTS               (FN_PARAM+79) /* ConfigItem Standardfonts */

#define FN_PARAM_WRTSHELL               (FN_PARAM2) /* SwWrtShell */

#define FN_COND_COLL                    (FN_PARAM2+1) /* Item for conditional templates */
#define FN_PARAM_SELECTION              (FN_PARAM2+2) /* selected word for format/character/hyperlink */

#define FN_PARAM_ACT_NUMBER             (FN_PARAM2+3) /* PointerItem die aktuelle NumRule */
#define FN_PARAM_CHILD_LEVELS           (FN_PARAM2+4) /* are child levels used? */

#define FN_PARAM_NUM_PRESET             (FN_PARAM2+5) /* predefined numbering */

#define FN_PARAM_HEIGHT                 (FN_PARAM2+6) /* Param for height */
#define FN_PARAM_DIST                   (FN_PARAM2+7) /* and width, for metric transformation */

#define FN_PARAM_SHADOWCURSOR           (FN_PARAM2+8) /* for ShadowCursor options */

#define FN_PARAM_ACT_NUMLEVEL           (FN_PARAM2+9) /* BYTE-Item with current NumLevel */

#define FN_PARAM_10                     (FN_PARAM2+11)

#define FN_TEXT_RANGE                   (FN_PARAM2+12) /* TextRange Property*/

#define FN_PARAM_CRSR_IN_PROTECTED      (FN_PARAM2+13) /* Cursor in protected areas */
#define FN_PARAM_TOX_TYPE               (FN_PARAM2+14) /* TOX type in tox dialog*/
#define FN_PARAM_LINK_DISPLAY_NAME      (FN_PARAM2+15) /* LinkDisplayName property*/
#define FN_PARAM_NUM_LEVEL              (FN_PARAM2+16) /* rtf filter*/
#define FN_PARAM_COUNTOUR_PP            (FN_PARAM2+17) /* contour PolyPolygon*/

#define FN_ANCHOR_POSITION              (FN_PARAM2+18) /* AnchorPosition property */
#define FN_DROP_TEXT                    (FN_PARAM2+18) /* text set in drop caps tab page - for recording */
#define FN_DROP_CHAR_STYLE_NAME         (FN_PARAM2+19) /* character style of drop caps - for recording */
#define FN_PARAM_CHAIN_PREVIOUS         (FN_PARAM2+20) /* Name of frame to be added as chain successor */
#define FN_PARAM_CHAIN_NEXT             (FN_PARAM2+21) /* Name of frame to be added as chain predecessor */
#define FN_PARAM_DATABASE_PROPERTIES    (FN_PARAM2+22) /* transport a Sequence<PropertyValue> containing database properties*/
#define FN_SHAPE_TRANSFORMATION_IN_HORI_L2R (FN_PARAM2+23)
#define FN_SHAPE_POSITION_LAYOUT_DIR    (FN_PARAM2+24)
#define FN_SHAPE_STARTPOSITION_IN_HORI_L2R (FN_PARAM2+25)
#define FN_SHAPE_ENDPOSITION_IN_HORI_L2R   (FN_PARAM2+26)
#define FN_PARAM_PAM                    (FN_PARAM2+27) /* Point and Mark */

/*--------------------------------------------------------------------
    Bereich: Druckoptionen
 --------------------------------------------------------------------*/

#define FN_PRNOPT_GRAPHIC           (FN_PRNOPT + 1)
#define FN_PRNOPT_TABLE             (FN_PRNOPT + 2)
#define FN_PRNOPT_LEFT_PAGE         (FN_PRNOPT + 3)
#define FN_PRNOPT_RIGHT_PAGE        (FN_PRNOPT + 4)
#define FN_PRNOPT_REVERSE           (FN_PRNOPT + 5)
#define FN_PRNOPT_PAPER_FROM_SETUP  (FN_PRNOPT + 6)
#define FN_PRNOPT_POSTITS           (FN_PRNOPT + 8)

#define FN_PRNOPT_BEGIN  FN_PRNOPT_GRAPHIC
#define FN_PRNOPT_END    FN_PRNOPT_POSTITS

/*--------------------------------------------------------------------
    Description: Status: not more than 19!
 --------------------------------------------------------------------*/

#define FN_STAT_PAGE                (FN_STAT + 1)
#define FN_STAT_TEMPLATE            (FN_STAT + 2)
#define FN_STAT_ZOOM                (FN_STAT + 3)
#define FN_STAT_INSOVER             (FN_STAT + 4)
#define FN_STAT_SELMODE             (FN_STAT + 5)
#define FN_STAT_CONTEXT             (FN_STAT + 7)
#define FN_STAT_BOOKMARK            (FN_STAT + 8)  /* For Popup Bookmarks*/

/*--------------------------------------------------------------------
    Region: Page preview
 --------------------------------------------------------------------*/

#define FN_SHOW_TWO_PAGES           (FN_PGPREVIEW + 1)
#ifndef FN_SHOW_MULTIPLE_PAGES
#define FN_SHOW_MULTIPLE_PAGES      (FN_PGPREVIEW + 2)
#endif
#define FN_PRINT_PAGEPREVIEW        (FN_PGPREVIEW + 3)
#define FN_CLOSE_PAGEPREVIEW        (FN_PGPREVIEW + 4)
#define FN_SHOW_BOOKVIEW            (FN_PGPREVIEW + 5)

/*--------------------------------------------------------------------
    Region: Setting Attributes
 --------------------------------------------------------------------*/

#define FN_OPAQUE                       (FN_FRAME + 1)
#define FN_SET_PROTECT                  (FN_FRAME + 2)
#define FN_SURROUND                     (FN_FRAME + 3)
#define FN_VERT_ORIENT                  (FN_FRAME + 4)
#define FN_HORI_ORIENT                  (FN_FRAME + 5)
#define FN_SET_FRM_NAME                 (FN_FRAME + 6)
#define FN_KEEP_ASPECT_RATIO            (FN_FRAME + 7)
#define FN_GRF_KEEP_ZOOM                (FN_FRAME + 8)

#define FN_IID_COL1                     (FN_FRAME + 13)
#define FN_IID_COL2                     (FN_FRAME + 14)
#define FN_IID_COL3                     (FN_FRAME + 15)
#define FN_IID_COLLEFT                  (FN_FRAME + 16)
#define FN_IID_COLRIGHT                 (FN_FRAME + 17)

#define FN_SET_FRM_ALT_NAME             (FN_FRAME + 18)
#define FN_UNO_TITLE                    (FN_FRAME + 19)
#define FN_UNO_DESCRIPTION              (FN_FRAME + 20)

//Member IDs for Fill / SetVariable of items
#define         MID_STYLE       0xe0
#define         MID_PWIDTH      0xe1
#define         MID_ADJUST      0xe2
#define         MID_TDIST       0xe3
#define         MID_BDIST       0xe4
#define         MID_LINES       0xe5
#define         MID_CHARS       0xe6
#define         MID_DIST        0xe7

#define         MID_1           0xe8
#define         MID_2           0xe9
#define         MID_3           0xea
#define         MID_4           0xeb
#define         MID_5           0xec
#define         MID_6           0xed
#define         MID_7           0xee
#define         MID_8           0xef
#define         MID_9           0xf0
#define         MID_A           0xf1
#define         MID_B           0xf2
#define         MID_C           0xf3
#define         MID_D           0xf4
#define         MID_E           0xf5
#define         MID_F           0xf6
#define         MID_10          0xf7
#define         MID_11          0xf8
#define         MID_12          0xf9
#define         MID_13          0xfa

#ifndef SID_SW_DOCMAN_PATH
#define SID_SW_DOCMAN_PATH                              (SID_OFASLOTS_START +   6)
#endif

// defines for Event-assignments per Macro
#define MAC_EVENT_MOUSEOVER         0x01            //
#define MAC_EVENT_MOUSECLICK        0x02            //
#define MAC_EVENT_MOUSEOUT          0x03            //
#define MAC_EVENT_OBJECT_SELECT     0x04            //
#define MAC_EVENT_KEYINPUT_ALPHA    0x05            //
#define MAC_EVENT_KEYINPUT_NOALPHA  0x06            //
#define MAC_EVENT_FRM_RESIZE        0x07            //
#define MAC_EVENT_FRM_MOVE          0x08            //
#define MAC_EVENT_IMAGE_LOAD        0x09            //
#define MAC_EVENT_IMAGE_ABORT       0x0a            //
#define MAC_EVENT_IMAGE_ERROR       0x0b            //

#endif /* INCLUDED_SW_CMDID_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
