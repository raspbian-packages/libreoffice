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
#include "app.hxx"

#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/frame/XDesktop.hpp>

#include <bf_svtools/svtools.hrc>
#include <bf_svtools/saveopt.hxx>
#include <bf_svtools/localisationoptions.hxx>

#include <tools/config.hxx>
#include <tools/resary.hxx>

#include <bf_so3/soerr.hxx>
#include <bf_svtools/intitem.hxx>
#include <bf_svtools/eitem.hxx>
#include <bf_svtools/stritem.hxx>
#include <bf_so3/inetbnd.hxx>
#include <vcl/msgbox.hxx>
#include <bf_svtools/ehdl.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/security.hxx>
#include <ucbhelper/configurationkeys.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <bf_svtools/historyoptions.hxx>
#include <bf_svtools/moduleoptions.hxx>

#include <rtl/logfile.hxx>

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "appimp.hxx"
#include "appdata.hxx"
#include "arrdecl.hxx"
#include "cfgmgr.hxx"
#include "docfac.hxx"
#include "evntconf.hxx"
#include "frameobj.hxx"
#include "interno.hxx"
#include "macrconf.hxx"
#include "plugobj.hxx"
#include "progress.hxx"
#include "sfxtypes.hxx"

#include <bf_svtools/imgdef.hxx>

#include "nochaos.hxx"
#include "fcontnr.hxx"
#include "helper.hxx"   // SfxContentHelper::Kill()

#include <legacysmgr/legacy_binfilters_smgr.hxx>

#include <osl/mutex.hxx>

#include <cppuhelper/implbase1.hxx>

