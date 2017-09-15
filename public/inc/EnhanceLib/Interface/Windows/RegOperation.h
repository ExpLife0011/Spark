#pragma once

#ifndef __REG_OPERATION_H__
#define __REG_OPERATION_H__

#include <Windows.h>
#include <tchar.h>
#include "DllExport.h"

#ifdef UNICODE
#define RegOpReadDword          RegOpReadDwordW
#define RegOpReadString         RegOpReadStringW
#define RegOpWriteDword         RegOpWriteDwordW
#define RegOpWriteString        RegOpWriteStringW
#define RegOpReadMultiString    RegOpReadMultiStringW
#define ReadOpBinary            ReadOpBinaryW
#else
#define RegOpReadDword          RegOpReadDwordA
#define RegOpReadString         RegOpReadStringA
#define RegOpWriteDword         RegOpWriteDwordA
#define RegOpWriteString        RegOpWriteStringA
#define RegOpReadMultiString    RegOpReadMultiStringA
#define ReadOpBinary            ReadOpBinaryA
#endif

extern "C"
{
    DLL_COMMONLIB_API BOOL WINAPI RegOpReadDwordA(
        HKEY Key,
        const CHAR *Subkeyname,
        const CHAR *ValueName,
        DWORD *Value,
        DWORD DefaultVal);

    DLL_COMMONLIB_API BOOL WINAPI RegOpReadDwordW(
        HKEY Key,
        const WCHAR *Subkeyname,
        const WCHAR *ValueName,
        DWORD *Value,
        DWORD DefaultVal);

    DLL_COMMONLIB_API BOOL WINAPI RegOpReadStringA(
        HKEY Key,
        const CHAR *Subkeyname,
        const CHAR *ValueName,
        CHAR  *Buffer,
        DWORD *Length,
        const CHAR *DefaultVal);

    DLL_COMMONLIB_API BOOL WINAPI RegOpReadStringW(
        HKEY Key,
        const WCHAR *Subkeyname,
        const WCHAR *ValueName,
        WCHAR  *Buffer,
        DWORD *Length,
        const WCHAR *DefaultVal);

    DLL_COMMONLIB_API BOOL WINAPI RegOpWriteDwordA(
        HKEY Key,
        const CHAR *Subkeyname,
        const CHAR *ValueName,
        DWORD Value);

    DLL_COMMONLIB_API BOOL WINAPI RegOpWriteDwordW(
        HKEY Key,
        const WCHAR *Subkeyname,
        const WCHAR *ValueName,
        DWORD Value);

    DLL_COMMONLIB_API BOOL WINAPI RegOpWriteStringA(
        HKEY Key,
        const CHAR *Subkeyname,
        const CHAR *ValueName,
        const CHAR *Value);

    DLL_COMMONLIB_API BOOL WINAPI RegOpWriteStringW(
        HKEY Key,
        const WCHAR *Subkeyname,
        const WCHAR *ValueName,
        const WCHAR *Value);

    DLL_COMMONLIB_API BOOL WINAPI RegOpReadMultiStringA(
        HKEY Key,
        const CHAR *Subkeyname,
        const CHAR *ValueName,
        CHAR *Buffer,
        DWORD *Length);

    DLL_COMMONLIB_API BOOL WINAPI RegOpReadMultiStringW(
        HKEY Key,
        const WCHAR *Subkeyname,
        const WCHAR *ValueName,
        WCHAR *Buffer,
        DWORD *Length);

    DLL_COMMONLIB_API BOOL WINAPI ReadOpBinaryA(
        HKEY hKey,
        const CHAR *subkeyname,
        const CHAR *valueName,
        BYTE **val,
        int *vallen);

    DLL_COMMONLIB_API BOOL WINAPI ReadOpBinaryW(
        HKEY hKey,
        const WCHAR *subkeyname,
        const WCHAR *valueName,
        BYTE **val,
        int *vallen);
}

#endif

