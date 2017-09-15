/**-----------------------------------------------------------------------------
 * @file     CTLogExLite.h
 *
 * @author   yangrz@centerm.com.cn
 *
 * @date     2011/8/30
 *
 * @brief    CTLogEx的简化版本，
 *           有以下限制：
 *           1. inline函数实现，会增加二进制的大小。
 *           2. 不支持从ini文件动态读取配置；
 *           3. 最大支持2048个字符;
 *           4. 不支持设置输出格式;
 *           5. 不支持输出到文件
 *
 *           有以下优点:
 *           1. 不需要初始化, 包含头文件即可使用，不必连接额外的库, 适合临时调试以及小模块。
 *           2. 可以通过
 *              #define CTLOGEX_LEVEL 0 
 *              关闭log
 *              调试信息不会被编译到二进制中。
 *
 *           使用方法：
 *           // 设置模块名称，默认"default"
 *           #define MODULE_NAME "CTLOGEX"
 *           // 设置调试等级, 默认LOG_LEVEL_TRACE
 *           #define CTLOGEX_LEVEL  0 // 禁用log
 *           // 设置输出方式, windows 默认调试口，Linux 默认标准错误
 *           #define CTLOGEX_OUTPUT  LOG_OUTPUT_STDERR
 *           // 设置DUMP，默认开启
 *           #define CTLOGEX_DUMP    0 
 *           // 使用高精度时钟, 默认关闭
 *           #define USE_HIGH_PERFORMANCE_COUNTER 1
 *
 *           #include "CTLogExLite.h"
 *
 * @version  
 *          v1.00 初始化
 *
 *----------------------------------------------------------------------------*/
#ifndef __CTLOGEX_H__
#define __CTLOGEX_H__

#ifndef CTLOGEX_LEVEL 
# define CTLOGEX_LEVEL LOG_LEVEL_TRACE
#endif

#ifndef MODULE_NAME
# define MODULE_NAME _T("default")
#endif

#ifndef CTLOGEX_OUTPUT 
# ifdef _WIN32
#  define CTLOGEX_OUTPUT LOG_OUTPUT_DBGPORT
# else
#  define CTLOGEX_OUTPUT LOG_OUTPUT_STDERR
# endif
#endif

#ifndef CTLOGEX_DUMP
# define CTLOGEX_DUMP 1
#endif

#ifdef _WIN32 /* windows */
#include <windows.h>
#include <tchar.h>

# define CTLOGAPI __cdecl

# ifndef __cplusplus
#  define inline 
# endif

#else        /* Linux */
# define CTLOGAPI

# ifndef _T
#  define _T(x) x
# endif

# ifndef TCHAR
#  define TCHAR char
# endif

# ifndef _ftprintf
#  define _ftprintf   fprintf
# endif

# ifndef _sntprintf
#  define _sntprintf  snprintf
# endif

# ifndef _vsntprintf     
#  define _vsntprintf vsnprintf
# endif 

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>

#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef __GNUC__
# define __attribute__(x) /*NOTHING*/
#endif

#if defined(_MSC_VER) && defined(_PREFAST_)
# include <sal.h>
#else
# ifndef _Printf_format_string_
#  define _Printf_format_string_
# endif
#endif

/**
 * @brief 调试等级
 */
#define LOG_LEVEL_NONE      0   /**< 关闭log */
#define LOG_LEVEL_FATAL     1   /**< 致命错误 */
#define LOG_LEVEL_ERROR     2   /**< 错误 */
#define LOG_LEVEL_WARN      3   /**< 警告 */
#define LOG_LEVEL_INFO      4   /**< 信息 */
#define LOG_LEVEL_DEBUG     5   /**< 调试 */
#define LOG_LEVEL_TRACE     6   /**< 跟踪 */
#define LOG_TYPE_DUMP       100 /**< DUMP信息 */
#define LOG_TYPE_TMP        101 /**< 临时调试 */

/**
 * @brief log的输出类型
 */
enum log_output_type_t
{
    LOG_OUTPUT_DISABLE  = 0x0, /* 禁用log */
    LOG_OUTPUT_STDERR   = 0x1, /* 输出到控制台 */
    LOG_OUTPUT_DBGPORT  = 0x2, /* 输出到Windows调试口 */
    LOG_OUTPUT_FILE     = 0x3, /* 输出到文件 */
};

