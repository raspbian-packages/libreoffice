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

#ifndef _IOS_CLIPBOARD_HXX_
#define _IOS_CLIPBOARD_HXX_

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <cppuhelper/compbase4.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <boost/utility.hpp>
#include <list>

#include <premac.h>
#import <UIKit/UIKit.h>
#include <postmac.h>

class IosClipboard;

@interface EventListener : NSObject
{
     IosClipboard* pIosClipboard;
}

// Init the pasteboard change listener with a reference to the OfficeClipboard
// instance
- (EventListener*)initWithIosClipboard: (IosClipboard*) pcb;

// Promiss resolver function
- (void)pasteboard:(UIPasteboard*)sender provideDataForType:(NSString *)type;

-(void)applicationDidBecomeActive:(NSNotification*)aNotification;

-(void)disposing;
@end


class IosClipboard : public ::cppu::BaseMutex,
                      public ::cppu::WeakComponentImplHelper4< com::sun::star::datatransfer::clipboard::XClipboardEx,
                                                               com::sun::star::datatransfer::clipboard::XClipboardNotifier,
                                                               com::sun::star::datatransfer::clipboard::XFlushableClipboard,
                                                               com::sun::star::lang::XServiceInfo >,
                      private ::boost::noncopyable
{
public:
  /* Create a clipboard instance.

     @param pasteboard
     If not equal NULL the instance will be instantiated with the provided
     pasteboard reference and 'bUseSystemClipboard' will be ignored

     @param bUseSystemClipboard
     If 'pasteboard' is NULL 'bUseSystemClipboard' determines whether the
     system clipboard will be created (bUseSystemClipboard == true) or if
     the DragPasteboard if bUseSystemClipboard == false
   */
  IosClipboard(UIPasteboard* pasteboard = NULL,
                bool bUseSystemClipboard = true);

  ~IosClipboard();

  //------------------------------------------------
  // XClipboard
  //------------------------------------------------

  virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getContents()
    throw( ::com::sun::star::uno::RuntimeException );

  virtual void SAL_CALL setContents( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTransferable,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
    throw( ::com::sun::star::uno::RuntimeException );

  virtual ::rtl::OUString SAL_CALL getName()
    throw( ::com::sun::star::uno::RuntimeException );

  //------------------------------------------------
  // XClipboardEx
  //------------------------------------------------

  virtual sal_Int8 SAL_CALL getRenderingCapabilities()
    throw( ::com::sun::star::uno::RuntimeException );

  //------------------------------------------------
  // XClipboardNotifier
  //------------------------------------------------

  virtual void SAL_CALL addClipboardListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
    throw( ::com::sun::star::uno::RuntimeException );

  virtual void SAL_CALL removeClipboardListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
    throw( ::com::sun::star::uno::RuntimeException );

  //------------------------------------------------
  // XFlushableClipboard
  //------------------------------------------------

  virtual void SAL_CALL flushClipboard( ) throw( com::sun::star::uno::RuntimeException );

  //------------------------------------------------
  // XServiceInfo
  //------------------------------------------------

  virtual ::rtl::OUString SAL_CALL getImplementationName()
    throw(::com::sun::star::uno::RuntimeException);

  virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
    throw(::com::sun::star::uno::RuntimeException);

  virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
    throw(::com::sun::star::uno::RuntimeException);

  /* Get a reference to the used pasteboard.
   */
  UIPasteboard* getPasteboard() const;

  /* Notify the current clipboard owner that he is no longer the clipboard owner.
   */
  void fireLostClipboardOwnershipEvent(::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner> oldOwner,
                                       ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > oldContent);

  void pasteboardChangedOwner();

  void provideDataForType(UIPasteboard* sender, NSString* type);

  void applicationDidBecomeActive(NSNotification* aNotification);

private:

  /* Notify all registered XClipboardListener that the clipboard content
     has changed.
  */
  void fireClipboardChangedEvent();

private:
  ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XMimeContentTypeFactory > mrXMimeCntFactory;
  ::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener > > mClipboardListeners;
  ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > mXClipboardContent;
  com::sun::star::uno::Reference< com::sun::star::datatransfer::clipboard::XClipboardOwner > mXClipboardOwner;
  bool mIsSystemPasteboard;
  UIPasteboard* mPasteboard;
  int mPasteboardChangeCount;
  EventListener* mEventListener;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
