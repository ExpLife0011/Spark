#pragma once

#ifndef __CLIENT_LOG_EX_H__
#define __CLIENT_LOG_EX_H__

#include "ServerLogEx.h"

typedef enum 
{
    CLIENT_PLUGIN = SERVER_MAX,
    CLIENT_CTRL,
    CLIENT_MODULE,
    CLIENT_VC,
    CLIENT_MAX,
};

#define PLUGIN_TRACE(...)           LOG_TRACE(CLIENT_PLUGIN, __VA_ARGS__)
#define PLUGIN_DEBUG(...)           LOG_DEBUG(CLIENT_PLUGIN, __VA_ARGS__)
#define PLUGIN_INFO(...)            LOG_INFO(CLIENT_PLUGIN, __VA_ARGS__)
#define PLUGIN_WARN(...)            LOG_WARN(CLIENT_PLUGIN, __VA_ARGS__)
#define PLUGIN_ERROR(...)           LOG_ERROR(CLIENT_PLUGIN, __VA_ARGS__)
#define PLUGIN_FATAL(...)           LOG_FATAL(CLIENT_PLUGIN, __VA_ARGS__)

#define PLUGIN_ENTER()              PLUGIN_TRACE(_T("Enter\n"))
#define PLUGIN_LEAVE()              PLUGIN_TRACE(_T("Leave\n"))

#define CTRL_TRACE(...)             LOG_TRACE(CLIENT_CTRL, __VA_ARGS__)
#define CTRL_DEBUG(...)             LOG_DEBUG(CLIENT_CTRL, __VA_ARGS__)
#define CTRL_INFO(...)              LOG_INFO(CLIENT_CTRL, __VA_ARGS__)
#define CTRL_WARN(...)              LOG_WARN(CLIENT_CTRL, __VA_ARGS__)
#define CTRL_ERROR(...)             LOG_ERROR(CLIENT_CTRL, __VA_ARGS__)
#define CTRL_FATAL(...)             LOG_FATAL(CLIENT_CTRL, __VA_ARGS__)
#define CTRL_DUMP(...)              LOG_DUMP(CLIENT_CTRL, __VA_ARGS__)

#define CTRL_ENTER()                CTRL_TRACE(_T("Enter\n"))
#define CTRL_LEAVE()                CTRL_TRACE(_T("Leave\n"))

#define CLIENT_MODULE_TRACE(...)    LOG_TRACE(CLIENT_MODULE, __VA_ARGS__)
#define CLIENT_MODULE_DEBUG(...)    LOG_DEBUG(CLIENT_MODULE, __VA_ARGS__)
#define CLIENT_MODULE_INFO(...)     LOG_INFO(CLIENT_MODULE, __VA_ARGS__)
#define CLIENT_MODULE_WARN(...)     LOG_WARN(CLIENT_MODULE, __VA_ARGS__)
#define CLIENT_MODULE_ERROR(...)    LOG_ERROR(CLIENT_MODULE, __VA_ARGS__)
#define CLIENT_MODULE_FATAL(...)    LOG_FATAL(CLIENT_MODULE, __VA_ARGS__)
#define CLIENT_MODULE_DUMP(...)     LOG_DUMP(CLIENT_MODULE, __VA_ARGS__)

#define CLIENT_MODULE_ENTER()       CLIENT_MODULE_TRACE(_T("Enter\n"))
#define CLIENT_MODULE_LEAVE()       CLIENT_MODULE_TRACE(_T("Leave\n"))

#define CLIENT_VC_TRACE(...)        LOG_TRACE(CLIENT_VC, __VA_ARGS__)
#define CLIENT_VC_DEBUG(...)        LOG_DEBUG(CLIENT_VC, __VA_ARGS__)
#define CLIENT_VC_INFO(...)         LOG_INFO(CLIENT_VC, __VA_ARGS__)
#define CLIENT_VC_WARN(...)         LOG_WARN(CLIENT_VC, __VA_ARGS__)
#define CLIENT_VC_ERROR(...)        LOG_ERROR(CLIENT_VC, __VA_ARGS__)
#define CLIENT_VC_FATAL(...)        LOG_FATAL(CLIENT_VC, __VA_ARGS__)

#define CLIENT_VC_ENTER()           CLIENT_VC_TRACE(_T("Enter\n"))
#define CLIENT_VC_LEAVE()           CLIENT_VC_TRACE(_T("Leave\n"))

#endif