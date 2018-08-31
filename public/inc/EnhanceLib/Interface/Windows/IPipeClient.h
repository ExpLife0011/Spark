#pragma once

#ifndef __IPIPE_CLIENT_H__
#define __IPIPE_CLIENT_H__

#include "Common/ICommunication.h"
#include "DllExport.h"
#include "Base/ParamSet.h"

namespace enlib
{
    class DLL_COMMONLIB_API IPipeClient : public virtual ICommunication, public virtual CParamSet
    {
    public:
        virtual BOOL WINAPI Connect() = 0;

        virtual void WINAPI DisConnect() = 0;
    
        virtual BOOL WINAPI IsConnected() = 0;
    };
};

DLL_COMMONLIB_API enlib::CObjPtr<enlib::IPipeClient> WINAPI CreateIPipeClientInstance(TCHAR *PipeName, DWORD Timeout);

#endif