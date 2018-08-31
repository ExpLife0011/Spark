#pragma once

#ifndef __ISOCKET_CLIENT_H__
#define __ISOCKET_CLIENT_H__

#include "Socket/ISocketBase.h"
#include "DllExport.h"

namespace enlib
{
    class DLL_COMMONLIB_API ISocketClient : public virtual ISocketBase
    {
    public:
        virtual BOOL WINAPI Connect() = 0;
    };
};

DLL_COMMONLIB_API enlib::CObjPtr<enlib::ISocketClient> WINAPI CreateISocketClientInstance(const CHAR *address, WORD port);

#endif