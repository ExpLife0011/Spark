/**-----------------------------------------------------------------------------
 * @file     CTLogEx.cpp
 *
 * @author   yangrz@centerm.com.cn
 *
 * @date     2011/2/1
 *
 * @brief    
 *
 * @version  
 *
 *----------------------------------------------------------------------------*/
#include <string>
#include <map>
#ifdef _WIN32
#include "CTLogEx_OsWin.h"
#else
#include "CTLogEx_OsUnix.h"
#endif
#include "CTLogEx.h"

// 默认配置文件
#ifdef _WIN32
#define DEF_CONFIG_FILE _T("logconf.ini")
#else
#define DEF_CONFIG_FILE _T("/etc/Xred.conf.d/logconf.ini")
#endif

#define MAX_CONF_BUF_SIZE 2048

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

using namespace std;

// log 模块配置
class CLogModuleConf
{
public:
    CLogModuleConf(BOOL bDefault);
    CLogModuleConf(CLogModuleConf &conf);

    void CopyIfInvalid(CLogModuleConf &conf);

    log_level_t m_level;          // log等级
    BOOL        m_b_dump;         // 是否输出DUMP信息
    BOOL        m_b_performance_counter; // 显示性能计数器显示时间
    /* 
     * 输出格式 
     */
    tstring     m_trace_format;   
    tstring     m_debug_format;   
    tstring     m_info_format;   
    tstring     m_warn_format;
    tstring     m_error_format;
    tstring     m_fatal_format;
};

class NoCaseCmp
{
    public:
        bool operator() (const tstring &x, const tstring &y) const
        {
            return _tcsicmp(x.c_str(), y.c_str()) < 0;
        }
};

/* log 配置 */
class CLogConf
{
public:
    CLogConf();
    CLogConf(CLogConf &conf);
    ~CLogConf();
    CLogModuleConf     m_defaultconf;   // 默认配置
    map<tstring, CLogModuleConf *, NoCaseCmp> m_moduleconf_table;  // 模块配置表

    tstring             m_prefix;      // 输出前缀

    log_output_type_t   m_output_type; // 输出类型
    /* 
     * 当m_output_type等于LOG_OUTPUT_FILE下面的域有效 
     */
    tstring             m_logfile;       // log文件名
    long                m_max_logsize;   // log文件最大长度, 以字节为单位
};

static volatile LONG g_running = FALSE;
// log 配置
static CLogConf *g_logconf = NULL;
// log 配置文件
static TCHAR    *g_config_file;
// 输出文件
static FILE*     g_logfp = NULL;
// 代码指定的配置
static CLogConf *g_code_logconf;

CLogModuleConf::CLogModuleConf(BOOL bDefault)
{
    if ( bDefault )
    {
        m_b_dump        = FALSE;
        m_b_performance_counter = FALSE;
        m_level         = LOG_LEVEL_INFO;
        m_trace_format  = TRACE_DEFAULT_FMT;
        m_debug_format  = DEBUG_DEFAULT_FMT;
        m_info_format   = INFO_DEFAULT_FMT;
        m_warn_format   = WARN_DEFAULT_FMT;
        m_error_format  = ERROR_DEFAULT_FMT;
        m_fatal_format  = FATAL_DEFAULT_FMT;
    }
    else
    {
        m_b_dump = -1;
        m_level = (log_level_t)-1;
        m_b_performance_counter = -1;
    }
}

void 
CLogModuleConf::CopyIfInvalid(CLogModuleConf &conf)
{
    if ( m_b_dump == -1 )
    {
        m_b_dump = conf.m_b_dump;
    }
    if ( m_b_performance_counter )
    {
        m_b_performance_counter = conf.m_b_performance_counter;
    }
    if ( m_level == -1 )
    {
        m_level = conf.m_level;
    }
    if ( m_trace_format.empty() )
    {
        m_trace_format = conf.m_trace_format;
    }
    if ( m_debug_format.empty() )
    {
        m_debug_format = conf.m_debug_format;
    }
    if ( m_info_format.empty() )
    {
        m_info_format = conf.m_info_format;
    }
    if ( m_warn_format.empty() )
    {
        m_warn_format = conf.m_warn_format;
    }
    if ( m_error_format.empty() )
    {
        m_error_format = conf.m_error_format;
    }
    if ( m_fatal_format.empty() )
    {
        m_fatal_format = conf.m_fatal_format;
    }
}

