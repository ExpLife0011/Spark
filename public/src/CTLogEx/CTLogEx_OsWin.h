/**-----------------------------------------------------------------------------
 * @file     CTLogEx_OsWin.h
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
#ifndef __CTLOGEX_OS_WIN_H__
#define __CTLOGEX_OS_WIN_H__

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <string>

typedef std::wstring tstring;

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
void Log_OsShowError(TCHAR *fmt, ...);
/*
 * 锁
 */
void Log_OsLock();
void Log_OsUnlock();

/*
 * 获取线程ID
 */
unsigned long Log_CurrentThreadID();


#endif
