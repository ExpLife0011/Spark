/**-----------------------------------------------------------------------------
 * @file     CTLogEx_OsDep.cpp
 *
 * @author   yangrz@centerm.com.cn
 *
 * @date     2011/2/8
 *
 * @brief    
 *
 * @version  
 *
 *----------------------------------------------------------------------------*/
#include "CTLogEx_OsWin.h"
#include "CTLogEx.h"
#include <process.h>

// 全局锁
CRITICAL_SECTION g_cs;

// 监听线程
static HANDLE g_monitor_thread = NULL;
static HANDLE g_monitor_stop_event = NULL;

// 计数器频率
static LARGE_INTEGER g_freq;

/*
 * 显示log模块的错误信息
 * 直接输出到windows调试口
 */
void 
Log_OsShowError(wchar_t *fmt, ...)
{
    wchar_t szmsg[2048];
    wchar_t out[2048];
    va_list args;

    va_start(args, fmt);
    vswprintf(szmsg, sizeof(szmsg)/sizeof(szmsg[0]), fmt, args);
    va_end(args);

    swprintf(out, sizeof(szmsg)/sizeof(szmsg[0]), L"Centerm log module: %s", szmsg);
    OutputDebugString(out);
}

static void
ParseFilePath(const wchar_t *pathname, 
              tstring &dirname, tstring &filename)
{
    const wchar_t *p;

    if ( pathname == NULL )
    {
        dirname = L"";
        filename = L"";
        return;
    }
    p = wcsrchr(pathname, L'/');
    if ( p == NULL )
    {
        p = wcsrchr(pathname, L'\\');
    }
    if ( p == NULL || p == pathname )
    {
        dirname = L".";
        filename = pathname;
    }
    else
    {
        dirname = tstring(pathname, p);
        filename = p + 1;
    }
}

/*
 * 判断文件名是不是全路径
 */
static BOOL
IsFullpath(const wchar_t *filename) 
{
    wchar_t prefix[3];

    if ( filename == NULL )
    {
        return FALSE;
    }
    /*
     * XXX: 以"x:"开始的文件名认为是全路径
     *      暂不处理网络磁盘文件名，如\\test\test.ini
     */
    wcscpy(prefix, L"a:");
    for ( int i = 0; i < 26; i++ )
    {
        if ( _wcsnicmp(prefix, filename, 2) == 0 )
        {
            return TRUE;
        }
        prefix[0]++;
    }
    return FALSE;
}

/*
 * 获取文件完整路径
 * Windows API GetFullPathName 使用当前路径作为起始路径，这一个函数使用可执行程序路径作为起始路径
 */
tstring
Log_OsGetFullPathNameRelativeExec(const wchar_t *filename)
{
    wchar_t     imagePath[MAX_PATH + 1];
    wchar_t     *p;
    DWORD       ret;
    tstring     fullpath;

    fullpath = L"";
    if ( filename == NULL )
    {
        return fullpath;
    }
    if ( IsFullpath(filename) )
    {
        ret = GetFullPathName(filename, MAX_PATH, imagePath, NULL);
        if ( ret > 0 && ret <= MAX_PATH )
        {
            fullpath = imagePath;
        }
        return fullpath;
    }

    /* 
     * 获取当前程序的全路径 
     */
    memset(imagePath, 0, sizeof(imagePath));
    GetModuleFileName(NULL, imagePath, MAX_PATH);

    p = wcsrchr(imagePath, L'\\');
    if ( p != NULL )
    {
        tstring fullpath_tmp;

        *(p + 1) = NULL;
        fullpath_tmp = tstring(imagePath) + tstring(filename);

        ret = GetFullPathName(fullpath_tmp.c_str(), MAX_PATH, imagePath, NULL);
        if ( ret > 0 && ret <= MAX_PATH )
        {
            fullpath = imagePath;
        }
    }
    return fullpath;
}

