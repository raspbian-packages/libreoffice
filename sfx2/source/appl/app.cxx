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


#if defined UNX
#include <limits.h>
#else // UNX
#include <stdlib.h>
#define PATH_MAX _MAX_PATH
#endif // UNX

#include <sfx2/app.hxx>
#include <sfx2/frame.hxx>
#include <tools/simplerm.hxx>
#include <tools/config.hxx>
#include <basic/basrdll.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <svtools/asynclink.hxx>
#include <svl/stritem.hxx>
#include <vcl/sound.hxx>
#include <svl/eitem.hxx>
#include <svl/urlbmk.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>

#include <svl/svdde.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <osl/file.hxx>

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#include <com/sun/star/mozilla/XPluginInstance.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <basic/basmgr.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <rtl/logfile.hxx>
#include <sfx2/appuno.hxx>
#include "sfx2/sfxhelp.hxx"
#include <sfx2/request.hxx>
#include "sfxtypes.hxx"
#include "sfx2/sfxresid.hxx"
#include "arrdecl.hxx"
#include <sfx2/progress.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include "fltfnc.hxx"
#include "nfltdlg.hxx"
#include <sfx2/new.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/genlink.hxx>
#include <sfx2/viewfrm.hxx>
#include "appdata.hxx"
#include "openflag.hxx"
#include "app.hrc"
#include "virtmenu.hxx"
#include <sfx2/module.hxx>
#include <sfx2/event.hxx>
#include "imestatuswindow.hxx"
#include "workwin.hxx"
#include <sfx2/module.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/sfxdlg.hxx>
#include "sfx2/stbitem.hxx"
#include "eventsupplier.hxx"
#include <sfx2/dockwin.hxx>
#include <tools/svlibrary.hxx>

#ifdef DBG_UTIL
#include <sfx2/tbxctrl.hxx>
#include <sfx2/mnuitem.hxx>
#endif

#include <unotools/saveopt.hxx>
#include <unotools/undoopt.hxx>
#include <svtools/helpopt.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/historyoptions.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/startoptions.hxx>
#include <unotools/securityoptions.hxx>
#include <unotools/localisationoptions.hxx>
#include <unotools/inetoptions.hxx>
#include <unotools/fontoptions.hxx>
#include <unotools/internaloptions.hxx>
#include <unotools/workingsetoptions.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <unotools/syslocale.hxx>
#include <framework/addonsoptions.hxx>
#include <svtools/ttprops.hxx>
#include <unotools/extendedsecurityoptions.hxx>
#include <rtl/instance.hxx>
#include <rtl/strbuf.hxx>

using namespace ::com::sun::star;

// Static member
SfxApplication* SfxApplication::pApp = NULL;
static BasicDLL*       pBasic   = NULL;
static SfxHelp*        pSfxHelp = NULL;

namespace
{
    class SfxPropertyHandler : public PropertyHandler
    {
        virtual void Property( ApplicationProperty& );
    };

