/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/macros.h>
#include "iodlg.hxx"
#include <svtools/PlaceEditDialog.hxx>
#include "PlacesListBox.hxx"
#include "fpsofficeResMgr.hxx"
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <unotools/ucbhelper.hxx>
#include "svtools/ehdl.hxx"
#include "svl/urihelper.hxx"
#include "unotools/pathoptions.hxx"
#include "unotools/viewoptions.hxx"
#include "svtools/fileview.hxx"
#include "svtools/sfxecode.hxx"
#include "svtools/svtabbx.hxx"
#include "svtools/treelistentry.hxx"
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/localfilehelper.hxx>

#include "svtools/helpid.hrc"
#include <svtools/svtools.hrc>
#include "OfficeFilePicker.hrc"
#include "iodlg.hrc"
#include "asyncfilepicker.hxx"
#include "iodlgimp.hxx"
#include "svtools/inettbc.hxx"
#include "unotools/syslocale.hxx"
#include "QueryFolderName.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <comphelper/interaction.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <osl/file.h>
#include <vcl/dibtools.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/task/InteractionHandler.hpp>
#include "com/sun/star/ucb/InteractiveAugmentedIOException.hpp"
#include "fpinteraction.hxx"
#include <osl/process.h>

#include <officecfg/Office/Common.hxx>

#include <algorithm>
#include <functional>
#include <vector>

//#define AUTOSELECT_USERFILTER
    // define this for the experimental feature of user-filter auto selection
    // means if the user enters e.g. *.doc<enter>, and there is a filter which is responsible for *.doc files (only),
    // then this filter is selected automatically

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::sdbc;
using namespace ::utl;
using namespace ::svt;

using namespace ExtendedFilePickerElementIds;
using namespace CommonFilePickerElementIds;
using namespace InternalFilePickerElementIds;

#define IODLG_CONFIGNAME        OUString("FileDialog")
#define IMPGRF_CONFIGNAME       OUString("ImportGraphicDialog")

#define GET_DECODED_NAME(aObj) \
    aObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET )

// Time to wait while traveling in the filterbox until
// the browsebox gets filtered ( in ms).
#define TRAVELFILTER_TIMEOUT    750

#define WIDTH_ADDITION  15

// functions -------------------------------------------------------------

namespace
{


    OUString getMostCurrentFilter( SvtExpFileDlg_Impl* pImpl )
    {
        DBG_ASSERT( pImpl, "invalid impl pointer" );
        const SvtFileDialogFilter_Impl* pFilter = pImpl->_pUserFilter;

        if ( !pFilter )
            pFilter = pImpl->GetCurFilter();

        if ( !pFilter )
            return OUString();

        return pFilter->GetType();
    }


    bool restoreCurrentFilter( SvtExpFileDlg_Impl* _pImpl )
    {
        DBG_ASSERT( _pImpl->GetCurFilter(), "restoreCurrentFilter: no current filter!" );
        DBG_ASSERT( !_pImpl->GetCurFilterDisplayName().isEmpty(), "restoreCurrentFilter: no current filter (no display name)!" );

        _pImpl->SelectFilterListEntry( _pImpl->GetCurFilterDisplayName() );

#ifdef DBG_UTIL
        OUString sSelectedDisplayName;
        DBG_ASSERT( ( _pImpl->GetSelectedFilterEntry( sSelectedDisplayName ) == _pImpl->GetCurFilter() )
                &&  ( sSelectedDisplayName == _pImpl->GetCurFilterDisplayName() ),
            "restoreCurrentFilter: inconsistence!" );
#endif
        return _pImpl->m_bNeedDelayedFilterExecute;
    }


    OUString GetFsysExtension_Impl( const OUString& rFile, const OUString& rLastFilterExt )
    {
        sal_Int32 nDotPos = rFile.lastIndexOf( '.' );
        if ( nDotPos != -1 )
        {
            if ( !rLastFilterExt.isEmpty() )
            {
                if ( rFile.copy( nDotPos + 1 ).equalsIgnoreAsciiCase( rLastFilterExt ) )
                    return rLastFilterExt;
            }
            else
                return rFile.copy( nDotPos );
        }
        return OUString();
    }


    void SetFsysExtension_Impl( OUString& rFile, const OUString& rExtension )
    {
        const sal_Unicode* p0 = rFile.getStr();
        const sal_Unicode* p1 = p0 + rFile.getLength() - 1;
        while ( p1 >= p0 && *p1 != '.' )
            p1--;
        if ( p1 >= p0 )
            // remove old extension
            rFile = rFile.copy( 0, p1 - p0 + 1 - ( rExtension.getLength() > 0 ? 0 : 1 ) );
        else if ( !rExtension.isEmpty() )
            // no old extension
            rFile += ".";
        rFile += rExtension;
    }


    // move the control with the given offset
    void lcl_MoveControl( Control* _pControl, sal_Int32 _nDeltaX, sal_Int32 _nDeltaY, sal_Int32* _pMaxY = NULL )
    {
        if ( _pControl )
        {
            Point aNewPos = _pControl->GetPosPixel();

            // adjust the vertical position
            aNewPos.Y() += _nDeltaY;
            if ( _pMaxY && ( aNewPos.Y() > *_pMaxY ) )
                *_pMaxY = aNewPos.Y();

            // adjust the horizontal position
            aNewPos.X() += _nDeltaX;

            _pControl->SetPosPixel( aNewPos );
        }
    }


    void lcl_autoUpdateFileExtension( SvtFileDialog* _pDialog, const OUString& _rLastFilterExt )
    {
        // if auto extension is enabled ....
        if ( _pDialog->isAutoExtensionEnabled() )
        {
            // automatically switch to the extension of the (maybe just newly selected) extension
            OUString aNewFile = _pDialog->getCurrentFileText( );
            OUString aExt = GetFsysExtension_Impl( aNewFile, _rLastFilterExt );

            // but only if there already is an extension
            if ( !aExt.isEmpty() )
            {
                // check if it is a real file extension, and not only the "post-dot" part in
                // a directory name
                bool bRealExtensions = true;
                if ( -1 != aExt.indexOf( '/' ) )
                    bRealExtensions = false;
                else if ( -1 != aExt.indexOf( '\\' ) )
                    bRealExtensions = false;
                else
                {
                    // no easy way to tell, because the part containing the dot already is the last
                    // segment of the complete file name
                    // So we have to check if the file name denotes a folder or a file.
                    // For performance reasons, we do this for file urls only
                    INetURLObject aURL( aNewFile );
                    if ( INET_PROT_NOT_VALID == aURL.GetProtocol() )
                    {
                        OUString sURL;
                        if ( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aNewFile, sURL ) )
                            aURL = INetURLObject( sURL );
                    }
                    if ( INET_PROT_FILE == aURL.GetProtocol() )
                    {
                        try
                        {
                            bRealExtensions = !_pDialog->ContentIsFolder( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
                        }
                        catch( const ::com::sun::star::uno::Exception& )
                        {
                            DBG_WARNING( "Exception in lcl_autoUpdateFileExtension" );
                        }
                    }
                }

                if ( bRealExtensions )
                {
                    SetFsysExtension_Impl( aNewFile, _pDialog->GetDefaultExt() );
                    _pDialog->setCurrentFileText( aNewFile );
                }
            }
        }
    }


    bool lcl_getHomeDirectory( const OUString& _rForURL, OUString& /* [out] */ _rHomeDir )
    {
        _rHomeDir = "";

        // now ask the content broker for a provider for this scheme

        try
        {
            // get the provider for the current scheme
            Reference< XContentProvider > xProvider(
                UniversalContentBroker::create(
                    comphelper::getProcessComponentContext() )->
                queryContentProvider( _rForURL ) );

            DBG_ASSERT( xProvider.is(), "lcl_getHomeDirectory: could not find a (valid) content provider for the current URL!" );
            Reference< XPropertySet > xProviderProps( xProvider, UNO_QUERY );
            if ( xProviderProps.is() )
            {
                Reference< XPropertySetInfo > xPropInfo = xProviderProps->getPropertySetInfo();
                const OUString sHomeDirPropertyName( "HomeDirectory" );
                if ( !xPropInfo.is() || xPropInfo->hasPropertyByName( sHomeDirPropertyName ) )
                {
                    OUString sHomeDirectory;
                    xProviderProps->getPropertyValue( sHomeDirPropertyName ) >>= sHomeDirectory;
                    _rHomeDir = sHomeDirectory;
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "lcl_getHomeDirectory: caught an exception!" );
        }
        return !_rHomeDir.isEmpty();
    }


    static OUString lcl_ensureFinalSlash( const OUString& _rDir )
    {
        INetURLObject aWorkPathObj( _rDir, INET_PROT_FILE );
        aWorkPathObj.setFinalSlash();
        return  aWorkPathObj.GetMainURL( INetURLObject::NO_DECODE );
    }


    /** retrieves the value of an environment variable
        @return <TRUE/> if and only if the retrieved string value is not empty
    */
    bool getEnvironmentValue( const sal_Char* _pAsciiEnvName, OUString& _rValue )
    {
        _rValue = OUString();
        OUString sEnvName = OUString::createFromAscii( _pAsciiEnvName );
        osl_getEnvironment( sEnvName.pData, &_rValue.pData );
        return !_rValue.isEmpty();
    }
}


// ControlChain_Impl


struct ControlChain_Impl
{
    Window*            _pControl;
    ControlChain_Impl* _pNext;
    bool               _bHasOwnership;

    ControlChain_Impl( Window* pControl, ControlChain_Impl* pNext );
    ~ControlChain_Impl();
};



ControlChain_Impl::ControlChain_Impl
(
    Window* pControl,
    ControlChain_Impl* pNext
)
    : _pControl( pControl ),
      _pNext( pNext ),
      _bHasOwnership( true )
{
}



ControlChain_Impl::~ControlChain_Impl()
{
    if ( _bHasOwnership )
    {
        delete _pControl;
    }
    delete _pNext;
}


// SvtFileDialog

SvtFileDialog::SvtFileDialog
(
    Window* _pParent,
    WinBits nBits,
    WinBits nExtraBits
) :
    ModalDialog( _pParent, SvtResId( DLG_FPICKER_EXPLORERFILE ) )

    ,_pUserControls( NULL )
    ,_pCbReadOnly( NULL )
    ,_pCbLinkBox( NULL)
    ,_pCbPreviewBox( NULL )
    ,_pCbSelection( NULL )
    ,_pPbPlay( NULL )
    ,_pPrevWin( NULL )
    ,_pPrevBmp( NULL )
    ,_pFileView( NULL )
    ,_pFileNotifier( NULL )
    ,_pImp( new SvtExpFileDlg_Impl( nBits ) )
    ,_nExtraBits( nExtraBits )
    ,_bIsInExecute( false )
    ,m_bInExecuteAsync( false )
    ,m_bHasFilename( false )
    ,m_context(comphelper::getProcessComponentContext())
{
    Init_Impl( nBits );
}



SvtFileDialog::SvtFileDialog ( Window* _pParent, WinBits nBits )
    :ModalDialog( _pParent, SvtResId( DLG_FPICKER_EXPLORERFILE ) )
    ,_pUserControls( NULL )
    ,_pCbReadOnly( NULL )
    ,_pCbLinkBox( NULL)
    ,_pCbPreviewBox( NULL )
    ,_pCbSelection( NULL )
    ,_pPbPlay( NULL )
    ,_pPrevWin( NULL )
    ,_pPrevBmp( NULL )
    ,_pFileView( NULL )
    ,_pFileNotifier( NULL )
    ,_pImp( new SvtExpFileDlg_Impl( nBits ) )
    ,_nExtraBits( 0L )
    ,_bIsInExecute( false )
    ,m_bHasFilename( false )
{
    Init_Impl( nBits );
}



SvtFileDialog::~SvtFileDialog()
{
    if ( !_pImp->_aIniKey.isEmpty() )
    {
        // save window state
        SvtViewOptions aDlgOpt( E_DIALOG, _pImp->_aIniKey );
        aDlgOpt.SetWindowState(OStringToOUString(GetWindowState(), osl_getThreadTextEncoding()));
        OUString sUserData = _pFileView->GetConfigString();
        aDlgOpt.SetUserItem( OUString( "UserData" ),
                             makeAny( sUserData ) );
    }

    _pFileView->SetSelectHdl( Link() );

    // Save bookmarked places
    if(_pImp->_pPlaces->IsUpdated()) {
        const std::vector<PlacePtr> aPlaces = _pImp->_pPlaces->GetPlaces();
        Sequence< OUString > placesUrlsList(_pImp->_pPlaces->GetNbEditablePlaces());
        Sequence< OUString > placesNamesList(_pImp->_pPlaces->GetNbEditablePlaces());
        int i(0);
        for(std::vector<PlacePtr>::const_iterator it = aPlaces.begin(); it != aPlaces.end(); ++it) {
            if((*it)->IsEditable()) {
                placesUrlsList[i] = (*it)->GetUrl();
                placesNamesList[i] = (*it)->GetName();
                ++i;
            }
        }

        boost::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(m_context));
        officecfg::Office::Common::Misc::FilePickerPlacesUrls::set(placesUrlsList, batch, m_context);
        officecfg::Office::Common::Misc::FilePickerPlacesNames::set(placesNamesList, batch, m_context);
        batch->commit();
    }

    delete _pImp;
    delete _pFileView;
    delete _pSplitter;

    delete _pCbReadOnly;
    delete _pCbLinkBox;
    delete _pCbPreviewBox;
    delete _pCbSelection;
    delete _pPbPlay;
    delete _pPrevWin;
    delete _pPrevBmp;

    delete _pUserControls;
}



