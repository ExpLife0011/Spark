/**-----------------------------------------------------------------------------
 * @file     CTLogEx_OsUnix.h
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
//#include "windef.h"
#include <winpr/wtypes.h>
#include <winpr/tchar.h>
#include <winpr/file.h>
//#include <tchar.h>
#include <string.h>
#include <tchar.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

typedef std::string tstring;

typedef struct _MYSYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    DWORD dwSecond;
    DWORD dwUs;
} MYSYSTEMTIME, *PMYSYSTEMTIME;

/*
 * 操作系统相关部分的初始化
 */
BOOL Log_OsInit(const TCHAR *confname, int flags);
void Log_OsDone();
/*
 * 获取文件路径相对于可执行程序的完整路径
 */
tstring Log_OsGetFullPathNameRelativeExec(const TCHAR *filename);
/*
 * 移动文件
 */
BOOL Log_OsMoveFile(const TCHAR *orgfilename, const TCHAR *newfilename);
/*
 * 获取系统时间
 */
void Log_OsGetTime(MYSYSTEMTIME *time, BOOL b_performance_counter);
/*
 * 显示log模块的错误信息
 */
void Log_OsShowError(const TCHAR *fmt, ...);
/*
 * 锁
 */
void Log_OsLock();
void Log_OsUnlock();

/*
 * 获取线程ID
 */
unsigned long Log_CurrentThreadID();

UINT
GetPrivateProfileInt(
    const char *lpAppName,
    const char *lpKeyName,
    INT         nDefault,
    const char *lpFileName);

DWORD
GetPrivateProfileString(
    const char *lpAppName,
    const char *lpKeyName,
    const char *lpDefault,
    char       *lpReturnedString,
    DWORD      nSize,
    const char *lpFileName);
    
