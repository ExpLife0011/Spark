#pragma once

#ifndef __CVDISK_SERVER_LOG_EX_H__
#define __CVDISK_SERVER_LOG_EX_H__

#include "LibLogEx.h"

typedef enum 
{
    SERVER_SERVICE = LIB_BASE_MAX,
    SERVER_CONNECTION,
    SERVER_VC,
    SERVER_MODULE,
    SERVER_CONF,
    SERVER_DRIVER,
    SERVER_MAX,
};

#define SERVICE_TRACE(...)          LOG_TRACE(SERVER_SERVICE, __VA_ARGS__)
#define SERVICE_DEBUG(...)          LOG_DEBUG(SERVER_SERVICE, __VA_ARGS__)
#define SERVICE_INFO(...)           LOG_INFO(SERVER_SERVICE, __VA_ARGS__)
#define SERVICE_WARN(...)           LOG_WARN(SERVER_SERVICE, __VA_ARGS__)
#define SERVICE_ERROR(...)          LOG_ERROR(SERVER_SERVICE, __VA_ARGS__)
#define SERVICE_FATAL(...)          LOG_FATAL(SERVER_SERVICE, __VA_ARGS__)

#define SERVICE_ENTER()             SERVICE_TRACE(_T("Enter\n"))
#define SERVICE_LEAVE()             SERVICE_TRACE(_T("Leave\n"))

#define CONNECTION_TRACE(...)       LOG_TRACE(SERVER_CONNECTION, __VA_ARGS__)
#define CONNECTION_DEBUG(...)       LOG_DEBUG(SERVER_CONNECTION, __VA_ARGS__)
#define CONNECTION_INFO(...)        LOG_INFO(SERVER_CONNECTION, __VA_ARGS__)
#define CONNECTION_WARN(...)        LOG_WARN(SERVER_CONNECTION, __VA_ARGS__)
#define CONNECTION_ERROR(...)       LOG_ERROR(SERVER_CONNECTION, __VA_ARGS__)
#define CONNECTION_FATAL(...)       LOG_FATAL(SERVER_CONNECTION, __VA_ARGS__)
#define CONNECTION_DUMP(Buff, Len)  LOG_DUMP(SERVER_CONNECTION, Buff, Len)

#define CONNECTION_ENTER()          CONNECTION_TRACE(_T("Enter\n"))
#define CONNECTION_LEAVE()          CONNECTION_TRACE(_T("Leave\n"))

#define VC_TRACE(...)               LOG_TRACE(SERVER_VC, __VA_ARGS__)
#define VC_DEBUG(...)               LOG_DEBUG(SERVER_VC, __VA_ARGS__)
#define VC_INFO(...)                LOG_INFO(SERVER_VC, __VA_ARGS__)
#define VC_WARN(...)                LOG_WARN(SERVER_VC, __VA_ARGS__)
#define VC_ERROR(...)               LOG_ERROR(SERVER_VC, __VA_ARGS__)
#define VC_FATAL(...)               LOG_FATAL(SERVER_VC, __VA_ARGS__)

#define VC_ENTER()                  VC_TRACE(_T("Enter\n"))
#define VC_LEAVE()                  VC_TRACE(_T("Leave\n"))

#define SERVER_CONF_TRACE(...)      LOG_TRACE(SERVER_CONF, __VA_ARGS__)
#define SERVER_CONF_DEBUG(...)      LOG_DEBUG(SERVER_CONF, __VA_ARGS__)
#define SERVER_CONF_INFO(...)       LOG_INFO(SERVER_CONF, __VA_ARGS__)
#define SERVER_CONF_WARN(...)       LOG_WARN(SERVER_CONF, __VA_ARGS__)
#define SERVER_CONF_ERROR(...)      LOG_ERROR(SERVER_CONF, __VA_ARGS__)
#define SERVER_CONF_FATAL(...)      LOG_FATAL(SERVER_CONF, __VA_ARGS__)

#define SERVER_CONF_ENTER()         SERVER_CONF_TRACE(_T("Enter\n"))
#define SERVER_CONF_LEAVE()         SERVER_CONF_TRACE(_T("Leave\n"))

#define SERVER_MODULE_TRACE(...)    LOG_TRACE(SERVER_MODULE, __VA_ARGS__)
#define SERVER_MODULE_DEBUG(...)    LOG_DEBUG(SERVER_MODULE, __VA_ARGS__)
#define SERVER_MODULE_INFO(...)     LOG_INFO(SERVER_MODULE, __VA_ARGS__)
#define SERVER_MODULE_WARN(...)     LOG_WARN(SERVER_MODULE, __VA_ARGS__)
#define SERVER_MODULE_ERROR(...)    LOG_ERROR(SERVER_MODULE, __VA_ARGS__)
#define SERVER_MODULE_FATAL(...)    LOG_FATAL(SERVER_MODULE, __VA_ARGS__)
#define SERVER_MODULE_DUMP(...)     LOG_DUMP(SERVER_MODULE, __VA_ARGS__)

#define SERVER_MODULE_ENTER()       SERVER_MODULE_TRACE(_T("Enter\n"))
#define SERVER_MODULE_LEAVE()       SERVER_MODULE_TRACE(_T("Leave\n"))

#define DRIVER_TRACE(...)           LOG_TRACE(SERVER_DRIVER, __VA_ARGS__)
#define DRIVER_DEBUG(...)           LOG_DEBUG(SERVER_DRIVER, __VA_ARGS__)
#define DRIVER_INFO(...)            LOG_INFO(SERVER_DRIVER, __VA_ARGS__)
#define DRIVER_WARN(...)            LOG_WARN(SERVER_DRIVER, __VA_ARGS__)
#define DRIVER_ERROR(...)           LOG_ERROR(SERVER_DRIVER, __VA_ARGS__)
#define DRIVER_FATAL(...)           LOG_FATAL(SERVER_DRIVER, __VA_ARGS__)

#define DRIVER_ENTER()              DRIVER_TRACE(_T("Enter\n"))
#define DRIVER_LEAVE()              DRIVER_TRACE(_T("Leave\n"))

#endif