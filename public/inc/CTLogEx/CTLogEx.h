/**-----------------------------------------------------------------------------
 * @file     CTLogEx.h
 *
 * @author   yangrz@centerm.com.cn
 *
 * @date     2011/1/31
 *
 * @brief    log系统导出接口\n
 *           其他模块使用这一些接口输出log信息 
 *
 * @version  
 *
 *----------------------------------------------------------------------------*/
#ifndef __CTLOGEX_H__
#define __CTLOGEX_H__

#ifdef _WIN32
# define CTLOGAPI __cdecl
#else
# define CTLOGAPI
# ifndef _T
#  define _T(x) x
# endif
#endif

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

/**
 * @brief 监控配置文件变化
 */
#define CTLOG_INIT_FLAGS_MONITOR 0x1

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 初始化log系统
 *
 * @param[in] config_file               配置文件路径\n
 *                                      如果这一个参数是NULL或者配置文件不存在，则使用默认值
 * @param[in] flags                     见CTLOG_INIT_FLAGS_XXXXX 定义
 *
 * @return 成功 非0\n
 *         失败 0
 */
#ifdef _WIN32
int CTLOGAPI CTLogEx_initW(const wchar_t *config_file);
int CTLOGAPI CTLogEx_initExW(const wchar_t *config_file, int flags);
#endif
int CTLOGAPI CTLogEx_initA(const char *config_file);
int CTLOGAPI CTLogEx_initExA(const char *config_file, int flags);
#ifdef _UNICODE 
#define CTLogEx_init CTLogEx_initW
#define CTLogEx_initEx CTLogEx_initExW
#else
#define CTLogEx_init CTLogEx_initA
#define CTLogEx_initEx CTLogEx_initExA
#endif

/**
 * @brief 设置log系统字符串类型参数 
 *
 * @param[in] modulename 模块名，"GLOBAL"表示全局(默认)设置
 * @param[in] filedname  域名
 * @param[in] value      值
 */
#ifdef _WIN32
void CTLOGAPI CTLogEx_SetOptStrW(const wchar_t *modulename, 
                                 const wchar_t *filedname, 
                                 const wchar_t *value);
#endif
void CTLOGAPI CTLogEx_SetOptStrA(const char *modulename, 
                                 const char *filedname, 
                                 const char *value);
#ifdef _UNICODE 
#define CTLogEx_SetOptStr CTLogEx_SetOptStrW
#else
#define CTLogEx_SetOptStr CTLogEx_SetOptStrA
#endif

/**
 * @brief 设置log系统int类型参数 
 *
 * @param[in] modulename 模块名，"GLOBAL"表示全局(默认)设置
 * @param[in] filedname  域名
 * @param[in] value      值
 */
#ifdef _WIN32
void CTLOGAPI CTLogEx_SetOptIntW(const wchar_t *modulename, 
                                 const wchar_t *filedname, 
                                 int value);
#endif
void CTLOGAPI CTLogEx_SetOptIntA(const char *modulename, 
                                 const char *filedname, 
                                 int value);
#ifdef _UNICODE 
#define CTLogEx_SetOptInt CTLogEx_SetOptIntW
#else
#define CTLogEx_SetOptInt CTLogEx_SetOptIntA
#endif

/**
 * @brief 设置log输出类型
 *
 * @param[in] type 输出类型
 * @see   log_output_type_t
 */
#define CTLogEx_setOutput(type) \
    CTLogEx_SetOptInt(_T("GLOBAL"), _T("log_output_type"), (log_output_type_t)(type));
/**
 * @brief 设置log输出文件名
 *
 * @param[in] filename
 */
#define CTLogEx_setLogFile(filename) \
    { CTLogEx_SetOptStr(_T("GLOBAL"), _T("log_output_filepath"), (filename));  \
      CTLogEx_SetOptInt(_T("GLOBAL"), _T("log_output_type"),     LOG_OUTPUT_FILE); }

/**
 * @brief 禁用/启用DUMP
 *
 * @param filename
 */
#define CTLogEx_enableDump(bEnable) \
    CTLogEx_SetOptInt(_T("GLOBAL"), _T("log_dump"), bEnable);

/**
 * @brief 设置log默认等级
 *
 * @param[in] level log等级
 * @see log_level_t
 */
