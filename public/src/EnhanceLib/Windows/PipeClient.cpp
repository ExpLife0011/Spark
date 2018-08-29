#include "stdafx.h"
#include "Windows/PipeHelper.h"
#include "Windows/PipeClient.h"
#include "Common/Buffer.h"
#include <stdint.h>

extern uint32_t SuperFastHash(const char * data, int len, int nStep);

CPipeClient::CPipeClient(TCHAR *pipename, DWORD timeout) : CCommunication(), CParamSet()
{
#ifdef UNICODE
    m_szPipeName = wcsdup(pipename);
#else
    m_szPipeName = strdup(pipename);
#endif
    m_dwTimeout = timeout;
    m_hPipe = INVALID_HANDLE_VALUE;
}

CPipeClient::~CPipeClient()
{
    std::map<UINT32, CBaseObjPtr<CBaseObject>>::iterator Itor;
    std::list<CBaseObjPtr<CBaseObject>> TmpList;
    std::list<CBaseObjPtr<CBaseObject>>::iterator TmpListItor;

    free(m_szPipeName);

    if (m_hPipe != INVALID_HANDLE_VALUE)
    {
        DisconnectNamedPipe(m_hPipe);
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
    }
}

HANDLE CPipeClient::InitSyncLock()
{
    TCHAR szSyncEventName[MAX_PATH] = { 0 };
    int nPipeNameLen = _tcslen(m_szPipeName);
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

    HANDLE hSyncEvent = OpenEvent(EVENT_ALL_ACCESS, TRUE, szSyncEventName);

    return hSyncEvent;
}

BOOL CPipeClient::Connect()
{
    int retry = 0;

    SetLastError(ERROR_SUCCESS);

    if (IsConnected())
    {
        return FALSE;
    }

connect:
    HANDLE hSyncEvent = InitSyncLock();
    if (hSyncEvent)
    {
        DWORD dwRet = WaitForSingleObject(hSyncEvent, m_dwTimeout);
        if (dwRet == WAIT_OBJECT_0)
        {
            m_hPipe = PipeConnect(m_szPipeName, m_dwTimeout);
        }
        else
        {
            m_hPipe = INVALID_HANDLE_VALUE;
        }
        CloseHandle(hSyncEvent);
    }
    else
    {
        m_hPipe = PipeConnect(m_szPipeName, m_dwTimeout);
    }

    if (m_hPipe != INVALID_HANDLE_VALUE)
    {
        RegisterEndHandle(CPipeClient::PipeClear);
        StartCommunication();

        return TRUE;
    }
    else
    {
        if (GetLastError() == ERROR_PIPE_BUSY)
        {
            retry++;
            if (retry < 3)
            {
                goto connect;
            }
        }
    }

    return FALSE;
}

void CPipeClient::DisConnect()
{
    StopCommunication();
}

BOOL CPipeClient::IsConnected()
{
    return (m_hPipe != INVALID_HANDLE_VALUE);
}

void CPipeClient::PipeClear(ICommunication* param)
{
    CPipeClient *Pipe = dynamic_cast<CPipeClient *>(param);

    if (Pipe)
    {
        Pipe->StopCommunication();
    }
}

IPacketBuffer* CPipeClient::RecvAPacket(HANDLE StopEvent)
{
    IPacketBuffer* Buffer;
    SetLastError(ERROR_SUCCESS);

    if (!IsConnected())
    {
        SetLastError(ERROR_BUSY);
        return NULL;
    }

    Buffer = PipeRecvAPacket(m_hPipe, INFINITE, StopEvent);

    return Buffer;
}

BOOL CPipeClient::SendAPacket(IPacketBuffer* Buffer, HANDLE StopEvent)
{
    BOOL ReturnValue;
    SetLastError(ERROR_SUCCESS);

    if (!IsConnected())
    {
        SetLastError(ERROR_BUSY);
        return FALSE;
    }

    ReturnValue = PipeWriteNBytes(m_hPipe, Buffer->GetData(), Buffer->GetBufferLength(), INFINITE, StopEvent);

    return ReturnValue;
}
