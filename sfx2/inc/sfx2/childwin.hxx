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
#ifndef _SFX_CHILDWIN_HXX
#define _SFX_CHILDWIN_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <vcl/window.hxx>
#include <com/sun/star/frame/XFrame.hpp>

#ifdef ENABLE_INIMANAGER
#include "inimgr.hxx"
#endif
#include <sfx2/shell.hxx>
#include <sfx2/chalign.hxx>

class SfxWorkWindow;
class SfxModule;
class SfxBindings;
class SfxShell;
class SfxChildWindow;
class SfxChildWindowContext;
class SfxChildWinContextArr_Impl;

#define SFX_CHILDWIN_ZOOMIN          0x01  // Fully retracted Float
#define SFX_CHILDWIN_SMALL           0x02  // Half retracted Float
#define SFX_CHILDWIN_FORCEDOCK       0x04  // Float forbidden
#define SFX_CHILDWIN_AUTOHIDE        0x08  // DockingWindow in AutoHide mode
#define SFX_CHILDWIN_TASK            0x10  // ChildWindow inside the Task
#define SFX_CHILDWIN_CANTGETFOCUS    0x20  // ChildWindow can not get focus
#define SFX_CHILDWIN_ALWAYSAVAILABLE 0x40   // ChildWindow is never disabled
#define SFX_CHILDWIN_NEVERHIDE       0x80  // ChildWindow is can always made
                                           // visible/is visible
#define CHILDWIN_NOPOS            USHRT_MAX

// ChildWindow Configuration
struct SfxChildWinInfo
{
    sal_Bool                bVisible;
    Point               aPos;
    Size                aSize;
    sal_uInt16              nFlags;
    String              aExtraString;
    rtl::OString        aWinState;

                        SfxChildWinInfo()
                        {
                            bVisible = sal_False;
                            nFlags = 0;
                        }
    sal_Bool                GetExtraData_Impl( SfxChildAlignment    *pAlign,
                                           SfxChildAlignment    *pLastAlign = 0,
                                           Size                 *pSize = 0,
                                           sal_uInt16               *pLine = 0,
                                           sal_uInt16               *pPos = 0 ) const;
};

// ChildWindow factory methods
typedef SfxChildWindow* (*SfxChildWinCtor)( ::Window *pParentWindow,
                                            sal_uInt16 nId,
                                            SfxBindings *pBindings,
                                            SfxChildWinInfo *pInfo);

// ChildWindowsContexts factory methods
typedef SfxChildWindowContext* (*SfxChildWinContextCtor)( ::Window *pParentWindow,
                                            SfxBindings *pBindings,
                                            SfxChildWinInfo *pInfo);
struct SfxChildWinContextFactory
{
    SfxChildWinContextCtor  pCtor;      // Factory method
    sal_uInt16              nContextId; // Idenifier for SfxInterface
    SfxChildWinInfo         aInfo;      // Configuration

    SfxChildWinContextFactory( SfxChildWinContextCtor pTheCtor, sal_uInt16 nID )
        : pCtor(pTheCtor)
        , nContextId(nID)
    {}
};

SV_DECL_PTRARR_DEL( SfxChildWinContextArr_Impl, SfxChildWinContextFactory*, 2, 2 )

struct SfxChildWinFactory
{
    SfxChildWinCtor            pCtor;  // Factory method
    sal_uInt16              nId;            // ChildWindow-Id ( SlotId )
    SfxChildWinInfo            aInfo;  // Configuration
    sal_uInt16                  nPos;  // Position in UI
    SfxChildWinContextArr_Impl *pArr;  // Array for Contexts

    SfxChildWinFactory( SfxChildWinCtor pTheCtor, sal_uInt16 nID,
            sal_uInt16 n )
        : pCtor(pTheCtor)
        , nId( nID )
        , nPos(n)
        , pArr( NULL )
    {}

    ~SfxChildWinFactory()
    {
        delete pArr;
    }

};

class FloatingWindow;
struct SfxChildWindow_Impl;
class SFX2_DLLPUBLIC SfxChildWindowContext
{
friend class SfxChildWindow;
    ::Window*           pWindow;
    sal_uInt16          nContextId;

protected:
                        SfxChildWindowContext( sal_uInt16 nId );

public:
    virtual             ~SfxChildWindowContext();

    void                SetWindow( ::Window* pWin )
                        { pWindow=pWin; }
    ::Window*           GetWindow() const
                        { return pWindow; }
    sal_uInt16              GetContextId() const
                        { return nContextId; }

    FloatingWindow*     GetFloatingWindow() const;
    SfxChildAlignment   GetAlignment() const;

    virtual void        Resizing( Size& rSize );
    virtual sal_Bool        Close();
    static void         RegisterChildWindowContext(SfxModule*, sal_uInt16, SfxChildWinContextFactory*);
};