CLogModuleConf::CLogModuleConf(CLogModuleConf &conf)
{
    m_b_dump        = conf.m_b_dump;
    m_level         = conf.m_level;
    m_trace_format  = conf.m_trace_format;
    m_debug_format  = conf.m_debug_format;
    m_info_format   = conf.m_info_format;
    m_warn_format   = conf.m_warn_format;
    m_error_format  = conf.m_error_format;
    m_fatal_format  = conf.m_fatal_format;
    m_b_performance_counter = conf.m_b_performance_counter;
}

CLogConf::CLogConf(): m_defaultconf(TRUE)
{
    m_prefix = _T("");
#ifdef _WIN32
    m_output_type = LOG_OUTPUT_DBGPORT;
#else
    m_output_type = LOG_OUTPUT_STDERR;
#endif
    m_logfile = Log_OsGetFullPathNameRelativeExec(_T("ctlogex.log"));
    m_max_logsize = 1024 * 1024 * 10;
}

CLogConf::CLogConf(CLogConf &conf): m_defaultconf(FALSE)
{
    map<tstring, CLogModuleConf *, NoCaseCmp>::iterator itor;

    m_defaultconf   = conf.m_defaultconf;
    m_prefix        = conf.m_prefix;
    m_output_type   = conf.m_output_type;
    m_logfile       = conf.m_logfile;
    m_max_logsize   = conf.m_max_logsize;

    for ( itor = conf.m_moduleconf_table.begin();
          itor != conf.m_moduleconf_table.end();
          itor++ )
    {
        CLogModuleConf *moduleconf;

        moduleconf = new CLogModuleConf(*(itor->second));
        m_moduleconf_table[itor->first] = moduleconf;
    }
}

CLogConf::~CLogConf()
{
    map<tstring, CLogModuleConf *, NoCaseCmp>::iterator itor;

    /*
     * 清除所有模块配置
     */
    for ( itor = m_moduleconf_table.begin();
          itor != m_moduleconf_table.end();
          itor++ )
    {
        CLogModuleConf *module_conf = itor->second;
        delete module_conf;
    }
}

/*
 * 载入模块配置
 */
static void
LoadModuleConf(CLogModuleConf &moduleconf, const TCHAR *module_name, const TCHAR *confname)
{
    TCHAR buf[MAX_CONF_BUF_SIZE];

    moduleconf.m_level = (log_level_t)GetPrivateProfileInt(module_name, _T("log_level"), moduleconf.m_level, confname);
    moduleconf.m_b_dump = GetPrivateProfileInt(module_name, _T("log_dump"), moduleconf.m_b_dump, confname);
    moduleconf.m_b_performance_counter = 
        GetPrivateProfileInt(module_name, _T("performance_counter"), moduleconf.m_b_performance_counter, confname);
    if ( GetPrivateProfileString(module_name, _T("log_trace_fmt"), NULL, 
                                 buf, sizeof(buf)/sizeof(TCHAR), confname))
    {
        moduleconf.m_trace_format = buf;
    }
    if ( GetPrivateProfileString(module_name, _T("log_debug_fmt"), NULL, 
                                 buf, sizeof(buf)/sizeof(TCHAR), confname))
    {
        moduleconf.m_debug_format = buf;
    }
    if ( GetPrivateProfileString(module_name, _T("log_info_fmt"), NULL, 
                                 buf, sizeof(buf)/sizeof(TCHAR), confname))
    {
        moduleconf.m_info_format = buf;
    }
    if ( GetPrivateProfileString(module_name, _T("log_warn_fmt"), NULL, 
                                 buf, sizeof(buf)/sizeof(TCHAR), confname))
    {
        moduleconf.m_warn_format = buf;
    }
    if ( GetPrivateProfileString(module_name, _T("log_error_fmt"), NULL, 
                                 buf, sizeof(buf)/sizeof(TCHAR), confname))
    {
        moduleconf.m_error_format = buf;
    }
    if ( GetPrivateProfileString(module_name, _T("log_fatal_fmt"), NULL, 
                                 buf, sizeof(buf)/sizeof(TCHAR), confname))
    {
        moduleconf.m_fatal_format = buf;
    }
}

/*
 *  载入log系统的配置文件
 */
