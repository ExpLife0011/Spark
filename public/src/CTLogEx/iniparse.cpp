/**-----------------------------------------------------------------------------
 * @file     iniparse.cpp
 *
 * @author   yangrz@centerm.com.cn
 *
 * @date     2011-02-21
 *
 * @brief    
 *
 * @version  
 *
 *----------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <map>
#include <string>

/* 全局session名称，使用UUID避免冲突 */
#define DEFAULT_GLOBAL_SESSION_NAME "849a6413-0cc1-431a-80bd-5e755c620a67"

using namespace std;

typedef map<string, string> ini_subdic_t;
typedef map<string, ini_subdic_t *> ini_dic_t;

/*
 *  空白符?
 */
#define IS_SPACE(p) ((p) == ' ' || (p) == '\t' )

static int
ini_parse_line(const char *line, char **session_name, char **name, char **value)
{
    const char *name_start, *name_end, *value_start, *value_end = NULL;
    const char *p;
    int  len;
    int  has_value = 0;

    *session_name = NULL;
    *name = NULL;
    *value = NULL;
    if ( line == NULL || *line == 0 )
    {
        return -1;
    }

    name_start = line;
    /* 清除前面的空白符 */
    while ( *name_start && IS_SPACE(*name_start) )
    {
        name_start++;
    }
    /* 空行? */
    if ( *name_start == 0 )
    {
        return -1;
    }
    /* 注释? */
    if ( *name_start == ';' )
    {
        return -1;
    }

    /* session name */
    if ( *name_start == '[' )
    {
        name_end = strchr(name_start, ']');
        if ( name_end == NULL )
        {
            goto ERROR;
        }
        name_start++;
        name_end--;
        /* 清除前面的空格 */
        while ( IS_SPACE(*name_start) && name_end >= name_start )
        {
            name_start++;
        }
        /* 清除后面的空格 */
        while ( IS_SPACE(*name_end) && name_end >= name_start )
        {
            name_end--;
        }
        if ( name_start > name_end )
        {
            *session_name = strdup(DEFAULT_GLOBAL_SESSION_NAME);
            return (*session_name != NULL) ? 0 : -1;
        }
        else
        {
            *session_name = (char *)calloc(name_end - name_start + 2, 1);
            if ( *session_name == NULL )
            {
                return -1;
            }
            memcpy(*session_name, name_start, name_end - name_start + 1);
        }
        return 0;
    }
    else
    {
        p = name_start;
        while ( *p && *p != '=' )
        {
            p++;
        }
        if ( *p == 0 ) 
        {
            goto ERROR;
        }
        /* 清除后面的空白符 */
        name_end = p - 1;
        while ( IS_SPACE(*name_end) && name_end >= name_start )
        {
            name_end--;
        }
        if ( name_end < name_start )
        {
            goto ERROR;
        }

        value_start = p + 1;

        /* 清除前面的空白符 */
        while ( value_start && IS_SPACE(*value_start) )
        {
            value_start++;
        }
        /* 有值? */
        if ( *value_start != 0 )
        {
            value_end = line + strlen(line) - 1;
            /* 清除后面的空白符 */
            while ( value_end >= value_start && IS_SPACE(*value_end) )
            {
                value_end--;
            }
            if ( value_end >= value_start )
            {
                has_value = 1;
            }
        }

        len = name_end - name_start + 1;
        *name = (char *)malloc(len + 1);
        if ( *name == NULL )
        {
            return -1;
        }
        memcpy(*name, name_start, len);
        (*name)[len] = 0;

        if ( has_value )
        {
            len = value_end - value_start + 1;
            *value = (char *)malloc(len + 1);
            if ( *value == NULL )
            {
                return -1;
            }
            memcpy(*value, value_start, len);
            (*value)[len] = 0;
        }
        else
        {
            *value = NULL;
        }

        return 0;
    }

ERROR:
    return -1;
}

ini_dic_t *
ini_load_file(const char *file)
{
    FILE *fp;
    char line[2048];
    char *session_name, *new_session_name;
    ini_dic_t *dic;

    fp = fopen(file, "ra");
    if ( fp == NULL )
    {
        return NULL;
    }

    session_name = strdup(DEFAULT_GLOBAL_SESSION_NAME);
    if ( session_name == NULL )
    {
        return NULL;
    }
    dic = new ini_dic_t;

    while ( 1 )
    {
        char *name, *value;
        int  len;

        memset(line, 0, sizeof(line));
        if ( fgets(line, sizeof(line), fp) == NULL )
        {
            break;
        }
        len = strlen(line);
        if ( len > 0 )
        {
            if ( line[len-1] == '\n' )
            {
                line[len-1] = 0;
            }

			if ( line[len-2] == '\r' )
			{
			    line[len-2] = 0;
			}
        }
        if ( ini_parse_line(line, &new_session_name, &name, &value) >= 0 )
        {
            if ( new_session_name )
            {
                free(session_name);
                session_name = new_session_name;
            }
            else
            {
                ini_dic_t::iterator itor;
                ini_subdic_t        *subdic;

                itor = dic->find(session_name);
                if ( itor == dic->end() )
                {
                    subdic = new ini_subdic_t;
                    (*dic)[session_name] = subdic;
                }
                else
                {
                    subdic = itor->second;
                }

                /*
                 * 重复时强制使用第一个，保持和windows一致
                 */
                if ( value && subdic->find(name) == subdic->end() )
                {
                    (*subdic)[name] = value;
                }
                if ( value )
                {
                    free(value);
                }
                if ( name )
                {
                    free(name);
                }
            }
        }
    }
    if ( session_name  )
    {
        free(session_name);
    }

    fclose(fp);

    return dic;
}

void
ini_done(ini_dic_t *dic)
{
    ini_dic_t::iterator itor;

    if ( dic )
    {
        for ( itor = dic->begin(); itor != dic->end(); itor++ )
        {
            ini_subdic_t *subdic;
            subdic = itor->second;
            delete subdic;
        }
        delete dic;
    }
}

void
ini_get_string(const char *filename, const char *appname, 
               const char *keyname, std::string &value)
{
    ini_dic_t *dic;
    ini_dic_t::iterator itor;
    ini_subdic_t *subdic;
    ini_subdic_t::iterator subitor;

    if ( appname == NULL )
    {
        appname = DEFAULT_GLOBAL_SESSION_NAME;
    }
    value = "";
    dic = ini_load_file(filename);
    if ( dic == NULL )
    {
        return;
    }
    itor = dic->find(appname);
    if ( itor == dic->end() )
    {
        goto EXIT;
    }

    subdic = itor->second;
    subitor = subdic->find(keyname);
    if ( subitor == subdic->end() )
    {
        goto EXIT;
    }
    value = subitor->second;

EXIT:
    ini_done(dic);
}

int
ini_get_int(const char *filename, const char *appname, 
             const char *keyname, int *value)
{
    string strvalue;
    long   lvalue;

    ini_get_string(filename, appname, keyname, strvalue);
    if ( strvalue.empty() )
    {
        return -1;
    }

    errno = 0;
    lvalue = strtol(strvalue.c_str(), (char **)0, 10);
    if ( errno != 0 )
    {
        return -1;
    }
    *value = (int)lvalue;

    return 0;
}
