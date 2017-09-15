#include "stdafx.h"
#include <WtsApi32.h>
#include "Log\LogEx.h"
#include "Windows\IPipeClient.h"
#include "RemoteSessionManager.h"
#include "SparkControl.h"

#define SPARK_REMOTE_APP	    _T("Global\\SPARK_REMOTE_APP_%d")
#define PIPE_TIMEOUT            5000

CRemoteSession::CRemoteSession(DWORD SessionID)
{
    m_dwSessionId = SessionID;
    memset(m_szDomainName, 0, sizeof(WCHAR) * MAX_PATH);
    memset(m_szUserName, 0, sizeof(WCHAR) * MAX_PATH);
    memset(m_szSessionName, 0, sizeof(WCHAR) * MAX_PATH);

    Update();
}

CRemoteSession::~CRemoteSession()
{

}

void CRemoteSession::ReplaceToSpark()
{
	memset(m_szSessionName, 0, sizeof(WCHAR) * MAX_PATH);
    wcsncpy(m_szSessionName, L"Spark#7986", MAX_PATH);
}

void CRemoteSession::Update()
{
    WTSINFO* Info;
    DWORD Length;

    if (!WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, m_dwSessionId, WTSSessionInfo, (LPWSTR*)&Info, &Length))
    {
        DRIVER_ERROR(_T("WTSQuerySessionInformation Fail Code %d\r\n"), GetLastError());
        return;
    }

    DRIVER_INFO(_T("SessionID      %d"), Info->SessionId);
    DRIVER_INFO(_T("DomainName     %s"), Info->Domain);
    DRIVER_INFO(_T("UserName       %s"), Info->UserName);
    DRIVER_INFO(_T("WinStationName %s"), Info->WinStationName);

    wcsncpy(m_szDomainName, Info->Domain, MAX_PATH);
    wcsncpy(m_szUserName, Info->UserName, MAX_PATH);
    wcsncpy(m_szSessionName, Info->WinStationName, MAX_PATH);

    WTSFreeMemory(Info);
}

DWORD CRemoteSession::GetSessionID()
{
    return m_dwSessionId;
}

const WCHAR* CRemoteSession::GetUserName()
{
    return m_szUserName;
}

const WCHAR* CRemoteSession::GetDomainName()
{
    return m_szDomainName;
}

const WCHAR* CRemoteSession::GetSessionName()
{
    return m_szSessionName;
}

BOOL CRemoteSession::IsSparkSession(BOOL *IsRemoteApp)
{
    if (IsRemoteApp == NULL)
    {
        return FALSE;
    }

    if (wcsnicmp(m_szSessionName, _T("Spark"), 5) == 0)
    {
        TCHAR szEventName[MAX_PATH] = { 0 };
        HANDLE hEvent = NULL;
        _stprintf(szEventName, SPARK_REMOTE_APP, m_dwSessionId);
        hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, szEventName);
        if (hEvent)
        {
            CloseHandle(hEvent);
            *IsRemoteApp = TRUE;
            return TRUE;
        }
        else
        {
            *IsRemoteApp = FALSE;
            return TRUE;
        }
    }

    return FALSE;
}


BOOL CRemoteSession::IsConsole()
{
    return (wcsicmp(m_szSessionName, _T("Console")) == 0);
}

BOOL CRemoteSession::MatchUser(const WCHAR* Domain, const WCHAR* UserName)
{
    DRIVER_TRACE(_T("Match to %s/%s\r\n"), m_szDomainName, m_szUserName);
    if (wcsicmp(m_szDomainName, Domain) == 0
        && wcsicmp(m_szUserName, UserName) == 0)
    {
        return TRUE;
    }

    return FALSE;
}

CRemoteSessionManager::CRemoteSessionManager()
{
    m_SessionList.clear();
    InitializeCriticalSection(&m_csLock);
}

CRemoteSessionManager::~CRemoteSessionManager()
{
    std::list<CRemoteSession*>::iterator Itor;

    EnterCriticalSection(&m_csLock);

    for (Itor = m_SessionList.begin(); Itor != m_SessionList.end(); Itor++)
    {
        (*Itor)->Release();
    }

    m_SessionList.clear();

    LeaveCriticalSection(&m_csLock);

    DeleteCriticalSection(&m_csLock);
}

void CRemoteSessionManager::SendPacketToService(CRemoteSession* Session, BOOL Add)
{
    PSparkSessionInfoUpdateRequest Request;
    IPacketBuffer* Buffer = NULL;
    IPipeClient* Client = CreateIPipeClientInstance(SPARK_PROVIDER_PIPE_NAME, SPARK_PROVIDER_PIPE_TIMEOUT);

    if (Client->Connect())
    {
        HANDLE DoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        Buffer = CreateIBufferInstance(sizeof(SparkSessionInfoUpdateRequest));
        Request = (PSparkSessionInfoUpdateRequest)Buffer->GetData();
        memset(Request, 0, sizeof(SparkSessionInfoUpdateRequest));
        Request->dwAdd = Add;
        Request->stInfo.dwSessionID = Session->GetSessionID();
        if (Add)
        {
            wcsncpy(Request->stInfo.szDomainName, Session->GetDomainName(), MAX_PATH);
            wcsncpy(Request->stInfo.szUsername, Session->GetUserName(), MAX_PATH);
            wcsncpy(Request->stInfo.szSessionName, Session->GetSessionName(), MAX_PATH);
        }

        Client->SendRequest(SparkSessionInfoUpdate, Buffer, DoneEvent);

        DWORD Ret = WaitForSingleObject(DoneEvent, PIPE_TIMEOUT);

        if (Ret != WAIT_OBJECT_0)
        {
            Client->CancelIO();
        }

        Client->DisConnect();

        Buffer->Release();
    }
    
    Client->Release();
}

