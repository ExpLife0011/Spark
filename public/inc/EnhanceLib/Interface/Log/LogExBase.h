#pragma once

#ifndef __LOG_EX_BASE_H__
#define __LOG_EX_BASE_H__

#ifdef WIN32
#include "StdAfx.h"
#else
#include "Windef.h"
#endif

#ifndef MODULE_NAME
#define MODULE_NAME (enlib::CLogEx::GetModuleName())
#endif

#include "DllExport.h"

#define LOG_LEVEL_DEBUG   1
#define LOG_LEVEL_TRACE   2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_WARNING 4
#define LOG_LEVEL_ERROR   5

namespace enlib
{
    class CLogEx
    {
    public:
        static DLL_COMMONLIB_API void WINAPI LogInit(CHAR* LogPath, DWORD Level);

        static DLL_COMMONLIB_API void WINAPI LogInit(HMODULE DllModule, CHAR* FileName, DWORD level);

        static DLL_COMMONLIB_API void WINAPI Dump(unsigned char* Buffer, unsigned int Length);

        static DLL_COMMONLIB_API void WINAPI SetModuleName(CHAR* ModuleName);

        static DLL_COMMONLIB_API CHAR* WINAPI GetModuleName();

        static DLL_COMMONLIB_API BOOL CLogEx::LogPrintf(DWORD loglevel,
            const CHAR* ModuleName,
            const CHAR* FunctionName,
            const CHAR* FileName,
            int line,
            CHAR* logformat, ...);

        static DLL_COMMONLIB_API void WINAPI LogDone();
    };
};

#define L_TRACE(...)                                                                                   \
        enlib::CLogEx::LogPrintf(LOG_LEVEL_TRACE, MODULE_NAME, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);   \

#define L_DEBUG(...)                                                                           \
        enlib::CLogEx::LogPrintf(LOG_LEVEL_DEBUG, MODULE_NAME, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);   \

#define L_INFO(...)                                                                            \
        enlib::CLogEx::LogPrintf(LOG_LEVEL_INFO, MODULE_NAME, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);    \

#define L_WARN(...)                                                                            \
        enlib::CLogEx::LogPrintf(LOG_LEVEL_WARNING, MODULE_NAME, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \

#define L_ERROR(...)                                                                           \
        enlib::CLogEx::LogPrintf(LOG_LEVEL_ERROR, MODULE_NAME,  __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);  \

#define L_DUMP(Buffer, Length)                                                               \
        enlib::CLogEx::Dump((unsigned char*)Buffer, (unsigned int)Length);                                    \

#define L_TRACE_ENTER() L_TRACE(("Enter\n"))
#define L_TRACE_LEAVE() L_TRACE(("Leave\n"))

#endif
