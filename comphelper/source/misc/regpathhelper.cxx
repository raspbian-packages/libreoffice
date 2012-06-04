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


#include <stdio.h>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <osl/thread.h>
#include <osl/process.h>
#include <rtl/textenc.h>
#include <rtl/uri.h>
#include <rtl/uri.hxx>

using namespace osl;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringToOUString;

#define PATH_DELEMITTER '/'

#define USER_REGISTRY_NAME_ENV      "STAR_USER_REGISTRY"
#define SYSTEM_REGISTRY_NAME_ENV    "STAR_REGISTRY"
#define REGISTRY_SYSTEM_NAME    "services.rdb"

#define REGISTRY_LOCAL_NAME     "user60.rdb"

#ifdef SAL_UNX
#define CONFIG_PATH_PREFIX      "."
#else
#define CONFIG_PATH_PREFIX      ""
#endif

namespace comphelper
{

/**
   @return sal_True, if the office is started in a portal
                     environment.
           sal_False, if the common office is started
 */
static sal_Bool retrievePortalUserDir( OUString *pDirectory )
{
    sal_uInt32 nArgs = osl_getCommandArgCount();
    sal_Bool bIsPortalUser = sal_False;
    OUString sArg;
    while( nArgs > 0 )
    {
        if ( !osl_getCommandArg(--nArgs, &sArg.pData) )
        {
            if ( sArg.indexOf(OUString( RTL_CONSTASCII_USTRINGPARAM( "-userid" ))) == 0 )
            {

                bIsPortalUser = sal_True;
                sal_Int32 nStart = sArg.lastIndexOf( '[' );
                sal_Int32 nEnd   = sArg.lastIndexOf( ']' );
                if( -1 == nStart || -1 == nEnd || nEnd < nStart)
                {
                    *pDirectory = OUString();
                }
                else
                {
                    OUString aEncHome = sArg.copy( nStart + 1 , nEnd - nStart -1 );
                    *pDirectory = rtl::Uri::decode(aEncHome,
                                                   rtl_UriDecodeWithCharset,
                                                   RTL_TEXTENCODING_UTF8);
                }
                  break;
            }
          }
     }
    return bIsPortalUser;
}


static OUString getDefaultLocalRegistry()
{
    OUString uBuffer, userRegistryName;
    OUString portalUserDir;

    sal_Bool bIsPortalUser = retrievePortalUserDir( &portalUserDir );

    if ( bIsPortalUser )
       {
        if(  portalUserDir.getLength() )
        {
            FileBase::getFileURLFromSystemPath( portalUserDir , portalUserDir );
            userRegistryName = portalUserDir;
            userRegistryName += OUString( RTL_CONSTASCII_USTRINGPARAM(
                "/user/" REGISTRY_LOCAL_NAME ) );

            // Directory creation is probably necessary for bootstrapping a new
            // user in the portal environment (the ucb uses this function).
            // This should be solved differently, as
            // no one expects this function to create anything ...
            OUString sSeparator(RTL_CONSTASCII_USTRINGPARAM("/"));
            OUString sPath(RTL_CONSTASCII_USTRINGPARAM("file://"));
            FileBase::RC retRC = FileBase::E_None;

            sal_Int32 nIndex = 3;
            sPath += userRegistryName.getToken(2, '/', nIndex);
            while( nIndex != -1 )
            {
                sPath += sSeparator;
                sPath += userRegistryName.getToken(0, '/', nIndex);
                if( nIndex == -1 )
                    break;
                Directory aDir( sPath );
                if( aDir.open() == FileBase::E_NOENT )
                {
                    retRC = Directory::create(sPath);
                    if ( retRC != FileBase::E_None && retRC != FileBase::E_EXIST)
                    {
                        return OUString();
                    }
                }
            }
        }
    }
    else /* bIsPortalUser */
    {
        ::osl::Security aUserSecurity;
        aUserSecurity.getConfigDir( userRegistryName );
        userRegistryName += OUString( RTL_CONSTASCII_USTRINGPARAM(
            "/" CONFIG_PATH_PREFIX REGISTRY_LOCAL_NAME ) );
    }

    return userRegistryName;
}


OUString getPathToUserRegistry()
{
    OUString    userRegistryName;
    FILE        *f=NULL;

    // search the environment STAR_USER_REGISTRY
    OString sBuffer( getenv(USER_REGISTRY_NAME_ENV) );
    if ( sBuffer.getLength() > 0 )
    {
        f = fopen( sBuffer.getStr(), "r" );

        if (f != NULL)
        {
            fclose(f);
            userRegistryName = OStringToOUString( sBuffer, osl_getThreadTextEncoding() );
        }
    }

    if ( !userRegistryName.getLength() )
    {
        userRegistryName = getDefaultLocalRegistry();
    }

    return userRegistryName;
}

OUString getPathToSystemRegistry()
{
    OUString uBuffer;
    OUString registryBaseName( RTL_CONSTASCII_USTRINGPARAM(REGISTRY_SYSTEM_NAME) );
    OUString systemRegistryName;
    FILE     *f=NULL;

    // search in the directory of the executable
    if(osl_Process_E_None == osl_getExecutableFile(&uBuffer.pData))
    {
        sal_uInt32  lastIndex = uBuffer.lastIndexOf(PATH_DELEMITTER);
        if (lastIndex > 0)
        {
            uBuffer = uBuffer.copy(0, lastIndex + 1);
        }

        uBuffer += registryBaseName;

        if (!FileBase::getSystemPathFromFileURL(uBuffer, systemRegistryName))
        {
            OString tmpStr( OUStringToOString(systemRegistryName, osl_getThreadTextEncoding()) );
            f = fopen( tmpStr.getStr(), "r" );
        }
    }

    if (f == NULL)
    {
        // search the environment STAR_REGISTRY
        OString tmpStr( getenv(SYSTEM_REGISTRY_NAME_ENV) );
        if ( tmpStr.getLength() > 0 )
        {
            f = fopen(tmpStr.getStr(), "r");

            if (f != NULL)
            {
                fclose(f);
                systemRegistryName = OStringToOUString( tmpStr, osl_getThreadTextEncoding() );
            } else
            {
                systemRegistryName = OUString();
            }
        }
    } else
    {
        fclose(f);
    }

    return systemRegistryName;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