static BOOL
LoadLogConf(const TCHAR *config_file)
{
    Log_OsLock();
    {
        TCHAR buf[MAX_CONF_BUF_SIZE];
        map<tstring, CLogModuleConf *, NoCaseCmp>::iterator itor;

        if ( g_logfp )
        {
            fclose(g_logfp);
            g_logfp = NULL;
        }
        if ( g_logconf )
        {
            delete g_logconf;
        }

        g_logconf = new CLogConf(*g_code_logconf);

        if ( GetPrivateProfileString(_T("GLOBAL"), _T("log_prefix"), NULL, 
                                     buf, sizeof(buf)/sizeof(TCHAR), config_file))
        {
            g_logconf->m_prefix = buf;
        }
        g_logconf->m_output_type = (log_output_type_t)GetPrivateProfileInt(_T("GLOBAL"), _T("log_output_type"), 
                                                                           g_logconf->m_output_type, 
                                                                           config_file);
        if ( g_logconf->m_output_type == LOG_OUTPUT_FILE )
        {
            if ( GetPrivateProfileString(_T("GLOBAL"), _T("log_output_filepath"), NULL, 
                                         buf, sizeof(buf)/sizeof(TCHAR), config_file))
            {
                g_logconf->m_logfile = Log_OsGetFullPathNameRelativeExec(buf);
            }
            g_logconf->m_max_logsize = GetPrivateProfileInt(_T("GLOBAL"), _T("log_output_filesize"), 
                                                            g_logconf->m_max_logsize / 1024 / 1024, config_file) 
                                                            * 1024 * 1024;
            if ( g_logconf->m_max_logsize > 0 )
            {
                g_logfp = _tfopen(g_logconf->m_logfile.c_str(), _T("a"));
                if ( g_logfp )
                {
                    // 关闭IO缓存
                    setbuf(g_logfp, NULL);
                }
                else
                {
                    Log_OsShowError(_T("fopen %s fail: %s\n"), g_logconf->m_logfile.c_str(), _tcserror(errno));
                }
            }
        }
        // 载入全局配置
        LoadModuleConf(g_logconf->m_defaultconf, _T("GLOBAL"), config_file);
        /* 
         * 载入代码模块配置参数 
         */
        for ( itor = g_logconf->m_moduleconf_table.begin();
              itor != g_logconf->m_moduleconf_table.end();
              itor++ )
        {
            CLogModuleConf *moduleconf = itor->second;

            moduleconf->CopyIfInvalid(g_logconf->m_defaultconf);
        }
    }
    Log_OsUnlock();

    return TRUE;
}

static inline BOOL 
GrowIfNeed(TCHAR *&out, TCHAR *&out_pos, 
           TCHAR *base, int &max_chars, int new_chars)
{
    if ( (out_pos - out + new_chars) >= (max_chars - 1) ) 
    {
        TCHAR  *old_out = out;
        int new_max_chars;

        new_max_chars = (out_pos - out + 1 + new_chars);
        new_max_chars = ( new_max_chars / max_chars + 1) * max_chars;
        if ( out == base )
        {
            out = (TCHAR *) malloc(new_max_chars * sizeof(TCHAR));
            if ( out == NULL )
            {
                Log_OsShowError(_T("%d: malloc %d fail\n"), __LINE__, new_max_chars * sizeof(TCHAR));
                return FALSE;
            }
            memcpy(out, base, max_chars * sizeof(TCHAR));
        }
        else
        {
            TCHAR *ret = (TCHAR *)realloc(out, new_max_chars * sizeof(TCHAR));
            if ( ret == NULL )
            {
                Log_OsShowError(_T("%d: realloc %d fail\n"), __LINE__, new_max_chars * sizeof(TCHAR));
                return FALSE;
            }
            out = ret;
        }
        out_pos = out + (out_pos - old_out);
        max_chars = new_max_chars;
    }
    return TRUE;
}
#define GROW_IF_NEED(out, out_pos, base, max_chars, new_chars) \
    do { if ( GrowIfNeed(out, out_pos, base, max_chars, new_chars) == FALSE ) return; } while ( 0 );
#define ADD_IF_LG(pos, len) { if ( (len) > 0 ) { (pos) += (len); } }

static inline const TCHAR *
GetLoglevelString(log_level_t level)
{
    const TCHAR *ret = _T("UNKOWN");

    switch ( level )
    {
        case LOG_LEVEL_TRACE:
            ret = _T("TRACE");
            break;
        case LOG_LEVEL_DEBUG:
            ret = _T("DEBUG");
            break;
        case LOG_LEVEL_INFO:
            ret = _T("_INFO");
            break;
        case LOG_LEVEL_WARN:
            ret = _T("_WARN");
            break;
        case LOG_LEVEL_ERROR:
            ret = _T("ERROR");
            break;
        case LOG_LEVEL_FATAL:
            ret = _T("FATAL");
            break;

        case LOG_TYPE_DUMP:
            ret = _T("_DUMP");
            break;
        case LOG_TYPE_TMP:
            ret = _T("_TEMP");
    }
    return ret;
}

