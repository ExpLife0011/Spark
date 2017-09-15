#pragma once

#ifndef __IPIPE_SERVER_H__
#define __IPIPE_SERVER_H__

#include <Windows.h>
#include "Common/ICommunication.h"
#include "DllExport.h"

class DLL_COMMONLIB_API IPipeServer : public virtual ICommunication
{
public:
    virtual BOOL WINAPI Start() = 0;

    virtual void WINAPI Stop() = 0;

    virtual BOOL WINAPI IsConnected() = 0;

    virtual PVOID WINAPI GetParam() = 0;

	virtual VOID WINAPI SetParam(PVOID Param) = 0;
};

class DLL_COMMONLIB_API IPipeServerService : public virtual CBaseObject
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
    DLL_COMMONLIB_API IPipeServerService* WINAPI CreateIPipeServerServiceInstance(TCHAR* PipeName, DWORD Timeout, HANDLE StopEvent);
}

#endif