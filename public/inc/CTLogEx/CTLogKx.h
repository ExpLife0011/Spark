/**-----------------------------------------------------------------------------
 * @file     CTLogKx.h
 *
 * @author   yangrz@centerm.com.cn
 *
 * @date     2011/3/31
 *
 * @brief    使用方法：定义以下两个宏
 *           #define MODULE_NAME xxxx
 *           #define DEBUG_LEVEL LOG_LEVEL_XXX
 *           #define DUMP_DATA xxx
 *           是否使用高性能计数器显示时间
 *           #define USE_HIGH_PERFORMANCE_COUNTER xxx
 *
 * @version  
 *
 *----------------------------------------------------------------------------*/
#ifndef __CTLOG_KX_H__
#define __CTLOG_KX_H__

/**
 * @brief 调试等级
 */
enum log_level_t
{
    LOG_LEVEL_FATAL = 1, /**< 致命错误 */
    LOG_LEVEL_ERROR = 2, /**< 错误 */
    LOG_LEVEL_WARN  = 3, /**< 警告 */
    LOG_LEVEL_INFO  = 4, /**< 信息 */
    LOG_LEVEL_DEBUG = 5, /**< 调试 */
    LOG_LEVEL_TRACE = 6, /**< 跟踪 */

    LOG_TYPE_DUMP   = 100, /**< DUMP信息 */
    LOG_TYPE_TMP    = 101, /**< 临时调试 */
};

/* 
 * 是否使用高性能计数器显示时间 
 */
#if USE_HIGH_PERFORMANCE_COUNTER
# define DCECL_LOG_INFOMATION                                                           \
    ULONGLONG us, s, min, hour;                                                         \
    LARGE_INTEGER freq, count;                                                          \
    count = KeQueryPerformanceCounter(&freq);                                           \
    us = count.QuadPart * 1000000LL / freq.QuadPart;                                    \
    s = us / 1000000LL;                                                                 \
    us = us % 1000000LL;                                                                \
    min = s / 60LL;                                                                     \
    s   = s % 60LL;                                                                     \
    hour = min / 60LL;                                                                  \
    min  = min % 60LL;                                                                  \

# define TIME_FORMAT         "%02d:%02d:%02d.%06d"
# define TIME_FORMAT_ARGS    (ULONG)hour, (ULONG)min, (ULONG)s, (ULONG)us
#else
# define DCECL_LOG_INFOMATION \
        LARGE_INTEGER systemTime;                                                       \
        LARGE_INTEGER localTime;                                                        \
        TIME_FIELDS   time;                                                             \
        KeQuerySystemTime(&systemTime);                                                 \
        ExSystemTimeToLocalTime(&systemTime, &localTime);                               \
        RtlTimeToTimeFields(&localTime, &time);
# define TIME_FORMAT         "%02d:%02d:%02d.%03d000"
# define TIME_FORMAT_ARGS    time.Hour, time.Minute, time.Second, time.Milliseconds
#endif

#if DBG
# define L_TRACE(fmt, ...)                                                              \
    if ( LOG_LEVEL_TRACE <= DEBUG_LEVEL )                                               \
    {                                                                                   \
        DCECL_LOG_INFOMATION;                                                           \
        DbgPrint(TIME_FORMAT ## " %04x-%04x TRACE(%s)(%s) " ## fmt,                     \
                 TIME_FORMAT_ARGS,                                                      \
                 (ULONG)PsGetCurrentProcessId(), (ULONG)PsGetCurrentThreadId(),         \
                 MODULE_NAME,                                                           \
                 __FUNCTION__,                                                          \
                 __VA_ARGS__);                                                          \
    }

# define L_DEBUG(fmt, ...)                                                              \
    if ( LOG_LEVEL_DEBUG <= DEBUG_LEVEL )                                               \
    {                                                                                   \
        DCECL_LOG_INFOMATION;                                                           \
        DbgPrint(TIME_FORMAT ## " %04x-%04x DEBUG(%s) " ## fmt,                         \
                 TIME_FORMAT_ARGS,                                                      \
                 (ULONG)PsGetCurrentProcessId(), (ULONG)PsGetCurrentThreadId(),         \
                 MODULE_NAME,                                                           \
                 __VA_ARGS__);                                                          \
    }

# define L_INFO(fmt, ...)                                                               \
    if ( LOG_LEVEL_INFO <= DEBUG_LEVEL )                                                \
    {                                                                                   \
        DCECL_LOG_INFOMATION;                                                           \
        DbgPrint(TIME_FORMAT ## " %04x-%04x _INFO(%s) " ## fmt,                         \
                 TIME_FORMAT_ARGS,                                                      \
                 (ULONG)PsGetCurrentProcessId(), (ULONG)PsGetCurrentThreadId(),         \
                 MODULE_NAME,                                                           \
                 __VA_ARGS__);                                                          \
    }
