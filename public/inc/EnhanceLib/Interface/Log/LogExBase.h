#pragma once

#ifndef __LOG_EX_BASE_H__
#define __LOG_EX_BASE_H__

#ifdef WIN32
#include "StdAfx.h"
#else
#include "Windef.h"
#endif

#ifndef MODULE_NAME
#define MODULE_NAME (CLogEx::GetModuleName())
#endif

#ifdef WIN32
    #ifdef UNICODE
        #define GetModuleName GetModuleNameW
        #define SetModuleName SetModuleNameW
        #define LogInit       LogInitW
    #else
        #define GetModuleName GetModuleNameA
        #define SetModuleName SetModuleNameA
        #define LogInit       LogInitA
    #endif
#else
    #define GetModuleName GetModuleNameA
    #define SetModuleName SetModuleNameA
    #define LogInit       LogInitA
#endif

#include "CTLogEx.h"

#include "DllExport.h"

class CLogEx
{
public:
    static DLL_COMMONLIB_API void WINAPI LogInitA(CHAR* ConfigFullPath);

#ifdef WIN32
    static DLL_COMMONLIB_API void WINAPI LogInitW(WCHAR* ConfigFullPath);

    static DLL_COMMONLIB_API void WINAPI LogInitW(HMODULE DllModule, WCHAR* FileName);
#endif

    static DLL_COMMONLIB_API void WINAPI SetLogLevel(DWORD Level);

    static DLL_COMMONLIB_API void WINAPI SetOutputType(DWORD Level);

    static DLL_COMMONLIB_API void WINAPI AddOutputModule(DWORD Module);

    static DLL_COMMONLIB_API void WINAPI RemoveOutputModule(DWORD Module);

	static DLL_COMMONLIB_API void WINAPI Dump(unsigned char* Buffer, unsigned int Length);

#ifdef WIN32
    static DLL_COMMONLIB_API void WINAPI SetModuleNameW(WCHAR* ModuleName);

    static DLL_COMMONLIB_API WCHAR* WINAPI GetModuleNameW();
#endif

    static DLL_COMMONLIB_API void WINAPI SetModuleNameA(CHAR* ModuleName);

    static DLL_COMMONLIB_API CHAR* WINAPI GetModuleNameA();

    static DLL_COMMONLIB_API DWORD WINAPI GetModuleMask();

    static DLL_COMMONLIB_API void WINAPI LogDone();

};

#define LOG_TRACE(Moduel, ...)                                                                       \
    if (CLogEx::GetModuleMask() & (1 << Moduel)) {                                                               \
        CTLogEx_printf(MODULE_NAME, LOG_LEVEL_TRACE, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
    }

#define LOG_DEBUG(Moduel, ...)                                                                       \
    if (CLogEx::GetModuleMask() & (1 << Moduel)) {                                                               \
        CTLogEx_printf(MODULE_NAME, LOG_LEVEL_DEBUG, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
    }

#define LOG_INFO(Moduel, ...)                                                                       \
    if (CLogEx::GetModuleMask() & (1 << Moduel)) {                                                              \
        CTLogEx_printf(MODULE_NAME, LOG_LEVEL_INFO, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
    }   

#define LOG_WARN(Moduel, ...)                                                                       \
    if (CLogEx::GetModuleMask() & (1 << Moduel)) {                                                              \
        CTLogEx_printf(MODULE_NAME, LOG_LEVEL_WARN, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
    }   

#define LOG_ERROR(Moduel, ...)                                                                       \
    if (CLogEx::GetModuleMask() & (1 << Moduel)) {                                                               \
        CTLogEx_printf(MODULE_NAME, LOG_LEVEL_ERROR, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
    }

#define LOG_FATAL(Moduel, ...)                                                                       \
    if (CLogEx::GetModuleMask() & (1 << Moduel)) {                                                               \
        CTLogEx_printf(MODULE_NAME, LOG_LEVEL_FATAL, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
    }

#define LOG_DUMP(Module, Buffer, Length)                                                             \
    if (gModuleMask & (1 << Module)) {                                                               \
        CLogEx::Dump((unsigned char*)Buffer, (unsigned int)Length);                                                        \
    }	

#endif