    void SfxPropertyHandler::Property( ApplicationProperty& rProp )
    {
        TTProperties* pTTProperties = PTR_CAST( TTProperties, &rProp );
        if ( pTTProperties )
        {
            pTTProperties->nPropertyVersion = TT_PROPERTIES_VERSION;
            switch ( pTTProperties->nActualPR )
            {
                case TT_PR_SLOTS:
                {
                    pTTProperties->nSidOpenUrl = SID_OPENURL;
                    pTTProperties->nSidFileName = SID_FILE_NAME;
                    pTTProperties->nSidNewDocDirect = SID_NEWDOCDIRECT;
                    pTTProperties->nSidCopy = SID_COPY;
                    pTTProperties->nSidPaste = SID_PASTE;
                    pTTProperties->nSidSourceView = SID_SOURCEVIEW;
                    pTTProperties->nSidSelectAll = SID_SELECTALL;
                    pTTProperties->nSidReferer = SID_REFERER;
                    pTTProperties->nActualPR = 0;
                }
                break;
                case TT_PR_DISPATCHER:
                {
                    // interface for TestTool
                    SfxViewFrame* pViewFrame=0;
                    SfxDispatcher* pDispatcher=0;
                    pViewFrame = SfxViewFrame::Current();
                    if ( !pViewFrame )
                        pViewFrame = SfxViewFrame::GetFirst();
                    if ( pViewFrame )
                        pDispatcher = pViewFrame->GetDispatcher();
                    else
                        pDispatcher = NULL;
                    if ( !pDispatcher )
                        pTTProperties->nActualPR = TT_PR_ERR_NODISPATCHER;
                    else
                    {
                        pDispatcher->SetExecuteMode(EXECUTEMODE_DIALOGASYNCHRON);
                        if ( pTTProperties->mnSID == SID_NEWDOCDIRECT
                          || pTTProperties->mnSID == SID_OPENDOC )
                        {
                            SfxPoolItem** pArgs = pTTProperties->mppArgs;
                            SfxAllItemSet aSet( SFX_APP()->GetPool() );
                            if ( pArgs && *pArgs )
                            {
                                for ( SfxPoolItem **pArg = pArgs; *pArg; ++pArg )
                                    aSet.Put( **pArg );
                            }
                            if ( pTTProperties->mnSID == SID_NEWDOCDIRECT )
                            {
                                String aFactory = String::CreateFromAscii("private:factory/");
                                if ( pArgs && *pArgs )
                                {
                                    SFX_ITEMSET_ARG( &aSet, pFactoryName, SfxStringItem, SID_NEWDOCDIRECT, sal_False );
                                    if ( pFactoryName )
                                        aFactory += pFactoryName->GetValue();
                                    else
                                        aFactory += String::CreateFromAscii("swriter");
                                }
                                else
                                    aFactory += String::CreateFromAscii("swriter");

                                aSet.Put( SfxStringItem( SID_FILE_NAME, aFactory ) );
                                aSet.ClearItem( SID_NEWDOCDIRECT );
                                pTTProperties->mnSID = SID_OPENDOC;
                            }

                            aSet.Put( SfxStringItem( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") ) );
                            if ( pDispatcher->ExecuteFunction( pTTProperties->mnSID, aSet, pTTProperties->mnMode )
                                        == EXECUTE_NO )
                                pTTProperties->nActualPR = TT_PR_ERR_NOEXECUTE;
                            else
                                pTTProperties->nActualPR = 0;
                        }
                        else
                        {
                            if ( pDispatcher->ExecuteFunction(
                                    pTTProperties->mnSID, pTTProperties->mppArgs, pTTProperties->mnMode )
                                == EXECUTE_NO )
                                pTTProperties->nActualPR = TT_PR_ERR_NOEXECUTE;
                            else
                                pTTProperties->nActualPR = 0;
                        }
                    }
                }
                break;
                default:
                {
                    pTTProperties->nPropertyVersion = 0;
                }
            }
            return;
        }
    }

    class thePropertyHandler
        : public rtl::Static<SfxPropertyHandler, thePropertyHandler> {};

    class theApplicationMutex
        : public rtl::Static<osl::Mutex, theApplicationMutex> {};
}

#include <framework/imageproducer.hxx>
#include <framework/sfxhelperfunctions.hxx>
#include "sfx2/imagemgr.hxx"
#include "fwkhelper.hxx"

SfxApplication* SfxApplication::GetOrCreate()
{
    // SFX on demand
    ::osl::MutexGuard aGuard(theApplicationMutex::get());
    if (!pApp)
    {
        RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mb93783) ::SfxApplication::SetApp" );

        pApp = new SfxApplication;

        // at the moment a bug may occur when Initialize_Impl returns FALSE,
        // but this is only temporary because all code that may cause such
        // a fault will be moved outside the SFX
        pApp->Initialize_Impl();

        ::framework::SetImageProducer( GetImage );
        ::framework::SetRefreshToolbars( RefreshToolbars );
        ::framework::SetToolBoxControllerCreator( SfxToolBoxControllerFactory );
        ::framework::SetStatusBarControllerCreator( SfxStatusBarControllerFactory );
        ::framework::SetDockingWindowCreator( SfxDockingWindowFactory );
        ::framework::SetIsDockingWindowVisible( IsDockingWindowVisible );
        ::framework::SetActivateToolPanel( &SfxViewFrame::ActivateToolPanel );

        Application::SetHelp( pSfxHelp );
        if ( SvtHelpOptions().IsHelpTips() )
            Help::EnableQuickHelp();
        else
            Help::DisableQuickHelp();
        if ( SvtHelpOptions().IsHelpTips() && SvtHelpOptions().IsExtendedHelp() )
            Help::EnableBalloonHelp();
        else
            Help::DisableBalloonHelp();
    }
    return pApp;
}

SfxApplication::SfxApplication()
    : pAppData_Impl( 0 )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mb93783) ::SfxApplication::SfxApplication" );

    SetName( DEFINE_CONST_UNICODE("StarOffice") );
    GetpApp()->SetPropertyHandler( &thePropertyHandler::get() );

    SvtViewOptions::AcquireOptions();

    pAppData_Impl = new SfxAppData_Impl( this );
    pAppData_Impl->m_xImeStatusWindow->init();
    pApp->PreInit();

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ initialize DDE" );

    sal_Bool bOk = InitializeDde();

