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

#include <sal/types.h>
#include <osl/diagnose.h>
#include <osl/module.h>
#include <osl/thread.h>
#include <osl/process.h>
#include <osl/file.h>

#include "system.h"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#ifdef AIX
#include <sys/ldr.h>
#endif

#ifdef ANDROID
#include <lo-bootstrap.h>
#endif

/* implemented in file.c */
extern int UnicodeToText(char *, size_t, const sal_Unicode *, sal_Int32);

static sal_Bool getModulePathFromAddress(void * address, rtl_String ** path) {
    sal_Bool result = sal_False;
/* Bah, we do want to use dladdr here also on iOS, I think? */
#if !defined(NO_DL_FUNCTIONS) || defined(IOS)
#if defined(AIX)
    int i;
    int size = 4 * 1024;
    char *buf, *filename=NULL;
    struct ld_info *lp;

    if ((buf = malloc(size)) == NULL)
        return result;

    while((i = loadquery(L_GETINFO, buf, size)) == -1 && errno == ENOMEM)
    {
        size += 4 * 1024;
        if ((buf = malloc(size)) == NULL)
            break;
    }

    lp = (struct ld_info*) buf;
    while (lp)
    {
        unsigned long start = (unsigned long)lp->ldinfo_dataorg;
        unsigned long end = start + lp->ldinfo_datasize;
        if (start <= (unsigned long)address && end > (unsigned long)address)
        {
            filename = lp->ldinfo_filename;
            break;
        }
        if (!lp->ldinfo_next)
            break;
        lp = (struct ld_info*) ((char *) lp + lp->ldinfo_next);
    }

    if (filename)
    {
        rtl_string_newFromStr(path, filename);
        result = sal_True;
    }
    else
    {
        result = sal_False;
    }

    free(buf);
#else
    Dl_info dl_info;

#ifdef ANDROID
    result = lo_dladdr(address, &dl_info);
#else
    result = dladdr(address, &dl_info);
#endif

    if (result != 0)
    {
        rtl_string_newFromStr(path, dl_info.dli_fname);
#ifdef ANDROID
        free((void *) dl_info.dli_fname);
#endif
        result = sal_True;
    }
    else
    {
        result = sal_False;
    }
#endif
#endif
    return result;
}

/*****************************************************************************/
/* osl_loadModule */
/*****************************************************************************/

oslModule SAL_CALL osl_loadModule(rtl_uString *ustrModuleName, sal_Int32 nRtldMode)
{
    oslModule pModule=0;
    rtl_uString* ustrTmp = NULL;

    OSL_ENSURE(ustrModuleName,"osl_loadModule : string is not valid");

    /* ensure ustrTmp hold valid string */
    if (osl_File_E_None != osl_getSystemPathFromFileURL(ustrModuleName, &ustrTmp))
        rtl_uString_assign(&ustrTmp, ustrModuleName);

    if (ustrTmp)
    {
        char buffer[PATH_MAX];

        if (UnicodeToText(buffer, PATH_MAX, ustrTmp->buffer, ustrTmp->length))
            pModule = osl_loadModuleAscii(buffer, nRtldMode);
        rtl_uString_release(ustrTmp);
    }

    return pModule;
}

/*****************************************************************************/
/* osl_loadModuleAscii */
/*****************************************************************************/

oslModule SAL_CALL osl_loadModuleAscii(const sal_Char *pModuleName, sal_Int32 nRtldMode)
{
    OSL_ASSERT(
        (nRtldMode & SAL_LOADMODULE_LAZY) == 0 ||
        (nRtldMode & SAL_LOADMODULE_NOW) == 0); /* only either LAZY or NOW */
    if (pModuleName)
    {
#ifndef NO_DL_FUNCTIONS
#ifdef ANDROID
        void *(*lo_dlopen)(const char *) = dlsym(RTLD_DEFAULT, "lo_dlopen");
        void *pLib;
        (void) nRtldMode;
        OSL_ASSERT(lo_dlopen != NULL);
        pLib = (*lo_dlopen)(pModuleName);
#else
        int rtld_mode =
            ((nRtldMode & SAL_LOADMODULE_NOW) ? RTLD_NOW : RTLD_LAZY) |
            ((nRtldMode & SAL_LOADMODULE_GLOBAL) ? RTLD_GLOBAL : RTLD_LOCAL);
        void* pLib = dlopen(pModuleName, rtld_mode);
#endif
#if OSL_DEBUG_LEVEL > 1
        if (pLib == 0)
            OSL_TRACE("Error osl_loadModule: %s", dlerror());
#endif /* OSL_DEBUG_LEVEL */

        return ((oslModule)(pLib));

#else   /* NO_DL_FUNCTIONS */
        printf("No DL Functions\n");
#endif  /* NO_DL_FUNCTIONS */
    }
    return NULL;
}

