#ifndef __ISERVICE_H__
#define __ISERVICE_H__

#include <Windows.h>
#include <DllExport.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

typedef BOOL (WINAPI *IServiceFunc)();

typedef struct
{
    BOOL         ConsoloMode;
    WCHAR*       ServiceName;
    WCHAR*       LogConfigPath;
    WCHAR*       LogModuleName;
    IServiceFunc StartFun;
    IServiceFunc StopFun;
} ServiceBaseInfo;

#pragma pack()

DLL_COMMONLIB_API int WINAPI ServiceMain(ServiceBaseInfo *ServiceInfo);

#ifdef __cplusplus
}
#endif

#endif