static BOOL
GetModuleConf(const TCHAR *module_name, 
              log_level_t loglevel,
              BOOL &b_dump,
              BOOL &b_performance_counter,
              tstring &fmt,
              tstring &output_prefix)
{
    BOOL ret = FALSE;

    /*
     * 临时调试宏强制输出
     */
    if ( loglevel == LOG_TYPE_TMP )
    {
        fmt = TMP_DEFAULT_FMT;
        return TRUE;
    }
    Log_OsLock();
    do
    {
        if ( g_running == FALSE )
        {
            break;
        }
        if ( g_logconf->m_output_type == LOG_OUTPUT_DISABLE )
        {
            break;
        }

        map<tstring, CLogModuleConf *, NoCaseCmp>::iterator itor;
        CLogModuleConf *module_conf;
        itor = g_logconf->m_moduleconf_table.find(module_name);
        if ( itor == g_logconf->m_moduleconf_table.end() )
        {
            module_conf = new CLogModuleConf(g_logconf->m_defaultconf);
            LoadModuleConf(*module_conf, module_name, g_config_file);
            (g_logconf->m_moduleconf_table)[module_name] = module_conf;
        }
        else
        {
            module_conf = itor->second;
        }
        if ( loglevel != LOG_TYPE_DUMP 
             && (loglevel && module_conf->m_level < loglevel) )
        {
            break;
        }
        switch ( loglevel )
        {
            case LOG_LEVEL_TRACE:
                fmt = module_conf->m_trace_format;
                break;
            case LOG_LEVEL_DEBUG:
                fmt = module_conf->m_debug_format;
                break;
            case LOG_LEVEL_INFO:
                fmt = module_conf->m_info_format;
                break;
            case LOG_LEVEL_WARN:
                fmt = module_conf->m_warn_format;
                break;
            case LOG_LEVEL_ERROR:
                fmt = module_conf->m_error_format;
                break;
            case LOG_LEVEL_FATAL:
                fmt = module_conf->m_fatal_format;
                break;
            case LOG_TYPE_DUMP:
                fmt = DUMP_DEFAULT_FMT;
                break;
            default:
                goto EXIT;
        }
        output_prefix = g_logconf->m_prefix;
        b_dump = module_conf->m_b_dump;
        b_performance_counter = module_conf->m_b_performance_counter;
        ret = TRUE;
    }
    while (FALSE);

EXIT:
    Log_OsUnlock();

    return ret;
}

/*
 * 写入log信息
 */
static void
CTLogEx_puts(TCHAR *out_str)
{
    Log_OsLock();
    if ( g_running == FALSE )
    {
        goto EXIT;
    }
    switch ( g_logconf->m_output_type )
    {
        case LOG_OUTPUT_STDERR:
            _ftprintf(stderr, _T("%s"), out_str);
            break;

#ifdef _WIN32
        case LOG_OUTPUT_DBGPORT:
            OutputDebugString(out_str);
            break;
#endif

        case LOG_OUTPUT_FILE:
            if ( g_logfp )
            {
                if ( ftell(g_logfp) > g_logconf->m_max_logsize )
                {
                    TCHAR bak_filename[MAX_CONF_BUF_SIZE];

                    fclose(g_logfp);
                    _sntprintf(bak_filename, sizeof(bak_filename)/sizeof(TCHAR), _T("%s.bak"), g_logconf->m_logfile.c_str());
                    Log_OsMoveFile(g_logconf->m_logfile.c_str(), bak_filename);
                    g_logfp = _tfopen(g_logconf->m_logfile.c_str(), _T("a"));
                    if ( g_logfp )
                    {
                        // 关闭IO缓存
                        setbuf(g_logfp, NULL);
                    }
                    else
                    {
                        Log_OsShowError(_T("fopen %s fail: %s\n"), g_logconf->m_logfile.c_str(), _tcserror(errno));
                    }
                }
                if ( g_logfp )
                {
                    if ( ftell(g_logfp) <= g_logconf->m_max_logsize )
                    {
                        _ftprintf(g_logfp, _T("%s"), out_str);
                    }
                }
            }
            break;

        default:
            break;
    }

EXIT:
    Log_OsUnlock();
}

