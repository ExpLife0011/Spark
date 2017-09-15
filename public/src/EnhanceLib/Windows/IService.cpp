#include "Windows/Service.h"

int WINAPI ServiceMain(ServiceBaseInfo *ServiceInfo)
{
	return ServiceMainLib(ServiceInfo);
}