#include "stdafx.h"
#include "Windows/PipeServer.h"
#include "Windows/PipeHelper.h"
#include "Common/Buffer.h"
#include "Windows/GlobalEvent.h"
#include <stdint.h>
#include "Base/SuperHash.h"

CPipeServer::CPipeServer(HANDLE Pipe) : CCommunication(), CParamSet()
{
    m_hPipe  = Pipe;
}

CPipeServer::~CPipeServer()
{
	if (m_hPipe != INVALID_HANDLE_VALUE)
	{
        DisconnectNamedPipe(m_hPipe);
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
	}
}

BOOL CPipeServer::Start()
{
    if (IsConnected())
    {
        RegisterEndHandle(CPipeServer::PipeClear);
        StartCommunication();
        return TRUE;
    }

    return FALSE;
}

void CPipeServer::Stop()
{
    StopCommunication();
}

BOOL CPipeServer::IsConnected()
{
    return (m_hPipe != INVALID_HANDLE_VALUE);
}

void CPipeServer::PipeClear(ICommunication* param)
{
    CPipeServer *Pipe = dynamic_cast<CPipeServer*>(param);
    if (Pipe)
    {
        Pipe->StopCommunication();
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

    m_pMainThread = CreateIThreadInstanceEx(CPipeServerService::ServiceMainThreadProc, this,
        CPipeServerService::ServiceMainThreadEndProc, this);

    InitializeCriticalSection(&m_csLock);
}

//析构函数
CPipeServerService::~CPipeServerService()
{
    std::map<UINT32, CBaseObjPtr<CBaseObject>>::iterator Itor;
    std::list<CBaseObjPtr<CBaseObject>> TmpList;
    std::list<CBaseObjPtr<CBaseObject>>::iterator TmpListItor;

    StopMainService();

    m_pMainThread->Release();

    m_pMainThread = NULL;

    if (m_hSyncLocker)
    {
        CloseHandle(m_hSyncLocker);
        m_hSyncLocker = NULL;
    }

    free(m_szPipeName);

    EnterCriticalSection(&m_csLock);
    for (Itor = m_ParamMap.begin(); Itor != m_ParamMap.end(); Itor++)
    {
        if (Itor->second != NULL)
        {
            TmpList.push_back(Itor->second);
            Itor->second = NULL;
        }
    }
    m_ParamMap.clear();
    LeaveCriticalSection(&m_csLock);

    for (TmpListItor = TmpList.begin(); TmpListItor != TmpList.end(); TmpListItor++)
    {
        (*TmpListItor) = NULL;
    }

    TmpList.clear();
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
    if (!m_pMainThread->IsMainThreadRunning())
    {
        AddRef();
        m_pMainThread->StartMainThread();
    }
    
    return TRUE;
}

void CPipeServerService::StopMainService()
{
    if (m_pMainThread->IsMainThreadRunning()) 
    {
        m_pMainThread->StopMainThread();
    }
}

BOOL CPipeServerService::RegisterRequestHandle(DWORD Type, RequestPacketHandle Func)
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

BOOL CPipeServerService::RegisterRequestHandle(DWORD Type, RequestDataHandle Func)
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

void CPipeServerService::RegisterEndHandle(EndHandle Func)
{
    EnterCriticalSection(&m_csLock);
    m_EndList.push_back(Func);
    LeaveCriticalSection(&m_csLock);
}

void CPipeServerService::RegisterConnectHandle(ConnectHandle Func)
{
    EnterCriticalSection(&m_csLock);
    m_ConnectList.push_back(Func);
    LeaveCriticalSection(&m_csLock);
}

VOID CPipeServerService::SetParam(const CHAR* ParamKeyword, CBaseObjPtr<CBaseObject> Param)
{
	UINT32 uHash = SuperFastHash(ParamKeyword, strlen(ParamKeyword), 1);
   
    EnterCriticalSection(&m_csLock);
	m_ParamMap[uHash] = Param;
    LeaveCriticalSection(&m_csLock);

    return;
}

void CPipeServerService::InitalizeServer(CPipeServer *Server)
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

BOOL CPipeServerService::ServiceMainThreadProc(LPVOID Parameter, HANDLE StopEvent)
{
    HANDLE          hPipe, hEvent; 
    CPipeServerService* Service = (CPipeServerService*)Parameter;
    OVERLAPPED      ol;

    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    BOOL  ret;
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;

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
        if (Pipe)
        {
            Pipe->AddRef();

            Service->InitalizeServer(Pipe);
            if (!Pipe->Start())
            {
                Pipe->Release();
            }

            Pipe->Release();
        }
    } else {
        CloseHandle(hPipe);
    }

    CloseHandle(hEvent);

    return TRUE; 
}

VOID CPipeServerService::ServiceMainThreadEndProc(LPVOID Parameter)
{
    CPipeServerService* Service = (CPipeServerService*)Parameter;
    Service->Release();
}
