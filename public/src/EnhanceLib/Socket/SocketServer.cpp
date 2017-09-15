#include "StdAfx.h"
#include "SocketServer.h"
#include "SocketHelper.h"

CSocketServer::CSocketServer(SOCKET socket, const CHAR *address, WORD port) : CSocketBase()
{
    strncpy(m_szSrcAddress, address, 128);
    m_dwSrcPort = port;

    m_socket = socket;
}

CSocketServer::~CSocketServer()
{

}

BOOL WINAPI CSocketServer::Start()
{
    struct sockaddr_in  SrcAddress;

    int len = sizeof(struct sockaddr_in);
    if (getpeername(m_socket, (struct sockaddr*)&SrcAddress, &len) >= 0)
    {
        m_dwDstPort = SrcAddress.sin_port;
        strcpy(m_szDstAddress, inet_ntoa(SrcAddress.sin_addr));
    }

    RegisterEndHandle(CSocketServer::SelfRelease);

    Open();

    return TRUE;
}

void CSocketServer::SelfRelease(ICommunication* param)
{
    CSocketBase *Socket = dynamic_cast<CSocketBase *>(param);

    Socket->Release();
}

CSocketServerService::CSocketServerService(WORD Port, HANDLE StopEvent) : CBaseObject()
{
    m_dwSrcPort = Port;
    m_hStopEvent = StopEvent;
    m_ListenSocket = INVALID_SOCKET;

    strcpy(m_szSrcAddress, "127.0.0.1");

    AddRef();
    m_pMainThread = CreateIThreadInstance(CSocketServerService::ServiceMainThreadProc, this);

    InitializeCriticalSection(&m_csLock);
}

CSocketServerService::~CSocketServerService()
{
    StopMainService();

    m_pMainThread->Release();

    m_pMainThread = NULL;

    DeleteCriticalSection(&m_csLock);
}

BOOL WINAPI CSocketServerService::StartMainService()
{
    int Ret;
    int flag;
    struct sockaddr_in sockAddr;
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET)
    {
        return FALSE;
    }

    flag = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(int));

    sockAddr.sin_addr.s_addr = INADDR_ANY;
    sockAddr.sin_port = htons(m_dwSrcPort);
    sockAddr.sin_family = AF_INET;
    Ret = bind(sock, (struct sockaddr *)&sockAddr, sizeof(sockAddr));

    if (Ret < 0)
    {
        closesocket(sock);
        return FALSE;
    }

    listen(sock, 0x10);

    m_ListenSocket = sock;

    m_pMainThread->StartMainThread();

    return TRUE;
}

void WINAPI CSocketServerService::StopMainService()
{
    if (m_ListenSocket != INVALID_SOCKET)
    {
        shutdown(m_ListenSocket, SD_BOTH);
        closesocket(m_ListenSocket);
        m_ListenSocket = INVALID_SOCKET;
    }

    if (m_pMainThread->IsMainThreadRunning())
    {
        m_pMainThread->StopMainThread();
    }
}

BOOL WINAPI CSocketServerService::RegisterRequestHandle(DWORD Type, RequestPacketHandle Func)
{
    EnterCriticalSection(&m_csLock);
    if (m_ReqPacketList.find(Type) != m_ReqPacketList.end())
    {
        return FALSE;
    }
    else
    {
        m_ReqPacketList[Type] = Func;
    }
    LeaveCriticalSection(&m_csLock);

    return TRUE;
}

void WINAPI CSocketServerService::RegisterEndHandle(EndHandle Func)
{
    EnterCriticalSection(&m_csLock);
    m_EndList.push_back(Func);
    LeaveCriticalSection(&m_csLock);
}

VOID WINAPI CSocketServerService::SetParam(PVOID Param)
{
    EnterCriticalSection(&m_csLock);
    m_pParam = Param;
    LeaveCriticalSection(&m_csLock);
}

BOOL CSocketServerService::ServiceMainThreadProc(LPVOID Parameter, HANDLE StopEvent)
{
    CSocketServerService* Service = (CSocketServerService*)Parameter;

    while (TRUE)
    {
        DWORD Ret = WaitForSingleObject(StopEvent, 0);
        if (Ret != WAIT_TIMEOUT)
        {
            break;
        }

        SOCKET s = accept(Service->m_ListenSocket, NULL, NULL);

        if (s == NULL || s == INVALID_SOCKET)
        {
            break;
        }

        //when socket server disconnect, it will release itself
        CSocketServer* Server = new CSocketServer(s, Service->m_szSrcAddress, Service->m_dwSrcPort);
        Service->InitalizeServer(Server);
        Server->Start();
    }

    Service->Release();
    return FALSE;
}

void CSocketServerService::InitalizeServer(CSocketServer* Server)
{
    std::map<DWORD, RequestPacketHandle>::iterator ReqPacketIterator;
    std::list<EndHandle>::iterator EndIterator;
    EnterCriticalSection(&m_csLock);
    for (ReqPacketIterator = m_ReqPacketList.begin(); ReqPacketIterator != m_ReqPacketList.end(); ReqPacketIterator++)
    {
        Server->RegisterRequestHandle(ReqPacketIterator->first, ReqPacketIterator->second);
    }

    for (EndIterator = m_EndList.begin(); EndIterator != m_EndList.end(); EndIterator++)
    {
        Server->RegisterEndHandle(*EndIterator);
    }

    Server->SetParam(Server);

    LeaveCriticalSection(&m_csLock);
}