#include "Socket/ISocketClient.h"
#include "Socket/ISocketServer.h"
#include "Socket/SocketClient.h"
#include "Socket/SocketServer.h"

using namespace enlib;

CObjPtr<ISocketClient> WINAPI CreateISocketClientInstance(const CHAR *address, WORD port)
{
    CObjPtr<ISocketClient> pRet = NULL;
    ISocketClient* pClient = NULL;
    pClient = new CSocketClient(address, port);
    if (pClient)
    {
        pRet = pClient;
        pClient->Release();
        pClient = NULL;
    }

    return pRet;
}

CObjPtr<ISocketServerService> WINAPI CreateISocketServerServiceInstance(WORD Port, HANDLE StopEvent)
{
    CObjPtr<ISocketServerService> pRet = NULL;
    ISocketServerService* pService = NULL;
    pService = new CSocketServerService(Port, StopEvent);

    if (pService)
    {
        pRet = pService;
        pService->Release();
        pService = NULL;
    }

    return pRet;
}