// 跟踪log的默认输出格式
#define TRACE_DEFAULT_FMT _T("%d %T %p(%M)(%f) %m")
// 调试log的默认输出格式
#define DEBUG_DEFAULT_FMT _T("%d %T %p(%M) %m")
// 信息log的默认输出格式
#define INFO_DEFAULT_FMT  _T("%d %T %p(%M) %m")
// 警告log的默认输出格式
#define WARN_DEFAULT_FMT  _T("%d %T %p(%M)(%f) %m")
// 错误log的默认输出格式
#define ERROR_DEFAULT_FMT _T("%d %T %p(%M)(%f:%L) %m")
// 致命错误log的默认输出格式
#define FATAL_DEFAULT_FMT _T("%d %T %p(%M)(%f:%L) %m")
// DUMP数据输出格式
#define DUMP_DEFAULT_FMT  _T("%d %T %p(%M)(%f) %m")
// 临时调试信息的输出格式
#define TMP_DEFAULT_FMT _T("%d %T %p(%M)(%f:%L) %m")

#ifdef _UNICODE
#define CHARSTRING_FMT L"%S"
#else
#define CHARSTRING_FMT "%s"
#endif
#define MAX_CONF_BUF_SIZE 2048

typedef struct _MYSYSTEMTIME {
    unsigned short wYear;
    unsigned short wMonth;
    unsigned short wDayOfWeek;
    unsigned short wDay;
    unsigned short wHour;
    unsigned short wMinute;
    unsigned int   dwSecond;
    unsigned int   dwUs;
} MYSYSTEMTIME, *PMYSYSTEMTIME;

#ifdef _WIN32
static inline unsigned long 
Log_CurrentThreadID()
{
    return GetCurrentThreadId();
}

static inline void 
Log_OsGetTime(MYSYSTEMTIME *time)
{
#if USE_HIGH_PERFORMANCE_COUNTER
    LARGE_INTEGER freq;
    LARGE_INTEGER cur;
    ULONGLONG us, s, min, hour;

    memset(time, 0, sizeof(SYSTEMTIME));

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&cur);

    us = cur.QuadPart * 1000000LL / freq.QuadPart;

    time->dwUs = us % 1000000LL;
    s = us / 1000000LL;
    time->dwSecond = s % 60LL;
    min = s / 60LL;
    time->wMinute = min % 60LL;
    hour = min / 60LL;
    time->wHour = hour % 24LL;
#else
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
#endif
}

/*
 * VC下 sprintf空间不足时返回-1
 */
#define ADD_WICH_CHECK(pos, ret, count)                  \
    {                                               \
        if ( (ret) >=0 )                            \
            (pos) += (ret);                         \
        else                                        \
            (pos) += (count);                       \
    }

#else
static inline unsigned long 
Log_CurrentThreadID()
{
    return syscall(SYS_gettid);
}

static inline void 
Log_OsGetTime(MYSYSTEMTIME *time)
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

/*
 * gcc sprintf空间不足时返回需要的字节数
 */
#define ADD_WICH_CHECK(pos, ret, count)             \
    {                                               \
        if ( (ret) <= count )                       \
            (pos) += (ret);                         \
        else                                        \
            (pos) += (count);                       \
    }

#endif

#ifdef _WIN32
# pragma warning(push)
/*  C4996: '_CRT_SECURE_NO_WARNINGS. */
# pragma warning(disable: 4996)
#endif

static inline void
CTLogEx_puts(TCHAR *out_str, enum log_output_type_t out_type)
{
#ifdef _WIN32
    if ( out_type == LOG_OUTPUT_DBGPORT )
    {
        OutputDebugString(out_str);
    }
    else
#endif
    {
        _ftprintf(stderr, _T("%s"), out_str);
    }
}