void SvtFileDialog::Init_Impl
(
    WinBits nStyle
)
{
    m_aImages = ImageList( SvtResId( RID_FILEPICKER_IMAGES ) );

    _pImp->_nStyle = nStyle;
    _pImp->_a6Size = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    _pImp->_eMode = ( nStyle & WB_SAVEAS ) ? FILEDLG_MODE_SAVE : FILEDLG_MODE_OPEN;
    _pImp->_eDlgType = FILEDLG_TYPE_FILEDLG;

    if ( ( nStyle & SFXWB_PATHDIALOG ) == SFXWB_PATHDIALOG )
        _pImp->_eDlgType = FILEDLG_TYPE_PATHDLG;

    // Set the directory for the "back to the default dir" button
    INetURLObject aStdDirObj( SvtPathOptions().GetWorkPath() );
    SetStandardDir( aStdDirObj.GetMainURL( INetURLObject::NO_DECODE ) );

    // determine range
    if ( !( nStyle & SFXWB_NOREMOTE ) )
    {
        _pImp->_nState |= FILEDLG_STATE_REMOTE;
    }

    // Create control element, the order defines the tab control.
    _pImp->_pFtFileName = new FixedText( this, SvtResId( FT_EXPLORERFILE_FILENAME ) );

    SvtURLBox* pURLBox = new SvtURLBox( this, SvtResId( ED_EXPLORERFILE_FILENAME ) );
    _pImp->_pEdFileName = pURLBox;
    _pImp->_pEdFileName->Show();
    pURLBox->SetSelectHdl( LINK( this, SvtFileDialog, EntrySelectHdl_Impl ) );
    pURLBox->SetOpenHdl( STATIC_LINK( this, SvtFileDialog, OpenHdl_Impl ) );

    // in folder picker mode, only auto-complete directories (no files)
    bool bIsFolderPicker = ( _pImp->_eDlgType == FILEDLG_TYPE_PATHDLG );
    pURLBox->SetOnlyDirectories( bIsFolderPicker );

    // in save mode, don't use the autocompletion as selection in the edit part
    bool bSaveMode = ( FILEDLG_MODE_SAVE == _pImp->_eMode );
    pURLBox->SetNoURLSelection( bSaveMode );

    _pImp->_pEdFileName->SetHelpId( HID_FILEDLG_AUTOCOMPLETEBOX );

    _pImp->_pFtFileType = new FixedText( this, SvtResId( FT_EXPLORERFILE_FILETYPE ) );
    _pImp->CreateFilterListControl( this, SvtResId( LB_EXPLORERFILE_FILETYPE ) );

    // move the filter listbox to the space occupied by the version listbox
    // if that box isn't needed
    if ( !( _nExtraBits & SFX_EXTRA_SHOWVERSIONS ) &&
         !( _nExtraBits & SFX_EXTRA_TEMPLATES ) &&
         !( _nExtraBits & SFX_EXTRA_IMAGE_TEMPLATE ) )
    {
        {
            FixedText aSharedListBoxLabel( this, SvtResId( FT_EXPLORERFILE_SHARED_LISTBOX ) );
            _pImp->_pFtFileType->SetPosPixel( aSharedListBoxLabel.GetPosPixel() );
        }

        {
            ListBox aSharedListBox( this, SvtResId( LB_EXPLORERFILE_SHARED_LISTBOX ) );
            _pImp->GetFilterListControl()->SetPosPixel( aSharedListBox.GetPosPixel() );
        }
    }

    _pImp->_pEdCurrentPath = new SvtURLBox( this, SvtResId(ED_EXPLORERFILE_CURRENTPATH) );
    _pImp->_pEdCurrentPath->Show();

    _pImp->_pBtnFileOpen = new PushButton( this, SvtResId( BTN_EXPLORERFILE_OPEN ) );
    _pImp->_pBtnCancel = new CancelButton( this, SvtResId( BTN_EXPLORERFILE_CANCEL ) );
    _pImp->_pBtnHelp = new HelpButton( this, SvtResId( BTN_EXPLORERFILE_HELP ) );

    _pImp->_pBtnConnectToServer = new PushButton ( this, SvtResId ( BTN_EXPLORERFILE_CONNECT_TO_SERVER ) );
    _pImp->_pBtnConnectToServer->SetAccessibleName( _pImp->_pBtnConnectToServer->GetQuickHelpText() );

    _pImp->_pBtnUp = new SvtUpButton_Impl( this, SvtResId( BTN_EXPLORERFILE_UP ) );
    _pImp->_pBtnNewFolder = new ImageButton( this, SvtResId( BTN_EXPLORERFILE_NEWFOLDER ) );
    _pImp->_pBtnNewFolder->SetStyle( _pImp->_pBtnNewFolder->GetStyle() | WB_NOPOINTERFOCUS );

    _pImp->_pBtnUp->SetAccessibleName( _pImp->_pBtnUp->GetQuickHelpText() );
    _pImp->_pBtnNewFolder->SetAccessibleName( _pImp->_pBtnNewFolder->GetQuickHelpText() );

    if ( ( nStyle & SFXWB_MULTISELECTION ) == SFXWB_MULTISELECTION )
        _pImp->_bMultiSelection = true;

    _pFileView = new SvtFileView( this, SvtResId( CTL_EXPLORERFILE_FILELIST ),
                                       FILEDLG_TYPE_PATHDLG == _pImp->_eDlgType,
                                       _pImp->_bMultiSelection );
    _pFileView->EnableAutoResize();

    _pFileView->SetHelpId( HID_FILEDLG_STANDARD );
    _pFileView->SetStyle( _pFileView->GetStyle() | WB_TABSTOP );

    _pSplitter = new Splitter( this, SvtResId( EXPLORERFILE_SPLITTER ) );
    _pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor() ));
    _pSplitter->SetSplitHdl( LINK( this, SvtFileDialog, Split_Hdl ) );

    // determine the size of the buttons
    Size aSize = _pImp->_pBtnNewFolder->GetSizePixel();
    Image aNewFolderImg( GetButtonImage( IMG_FILEDLG_CREATEFOLDER ) );
    _pImp->_pBtnNewFolder->SetModeImage( aNewFolderImg );

    // set position of the buttons
    Size aDlgSize = GetOutputSizePixel();
    long n6AppFontInPixel =
            LogicToPixel( Size( 6, 0 ), MAP_APPFONT ).Width();
    long n3AppFontInPixel =
            LogicToPixel( Size( 3, 0 ), MAP_APPFONT ).Width();
    long nHalf3AppFontInPixel = n3AppFontInPixel/2;

    // nDelta is the space between the right border and the left border of the
    // component currently positioned
    long nDelta = n6AppFontInPixel;

    // New folder
    Point aPos(
        aDlgSize.Width() - nDelta,
        _pImp->_pBtnNewFolder->GetPosPixel().Y()
    );
    nDelta += aSize.Width() + nHalf3AppFontInPixel;
    aPos.X() = aDlgSize.Width() - nDelta;
    _pImp->_pBtnNewFolder->SetPosPixel(aPos);

    // Previous level (up)
    nDelta += aSize.Width() + nHalf3AppFontInPixel;
    aPos.X() = aDlgSize.Width() - nDelta;
    _pImp->_pBtnUp->SetPosPixel(aPos);

    // Connect to server ("...")
    nDelta += _pImp->_pBtnConnectToServer->GetSizePixel().Width() + nHalf3AppFontInPixel;
    aPos.X() = aDlgSize.Width() - nDelta;
    _pImp->_pBtnConnectToServer->SetPosPixel(aPos);

    // Set the size of the URL bar
    nDelta += nHalf3AppFontInPixel; // right margin of the URL bar
    aSize.Width() = aDlgSize.Width()
        - _pImp->_pEdCurrentPath->GetPosPixel().X()
        - nDelta;
    _pImp->_pEdCurrentPath->SetOutputSizePixel(aSize);

    aPos.X() = _pImp->_pEdCurrentPath->GetPosPixel().X();
    _pImp->_pEdCurrentPath->SetPosPixel(aPos);

    if ( nStyle & SFXWB_READONLY )
    {
        _pCbReadOnly = new CheckBox( this, SvtResId( CB_EXPLORERFILE_READONLY ) );
        _pCbReadOnly->SetHelpId( HID_FILEOPEN_READONLY );
        _pCbReadOnly->SetText( SvtResId( STR_SVT_FILEPICKER_READONLY ) );
        AddControl( _pCbReadOnly );
        ReleaseOwnership( _pCbReadOnly );
        _pCbReadOnly->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
    }

    if ( nStyle & SFXWB_PASSWORD )
    {
        _pImp->_pCbPassword = new CheckBox( this, SvtResId( CB_EXPLORERFILE_PASSWORD ) );
        _pImp->_pCbPassword->SetText( SvtResId( STR_SVT_FILEPICKER_PASSWORD ) );
        AddControl( _pImp->_pCbPassword );
        ReleaseOwnership( _pImp->_pCbPassword );
        _pImp->_pCbPassword->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
    }

    // set the ini file for extracting the size
    _pImp->_aIniKey = IODLG_CONFIGNAME;

    AddControls_Impl( );

    // Determine the amount of pixel the other elements have to be adjusted in their position.
    aPos.Y() += aSize.Height();
    aPos.Y() += LogicToPixel( Size( 0, 6 ), MAP_APPFONT ).Height();
    long nYOffset = aPos.Y();
    aPos = _pFileView->GetPosPixel();

    aPos.Y() = nYOffset;
    nYOffset -= aPos.Y();

    // Adjust the position of the other elements.
    _pFileView->SetPosPixel( aPos );

    aPos.X() = _pSplitter->GetPosPixel().X();
    _pSplitter->SetPosPixel( aPos );

    aPos.X() = _pImp->_pPlaces->GetPosPixel().X();
    _pImp->_pPlaces->SetPosPixel( aPos );


    lcl_MoveControl( _pImp->_pFtFileName, 0, nYOffset );
    lcl_MoveControl( _pImp->_pEdFileName, 0, nYOffset );

    lcl_MoveControl( _pImp->_pFtFileVersion, 0, nYOffset );
    lcl_MoveControl( _pImp->_pLbFileVersion, 0, nYOffset );

    lcl_MoveControl( _pImp->_pFtTemplates, 0, nYOffset );
    lcl_MoveControl( _pImp->_pLbTemplates, 0, nYOffset );

    lcl_MoveControl( _pImp->_pFtImageTemplates, 0, nYOffset );
    lcl_MoveControl( _pImp->_pLbImageTemplates, 0, nYOffset );

    lcl_MoveControl( _pImp->_pFtFileType, 0, nYOffset );
    lcl_MoveControl( _pImp->GetFilterListControl(), 0, nYOffset );

    lcl_MoveControl( _pImp->_pBtnFileOpen, 0, nYOffset );
    lcl_MoveControl( _pImp->_pBtnCancel, 0, nYOffset );

    lcl_MoveControl( _pImp->_pBtnHelp, 0, nYOffset + 3 );
        // a little more spacing between Cancel- and HelpButton

    // adjust size of the dialog
    aSize = GetSizePixel();
    aSize.Height() += nYOffset;
    SetSizePixel( aSize );

    // adjust the labels to the mode
    sal_uInt16 nResId = STR_EXPLORERFILE_OPEN;
    sal_uInt16 nButtonResId = 0;

    if ( nStyle & WB_SAVEAS )
    {
        nResId = STR_EXPLORERFILE_SAVE;
        nButtonResId = STR_EXPLORERFILE_BUTTONSAVE;
    }

    if ( ( nStyle & SFXWB_PATHDIALOG ) == SFXWB_PATHDIALOG )
    {
        _pImp->_pFtFileName->SetText( SvtResId( STR_PATHNAME ) );
        nResId = STR_PATHSELECT;
        nButtonResId = STR_BUTTONSELECT;
    }

    SetText( SvtResId( nResId ) );

    if ( nButtonResId )
        _pImp->_pBtnFileOpen->SetText( SvtResId( nButtonResId ) );

    if ( FILEDLG_TYPE_FILEDLG != _pImp->_eDlgType )
    {
        _pImp->_pFtFileType->Hide();
        _pImp->GetFilterListControl()->Hide();
    }

    // Setting preferences of the control elements.
    _pImp->_pBtnNewFolder->SetClickHdl( STATIC_LINK( this, SvtFileDialog, NewFolderHdl_Impl ) );
    _pImp->_pBtnFileOpen->SetClickHdl( STATIC_LINK( this, SvtFileDialog, OpenHdl_Impl ) );
    _pImp->_pBtnCancel->SetClickHdl( LINK( this, SvtFileDialog, CancelHdl_Impl ) );
    _pImp->SetFilterListSelectHdl( STATIC_LINK( this, SvtFileDialog, FilterSelectHdl_Impl ) );
    _pImp->_pEdFileName->SetGetFocusHdl( STATIC_LINK( this, SvtFileDialog, FileNameGetFocusHdl_Impl ) );
    _pImp->_pEdFileName->SetModifyHdl( STATIC_LINK( this, SvtFileDialog, FileNameModifiedHdl_Impl ) );
    _pImp->_pEdCurrentPath->SetOpenHdl ( STATIC_LINK( this, SvtFileDialog, URLBoxModifiedHdl_Impl ) );
    _pImp->_pBtnConnectToServer->SetClickHdl( STATIC_LINK ( this, SvtFileDialog, ConnectToServerPressed_Hdl ) );


    _pFileView->SetSelectHdl( LINK( this, SvtFileDialog, SelectHdl_Impl ) );
    _pFileView->SetDoubleClickHdl( LINK( this, SvtFileDialog, DblClickHdl_Impl ) );
    _pFileView->SetOpenDoneHdl( LINK( this, SvtFileDialog, OpenDoneHdl_Impl ) );

    FreeResource();

    // set timer for the filterbox travel
    _pImp->_aFilterTimer.SetTimeout( TRAVELFILTER_TIMEOUT );
    _pImp->_aFilterTimer.SetTimeoutHdl( STATIC_LINK( this, SvtFileDialog, FilterSelectHdl_Impl ) );

    if ( WB_SAVEAS & nStyle )
    {
        // different help ids if in save-as mode
        SetHelpId( HID_FILESAVE_DIALOG );

        _pImp->_pEdFileName->SetHelpId( HID_FILESAVE_FILEURL );
        _pImp->_pBtnFileOpen->SetHelpId( HID_FILESAVE_DOSAVE );
        _pImp->_pBtnNewFolder->SetHelpId( HID_FILESAVE_CREATEDIRECTORY );
        _pImp->_pBtnUp->SetHelpId( HID_FILESAVE_LEVELUP );
        _pImp->GetFilterListControl()->SetHelpId( HID_FILESAVE_FILETYPE );
        _pFileView->SetHelpId( HID_FILESAVE_FILEVIEW );

        // formerly, there was only _pLbFileVersion, which was used for 3 different
        // use cases. For reasons of maintainability, I introduced extra members (_pLbTemplates, _pLbImageTemplates)
        // for the extra use cases, and separated _pLbFileVersion
        // I did not find out in which cases the help ID is really needed HID_FILESAVE_TEMPLATE - all
        // tests I made lead to a dialog where _no_ of the three list boxes was present.
        if ( _pImp->_pLbFileVersion )
            _pImp->_pLbFileVersion->SetHelpId( HID_FILESAVE_TEMPLATE );
        if ( _pImp->_pLbTemplates )
            _pImp->_pLbTemplates->SetHelpId( HID_FILESAVE_TEMPLATE );
        if ( _pImp->_pLbImageTemplates )
            _pImp->_pLbImageTemplates->SetHelpId( HID_FILESAVE_TEMPLATE );

        if ( _pImp->_pCbPassword ) _pImp->_pCbPassword->SetHelpId( HID_FILESAVE_SAVEWITHPASSWORD );
        if ( _pImp->_pCbAutoExtension ) _pImp->_pCbAutoExtension->SetHelpId( HID_FILESAVE_AUTOEXTENSION );
        if ( _pImp->_pCbOptions ) _pImp->_pCbOptions->SetHelpId( HID_FILESAVE_CUSTOMIZEFILTER );
        if ( _pCbSelection ) _pCbSelection->SetHelpId( HID_FILESAVE_SELECTION );
    }

    // correct the z-order of the controls
    implArrangeControls();

    /// read our settings from the configuration
    m_aConfiguration = OConfigurationTreeRoot::createWithComponentContext(
        ::comphelper::getProcessComponentContext(),
        OUString( "/org.openoffice.Office.UI/FilePicker" )
    );
}



