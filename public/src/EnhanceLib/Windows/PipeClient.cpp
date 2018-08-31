#include "stdafx.h"
#include "Windows/PipeHelper.h"
#include "Windows/PipeClient.h"
#include "Common/Buffer.h"
#include <stdint.h>

using namespace enlib;

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

void CPipeClient::PipeClear(CObjPtr<ICommunication> param)
{
    CObjPtr<CPipeClient> spPipe = NULL;
    spPipe = param;

    if (spPipe)
    {
        spPipe->StopCommunication();
    }
}

CObjPtr<IPacketBuffer> CPipeClient::RecvAPacket(HANDLE StopEvent)
{
    CObjPtr<IPacketBuffer> spRet = NULL;
    SetLastError(ERROR_SUCCESS);

    if (!IsConnected())
    {
        SetLastError(ERROR_BUSY);
        spRet = NULL;
    }
    else
    {
        spRet = PipeRecvAPacket(m_hPipe, INFINITE, StopEvent);
    }

    return spRet;
}

BOOL CPipeClient::SendAPacket(CObjPtr<IPacketBuffer> Buffer, HANDLE StopEvent)
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
