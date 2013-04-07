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

#ifndef _PAD_FONTENTRY_HXX_
#define _PAD_FONTENTRY_HXX_

#include "progress.hxx"
#include "helper.hxx"

#include "vcl/timer.hxx"
#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/combobox.hxx"
#include "vcl/lstbox.hxx"
#include "vcl/group.hxx"
#include "vcl/fontmanager.hxx"

#include "tools/urlobj.hxx"

namespace padmin {

    class FontImportDialog :
        public ModalDialog,
        public ::psp::PrintFontManager::ImportFontCallback
    {
        OKButton                            m_aOKBtn;
        CancelButton                        m_aCancelBtn;
        PushButton                          m_aSelectAllBtn;
        ListBox                             m_aNewFontsBox;
        FixedLine                           m_aFromFL;
        Edit                                m_aFromDirEdt;
        PushButton                          m_aFromBtn;
        CheckBox                            m_aSubDirsBox;
        FixedLine                           m_aTargetOptFL;
        CheckBox                            m_aLinkOnlyBox;
        FixedText                           m_aFixedText;
        bool                                m_bOverwriteAll;
        bool                                m_bOverwriteNone;
        ProgressDialog*                     m_pProgress;
        int                                 m_nFont;

        String                              m_aImportOperation;
        String                              m_aOverwriteQueryText;
        String                              m_aOverwriteAllText;
        String                              m_aOverwriteNoneText;
        String                              m_aNoAfmText;
        String                              m_aAfmCopyFailedText;
        String                              m_aFontCopyFailedText;
        String                              m_aNoWritableFontsDirText;
        String                              m_aFontsImportedText;

        ::boost::unordered_map< ::rtl::OString, ::std::list< ::psp::FastPrintFontInfo >, ::rtl::OStringHash >
                                            m_aNewFonts;

        Timer                               m_aRefreshTimer;
        DECL_LINK( RefreshTimeoutHdl, void* );


        ::psp::PrintFontManager&            m_rFontManager;

        DECL_LINK( ClickBtnHdl, Button* );
        DECL_LINK( ModifyHdl, Edit* );
        DECL_LINK( ToggleHdl, CheckBox* );

        // implement ImportFontCallback
        virtual void importFontsFailed( ::psp::PrintFontManager::ImportFontCallback::FailCondition eReason );
        virtual void progress( const ::rtl::OUString& rFile );
        virtual bool queryOverwriteFile( const ::rtl::OUString& rFile );
        virtual void importFontFailed( const ::rtl::OUString& rFile, ::psp::PrintFontManager::ImportFontCallback::FailCondition eReason );
        virtual bool isCanceled();

        void copyFonts();
        void fillFontBox();
    public:
        FontImportDialog( Window* );
        ~FontImportDialog();
    };

    class FontNameDlg : public ModalDialog
    {
    private:
        OKButton                    m_aOKButton;
        PushButton                  m_aRenameButton;
        PushButton                  m_aRemoveButton;
        PushButton                  m_aImportButton;

        DelListBox                  m_aFontBox;
        FixedText                   m_aFixedText;

        String                      m_aRenameString;
        String                      m_aRenameTTCString;
        String                      m_aNoRenameString;

        ::psp::PrintFontManager&    m_rFontManager;

        // maps fontID to XLFD
        ::boost::unordered_map< ::psp::fontID, String >
                                    m_aFonts;
        void init();
    public:
        FontNameDlg( Window* );
        ~FontNameDlg();

        DECL_LINK( ClickBtnHdl, Button* );
        DECL_LINK( DelPressedHdl, ListBox* );
        DECL_LINK( SelectHdl, ListBox* );

        static String fillFontEntry( ::psp::FastPrintFontInfo& rInfo, const String& rFile, bool bAddRegular );
        static String fillFontEntry( const ::std::list< ::psp::FastPrintFontInfo >& rInfos, const String& rFile );
    };
} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