#ifdef _UNICODE
void CTLOGAPI CTLogEx_printfW
#else
void CTLOGAPI CTLogEx_printfA 
#endif
                (const TCHAR *module_name, 
                log_level_t loglevel, 
                const char *function_name,
                const char *file_name,
                int line,
                const TCHAR *pszFormat, ...)
{
    TCHAR           buf[MAX_CONF_BUF_SIZE];
    TCHAR           *out = buf;
    tstring         fmt;
    tstring         output_prefix;
    BOOL            b_dump;
    BOOL            b_performance_counter;
    int             max_out_chars = sizeof(buf) / sizeof(buf[0]);
    TCHAR          *out_p;

    if ( module_name == NULL || function_name == NULL || file_name == NULL
         || pszFormat == NULL || !g_running )
    {
        return;
    }

    if ( GetModuleConf(module_name, loglevel, b_dump, b_performance_counter, fmt, output_prefix) == FALSE )
    {
        return;
    }

    /* 
     * 解析输出格式 
     */
    {
        const TCHAR *p;

        p = fmt.c_str();
        out_p = out;
        while ( *p )
        {
            BOOL b_match = FALSE;
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
                b_match = TRUE;
                int ret;

                switch ( *(p + 1) )
                {
                    case _T('F'):
                        GROW_IF_NEED(out, out_p, buf, max_out_chars, strlen(file_name));
                        ret = _sntprintf(out_p, max_out_chars - (out_p - out), CHARSTRING_FMT, file_name);
                        ADD_IF_LG(out_p, ret);
                        break;

                    case _T('L'):
                        GROW_IF_NEED(out, out_p, buf, max_out_chars, 16);
                        ret = _sntprintf(out_p, max_out_chars - (out_p - out), _T("%d"), line);
                        ADD_IF_LG(out_p, ret);
                        break;

                    case _T('T'):
                        GROW_IF_NEED(out, out_p, buf, max_out_chars, 16);
                        ret = _sntprintf(out_p, max_out_chars - (out_p - out), _T("%08ld"), Log_CurrentThreadID());
                        ADD_IF_LG(out_p, ret);
                        break;

                    case _T('m'):
                    {
                        va_list args;

#ifdef _WIN32
                        va_start(args, pszFormat);
                        GROW_IF_NEED(out, out_p, buf, max_out_chars, _vscwprintf(pszFormat, args));
                        ret = _vsntprintf(out_p, max_out_chars - (out_p - out), pszFormat, args);
                        ADD_IF_LG(out_p, ret);
                        va_end(args);
#else
                        while ( 1 )
                        {
                            int size = max_out_chars - (out_p - out);

                            va_start(args, pszFormat);
                            ret = vsnprintf(out_p, size, pszFormat, args);
                            va_end(args);
                            /* If that worked, return the string. */
                            if ( ret > -1 && ret < size )
                            {
                                ADD_IF_LG(out_p, ret);
                                break;
                            }
                            /* Else try again with more space. */
                            if ( ret > -1 ) /* glibc 2.1 */
                            {
                                size = ret + 1; /* precisely what is needed */
                            }
                            else      /* glibc 2.0 */
                            {
                                size *= 2; /* twice the old size */
                            }
                            GROW_IF_NEED(out, out_p, buf, max_out_chars, size);
                        }
#endif
                        break;
                    }

                    case _T('M'):
                        GROW_IF_NEED(out, out_p, buf, max_out_chars, _tcslen(module_name));
                        ret = _sntprintf(out_p, max_out_chars - (out_p - out), _T("%s"), module_name);
                        ADD_IF_LG(out_p, ret);
                        break;

                    case _T('f'):
                        GROW_IF_NEED(out, out_p, buf, max_out_chars, strlen(function_name));
                        ret = _sntprintf(out_p, max_out_chars - (out_p - out), CHARSTRING_FMT, function_name);
                        ADD_IF_LG(out_p, ret);
                        break;

                    case _T('p'):
                    {
                        const TCHAR *loglevel_string = GetLoglevelString(loglevel);

                        GROW_IF_NEED(out, out_p, buf, max_out_chars, _tcslen(loglevel_string));
                        ret = _sntprintf(out_p, max_out_chars - (out_p - out), _T("%s"), loglevel_string);
                        ADD_IF_LG(out_p, ret);
                        break;
                    }

                    case _T('d'):
                    {
                        MYSYSTEMTIME time;
                        TCHAR time_buf[128];

                        Log_OsGetTime(&time, b_performance_counter);
                        _sntprintf(time_buf, sizeof(time_buf)/sizeof(TCHAR), 
                                   _T("%02d:%02d:%02d.%06d"), time.wHour, time.wMinute, 
                                   time.dwSecond, time.dwUs);
                        GROW_IF_NEED(out, out_p, buf, max_out_chars, _tcslen(time_buf));
                        ret = _sntprintf(out_p, max_out_chars - (out_p - out), _T("%s"), time_buf);
                        ADD_IF_LG(out_p, ret);
                        break;
                    }

                    case _T('P'):
                        GROW_IF_NEED(out, out_p, buf, max_out_chars, _tcslen(output_prefix.c_str()));
                        ret = _sntprintf(out_p, max_out_chars - (out_p - out), _T("%s"), output_prefix.c_str());
                        ADD_IF_LG(out_p, ret);
                        break;

                    default:
                        b_match = FALSE;
                        break;
                }
                if ( b_match )
                {
                    p += 2;
                }
            }
            if ( !b_match )
            {
                GROW_IF_NEED(out, out_p, buf, max_out_chars, 1);
                *out_p = *p;
                out_p++;
                p++;
            }
        }
        GROW_IF_NEED(out, out_p, buf, max_out_chars, 1);
        *out_p = 0;
    }

    /*
     * 写入log信息
     */
    CTLogEx_puts(out);
    if ( out != buf )
    {
        free(out);
    }
}