class SFX2_DLLPUBLIC SfxChildWindow
{
    ::Window*               pParent;        // parent window ( Topwindow )
    sal_uInt16              nType;          // ChildWindow-Id

protected:
    SfxChildAlignment           eChildAlignment; // Current ::com::sun::star::drawing::Alignment
    ::Window*                   pWindow;         // actual contents
    SfxChildWindow_Impl*        pImp;            // Implementation data

private:
    SfxChildWindowContext*      pContext;        // With context-sensitive ChildWindows:
                                                 // Annother window in pWindow
    SAL_DLLPRIVATE SfxChildWindowContext*
                        GetContext() const
                        { return pContext; }

protected:
                        SfxChildWindow(::Window *pParentWindow, sal_uInt16 nId);

public:
    virtual             ~SfxChildWindow();
    void                Destroy();
    ::Window*           GetWindow() const
                        { return pWindow; }
    ::Window*           GetParent() const
                        { return pParent; }
    SfxChildAlignment   GetAlignment() const
                        { return eChildAlignment; }
    void                SetAlignment(SfxChildAlignment eAlign);
    Size                GetSizePixel() const
                        { return pWindow->GetSizePixel(); }
    void                SetPosSizePixel(const Point& rPoint, const Size& rSize)
                        { pWindow->SetPosSizePixel(rPoint, rSize); }
    Point               GetPosPixel()
                        { return pWindow->GetPosPixel(); }
    virtual void                Hide();
    virtual void                Show( sal_uInt16 nFlags );
    sal_uInt16          GetFlags() const
                        { return GetInfo().nFlags; }
    sal_Bool                CanGetFocus() const;
    sal_uInt16              GetPosition();
    sal_uInt16              GetType()
                        { return nType; }

    void                CreateContext( sal_uInt16 nContextId, SfxBindings& );
    sal_uInt16              GetContextId() const
                        { return pContext ? pContext->GetContextId(): 0; }

    ::Window*           GetContextWindow() const
                        { return pContext ? pContext->GetWindow(): 0; }

    ::Window*           GetContextWindow( SfxModule *pModule ) const;

    virtual SfxChildWinInfo GetInfo() const;
    void                SaveStatus(const SfxChildWinInfo& rInfo);

    static void         RegisterChildWindow(SfxModule*, SfxChildWinFactory*);

    static SfxChildWindow* CreateChildWindow( sal_uInt16, ::Window*, SfxBindings*, SfxChildWinInfo&);
    void                SetHideNotDelete( sal_Bool bOn );
    sal_Bool                IsHideNotDelete() const;
    sal_Bool                IsHideAtToggle() const;
    sal_Bool                IsVisible() const;
    void                SetWantsFocus( sal_Bool );
    sal_Bool            WantsFocus() const;

    virtual sal_Bool    QueryClose();
    virtual com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >           GetFrame();
    void                SetFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & );

    SAL_DLLPRIVATE static void InitializeChildWinFactory_Impl(sal_uInt16, SfxChildWinInfo&);
    SAL_DLLPRIVATE void SetVisible_Impl( sal_Bool bVis );
    SAL_DLLPRIVATE void SetWorkWindow_Impl( SfxWorkWindow* );
    SAL_DLLPRIVATE void Activate_Impl();
    SAL_DLLPRIVATE void Deactivate_Impl();

    SAL_DLLPRIVATE SfxChildWindowContext*
                        GetContext_Impl() const
                        { return pContext; }
    SAL_DLLPRIVATE void SetFactory_Impl( SfxChildWinFactory* );
};

//------------------------------------------------------------------
//! soon obsolete !
#define SFX_DECL_CHILDWINDOW_CONTEXT(Class) \
        static  SfxChildWindowContext* CreateImpl(::Window *pParent, \
                    SfxBindings *pBindings, SfxChildWinInfo* pInfo ); \
        static  void RegisterChildWindowContext(SfxModule *pMod=0); \

//! The Macro of the future ...
#define SFX_DECL_CHILDWINDOWCONTEXT(Class) \
        static  SfxChildWindowContext* CreateImpl(::Window *pParent, \
                    SfxBindings *pBindings, SfxChildWinInfo* pInfo ); \
        static  void RegisterChildWindowContext(sal_uInt16, SfxModule *pMod=0); \

