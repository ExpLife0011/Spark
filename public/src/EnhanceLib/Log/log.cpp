#include "stdafx.h"
#include "Log/log.h"
#include "Log/LogExBase.h"

using namespace enlib;

CLogWriter::CLogWriter() : CObject()
{
    m_dwLogLevel = LOG_LEVEL_INFO;
    m_hFileHandle = NULL;
    m_bIsAppend = TRUE;
    ZeroMemory(m_szLogPath, MAX_PATH);

    InitializeCriticalSection(&m_csLock);
}

CLogWriter::~CLogWriter()
{
    LogClose();
    DeleteCriticalSection(&m_csLock);
}

const CHAR* CLogWriter::LogLevelToString(DWORD loglevel) {
    switch (loglevel) {
        case LOG_LEVEL_DEBUG:
            return "DEBUG";
        case LOG_LEVEL_TRACE:
            return "TRACE";
        case LOG_LEVEL_INFO:
            return "NOTICE";
        case LOG_LEVEL_WARNING:
            return "WARN";
        case LOG_LEVEL_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

BOOL CLogWriter::CheckLevel(DWORD loglevel)
{
    if (loglevel >= m_dwLogLevel)
        return TRUE;
    else
        return FALSE;
}

BOOL CLogWriter::LogInit(DWORD loglevel, const CHAR* LogPath, BOOL append)
{
    m_dwLogLevel = loglevel;
    m_bIsAppend = append;
    if (strlen(LogPath) >= (MAX_PATH - 1))
    {
        fprintf(stderr, "the path of log file is too long:%d limit:%d\n", strlen(LogPath), MAX_PATH);
        return FALSE;
    }

    strncpy(m_szLogPath, LogPath, MAX_PATH);

    if (m_szLogPath[0] == '\0')
    {
        m_hFileHandle = stdout;
        fprintf(stderr, "now all the running-information are going to put to stderr\n");
        return TRUE;
    }

    m_hFileHandle = fopen(m_szLogPath, append ? "a" : "w");
    if (m_hFileHandle == NULL)
    {
        fprintf(stderr, "cannot open log file,file location is %s\n", m_szLogPath);
        return FALSE;
    }

    setvbuf(m_hFileHandle, (char *)NULL, _IOLBF, 0);

    fprintf(stderr, "now all the running-information are going to the file %s\n", m_szLogPath);
    return TRUE;
}

BOOL CLogWriter::Log(DWORD loglevel, 
        const CHAR* ModuleName,
        const CHAR* FunctionName,
        const CHAR* FileName,
        int line,
        CHAR* logmessage,
        DWORD Size)
{
    int _size;
    int prestrlen = 0;

    EnterCriticalSection(&m_csLock);
    CHAR* star = m_pBuffer;
    prestrlen = PremakeString(star, ModuleName, FunctionName, FileName, line, loglevel);
    star += prestrlen;

    strncpy(star, logmessage, LOG_BUFFSIZE);
    _size = Size;

    if (NULL == m_hFileHandle)
        fprintf(stderr, "%s", m_pBuffer);
    else
        WriteLog(m_pBuffer, prestrlen + _size);

    LeaveCriticalSection(&m_csLock);
    return TRUE;
}

int CLogWriter::PremakeString(CHAR* pBuffer,
        const CHAR* ModuleName,
        const CHAR* FunctionName,
        const CHAR* FileName,
        int line,
        DWORD loglevel)
{
    time_t now;
    now = time(&now);
    struct tm vtm;
    int ThreadId = GetCurrentThreadId();

    UNREFERENCED_PARAMETER(line);
    UNREFERENCED_PARAMETER(FileName);

#ifdef WIN32
    localtime_s(&vtm, &now);
#else
    localtime_r(&now, &vtm);
#endif
    return _snprintf(pBuffer, LOG_BUFFSIZE, "%02d-%02d %02d:%02d:%02d %s(%s) %08d [%s]",
        vtm.tm_mon + 1, vtm.tm_mday, vtm.tm_hour, vtm.tm_min, vtm.tm_sec,
        LogLevelToString(loglevel),
        ModuleName,
        ThreadId,
        FunctionName);
}

BOOL CLogWriter::WriteLog(CHAR* pbuffer, int len)
{
    if (1 == fwrite(pbuffer, len, 1, m_hFileHandle)) //only write 1 item
    {
        fflush(m_hFileHandle);
    }
    else
    {
        int x = errno;
        fprintf(stderr, "Failed to write to logfile. errno:%s    message:%s", strerror(x), pbuffer);
        return FALSE;
    }
    return TRUE;
}

BOOL CLogWriter::LogClose()
{
    if (m_hFileHandle == NULL)
    {
        return FALSE;
    }

    fflush(m_hFileHandle);
    fclose(m_hFileHandle);
    m_hFileHandle = NULL;
    return TRUE;
}


