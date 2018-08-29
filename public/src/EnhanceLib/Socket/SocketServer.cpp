#include "StdAfx.h"
#include "SocketServer.h"
#include "SocketHelper.h"
#include "Base/SuperHash.h"

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

BOOL CSocketServerService::StartMainService()
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

void CSocketServerService::StopMainService()
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

BOOL CSocketServerService::RegisterRequestHandle(DWORD Type, RequestPacketHandle Func)
{
    EnterCriticalSection(&m_csLock);
    if ((m_ReqPacketList.find(Type) != m_ReqPacketList.end())
        || (m_ReqDataList.find(Type) != m_ReqDataList.end()))
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

BOOL CSocketServerService::RegisterRequestHandle(DWORD Type, RequestDataHandle Func)
{
    EnterCriticalSection(&m_csLock);
    if ((m_ReqPacketList.find(Type) != m_ReqPacketList.end())
        || (m_ReqDataList.find(Type) != m_ReqDataList.end()))
    {
        return FALSE;
    }
    else
    {
        m_ReqDataList[Type] = Func;
    }
    LeaveCriticalSection(&m_csLock);

    return TRUE;
}

void CSocketServerService::RegisterEndHandle(EndHandle Func)
{
    EnterCriticalSection(&m_csLock);
    m_EndList.push_back(Func);
    LeaveCriticalSection(&m_csLock);
}

void CSocketServerService::RegisterConnectHandle(ConnectHandle Func)
{
    EnterCriticalSection(&m_csLock);
    m_ConnectList.push_back(Func);
    LeaveCriticalSection(&m_csLock);
}

VOID CSocketServerService::SetParam(const CHAR* ParamKeyword, CBaseObjPtr<CBaseObject> Param)
{
	UINT32 uHash = SuperFastHash(ParamKeyword, strlen(ParamKeyword), 1);
   
    EnterCriticalSection(&m_csLock);
	m_ParamMap[uHash] = Param;
    LeaveCriticalSection(&m_csLock);

    return;
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

void CSocketServerService::InitalizeServer(CSocketServer *Server)
{
    std::map<DWORD, RequestPacketHandle>::iterator ReqPacketIterator;
    std::map<DWORD, RequestDataHandle>::iterator ReqDataIterator;
    std::map<UINT32, CBaseObjPtr<CBaseObject>>::iterator ParamIterator;
    std::list<ConnectHandle>::iterator ConnectIterator;
    std::list<EndHandle>::iterator EndIterator;

    EnterCriticalSection(&m_csLock);

    for (ParamIterator = m_ParamMap.begin(); ParamIterator != m_ParamMap.end(); ParamIterator++)
    {
        Server->SetParam(ParamIterator->first, ParamIterator->second);
    }

    for (ConnectIterator = m_ConnectList.begin(); ConnectIterator != m_ConnectList.end(); ConnectIterator++)
    {
        if ((*ConnectIterator) != NULL)
        {
            (*ConnectIterator)(Server);
        }
    }

    for (ReqPacketIterator = m_ReqPacketList.begin(); ReqPacketIterator != m_ReqPacketList.end(); ReqPacketIterator++)
    {
        Server->RegisterRequestHandle(ReqPacketIterator->first, ReqPacketIterator->second);
    }

    for (ReqDataIterator = m_ReqDataList.begin(); ReqDataIterator != m_ReqDataList.end(); ReqDataIterator++)
    {
        Server->RegisterRequestHandle(ReqDataIterator->first, ReqDataIterator->second);
    }

    for (EndIterator = m_EndList.begin(); EndIterator != m_EndList.end(); EndIterator++)
    {
        Server->RegisterEndHandle(*EndIterator);
    }

    LeaveCriticalSection(&m_csLock);
}
