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
#ifndef _BASTYPES_HXX
#define _BASTYPES_HXX

#include "scriptdocument.hxx"

#include <iderid.hxx>
#include <svtools/tabbar.hxx>
#include <vcl/toolbox.hxx>

#include <boost/unordered_map.hpp>

class SbModule;
class SfxItemSet;
class SfxRequest;

#define LINE_SEP_CR     0x0D
#define LINE_SEP        0x0A

// Implementation: baside2b.cxx
sal_Int32 searchEOL( const ::rtl::OUString& rStr, sal_Int32 fromIndex );

// Meaning of bToBeKilled:
// While being in a reschedule-loop, I may not destroy the window.
// It must first break from the reschedule-loop to self-destroy then.
// Does unfortunately not work that way: Destroying Window with living Child!

struct BasicStatus
{
    bool bIsRunning : 1;
    bool bError : 1;
    bool bIsInReschedule : 1;
    sal_uInt16 nBasicFlags;

    BasicStatus():
        bIsRunning(false),
        bError(false),
        bIsInReschedule(false),
        nBasicFlags(0) { }
};

class BasicDockingWindow : public DockingWindow
{
    Rectangle       aFloatingPosAndSize;

protected:
    virtual sal_Bool    Docking( const Point& rPos, Rectangle& rRect );
    virtual void    EndDocking( const Rectangle& rRect, sal_Bool bFloatMode );
    virtual void    ToggleFloatingMode();
    virtual sal_Bool    PrepareToggleFloatingMode();
    virtual void    StartDocking();

public:
    BasicDockingWindow( Window* pParent );
};

// helper class for sorting TabBar
class TabBarSortHelper
{
public:
    sal_uInt16          nPageId;
    String          aPageText;

    bool operator<(const TabBarSortHelper& rComp) const { return (aPageText.CompareIgnoreCaseToAscii( rComp.aPageText ) == COMPARE_LESS); }
};

class BasicIDETabBar : public TabBar
{
protected:
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rCEvt );

    virtual long    AllowRenaming();
    virtual void    EndRenaming();

public:
                    BasicIDETabBar( Window* pParent );

    void            Sort();
};

#define BASWIN_OK               0x00
#define BASWIN_RUNNINGBASIC     0x01
#define BASWIN_TOBEKILLED       0x02
#define BASWIN_SUSPENDED        0x04
#define BASWIN_INRESCHEDULE     0x08

class Printer;
class BasicEntryDescriptor;

namespace svl
{
    class IUndoManager;
}

class IDEBaseWindow : public Window
{
private:
    ScrollBar*      pShellHScrollBar;
    ScrollBar*      pShellVScrollBar;

    DECL_LINK( ScrollHdl, ScrollBar * );
    sal_uInt8           nStatus;

    ScriptDocument      m_aDocument;
    String              m_aLibName;
    String              m_aName;

protected:
    virtual void    DoScroll( ScrollBar* pCurScrollBar );

public:
                    TYPEINFO();
                    IDEBaseWindow( Window* pParent, const ScriptDocument& rDocument, String aLibName, String aName );
    virtual         ~IDEBaseWindow();

    void            Init();
    virtual void    DoInit();
    virtual void    Deactivating();
    void            GrabScrollBars( ScrollBar* pHScroll, ScrollBar* pVScroll );

    ScrollBar*      GetHScrollBar() const { return pShellHScrollBar; }
    ScrollBar*      GetVScrollBar() const { return pShellVScrollBar; }

    virtual void    ExecuteCommand( SfxRequest& rReq );
    virtual void    GetState( SfxItemSet& );
    virtual long    Notify( NotifyEvent& rNEvt );

