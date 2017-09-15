#include "Socket/ISocketClient.h"
#include "Socket/ISocketServer.h"
#include "Socket/SocketClient.h"
#include "Socket/SocketServer.h"

ISocketClient* WINAPI CreateISocketClientInstance(const CHAR *address, WORD port)
{
    return new CSocketClient(address, port);
}

ISocketServerService* WINAPI CreateISocketServerServiceInstance(WORD Port, HANDLE StopEvent)
{
    return new CSocketServerService(Port, StopEvent);
}