static inline void CTLOGAPI 
CTLogEx_printf(const TCHAR *module_name, 
               int loglevel, 
               const char *function_name,
               const char *file_name,
               int line,
               _Printf_format_string_ const TCHAR *pszFormat, ...)
{
    TCHAR           buf[MAX_CONF_BUF_SIZE + 128];
    TCHAR           *out = buf;
    int             max_out_chars = MAX_CONF_BUF_SIZE;
    TCHAR          *out_p;

    /* 
     * 解析输出格式 
     */
    {
        const TCHAR *p;
        const TCHAR *prefix;

        switch ( loglevel )
        {
            case LOG_LEVEL_TRACE:
                prefix = _T("TRACE");
                p = TRACE_DEFAULT_FMT;
                break;
            case LOG_LEVEL_DEBUG:
                prefix = _T("DEBUG");
                p = DEBUG_DEFAULT_FMT;
                break;
            case LOG_LEVEL_INFO:
                prefix = _T("_INFO");
                p = INFO_DEFAULT_FMT;
                break;
            case LOG_LEVEL_WARN:
                prefix = _T("_WARN");
                p = WARN_DEFAULT_FMT;
                break;
            case LOG_LEVEL_ERROR:
                prefix = _T("ERROR");
                p = ERROR_DEFAULT_FMT;
                break;
            case LOG_LEVEL_FATAL:
                prefix= _T("FATAL");
                p = FATAL_DEFAULT_FMT;
                break;
            case LOG_TYPE_DUMP:
                prefix = _T("_DUMP");
                p = DUMP_DEFAULT_FMT;
                break;
            case LOG_TYPE_TMP:
                prefix = _T("_TEMP");
                p = TMP_DEFAULT_FMT;
                break;
            default:
                return;
        }

        out_p = out;
        while ( *p )
        {
            int b_match = 0;
            /*
             * %F 显示调用logger的源文件名
             * %L 显示调用logger的代码行
             * %m 显示输出消息
             * %M 模块名称
             * %f 显示调用logger的函数名
             * %p 显示该条日志的优先级
             * %d 显示日志记录时间
             * %P log前缀，见log_prefix配置
             * %T 线程ID
             */
            if ( *p == _T('%') )
            {
                int ret;
                int count;

                b_match = 1;
                switch ( *(p + 1) )
                {
                    case _T('F'):
                        count = max_out_chars - (int)(out_p - out);
                        ret = _sntprintf(out_p, count, CHARSTRING_FMT, file_name);
                        ADD_WICH_CHECK(out_p, ret, count);
                        break;

                    case _T('L'):
                        count = max_out_chars - (int)(out_p - out);
                        ret = _sntprintf(out_p, count, _T("%d"), line);
                        ADD_WICH_CHECK(out_p, ret, count);
                        break;

                    case _T('T'):
                        count = max_out_chars - (int)(out_p - out);
                        ret = _sntprintf(out_p, count, _T("%08ld"), Log_CurrentThreadID());
                        ADD_WICH_CHECK(out_p, ret, count);
                        break;

                    case _T('m'):
                    {
                        va_list args;

                        va_start(args, pszFormat);
                        count = max_out_chars - (int)(out_p - out);
                        ret = _vsntprintf(out_p, count, pszFormat, args);
                        ADD_WICH_CHECK(out_p, ret, count);
                        va_end(args);
                        break;
                    }

                    case _T('M'):
                        count = max_out_chars - (int)(out_p - out);
                        ret = _sntprintf(out_p, count, _T("%s"), module_name);
                        ADD_WICH_CHECK(out_p, ret, count);
                        break;

                    case _T('f'):
                        count = max_out_chars - (int)(out_p - out);
                        ret = _sntprintf(out_p, count, CHARSTRING_FMT, function_name);
                        ADD_WICH_CHECK(out_p, ret, count);
                        break;

                    case _T('p'):
                    {
                        count = max_out_chars - (int)(out_p - out);
                        ret = _sntprintf(out_p, count, _T("%s"), prefix);
                        ADD_WICH_CHECK(out_p, ret, count);
                        break;
                    }

                    case _T('d'):
                    {
                        MYSYSTEMTIME time;
                        TCHAR time_buf[128];

                        Log_OsGetTime(&time);
                        _sntprintf(time_buf, sizeof(time_buf)/sizeof(TCHAR), 
                                   _T("%02d:%02d:%02d.%06d"), time.wHour, time.wMinute, 
                                   time.dwSecond, time.dwUs);
                        count = max_out_chars - (int)(out_p - out);
                        ret = _sntprintf(out_p, count, _T("%s"), time_buf);
                        ADD_WICH_CHECK(out_p, ret, count);
                        break;
                    }

                    case _T('P'):
                        count = max_out_chars - (int)(out_p - out);
                        ret = _sntprintf(out_p, count, _T("%s"), prefix);
                        ADD_WICH_CHECK(out_p, ret, count);
                        break;

                    default:
                        b_match = 0;
                        break;
                }
                if ( b_match )
                {
                    p += 2;
                }
            }
            if ( !b_match )
            {
                *out_p = *p;
                out_p++;
                p++;
            }
        }
        *out_p = 0;
    }

    /*
     * 写入log信息
     */
    CTLogEx_puts(out, CTLOGEX_OUTPUT);
}