void CRemoteSessionManager::ReplaceToSpark(DWORD SessionID)
{
	BOOL found = FALSE;
	
	std::list<CRemoteSession*>::iterator Itor;
	CRemoteSession* Session = NULL;

	EnterCriticalSection(&m_csLock);

	for (Itor = m_SessionList.begin(); Itor != m_SessionList.end(); Itor++)
	{
		if ((*Itor)->GetSessionID() == SessionID)
		{
			Session = (*Itor);
			Session->ReplaceToSpark();
			found = TRUE;
			break;
		}
	}

    if (found)
    {
        SendPacketToService(Session, TRUE);
    }

	LeaveCriticalSection(&m_csLock);
}

void CRemoteSessionManager::AddSession(DWORD SessionID)
{
    BOOL found = FALSE;
    std::list<CRemoteSession*>::iterator Itor;
    CRemoteSession* Session = NULL;

    EnterCriticalSection(&m_csLock);

    for (Itor = m_SessionList.begin(); Itor != m_SessionList.end(); Itor++)
    {
        if ((*Itor)->GetSessionID() == SessionID)
        {
            Session = (*Itor);
            Session->Update();
            found = TRUE;
            break;
        }
    }

    if (!found)
    {
        Session = new CRemoteSession(SessionID);
        DRIVER_INFO(_T("Add Session %d\r\n"), SessionID);
        m_SessionList.push_back(Session);
    }

    SendPacketToService(Session, TRUE);

    LeaveCriticalSection(&m_csLock);
}

void CRemoteSessionManager::RemoveSession(DWORD SessionID)
{
    std::list<CRemoteSession*>::iterator Itor;
    CRemoteSession* Session = NULL;

    EnterCriticalSection(&m_csLock);

    for (Itor = m_SessionList.begin(); Itor != m_SessionList.end(); Itor++)
    {
        if ((*Itor)->GetSessionID() == SessionID)
        {
            DRIVER_INFO(_T("Remove Session %d\r\n"), SessionID);

            Session = (*Itor);
            SendPacketToService(Session, FALSE);
            m_SessionList.erase(Itor);
            Session->Release();
            break;
        }
    }

    LeaveCriticalSection(&m_csLock);
}

DWORD CRemoteSessionManager::FindSparkConnectedSessionID(const WCHAR* Domain, const WCHAR* UserName, BOOL RemoteAppConnection)
{
    DWORD SessionId = 0;
    std::list<CRemoteSession*>::iterator Itor;
    CRemoteSession* Session = NULL;

    EnterCriticalSection(&m_csLock);

    for (Itor = m_SessionList.begin(); Itor != m_SessionList.end(); Itor++)
    {
        if ((*Itor)->MatchUser(Domain, UserName))
        {
            BOOL IsRemoteApp;
            Session = (*Itor);
            if (Session->IsSparkSession(&IsRemoteApp))
            {
                if (RemoteAppConnection == IsRemoteApp)
                {
                    DRIVER_TRACE(_T("Spark Match IsRemoteApp %d\r\n"), IsRemoteApp);
                    SessionId = (*Itor)->GetSessionID();
                    break;
                }
            }
            else if (!RemoteAppConnection && Session->IsConsole())
            {
                DRIVER_TRACE(_T("Spark Match to Console\r\n"), IsRemoteApp);
                SessionId = (*Itor)->GetSessionID();
                break;
            }
        }
    }

    LeaveCriticalSection(&m_csLock);

    return SessionId;
}

void CRemoteSessionManager::NotifyServiceConnected()
{
    int i;
    PSparkSessionInfoSyncRequest Request;
    IPacketBuffer* Buffer = NULL;
    std::list<CRemoteSession*>::iterator Itor;

    IPipeClient* Client = CreateIPipeClientInstance(SPARK_PROVIDER_PIPE_NAME, SPARK_PROVIDER_PIPE_TIMEOUT);

    if (Client->Connect())
    {
        HANDLE DoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        Buffer = CreateIBufferInstance(sizeof(SparkSessionInfoSyncRequest) + sizeof(SparkSessionInfo) * m_SessionList.size());

        EnterCriticalSection(&m_csLock);

        Request = (PSparkSessionInfoSyncRequest)Buffer->GetData();
        Request->dwInfoCount = m_SessionList.size();

        for (i = 0, Itor = m_SessionList.begin(); Itor != m_SessionList.end(); i++, Itor++)
        {
            Request->stInfo[i].dwSessionID = (*Itor)->GetSessionID();
            wcsncpy(Request->stInfo[i].szDomainName, (*Itor)->GetDomainName(), MAX_PATH);
            wcsncpy(Request->stInfo[i].szUsername, (*Itor)->GetUserName(), MAX_PATH);
            wcsncpy(Request->stInfo[i].szSessionName, (*Itor)->GetSessionName(), MAX_PATH);
        }

        LeaveCriticalSection(&m_csLock);

        Client->SendRequest(SparkSessionInfoSync, Buffer, DoneEvent);

        DWORD Ret = WaitForSingleObject(DoneEvent, PIPE_TIMEOUT);

        if (Ret != WAIT_OBJECT_0)
        {
            Client->CancelIO();
        }

        Client->DisConnect();

        Buffer->Release();
    }

    Client->Release();
}