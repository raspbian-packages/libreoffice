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

#ifndef _SALGTKFOLDERPICKER_HXX_
#define _SALGTKFOLDERPICKER_HXX_

#include <list>
#include <memory>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase3.hxx>

#include "gtk/fpicker/SalGtkPicker.hxx"

class SalGtkFolderPicker :
        public SalGtkPicker,
    public cppu::WeakImplHelper2<
        ::com::sun::star::ui::dialogs::XFolderPicker,
        ::com::sun::star::util::XCancellable >
{
    public:

        // constructor
        SalGtkFolderPicker( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xServiceMgr );

        //------------------------------------------------------------------------------------
        // XExecutableDialog functions
        //------------------------------------------------------------------------------------

        virtual void SAL_CALL setTitle( const ::rtl::OUString& aTitle )
            throw( ::com::sun::star::uno::RuntimeException );

        virtual sal_Int16 SAL_CALL execute(  )
            throw( ::com::sun::star::uno::RuntimeException );

        //------------------------------------------------------------------------------------
        // XFolderPicker functions
        //------------------------------------------------------------------------------------

        virtual void SAL_CALL setDisplayDirectory( const rtl::OUString& rDirectory )
            throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );

        virtual rtl::OUString SAL_CALL getDisplayDirectory(  )
            throw( com::sun::star::uno::RuntimeException );

        virtual rtl::OUString SAL_CALL getDirectory( )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL setDescription( const rtl::OUString& rDescription )
            throw( com::sun::star::uno::RuntimeException );

        //------------------------------------------------
        // XCancellable
        //------------------------------------------------

        virtual void SAL_CALL cancel( )
            throw( ::com::sun::star::uno::RuntimeException );

        //------------------------------------------------
        // XEventListener
        //------------------------------------------------

        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent )
            throw(::com::sun::star::uno::RuntimeException);

    private:
        // prevent copy and assignment
        SalGtkFolderPicker( const SalGtkFolderPicker& );
        SalGtkFolderPicker& operator=( const SalGtkFolderPicker& );
};

#endif // _SALGTKFOLDERPICKER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