#ifdef DBG_UTIL
    if( !bOk )
    {
        rtl::OStringBuffer aStr(
            RTL_CONSTASCII_STRINGPARAM("No DDE-Service possible. Error: "));
        if( GetDdeService() )
            aStr.append(static_cast<sal_Int32>(GetDdeService()->GetError()));
        else
            aStr.append('?');
        DBG_ASSERT( sal_False, aStr.getStr() );
    }
#else
    (void)bOk;
#endif

    pSfxHelp = new SfxHelp;

    pBasic   = new BasicDLL;
    StarBASIC::SetGlobalErrorHdl( LINK( this, SfxApplication, GlobalBasicErrorHdl_Impl ) );
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "} initialize DDE" );
}

SfxApplication::~SfxApplication()
{
    OSL_ENSURE( GetObjectShells_Impl().Count() == 0, "Memory leak: some object shells were not removed!" );

    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );

    SfxModule::DestroyModules_Impl();

    delete pSfxHelp;
    Application::SetHelp( NULL );

    // delete global options
    SvtViewOptions::ReleaseOptions();
    delete pBasic;

    if ( !pAppData_Impl->bDowning )
        Deinitialize();

    delete pAppData_Impl;
    pApp = 0;
}

//====================================================================

const String& SfxApplication::GetLastDir_Impl() const

/*  [Description]

    Internal method by which the last set directory with the method
    <SfxApplication::SetLastDir_Impl()> in SFX is returned.

    This is usually the most recently addressed by the
    SfxFileDialog directory.

    [Cross-reference]
    <SfxApplication::SetLastDir_Impl()>
*/

{
    return pAppData_Impl->aLastDir;
}

const String& SfxApplication::GetLastSaveDirectory() const

/*  [Description]

    As <SfxApplication::GetLastDir_Impl()>, only external

    [Cross-reference]
    <SfxApplication::GetLastDir_Impl()>
*/

{
    return GetLastDir_Impl();
}

//--------------------------------------------------------------------

void SfxApplication::SetLastDir_Impl
(
    const String&   rNewDir     /* Complete directory path as a string */
)

/*  [Description]

    Internal Method, by which a directory path is set that was last addressed
    (eg by the SfxFileDialog).

    [Cross-reference]
    <SfxApplication::GetLastDir_Impl()>
*/

{
    pAppData_Impl->aLastDir = rNewDir;
}

//--------------------------------------------------------------------

void SfxApplication::ResetLastDir()
{
    String aEmpty;
    pAppData_Impl->aLastDir = aEmpty;
}

//--------------------------------------------------------------------

SfxDispatcher* SfxApplication::GetDispatcher_Impl()
{
    return pAppData_Impl->pViewFrame? pAppData_Impl->pViewFrame->GetDispatcher(): pAppData_Impl->pAppDispat;
}

