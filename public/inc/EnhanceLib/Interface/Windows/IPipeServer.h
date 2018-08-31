#pragma once

#ifndef __IPIPE_SERVER_H__
#define __IPIPE_SERVER_H__

#include <Windows.h>
#include "Common/ICommunication.h"
#include "DllExport.h"
#include "Base/ParamSet.h"

namespace enlib
{
    class DLL_COMMONLIB_API IPipeServer : public virtual ICommunication, public virtual CParamSet
    {
    public:
        virtual BOOL WINAPI Start() = 0;

        virtual void WINAPI Stop() = 0;

        virtual BOOL WINAPI IsConnected() = 0;
    };

    class DLL_COMMONLIB_API IPipeServerService : public virtual CObject
    {
    public:
        virtual BOOL WINAPI StartMainService() = 0;

        virtual void WINAPI StopMainService() = 0;

        virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestPacketHandle Func) = 0;

        virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestDataHandle Func) = 0;

        virtual void WINAPI RegisterEndHandle(EndHandle Func) = 0;

        virtual void WINAPI RegisterConnectHandle(ConnectHandle Func) = 0;

        virtual VOID WINAPI SetParam(const CHAR* ParamKeyword, CObjPtr<CObject> Param) = 0;
    };
};

DLL_COMMONLIB_API enlib::CObjPtr<enlib::IPipeServerService> WINAPI CreateIPipeServerServiceInstance(TCHAR* PipeName, DWORD Timeout, HANDLE StopEvent);


#endif