/**-----------------------------------------------------------------------------
 * @file     iniparse.h
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
#ifndef __INI_PARSE_H__
#define __INI_PARSE_H__
#include <string>

int ini_get_int(const char *filename, const char *appname, const char *keyname, int *value);
void ini_get_string(const char *filename, const char *appname, 
                    const char *keyname, std::string &value);

#endif