//--------------------------------------------------------------------
void SfxApplication::SetViewFrame_Impl( SfxViewFrame *pFrame )
{
    if ( pFrame != pAppData_Impl->pViewFrame )
    {
        // get the containerframes ( if one of the frames is an InPlaceFrame )
        SfxViewFrame *pOldContainerFrame = pAppData_Impl->pViewFrame;
        while ( pOldContainerFrame && pOldContainerFrame->GetParentViewFrame_Impl() )
            pOldContainerFrame = pOldContainerFrame->GetParentViewFrame_Impl();
        SfxViewFrame *pNewContainerFrame = pFrame;
        while ( pNewContainerFrame && pNewContainerFrame->GetParentViewFrame_Impl() )
            pNewContainerFrame = pNewContainerFrame->GetParentViewFrame_Impl();

        // DocWinActivate : both frames belong to the same TopWindow
        // TopWinActivate : both frames belong to different TopWindows

        sal_Bool bTaskActivate = pOldContainerFrame != pNewContainerFrame;

        if ( pOldContainerFrame )
        {
            if ( bTaskActivate )
                NotifyEvent( SfxViewEventHint( SFX_EVENT_DEACTIVATEDOC, GlobalEventConfig::GetEventName(STR_EVENT_DEACTIVATEDOC), pOldContainerFrame->GetObjectShell(), pOldContainerFrame->GetFrame().GetController() ) );
            pOldContainerFrame->DoDeactivate( bTaskActivate, pFrame );

            if( pOldContainerFrame->GetProgress() )
                pOldContainerFrame->GetProgress()->Suspend();
        }

        pAppData_Impl->pViewFrame = pFrame;

        if( pNewContainerFrame )
        {
            pNewContainerFrame->DoActivate( bTaskActivate );
            if ( bTaskActivate && pNewContainerFrame->GetObjectShell() )
            {
                pNewContainerFrame->GetObjectShell()->PostActivateEvent_Impl( pNewContainerFrame );
                NotifyEvent(SfxViewEventHint(SFX_EVENT_ACTIVATEDOC, GlobalEventConfig::GetEventName(STR_EVENT_ACTIVATEDOC), pNewContainerFrame->GetObjectShell(), pNewContainerFrame->GetFrame().GetController() ) );
            }

            SfxProgress *pProgress = pNewContainerFrame->GetProgress();
            if ( pProgress )
            {
                if( pProgress->IsSuspended() )
                    pProgress->Resume();
                else
                    pProgress->SetState( pProgress->GetState() );
            }

            if ( pAppData_Impl->pViewFrame->GetViewShell() )
            {
                SfxDispatcher* pDisp = pAppData_Impl->pViewFrame->GetDispatcher();
                pDisp->Flush();
                pDisp->Update_Impl(sal_True);
            }
        }
    }

    // even if the frame actually didn't change, ensure its document is forwarded
    // to SfxObjectShell::SetCurrentComponent.
    // Otherwise, the CurrentComponent might not be correct, in case it has meanwhile
    // been reset to some other document, by some non-SFX component.
    // #i49133# / 2007-12-19 / frank.schoenheit@sun.com
    if ( pFrame && pFrame->GetViewShell() )
        pFrame->GetViewShell()->SetCurrentDocument();
}

//---------------------------------------------------------------------

ResMgr* SfxApplication::CreateResManager( const char *pPrefix )
{
    return ResMgr::CreateResMgr(pPrefix);
}

//--------------------------------------------------------------------

ResMgr* SfxApplication::GetSfxResManager()
{
    return SfxResId::GetResMgr();
}

//------------------------------------------------------------------------

void SfxApplication::SetProgress_Impl
(
    SfxProgress *pProgress

)
{
    DBG_ASSERT( ( !pAppData_Impl->pProgress && pProgress ) ||
                ( pAppData_Impl->pProgress && !pProgress ),
                "Progress acitivation/deacitivation mismatch" );

    if ( pAppData_Impl->pProgress && pProgress )
    {
        pAppData_Impl->pProgress->Suspend();
        pAppData_Impl->pProgress->UnLock();
        delete pAppData_Impl->pProgress;
    }

    pAppData_Impl->pProgress = pProgress;
}

//------------------------------------------------------------------------

sal_uInt16 SfxApplication::GetFreeIndex()
{
    return pAppData_Impl->aIndexBitSet.GetFreeIndex()+1;
}

//------------------------------------------------------------------------

void SfxApplication::ReleaseIndex(sal_uInt16 i)
{
    pAppData_Impl->aIndexBitSet.ReleaseIndex(i-1);
}

//--------------------------------------------------------------------

Window* SfxApplication::GetTopWindow() const
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( SfxViewFrame::Current() );
    return pWork ? pWork->GetWindow() : NULL;
}

SfxTbxCtrlFactArr_Impl&     SfxApplication::GetTbxCtrlFactories_Impl() const
{
    return *pAppData_Impl->pTbxCtrlFac;
}

SfxStbCtrlFactArr_Impl&     SfxApplication::GetStbCtrlFactories_Impl() const
{
    return *pAppData_Impl->pStbCtrlFac;
}

SfxMenuCtrlFactArr_Impl&    SfxApplication::GetMenuCtrlFactories_Impl() const
{
    return *pAppData_Impl->pMenuCtrlFac;
}

SfxViewFrameArr_Impl&       SfxApplication::GetViewFrames_Impl() const
{
    return *pAppData_Impl->pViewFrames;
}

SfxViewShellArr_Impl&       SfxApplication::GetViewShells_Impl() const
{
    return *pAppData_Impl->pViewShells;
}

SfxObjectShellArr_Impl&     SfxApplication::GetObjectShells_Impl() const
{
    return *pAppData_Impl->pObjShells;
}

