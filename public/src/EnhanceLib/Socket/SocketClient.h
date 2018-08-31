#ifndef __SOCKET_CLIENT_H__
#define __SOCKET_CLIENT_H__

#include "Socket/ISocketClient.h"
#include "Socket/SocketBase.h"

namespace enlib
{
    class CSocketClient : public CSocketBase, public ISocketClient
    {
    public:
        CSocketClient(const CHAR *address, WORD port);

        ~CSocketClient();

        virtual BOOL WINAPI Connect();
    };
};

#endif