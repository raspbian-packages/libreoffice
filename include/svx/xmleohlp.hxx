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

#ifndef INCLUDED_SVX_XMLEOHLP_HXX
#define INCLUDED_SVX_XMLEOHLP_HXX

#include <cppuhelper/compbase2.hxx>
#include <osl/mutex.hxx>
#include <sot/storage.hxx>
#include <map>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <svx/svxdllapi.h>


// - SvXMLEmbeddedObjectHelper -


enum SvXMLEmbeddedObjectHelperMode
{
    EMBEDDEDOBJECTHELPER_MODE_READ = 0,
    EMBEDDEDOBJECTHELPER_MODE_WRITE = 1
};


// - SvXMLEmbeddedObjectHelper -


namespace comphelper {
class IEmbeddedHelper;
}
class SvGlobalName;
struct OUStringLess;
class OutputStorageWrapper_Impl;

class SVX_DLLPUBLIC SvXMLEmbeddedObjectHelper : public ::cppu::WeakComponentImplHelper2<
    ::com::sun::star::document::XEmbeddedObjectResolver,
    ::com::sun::star::container::XNameAccess >
{
    typedef ::std::map< OUString, OutputStorageWrapper_Impl*,
                         OUStringLess > SvXMLEmbeddedObjectHelper_Impl;
private:

    ::osl::Mutex                maMutex;

    const OUString       maReplacementGraphicsContainerStorageName;
    const OUString       maReplacementGraphicsContainerStorageName60;
    OUString             maCurContainerStorageName;


    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > mxRootStorage;  // package
    ::comphelper::IEmbeddedHelper*             mpDocPersist;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > mxContainerStorage; // container sub package for
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > mxTempStorage;  // package
                                                // objects
    SvXMLEmbeddedObjectHelperMode       meCreateMode;
    SvXMLEmbeddedObjectHelper_Impl      *mpStreamMap;

    SVX_DLLPRIVATE bool                 ImplGetStorageNames(
                                       const OUString& rURLStr,
                                       OUString& rContainerStorageName,
                                       OUString& rObjectStorageName,
                                       bool bInternalToExternal,
                                       bool *pGraphicRepl=0,
                                       bool *pOasisFormat=0 ) const;

    SVX_DLLPRIVATE com::sun::star::uno::Reference < com::sun::star::embed::XStorage > ImplGetContainerStorage(
                                    const OUString& rStorageName );

    SVX_DLLPRIVATE bool                 ImplReadObject(
                                    const OUString& rContainerStorageName,
                                    OUString& rObjName,
                                    const SvGlobalName *pClassId,
                                    SvStream* pTemp );

    SVX_DLLPRIVATE OUString              ImplInsertEmbeddedObjectURL(
                                    const OUString& rURLStr );

    SVX_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > ImplGetReplacementImage(
                                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject >& xObj );

protected:

                                SvXMLEmbeddedObjectHelper();
                                virtual ~SvXMLEmbeddedObjectHelper();
    void                        Init( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&,
                                      ::comphelper::IEmbeddedHelper& rDocPersist,
                                      SvXMLEmbeddedObjectHelperMode eCreateMode );

    virtual void SAL_CALL       disposing() SAL_OVERRIDE;

public:
                                SvXMLEmbeddedObjectHelper(
                                    ::comphelper::IEmbeddedHelper& rDocPersist,
                                    SvXMLEmbeddedObjectHelperMode eCreateMode );

    static SvXMLEmbeddedObjectHelper*   Create(
                                    const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&,
                                    ::comphelper::IEmbeddedHelper& rDocPersist,
                                    SvXMLEmbeddedObjectHelperMode eCreateMode,
                                    bool bDirect = true );
    static SvXMLEmbeddedObjectHelper*   Create(
                                    ::comphelper::IEmbeddedHelper& rDocPersist,
                                    SvXMLEmbeddedObjectHelperMode eCreateMode );
    static void                 Destroy( SvXMLEmbeddedObjectHelper* pSvXMLEmbeddedObjectHelper );

    void                        Flush();

    // XEmbeddedObjectResolver
    virtual OUString SAL_CALL resolveEmbeddedObjectURL( const OUString& aURL ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XNameAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    static void splitObjectURL(const OUString& aURLNoPar,
        OUString& rContainerStorageName,
        OUString& rObjectStorageName);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