#define NB_BYTE 16
static inline void 
CTLOGAPI CTLogEx_dump(const TCHAR *module_name, const char *function_name, const char *file_name, int line, 
                      const void *buf1, int len)
{
    unsigned char  *buf = (unsigned char *)buf1;
    char            str[NB_BYTE*10 + 1];
    int             i,j;
    char            c;
    TCHAR           msg[256];

    CTLogEx_printf(module_name, LOG_TYPE_DUMP, function_name, file_name, line, 
                   _T("---------- Begin Dump %d bytes -------------\n"), len);
    for ( i = 0; i < len; i += NB_BYTE )
    {
        char * p = str;

        memset( str, 0x20, NB_BYTE*10 +1 );

        // Show Hex 
        for ( j = i; j < len && j < i + NB_BYTE; j++ )
        {

            *p++ = ' ';
            c = (buf[j] >> 4) & 0xf;
            *p++ = (c<10) ? c+'0' : c-10+'a';
            c = buf[j] & 0xf;
            *p++ = (c<10) ? c+'0' : c-10+'a';
        }

        // PADs for not 16 bytes align
        p +=  (NB_BYTE - (j - i)) * 3 + 2;

        // Show ASCII
        for ( j = i; j < len && j < i + NB_BYTE; j++ )
        {
            c = buf[j];
            if ( ( c < 0x7F ) && ( c > 0x1F) )
            {
                *p++ = c;
            } else
                *p++ = '.';             
        }

        *p = 0;

        _sntprintf(msg, sizeof(msg)/sizeof(msg[0]), _T("\t%04x:") CHARSTRING_FMT _T("\n"), i, str);
        CTLogEx_puts(msg, CTLOGEX_OUTPUT);
    }
    CTLogEx_printf(module_name, LOG_TYPE_DUMP, function_name, file_name, line, 
                   _T("-------------- End Dump --------------\n"));
}

/**
 * @brief 跟踪等级日志输出
 *
 * @param[in] fmt 格式化字符串
 */
#if (CTLOGEX_LEVEL >= LOG_LEVEL_TRACE)
#define L_TRACE(...) CTLogEx_printf(MODULE_NAME, LOG_LEVEL_TRACE, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#else
#define L_TRACE(...)
#endif
#define L_TRACE_ENTER() L_TRACE(_T("Enter\n"))
#define L_TRACE_LEAVE() L_TRACE(_T("Leave\n"))

/**
 * @brief 调试等级日志输出
 *
 * @param[in] fmt 格式化字符串
 */
#if (CTLOGEX_LEVEL >= LOG_LEVEL_DEBUG)
#define L_DEBUG(...) CTLogEx_printf(MODULE_NAME, LOG_LEVEL_DEBUG, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#else
#define L_DEBUG(...)
#endif

/**
 * @brief 信息等级日志输出
 *
 * @param[in] fmt 格式化字符串
 */
#if (CTLOGEX_LEVEL >= LOG_LEVEL_INFO)
#define L_INFO(...)  CTLogEx_printf(MODULE_NAME, LOG_LEVEL_INFO,  __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#else
#define L_INFO(...)
#endif

/**
 * @brief 警告等级日志输出
 *
 * @param[in] fmt 格式化字符串
 */
#if (CTLOGEX_LEVEL >= LOG_LEVEL_WARN)
#define L_WARN(...)  CTLogEx_printf(MODULE_NAME, LOG_LEVEL_WARN,  __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#else
#define L_WARN(...)
#endif
/**
 * @brief 错误等级日志输出
 *
 * @param[in] fmt 格式化字符串
 */
#if (CTLOGEX_LEVEL >= LOG_LEVEL_ERROR)
#define L_ERROR(...) CTLogEx_printf(MODULE_NAME, LOG_LEVEL_ERROR, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#else
#define L_ERROR(...)
#endif

/**
 * @brief 致命错误等级日志输出
 *
 * @param[in] fmt 格式化字符串
 */
#if (CTLOGEX_LEVEL >= LOG_LEVEL_FATAL)
#define L_FATAL(...) CTLogEx_printf(MODULE_NAME, LOG_LEVEL_FATAL, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#else
#define L_FATAL(...)
#endif

/**
 * @brief 临时调试宏 \n
 * 这一部分信息强制输出，只用于临时调试, 提交到svn的版本不允许有这一个宏
 *
 * @param[in] fmt 格式化字符串
 */
#define L_TEMP(...) CTLogEx_printf(MODULE_NAME, LOG_TYPE_TMP, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);

/**
 * @brief dump数据块
 *
 * @param[in] buf         数据地址
 * @param[in] len         数据块大小
 */
#if CTLOGEX_DUMP
#define L_DUMP(buf, len) CTLogEx_dump(MODULE_NAME, __FUNCTION__, __FILE__, __LINE__, buf, len);
#else
#define L_DUMP(buf, len)
#endif

/**
 * @brief dump数据, 只用于临时调试, 提交到svn的版本不允许使用
 *
 * @param[in] buf         数据地址
 * @param[in] len         数据块大小
 */
#define L_TMPDUMP(buf, len) CTLogEx_dump(MODULE_NAME, __FUNCTION__, __FILE__, __LINE__, buf, len);

#ifdef _WIN32
# pragma warning(pop)
#endif

#endif
