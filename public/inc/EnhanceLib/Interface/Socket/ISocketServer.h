#pragma once

#ifndef __ISOCKET_SERVER_H__
#define __ISOCKET_SERVER_H__

#include "Socket/ISocketBase.h"
#include "DllExport.h"

class DLL_COMMONLIB_API ISocketServer : public virtual ISocketBase
{
public:
    virtual BOOL WINAPI Start() = 0;
};

class DLL_COMMONLIB_API ISocketServerService : public virtual CBaseObject
{
public:
    virtual BOOL WINAPI StartMainService() = 0;

    virtual void WINAPI StopMainService() = 0;

    virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestPacketHandle Func) = 0;

    virtual void WINAPI RegisterEndHandle(EndHandle Func) = 0;

    virtual VOID WINAPI SetParam(PVOID Param) = 0;
};

extern "C"
{
    DLL_COMMONLIB_API ISocketServerService* WINAPI CreateISocketServerServiceInstance(WORD Port, HANDLE StopEvent);
}

#endif