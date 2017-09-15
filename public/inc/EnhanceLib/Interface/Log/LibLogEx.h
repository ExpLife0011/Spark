#pragma once

#ifndef __LIB_LOG_EX_H__
#define __LIB_LOG_EX_H__

#include "LogExBase.h"

enum 
{
    LIB_BASE           = 0,
    LIB_COMMUNICATE,
    LIB_BASE_MAX,
};

#define LIB_BASE_TRACE(...)         LOG_TRACE(LIB_BASE, __VA_ARGS__)
#define LIB_BASE_DEBUG(...)         LOG_DEBUG(LIB_BASE, __VA_ARGS__)
#define LIB_BASE_INFO(...)          LOG_INFO(LIB_BASE, __VA_ARGS__)
#define LIB_BASE_WARN(...)          LOG_WARN(LIB_BASE, __VA_ARGS__)
#define LIB_BASE_ERROR(...)         LOG_ERROR(LIB_BASE, __VA_ARGS__)
#define LIB_BASE_FATAL(...)         LOG_FATAL(LIB_BASE, __VA_ARGS__)

#define LIB_BASE_ENTER()            LIB_BASE_TRACE(_T("Enter\n"))
#define LIB_BASE_LEAVE()            LIB_BASE_TRACE(_T("Leave\n"))

#define LIB_COMMUNICATE_TRACE(...)  LOG_TRACE(LIB_COMMUNICATE, __VA_ARGS__)
#define LIB_COMMUNICATE_DEBUG(...)  LOG_DEBUG(LIB_COMMUNICATE, __VA_ARGS__)
#define LIB_COMMUNICATE_INFO(...)   LOG_INFO(LIB_COMMUNICATE, __VA_ARGS__)
#define LIB_COMMUNICATE_WARN(...)   LOG_WARN(LIB_COMMUNICATE, __VA_ARGS__)
#define LIB_COMMUNICATE_ERROR(...)  LOG_ERROR(LIB_COMMUNICATE, __VA_ARGS__)
#define LIB_COMMUNICATE_FATAL(...)  LOG_FATAL(LIB_COMMUNICATE, __VA_ARGS__)

#define LIB_COMMUNICATE_ENTER()     LIB_COMMUNICATE_TRACE(_T("Enter\n"))
#define LIB_COMMUNICATE_LEAVE()     LIB_COMMUNICATE_TRACE(_T("Leave\n"))

#endif
