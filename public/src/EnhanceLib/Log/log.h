#pragma once

#ifndef __ENHANCE_LOG_H__
#define __ENHANCE_LOG_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Base/BaseObject.h"

#define LOG_BUFFSIZE   (1024 * 1024 * 4)
#define LOG_MODULE_LEN (32)

namespace enlib
{
    class CLogWriter : public CObject
    {
    public:
        CLogWriter();
        virtual ~CLogWriter();

        BOOL LogInit(DWORD loglevel, const CHAR* LogPath, BOOL append = TRUE);

        BOOL Log(DWORD loglevel,
            const CHAR* ModuleName,
            const CHAR* FunctionName,
            const CHAR* FileName,
            int line,
            CHAR* logmessage,
            DWORD Size);

        BOOL LogClose();
    private:
        const CHAR* LogLevelToString(DWORD loglevel);

        BOOL CheckLevel(DWORD loglevel);

        int PremakeString(CHAR* pBuffer,
            const CHAR* ModuleName,
            const CHAR* FunctionName,
            const CHAR* FileName,
            int line,
            DWORD loglevel);

        BOOL WriteLog(CHAR* pBuffer, int len);

        DWORD    m_dwLogLevel;
        FILE*    m_hFileHandle;
        BOOL     m_bIsAppend;
        CHAR     m_szLogPath[MAX_PATH];

        CRITICAL_SECTION m_csLock;
        CHAR m_pBuffer[LOG_BUFFSIZE];
    };
};

#endif
