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

#ifndef INCLUDED_SVX_GALTHEME_HXX
#define INCLUDED_SVX_GALTHEME_HXX

#include <svx/svxdllapi.h>

#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <vcl/salctype.hxx>
#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>
#include <svtools/transfer.hxx>
#include <sot/storage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/galmisc.hxx>
#include <vector>


// - GalleryObject -


// - SgaObjKind -


struct GalleryObject
{
    INetURLObject   aURL;
    sal_uInt32      nOffset;
    SgaObjKind      eObjKind;
    bool            mbDelete;

    //UI visualization buffering
    BitmapEx        maPreviewBitmapEx;
    Size            maPreparedSize;
    OUString        maTitle;
    OUString        maPath;
};

typedef ::std::vector< GalleryObject* > GalleryObjectList;

class GalleryThemeEntry;
class SgaObject;
class FmFormModel;
class ListBox;


// - GalDragParams -


struct GalDragParams
{
    Region  aDragRegion;
    sal_uIntPtr nDragObjPos;
    OUString aThemeName;
    OUString aFileName;
    SgaObjKind  eObjKind;
};


// - GalleryTheme -


class Gallery;
class GalleryProgress;
namespace unogallery
{
    class GalleryTheme;
    class GalleryItem;
}

class GalleryTheme : public SfxBroadcaster
{
    friend class Gallery;
    friend class GalleryThemeCacheEntry;
    friend class ::unogallery::GalleryTheme;
    friend class ::unogallery::GalleryItem;

private:

    GalleryObjectList           aObjectList;
    OUString                    m_aDestDir;
    bool                        m_bDestDirRelative;
    SotStorageRef               aSvDrawStorageRef;
    Gallery*                    pParent;
    GalleryThemeEntry*          pThm;
    sal_uIntPtr                 mnThemeLockCount;
    sal_uIntPtr                 mnBroadcasterLockCount;
    sal_uIntPtr                 nDragPos;
    bool                        bDragging;
    bool                        bAbortActualize;

    void                        ImplCreateSvDrawStorage();
    SVX_DLLPUBLIC SgaObject*                    ImplReadSgaObject( GalleryObject* pEntry );
    bool                        ImplWriteSgaObject( const SgaObject& rObj, size_t nPos, GalleryObject* pExistentEntry );
    void                        ImplWrite();
    const GalleryObject*        ImplGetGalleryObject( size_t nPos ) const
                                { return ( nPos < aObjectList.size() ) ? aObjectList[ nPos ] : NULL; }
    SVX_DLLPUBLIC const GalleryObject*      ImplGetGalleryObject( const INetURLObject& rURL );

    size_t                      ImplGetGalleryObjectPos( const GalleryObject* pObj ) const
                                {
                                    for ( size_t i = 0, n = aObjectList.size(); i < n; ++i )
                                        if ( pObj == aObjectList[ i ] )
                                            return i;
                                    return size_t(-1);
                                }
    INetURLObject               ImplGetURL( const GalleryObject* pObject ) const;
    INetURLObject               ImplCreateUniqueURL( SgaObjKind eObjKind, sal_uIntPtr nFormat = CVT_UNKNOWN );
    void                        ImplSetModified( bool bModified );
    void                        ImplBroadcast( sal_uIntPtr nUpdatePos );

                                GalleryTheme();
                                GalleryTheme( Gallery* pGallery, GalleryThemeEntry* pThemeEntry );
                                virtual ~GalleryTheme();

public:

    static GalleryThemeEntry*   CreateThemeEntry( const INetURLObject& rURL, bool bReadOnly );

    size_t                      GetObjectCount() const { return aObjectList.size(); }

    SVX_DLLPUBLIC SgaObject*    AcquireObject( size_t nPos );
    SVX_DLLPUBLIC void          ReleaseObject( SgaObject* pObj );

    SVX_DLLPUBLIC bool          InsertObject( const SgaObject& rObj, sal_uIntPtr nPos = CONTAINER_APPEND );
    SVX_DLLPUBLIC bool          RemoveObject( size_t nPos );
    bool                        ChangeObjectPos( size_t nOldPos, size_t nNewPos );

    SVX_DLLPUBLIC const OUString& GetName() const;
    const OUString&        GetRealName() const;

    // used for building gallery themes during compilation:
    const OUString&             GetDestDir() const { return m_aDestDir; }
    void                        SetDestDir(const OUString& rDestDir, bool bRelative = true)
                                { m_aDestDir = rDestDir; m_bDestDirRelative = bRelative; }