IMPL_STATIC_LINK( SvtFileDialog, NewFolderHdl_Impl, PushButton*, EMPTYARG )
{
    pThis->_pFileView->EndInplaceEditing( false );

    SmartContent aContent( pThis->_pFileView->GetViewURL( ) );
    OUString aTitle;
    aContent.getTitle( aTitle );
    QueryFolderNameDialog aDlg( pThis, aTitle, SVT_RESSTR(STR_SVT_NEW_FOLDER) );
    bool bHandled = false;

    while ( !bHandled )
    {
        if ( aDlg.Execute() == RET_OK )
        {
            OUString aUrl = aContent.createFolder( aDlg.GetName( ) );
            if ( !aUrl.isEmpty( ) )
            {
                pThis->_pFileView->CreatedFolder( aUrl, aDlg.GetName() );
                bHandled = true;
            }
        }
        else
            bHandled = true;
    }

    return 0;
}



IMPL_STATIC_LINK_NOINSTANCE( SvtFileDialog, ViewHdl_Impl, ImageButton*, EMPTYARG )
{
    return 0;
}


bool SvtFileDialog::createNewUserFilter( const OUString& _rNewFilter, bool _bAllowUserDefExt )
{
    // delete the old user filter and create a new one
    DELETEZ( _pImp->_pUserFilter );
    _pImp->_pUserFilter = new SvtFileDialogFilter_Impl( _rNewFilter, _rNewFilter );

    // remember the extension
    bool bIsAllFiles = _rNewFilter == FILEDIALOG_FILTER_ALL;
    if ( bIsAllFiles )
        EraseDefaultExt();
    else
        SetDefaultExt( _rNewFilter.copy( 2 ) );
        // TODO: this is nonsense. In the whole file there are a lotta places where we assume that a user filter
        // is always "*.<something>". But changing this would take some more time than I have now ...

    // now, the default extension is set to the one of the user filter (or empty)
    // if the former is not allowed (_bAllowUserDefExt = <FALSE/>), we have to use the ext of the current filter
    // (if possible)
    bool bUseCurFilterExt = true;
    OUString sUserFilter = _pImp->_pUserFilter->GetType();
    sal_Int32 nSepPos = sUserFilter.lastIndexOf( '.' );
    if ( nSepPos != -1 )
    {
        OUString sUserExt = sUserFilter.copy( nSepPos + 1 );
        if  (   ( -1 == sUserExt.indexOf( '*' ) )
            &&  ( -1 == sUserExt.indexOf( '?' ) )
            )
            bUseCurFilterExt = false;
    }

    if ( !_bAllowUserDefExt || bUseCurFilterExt )
    {
        if ( _pImp->GetCurFilter( ) )
            SetDefaultExt( _pImp->GetCurFilter( )->GetExtension() );
        else
            EraseDefaultExt();
    }

    // outta here
    return bIsAllFiles;
}


#define FLT_NONEMPTY        0x0001
#define FLT_CHANGED         0x0002
#define FLT_USERFILTER      0x0004
#define FLT_ALLFILESFILTER  0x0008


sal_uInt16 SvtFileDialog::adjustFilter( const OUString& _rFilter )
{
    sal_uInt16 nReturn = 0;

    const bool bNonEmpty = !_rFilter.isEmpty();
    if ( bNonEmpty )
    {
        nReturn |= FLT_NONEMPTY;

        bool bFilterChanged = true;

        // search for a corresponding filter
        SvtFileDialogFilter_Impl* pFilter = FindFilter_Impl( _rFilter, false, bFilterChanged );

#ifdef AUTOSELECT_USERFILTER
        // if we found a filter which without allowing multi-extensions -> select it
        if ( pFilter )
        {
            _pImp->SelectFilterListEntry( pFilter->GetName() );
            _pImp->SetCurFilter( pFilter );
        }
#endif // AUTOSELECT_USERFILTER

        // look for multi-ext filters if necessary
        if ( !pFilter )
            pFilter = FindFilter_Impl( _rFilter, true, bFilterChanged );

        if ( bFilterChanged )
            nReturn |= FLT_CHANGED;

        if ( !pFilter )
        {
            nReturn |= FLT_USERFILTER;
            // no filter found : use it as user defined filter
#ifdef AUTOSELECT_USERFILTER
            if ( createNewUserFilter( _rFilter, sal_True ) )
#else
            if ( createNewUserFilter( _rFilter, false ) )
#endif
            {   // it's the "all files" filter
                nReturn |= FLT_ALLFILESFILTER;

#ifdef AUTOSELECT_USERFILTER
                // select the "all files" entry
                OUString sAllFilesFilter( SvtResId( STR_FILTERNAME_ALL ) );
                if ( _pImp->HasFilterListEntry( sAllFilesFilter ) )
                {
                    _pImp->SelectFilterListEntry( sAllFilesFilter );
                    _pImp->SetCurFilter( _pImp->GetSelectedFilterEntry( sAllFilesFilter ) );
                }
                else
                    _pImp->SetNoFilterListSelection( ); // there is no "all files" entry
#endif // AUTOSELECT_USERFILTER
            }
#ifdef AUTOSELECT_USERFILTER
            else
                _pImp->SetNoFilterListSelection( );
#endif // AUTOSELECT_USERFILTER
        }
    }

    return nReturn;
}


IMPL_LINK_NOARG(SvtFileDialog, CancelHdl_Impl)
{
    if ( m_pCurrentAsyncAction.is() )
    {
        m_pCurrentAsyncAction->cancel();
        onAsyncOperationFinished();
    }
    else
    {
        EndDialog( sal_False );
    }
    return 1L;
}


IMPL_STATIC_LINK( SvtFileDialog, OpenHdl_Impl, void*, pVoid )
{
    if ( pThis->_pImp->_bMultiSelection && pThis->_pFileView->GetSelectionCount() > 1 )
    {
        // special open in case of multiselection
        pThis->OpenMultiSelection_Impl();
        return 0;
    }

    OUString aFileName;
    OUString aOldPath( pThis->_pFileView->GetViewURL() );
    if ( pThis->_pImp->_bDoubleClick || pThis->_pFileView->HasChildPathFocus() )
        // Selection done by doubleclicking in the view, get filename from the view
        aFileName = pThis->_pFileView->GetCurrentURL();

    if ( aFileName.isEmpty() )
    {
        // if an entry is selected in the view ....
        if ( pThis->_pFileView->GetSelectionCount() )
        {   // -> use this one. This will allow us to step down this folder
            aFileName = pThis->_pFileView->GetCurrentURL();
        }
    }

    if ( aFileName.isEmpty() )
    {
        if ( pThis->_pImp->_eMode == FILEDLG_MODE_OPEN && pThis->_pImp->_pEdFileName->IsTravelSelect() )
            // OpenHdl called from URLBox; travelling through the list of URLs should not cause an opening
            return 0;                   // MBA->PB: seems to be called never ?!

        // get the URL from from the edit field ( if not empty )
        if ( !pThis->_pImp->_pEdFileName->GetText().isEmpty() )
        {
            OUString aText = pThis->_pImp->_pEdFileName->GetText();

            // did we reach the root?
            if ( !INetURLObject( aOldPath ).getSegmentCount() )
            {
                if ( ( aText.getLength() == 2 && aText == ".." ) ||
                     ( aText.getLength() == 3 && ( aText == "..\\" || aText == "../" ) ) )
                    // don't go higher than the root
                    return 0;
            }

#if defined( UNX )
            if ( ( 1 == aText.getLength() ) && ( '~' == aText[0] ) )
            {
                // go to the home directory
                if ( lcl_getHomeDirectory( pThis->_pFileView->GetViewURL(), aFileName ) )
                    // in case we got a home dir, reset the text of the edit
                    pThis->_pImp->_pEdFileName->SetText( OUString() );
            }
            if ( aFileName.isEmpty() )
#endif
            {
                // get url from autocomplete edit
                aFileName = pThis->_pImp->_pEdFileName->GetURL();
            }
        }
        else if ( pVoid == pThis->_pImp->_pBtnFileOpen )
            // OpenHdl was called for the "Open" Button; if edit field is empty, use selected element in the view
            aFileName = pThis->_pFileView->GetCurrentURL();
    }

    // MBA->PB: ?!
    if ( aFileName.isEmpty() && pVoid == pThis->_pImp->_pEdFileName && pThis->_pImp->_pUserFilter )
    {
        DELETEZ( pThis->_pImp->_pUserFilter );
        return 0;
    }

    sal_Int32 nLen = aFileName.getLength();
    if ( !nLen )
    {
        // if the dialog was opened to select a folder, the last selected folder should be selected
        if( pThis->_pImp->_eDlgType == FILEDLG_TYPE_PATHDLG )
        {
            aFileName = pThis->_pImp->_pEdCurrentPath->GetText();
            nLen = aFileName.getLength();
        }
        else
            // no file selected !
            return 0;
    }

    // mark input as selected
    pThis->_pImp->_pEdFileName->SetSelection( Selection( 0, nLen ) );

    // if a path with wildcards is given, divide the string into path and wildcards
    OUString aFilter;
    if ( !pThis->IsolateFilterFromPath_Impl( aFileName, aFilter ) )
        return 0;

    // if a filter was retrieved, there were wildcards !
    sal_uInt16 nNewFilterFlags = pThis->adjustFilter( aFilter );
    if ( nNewFilterFlags & FLT_CHANGED )
    {
        // cut off all text before wildcard in edit and select wildcard
        pThis->_pImp->_pEdFileName->SetText( aFilter );
        pThis->_pImp->_pEdFileName->SetSelection( Selection( 0, aFilter.getLength() ) );
    }

    {
        INetURLObject aFileObject( aFileName );
        if ( ( aFileObject.GetProtocol() == INET_PROT_NOT_VALID ) && !aFileName.isEmpty() )
        {
            OUString sCompleted = SvtURLBox::ParseSmart( aFileName, pThis->_pFileView->GetViewURL(), SvtPathOptions().GetWorkPath() );
            if ( !sCompleted.isEmpty() )
                aFileName = sCompleted;
        }
    }

    // check if it is a folder
    bool bIsFolder = false;

    // first thing before doing anyhing with the content: Reset it. When the user presses "open" (or "save" or "export",
    // for that matter), s/he wants the complete handling, including all possible error messages, even if s/he
    // does the same thing for the same content twice, s/he wants both fails to be displayed.
    // Without the reset, it could be that the content cached all relevant information, and will not display any
    // error messages for the same content a second time ....
    pThis->m_aContent.bindTo( OUString( ) );

    if ( !aFileName.isEmpty() )
    {
        // Make sure we have own Interaction Handler in place. We do not need
        // to intercept interactions here, but to record the fact that there
        // was an interaction.
        SmartContent::InteractionHandlerType eInterActionHandlerType
            = pThis->m_aContent.queryCurrentInteractionHandler();
        if ( ( eInterActionHandlerType == SmartContent::IHT_NONE ) ||
             ( eInterActionHandlerType == SmartContent::IHT_DEFAULT ) )
            pThis->m_aContent.enableOwnInteractionHandler(
                OFilePickerInteractionHandler::E_NOINTERCEPTION );

        bIsFolder = pThis->m_aContent.isFolder( aFileName );

        // access denied to the given resource - and interaction was already
        // used => break following operations
        OFilePickerInteractionHandler* pHandler
            = pThis->m_aContent.getOwnInteractionHandler();

        OSL_ENSURE( pHandler, "Got no Interaction Handler!!!" );

        if ( pHandler->wasAccessDenied() )
            return 0;

        if ( pThis->m_aContent.isInvalid() &&
             ( pThis->_pImp->_eMode == FILEDLG_MODE_OPEN ) )
        {
            if ( !pHandler->wasUsed() )
                ErrorHandler::HandleError( ERRCODE_IO_NOTEXISTS );

            return 0;
        }

        // restore previous Interaction Handler
        if ( eInterActionHandlerType == SmartContent::IHT_NONE )
            pThis->m_aContent.disableInteractionHandler();
        else if ( eInterActionHandlerType == SmartContent::IHT_DEFAULT )
            pThis->m_aContent.enableDefaultInteractionHandler();
     }

    if  (   !bIsFolder                                      // no existent folder
        &&  pThis->_pImp->_pCbAutoExtension                 // auto extension is enabled in general
        &&  pThis->_pImp->_pCbAutoExtension->IsChecked()    // auto extension is really to be used
        &&  !pThis->GetDefaultExt().isEmpty()               // there is a default extension
        &&  !comphelper::string::equals(pThis->GetDefaultExt(), '*') // the default extension is not "all"
        && !(   FILEDLG_MODE_SAVE == pThis->_pImp->_eMode       // we're saving a file
            &&  pThis->_pFileView->GetSelectionCount()          // there is a selected file in the file view -> it will later on
            )                                                   //    (in SvtFileDialog::GetPathList) be taken as file to save to

        && FILEDLG_MODE_OPEN != pThis->_pImp->_eMode // #i83408# don't append extension on open
        )
    {
        // check extension and append the default extension if necessary
        appendDefaultExtension(aFileName,
                               pThis->GetDefaultExt(),
                               pThis->_pImp->GetCurFilter()->GetType());
    }

    bool bOpenFolder = ( FILEDLG_TYPE_PATHDLG == pThis->_pImp->_eDlgType ) &&
                       !pThis->_pImp->_bDoubleClick && pVoid != pThis->_pImp->_pEdFileName;
    if ( bIsFolder )
    {
        if ( bOpenFolder )
        {
            pThis->_aPath = aFileName;
        }
        else
        {
            if ( aFileName != pThis->_pFileView->GetViewURL() )
            {
                pThis->OpenURL_Impl( aFileName );
            }
            else
            {
                if ( nNewFilterFlags & FLT_CHANGED )
                    pThis->ExecuteFilter();
            }

            return 0;
        }
    }
    else if ( !( nNewFilterFlags & FLT_NONEMPTY ) )
    {
        // if applicable save URL
        pThis->_aPath = aFileName;
    }
    else
    {
        // if applicable filter again
        if ( nNewFilterFlags & FLT_CHANGED )
            pThis->ExecuteFilter();
        return 0;
    }

    INetURLObject aFileObj( aFileName );
    if ( aFileObj.HasError() )
    {
        ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
        return 0;
    }

    switch ( pThis->_pImp->_eMode )
    {
        case FILEDLG_MODE_SAVE:
        {
            if ( ::utl::UCBContentHelper::Exists( aFileObj.GetMainURL( INetURLObject::NO_DECODE ) ) )
            {
                OUString aMsg = SVT_RESSTR( STR_SVT_ALREADYEXISTOVERWRITE );
                aMsg = aMsg.replaceFirst(
                    "$filename$",
                    aFileObj.getName(INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET)
                );
                QueryBox aBox( pThis, WB_YES_NO, aMsg );
                if ( aBox.Execute() != RET_YES )
                    return 0;
            }
            else
            {
                OUString aCurPath;
                if ( ::utl::LocalFileHelper::ConvertURLToSystemPath( aFileName, aCurPath ) )
                {
                    // if content does not exist: at least its path must exist
                    INetURLObject aPathObj = aFileObj;
                    aPathObj.removeSegment();
                    bool bFolder = pThis->m_aContent.isFolder( aPathObj.GetMainURL( INetURLObject::NO_DECODE ) );
                    if ( !bFolder )
                    {
                        ErrorHandler::HandleError( ERRCODE_IO_NOTEXISTSPATH );
                        return 0;
                    }
                }
            }
        }
        break;

        case FILEDLG_MODE_OPEN:
        {
            // do an existence check herein, again

            if ( INET_PROT_FILE == aFileObj.GetProtocol( ) )
            {
                bool bExists = false;
                bExists = pThis->m_aContent.is( aFileObj.GetMainURL( INetURLObject::NO_DECODE ) );


                if ( !bExists )
                {
                    OUString sError( SVT_RESSTR( RID_FILEOPEN_NOTEXISTENTFILE ) );

                    OUString sInvalidFile( aFileObj.GetMainURL( INetURLObject::DECODE_TO_IURI ) );
                    if ( INET_PROT_FILE == aFileObj.GetProtocol() )
                    {   // if it's a file URL, transform the URL into system notation
                        OUString sURL( sInvalidFile );
                        OUString sSystem;
                        osl_getSystemPathFromFileURL( sURL.pData, &sSystem.pData );
                        sInvalidFile = sSystem;
                    }
                    sError = sError.replaceFirst( "$name$", sInvalidFile );

                    ErrorBox aError( pThis, WB_OK, sError );
                    aError.Execute();
                    return 0;
                }
            }
        }
        break;

        default:
            OSL_FAIL("SvtFileDialog, OpenHdl_Impl: invalid mode!");
    }

    // notify interested parties
    long nRet;

    if ( pThis->_aOKHdl.IsSet() )
        nRet = pThis->_aOKHdl.Call( pThis );
    else
        nRet = pThis->OK();

    if ( nRet )
    {
        pThis->EndDialog( sal_True );
    }

    return nRet;
}



