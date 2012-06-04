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

#undef UNICODE
#undef _UNICODE

#define _WIN32_WINDOWS 0x0410

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <msiquery.h>
#include <shellapi.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>
#include <assert.h>
#include <string.h>

#ifdef UNICODE
#define _UNICODE
#define _tstring    wstring
#else
#define _tstring    string
#endif
#include <tchar.h>
#include <string>

static std::_tstring GetMsiProperty( MSIHANDLE handle, const std::_tstring& sProperty )
{
    std::_tstring result;
    TCHAR szDummy[1] = TEXT("");
    DWORD nChars = 0;

    if ( MsiGetProperty( handle, sProperty.c_str(), szDummy, &nChars ) == ERROR_MORE_DATA )
    {
        DWORD nBytes = ++nChars * sizeof(TCHAR);
        LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca(nBytes));
        ZeroMemory( buffer, nBytes );
        MsiGetProperty(handle, sProperty.c_str(), buffer, &nChars);
        result = buffer;
    }

    return result;
}

/* creates a child process which is specified in lpCommand.

  out_exitCode is the exit code of the child process
**/
static BOOL ExecuteCommand( LPCTSTR lpCommand, DWORD * out_exitCode)
{
    BOOL                fSuccess = FALSE;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);

    fSuccess = CreateProcess(
        NULL,
        (LPTSTR)lpCommand,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
        );

    if ( fSuccess )
    {
        WaitForSingleObject( pi.hProcess, INFINITE );

        if (!GetExitCodeProcess( pi.hProcess, out_exitCode))
            fSuccess = FALSE;

        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
    }

    return fSuccess;
}

static BOOL RemoveCompleteDirectory( std::_tstring sPath )
{
    bool bDirectoryRemoved = true;

    std::_tstring sPattern = sPath + TEXT("\\") + TEXT("*.*");
    WIN32_FIND_DATA aFindData;

    // Finding all content in sPath

    HANDLE hFindContent = FindFirstFile( sPattern.c_str(), &aFindData );

    if ( hFindContent != INVALID_HANDLE_VALUE )
    {
        bool fNextFile = false;

        do
        {
            std::_tstring sFileName = aFindData.cFileName;
            std::_tstring sCurrentDir = TEXT(".");
            std::_tstring sParentDir = TEXT("..");

            if (( strcmp(sFileName.c_str(),sCurrentDir.c_str()) != 0 ) &&
                ( strcmp(sFileName.c_str(),sParentDir.c_str()) != 0 ))
            {
                std::_tstring sCompleteFileName = sPath + TEXT("\\") + sFileName;

                if ( aFindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
                {
                    RemoveCompleteDirectory(sCompleteFileName);
                }
                else
                {
                    DeleteFile( sCompleteFileName.c_str() );
                }
            }

            fNextFile = FindNextFile( hFindContent, &aFindData );

        } while ( fNextFile );

        FindClose( hFindContent );

        // empty directory can be removed now
        // RemoveDirectory is only successful, if the last handle to the directory is closed
        // -> first removing content -> closing handle -> remove empty directory

        if ( ! ( RemoveDirectory(sPath.c_str()) ) )
        {
            bDirectoryRemoved = false;
        }
    }

    return bDirectoryRemoved;
}

extern "C" UINT __stdcall RegisterExtensions(MSIHANDLE handle)
{
    // std::_tstring sInstDir = GetMsiProperty( handle, TEXT("INSTALLLOCATION") );
    std::_tstring sInstDir = GetMsiProperty( handle, TEXT("CustomActionData") );
    std::_tstring sUnoPkgFile = sInstDir + TEXT("program\\unopkg.exe");
    std::_tstring mystr;

    WIN32_FIND_DATA aFindFileData;
    bool registrationError = false;

    // Find unopkg.exe
    HANDLE hFindUnopkg = FindFirstFile( sUnoPkgFile.c_str(), &aFindFileData );

    if ( hFindUnopkg != INVALID_HANDLE_VALUE )
    {
        // unopkg.exe exists in program directory
        std::_tstring sCommand = sUnoPkgFile + " sync";

        DWORD exitCode = 0;
        bool fSuccess = ExecuteCommand( sCommand.c_str(), & exitCode);
        if ( ! fSuccess )
        {
            mystr = "ERROR: An error occured during registration of extensions!";
            MessageBox(NULL, mystr.c_str(), "ERROR", MB_OK);
            registrationError = true;
        }

        FindClose( hFindUnopkg );
    }

    if ( registrationError )
    {
        return 1;
    }
    else
    {
        return ERROR_SUCCESS;
    }
}


extern "C" UINT __stdcall RemoveExtensions(MSIHANDLE handle)
{
    std::_tstring mystr;

    // Finding the product with the help of the propery FINDPRODUCT,
    // that contains a Windows Registry key, that points to the install location.

    TCHAR szValue[8192];
    DWORD nValueSize = sizeof(szValue);
    HKEY  hKey;
    std::_tstring sInstDir;

    std::_tstring sProductKey = GetMsiProperty( handle, TEXT("FINDPRODUCT") );

    if ( ERROR_SUCCESS == RegOpenKey( HKEY_CURRENT_USER,  sProductKey.c_str(), &hKey ) )
    {
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("INSTALLLOCATION"), NULL, NULL, (LPBYTE)szValue, &nValueSize ) )
        {
            sInstDir = szValue;
        }
        RegCloseKey( hKey );
    }
    else if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE,  sProductKey.c_str(), &hKey ) )
    {
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("INSTALLLOCATION"), NULL, NULL, (LPBYTE)szValue, &nValueSize ) )
        {
            sInstDir = szValue;
        }
        RegCloseKey( hKey );
    }
    else
    {
        return ERROR_SUCCESS;
    }

    // Removing complete directory "Basis\prereg\bundled"

    std::_tstring sCacheDir = sInstDir + TEXT("share\\prereg\\bundled");

    RemoveCompleteDirectory( sCacheDir );

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