//! soon obsolete !
#define SFX_IMPL_CHILDWINDOW_CONTEXT(Class, MyID, ShellClass) \
        SfxChildWindowContext* Class::CreateImpl( ::Window *pParent, \
                SfxBindings *pBindings, SfxChildWinInfo* pInfo ) \
        {   \
            SfxChildWindowContext *pContext = new Class(pParent, \
                    /* cast is safe here! */static_cast< sal_uInt16 >(ShellClass::GetInterfaceId()), \
                    pBindings,pInfo); \
            return pContext; \
        } \
        void    Class::RegisterChildWindowContext(SfxModule* pMod)   \
        {   \
            SfxChildWinContextFactory *pFact = new SfxChildWinContextFactory( \
                Class::CreateImpl, \
                /* cast is safe here! */static_cast< sal_uInt16 >(ShellClass::GetInterfaceId()) );   \
            SfxChildWindowContext::RegisterChildWindowContext(pMod, MyID, pFact); \
        }

//! The Macro of the future ...
// As a parameter and because of ContextId, CreateImpl must be handed the
// factory. As long as Id is set to 0 and patched in
// SfxChildWindow::CreateContext
#define SFX_IMPL_CHILDWINDOWCONTEXT(Class, MyID) \
        SfxChildWindowContext* Class::CreateImpl( ::Window *pParent, \
                SfxBindings *pBindings, SfxChildWinInfo* pInfo ) \
        {   \
            SfxChildWindowContext *pContext = new Class(pParent,0,pBindings,pInfo);\
            return pContext; \
        } \
        void    Class::RegisterChildWindowContext(sal_uInt16 nId, SfxModule* pMod)   \
        {   \
            SfxChildWinContextFactory *pFact = new SfxChildWinContextFactory( \
                Class::CreateImpl, nId );   \
            SfxChildWindowContext::RegisterChildWindowContext(pMod, MyID, pFact); \
        }

#define SFX_DECL_CHILDWINDOW(Class) \
    public  :   \
        static  SfxChildWindow* CreateImpl(::Window *pParent, sal_uInt16 nId, \
                    SfxBindings *pBindings, SfxChildWinInfo* pInfo ); \
        static  void RegisterChildWindow (sal_Bool bVisible=sal_False, SfxModule *pMod=NULL, sal_uInt16 nFlags=0); \
        static  sal_uInt16 GetChildWindowId ();\
        virtual SfxChildWinInfo GetInfo() const

#define SFX_IMPL_CHILDWINDOW(Class, MyID) \
        SFX_IMPL_POS_CHILDWINDOW(Class, MyID, CHILDWIN_NOPOS)

#define SFX_IMPL_POS_CHILDWINDOW(Class, MyID, Pos) \
        SfxChildWindow* Class::CreateImpl( ::Window *pParent, \
                sal_uInt16 nId, SfxBindings *pBindings, SfxChildWinInfo* pInfo ) \
                {   \
                    SfxChildWindow *pWin = new Class(pParent, nId, pBindings, pInfo);\
                    return pWin; \
                } \
        sal_uInt16 Class::GetChildWindowId () \
                { return MyID; } \
        void    Class::RegisterChildWindow (sal_Bool bVis, SfxModule *pMod, sal_uInt16 nFlags)   \
                {   \
                    SfxChildWinFactory *pFact = new SfxChildWinFactory( \
                        Class::CreateImpl, MyID, Pos );   \
                    pFact->aInfo.nFlags |= nFlags;  \
                    pFact->aInfo.bVisible = bVis;         \
                    SfxChildWindow::RegisterChildWindow(pMod, pFact); \
                }

#define SFX_IMPL_FLOATINGWINDOW(Class, MyID)    \
        SFX_IMPL_CHILDWINDOW(Class, MyID)       \
        SfxChildWinInfo Class::GetInfo() const \
        {                                       \
            SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();     \
            ((SfxFloatingWindow*)GetWindow())->FillInfo( aInfo );  \
            return aInfo; }

#define SFX_IMPL_MODELESSDIALOG(Class, MyID)    \
        SFX_IMPL_CHILDWINDOW(Class, MyID)       \
        SfxChildWinInfo Class::GetInfo() const \
        {                                       \
            SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();     \
            ((SfxModelessDialog*)GetWindow())->FillInfo( aInfo );  \
            return aInfo; }

#define SFX_IMPL_DOCKINGWINDOW(Class, MyID) \
        SFX_IMPL_CHILDWINDOW(Class, MyID)       \
        SfxChildWinInfo Class::GetInfo() const \
        {                                       \
            SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();     \
            ((SfxDockingWindow*)GetWindow())->FillInfo( aInfo );  \
            return aInfo; }

#define SFX_IMPL_TOOLBOX(Class, MyID)   \
        SFX_IMPL_CHILDWINDOW(Class, MyID)       \
        SfxChildWinInfo Class::GetInfo() const \
        {                                       \
            SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();     \
            ((SfxToolbox*)GetWindow())->FillInfo( aInfo );  \
            return aInfo; }

//------------------------------------------------------------------

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
