/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "quickstarter.hxx"
#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#include <psapi.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <malloc.h>

std::string GetOfficeInstallationPath(MSIHANDLE handle)
{
    std::string progpath;
    DWORD sz = 0;
    LPTSTR dummy = TEXT("");

    if (MsiGetProperty(handle, TEXT("INSTALLLOCATION"), dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++; // space for the final '\0'
        DWORD nbytes = sz * sizeof(TCHAR);
        LPTSTR buff = reinterpret_cast<LPTSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetProperty(handle, TEXT("INSTALLLOCATION"), buff, &sz);
        progpath = buff;
    }
    return progpath;
}

std::string GetOfficeProductName(MSIHANDLE handle)
{
    std::string productname;
    DWORD sz = 0;
    LPTSTR dummy = TEXT("");

    if (MsiGetProperty(handle, TEXT("ProductName"), dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++; // space for the final '\0'
        DWORD nbytes = sz * sizeof(TCHAR);
        LPTSTR buff = reinterpret_cast<LPTSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetProperty(handle, TEXT("ProductName"), buff, &sz);
        productname = buff;
    }
    return productname;
}

std::string GetQuickstarterLinkName(MSIHANDLE handle)
{
    std::string quickstarterlinkname;
    DWORD sz = 0;
    LPTSTR dummy = TEXT("");

    if (MsiGetProperty(handle, TEXT("Quickstarterlinkname"), dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++; // space for the final '\0'
        DWORD nbytes = sz * sizeof(TCHAR);
        LPTSTR buff = reinterpret_cast<LPTSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetProperty(handle, TEXT("Quickstarterlinkname"), buff, &sz);
        quickstarterlinkname = buff;
    }
    else if (MsiGetProperty(handle, TEXT("ProductName"), dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++; // space for the final '\0'
        DWORD nbytes = sz * sizeof(TCHAR);
        LPTSTR buff = reinterpret_cast<LPTSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetProperty(handle, TEXT("ProductName"), buff, &sz);
        quickstarterlinkname = buff;
    }
    return quickstarterlinkname;
}

inline bool IsValidHandle( HANDLE handle )
{
    return NULL != handle && INVALID_HANDLE_VALUE != handle;
}

static DWORD WINAPI _GetModuleFileNameExA( HANDLE hProcess, HMODULE hModule, LPSTR lpFileName, DWORD nSize )
{
    typedef DWORD (WINAPI *FN_PROC)( HANDLE hProcess, HMODULE hModule, LPSTR lpFileName, DWORD nSize );

    static FN_PROC  lpProc = NULL;

    if ( !lpProc )
    {
        HMODULE hLibrary = LoadLibrary("PSAPI.DLL");

        if ( hLibrary )
            lpProc = reinterpret_cast< FN_PROC >(GetProcAddress( hLibrary, "GetModuleFileNameExA" ));
    }

    if ( lpProc )
        return lpProc( hProcess, hModule, lpFileName, nSize );

    return 0;

}

std::string GetProcessImagePath( DWORD dwProcessId )
{
    std::string sImagePath;

    HANDLE  hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId );

    if ( IsValidHandle( hProcess ) )
    {
        CHAR    szPathBuffer[MAX_PATH] = "";

        if ( _GetModuleFileNameExA( hProcess, NULL, szPathBuffer, sizeof(szPathBuffer) ) )
            sImagePath = szPathBuffer;

        CloseHandle( hProcess );
    }

    return sImagePath;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