void SvtFileDialog::EnableAutocompletion( bool _bEnable )
{
    _pImp->_pEdFileName->EnableAutocompletion( _bEnable );
}



IMPL_STATIC_LINK( SvtFileDialog, FilterSelectHdl_Impl, ListBox*, pBox )
{
    DBG_ASSERT( pBox, "SvtFileDialog:keine Instanz" );

    // was the handler executed by the travel timer?
    if ( pBox == (ListBox*)&pThis->_pImp->_aFilterTimer )
    {
        // filter the view again
        pThis->ExecuteFilter();
        return 0;
    }

    OUString sSelectedFilterDisplayName;
    SvtFileDialogFilter_Impl* pSelectedFilter = pThis->_pImp->GetSelectedFilterEntry( sSelectedFilterDisplayName );
    if ( !pSelectedFilter )
    {   // there is no current selection. This happens if for instance the user selects a group separator using
        // the keyboard, and then presses enter: When the selection happens, we immediately deselect the entry,
        // so in this situation there is no current selection.
        if ( restoreCurrentFilter( pThis->_pImp ) )
            pThis->ExecuteFilter();
    }
    else
    {
        if ( pSelectedFilter->isGroupSeparator() )
        {   // group separators can't be selected
            // return to the previously selected entry
            if ( pThis->_pImp->IsFilterListTravelSelect() )
            {
                pThis->_pImp->SetNoFilterListSelection( );

                // stop the timer for executing the filter
                if ( pThis->_pImp->_aFilterTimer.IsActive() )
                    pThis->_pImp->m_bNeedDelayedFilterExecute = true;
                pThis->_pImp->_aFilterTimer.Stop();
            }
            else
            {
                if ( restoreCurrentFilter( pThis->_pImp ) )
                    pThis->ExecuteFilter();
            }
        }
        else if (   ( pSelectedFilter != pThis->_pImp->GetCurFilter() )
                ||  pThis->_pImp->_pUserFilter
                )
        {
            // Store the old filter for the auto extension handling
            OUString sLastFilterExt = pThis->_pImp->GetCurFilter()->GetExtension();
            DELETEZ( pThis->_pImp->_pUserFilter );

            // if applicable remove filter of the user
            pThis->_pImp->SetCurFilter( pSelectedFilter, sSelectedFilterDisplayName );

            // if applicable show extension
            pThis->SetDefaultExt( pSelectedFilter->GetExtension() );
            sal_Int32 nSepPos = pThis->GetDefaultExt().indexOf( FILEDIALOG_DEF_EXTSEP );

            if ( nSepPos != -1 )
                pThis->EraseDefaultExt( nSepPos );

            // update the extension of the current file if necessary
            lcl_autoUpdateFileExtension( pThis, sLastFilterExt );

            // if the user is traveling fast through the filterbox
            // do not filter instantly
            if ( pThis->_pImp->IsFilterListTravelSelect() )
            {
                // FilterSelectHdl_Impl should be started again in
                // TRAVELFILTER_TIMEOUT ms
                pThis->_pImp->_aFilterTimer.Start();
            }
            else
            {
                // stop previously started timer
                pThis->_pImp->_aFilterTimer.Stop();

                // filter the view again
                pThis->ExecuteFilter();
            }
        }
    }

    return 0;
}



IMPL_STATIC_LINK( SvtFileDialog, FileNameGetFocusHdl_Impl, void*, EMPTYARG )
{
    pThis->_pFileView->SetNoSelection();
    pThis->_pFileView->Update();
    return 0;
}



IMPL_STATIC_LINK( SvtFileDialog, FileNameModifiedHdl_Impl, void*, EMPTYARG )
{
    FileNameGetFocusHdl_Impl( pThis, NULL );
    return 0;
}



IMPL_STATIC_LINK ( SvtFileDialog, URLBoxModifiedHdl_Impl, void*, EMPTYARG )
{
    OUString _aPath = pThis->_pImp->_pEdCurrentPath->GetURL();
    pThis->OpenURL_Impl(_aPath);
    return 0;
}



IMPL_STATIC_LINK ( SvtFileDialog, ConnectToServerPressed_Hdl, void*, EMPTYARG )
{
    pThis->_pFileView->EndInplaceEditing( false );

    PlaceEditDialog aDlg( pThis );
    short aRetCode = aDlg.Execute();

    switch (aRetCode) {
        case RET_OK :
        {
            PlacePtr newPlace = aDlg.GetPlace();
            pThis->_pImp->_pPlaces->AppendPlace(newPlace);

      break;
        }
        case RET_CANCEL :
        default :
            // Do Nothing
            break;
    };

    return 0;
}



IMPL_LINK_NOARG ( SvtFileDialog, AddPlacePressed_Hdl )
{
    // Maybe open the PlacesDialog would have been a better idea
    // there is an ux choice to make we did not make...
    INetURLObject aURLObj( _pFileView->GetViewURL() );
    PlacePtr newPlace(
        new Place( aURLObj.GetLastName(INetURLObject::DECODE_WITH_CHARSET),
                OUString(_pFileView->GetViewURL()), true));
    _pImp->_pPlaces->AppendPlace(newPlace);
    return 0;
}



IMPL_LINK_NOARG ( SvtFileDialog, RemovePlacePressed_Hdl )
{
    _pImp->_pPlaces->RemoveSelectedPlace();
    return 0;
}



SvtFileDialogFilter_Impl* SvtFileDialog::FindFilter_Impl
(
    const OUString& _rFilter,
    bool _bMultiExt,/*  TRUE - regard filter with several extensions
                            FALSE - do not ...
                        */
    bool& _rFilterChanged
)

/*  [Description]

    This method looks for the specified extension in the included filters.
*/

{
    SvtFileDialogFilter_Impl* pFoundFilter = NULL;
    SvtFileDialogFilterList_Impl* pList = _pImp->_pFilter;
    sal_uInt16 nFilter = pList->size();

    while ( nFilter-- )
    {
        SvtFileDialogFilter_Impl* pFilter = &(*pList)[ nFilter ];
        const OUString& rType = pFilter->GetType();
        OUString aSingleType = rType;

        if ( _bMultiExt )
        {
            sal_Int32 nIdx = 0;
            while ( !pFoundFilter && nIdx != -1 )
            {
                aSingleType = rType.getToken( 0, FILEDIALOG_DEF_EXTSEP, nIdx );
#ifdef UNX
                if ( aSingleType == _rFilter )
#else
                if ( aSingleType.equalsIgnoreAsciiCase( _rFilter ) )
#endif
                    pFoundFilter = pFilter;
            }
        }
#ifdef UNX
        else if ( rType == _rFilter )
#else
        else if ( rType.equalsIgnoreAsciiCase( _rFilter ) )
#endif
            pFoundFilter = pFilter;

        if ( pFoundFilter )
        {
            // activate filter
            _rFilterChanged = _pImp->_pUserFilter || ( _pImp->GetCurFilter() != pFilter );

            createNewUserFilter( _rFilter, false );

            break;
        }
    }
    return pFoundFilter;
}



void SvtFileDialog::ExecuteFilter()
{
    _pImp->m_bNeedDelayedFilterExecute = false;
    executeAsync( AsyncPickerAction::eExecuteFilter, OUString(), getMostCurrentFilter( _pImp ) );
}



void SvtFileDialog::OpenMultiSelection_Impl()

/*  [Description]

    OpenHandler for MultiSelection
*/

{
    sal_uLong nCount = _pFileView->GetSelectionCount();
    SvTreeListEntry* pEntry = nCount ? _pFileView->FirstSelected() : NULL;

    if ( nCount && pEntry )
        _aPath = _pFileView->GetURL( pEntry );

    // notify interested parties
    long nRet;

    if ( _aOKHdl.IsSet() )
        nRet = _aOKHdl.Call( this );
    else
        nRet = OK();

    if ( nRet )
        EndDialog( sal_True );
}