#define CTLogEx_setLogLevel(level) \
    CTLogEx_SetOptInt(_T("GLOBAL"), _T("log_level"), (log_level_t)(level));

/**
 * @brief 重新载入配置
 */
void CTLOGAPI CTLogEx_reload();

#ifdef _WIN32
void CTLOGAPI CTLogEx_printfW(const wchar_t *module_name, 
                              log_level_t loglevel, 
                              const char *function_name,
                              const char *file_name,
                              int line,
                              _Printf_format_string_ const wchar_t *pszFormat, ...);
#endif
void CTLOGAPI CTLogEx_printfA(const char *module_name, 
                              enum  log_level_t loglevel, 
                              const char *function_name,
                              const char *file_name,
                              int line,
                              _Printf_format_string_ const char *pszFormat, ...)
                              __attribute__((__format__(__printf__,6,7)));
#ifdef _UNICODE 
#define CTLogEx_printf CTLogEx_printfW
#else
#define CTLogEx_printf CTLogEx_printfA
#endif

/**
 * @brief 跟踪等级日志输出
 *
 * @param[in] fmt 格式化字符串
 */
#define L_TRACE(...) CTLogEx_printf(MODULE_NAME, LOG_LEVEL_TRACE, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#define L_TRACE_ENTER() L_TRACE(_T("Enter\n"))
#define L_TRACE_LEAVE() L_TRACE(_T("Leave\n"))
#define L_TRACE_EXIT() L_TRACE(_T("Exit\n"))

/**
 * @brief 调试等级日志输出
 *
 * @param[in] fmt 格式化字符串
 */
#define L_DEBUG(...) CTLogEx_printf(MODULE_NAME, LOG_LEVEL_DEBUG, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
/**
 * @brief 信息等级日志输出
 *
 * @param[in] fmt 格式化字符串
 */
#define L_INFO(...)  CTLogEx_printf(MODULE_NAME, LOG_LEVEL_INFO,  __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
/**
 * @brief 警告等级日志输出
 *
 * @param[in] fmt 格式化字符串
 */
#define L_WARN(...)  CTLogEx_printf(MODULE_NAME, LOG_LEVEL_WARN,  __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
/**
 * @brief 错误等级日志输出
 *
 * @param[in] fmt 格式化字符串
 */
#define L_ERROR(...) CTLogEx_printf(MODULE_NAME, LOG_LEVEL_ERROR, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
/**
 * @brief 致命错误等级日志输出
 *
 * @param[in] fmt 格式化字符串
 */
#define L_FATAL(...) CTLogEx_printf(MODULE_NAME, LOG_LEVEL_FATAL, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);


/**
 * @brief 临时调试宏 \n
 * 这一部分信息强制输出，只用于临时调试, 提交到svn的版本不允许有这一个宏
 *
 * @param[in] fmt 格式化字符串
 */
#define L_TEMP(...) CTLogEx_printf(MODULE_NAME, LOG_TYPE_TMP, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);

#ifdef _WIN32
void CTLOGAPI CTLogEx_dumpW(const wchar_t *module_name, int bForce, const char *function_name, const char *file_name, int line, 
                            const void *buf, int len);
#endif
void CTLOGAPI CTLogEx_dumpA(const char *module_name, int bForce, const char *function_name, const char *file_name, int line, 
                            const void *buf, int len);
#ifdef _UNICODE 
#define CTLogEx_dump CTLogEx_dumpW
#else
#define CTLogEx_dump CTLogEx_dumpA
#endif

/**
 * @brief dump数据块
 *
 * @param[in] buf         数据地址
 * @param[in] len         数据块大小
 */
#define L_DUMP(buf, len) CTLogEx_dump(MODULE_NAME, 0, __FUNCTION__, __FILE__, __LINE__, buf, len);

/**
 * @brief dump数据, 只用于临时调试, 提交到svn的版本不允许使用
 *
 * @param[in] buf         数据地址
 * @param[in] len         数据块大小
 */
#define L_TMPDUMP(buf, len) CTLogEx_dump(MODULE_NAME, 1, __FUNCTION__, __FILE__, __LINE__, buf, len);

/**
 * @brief 结束log系统
 */
void CTLOGAPI CTLogEx_done(void);

#ifdef __cplusplus
}
#endif

#endif