    const INetURLObject&        GetThmURL() const;
    SVX_DLLPUBLIC const INetURLObject&      GetSdgURL() const;
    const INetURLObject&        GetSdvURL() const;

    SVX_DLLPUBLIC sal_uInt32    GetId() const;
    void                        SetId( sal_uInt32 nNewId, bool bResetThemeName );

    void                        SetDragging( bool bSet ) { bDragging = bSet; }
    bool                        IsDragging() const { return bDragging; }

    void                        LockTheme() { ++mnThemeLockCount; }
    bool                        UnlockTheme();

    void                        LockBroadcaster() { mnBroadcasterLockCount++; }
    SVX_DLLPUBLIC void          UnlockBroadcaster( sal_uIntPtr nUpdatePos = 0 );
    bool                        IsBroadcasterLocked() const { return mnBroadcasterLockCount > 0; }

    void                        SetDragPos( sal_uIntPtr nPos ) { nDragPos = nPos; }
    sal_uIntPtr                     GetDragPos() const { return nDragPos; }

    bool                        IsThemeNameFromResource() const;

    SVX_DLLPUBLIC bool          IsReadOnly() const;
    SVX_DLLPUBLIC bool          IsDefault() const;
    bool                        IsModified() const;

    SVX_DLLPUBLIC void                      Actualize( const Link& rActualizeLink, GalleryProgress* pProgress = NULL );
    void                        AbortActualize() { bAbortActualize = true; }

    Gallery*                    GetParent() const { return pParent; }
    SotStorageRef               GetSvDrawStorage() const { return aSvDrawStorageRef; }

public:

    SgaObjKind                  GetObjectKind( sal_uIntPtr nPos ) const
                                {
                                    DBG_ASSERT( nPos < GetObjectCount(), "Position out of range" );
                                    return ImplGetGalleryObject( nPos )->eObjKind;
                                }


    const INetURLObject&        GetObjectURL( sal_uIntPtr nPos ) const
                                {
                                    DBG_ASSERT( nPos < GetObjectCount(), "Position out of range" );
                                    return ImplGetGalleryObject( nPos )->aURL;
                                }

    bool                        GetThumb( sal_uIntPtr nPos, BitmapEx& rBmp, bool bProgress = false );

    SVX_DLLPUBLIC bool         GetGraphic( sal_uIntPtr nPos, Graphic& rGraphic, bool bProgress = false );
    SVX_DLLPUBLIC bool         InsertGraphic( const Graphic& rGraphic, sal_uIntPtr nInsertPos = CONTAINER_APPEND );

    SVX_DLLPUBLIC bool          GetModel( sal_uIntPtr nPos, SdrModel& rModel, bool bProgress = false );
    SVX_DLLPUBLIC bool          InsertModel( const FmFormModel& rModel, sal_uIntPtr nInsertPos = CONTAINER_APPEND );

    bool                        GetModelStream( sal_uIntPtr nPos, SotStorageStreamRef& rModelStreamRef, bool bProgress = false );
    bool                        InsertModelStream( const SotStorageStreamRef& rModelStream, sal_uIntPtr nInsertPos = CONTAINER_APPEND );

    bool                        GetURL( sal_uIntPtr nPos, INetURLObject& rURL, bool bProgress = false );
    SVX_DLLPUBLIC bool          InsertURL( const INetURLObject& rURL, sal_uIntPtr nInsertPos = CONTAINER_APPEND );
    bool                        InsertFileOrDirURL( const INetURLObject& rFileOrDirURL, sal_uIntPtr nInsertPos = CONTAINER_APPEND );

    bool                        InsertTransferable( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& rxTransferable, sal_uIntPtr nInsertPos );

    void                        CopyToClipboard( Window* pWindow, sal_uIntPtr nPos );
    void                        StartDrag( Window* pWindow, sal_uIntPtr nPos );

public:

    SvStream&                   WriteData( SvStream& rOut ) const;
    SvStream&                   ReadData( SvStream& rIn );
    static SVX_DLLPUBLIC void   InsertAllThemes( ListBox& rListBox );

    // for buffering PreviewBitmaps and strings for object and path
    void GetPreviewBitmapExAndStrings(sal_uIntPtr nPos, BitmapEx& rBitmapEx, Size& rSize, OUString& rTitle, OUString& rPath) const;
    void SetPreviewBitmapExAndStrings(sal_uIntPtr nPos, const BitmapEx& rBitmapEx, const Size& rSize, const OUString& rTitle, const OUString& rPath);
};

SvStream& WriteGalleryTheme( SvStream& rOut, const GalleryTheme& rTheme );
SvStream& ReadGalleryTheme( SvStream& rIn, GalleryTheme& rTheme );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