#ifdef _UNICODE
void CTLOGAPI 
CTLogEx_printfA(const char *module_name, 
                log_level_t loglevel, 
                const char *function_name,
                const char *file_name,
                int line,
                const char *pszFormat, ...)
{
    va_list args;
    char    buf[MAX_CONF_BUF_SIZE];
    char    *out = buf;
    int     len;
    wchar_t module_name_w[MAX_CONF_BUF_SIZE];
    BOOL    b_dump;
    BOOL    b_performance_counter;
    tstring fmt;
    tstring output_prefix;
        
    if ( module_name == NULL || function_name == NULL || file_name == NULL
         || pszFormat == NULL )
    {
        return;
    }

    swprintf(module_name_w, MAX_CONF_BUF_SIZE, L"%S", module_name);
    if ( GetModuleConf(module_name_w, loglevel, b_dump, 
                       b_performance_counter, fmt, output_prefix) == FALSE )
    {
        return;
    }

    va_start(args, pszFormat);
    len = _vscprintf(pszFormat, args) + 1;
    if ( len > MAX_CONF_BUF_SIZE )
    {
        out = (char *)malloc(len);
        if ( out == NULL )
        {
            Log_OsShowError(L"%d: malloc %d fail\n", __LINE__, len);
            va_end(args);
            return;
        }
    }
    if ( vsnprintf(out, len, pszFormat, args) > 0 )
    {
        CTLogEx_printfW(module_name_w, loglevel, function_name, file_name, line, L"%S", out);
    }
    va_end(args);

    if ( out != buf )
    {
        free(out);
    }
}
#endif

#ifdef _UNICODE
void CTLOGAPI CTLogEx_dumpW
#else
void CTLOGAPI CTLogEx_dumpA
#endif
              (const TCHAR *module_name, int b_force, const char *function_name, 
               const char *file_name, int line, const void *buf1, int len)
{
    unsigned char  *buf = (unsigned char *)buf1;
    const int       NB_BYTE = 16;
    char            str[NB_BYTE*10 + 1];
    int             i,j;
    char            c;
    TCHAR           msg[256];
    BOOL            b_dump;
    BOOL            b_performance_counter;
    tstring         fmt;
    tstring         output_prefix;

    if ( module_name == NULL || function_name == NULL 
         || file_name == NULL || buf == NULL || len <= 0 || !g_running )
    {
        return;
    }

    if ( b_force )
    {
        b_dump = TRUE;
    }
    else
    {
        if ( GetModuleConf(module_name, LOG_TYPE_DUMP, b_dump, 
                           b_performance_counter, fmt, output_prefix) == FALSE )
        {
            Log_OsShowError(_T("%d\n"), __LINE__);
            return;
        }
    }

    // 禁止DUMP输出?
    if ( b_dump == FALSE )
    {
        return;
    }

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
        CTLogEx_puts(msg);
    }
    CTLogEx_printf(module_name, LOG_TYPE_DUMP, function_name, file_name, line, 
                   _T("-------------- End Dump --------------\n"));
}

#ifdef _UNICODE
void CTLOGAPI 
CTLogEx_dumpA(const char *module_name, int b_force, const char *function_name, 
              const char *file_name, int line, const void *buf, int len)
{
    wchar_t module_name_w[MAX_CONF_BUF_SIZE];

    if ( module_name == NULL || function_name == NULL 
         || file_name == NULL || buf == NULL || len <= 0 )
    {
        return;
    }
    
    swprintf(module_name_w, MAX_CONF_BUF_SIZE, L"%S", module_name);
    CTLogEx_dumpW(module_name_w, b_force, function_name, file_name, line, buf, len);
}
#endif

