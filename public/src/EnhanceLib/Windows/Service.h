#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <Windows.h>
#include "Windows/IService.h"

int WINAPI ServiceMainLib(ServiceBaseInfo *ServiceInfo);

#endif