void SfxApplication::Invalidate( sal_uInt16 nId )
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst(); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame ) )
        Invalidate_Impl( pFrame->GetBindings(), nId );
}

#define DOSTRING( x )                       #x
#define STRING( x )                         DOSTRING( x )

typedef long (SAL_CALL *basicide_handle_basic_error)(void*);
typedef rtl_uString* (SAL_CALL *basicide_choose_macro)(void*, sal_Bool, rtl_uString*);
typedef void* (SAL_CALL *basicide_macro_organizer)(sal_Int16);

extern "C" { static void SAL_CALL thisModule() {} }

IMPL_LINK( SfxApplication, GlobalBasicErrorHdl_Impl, StarBASIC*, pStarBasic )
{
    // get basctl dllname
    static ::rtl::OUString aLibName( RTL_CONSTASCII_USTRINGPARAM( SVLIBRARY( "basctl" ) ) );

    // load module
    oslModule handleMod = osl_loadModuleRelative(
        &thisModule, aLibName.pData, 0 );

    // get symbol
    ::rtl::OUString aSymbol( RTL_CONSTASCII_USTRINGPARAM( "basicide_handle_basic_error" ) );
    basicide_handle_basic_error pSymbol = (basicide_handle_basic_error) osl_getFunctionSymbol( handleMod, aSymbol.pData );

    // call basicide_handle_basic_error in basctl
    long nRet = pSymbol ? pSymbol( pStarBasic ) : 0;

    return nRet;
}

sal_Bool SfxApplication::IsXScriptURL( const String& rScriptURL )
{
    sal_Bool result = sal_False;

    ::com::sun::star::uno::Reference
        < ::com::sun::star::lang::XMultiServiceFactory > xSMgr =
            ::comphelper::getProcessServiceFactory();

    ::com::sun::star::uno::Reference
        < ::com::sun::star::uri::XUriReferenceFactory >
            xFactory( xSMgr->createInstance(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.uri.UriReferenceFactory" )) ),
                ::com::sun::star::uno::UNO_QUERY );

    if ( xFactory.is() )
    {
        try
        {
            ::com::sun::star::uno::Reference
                < ::com::sun::star::uri::XVndSunStarScriptUrl >
                    xUrl( xFactory->parse( rScriptURL ),
                        ::com::sun::star::uno::UNO_QUERY );

            if ( xUrl.is() )
            {
                result = sal_True;
            }
        }
        catch ( ::com::sun::star::uno::RuntimeException& )
        {
            // ignore, will just return FALSE
        }
    }
    return result;
}

::rtl::OUString
SfxApplication::ChooseScript()
{
    ::rtl::OUString aScriptURL;

    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    if ( pFact )
    {
        OSL_TRACE("create selector dialog");

        const SfxViewFrame* pViewFrame = SfxViewFrame::Current();
        const SfxFrame* pFrame = pViewFrame ? &pViewFrame->GetFrame() : NULL;
        uno::Reference< frame::XFrame > xFrame( pFrame ? pFrame->GetFrameInterface() : uno::Reference< frame::XFrame >() );

          AbstractScriptSelectorDialog* pDlg =
            pFact->CreateScriptSelectorDialog( NULL, sal_False, xFrame );

        OSL_TRACE("done, now exec it");

          sal_uInt16 nRet = pDlg->Execute();

        OSL_TRACE("has returned");

        if ( nRet == RET_OK )
        {
            aScriptURL = pDlg->GetScriptURL();
        }

          delete pDlg;
    }
    return aScriptURL;
}

void SfxApplication::MacroOrganizer( sal_Int16 nTabId )
{
    // get basctl dllname
    static ::rtl::OUString aLibName( RTL_CONSTASCII_USTRINGPARAM( SVLIBRARY( "basctl" ) ) );

    // load module
    oslModule handleMod = osl_loadModuleRelative(
        &thisModule, aLibName.pData, 0 );

    // get symbol
    ::rtl::OUString aSymbol( RTL_CONSTASCII_USTRINGPARAM( "basicide_macro_organizer" ) );
    basicide_macro_organizer pSymbol = (basicide_macro_organizer) osl_getFunctionSymbol( handleMod, aSymbol.pData );

    // call basicide_choose_macro in basctl
    pSymbol( nTabId );
}

ErrCode SfxApplication::CallBasic( const String& rCode, BasicManager* pMgr, SbxArray* pArgs, SbxValue* pRet )
{
    return pMgr->ExecuteMacro( rCode, pArgs, pRet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
