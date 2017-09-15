#pragma once

#ifndef __LOG_EX_H__
#define __LOG_EX_H__

#ifdef WIN32
#include "LogExBase.h"
#include "LibLogEx.h"
#include "ServerLogEx.h"
#include "ClientLogEx.h"

#else
#include "LogExBase.h"
#include "LibLogEx.h"
//#include "VDiskClientLogEx.h"
#endif

#endif
