/**-----------------------------------------------------------------------------
 * @file     CTLogEx_OsUnix.cpp
 *
 * @author   yangrz@centerm.com.cn
 *
 * @date     2011-02-08
 *
 * @brief    
 *
 * @version  
 *
 *----------------------------------------------------------------------------*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "CTLogEx_OsUnix.h"
#include "CTLogEx.h"
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include "iniparse.h"

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

BOOL 
Log_OsInit(const TCHAR *confname, int flags)
{
    return TRUE;
}

void 
Log_OsDone()
{
    // nothing to do
}

tstring 
Log_OsGetFullPathNameRelativeExec(const TCHAR *filename)
{
    char        imagePath[MAX_PATH + 1];
    char        *p;
    tstring     fullpath;

    fullpath = "";
    if ( filename == NULL )
    {
        return fullpath;
    }
    /* 全路径?? */
    if ( filename[0] == L'/' )
    {
        return filename;
    }

    /* 
     * 获取当前程序的全路径 
     */
    memset(imagePath, 0, sizeof(imagePath));
    if ( readlink("/proc/self/exe", imagePath, MAX_PATH) < 0 )
    {
        return fullpath;
    }

    p = strrchr(imagePath, '/');
    if ( p != NULL )
    {
        *(p + 1) = 0;
        fullpath = tstring(imagePath) + tstring(filename);
    }
    return fullpath;
}

BOOL 
Log_OsMoveFile(const TCHAR *orgfilename, const TCHAR *newfilename)
{
    if ( rename(orgfilename, newfilename) < 0 )
    {
        Log_OsShowError("rename %s => %s fail: %s\n", orgfilename, 
                        newfilename, strerror(errno));
        unlink(orgfilename);
    }
    return TRUE;
}

void 
Log_OsGetTime(MYSYSTEMTIME *time, BOOL b_performance_counter)
{
    struct timeval tv;
    struct tm tmp;

    gettimeofday(&tv, NULL);
    localtime_r(&(tv.tv_sec), &tmp);

    time->wYear      = tmp.tm_yday;
    time->wMonth     = tmp.tm_mon;
    time->wDayOfWeek = tmp.tm_wday;
    time->wDay       = tmp.tm_mday;
    time->wHour      = tmp.tm_hour;
    time->wMinute    = tmp.tm_min;
    time->dwSecond   = tmp.tm_sec;
    time->dwUs = tv.tv_usec;
}

void 
Log_OsShowError(const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void Log_OsLock()
{
    pthread_mutex_lock(&g_lock);
}

void Log_OsUnlock()
{
    pthread_mutex_unlock(&g_lock);
}

//------------------------------------------------------------------------------
// INI 文件操作
//------------------------------------------------------------------------------
UINT
GetPrivateProfileInt(
    const char *lpAppName,
    const char *lpKeyName,
    INT         nDefault,
    const char *lpFileName)
{
    int ret;

    if ( ini_get_int(lpFileName, lpAppName, lpKeyName, &ret) >= 0 )
    {
        return ret;
    }
    else
    {
        return nDefault;
    }
}

unsigned long 
Log_CurrentThreadID()
{
#ifdef ANDROID
    return 0;
#else
    return syscall(SYS_gettid);
#endif
}

DWORD
GetPrivateProfileString(
    const char *lpAppName,
    const char *lpKeyName,
    const char *lpDefault,
    char       *lpReturnedString,
    DWORD      nSize,
    const char *lpFileName)
{
    std::string strvalue;

    ini_get_string(lpFileName, lpAppName, lpKeyName, strvalue);
    if ( strvalue.empty() )
    {
        return 0;
    }
    else
    {
        if ( nSize > 0 )
        {
            memset(lpReturnedString, 0, nSize);
            strncpy(lpReturnedString, strvalue.c_str(), nSize - 1);
            return nSize;
        }
        else
        {
            return 0;
        }
    }
}