oslModule osl_loadModuleRelativeAscii(
    oslGenericFunction baseModule, char const * relativePath, sal_Int32 mode)
{
    OSL_ASSERT(relativePath != NULL);
    if (relativePath[0] == '/') {
        return osl_loadModuleAscii(relativePath, mode);
    } else {
        rtl_String * path = NULL;
        rtl_String * suffix = NULL;
        oslModule module;
        if (!getModulePathFromAddress(baseModule, &path)) {
            return NULL;
        }
        rtl_string_newFromStr_WithLength(
            &path, path->buffer,
            (rtl_str_lastIndexOfChar_WithLength(path->buffer, path->length, '/')
             + 1));
            /* cut off everything after the last slash; should the original path
               contain no slash, the resulting path is the empty string */
        rtl_string_newFromStr(&suffix, relativePath);
        rtl_string_newConcat(&path, path, suffix);
        rtl_string_release(suffix);
        module = osl_loadModuleAscii(path->buffer, mode);
        rtl_string_release(path);
        return module;
    }
}

/*****************************************************************************/
/* osl_getModuleHandle */
/*****************************************************************************/

sal_Bool SAL_CALL
osl_getModuleHandle(rtl_uString *pModuleName, oslModule *pResult)
{
    (void) pModuleName; /* avoid warning about unused parameter */
#if !defined(NO_DL_FUNCTIONS) || defined(IOS)
    *pResult = (oslModule) RTLD_DEFAULT;
#else
    *pResult = NULL;
#endif
    return sal_True;
}

/*****************************************************************************/
/* osl_unloadModule */
/*****************************************************************************/
void SAL_CALL osl_unloadModule(oslModule hModule)
{
    if (hModule)
    {
#ifndef NO_DL_FUNCTIONS
        int nRet = dlclose(hModule);

#if OSL_DEBUG_LEVEL > 1
        if (nRet != 0)
        {
            fprintf(stderr, "Error osl_unloadModule: %s\n", dlerror());
        }
#else
        (void) nRet;
#endif /* if OSL_DEBUG_LEVEL */

#endif /* ifndef NO_DL_FUNCTIONS */
    }
}

/*****************************************************************************/
/* osl_getSymbol */
/*****************************************************************************/
void* SAL_CALL
osl_getSymbol(oslModule Module, rtl_uString* pSymbolName)
{
    return (void *) osl_getFunctionSymbol(Module, pSymbolName);
}


/*****************************************************************************/
/* osl_getAsciiFunctionSymbol */
/*****************************************************************************/
oslGenericFunction SAL_CALL
osl_getAsciiFunctionSymbol(oslModule Module, const sal_Char *pSymbol)
{
    void *fcnAddr = NULL;

/* We do want to use dlsym on iOS */
#if !defined(NO_DL_FUNCTIONS) || defined(IOS)
    if (pSymbol)
    {
        fcnAddr = dlsym(Module, pSymbol);

        if (!fcnAddr)
            OSL_TRACE("Error osl_getAsciiFunctionSymbol: %s", dlerror());
    }
#endif

    return (oslGenericFunction) fcnAddr;
}

/*****************************************************************************/
/* osl_getFunctionSymbol */
/*****************************************************************************/
oslGenericFunction SAL_CALL
osl_getFunctionSymbol(oslModule module, rtl_uString *puFunctionSymbolName)
{
    oslGenericFunction pSymbol = NULL;

    if( puFunctionSymbolName )
    {
        rtl_String* pSymbolName = NULL;

        rtl_uString2String( &pSymbolName,
            rtl_uString_getStr(puFunctionSymbolName),
            rtl_uString_getLength(puFunctionSymbolName),
            RTL_TEXTENCODING_UTF8,
            OUSTRING_TO_OSTRING_CVTFLAGS );

        if( pSymbolName != NULL )
        {
            pSymbol = osl_getAsciiFunctionSymbol(module, rtl_string_getStr(pSymbolName));
            rtl_string_release(pSymbolName);
        }
    }

    return pSymbol;
}

/*****************************************************************************/
/* osl_getModuleURLFromAddress */
/*****************************************************************************/
sal_Bool SAL_CALL osl_getModuleURLFromAddress(void * addr, rtl_uString ** ppLibraryUrl)
{
    sal_Bool result = sal_False;
    rtl_String * path = NULL;
    if (getModulePathFromAddress(addr, &path))
    {
        rtl_uString * workDir = NULL;
        osl_getProcessWorkingDir(&workDir);
        if (workDir)
        {
#if OSL_DEBUG_LEVEL > 1
            OSL_TRACE("module.c::osl_getModuleURLFromAddress - %s", path->buffer);
#endif
            rtl_string2UString(ppLibraryUrl,
                               path->buffer,
                               path->length,
                               osl_getThreadTextEncoding(),
                               OSTRING_TO_OUSTRING_CVTFLAGS);

            OSL_ASSERT(*ppLibraryUrl != NULL);
            osl_getFileURLFromSystemPath(*ppLibraryUrl, ppLibraryUrl);
            osl_getAbsoluteFileURL(workDir, *ppLibraryUrl, ppLibraryUrl);

            rtl_uString_release(workDir);
            result = sal_True;
        }
        else
        {
            result = sal_False;
        }
        rtl_string_release(path);
    }
    return result;
}

/*****************************************************************************/
/* osl_getModuleURLFromFunctionAddress */
/*****************************************************************************/
sal_Bool SAL_CALL osl_getModuleURLFromFunctionAddress(oslGenericFunction addr, rtl_uString ** ppLibraryUrl)
{
    return osl_getModuleURLFromAddress((void*)addr, ppLibraryUrl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