#ifdef _UNICODE
void CTLOGAPI CTLogEx_SetOptStrW
#else
void CTLOGAPI CTLogEx_SetOptStrA
#endif
                   (const TCHAR *modulename, 
                    const TCHAR *filedname, 
                    const TCHAR *value)
{
    CLogModuleConf *moduleconf;
    BOOL           bModify = TRUE;

    if ( modulename == NULL || filedname == NULL || value == NULL || !g_running )
    {
        return;
    }

    Log_OsLock();
    if ( g_running == FALSE )
    {
        goto EXIT;
    }
    // 全局配置??
    if ( _tcsicmp(modulename, _T("GLOBAL")) == 0 )
    {
        if ( _tcsicmp(filedname, _T("log_output_type")) == 0 )
        {
            g_code_logconf->m_output_type = (log_output_type_t)_ttoi(value);
            goto EXIT;
        }
        else if ( _tcsicmp(filedname, _T("log_output_filepath")) == 0 )
        {
            g_code_logconf->m_logfile = Log_OsGetFullPathNameRelativeExec(value);
            goto EXIT;
        }
        else if ( _tcsicmp(filedname, _T("log_output_filesize")) == 0 )
        {
            g_code_logconf->m_max_logsize = _ttoi(value) * 1024 * 1024;
            goto EXIT;
        }
        else if ( _tcsicmp(filedname, _T("log_prefix")) == 0 )
        {
            g_code_logconf->m_prefix = value;
            goto EXIT;
        }
        else
        {
            moduleconf = &(g_code_logconf->m_defaultconf);
        }
    }
    else
    {
        map<tstring, CLogModuleConf *, NoCaseCmp>::iterator itor;

        itor = g_code_logconf->m_moduleconf_table.find(modulename);
        if ( itor == g_code_logconf->m_moduleconf_table.end() )
        {
            moduleconf = new CLogModuleConf(FALSE);
            (g_code_logconf->m_moduleconf_table)[modulename] = moduleconf;
        }
        else
        {
            moduleconf = itor->second;
        }
    }

    if ( _tcsicmp(filedname, _T("log_level")) == 0 )
    {
        moduleconf->m_level = (log_level_t)_ttoi(value);
    }
    else if ( _tcsicmp(filedname, _T("log_dump")) == 0 )
    {
        moduleconf->m_b_dump = _ttoi(value);
    }
    else if ( _tcsicmp(filedname, _T("performance_counter")) == 0 )
    {
        moduleconf->m_b_performance_counter = _ttoi(value);
    }
    else if ( _tcsicmp(filedname, _T("log_trace_fmt")) == 0 )
    {
        moduleconf->m_trace_format = value;
    }
    else if ( _tcsicmp(filedname, _T("log_debug_fmt")) == 0 )
    {
        moduleconf->m_debug_format = value;
    }
    else if ( _tcsicmp(filedname, _T("log_info_fmt")) == 0 )
    {
        moduleconf->m_info_format = value;
    }
    else if ( _tcsicmp(filedname, _T("log_warn_fmt")) == 0 )
    {
        moduleconf->m_warn_format = value;
    }
    else if ( _tcsicmp(filedname, _T("log_error_fmt")) == 0 )
    {
        moduleconf->m_error_format = value;
    }
    else if ( _tcsicmp(filedname, _T("log_fatal_fmt")) == 0 )
    {
        moduleconf->m_fatal_format = value;
    }
    else
    {
        bModify = FALSE;
    }

EXIT:
    Log_OsUnlock();

    if ( bModify )
    {
        // 重新载入配置
        LoadLogConf(g_config_file);
    }
    return;
}

#ifdef UNICODE
void CTLOGAPI 
CTLogEx_SetOptStrA(const char *modulename, 
                   const char *filedname, 
                   const char *value)
{
    if ( modulename && filedname && value )
    {
        wchar_t modulename_w[MAX_CONF_BUF_SIZE];
        wchar_t filedname_w[MAX_CONF_BUF_SIZE];
        wchar_t value_w[MAX_CONF_BUF_SIZE];

        swprintf(modulename_w, MAX_CONF_BUF_SIZE, L"%S", modulename);
        swprintf(filedname_w, MAX_CONF_BUF_SIZE, L"%S", filedname);
        swprintf(value_w, MAX_CONF_BUF_SIZE, L"%S", value);

        CTLogEx_SetOptStrW(modulename_w, filedname_w, value_w);
    }
}
#endif