# define L_WARN(fmt ,...)                                                               \
    if ( LOG_LEVEL_WARN <= DEBUG_LEVEL )                                                \
    {                                                                                   \
        DCECL_LOG_INFOMATION;                                                           \
        DbgPrint(TIME_FORMAT ## " %04x-%04x _WARN(%s)(%s) " ## fmt,                     \
                 TIME_FORMAT_ARGS,                                                      \
                 (ULONG)PsGetCurrentProcessId(), (ULONG)PsGetCurrentThreadId(),         \
                 MODULE_NAME,                                                           \
                 __FUNCTION__,                                                          \
                 __VA_ARGS__);                                                          \
    }

# define L_ERROR(fmt, ...)                                                              \
    if ( LOG_LEVEL_ERROR <= DEBUG_LEVEL )                                               \
    {                                                                                   \
        DCECL_LOG_INFOMATION;                                                           \
        DbgPrint(TIME_FORMAT ## " ERROR(%s:%s:%d) " ## fmt,                            \
                 TIME_FORMAT_ARGS,                                                      \
                 MODULE_NAME,                                                           \
                 __FUNCTION__,                                                          \
                 __LINE__,                                                              \
                 __VA_ARGS__);                                                          \
    }
# define L_FATAL(fmt, ...)                                                              \
    if ( LOG_LEVEL_FATAL <= DEBUG_LEVEL )                                               \
    {                                                                                   \
        DCECL_LOG_INFOMATION;                                                           \
        DbgPrint(TIME_FORMAT ## " %04x-%04x FATAL(%s)(%s:%d) " ## fmt,                  \
                 TIME_FORMAT_ARGS,                                                      \
                 (ULONG)PsGetCurrentProcessId(), (ULONG)PsGetCurrentThreadId(),         \
                 MODULE_NAME,                                                           \
                 __FUNCTION__,                                                          \
                 __LINE__,                                                              \
                 __VA_ARGS__);                                                          \
    }


# define L_TRACE_ENTER() L_TRACE("Enter\n")
# define L_TRACE_LEAVE() L_TRACE("Leave\n")
# if DUMP_DATA
#  define L_DUMP(buf, len) CTLogEx_dump(buf, len)
# else
#  define L_DUMP(buf, len)
# endif
#else
# define L_TRACE(...)
# define L_DEBUG(...)
# define L_INFO(...)
# define L_WARN(...)
# define L_ERROR(...)
# define L_FATAL(...)
# define L_DUMP(buf, len)
# define L_TRACE_ENTER()
# define L_TRACE_LEAVE()
#endif

/**< 临时调试强制输出 */
#define L_TEMP(fmt, ...)                                                                \
    {                                                                                   \
        DCECL_LOG_INFOMATION;                                                           \
        DbgPrint(TIME_FORMAT ## " %04x-%04x _TEMP(%s)(%s:%d) " ## fmt,                  \
                 TIME_FORMAT_ARGS,                                                      \
                 (ULONG)PsGetCurrentProcessId(), (ULONG)PsGetCurrentThreadId(),         \
                 MODULE_NAME,                                                           \
                 __FUNCTION__,                                                          \
                 __LINE__,                                                              \
                 __VA_ARGS__);                                                          \
    }
#define L_TMPDUMP(buf, len) CTLogEx_dump(buf, len)

#define NB_BYTE 16
#define L_DUMPMSG(fmt, ...)                                                             \
    {                                                                                   \
        DCECL_LOG_INFOMATION;                                                           \
        DbgPrint(TIME_FORMAT ## " %04x-%04x _DUMP(%s) " ## fmt,                         \
                 TIME_FORMAT_ARGS,                                                      \
                 (ULONG)PsGetCurrentProcessId(), (ULONG)PsGetCurrentThreadId(),         \
                 MODULE_NAME,                                                           \
                 __VA_ARGS__);                                                          \
    }                                                                                   
#define CTLogEx_dump(buf1, len1) \
{ \
    char str[NB_BYTE*5 + 1]; \
    int  i,j; \
    char c; \
    unsigned char *buf = (unsigned char *)(buf1); \
    int len = (int)(len1); \
    for ( i = 0; i < len; i += NB_BYTE ) \
    { \
        char * p = str; \
        memset(str, 0x20, NB_BYTE*5 + 1); \
        for ( j = i; j < len && j < i + NB_BYTE; j++ ) \
        { \
            *p++ = ' '; \
            c = (buf[j] >> 4) & 0xf; \
            *p++ = (c<10) ? c+'0' : c-10+'a'; \
            c = buf[j] & 0xf; \
            *p++ = (c<10) ? c+'0' : c-10+'a'; \
        } \
        p +=  (NB_BYTE - (j - i)) * 3 + 2; \
        for ( j = i; j < len && j < i + NB_BYTE; j++ ) \
        { \
            c = buf[j]; \
            if ( ( c < 0x7F ) && ( c > 0x1F) ) \
            { \
                *p++ = c; \
            } else \
                *p++ = '.';              \
        } \
        *p = 0; \
        DbgPrint("    %04x:%s\n", i, str); \
    } \
}

#endif