static DWORD WINAPI 
FileChangeMonitorThread(LPVOID param)
{
    WCHAR       *confname = (WCHAR *)param;
    HANDLE      hDir = INVALID_HANDLE_VALUE;
    HANDLE      hEvents[2];
    OVERLAPPED  ol;
    DWORD       shareMode;
    BYTE        pinfobuf[sizeof(FILE_NOTIFY_INFORMATION) + MAX_PATH * sizeof(wchar_t)];
    DWORD       bytesReturned;
    tstring     dirname, filename;
    FILE_NOTIFY_INFORMATION *pinfo;

    ZeroMemory(&ol, sizeof(OVERLAPPED));

    shareMode = FILE_SHARE_READ
                | FILE_SHARE_WRITE 
                | FILE_SHARE_DELETE;

    ParseFilePath(confname, dirname, filename);
    if ( filename.empty() )
    {
        goto EXIT;
    }

    hDir = CreateFile(dirname.c_str(), GENERIC_READ, 
                      shareMode, NULL, OPEN_EXISTING, 
                      FILE_FLAG_OVERLAPPED | FILE_FLAG_BACKUP_SEMANTICS, NULL );
    if ( INVALID_HANDLE_VALUE == hDir )
    {
        Log_OsShowError(L"CreateFile fail %d\n", GetLastError());
        goto EXIT;
    }
    ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    hEvents[0] = ol.hEvent;
    hEvents[1] = g_monitor_stop_event;

    pinfo = (FILE_NOTIFY_INFORMATION *)pinfobuf;
    while (TRUE)
    {
        BOOL ret;

        ret = ReadDirectoryChangesW(hDir, pinfo, sizeof(pinfobuf), 
                                    FALSE,
                                    FILE_NOTIFY_CHANGE_LAST_WRITE, &bytesReturned, &ol, 
                                    NULL);
        if ( FALSE == ret )
        {
            goto EXIT;
        }
        ret = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
        switch ( ret )
        {
            case WAIT_OBJECT_0:
                GetOverlappedResult(hDir, &ol, &bytesReturned, TRUE);
                break;

            case WAIT_OBJECT_0 + 1:
                goto EXIT;
                break;

            default:
                goto EXIT;
        }

        if ( _wcsnicmp(pinfo->FileName, filename.c_str(), pinfo->FileNameLength / 2) == 0 )
        {
            // 重新载入配置
            CTLogEx_reload();
        }
    }

EXIT:
    if ( hDir != INVALID_HANDLE_VALUE )
    {
        CloseHandle(hDir);
    }
    if ( ol.hEvent )
    {
        CloseHandle(ol.hEvent);
    }
    free(confname);
    return 0;
}

BOOL 
Log_OsMoveFile(const TCHAR *orgfilename, const TCHAR *newfilename)
{
    if ( MoveFileEx(orgfilename, newfilename, MOVEFILE_REPLACE_EXISTING) == FALSE )
    {
        Log_OsShowError(L"MoveFileEx %s => %s fail: %d\n", orgfilename, newfilename, GetLastError());
        DeleteFile(orgfilename);
    }
    return TRUE;
}

void 
Log_OsGetTime(MYSYSTEMTIME *time, BOOL b_performance_counter)
{
    if ( !b_performance_counter )
    {
        SYSTEMTIME t;

        GetLocalTime(&t);
        time->wYear      = t.wYear;
        time->wMonth     = t.wMonth;
        time->wDayOfWeek = t.wDayOfWeek;
        time->wDay       = t.wDay;
        time->wHour      = t.wHour;
        time->wMinute    = t.wMinute;
        time->dwSecond   = t.wSecond;
        time->dwUs       = t.wMilliseconds * 1000;
    }
    else
    {
        LARGE_INTEGER cur;
        ULONGLONG us, s, min, hour;

        memset(time, 0, sizeof(SYSTEMTIME));

        QueryPerformanceCounter(&cur);
        us = cur.QuadPart * 1000000LL / g_freq.QuadPart;

        time->dwUs = us % 1000000LL;
        s = us / 1000000LL;
        time->dwSecond = s % 60LL;
        min = s / 60LL;
        time->wMinute = min % 60LL;
        hour = min / 60LL;
        time->wHour = hour % 24LL;
    }
}

BOOL 
Log_OsInit(const TCHAR *confname, int flags)
{

    if ( confname )
    {
        TCHAR *confnameNew;

        confnameNew = _tcsdup(confname);
        if ( confnameNew == NULL )
        {
            return FALSE;
        }

        if ( flags & CTLOG_INIT_FLAGS_MONITOR )
        {
            g_monitor_stop_event = CreateEvent(NULL, FALSE, FALSE, NULL);
            // 启动监视文件变化的线程
            g_monitor_thread = (HANDLE)_beginthreadex(NULL, 0, 
                                                      (unsigned int (__stdcall *)(void *))FileChangeMonitorThread, 
                                                      (PVOID)confnameNew, 0, NULL);
        }else
        {
            free(confnameNew);
        }
    }

    QueryPerformanceFrequency(&g_freq);

    return TRUE;
}

void 
Log_OsDone()
{
    // 结束监听线程
    if ( g_monitor_thread )
    {
        SetEvent(g_monitor_stop_event);
        if ( WaitForSingleObject(g_monitor_thread, 5000) == WAIT_TIMEOUT )
        {
            Log_OsShowError(L"Teriminal file changle notify thread\n");
            TerminateThread(g_monitor_thread, (DWORD)-1);
        }
        CloseHandle(g_monitor_thread);
        CloseHandle(g_monitor_stop_event);
        g_monitor_stop_event = NULL;
        g_monitor_thread = NULL;
    }
} 

void 
Log_OsLock()
{
    EnterCriticalSection(&g_cs);
}

void 
Log_OsUnlock()
{
    LeaveCriticalSection(&g_cs);
}

BOOL WINAPI
DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved) 
{
    UNREFERENCED_PARAMETER(hDllHandle);
    UNREFERENCED_PARAMETER(lpreserved);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            InitializeCriticalSection(&g_cs);
            break;
        }

        case DLL_PROCESS_DETACH:
            break;

        default:
            break;
    }
    return TRUE;
}

unsigned long 
Log_CurrentThreadID()
{
    return GetCurrentThreadId();
}
