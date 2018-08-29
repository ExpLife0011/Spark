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

    virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestDataHandle Func);

    virtual void WINAPI RegisterEndHandle(EndHandle Func);

    virtual void WINAPI RegisterConnectHandle(ConnectHandle Func);

    virtual VOID WINAPI SetParam(const CHAR* ParamKeyword, CBaseObjPtr<CBaseObject> Param);

private:
    static BOOL ServiceMainThreadProc(LPVOID Parameter, HANDLE StopEvent);

    void InitalizeServer(CSocketServer* Server);

    IThread*                                    m_pMainThread;
    CHAR                                        m_szSrcAddress[128];
    WORD                                        m_dwSrcPort;
    HANDLE                                      m_hStopEvent;
    CRITICAL_SECTION                            m_csLock; 
    std::map<DWORD, RequestPacketHandle>        m_ReqPacketList;
    std::map<DWORD, RequestDataHandle>          m_ReqDataList;
    std::list<EndHandle>                        m_EndList;
    std::list<ConnectHandle>                    m_ConnectList;
    std::map<UINT32, CBaseObjPtr<CBaseObject>>  m_ParamMap;
    SOCKET                                      m_ListenSocket;
};

#endif