void SvtFileDialog::UpdateControls( const OUString& rURL )
{
       _pImp->_pEdFileName->SetBaseURL( rURL );

    INetURLObject aObj( rURL );


    {
        OUString sText;
        DBG_ASSERT( INET_PROT_NOT_VALID != aObj.GetProtocol(), "SvtFileDialog::UpdateControls: Invalid URL!" );

        if ( aObj.getSegmentCount() )
        {
            ::utl::LocalFileHelper::ConvertURLToSystemPath( rURL, sText );
            if ( !sText.isEmpty() )
            {
                // no Fsys path for server file system ( only UCB has mountpoints! )
                if ( INET_PROT_FILE != aObj.GetProtocol() )
                    sText = rURL.copy( INetURLObject::GetScheme( aObj.GetProtocol() ).getLength() );
            }

            if ( sText.isEmpty() && aObj.getSegmentCount() )
                sText = rURL;
        }

        // path mode ?
        if ( FILEDLG_TYPE_PATHDLG == _pImp->_eDlgType )
            // -> set new path in the edit field
            _pImp->_pEdFileName->SetText( sText );

        // in the "current path" field, truncate the trailing slash
        if ( aObj.hasFinalSlash() )
        {
            aObj.removeFinalSlash();
            OUString sURL( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
            if ( !::utl::LocalFileHelper::ConvertURLToSystemPath( sURL, sText ) )
                sText = sURL;
        }

        if ( sText.isEmpty() && !rURL.isEmpty() )
            // happens, for instance, for URLs which the INetURLObject does not know to belong to a hierarchical scheme
            sText = rURL;
        _pImp->_pEdCurrentPath->SetText( sText );
    }


    _aPath = rURL;
    if ( _pFileNotifier )
        _pFileNotifier->notify( DIRECTORY_CHANGED, 0 );
}



IMPL_LINK( SvtFileDialog, SelectHdl_Impl, SvTabListBox*, pBox )
{
    SvTreeListEntry* pEntry = pBox->FirstSelected();
    DBG_ASSERT( pEntry, "SelectHandler without selected entry" );
    SvtContentEntry* pUserData = (SvtContentEntry*)pEntry->GetUserData();

    if ( pUserData )
    {
        INetURLObject aObj( pUserData->maURL );
        if ( FILEDLG_TYPE_PATHDLG == _pImp->_eDlgType )
        {
            if ( aObj.GetProtocol() == INET_PROT_FILE )
            {
                if ( !pUserData->mbIsFolder )
                    aObj.removeSegment();
                OUString aName = aObj.getFSysPath( (INetURLObject::FSysStyle)(INetURLObject::FSYS_DETECT & ~INetURLObject::FSYS_VOS) );
                _pImp->_pEdFileName->SetText( aName );
                _pImp->_pEdFileName->SetSelection( Selection( 0, aName.getLength() ) );
                _aPath = pUserData->maURL;
            }
            else if ( !pUserData->mbIsFolder )
            {
                _pImp->_pEdFileName->SetText( pUserData->maURL );
                _pImp->_pEdFileName->SetSelection( Selection( 0, pUserData->maURL.getLength() ) );
                _aPath = pUserData->maURL;
            }
            else
                _pImp->_pEdFileName->SetText( OUString() );
        }
        else
        {
            if ( !pUserData->mbIsFolder )
            {
                OUString aName = pBox->GetEntryText( pEntry, 0 );
                _pImp->_pEdFileName->SetText( aName );
                _pImp->_pEdFileName->SetSelection( Selection( 0, aName.getLength() ) );
                _aPath = pUserData->maURL;
            }
        }
    }

    if ( _pImp->_bMultiSelection && _pFileView->GetSelectionCount() > 1 )
    {
        // clear the file edit for multiselection
        _pImp->_pEdFileName->SetText( OUString() );
    }

    FileSelect();

    return 0;
}



IMPL_LINK_NOARG(SvtFileDialog, DblClickHdl_Impl)
{
    _pImp->_bDoubleClick = true;
    OpenHdl_Impl( this, NULL );
    _pImp->_bDoubleClick = false;

    return 0;
}



IMPL_LINK_NOARG(SvtFileDialog, EntrySelectHdl_Impl)
{
    FileSelect();

    return 0;
}



IMPL_LINK( SvtFileDialog, OpenDoneHdl_Impl, SvtFileView*, pView )
{
    OUString sCurrentFolder( pView->GetViewURL() );
    // check if we can create new folders
    EnableControl( _pImp->_pBtnNewFolder, ContentCanMakeFolder( sCurrentFolder ) );

    // check if we can travel one level up
    bool bCanTravelUp = ContentHasParentFolder( pView->GetViewURL() );
    if ( bCanTravelUp )
    {
        // additional check: the parent folder should not be prohibited
        INetURLObject aCurrentFolder( sCurrentFolder );
        DBG_ASSERT( INET_PROT_NOT_VALID != aCurrentFolder.GetProtocol(),
            "SvtFileDialog::OpenDoneHdl_Impl: invalid current URL!" );

        aCurrentFolder.removeSegment();
    }
    EnableControl( _pImp->_pBtnUp, bCanTravelUp );

    return 0;
}



IMPL_LINK_NOARG(SvtFileDialog, AutoExtensionHdl_Impl)
{
    if ( _pFileNotifier )
        _pFileNotifier->notify( CTRL_STATE_CHANGED,
                                CHECKBOX_AUTOEXTENSION );

    // update the extension of the current file if necessary
    lcl_autoUpdateFileExtension( this, _pImp->GetCurFilter()->GetExtension() );

    return 0;
}



IMPL_LINK( SvtFileDialog, ClickHdl_Impl, CheckBox*, pCheckBox )
{
    if ( ! _pFileNotifier )
        return 0;

    sal_Int16 nId = -1;

    if ( pCheckBox == _pImp->_pCbOptions )
        nId = CHECKBOX_FILTEROPTIONS;
    else if ( pCheckBox == _pCbSelection )
        nId = CHECKBOX_SELECTION;
    else if ( pCheckBox == _pCbReadOnly )
        nId = CHECKBOX_READONLY;
    else if ( pCheckBox == _pImp->_pCbPassword )
        nId = CHECKBOX_PASSWORD;
    else if ( pCheckBox == _pCbLinkBox )
        nId = CHECKBOX_LINK;
    else if ( pCheckBox == _pCbPreviewBox )
        nId = CHECKBOX_PREVIEW;

    if ( nId != -1 )
        _pFileNotifier->notify( CTRL_STATE_CHANGED, nId );

    return 0;
}



IMPL_LINK_NOARG(SvtFileDialog, PlayButtonHdl_Impl)
{
    if ( _pFileNotifier )
        _pFileNotifier->notify( CTRL_STATE_CHANGED,
                                PUSHBUTTON_PLAY );

    return 0;
}



bool SvtFileDialog::Notify( NotifyEvent& rNEvt )

/*  [Description]

    This method gets called to catch <BACKSPACE>.
*/

{
    sal_uInt16 nType = rNEvt.GetType();
    bool nRet = false;

    if ( EVENT_KEYINPUT == nType && rNEvt.GetKeyEvent() )
    {
        const KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nCode = rKeyCode.GetCode();

        if ( !rKeyCode.GetModifier() &&
             KEY_BACKSPACE == nCode && !_pImp->_pEdFileName->HasChildPathFocus() )
        {
            nRet = false;

            if ( !nRet && _pImp->_pBtnUp->IsEnabled() )
            {
                PrevLevel_Impl();
                nRet = true;
            }
        }
    }
    return nRet || ModalDialog::Notify( rNEvt );
}



long SvtFileDialog::OK()
{
    return sal_True;
}



class SvtDefModalDialogParent_Impl
{
private:
    Window* _pOld;

public:
    SvtDefModalDialogParent_Impl( Window *pNew ) :
        _pOld( Application::GetDefDialogParent() )
        { Application::SetDefDialogParent( pNew ); }

    ~SvtDefModalDialogParent_Impl() { Application::SetDefDialogParent( _pOld ); }
};




void SvtFileDialog::updateListboxLabelSizes()
{
    sal_Int16 nLineControlId[5] = {
        LISTBOX_VERSION, LISTBOX_TEMPLATE, LISTBOX_IMAGE_TEMPLATE, LISTBOX_FILTER, EDIT_FILEURL
    };

    // determine the maximum width needed for the listbox labels
    long nMaxWidth = 0;
    for ( sal_Int32 i=0; i<5; ++i )
    {
        FixedText* pLabel = static_cast< FixedText* >( getControl( nLineControlId[i], sal_True ) );
        if ( !pLabel )
            continue;
        nMaxWidth = ::std::max( pLabel->GetTextWidth( pLabel->GetText() ), nMaxWidth );
    }

    // ensure that all labels are wide enough
    for ( sal_Int32 i=0; i<5; ++i )
    {
        FixedText* pLabel = static_cast< FixedText* >( getControl( nLineControlId[i], sal_True ) );
        ListBox* pListbox = static_cast< ListBox* >( getControl( nLineControlId[i], sal_False ) );
        if ( !pLabel || !pListbox )
            continue;
        Size aCurrentSize( pLabel->GetSizePixel() );
        if ( aCurrentSize.Width() >= nMaxWidth )
            continue;

        long nChange = nMaxWidth - aCurrentSize.Width();
        pLabel->SetSizePixel( Size( nMaxWidth, aCurrentSize.Height() ) );

        aCurrentSize = pListbox->GetSizePixel();
        pListbox->SetSizePixel( Size( aCurrentSize.Width() - nChange, aCurrentSize.Height() ) );
        lcl_MoveControl( pListbox, nChange, 0 );
    }
}

namespace
{

bool implIsInvalid( const OUString & rURL )
{
    SmartContent aContent( rURL );
    aContent.enableOwnInteractionHandler( ::svt::OFilePickerInteractionHandler::E_DOESNOTEXIST );
    aContent.isFolder();    // do this _before_ asking isInvalid! Otherwise result might be wrong.
    return aContent.isInvalid();
}

}


OUString SvtFileDialog::implGetInitialURL( const OUString& _rPath, const OUString& _rFallback )
{
    // an URL parser for the fallback
    INetURLObject aURLParser;

    // set the path
    bool bWasAbsolute = false;
    aURLParser = aURLParser.smartRel2Abs( _rPath, bWasAbsolute );

    // is it a valid folder?
    m_aContent.bindTo( aURLParser.GetMainURL( INetURLObject::NO_DECODE ) );
    bool bIsFolder = m_aContent.isFolder( );    // do this _before_ asking isInvalid!
    bool bIsInvalid = m_aContent.isInvalid();

    if ( bIsInvalid && m_bHasFilename && !aURLParser.hasFinalSlash() )
    {   // check if the parent folder exists
        INetURLObject aParent( aURLParser );
        aParent.removeSegment( );
        aParent.setFinalSlash( );
        bIsInvalid = implIsInvalid( aParent.GetMainURL( INetURLObject::NO_DECODE ) );
    }

    if ( bIsInvalid )
    {
        INetURLObject aFallback( _rFallback );
        bIsInvalid = implIsInvalid( aFallback.GetMainURL( INetURLObject::NO_DECODE ) );

        if ( !bIsInvalid )
            aURLParser = aFallback;
    }

    if ( bIsInvalid )
    {
        INetURLObject aParent( aURLParser );
        while ( bIsInvalid && aParent.removeSegment() )
        {
            aParent.setFinalSlash( );
            bIsInvalid = implIsInvalid( aParent.GetMainURL( INetURLObject::NO_DECODE ) );
        }

        if ( !bIsInvalid )
            aURLParser = aParent;
    }

    if ( !bIsInvalid && bIsFolder )
    {
        aURLParser.setFinalSlash();
    }
    return aURLParser.GetMainURL( INetURLObject::NO_DECODE );
}


short SvtFileDialog::Execute()
{
    if ( !PrepareExecute() )
        return 0;

    // start the dialog
    _bIsInExecute = true;
    short nResult = ModalDialog::Execute();
    _bIsInExecute = false;

    DBG_ASSERT( !m_pCurrentAsyncAction.is(), "SvtFilePicker::Execute: still running an async action!" );
        // the dialog should not be cancellable while an async action is running - first, the action
        // needs to be cancelled

    // remember last directory
    if ( RET_OK == nResult )
    {
        INetURLObject aURL( _aPath );
        if ( aURL.GetProtocol() == INET_PROT_FILE )
        {
            // remember the selected directory only for file URLs not for virtual folders
            sal_Int32 nLevel = aURL.getSegmentCount();
            bool bDir = m_aContent.isFolder( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
            if ( nLevel > 1 && ( FILEDLG_TYPE_FILEDLG == _pImp->_eDlgType || !bDir ) )
                aURL.removeSegment();
        }
    }

    return nResult;
}


void SvtFileDialog::StartExecuteModal( const Link& rEndDialogHdl )
{
    PrepareExecute();

    // start of the dialog
    ModalDialog::StartExecuteModal( rEndDialogHdl );
}


void SvtFileDialog::onAsyncOperationStarted()
{
    EnableUI( false );
    // the cancel button must be always enabled
    _pImp->_pBtnCancel->Enable( true );
    _pImp->_pBtnCancel->GrabFocus();
}


void SvtFileDialog::onAsyncOperationFinished()
{
    EnableUI( true );
    m_pCurrentAsyncAction = NULL;
    if ( !m_bInExecuteAsync )
        _pImp->_pEdFileName->GrabFocus();
        // (if m_bInExecuteAsync is true, then the operation was finished within the minium wait time,
        // and to the user, the operation appears to be synchronous)
}


void SvtFileDialog::RemovablePlaceSelected(bool enable)
{
    _pImp->_pPlaces->SetDelEnabled( enable );
}


void SvtFileDialog::displayIOException( const OUString& _rURL, IOErrorCode _eCode )
{
    try
    {
        // create make a human-readable string from the URL
        OUString sDisplayPath( _rURL );
        ::utl::LocalFileHelper::ConvertURLToSystemPath( _rURL, sDisplayPath );

        // build an own exception which tells "access denied"
        InteractiveAugmentedIOException aException;
        aException.Arguments.realloc( 2 );
        aException.Arguments[ 0 ] <<= sDisplayPath;
        aException.Arguments[ 1 ] <<= PropertyValue(
            OUString( "Uri" ),
            -1, aException.Arguments[ 0 ], PropertyState_DIRECT_VALUE
        );
            // (formerly, it was sufficient to put the URL first parameter. Nowadays,
            // the services expects the URL in a PropertyValue named "Uri" ...)
        aException.Code = _eCode;
        aException.Classification = InteractionClassification_ERROR;

        // let and interaction handler handle this exception
        ::comphelper::OInteractionRequest* pRequest = NULL;
        Reference< ::com::sun::star::task::XInteractionRequest > xRequest = pRequest =
            new ::comphelper::OInteractionRequest( makeAny( aException ) );
        pRequest->addContinuation( new ::comphelper::OInteractionAbort( ) );

        Reference< XInteractionHandler2 > xHandler(
            InteractionHandler::createWithParent( ::comphelper::getProcessComponentContext(), 0 ) );
        xHandler->handle( xRequest );
    }
    catch( const Exception& )
    {
        OSL_FAIL( "iodlg::displayIOException: caught an exception!" );
    }
}


void SvtFileDialog::EnableUI( bool _bEnable )
{
    Enable( _bEnable );

    if ( _bEnable )
    {
        for ( ::std::set< Control* >::iterator aLoop = m_aDisabledControls.begin();
              aLoop != m_aDisabledControls.end();
              ++aLoop
            )
        {
            (*aLoop)->Enable( false );
        }
    }
}


void SvtFileDialog::EnableControl( Control* _pControl, bool _bEnable )
{
    if ( !_pControl )
    {
        SAL_WARN( "fpicker.office", "SvtFileDialog::EnableControl: invalid control!" );
        return;
    }

    _pControl->Enable( _bEnable );

    if ( _bEnable )
    {
        ::std::set< Control* >::iterator aPos = m_aDisabledControls.find( _pControl );
        if ( m_aDisabledControls.end() != aPos )
            m_aDisabledControls.erase( aPos );
    }
    else
        m_aDisabledControls.insert( _pControl );
}



short SvtFileDialog::PrepareExecute()
{
    OUString aEnvValue;
    if ( getEnvironmentValue( "WorkDirMustContainRemovableMedia", aEnvValue ) && aEnvValue == "1" )
    {
        try
        {
            INetURLObject aStdDir( GetStandardDir() );
            ::ucbhelper::Content aCnt( OUString( aStdDir.GetMainURL(
                                                    INetURLObject::NO_DECODE ) ),
                                 Reference< XCommandEnvironment >(),
                                 comphelper::getProcessComponentContext() );
            Sequence< OUString > aProps(2);
            aProps[0] = "IsVolume";
            aProps[1] = "IsRemoveable";

            Reference< XResultSet > xResultSet
                = aCnt.createCursor( aProps, ::ucbhelper::INCLUDE_FOLDERS_ONLY );
            if ( xResultSet.is() )
            {
                Reference< XRow > xRow( xResultSet, UNO_QUERY );

                bool bEmpty = true;
                if ( !xResultSet->next() )
                {
                    // folder is empty
                    bEmpty = true;
                }
                else
                {
                                bEmpty = false;
                }

                if ( bEmpty )
                {
                    ErrorBox aBox( this, WB_OK, SVT_RESSTR( STR_SVT_NOREMOVABLEDEVICE ) );
                    aBox.Execute();
                    return 0;
                }
            }
        }
        catch ( ContentCreationException const & )
        {
        }
        catch ( CommandAbortedException const & )
        {
        }
    }

    if ( ( _pImp->_nStyle & WB_SAVEAS ) && m_bHasFilename )
        // when doing a save-as, we do not want the handler to handle "this file does not exist" messages
        // - finally we're going to save that file, aren't we?
        m_aContent.enableOwnInteractionHandler(::svt::OFilePickerInteractionHandler::E_DOESNOTEXIST);
    else
        m_aContent.enableDefaultInteractionHandler();

    // possibly just a filename without a path
    OUString aFileNameOnly;
    if( !_aPath.isEmpty() && (_pImp->_eMode == FILEDLG_MODE_SAVE)
                     && (_aPath.indexOf(':') == -1)
                     && (_aPath.indexOf('\\') == -1)
                     && (_aPath.indexOf('/') == -1))
    {
        aFileNameOnly = _aPath;
        _aPath = "";
    }

    // no starting path specified?
    if ( _aPath.isEmpty() )
    {
        // then use the standard directory
        _aPath = lcl_ensureFinalSlash( _pImp->GetStandardDir() );

        // attach given filename to path
        if ( !aFileNameOnly.isEmpty() )
            _aPath += aFileNameOnly;
    }


    _aPath = implGetInitialURL( _aPath, GetStandardDir() );

    if ( _pImp->_nStyle & WB_SAVEAS && !m_bHasFilename )
        // when doing a save-as, we do not want the handler to handle "this file does not exist" messages
        // - finally we're going to save that file, aren't we?
        m_aContent.enableOwnInteractionHandler(::svt::OFilePickerInteractionHandler::E_DOESNOTEXIST);

    // if applicable show filter
    _pImp->InitFilterList();

    // set up initial filter
    sal_uInt16 nFilterCount = GetFilterCount();
    OUString aAll = SvtResId( STR_FILTERNAME_ALL ).toString();
    bool bHasAll = _pImp->HasFilterListEntry( aAll );
    if ( _pImp->GetCurFilter() || nFilterCount == 1 || ( nFilterCount == 2 && bHasAll ) )
    {
        // if applicable set the only filter or the only filter that
        // does not refer to all files, as the current one
        if ( !_pImp->GetCurFilter() )
        {
            sal_uInt16 nPos = 0;
            if ( 2 == nFilterCount && bHasAll )
            {
                nPos = nFilterCount;
                while ( nPos-- )
                {
                    if ( aAll != GetFilterName( nPos ) )
                        break;
                }
            }
            SvtFileDialogFilter_Impl* pNewCurFilter = &(*_pImp->_pFilter)[ nPos ];
            DBG_ASSERT( pNewCurFilter, "SvtFileDialog::Execute: invalid filter pos!" );
            _pImp->SetCurFilter( pNewCurFilter, pNewCurFilter->GetName() );
        }

        // adjust view
        _pImp->SelectFilterListEntry( _pImp->GetCurFilter()->GetName() );
        SetDefaultExt( _pImp->GetCurFilter()->GetExtension() );
        sal_Int32 nSepPos = GetDefaultExt().indexOf( FILEDIALOG_DEF_EXTSEP );
        if ( nSepPos != -1 )
            EraseDefaultExt( nSepPos );
    }
    else
    {
        // if applicable set respectively create filter for all files
        if ( !bHasAll )
        {
            SvtFileDialogFilter_Impl* pAllFilter = implAddFilter( aAll, OUString(FILEDIALOG_FILTER_ALL) );
            _pImp->InsertFilterListEntry( pAllFilter );
            _pImp->SetCurFilter( pAllFilter, aAll );
        }
        _pImp->SelectFilterListEntry( aAll );
    }

    _pImp->_pDefaultFilter = _pImp->GetCurFilter();

    // if applicable isolate filter
    OUString aFilter;

    if ( !IsolateFilterFromPath_Impl( _aPath, aFilter ) )
        return 0;

    sal_uInt16 nNewFilterFlags = adjustFilter( aFilter );
    if ( nNewFilterFlags & ( FLT_NONEMPTY | FLT_USERFILTER ) )
    {
        _pImp->_pEdFileName->SetText( aFilter );
    }

    // create and show instance for set path
    INetURLObject aFolderURL( _aPath );
    OUString aFileName( aFolderURL.getName( INetURLObject::LAST_SEGMENT, false ) );
    sal_Int32 nFileNameLen = aFileName.getLength();
    bool bFileToSelect = nFileNameLen != 0;
    if ( bFileToSelect && aFileName[ nFileNameLen - 1 ] != '/' )
    {
        _pImp->_pEdFileName->SetText( GET_DECODED_NAME( aFolderURL ) );
        aFolderURL.removeSegment();
    }

    INetURLObject aObj = aFolderURL;
    if ( aObj.GetProtocol() == INET_PROT_FILE )
    {
        // set folder as current directory
        aObj.setFinalSlash();
    }

    UpdateControls( aObj.GetMainURL( INetURLObject::NO_DECODE ) );

    // Somebody might want to enable some controls according to the current filter
    FilterSelect();

    ViewHdl_Impl( this, NULL );
    OpenURL_Impl( aObj.GetMainURL( INetURLObject::NO_DECODE ) );

    _pFileView->Show();
    _pSplitter->Show();
    SvtDefModalDialogParent_Impl aDefParent( this );

    // if applicable read and set size from ini
    InitSize();

    return 1;
}


void SvtFileDialog::executeAsync( ::svt::AsyncPickerAction::Action _eAction,
                                    const OUString& _rURL, const OUString& _rFilter )
{
    DBG_ASSERT( !m_pCurrentAsyncAction.is(), "SvtFileDialog::executeAsync: previous async action not yet finished!" );

    m_pCurrentAsyncAction = new AsyncPickerAction( this, _pFileView, _eAction );

    bool bReallyAsync = true;
    m_aConfiguration.getNodeValue( OUString( "FillAsynchronously" ) ) >>= bReallyAsync;

    sal_Int32 nMinTimeout = 0;
    m_aConfiguration.getNodeValue( OUString( "Timeout/Min" ) ) >>= nMinTimeout;
    sal_Int32 nMaxTimeout = 0;
    m_aConfiguration.getNodeValue( OUString( "Timeout/Max" ) ) >>= nMaxTimeout;

    m_bInExecuteAsync = true;
    m_pCurrentAsyncAction->execute( _rURL, _rFilter, bReallyAsync ? nMinTimeout : -1, nMaxTimeout, GetBlackList() );
    m_bInExecuteAsync = false;
}



void SvtFileDialog::FileSelect()
{
    if ( _pFileNotifier )
        _pFileNotifier->notify( FILE_SELECTION_CHANGED, 0 );
}



void SvtFileDialog::FilterSelect()
{
    if ( _pFileNotifier )
        _pFileNotifier->notify( CTRL_STATE_CHANGED,
                                LISTBOX_FILTER );
}



void SvtFileDialog::SetStandardDir( const OUString& rStdDir )

/*  [Description]

   This method sets the path for the default button.
*/

{
    INetURLObject aObj( rStdDir );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid protocol!" );
    aObj.setFinalSlash();
    _pImp->SetStandardDir( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
}

void SvtFileDialog::SetBlackList( const ::com::sun::star::uno::Sequence< OUString >& rBlackList )
{
    _pImp->SetBlackList( rBlackList );
}



const ::com::sun::star::uno::Sequence< OUString >& SvtFileDialog::GetBlackList() const
{
    return _pImp->GetBlackList();
}


const OUString& SvtFileDialog::GetStandardDir() const

/*  [Description]

    This method returns the standard path.
*/

{
    return _pImp->GetStandardDir();
}



void SvtFileDialog::PrevLevel_Impl()
{
    _pFileView->EndInplaceEditing( false );

    OUString sDummy;
    executeAsync( AsyncPickerAction::ePrevLevel, sDummy, sDummy );
}



void SvtFileDialog::OpenURL_Impl( const OUString& _rURL )
{
    _pFileView->EndInplaceEditing( false );

    executeAsync( AsyncPickerAction::eOpenURL, _rURL, getMostCurrentFilter( _pImp ) );
}


SvtFileDialogFilter_Impl* SvtFileDialog::implAddFilter( const OUString& _rFilter, const OUString& _rType )
{
    SvtFileDialogFilter_Impl* pNewFilter = new SvtFileDialogFilter_Impl( _rFilter, _rType );
    _pImp->_pFilter->push_front( pNewFilter );

    if ( !_pImp->GetCurFilter() )
        _pImp->SetCurFilter( pNewFilter, _rFilter );

    return pNewFilter;
}



void SvtFileDialog::AddFilter( const OUString& _rFilter, const OUString& _rType )
{
    DBG_ASSERT( !IsInExecute(), "SvtFileDialog::AddFilter: currently executing!" );
    implAddFilter ( _rFilter, _rType );
}


void SvtFileDialog::AddFilterGroup( const OUString& _rFilter, const Sequence< StringPair >& _rFilters )
{
    DBG_ASSERT( !IsInExecute(), "SvtFileDialog::AddFilter: currently executing!" );

    implAddFilter( _rFilter, OUString() );
    const StringPair* pSubFilters       =               _rFilters.getConstArray();
    const StringPair* pSubFiltersEnd    = pSubFilters + _rFilters.getLength();
    for ( ; pSubFilters != pSubFiltersEnd; ++pSubFilters )
        implAddFilter( pSubFilters->First, pSubFilters->Second );
}


void SvtFileDialog::SetCurFilter( const OUString& rFilter )
{
    DBG_ASSERT( !IsInExecute(), "SvtFileDialog::SetCurFilter: currently executing!" );

    // look for corresponding filter
    sal_uInt16 nPos = _pImp->_pFilter->size();

    while ( nPos-- )
    {
        SvtFileDialogFilter_Impl* pFilter = &(*_pImp->_pFilter)[ nPos ];
        if ( pFilter->GetName() == rFilter )
        {
            _pImp->SetCurFilter( pFilter, rFilter );
            break;
        }
    }
}



OUString SvtFileDialog::GetCurFilter() const
{
    OUString aFilter;

    const SvtFileDialogFilter_Impl* pCurrentFilter = _pImp->GetCurFilter();
    if ( pCurrentFilter )
        aFilter = pCurrentFilter->GetName();

    return aFilter;
}

OUString SvtFileDialog::getCurFilter( ) const
{
    return GetCurFilter();
}



sal_uInt16 SvtFileDialog::GetFilterCount() const
{
    return _pImp->_pFilter->size();
}



const OUString& SvtFileDialog::GetFilterName( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < GetFilterCount(), "invalid index" );
    return (*_pImp->_pFilter)[ nPos ].GetName();
}



void SvtFileDialog::InitSize()
{
    if ( _pImp->_aIniKey.isEmpty() )
        return;

    Size aDlgSize = GetResizeOutputSizePixel();
    SetMinOutputSizePixel( aDlgSize );

    if ( !_pImp->_nFixDeltaHeight )
    {
        // calculate and save fixsize
        long nBoxH = _pFileView->GetSizePixel().Height();
        long nH = GetSizePixel().Height();
        _pImp->_nFixDeltaHeight = nH - nBoxH;
    }

    // initialize from config
    SvtViewOptions aDlgOpt( E_DIALOG, _pImp->_aIniKey );

    if ( aDlgOpt.Exists() )
    {
        SetWindowState(OUStringToOString(aDlgOpt.GetWindowState(), osl_getThreadTextEncoding()));

        Any aUserData = aDlgOpt.GetUserItem( OUString( "UserData" ));
        OUString sCfgStr;
        if ( aUserData >>= sCfgStr )
            _pFileView->SetConfigString( sCfgStr );
    }
}



std::vector<OUString> SvtFileDialog::GetPathList() const
{
    std::vector<OUString> aList;
    sal_uLong           nCount = _pFileView->GetSelectionCount();
    SvTreeListEntry*    pEntry = nCount ? _pFileView->FirstSelected() : NULL;

    if ( ! pEntry )
    {
        if ( !_pImp->_pEdFileName->GetText().isEmpty() && _bIsInExecute )
            aList.push_back(_pImp->_pEdFileName->GetURL());
        else
            aList.push_back(_aPath);
    }
    else
    {
        while ( pEntry )
        {
            aList.push_back(_pFileView->GetURL(pEntry));
            pEntry = _pFileView->NextSelected( pEntry );
        }
    }

    return aList;
}



void SvtFileDialog::implArrangeControls()
{
    // this is the list of controls in the order they should be tabbed
    // from topleft to bottomright
    // pb: #136070# new order so all LabeledBy relations are correct now
    Control* pControls[] =
    {
        _pImp->_pEdCurrentPath, _pImp->_pBtnConnectToServer,
        _pImp->_pBtnUp, _pImp->_pBtnNewFolder,                              // image buttons
        _pImp->_pPlaces,                                                    // list of places
        _pFileView,                                                         // the file view
        _pImp->_pFtFileName, _pImp->_pEdFileName,
        _pImp->_pFtFileVersion, _pImp->_pLbFileVersion,
        _pImp->_pFtTemplates, _pImp->_pLbTemplates,
        _pImp->_pFtImageTemplates, _pImp->_pLbImageTemplates,
        _pImp->_pFtFileType, _pImp->GetFilterListControl(),                 // edit fields/list boxes
        _pImp->_pCbPassword, _pImp->_pCbAutoExtension, _pImp->_pCbOptions,  // checkboxes
        _pCbReadOnly, _pCbLinkBox, _pCbPreviewBox, _pCbSelection, _pPbPlay, // check boxes (continued)
        _pImp->_pBtnFileOpen, _pImp->_pBtnCancel, _pImp->_pBtnHelp          // buttons

        // (including the FixedTexts is important - not for tabbing order (they're irrelevant there),
        // but for working keyboard shortcuts)
    };

    // loop through all these controls and adjust the z-order
    Window* pPreviousWin = NULL;
    Control** pCurrent = pControls;
    for ( sal_Int32 i = 0; i < sal_Int32(sizeof( pControls ) / sizeof( pControls[ 0 ] )); ++i, ++pCurrent )
    {
        if ( !*pCurrent )
            // this control is not available in the current operation mode -> skip
            continue;

        if ( pPreviousWin )
            (*pCurrent)->SetZOrder( pPreviousWin, WINDOW_ZORDER_BEHIND );
        else
            (*pCurrent)->SetZOrder( NULL, WINDOW_ZORDER_FIRST );

        pPreviousWin = *pCurrent;
    }

    // FileName edit not the first control but it should have the focus initially
    _pImp->_pEdFileName->GrabFocus();
}



bool SvtFileDialog::IsolateFilterFromPath_Impl( OUString& rPath, OUString& rFilter )
{
    OUString aEmpty;
    OUString aReversePath = comphelper::string::reverseString(rPath);
    sal_Int32 nQuestionMarkPos = rPath.indexOf( '?' );
    sal_Int32 nWildCardPos = rPath.indexOf( FILEDIALOG_DEF_WILDCARD );

    if ( nQuestionMarkPos != -1 )
    {
        // use question mark as wildcard only for files
        INetProtocol eProt = INetURLObject::CompareProtocolScheme( rPath );

        if ( INET_PROT_NOT_VALID != eProt && INET_PROT_FILE != eProt )
            nQuestionMarkPos = -1;

        nWildCardPos = std::min( nWildCardPos, nQuestionMarkPos );
    }

    rFilter = aEmpty;

    if ( nWildCardPos != -1 )
    {
        sal_Int32 nPathTokenPos = aReversePath.indexOf( '/' );

        if ( nPathTokenPos == -1 )
        {
            OUString aDelim(
#if defined(WNT)
                    '\\'
#else
                    '/'
#endif
            );

            nPathTokenPos = aReversePath.indexOf( aDelim );
#if !defined( UNX )
            if ( nPathTokenPos == -1 )
            {
                nPathTokenPos = aReversePath.indexOf( ':' );
            }
#endif
        }

        // check syntax
        if ( nPathTokenPos != -1 )
        {
            if ( nPathTokenPos < (rPath.getLength() - nWildCardPos - 1) )
            {
                ErrorHandler::HandleError( ERRCODE_SFX_INVALIDSYNTAX );
                return false;
            }

            // cut off filter
            rFilter = aReversePath;
            rFilter = rFilter.copy( 0, nPathTokenPos );
            rFilter = comphelper::string::reverseString(rFilter);

            // determine folder
            rPath = aReversePath;
            rPath = rPath.copy( nPathTokenPos );
            rPath = comphelper::string::reverseString(rPath);
        }
        else
        {
            rFilter = rPath;
            rPath = aEmpty;
        }
    }

    return true;
}


void SvtFileDialog::implUpdateImages( )
{
    m_aImages = ImageList( SvtResId( RID_FILEPICKER_IMAGES ) );

    // set the appropriate images on the buttons
    if ( _pImp->_pBtnUp )
        _pImp->_pBtnUp->SetModeImage( GetButtonImage( IMG_FILEDLG_BTN_UP ) );

    if ( _pImp->_pBtnNewFolder )
        _pImp->_pBtnNewFolder->SetModeImage( GetButtonImage( IMG_FILEDLG_CREATEFOLDER ) );
}


void SvtFileDialog::DataChanged( const DataChangedEvent& _rDCEvt )
{
    if ( DATACHANGED_SETTINGS == _rDCEvt.GetType() )
        implUpdateImages( );

    ModalDialog::DataChanged( _rDCEvt );
}


void SvtFileDialog::Resize()
{
    if ( IsRollUp() )
        return;

    Size aDlgSize = GetResizeOutputSizePixel();
    Size aOldSize = _pImp->_aDlgSize;
    _pImp->_aDlgSize = aDlgSize;
    long nWinDeltaW = 0;

    if ( _pPrevWin &&
         _pPrevWin->GetPosPixel().X() > _pFileView->GetPosPixel().X() )
    {
        nWinDeltaW = _pPrevWin->GetOutputSizePixel().Width() + _pImp->_a6Size.Width();
    }

    Size aNewSize = _pFileView->GetSizePixel();
    Point aBoxPos( _pFileView->GetPosPixel() );
    long nDeltaY = aNewSize.Height();
    long nDeltaX = aNewSize.Width();
    aNewSize.Height() = aDlgSize.Height() - _pImp->_nFixDeltaHeight;
    aNewSize.Width() = aDlgSize.Width() - aBoxPos.X() - _pImp->_a6Size.Width() - nWinDeltaW;
    if ( aOldSize.Height() )
        nDeltaY = _pImp->_aDlgSize.Height() - aOldSize.Height();
    else
        nDeltaY = aNewSize.Height() - nDeltaY;
    nDeltaX = aNewSize.Width() - nDeltaX;

    if ( nWinDeltaW )
        nWinDeltaW = nDeltaX * 2 / 3;
    aNewSize.Width() -= nWinDeltaW;
    nDeltaX -= nWinDeltaW;

    _pFileView->SetSizePixel( aNewSize );

    // Resize the Splitter to fit the height
    Size splitterNewSize = _pSplitter->GetSizePixel( );
    splitterNewSize.Height() += nDeltaY;
    _pSplitter->SetSizePixel( splitterNewSize );
    sal_Int32 nMinX = _pImp->_pPlaces->GetPosPixel( ).X( );
    sal_Int32 nMaxX = _pFileView->GetPosPixel( ).X( ) + _pFileView->GetSizePixel( ).Width() - nMinX;
    _pSplitter->SetDragRectPixel( Rectangle( Point( nMinX, 0 ), Size( nMaxX, aDlgSize.Width() ) ) );

    // Resize the places list box to fit the height of the FileView
    Size placesNewSize(_pImp->_pPlaces->GetSizePixel());
    placesNewSize.Height() += nDeltaY;
    _pImp->_pPlaces->SetSizePixel( placesNewSize );

    if ( !nDeltaY && !nDeltaX )
        // This resize was only called to show or hide the indicator.
        return;


    // move controls

    // controls to move vertically
    {
        Control* aMoveControlsVert[] =
        {
            _pImp->_pFtFileName, _pImp->_pEdFileName, _pImp->_pFtFileVersion, _pImp->_pLbFileVersion,
            _pImp->_pFtTemplates, _pImp->_pLbTemplates, _pImp->_pFtImageTemplates, _pImp->_pLbImageTemplates,
            _pImp->_pFtFileType, _pImp->GetFilterListControl(), _pCbReadOnly, _pCbLinkBox, _pCbPreviewBox,
            _pPbPlay, _pImp->_pCbPassword, _pImp->_pCbAutoExtension, _pImp->_pCbOptions, _pCbSelection
        };
        Control** ppMoveControls = aMoveControlsVert;
        Control** ppMoveControlsEnd = ppMoveControls + sizeof( aMoveControlsVert ) / sizeof( aMoveControlsVert[0] );
        for ( ; ppMoveControls != ppMoveControlsEnd; ++ppMoveControls )
            lcl_MoveControl( *ppMoveControls, 0, nDeltaY );
    }

    // controls to move vertically and horizontally
    {
        Control* aMoveControlsBoth[] =
        {
            _pImp->_pBtnFileOpen, _pImp->_pBtnCancel, _pImp->_pBtnHelp
        };
        Control** ppMoveControls = aMoveControlsBoth;
        Control** ppMoveControlsEnd = ppMoveControls + sizeof( aMoveControlsBoth ) / sizeof( aMoveControlsBoth[0] );
        for ( ; ppMoveControls != ppMoveControlsEnd; ++ppMoveControls )
            lcl_MoveControl( *ppMoveControls, nDeltaX, nDeltaY );
    }

    // controls to move horizontally
    {
        Control* aMoveControlsHor[] =
        {
            _pImp->_pBtnConnectToServer,
            _pImp->_pBtnUp, _pImp->_pBtnNewFolder
        };
        Control** ppMoveControls = aMoveControlsHor;
        Control** ppMoveControlsEnd = ppMoveControls + sizeof( aMoveControlsHor ) / sizeof( aMoveControlsHor[0] );
        for ( ; ppMoveControls != ppMoveControlsEnd; ++ppMoveControls )
            lcl_MoveControl( *ppMoveControls, nDeltaX, 0 );
    }


    // resize controls
    {
        Control* aSizeControls[] =
        {
            _pImp->_pEdFileName, _pImp->_pLbFileVersion, _pImp->_pLbTemplates, _pImp->_pLbImageTemplates,
            _pImp->GetFilterListControl(), _pImp->_pEdCurrentPath,
        };
        sal_Int32 nSizeControls = sizeof( aSizeControls ) / sizeof( aSizeControls[0] );
        Control** ppSizeControls = aSizeControls;
        for ( sal_Int32 j=0; j<nSizeControls; ++j, ++ppSizeControls )
        {
            if ( *ppSizeControls )
            {
                aNewSize = (*ppSizeControls)->GetSizePixel();
                aNewSize.Width() += nDeltaX;
                (*ppSizeControls)->SetSizePixel( aNewSize );
            }
        }
    }

    // align additional controls
    if ( _pPrevWin &&
         _pPrevWin->GetPosPixel().X() > _pFileView->GetPosPixel().X() )
    {
        // special alignment for controls of the type window
        // also adjust the size
        Point aNewPos = _pPrevWin->GetPosPixel();
        aNewPos.X() += nDeltaX;
        _pPrevWin->SetPosPixel( aNewPos );
        _pPrevBmp->SetPosPixel( aNewPos );
        aNewSize = _pPrevWin->GetOutputSizePixel();
        aNewSize.Width() += nWinDeltaW;
        aNewSize.Height() += nDeltaY;
        if ( !aOldSize.Height() )
            aNewSize.Height() -= ( _pImp->_a6Size.Height() / 2 );
        _pPrevWin->SetOutputSizePixel( aNewSize );
        _pPrevBmp->SetOutputSizePixel( aNewSize );
        _pPrevBmp->Invalidate();
    }

    if ( _pFileNotifier )
        _pFileNotifier->notify( DIALOG_SIZE_CHANGED, 0 );
}


Control* SvtFileDialog::getControl( sal_Int16 _nControlId, sal_Bool _bLabelControl ) const
{
    Control* pReturn = NULL;

    switch ( _nControlId )
    {
        case CONTROL_FILEVIEW:
            pReturn = _bLabelControl ? NULL : static_cast< Control* >( _pFileView );
            break;

        case EDIT_FILEURL:
            pReturn =   _bLabelControl
                    ?   static_cast< Control* >( _pImp->_pFtFileName )
                    :   static_cast< Control* >( _pImp->_pEdFileName );
            break;

        case EDIT_FILEURL_LABEL:
            pReturn = static_cast< Control* >( _pImp->_pFtFileName );
            break;

        case CHECKBOX_AUTOEXTENSION:
            pReturn = _pImp->_pCbAutoExtension;
            break;

        case CHECKBOX_PASSWORD:
            pReturn = _pImp->_pCbPassword;
            break;

        case CHECKBOX_FILTEROPTIONS:
            pReturn = _pImp->_pCbOptions;
            break;

        case CHECKBOX_READONLY:
            pReturn = _pCbReadOnly;
            break;

        case CHECKBOX_LINK:
            pReturn = _pCbLinkBox;
            break;

        case CHECKBOX_PREVIEW:
            pReturn = _pCbPreviewBox;
            break;

        case CHECKBOX_SELECTION:
            pReturn = _pCbSelection;
            break;

        case LISTBOX_FILTER:
            pReturn = _bLabelControl ? _pImp->_pFtFileType : _pImp->GetFilterListControl();
            break;

        case LISTBOX_FILTER_LABEL:
            pReturn = _pImp->_pFtFileType;
            break;

        case FIXEDTEXT_CURRENTFOLDER:
            pReturn = _pImp->_pEdCurrentPath;
            break;

        case LISTBOX_VERSION:
            pReturn =   _bLabelControl
                    ?   static_cast< Control* >( _pImp->_pFtFileVersion )
                    :   static_cast< Control* >( _pImp->_pLbFileVersion );
            break;

        case LISTBOX_TEMPLATE:
            pReturn =   _bLabelControl
                    ?   static_cast< Control* >( _pImp->_pFtTemplates )
                    :   static_cast< Control* >( _pImp->_pLbTemplates );
            break;

        case LISTBOX_IMAGE_TEMPLATE:
            pReturn =   _bLabelControl
                    ?   static_cast< Control* >( _pImp->_pFtImageTemplates )
                    :   static_cast< Control* >( _pImp->_pLbImageTemplates );
            break;

        case LISTBOX_VERSION_LABEL:
            pReturn = _pImp->_pFtFileVersion;
            break;

        case LISTBOX_TEMPLATE_LABEL:
            pReturn = _pImp->_pFtTemplates;
            break;

        case LISTBOX_IMAGE_TEMPLATE_LABEL:
            pReturn = _pImp->_pFtImageTemplates;
            break;

        case PUSHBUTTON_OK:
            pReturn = _pImp->_pBtnFileOpen;
            break;

        case PUSHBUTTON_CANCEL:
            pReturn = _pImp->_pBtnCancel;
            break;

        case PUSHBUTTON_PLAY:
            pReturn = _pPbPlay;
            break;

        case PUSHBUTTON_HELP:
            pReturn = _pImp->_pBtnHelp;
            break;

        case TOOLBOXBUTOON_LEVEL_UP:
            pReturn = _pImp->_pBtnUp;
            break;

        case TOOLBOXBUTOON_NEW_FOLDER:
            pReturn = _pImp->_pBtnNewFolder;
            break;

        case LISTBOX_FILTER_SELECTOR:
            // only exists on SalGtkFilePicker
            break;

        default:
            SAL_WARN( "fpicker.office", "SvtFileDialog::getControl: invalid id!" );
    }
    return pReturn;
}


void SvtFileDialog::enableControl( sal_Int16 _nControlId, sal_Bool _bEnable )
{
    Control* pControl = getControl( _nControlId, sal_False );
    if ( pControl )
        EnableControl( pControl, _bEnable );
    Control* pLabel = getControl( _nControlId, sal_True );
    if ( pLabel )
        EnableControl( pLabel, _bEnable );
}


void SvtFileDialog::AddControls_Impl( )
{
    // create the "insert as link" checkbox, if needed
    if ( _nExtraBits & SFX_EXTRA_INSERTASLINK )
    {
        _pCbLinkBox = new CheckBox( this );
        _pCbLinkBox ->SetText( SvtResId( STR_SVT_FILEPICKER_INSERT_AS_LINK ) );
        _pCbLinkBox ->SetHelpId( HID_FILEDLG_LINK_CB );
        AddControl( _pCbLinkBox  );
        ReleaseOwnership( _pCbLinkBox );
        _pCbLinkBox->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
    }

    // create the "show preview" checkbox ( and the preview window, too ), if needed
    if ( _nExtraBits & SFX_EXTRA_SHOWPREVIEW  )
    {
        _pImp->_aIniKey = IMPGRF_CONFIGNAME;
        // because the "<All Formats> (*.bmp,*...)" entry is to wide,
        // we need to disable the auto width feature of the filter box
        _pImp->DisableFilterBoxAutoWidth();

        // "preview"
        _pCbPreviewBox = new CheckBox( this );
        _pCbPreviewBox->SetText( SvtResId( STR_SVT_FILEPICKER_SHOW_PREVIEW ) );
        _pCbPreviewBox->SetHelpId( HID_FILEDLG_PREVIEW_CB );
        AddControl( _pCbPreviewBox );
        ReleaseOwnership( _pCbPreviewBox );
        _pCbPreviewBox->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );

        // generate preview window just here
        _pPrevWin = new Window( this, WinBits( WB_BORDER ) );
        AddControl( _pPrevWin );
        ReleaseOwnership( _pPrevWin );
        _pPrevWin->Hide();

        _pPrevBmp = new FixedBitmap( this, WinBits( WB_BORDER ) );
        _pPrevBmp->SetBackground( Wallpaper( Color( COL_WHITE ) ) );
        _pPrevBmp->Show();
        _pPrevBmp->SetAccessibleName(SVT_RESSTR(STR_PREVIEW));
    }

    if ( _nExtraBits & SFX_EXTRA_AUTOEXTENSION )
    {
        _pImp->_pCbAutoExtension = new CheckBox( this, SvtResId( CB_AUTO_EXTENSION ) );
        _pImp->_pCbAutoExtension->SetText( SvtResId( STR_SVT_FILEPICKER_AUTO_EXTENSION ) );
        _pImp->_pCbAutoExtension->Check( true );
        AddControl( _pImp->_pCbAutoExtension );
        ReleaseOwnership( _pImp->_pCbAutoExtension );
        _pImp->_pCbAutoExtension->SetClickHdl( LINK( this, SvtFileDialog, AutoExtensionHdl_Impl ) );
    }

    if ( _nExtraBits & SFX_EXTRA_FILTEROPTIONS )
    {
        _pImp->_pCbOptions = new CheckBox( this, SvtResId( CB_OPTIONS ) );
        _pImp->_pCbOptions->SetText( SvtResId( STR_SVT_FILEPICKER_FILTER_OPTIONS ) );
        AddControl( _pImp->_pCbOptions );
        ReleaseOwnership( _pImp->_pCbOptions );
        _pImp->_pCbOptions->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
    }

    if ( _nExtraBits & SFX_EXTRA_SELECTION )
    {
        _pCbSelection = new CheckBox( this, SvtResId( CB_OPTIONS ) );
        _pCbSelection->SetText( SvtResId( STR_SVT_FILEPICKER_SELECTION ) );
        AddControl( _pCbSelection );
        ReleaseOwnership( _pCbSelection );
        _pCbSelection->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
    }

    if ( _nExtraBits & SFX_EXTRA_PLAYBUTTON )
    {
        _pPbPlay = new PushButton( this );
        _pPbPlay->SetText( SvtResId( STR_SVT_FILEPICKER_PLAY ) );
        _pPbPlay->SetHelpId( HID_FILESAVE_DOPLAY );
        AddControl( _pPbPlay );
        ReleaseOwnership( _pPbPlay );
        _pPbPlay->SetClickHdl( LINK( this, SvtFileDialog, PlayButtonHdl_Impl ) );
    }

    if ( _nExtraBits & SFX_EXTRA_SHOWVERSIONS )
    {
        _pImp->_pFtFileVersion = new FixedText( this, SvtResId( FT_EXPLORERFILE_SHARED_LISTBOX ) );
        _pImp->_pFtFileVersion->SetText( SvtResId( STR_SVT_FILEPICKER_VERSION ) );

        _pImp->_pLbFileVersion = new ListBox( this, SvtResId( LB_EXPLORERFILE_SHARED_LISTBOX ) );
        _pImp->_pLbFileVersion->SetHelpId( HID_FILEOPEN_VERSION );
    }
    else if ( _nExtraBits & SFX_EXTRA_TEMPLATES )
    {
        _pImp->_pFtTemplates = new FixedText( this, SvtResId( FT_EXPLORERFILE_SHARED_LISTBOX ) );
        _pImp->_pFtTemplates->SetText( SvtResId( STR_SVT_FILEPICKER_TEMPLATES ) );

        _pImp->_pLbTemplates = new ListBox( this, SvtResId( LB_EXPLORERFILE_SHARED_LISTBOX ) );
        _pImp->_pLbTemplates->SetHelpId( HID_FILEOPEN_VERSION );
            // This is strange. During the re-factoring during 96930, I discovered that this help id
            // is set in the "Templates mode". This was hidden in the previous implementation.
            // Shouldn't this be a more meaningfull help id.
    }
    else if ( _nExtraBits & SFX_EXTRA_IMAGE_TEMPLATE )
    {
        _pImp->_pFtImageTemplates = new FixedText( this, SvtResId( FT_EXPLORERFILE_SHARED_LISTBOX ) );
        _pImp->_pFtImageTemplates->SetText( SvtResId( STR_SVT_FILEPICKER_IMAGE_TEMPLATE ) );

        _pImp->_pLbImageTemplates = new ListBox( this, SvtResId( LB_EXPLORERFILE_SHARED_LISTBOX ) );
        _pImp->_pLbImageTemplates->SetHelpId( HID_FILEOPEN_IMAGE_TEMPLATE );
    }

    _pImp->_pPlaces = new PlacesListBox( this, SVT_RESSTR(STR_PLACES_TITLE), SvtResId(LB_EXPLORERFILE_PLACES_LISTBOX) );
    _pImp->_pPlaces->SetAddHdl( LINK ( this, SvtFileDialog, AddPlacePressed_Hdl ) );
    _pImp->_pPlaces->SetDelHdl( LINK ( this, SvtFileDialog, RemovePlacePressed_Hdl ) );

    initDefaultPlaces();
}


sal_Int32 SvtFileDialog::getTargetColorDepth()
{
    if ( _pPrevBmp )
        return _pPrevBmp->GetBitCount();
    else
        return 0;
}


sal_Int32 SvtFileDialog::getAvailableWidth()
{
    if ( _pPrevBmp )
        return _pPrevBmp->GetOutputSizePixel().Width();
    else
        return 0;
}


sal_Int32 SvtFileDialog::getAvailableHeight()
{
    if ( _pPrevBmp )
        return _pPrevBmp->GetOutputSizePixel().Height();
    else
        return 0;
}


void SvtFileDialog::setImage( sal_Int16 /*aImageFormat*/, const Any& rImage )
{
    if ( ! _pPrevBmp || ! _pPrevBmp->IsVisible() )
        return;

    Sequence < sal_Int8 > aBmpSequence;

    if ( rImage >>= aBmpSequence )
    {
        Bitmap          aBmp;
        SvMemoryStream  aData( aBmpSequence.getArray(),
                               aBmpSequence.getLength(),
                               STREAM_READ );
        ReadDIB(aBmp, aData, true);

        _pPrevBmp->SetBitmap( aBmp );
    }
    else
    {
        Bitmap aEmpty;
        _pPrevBmp->SetBitmap( aEmpty );
    }
}


bool SvtFileDialog::setShowState( bool /*bShowState*/ )
{
    // #97633 for the system filedialog it's
    // useful to make the preview switchable
    // because the preview occupies
    // half of the size of the file listbox
    // which is not the case here,
    // so we (TRA/FS) decided not to make
    // the preview window switchable because
    // else we would have to change the layout
    // of the file dialog dynamically
    // support for set/getShowState is opionally
    // see com::sun::star::ui::dialogs::XFilePreview

    return false;
}


OUString SvtFileDialog::getCurrentFileText( ) const
{
    OUString sReturn;
    if ( _pImp && _pImp->_pEdFileName )
        sReturn = _pImp->_pEdFileName->GetText();
    return sReturn;
}


void SvtFileDialog::setCurrentFileText( const OUString& _rText, bool _bSelectAll )
{
    if ( _pImp && _pImp->_pEdFileName )
    {
        _pImp->_pEdFileName->SetText( _rText );
        if ( _bSelectAll )
            _pImp->_pEdFileName->SetSelection( Selection( 0, _rText.getLength() ) );
    }
}


bool SvtFileDialog::isAutoExtensionEnabled()
{
    return _pImp->_pCbAutoExtension && _pImp->_pCbAutoExtension->IsChecked();
}


bool SvtFileDialog::getShowState()
{
    if ( _pPrevBmp )
        return _pPrevBmp->IsVisible();
    else
        return false;
}


void SvtFileDialog::ReleaseOwnership( Window* pUserControl )

/*
  [Description]
  This method ensures that the specified element is no longer in possession
  of the instance.
*/

{
    ControlChain_Impl* pElement = _pUserControls;
    while ( pElement )
    {
        if ( pElement->_pControl == pUserControl )
        {
            pElement->_bHasOwnership = false;
            break;
        }
        pElement = pElement->_pNext;
    }
}



bool SvtFileDialog::AddControl( Window* pControl, sal_Bool bNewLine )
{
    // control already exists
    ControlChain_Impl* pElement = _pUserControls;
    while ( pElement )
    {
        if ( pElement->_pControl == pControl )
            return false;
        pElement = pElement->_pNext;
    }

    // Check if controls have already been added.
    Size aNewControlSize( pControl->GetOutputSizePixel() );
    Size aDlgSize( GetOutputSizePixel() );
    WindowType nType = pControl->GetType();
    if ( !aNewControlSize.Height() )
    {
        // Detect a size.
        Size aSize( 0, 10 );
        if ( nType == WINDOW_PUSHBUTTON )
        {
            Size aDefSiz = LogicToPixel( Size( 50, 14 ), MAP_APPFONT );
            long nTextWidth = pControl->GetTextWidth( pControl->GetText() );
            aSize.Width() = nTextWidth + WIDTH_ADDITION;

            // PushButton:  Minimum width 50 logical units,
            //              height always 14 logical units.
            if ( aDefSiz.Width() > aSize.Width() )
                aSize.Width() = aDefSiz.Width();
            aSize.Height() = aDefSiz.Height();
            aNewControlSize = aSize;
        }
        else
            aNewControlSize = LogicToPixel( aSize, MAP_APPFONT );
        if ( nType != WINDOW_PUSHBUTTON )
            aNewControlSize.Width() = pControl->GetTextWidth( pControl->GetText() ) + WIDTH_ADDITION;
        if ( nType == WINDOW_CHECKBOX )
            aNewControlSize.Width() += WIDTH_ADDITION;
        if ( nType == WINDOW_WINDOW )
        {
            aNewControlSize.Height() = GetOutputSizePixel().Height() - 18;
            aNewControlSize.Width() = 200;
            aDlgSize.Width() += 210;
            SetOutputSizePixel( aDlgSize );
        }
        pControl->SetOutputSizePixel( aNewControlSize );
    }
    Point aNewControlPos;
    Size* pNewDlgSize = NULL;
    bool bNewRow = bNewLine;

    if ( nType == WINDOW_WINDOW )
    {
        aNewControlPos.X() = aDlgSize.Width() - 210;
        aNewControlPos.Y() = 8;
    }
    else if ( _pUserControls )
    {
        Point aNewControlRange( _pUserControls->_pControl->GetPosPixel() );
        long nPrevControlHeight = _pUserControls->_pControl->GetSizePixel().Height();
        aNewControlRange +=
            Point( _pUserControls->_pControl->GetOutputSizePixel().Width(), 0 );
        aNewControlPos = aNewControlRange;
        if ( nPrevControlHeight > aNewControlSize.Height() )
        {
            long nY = nPrevControlHeight;
            nY -= aNewControlSize.Height();
            nY /= 2;
            aNewControlPos.Y() += nY;
        }
        aNewControlPos += LogicToPixel( Point( 3, 0 ), MAP_APPFONT );
        aNewControlRange += LogicToPixel( Point( 9, 0 ), MAP_APPFONT );
        aNewControlRange += Point( aNewControlSize.Width(), 0 );

        // Check if a new row has to be created.
        if ( aNewControlRange.X() > aDlgSize.Width() )
            bNewRow = true;
    }
    else
    {
        // Create a new row if there was no usercontrol before.
        bNewRow = true;
    }

    // Check if a new row has to be created.
    Size aBorderSize = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    long nLeftBorder = aBorderSize.Width();
    long nLowerBorder = aBorderSize.Height();
    if ( bNewRow )
    {
        // Set control at the beginning of a new line.
        long nSmallBorderHeight = nLowerBorder / 2;
        aNewControlPos = Point( nLeftBorder, 0 );
        aNewControlPos += Point( 0, aDlgSize.Height() );
        aNewControlPos.Y() -= nSmallBorderHeight;
        // Set new size.
        pNewDlgSize = new Size( aDlgSize );
        pNewDlgSize->Height() -= nSmallBorderHeight;
        pNewDlgSize->Height() += aNewControlSize.Height();
        pNewDlgSize->Height() += nLowerBorder;
    }
    else
    {
        // Check if the window has to be resized.
        Size aNewControlRange( 0, aNewControlPos.Y() );
        aNewControlRange.Height() += aNewControlSize.Height();
        aNewControlRange.Height() += nLowerBorder;
        if ( aNewControlRange.Height() > aDlgSize.Height() )
            pNewDlgSize = new Size( aDlgSize.Width(), aNewControlRange.Height() );
    }

    // Update view.
    if ( pNewDlgSize )
    {
        SetOutputSizePixel( *pNewDlgSize );
        delete pNewDlgSize;
    }
    pControl->SetPosPixel( aNewControlPos );
    pControl->Show();
    _pUserControls = new ControlChain_Impl( pControl, _pUserControls );

    return true;
}

bool SvtFileDialog::ContentHasParentFolder( const OUString& rURL )
{
    m_aContent.bindTo( rURL );

    if ( m_aContent.isInvalid() )
        return false;

    return m_aContent.hasParentFolder( ) && m_aContent.isValid();
}

bool SvtFileDialog::ContentCanMakeFolder( const OUString& rURL )
{
    m_aContent.bindTo( rURL );

    if ( m_aContent.isInvalid() )
        return false;

    return m_aContent.canCreateFolder( ) && m_aContent.isValid();
}

bool SvtFileDialog::ContentGetTitle( const OUString& rURL, OUString& rTitle )
{
    m_aContent.bindTo( rURL );

    if ( m_aContent.isInvalid() )
        return false;

    OUString sTitle;
    m_aContent.getTitle( sTitle );
    rTitle = sTitle;

    return m_aContent.isValid();
}

void SvtFileDialog::appendDefaultExtension(OUString& _rFileName,
                                           const OUString& _rFilterDefaultExtension,
                                           const OUString& _rFilterExtensions)
{
    OUString aTemp(_rFileName);
    aTemp = aTemp.toAsciiLowerCase();
    OUString aType(_rFilterExtensions);
    aType = aType.toAsciiLowerCase();

    if ( ! aType.equals(FILEDIALOG_FILTER_ALL) )
    {
        sal_uInt16 nWildCard = comphelper::string::getTokenCount(aType, FILEDIALOG_DEF_EXTSEP);
        sal_uInt16 nIndex;
        sal_Int32 nPos = 0;

        for ( nIndex = 0; nIndex < nWildCard; nIndex++ )
        {
            OUString aExt(aType.getToken( 0, FILEDIALOG_DEF_EXTSEP, nPos ));
            // take care of a leading *
            sal_Int32 nExtOffset = (aExt[0] == '*' ? 1 : 0);
            const sal_Unicode* pExt = aExt.getStr() + nExtOffset;
            sal_Int32 nExtLen = aExt.getLength() - nExtOffset;
            sal_Int32 nOffset = aTemp.getLength() - nExtLen;
            // minimize search by starting at last possible index
            if ( aTemp.indexOf(pExt, nOffset) == nOffset )
                break;
        }

        if ( nIndex >= nWildCard )
        {
            _rFileName += ".";
            _rFileName += _rFilterDefaultExtension;
        }
    }
}

void SvtFileDialog::initDefaultPlaces( )
{
    PlacePtr pRootPlace( new Place( SVT_RESSTR(STR_DEFAULT_DIRECTORY), GetStandardDir() ) );
    _pImp->_pPlaces->AppendPlace( pRootPlace );

    // Load from user settings
    Sequence< OUString > placesUrlsList(officecfg::Office::Common::Misc::FilePickerPlacesUrls::get(m_context));
    Sequence< OUString > placesNamesList(officecfg::Office::Common::Misc::FilePickerPlacesNames::get(m_context));

    for(sal_Int32 nPlace = 0; nPlace < placesUrlsList.getLength() && nPlace < placesNamesList.getLength(); ++nPlace)
    {
        PlacePtr pPlace(new Place(placesNamesList[nPlace], placesUrlsList[nPlace], true));
        _pImp->_pPlaces->AppendPlace(pPlace);
    }

    // Reset the placesList "updated" state
    _pImp->_pPlaces->IsUpdated();
}

IMPL_LINK_NOARG( SvtFileDialog, Split_Hdl )
{
    sal_Int32 nSplitPos = _pSplitter->GetSplitPosPixel();

    // Resize the places list
    sal_Int32 nPlaceX = _pImp->_pPlaces->GetPosPixel( ).X();
    Size placeSize = _pImp->_pPlaces->GetSizePixel( );
    placeSize.Width() = nSplitPos - nPlaceX;
    _pImp->_pPlaces->SetSizePixel( placeSize );

    // Change Pos and size of the fileview
    Point fileViewPos = _pFileView->GetPosPixel();
    sal_Int32 nOldX = fileViewPos.X();
    sal_Int32 nNewX = nSplitPos + _pSplitter->GetSizePixel().Width();
    fileViewPos.X() = nNewX;
    Size fileViewSize = _pFileView->GetSizePixel();
    fileViewSize.Width() -= ( nNewX - nOldX );
    _pFileView->SetPosSizePixel( fileViewPos, fileViewSize );

    _pSplitter->SetPosPixel( Point( nSplitPos, _pSplitter->GetPosPixel().Y() ) );
    return 0;
}

QueryFolderNameDialog::QueryFolderNameDialog(Window* _pParent,
    const OUString& rTitle, const OUString& rDefaultText, OUString* pGroupName)
    : ModalDialog(_pParent, "FolderNameDialog", "fps/ui/foldernamedialog.ui")
{
    get(m_pNameEdit, "entry");
    get(m_pNameLine, "frame");
    get(m_pOKBtn, "ok");

    SetText( rTitle );
    m_pNameEdit->SetText( rDefaultText );
    m_pNameEdit->SetSelection( Selection( 0, rDefaultText.getLength() ) );
    m_pOKBtn->SetClickHdl( LINK( this, QueryFolderNameDialog, OKHdl ) );
    m_pNameEdit->SetModifyHdl( LINK( this, QueryFolderNameDialog, NameHdl ) );

    if (pGroupName)
        m_pNameLine->set_label( *pGroupName );
};


IMPL_LINK_NOARG(QueryFolderNameDialog, OKHdl)
{
    // trim the strings
    m_pNameEdit->SetText(comphelper::string::strip(m_pNameEdit->GetText(), ' '));
    EndDialog( RET_OK );
    return 1;
}


IMPL_LINK_NOARG(QueryFolderNameDialog, NameHdl)
{
    // trim the strings
    OUString aName = comphelper::string::strip(m_pNameEdit->GetText(), ' ');
    if ( !aName.isEmpty() )
    {
        if ( !m_pOKBtn->IsEnabled() )
            m_pOKBtn->Enable( true );
    }
    else
    {
        if ( m_pOKBtn->IsEnabled() )
            m_pOKBtn->Enable( false );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