namespace binfilter {

#ifdef UNX
#define stricmp(a,b) strcmp(a,b)
#endif

#ifdef MAC
int svstricmp( const char* pStr1, const char* pStr2);
#define stricmp svstricmp
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;

void doFirstTimeInit();

class SfxTerminateListener_Impl : public ::cppu::WeakImplHelper1< XTerminateListener  >
{
public:
    virtual void SAL_CALL queryTermination( const EventObject& aEvent ) throw( TerminationVetoException, RuntimeException );
    virtual void SAL_CALL notifyTermination( const EventObject& aEvent ) throw( RuntimeException );
    virtual void SAL_CALL disposing( const EventObject& Source ) throw( RuntimeException );
};

/*N*/ void SAL_CALL SfxTerminateListener_Impl::disposing( const EventObject& /*Source*/ ) throw( RuntimeException )
/*N*/ {
/*N*/ }

/*N*/ void SAL_CALL SfxTerminateListener_Impl::queryTermination( const EventObject& /*aEvent*/ ) throw(TerminationVetoException, RuntimeException )
/*N*/ {
/*N*/ }

/*N*/ void SAL_CALL SfxTerminateListener_Impl::notifyTermination( const EventObject& aEvent ) throw(RuntimeException )
/*N*/ {
/*N*/     Reference< XDesktop > xDesktop( aEvent.Source, UNO_QUERY );
/*N*/     if( xDesktop.is() == sal_True )
/*N*/         xDesktop->removeTerminateListener( this );
/*N*/
/*N*/     SolarMutexGuard aGuard;
/*N*/     SfxApplication* pApp = SFX_APP();
/*N*/     pApp->Broadcast( SfxSimpleHint( SFX_HINT_DEINITIALIZING ) );
/*N*/     pApp->NotifyEvent(SfxEventHint( SFX_EVENT_CLOSEAPP) );
/*N*/     pApp->Deinitialize();
/*N*/   Application::Quit();
/*N*/ }

//====================================================================

//====================================================================

/*N*/ bool SfxApplication::Initialize_Impl()
/*N*/ {
/*N*/
/*N*/     Reference < XDesktop > xDesktop ( ::legacy_binfilters::getLegacyProcessServiceFactory()->createInstance(
                                           ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" )) ), UNO_QUERY );
/*N*/     xDesktop->addTerminateListener( new SfxTerminateListener_Impl() );
/*N*/
/*N*/     // StarObjects initialisieren
/*N*/   SvFactory::Init();
/*N*/
/*N*/   // Factory f"ur das SfxFrameObject anlegen; da der Pointer in den AppDaten
/*N*/   // liegt, dieser aber nicht exportierbar ist, mu\s ein exportierbarer
/*N*/   // Wrapper angelegt werden
/*N*/   pAppData_Impl->pSfxFrameObjectFactoryPtr = new SfxFrameObjectFactoryPtr;
/*N*/   pAppData_Impl->pSfxFrameObjectFactoryPtr->pSfxFrameObjectFactory = SfxFrameObject::ClassFactory();
/*N*/
/*N*/   pImp->pEventHdl = new UniqueIndex( 1 );
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/   // Der SimplerErrorHandler dient Debugzwecken. In der Product werden
/*N*/   // nichtgehandelte Fehler durch Errorcode 1 an SFX gegeben.
/*N*/   new SimpleErrorHandler;
/*N*/ #endif
/*N*/   new SfxErrorHandler(RID_ERRHDL, ERRCODE_AREA_TOOLS, ERRCODE_AREA_LIB1);
/*N*/   new SfxErrorHandler(
/*N*/       RID_SO_ERROR_HANDLER, ERRCODE_AREA_SO, ERRCODE_AREA_SO_END);
/*N*/   new SfxErrorHandler(
/*N*/       (RID_SJ_START +1), ERRCODE_AREA_JAVA, ERRCODE_AREA_JAVA_END);
/*N*/   new SfxErrorHandler(
/*N*/       RID_BASIC_START, ERRCODE_AREA_SBX, ERRCODE_AREA_SBX_END );
/*N*/
/*N*/   /////////////////////////////////////////////////////////////////
/*N*/
/*N*/   pImp->pObjShells = new SfxObjectShellArr_Impl;
/*N*/
/*N*/ //    SolarMutexGuard aGuard;
/*N*/
/*N*/   SfxEventConfiguration::RegisterEvent(SFX_EVENT_STARTAPP,        String(),   String::CreateFromAscii("OnStartApp") );
/*N*/   SfxEventConfiguration::RegisterEvent(SFX_EVENT_CLOSEAPP,        String(),   String::CreateFromAscii("OnCloseApp") );
/*N*/   SfxEventConfiguration::RegisterEvent(SFX_EVENT_CREATEDOC,       String(),   String::CreateFromAscii("OnNew") );
/*N*/   SfxEventConfiguration::RegisterEvent(SFX_EVENT_OPENDOC,         String(),   String::CreateFromAscii("OnLoad") );
/*N*/   SfxEventConfiguration::RegisterEvent(SFX_EVENT_SAVEASDOC,       String(),   String::CreateFromAscii("OnSaveAs") );
/*N*/   SfxEventConfiguration::RegisterEvent(SFX_EVENT_SAVEASDOCDONE,   String(),   String::CreateFromAscii("OnSaveAsDone") );
/*N*/   SfxEventConfiguration::RegisterEvent(SFX_EVENT_SAVEDOC,         String(),   String::CreateFromAscii("OnSave") );
/*N*/   SfxEventConfiguration::RegisterEvent(SFX_EVENT_SAVEDOCDONE,     String(),   String::CreateFromAscii("OnSaveDone") );
/*N*/   SfxEventConfiguration::RegisterEvent(SFX_EVENT_PREPARECLOSEDOC, String(),   String::CreateFromAscii("OnPrepareUnload") );
/*N*/   SfxEventConfiguration::RegisterEvent(SFX_EVENT_CLOSEDOC,        String(),   String::CreateFromAscii("OnUnload") );
/*N*/   SfxEventConfiguration::RegisterEvent(SFX_EVENT_ACTIVATEDOC,     String(),   String::CreateFromAscii("OnFocus") );
/*N*/   SfxEventConfiguration::RegisterEvent(SFX_EVENT_DEACTIVATEDOC,   String(),   String::CreateFromAscii("OnUnfocus") );
/*N*/   SfxEventConfiguration::RegisterEvent(SFX_EVENT_PRINTDOC,        String(),   String::CreateFromAscii("OnPrint") );
/*N*/   SfxEventConfiguration::RegisterEvent(SFX_EVENT_MODIFYCHANGED,   String(),   String::CreateFromAscii("OnModifyChanged") );
/*N*/
/*N*/   // Subklasse initialisieren
/*N*/   bDowning = sal_False;
/*N*/   bInInit = sal_True;
/*N*/   Init();
/*N*/
/*N*/   // get CHAOS item pool...
/*N*/   pAppData_Impl->pPool = NoChaos::GetItemPool();
/*N*/   SetPool( pAppData_Impl->pPool );
/*N*/
/*N*/   bInInit = sal_False;
/*N*/   if ( bDowning )
/*N*/       return sal_False;
/*N*/
/*N*/     return sal_True;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
