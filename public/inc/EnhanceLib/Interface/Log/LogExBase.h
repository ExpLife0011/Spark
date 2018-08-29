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

#include "DllExport.h"
#include "Log\log.h"

class CLogEx
{
public:
    static DLL_COMMONLIB_API void WINAPI LogInit(CHAR* LogPath, DWORD Level);

    static DLL_COMMONLIB_API void WINAPI LogInit(HMODULE DllModule, CHAR* FileName, DWORD level);

	static DLL_COMMONLIB_API void WINAPI Dump(unsigned char* Buffer, unsigned int Length);

    static DLL_COMMONLIB_API void WINAPI SetModuleName(CHAR* ModuleName);

    static DLL_COMMONLIB_API CHAR* WINAPI GetModuleName();

    static DLL_COMMONLIB_API void WINAPI LogDone();
};

extern CLogWriter gLogWriter;

#define L_TRACE(...)                                                                                 \
        gLogWriter.Log(LOG_LEVEL_TRACE, MODULE_NAME, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \

#define L_DEBUG(Moduel, ...)                                                                         \
        gLogWriter.Log(LOG_LEVEL_DEBUG, MODULE_NAME, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
 
#define L_INFO(Moduel, ...)                                                                          \
        gLogWriter.Log(LOG_LEVEL_INFO, MODULE_NAME, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);  \
 
#define L_WARN(Moduel, ...)                                                                             \
        gLogWriter.Log(LOG_LEVEL_WARNING, MODULE_NAME, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);  \
 
#define L_ERROR(Moduel, ...)                                                                          \
        gLogWriter.Log(LOG_LEVEL_ERROR, MODULE_NAME,  __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
 
#define LOG_DUMP(Module, Buffer, Length)                                                             \
        CLogEx::Dump((unsigned char*)Buffer, (unsigned int)Length);                                  \

#endif
