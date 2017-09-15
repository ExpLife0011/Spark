#ifndef __SOCKET_SERVER_H__
#define __SOCKET_SERVER_H__

#include "Socket/ISocketServer.h"
#include "Socket/SocketBase.h"

class CSocketServer : public CSocketBase, public ISocketServer
{
public:
    CSocketServer(SOCKET socket, const CHAR* DstAddress, WORD DstPort);

    ~CSocketServer();

    virtual BOOL WINAPI Start();

private:
    //release self when disconnected
    static void SelfRelease(ICommunication* param);
};

class CSocketServerService : public ISocketServerService
{
public:
    CSocketServerService(WORD Port, HANDLE StopEvent);

    ~CSocketServerService();

    virtual BOOL WINAPI StartMainService();

    virtual void WINAPI StopMainService();

    virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestPacketHandle Func);

    virtual void WINAPI RegisterEndHandle(EndHandle Func);

    virtual VOID WINAPI SetParam(PVOID Param);

private:
    static BOOL ServiceMainThreadProc(LPVOID Parameter, HANDLE StopEvent);

    void InitalizeServer(CSocketServer* Server);

    IThread*                             m_pMainThread;
    CHAR                                 m_szSrcAddress[128];
    WORD                                 m_dwSrcPort;
    HANDLE                               m_hStopEvent;
    CRITICAL_SECTION                     m_csLock;
    std::map<DWORD, RequestPacketHandle> m_ReqPacketList;
    std::list<EndHandle>                 m_EndList;
    PVOID                                m_pParam;
    SOCKET                               m_ListenSocket;
};

#endif