#ifdef _UNICODE
void CTLOGAPI CTLogEx_SetOptIntW
#else
void CTLOGAPI CTLogEx_SetOptIntA
#endif
                  (const TCHAR *modulename, 
                   const TCHAR *filedname, 
                   int value)
{
    TCHAR buf[64];

    _sntprintf(buf, sizeof(buf)/sizeof(TCHAR), _T("%d"), value);
    CTLogEx_SetOptStr(modulename, filedname, buf);
}

#ifdef _UNICODE
void CTLOGAPI 
CTLogEx_SetOptIntA(const char *modulename, 
                   const char *filedname, 
                   int value)
{
    if ( modulename && filedname )
    {
        wchar_t modulename_w[MAX_CONF_BUF_SIZE];
        wchar_t filedname_w[MAX_CONF_BUF_SIZE];

        swprintf(modulename_w, MAX_CONF_BUF_SIZE, L"%S", modulename);
        swprintf(filedname_w, MAX_CONF_BUF_SIZE, L"%S", filedname);

        CTLogEx_SetOptIntW(modulename_w, filedname_w, value);
    }
}
#endif

#ifdef _UNICODE
int CTLOGAPI CTLogEx_initExW
#else
int CTLOGAPI CTLogEx_initExA
#endif
        (const TCHAR *config_file, int flags)
{
    // 不允许重复初始化
    if ( g_running != FALSE )
    {
        int bSame = 1;
        if ( g_config_file != config_file )
        {
            if ( g_config_file && config_file )
            {
                tstring fullpath = Log_OsGetFullPathNameRelativeExec(config_file);
                bSame = (_tcsicmp(g_config_file, fullpath.c_str()) == 0);
            }
            else
            {
                bSame = 0;
            }
        }
             
        if ( !bSame )
        {
            Log_OsShowError(_T("error: recall CTLogEx_init, use org config_file %s\n"),
                            g_config_file ? g_config_file : _T("NULL"));
        }
        return FALSE;
    }
    g_running = TRUE;

    tstring fullpath = Log_OsGetFullPathNameRelativeExec(config_file ? config_file : DEF_CONFIG_FILE);
    if ( fullpath.empty() )
    {
        Log_OsShowError(_T("Log_OsGetFullPathNameRelativeExec fail\n"));
    }
    else
    {
        g_config_file = _tcsdup(fullpath.c_str());
        if ( g_config_file == NULL )
        {
            Log_OsShowError(_T("%d:_tcsdup fail\n"), __LINE__);
            g_running = FALSE;
            return FALSE;
        }
    }

    if ( config_file )
    {
        Log_OsInit(g_config_file, flags);
    }
    else
    {
        Log_OsInit(NULL, flags);
    }
    g_code_logconf = new CLogConf;
    LoadLogConf(g_config_file);

    return TRUE;
}

#ifdef _UNICODE
BOOL CTLOGAPI 
CTLogEx_initExA(const char *config_file, int flags)
{
    if ( config_file )
    {
        wchar_t config_file_w[MAX_CONF_BUF_SIZE];

        swprintf(config_file_w, sizeof(config_file_w)/sizeof(config_file_w[0]), 
                 L"%S", config_file);
        return CTLogEx_initExW(config_file_w, flags);
    }
    else
    {
        return CTLogEx_initExW(NULL, flags);
    }
}
#endif

#ifdef _UNICODE
int CTLOGAPI CTLogEx_initW
#else
int CTLOGAPI CTLogEx_initA
#endif
        (const TCHAR *config_file)
{
    return CTLogEx_initEx(config_file, 0);
}

#ifdef _UNICODE
BOOL CTLOGAPI 
CTLogEx_initA(const char *config_file)
{
    return CTLogEx_initExA(config_file, 0);
}
#endif

void CTLOGAPI
CTLogEx_reload()
{
    LoadLogConf(g_config_file);
}

void CTLOGAPI 
CTLogEx_done(void)
{
    if ( g_running != FALSE )
    {
        g_running = FALSE;
        Log_OsLock();
        {
            Log_OsDone();
            if ( g_logconf )
            {
                delete g_logconf;
                g_logconf = NULL;
            }
            if ( g_config_file )
            {
                free(g_config_file);
                g_config_file = NULL;
            }
            if ( g_logfp )
            {
                fclose(g_logfp);
                g_logfp = NULL;
            }
            if ( g_code_logconf )
            {
                delete g_code_logconf;
                g_code_logconf = NULL;
            }
        }
        Log_OsUnlock();
    }
}