    virtual void    StoreData();
    virtual void    UpdateData();
    virtual sal_Bool    CanClose();

    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter ) = 0;
    // print page
    virtual void printPage( sal_Int32 nPage, Printer* pPrinter ) = 0;

    virtual String  GetTitle();
    String          CreateQualifiedName();
    virtual BasicEntryDescriptor CreateEntryDescriptor() = 0;

    virtual sal_Bool    IsModified();
    virtual sal_Bool    IsPasteAllowed();

    virtual sal_Bool    AllowUndo();

    virtual void    SetReadOnly( sal_Bool bReadOnly );
    virtual sal_Bool    IsReadOnly();

    sal_uInt8           GetStatus()             { return nStatus; }
    void            SetStatus( sal_uInt8 n )        { nStatus = n; }
    void            AddStatus( sal_uInt8 n )        { nStatus = nStatus | n; }
    void            ClearStatus( sal_uInt8 n )  { nStatus = nStatus & ~n; }

    virtual Window* GetLayoutWindow();

    virtual ::svl::IUndoManager*
                    GetUndoManager();

    virtual sal_uInt16  GetSearchOptions();

    virtual void    BasicStarted();
    virtual void    BasicStopped();

    sal_Bool            IsSuspended() const
                        { return ( nStatus & BASWIN_SUSPENDED ) ? sal_True : sal_False; }

    const ScriptDocument&
                    GetDocument() const { return m_aDocument; }
    void            SetDocument( const ScriptDocument& rDocument ) { m_aDocument = rDocument; }
    bool            IsDocument( const ScriptDocument& rDocument ) const { return rDocument == m_aDocument; }
    const String&   GetLibName() const { return m_aLibName; }
    void            SetLibName( const String& aLibName ) { m_aLibName = aLibName; }
    const String&   GetName() const { return m_aName; }
    void            SetName( const String& aName ) { m_aName = aName; }
};

class LibInfoKey
{
private:
    ScriptDocument      m_aDocument;
    ::rtl::OUString m_aLibName;

public:
    LibInfoKey( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName );
    ~LibInfoKey();

    LibInfoKey( const LibInfoKey& rKey );
    LibInfoKey& operator=( const LibInfoKey& rKey );

    bool operator==( const LibInfoKey& rKey ) const;

    const ScriptDocument&
                    GetDocument() const { return m_aDocument; }
    const ::rtl::OUString& GetLibName() const { return m_aLibName; }
};

class LibInfoItem
{
private:
    ScriptDocument      m_aDocument;
    ::rtl::OUString m_aLibName;
    ::rtl::OUString m_aCurrentName;
    sal_uInt16              m_nCurrentType;

public:
    LibInfoItem( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rCurrentName, sal_uInt16 nCurrentType );
    ~LibInfoItem();

    LibInfoItem( const LibInfoItem& rItem );
    LibInfoItem& operator=( const LibInfoItem& rItem );

    const ScriptDocument&
                    GetDocument() const { return m_aDocument; }
    const ::rtl::OUString& GetLibName() const { return m_aLibName; }
    const ::rtl::OUString& GetCurrentName() const { return m_aCurrentName; }
    sal_uInt16          GetCurrentType() const { return m_nCurrentType; }
};

class LibInfos
{
private:

    struct LibInfoKeyHash
    {
        size_t operator()( const LibInfoKey& rKey ) const
        {
            size_t nHash = (size_t) rKey.GetDocument().hashCode();
            nHash += (size_t) ::rtl::OUString( rKey.GetLibName() ).hashCode();
            return nHash;
        }
    };

    typedef ::boost::unordered_map< LibInfoKey, LibInfoItem*, LibInfoKeyHash, ::std::equal_to< LibInfoKey > > LibInfoMap;
    LibInfoMap  m_aLibInfoMap;

public:
                    LibInfos();
                    ~LibInfos();

    void            InsertInfo( LibInfoItem* pItem );
    void            RemoveInfoFor( const ScriptDocument& _rDocument );

    LibInfoItem*    GetInfo( const LibInfoKey& rKey );
};

void            CutLines( ::rtl::OUString& rStr, sal_Int32 nStartLine, sal_Int32 nLines, sal_Bool bEraseTrailingEmptyLines = sal_False );
String          CreateMgrAndLibStr( const String& rMgrName, const String& rLibName );
sal_uLong           CalcLineCount( SvStream& rStream );

sal_Bool            QueryReplaceMacro( const String& rName, Window* pParent = 0 );
sal_Bool            QueryDelMacro( const String& rName, Window* pParent = 0 );
sal_Bool            QueryDelDialog( const String& rName, Window* pParent = 0 );
sal_Bool            QueryDelModule( const String& rName, Window* pParent = 0 );
sal_Bool            QueryDelLib( const String& rName, sal_Bool bRef = sal_False, Window* pParent = 0 );
sal_Bool            QueryPassword( const ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >& xLibContainer, const String& rLibName, String& rPassword, sal_Bool bRepeat = sal_False, sal_Bool bNewTitle = sal_False );

class ModuleInfoHelper
{
ModuleInfoHelper();
ModuleInfoHelper(const ModuleInfoHelper&);
ModuleInfoHelper& operator = (const ModuleInfoHelper&);
public:
    static void getObjectName( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rLib, const ::rtl::OUString& rModName, ::rtl::OUString& rObjName );
    static sal_Int32 getModuleType(  const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rLib, const ::rtl::OUString& rModName );
};
#endif  // _BASTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
