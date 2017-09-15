#pragma once

#ifndef __GLOBAL_EVENT_H__
#define __GLOBAL_EVENT_H__

#include <Windows.h>
#include <tchar.h>
#include "DllExport.h"

extern "C"
{
    DLL_COMMONLIB_API HANDLE WINAPI CreateGlobalEventW(WCHAR* pEventName);

    DLL_COMMONLIB_API HANDLE WINAPI CreateGlobalEventA(CHAR* pEventName);

    DLL_COMMONLIB_API HANDLE WINAPI CreateGlobalAutoEventA(CHAR* pEventName);

    DLL_COMMONLIB_API HANDLE WINAPI CreateGlobalAutoEventW(WCHAR* pEventName);
}

#ifdef UNICODE
#define CreateGlobalAutoEvent CreateGlobalAutoEventW
#define CreateGlobalEvent     CreateGlobalEventW
#else
#define CreateGlobalAutoEvent CreateGlobalAutoEventA
#define CreateGlobalEvent     CreateGlobalEventA
#endif

#endif