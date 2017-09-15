#include "stdafx.h"
#include "Windows/PipeServer.h"
#include "Windows/PipeHelper.h"
#include "Common/Buffer.h"
#include "Windows/GlobalEvent.h"

CPipeServer::CPipeServer(HANDLE Pipe) : CCommunication()
{
    m_hPipe  = Pipe;
    m_bAlive = FALSE;
    m_pParam = NULL;
}

CPipeServer::~CPipeServer()
{
	if (m_hPipe != INVALID_HANDLE_VALUE)
	{
		PipeDisconnect(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
	}

}

BOOL CPipeServer::Start()
{
    if (IsConnected())
    {
        RegisterEndHandle(CPipeServer::PipeClear);
        StartCommunication();
        m_bAlive = TRUE;
        return TRUE;
    }

    return FALSE;
}

//停止通信
void CPipeServer::Stop()
{
    m_bAlive = FALSE;

    StopCommunication();

    if (IsConnected())
    {
		PipeDisconnect(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
    }
}

BOOL CPipeServer::IsConnected()
{
    return (m_hPipe != INVALID_HANDLE_VALUE);
}

void CPipeServer::PipeClear(ICommunication* param)
{
    CPipeServer *Pipe = dynamic_cast<CPipeServer*>(param);
    if (Pipe->m_bAlive)
    {
        Pipe->StopCommunication();
        DisconnectNamedPipe(Pipe->m_hPipe); 
        CloseHandle(Pipe->m_hPipe);
        Pipe->m_hPipe = INVALID_HANDLE_VALUE;
        Pipe->m_bAlive = FALSE;

        //relase self
        Pipe->Release();
    }
}

IPacketBuffer* CPipeServer::RecvAPacket(HANDLE StopEvent)
{
	IPacketBuffer* Buffer;

    if (!IsConnected())
    {
        return NULL;
    }

    Buffer = PipeRecvAPacket(m_hPipe, INFINITE, StopEvent);

	return Buffer;
}

BOOL CPipeServer::SendAPacket(IPacketBuffer* Buffer, HANDLE StopEvent)
{
	BOOL ReturnValue;

    if (!IsConnected())
    {
        return FALSE;
    }

	ReturnValue = PipeWriteNBytes(m_hPipe, (BYTE*)Buffer->GetData(), Buffer->GetBufferLength(), INFINITE, StopEvent);

	return ReturnValue;
}

PVOID CPipeServer::GetParam()
{
	return m_pParam;
}

VOID CPipeServer::SetParam(PVOID Param)
{
	m_pParam = Param;
}

CPipeServerService::CPipeServerService(TCHAR* PipeName, DWORD Timeout, HANDLE StopEvent) : CBaseObject()
{
#ifdef UNICODE
    m_szPipeName = wcsdup(PipeName);
#else
    m_szPipeName = strdup(PipeName);
#endif

    InitSyncLock();

    m_hStopEvent = StopEvent;
    m_dwTimeout = Timeout;

    AddRef();
    m_pMainThread = CreateIThreadInstance(CPipeServerService::ServiceMainThreadProc, this);

    InitializeCriticalSection(&m_csLock);
}

//析构函数
CPipeServerService::~CPipeServerService()
{
    StopMainService();

    m_pMainThread->Release();

    m_pMainThread = NULL;

    if (m_hSyncLocker)
    {
        CloseHandle(m_hSyncLocker);
        m_hSyncLocker = NULL;
    }

    free(m_szPipeName);
    DeleteCriticalSection(&m_csLock);
}

void CPipeServerService::InitSyncLock()
{
    TCHAR szSyncEventName[MAX_PATH];
    DWORD nPipeNameLen = _tcslen(m_szPipeName);
    TCHAR* ptr = m_szPipeName + nPipeNameLen;
    while (*ptr != _T('\\') && ptr != m_szPipeName)
    {
        ptr--;
    }

    if (*ptr == _T('\\'))
    {
        ptr++;
    }

    _stprintf(szSyncEventName, _T("Global\\%s_Locker"), ptr);

    m_hSyncLocker = CreateGlobalEvent(szSyncEventName);

    ResetEvent(m_hSyncLocker);
}

BOOL CPipeServerService::StartMainService()
{
    m_pMainThread->StartMainThread();
    
    return TRUE;
}

void CPipeServerService::StopMainService()
{
    if (m_pMainThread->IsMainThreadRunning()) 
    {
        m_pMainThread->StopMainThread();
    }
}

//注册请求的处理函数
BOOL CPipeServerService::RegisterRequestHandle(DWORD Type, RequestPacketHandle Func)
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

void CPipeServerService::RegisterEndHandle(EndHandle Func)
{
    EnterCriticalSection(&m_csLock);
    m_EndList.push_back(Func);
    LeaveCriticalSection(&m_csLock);
}

VOID CPipeServerService::SetParam(PVOID Param)
{
    EnterCriticalSection(&m_csLock);
    m_pParam = Param;
    LeaveCriticalSection(&m_csLock);
}

void CPipeServerService::InitalizeServer(CPipeServer *Server)
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

//主线程处理函数
BOOL CPipeServerService::ServiceMainThreadProc(LPVOID Parameter, HANDLE StopEvent)
{
    HANDLE          hPipe, hEvent; 
    CPipeServerService* Service = (CPipeServerService*)Parameter;
    OVERLAPPED      ol;

    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    BOOL  ret;
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    /*
        * 允许所有用户连接
        */
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd,TRUE,NULL,FALSE);
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = &sd;

    memset(&ol, 0, sizeof(OVERLAPPED));
    ol.hEvent = hEvent;
    ResetEvent(ol.hEvent);
    hPipe = CreateNamedPipe(Service->m_szPipeName,
                            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                            PIPE_UNLIMITED_INSTANCES,
                            0,
                            0,
                            Service->m_dwTimeout,
                            &sa);

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        CloseHandle(hEvent);
        return FALSE; 
    }

    if (Service->m_hSyncLocker)
    {
        SetEvent(Service->m_hSyncLocker);
    }

    ret = ConnectNamedPipe(hPipe, &ol);
    if (ret == FALSE)
    {
        if (GetLastError() == ERROR_PIPE_CONNECTED)
        {
            ret = TRUE;
        }
        else if (GetLastError() == ERROR_IO_PENDING)
        {
            DWORD waitRet;
            HANDLE handles[3] = { ol.hEvent, Service->m_hStopEvent, StopEvent};

            waitRet = WaitForMultipleObjects(3, handles, FALSE, INFINITE);
            switch (waitRet) {
                case WAIT_OBJECT_0:
                    if (HasOverlappedIoCompleted(&ol) != FALSE) {
                        ret = TRUE;
                    }
                    break;
                case WAIT_OBJECT_0 + 1:
                case WAIT_OBJECT_0 + 2:
                default:
                    CloseHandle(hPipe);
                    CloseHandle(hEvent);
                    return FALSE; 
            }
        }
    }

    if (ret) {
        CPipeServer *Pipe = new CPipeServer(hPipe);
        Service->InitalizeServer(Pipe);
        if (!Pipe->Start())
        {
            Pipe->Release();
        } 
    } else {
        CloseHandle(hPipe);
    }

    CloseHandle(hEvent);

    Service->Release();
    return TRUE; 
}
