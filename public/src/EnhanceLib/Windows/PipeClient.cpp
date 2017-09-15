#include "stdafx.h"
#include "Windows/PipeHelper.h"
#include "Windows/PipeClient.h"
#include "Common/Buffer.h"

CPipeClient::CPipeClient(TCHAR *pipename, DWORD timeout) : CCommunication()
{
#ifdef UNICODE
    m_szPipeName = wcsdup(pipename);
#else
    m_szPipeName = strdup(pipename);
#endif
    m_dwTimeout = timeout;
    m_hPipe = INVALID_HANDLE_VALUE;
    m_bAlive = FALSE;
	m_pParam = NULL;
}

CPipeClient::~CPipeClient()
{
    DisConnect();
    free(m_szPipeName);
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
    //防止重复连接
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
        m_bAlive = TRUE;

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
    m_bAlive = FALSE;
    
    StopCommunication();

    if (IsConnected())
    {
        PipeDisconnect(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
    }
}

//客户端是否连接
BOOL CPipeClient::IsConnected()
{
    return (m_hPipe != INVALID_HANDLE_VALUE);
}

//接受线程结束清理函数
void CPipeClient::PipeClear(ICommunication* param)
{
    CPipeClient *Pipe = dynamic_cast<CPipeClient *>(param);
    
    //如果对象未激活，就啥都不干
    if (Pipe->m_bAlive)
    {
        Pipe->StopCommunication();
        //断开PIPE
        PipeDisconnect(Pipe->m_hPipe);
        //清理PIPE信息
        Pipe->m_hPipe = INVALID_HANDLE_VALUE;
        Pipe->m_bAlive = FALSE;
    }
}

//同步接受一个数据包
IPacketBuffer* CPipeClient::RecvAPacket(HANDLE StopEvent)
{
	IPacketBuffer* Buffer;
    //清空LastError
    SetLastError(ERROR_SUCCESS);

    if (!IsConnected())
    {
        //LastError未连接
        SetLastError(ERROR_BUSY);
        return NULL;
    }

    Buffer = PipeRecvAPacket(m_hPipe, INFINITE, StopEvent);

	return Buffer;
}

//同步发送一个数据包
BOOL CPipeClient::SendAPacket(IPacketBuffer* Buffer, HANDLE StopEvent)
{
	BOOL ReturnValue;
    //清空LastError
    SetLastError(ERROR_SUCCESS);

    if (!IsConnected())
    {
        //LastError未连接
        SetLastError(ERROR_BUSY);
        return FALSE;
    }

    ReturnValue = PipeWriteNBytes(m_hPipe, Buffer->GetData(), Buffer->GetBufferLength(), INFINITE, StopEvent);
    
	return ReturnValue;
}

PVOID CPipeClient::GetParam()
{
	return m_pParam;
}

VOID CPipeClient::SetParam(PVOID Param)
{
	m_pParam = Param;
}