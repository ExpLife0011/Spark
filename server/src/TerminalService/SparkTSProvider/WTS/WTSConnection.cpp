#include "stdafx.h"
#include "Log\LogEx.h"
#include "Common\IThread.h"
#include "SparkControl.h"
#include "WTS\WTSConnection.h"
#include "WTS\WTSLicenseConnection.h"
#include "SessionHelper\SessionHelp.h"
#include "RemoteSessionManager.h"
#include "Windows\RegOperation.h"
#include <WtsApi32.h>

#define TS_SERVICE_INIT_TIMEOUT          60000
#define TS_SERVICE_COMMUNICATION_TIMEOUT 10000

CSparkWTSConnection::CSparkWTSConnection(DWORD ConnectionId, CRemoteSessionManager* Manager, IWTSProtocolListenerCallback* ListenerCallback)
{
    m_lRef = 1;
    m_dwConnectionID = ConnectionId;
    
    ZeroMemory(&m_stClientInfo, sizeof(SparkClientInfo));

    m_pPipeClient = CreateIPipeClientInstance(SPARK_PROVIDER_PIPE_NAME, SPARK_PROVIDER_PIPE_TIMEOUT);
    m_pPipeClient->RegisterEndHandle(CSparkWTSConnection::DisConnectFromService);
    m_pPipeClient->RegisterRequestHandle(SparkCallbackOnReady, CSparkWTSConnection::OnReadyHandle);
    m_pPipeClient->RegisterRequestHandle(SparkCallbackOnBrokenConnection, CSparkWTSConnection::OnBrokenConnectionHandle);
    m_pPipeClient->RegisterRequestHandle(SparkCallbackRelease, CSparkWTSConnection::OnReleaseHandle);
    m_pPipeClient->RegisterRequestHandle(SparkRedrawRect, CSparkWTSConnection::OnRedrawRect);

	m_pCallBack = NULL;

    AddRef();
    m_pOnConnectThread = CreateIThreadInstance(CSparkWTSConnection::OnConnectThreadProcess, this);

    m_pListenerCallBack = ListenerCallback;
    if (m_pListenerCallBack)
    {
        m_pListenerCallBack->AddRef();
    }

    m_hOnConnectThreadStoped = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hServiceInitOK = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hWaitForOnReady = CreateEvent(NULL, FALSE, FALSE, NULL);
    
    m_bAccept = FALSE;
    m_dwSessionID = 0;

    m_pSessionManager = Manager;
    if (m_pSessionManager)
    {
        m_pSessionManager->AddRef();
    }

    DllAddRef();
}

CSparkWTSConnection::~CSparkWTSConnection()
{
    DRIVER_ENTER();

    if (m_pPipeClient)
    {
        m_pPipeClient->DisConnect();
        m_pPipeClient->Release();
        m_pPipeClient = NULL;
    }

    if (m_pOnConnectThread)
    {
        m_pOnConnectThread->Release();
        m_pOnConnectThread = NULL;
    }

    if (m_bAccept)
    {
        CSessionHelp::GetInstance()->Disable();
    }

	if (m_pCallBack)
	{
        m_pCallBack->Release();
        m_pCallBack = NULL;
	}

    if (m_pListenerCallBack)
    {
        m_pListenerCallBack->Release();
        m_pListenerCallBack = NULL;
    }

    if (m_pSessionManager)
    {
        m_pSessionManager->Release();
        m_pSessionManager = NULL;
    }

    DllRelease();

    DRIVER_LEAVE();
}

HRESULT CSparkWTSConnection::QueryInterface(const IID&  iid, VOID** ppv)
{
    DRIVER_ENTER();

    TCHAR guidString[512];

    if (iid == IID_IUnknown)
    {
        DRIVER_INFO(_T("CSparkWTSConnection::QueryInterface type IUnknow\r\n"));
        *ppv = this;
    }
    else if (iid == __uuidof(IWTSProtocolConnection))
    {
        DRIVER_INFO(_T("CSparkWTSConnection::QueryInterface type IWTSProtocolConnection\r\n"));
        *ppv = (IWTSProtocolConnection*)this;
    }
    else if (iid == _uuidof(IWTSConnectionRdp))
    {
        DRIVER_INFO(_T("CSparkWTSConnection::QueryInterface type IWTSConnectionRdp\r\n"));
        *ppv = (IWTSConnectionRdp*)this;
    }
    else
    {
        DRIVER_ERROR(_T("CSparkWTSConnection::QueryInterface %s. Returning E_NOINTERFACE.\r\n"), GUIDToString(iid, guidString));
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();

    DRIVER_LEAVE();

    return S_OK;
}

ULONG CSparkWTSConnection::AddRef()
{
    ULONG lRef = InterlockedIncrement(&m_lRef);
	return lRef;

}

ULONG CSparkWTSConnection::Release()
{
    ULONG Ref = InterlockedDecrement(&m_lRef);

    if (Ref == 0)
    {
        delete this;
        return 0;
    }
    return Ref;
}

HRESULT CSparkWTSConnection::GetLogonErrorRedirector(IWTSProtocolLogonErrorRedirector **ppLogonErrorRedir)
{
    DRIVER_ENTER();

    *ppLogonErrorRedir = NULL;

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWTSConnection::SendPolicyData(WTS_POLICY_DATA *pPolicyData)
{
    DRIVER_ENTER();

    UNREFERENCED_PARAMETER(pPolicyData);

    SetEvent(this->m_hWaitForOnReady);

    DRIVER_LEAVE();
    return S_OK;
}

HRESULT CSparkWTSConnection::AcceptConnection(void)
{
    DRIVER_ENTER();

    if (!this->m_bAccept)
    {
        DRIVER_INFO(_T("Wait OnConnect\r\n"));
        WaitForSingleObject(this->m_hOnConnectThreadStoped, INFINITE);

        if (WaitForSingleObject(this->m_hServiceInitOK, 0) != WAIT_OBJECT_0)
        {
            DRIVER_INFO(_T("Service Init Fail"));
            DRIVER_LEAVE();
            return E_FAIL;
        }

        DRIVER_INFO(_T("Service Init OK"));
        this->m_bAccept = TRUE;
        CSessionHelp::GetInstance()->Enable();
    }
    DRIVER_LEAVE();
    return S_OK;
}

HRESULT CSparkWTSConnection::GetClientData(WTS_CLIENT_DATA *pClientData)
{
    IPacketBuffer* Buffer = NULL;
    PSparkGetClientInfoRequest Request = NULL;
    IPacketBuffer* Reply = NULL;
    PSparkGetClientInfoRespone Respone = NULL;

    HANDLE DoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    HRESULT Result;

    DRIVER_ENTER();

    Buffer = CreateIBufferInstance(sizeof(SparkGetClientInfoRequest));
    Request = (PSparkGetClientInfoRequest)Buffer->GetData();

    Request->stConnectionInfo.eType = SPARK_PROVIDER_WTS;
    Request->stConnectionInfo.dwConnectionID = m_dwConnectionID;
    Request->dwPid = GetCurrentProcessId();

    if (!m_pPipeClient->SendRequestWithRespone(SparkGetClientInfo, Buffer, &Reply, DoneEvent))
    {
        DRIVER_ERROR(_T("Send Reques Fail %d\r\n"), GetLastError());
        m_pPipeClient->DisConnect();
        Result = E_FAIL;
        goto exit;
    }

    DWORD Ret = WaitForSingleObject(DoneEvent, TS_SERVICE_COMMUNICATION_TIMEOUT);

    if (Ret != WAIT_OBJECT_0)
    {
        m_pPipeClient->CancelIO();
        m_pPipeClient->DisConnect();
        Result = E_FAIL;
        goto exit;
    }

    if (Reply != NULL)
    {
        Respone = (PSparkGetClientInfoRespone)Reply->GetData();
        if (Respone->dwResult != ERROR_SUCCESS)
        {
            DRIVER_ERROR(_T("Respone Result Fail %d\r\n"), Respone->dwResult);
            Result = E_FAIL;
        }
        else
        {
            memcpy(&m_stClientInfo, &Respone->stClientInfo, sizeof(SparkClientInfo));

            memcpy(pClientData, &m_stClientInfo.stClientData, sizeof(WTS_CLIENT_DATA));

            DRIVER_INFO(_T("GetClientData OK\r\n"));

            Result = S_OK;
        }
    }
    else
    {
        DRIVER_ERROR(_T("Respone Result Timout\r\n"));
        Result = E_FAIL;
    }

exit:
    if (Buffer)
    {
        Buffer->Release();
    }

    if (Reply)
    {
        Reply->Release();
    }

    if (DoneEvent)
    {
        CloseHandle(DoneEvent);
    }

    DRIVER_LEAVE();

    return Result;
}

HRESULT CSparkWTSConnection::GetUserCredentials(WTS_USER_CREDENTIAL *pUserCreds)
{
    HRESULT Result = E_NOTIMPL; 

    DRIVER_ENTER();

    if (m_stClientInfo.bAutoLogin)
    {
        wcscpy(pUserCreds->Domain, m_stClientInfo.szDomain);
        wcscpy(pUserCreds->UserName, m_stClientInfo.szUsername);
        wcscpy(pUserCreds->Password, m_stClientInfo.szPassword);
        Result = S_OK;
    }

    DRIVER_INFO(_T("GetUserCredentials Domain %s, Username %s Password %s\r\n"), pUserCreds->Domain, pUserCreds->UserName, pUserCreds->Password);

    DRIVER_LEAVE();

    return Result;
}

HRESULT CSparkWTSConnection::GetLicenseConnection(IWTSProtocolLicenseConnection **ppLicenseConnection)
{
    CONTEXT contex;

    RtlCaptureContext(&contex);
#ifdef _WIN64
    CSessionHelp::GetInstance()->Install((BYTE *)contex.Rbp);
#else
    CSessionHelp::GetInstance()->Install((BYTE *)contex.Ebp);
#endif

    DRIVER_ENTER();

    CSparkWTSLicenseConnection *License = new CSparkWTSLicenseConnection();

    *ppLicenseConnection = License;

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWTSConnection::AuthenticateClientToSession(WTS_SESSION_ID *SessionId)
{
    DRIVER_ENTER();

    UNREFERENCED_PARAMETER(SessionId);

    DRIVER_LEAVE();

    return E_NOTIMPL;
}

HRESULT CSparkWTSConnection::NotifySessionId(WTS_SESSION_ID *SessionId)
{
    IPacketBuffer * Buffer = CreateIBufferInstance(sizeof(SparkNotifySessionIdRequest));
    PSparkNotifySessionIdRequest Request;
 
    DRIVER_ENTER();

    Request = (PSparkNotifySessionIdRequest)Buffer->GetData();

    m_dwSessionID = SessionId->SessionId;
    DRIVER_INFO(_T("Connect to Session %d\r\n"), SessionId->SessionId);

    Request->stConnectionInfo.eType = SPARK_PROVIDER_WTS;
    Request->stConnectionInfo.dwConnectionID = m_dwConnectionID;
    Request->dwSessionID = m_dwSessionID;
	L_INFO(_T("notify session id %d\r\n"), Request->dwSessionID);
    m_pPipeClient->SendRequest(SparkNotifySessionId, Buffer, NULL);

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWTSConnection::GetProtocolHandles(HANDLE_PTR *pKeyboardHandle,
                                        HANDLE_PTR *pMouseHandle,
                                        HANDLE_PTR *pBeepHandle,
                                        HANDLE_PTR *pVideoHandle)
{
    
    *pKeyboardHandle = (HANDLE_PTR)m_stClientInfo.dwKeyBroadHandle;
    *pMouseHandle    = (HANDLE_PTR)m_stClientInfo.dwMouseHandle;
    *pBeepHandle     = (HANDLE_PTR)m_stClientInfo.dwBeepHandle;
    *pVideoHandle    = (HANDLE_PTR)m_stClientInfo.dwVideoHandle;
			
	DRIVER_INFO(_T("GetProtocolHandles VideoHandle %X \r\n"), *pVideoHandle);

    DRIVER_INFO(_T("GetInputHandles Keybroad %x, Mouse %x Beep %x Video %x\r\n"), *pKeyboardHandle, *pMouseHandle, *pBeepHandle, *pVideoHandle);

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWTSConnection::ConnectNotify(ULONG SessionId)
{
    DRIVER_ENTER();

    DRIVER_INFO(_T("ConnectNotify %d\r\n"), SessionId);

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWTSConnection::IsUserAllowedToLogon(ULONG SessionId,
                                                  HANDLE_PTR UserToken,
                                                  WCHAR *pDomainName,
                                                  WCHAR *pUserName)
{
    DRIVER_ENTER();

    UNREFERENCED_PARAMETER(UserToken);

    if (!m_stClientInfo.bAutoLogin)
    {
        wcsncpy(m_stClientInfo.szDomain, pDomainName, MAX_PATH);
        wcsncpy(m_stClientInfo.szUsername, pUserName, MAX_PATH);
    }

    DRIVER_INFO(_T("Login Check OK %s/%s Session %d \r\n"), pDomainName, pUserName, SessionId);

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWTSConnection::SessionArbitrationEnumeration(HANDLE_PTR hUserToken,
                                                   BOOL bSingleSessionPerUserEnabled,
                                                   ULONG *pSessionIdArray,
                                                   ULONG *pdwSessionIdentifierCount)
{
    DWORD Session = 0;
    HRESULT Result = S_OK;

    DRIVER_ENTER();

    DRIVER_INFO(_T("hUserToken 0x%x bSingleSessionPerUserEnabled %d pSessionIdArray %d\r\n"), hUserToken, bSingleSessionPerUserEnabled, pSessionIdArray);

    if (pdwSessionIdentifierCount == NULL)
    {
        DRIVER_ERROR(_T("pdwSessionIdentifierCount is null\r\b"));
        return E_FAIL;
    }

    DRIVER_INFO(_T("Current Name %s/%s\r\n"), m_stClientInfo.szDomain, m_stClientInfo.szUsername);
    
    Session = this->m_pSessionManager->FindSparkConnectedSessionID(m_stClientInfo.szDomain, m_stClientInfo.szUsername, m_stClientInfo.bRemoteAppConnection);

    if (Session != 0)
    {
        DRIVER_TRACE(_T("Get Xred Session %d\r\n"), Session);
        *pdwSessionIdentifierCount = 1;

        if (pSessionIdArray == NULL)
        {
            Result = E_NOT_SUFFICIENT_BUFFER;
        }
        else
        {
            pSessionIdArray[0] = Session;
            this->m_pSessionManager->ReplaceToSpark(Session);
            L_INFO(_T("Replace Session %d To Ivy\r\n"), Session);
        }
    }
    else
    {
        DRIVER_TRACE(_T("Create New Session\r\n"));
        *pdwSessionIdentifierCount = 0;
    } 
    
    DRIVER_LEAVE();
    return Result;
}

HRESULT CSparkWTSConnection::LogonNotify(HANDLE_PTR hClientToken, WCHAR *wszUserName, WCHAR *wszDomainName,
                                 WTS_SESSION_ID *SessionId)
{
    DRIVER_ENTER();

    UNREFERENCED_PARAMETER(hClientToken);
    UNREFERENCED_PARAMETER(wszUserName);
    UNREFERENCED_PARAMETER(wszDomainName);
    UNREFERENCED_PARAMETER(SessionId);

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWTSConnection::GetUserData(WTS_POLICY_DATA *pPolicyData, WTS_USER_DATA *pClientData)
{
    DRIVER_ENTER();

    UNREFERENCED_PARAMETER(pPolicyData);
    UNREFERENCED_PARAMETER(pClientData);

    DRIVER_LEAVE();
    return S_OK;
}

HRESULT CSparkWTSConnection::DisconnectNotify(void)
{
    DRIVER_ENTER();
    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWTSConnection::Close(void)
{
    IPacketBuffer* Buffer = NULL;

    DRIVER_ENTER();

    HANDLE DoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    
    Buffer = CreateIBufferInstance(sizeof(SparkCloseConnectionRequest));

    PSparkCloseConnectionRequest Request = (PSparkCloseConnectionRequest)Buffer->GetData();

    Request->stConnectionInfo.eType = SPARK_PROVIDER_WTS;
    Request->stConnectionInfo.dwConnectionID = m_dwConnectionID;

    DRIVER_INFO(_T("Close ConnectID %d\r\n"), m_dwConnectionID);

    m_pPipeClient->SendRequest(SparkCloseConnection, Buffer, DoneEvent);

    if (WaitForSingleObject(DoneEvent, TS_SERVICE_COMMUNICATION_TIMEOUT) != WAIT_OBJECT_0)
    {
        m_pPipeClient->CancelIO();
    }

    m_pPipeClient->DisConnect();

	if (m_pCallBack)
	{
        m_pCallBack->Release();
        m_pCallBack = NULL;
	}
	
    DRIVER_LEAVE();

    if (Buffer)
    {
        Buffer->Release();
    }

    return S_OK;
}

HRESULT CSparkWTSConnection::GetProtocolStatus(WTS_PROTOCOL_STATUS *pProtocolStatus)
{
    UNREFERENCED_PARAMETER(pProtocolStatus);
    return E_NOTIMPL;
}
    
HRESULT CSparkWTSConnection::GetLastInputTime(ULONG64 *pLastInputTime)
{
    *pLastInputTime = 1000;

    return S_OK;
}
    
HRESULT CSparkWTSConnection::SetErrorInfo(ULONG ulError)
{
    DRIVER_ENTER();

    DRIVER_ERROR(_T("Error 0x%08x\r\n"), ulError);

    DRIVER_LEAVE();

    return S_OK;
}
    
HRESULT CSparkWTSConnection::CreateVirtualChannel(CHAR *szEndpointName, BOOL bStatic, ULONG RequestedPriority, ULONG_PTR *phChannel)
{
    DRIVER_ENTER();

    UNREFERENCED_PARAMETER(szEndpointName);
    UNREFERENCED_PARAMETER(bStatic);
    UNREFERENCED_PARAMETER(RequestedPriority);
    UNREFERENCED_PARAMETER(phChannel);

    DRIVER_LEAVE();

    return E_NOTIMPL;
}

HRESULT CSparkWTSConnection::SendBeep(ULONG Frequency, ULONG Duration)
{
    DRIVER_ENTER();

    UNREFERENCED_PARAMETER(Frequency);
    UNREFERENCED_PARAMETER(Duration);

    DRIVER_LEAVE();

    return E_NOTIMPL;
}


HRESULT CSparkWTSConnection::QueryProperty(GUID QueryType, ULONG ulNumEntriesIn,
                                   ULONG ulNumEntriesOut, PWTS_PROPERTY_VALUE pPropertyEntriesIn,
                                   PWTS_PROPERTY_VALUE pPropertyEntriesOut)
{
    TCHAR GuidStr[512];
    HRESULT Result = E_NOTIMPL;
    DRIVER_ENTER();

    GUIDToString(QueryType, GuidStr);

    DRIVER_INFO(_T("QueryType %s %d %d"), GuidStr, ulNumEntriesIn, ulNumEntriesOut);

    if ( QueryType == WTS_QUERY_ALLOWED_INITIAL_APP )
    {
        DRIVER_TRACE(L"WTS_QUERY_ALLOWED_INITIAL_APP\r\n");
        if (pPropertyEntriesIn)
        {
            DRIVER_TRACE(_T("pPropertyEntriesIn[0].type %d\r\n"), pPropertyEntriesIn[0].Type);
            DRIVER_TRACE(_T("pPropertyEntriesIn[0].u.strVal.pstrVal %s\r\n"), pPropertyEntriesIn[0].u.strVal.pstrVal);
            DRIVER_TRACE(_T("pPropertyEntriesIn[0].u.strVal.size %d\r\n"), pPropertyEntriesIn[0].u.strVal.size);


            DRIVER_TRACE(_T("pPropertyEntriesIn[1].type %d\r\n"), pPropertyEntriesIn[1].Type);
            DRIVER_TRACE(_T("pPropertyEntriesIn[1].u.strVal.pstrVal %s\r\n"), pPropertyEntriesIn[1].u.strVal.pstrVal);
            DRIVER_TRACE(_T("pPropertyEntriesIn[1].u.strVal.size %d\r\n"), pPropertyEntriesIn[1].u.strVal.size);


            DRIVER_TRACE(_T("pPropertyEntriesIn[2].type %d\r\n"), pPropertyEntriesIn[2].Type);
            DRIVER_TRACE(_T("pPropertyEntriesIn[2].u.ulVal %d\r\n"), pPropertyEntriesIn[2].u.ulVal);

            if (pPropertyEntriesOut)
            {
                pPropertyEntriesOut[0].Type = pPropertyEntriesIn[0].Type;
                pPropertyEntriesOut[0].u.strVal.pstrVal = (WCHAR*)LocalAlloc(0, MAX_PATH * sizeof(WCHAR));
                memset(pPropertyEntriesOut[0].u.strVal.pstrVal, 0, MAX_PATH * sizeof(WCHAR));
                wcscpy(pPropertyEntriesOut[0].u.strVal.pstrVal, pPropertyEntriesIn[0].u.strVal.pstrVal);
                pPropertyEntriesOut[0].u.strVal.size = pPropertyEntriesIn[0].u.strVal.size;

                pPropertyEntriesOut[1].Type = pPropertyEntriesIn[1].Type;
                pPropertyEntriesOut[1].u.strVal.pstrVal = (WCHAR*)LocalAlloc(0, MAX_PATH * sizeof(WCHAR));
                memset(pPropertyEntriesOut[1].u.strVal.pstrVal, 0, MAX_PATH * sizeof(WCHAR));
                wcscpy(pPropertyEntriesOut[1].u.strVal.pstrVal, pPropertyEntriesIn[1].u.strVal.pstrVal);
                pPropertyEntriesOut[1].u.strVal.size = pPropertyEntriesIn[1].u.strVal.size;

                pPropertyEntriesOut[2].Type = pPropertyEntriesIn[2].Type;
                pPropertyEntriesIn[2].u.ulVal = 1;

                Result = S_OK;
            }
        }
    }
    else if ( QueryType == WTS_QUERY_LOGON_SCREEN_SIZE )
    {
        DRIVER_TRACE(L"WTS_QUERY_LOGON_SCREEN_SIZE\r\n");
    }
    else if ( QueryType == WTS_QUERY_AUDIOENUM_DLL )
    {
        DRIVER_TRACE(L"WTS_QUERY_AUDIOENUM_DLL\r\n");
        if (pPropertyEntriesOut)
        {
            DWORD Length = MAX_PATH;
            pPropertyEntriesOut->Type = WTS_VALUE_TYPE_STRING;
            pPropertyEntriesOut->u.strVal.pstrVal = (WCHAR*)LocalAlloc(0, MAX_PATH * sizeof(WCHAR));

            memset(pPropertyEntriesOut->u.strVal.pstrVal, 0, MAX_PATH * sizeof(WCHAR));
			
			RegOpReadString(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\Spark"),
					_T("AudioEnumeratorDll"), pPropertyEntriesOut->u.strVal.pstrVal, &Length, _T(""));
			
			pPropertyEntriesOut->u.strVal.size = MAX_PATH;
            Result = S_OK;
        }
    }
    else if ( QueryType == WTS_QUERY_MF_FORMAT_SUPPORT )
    {
        DRIVER_TRACE(L"WTS_QUERY_MF_FORMAT_SUPPORT\r\n");
    }

    DRIVER_LEAVE();

    return Result;
}

HRESULT CSparkWTSConnection::GetShadowConnection(IWTSProtocolShadowConnection **ppShadowConnection)
{
    DRIVER_ENTER();

    *ppShadowConnection = NULL;

    DRIVER_LEAVE();

    return E_NOTIMPL;
}

HRESULT CSparkWTSConnection::PrepareForAccept(void *a1, int a2, int a3, int a4)
{
    DRIVER_ENTER();

    UNREFERENCED_PARAMETER(a1);
    UNREFERENCED_PARAMETER(a2);
    UNREFERENCED_PARAMETER(a3);
    UNREFERENCED_PARAMETER(a4);

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWTSConnection::GetClientMonitorData(UINT *pNumMonitors, UINT *pPrimaryMonitor)
{
    DRIVER_ENTER();

    *pNumMonitors = m_stClientInfo.dwDisplayCount;
    *pPrimaryMonitor = m_stClientInfo.dwPrimaryDisplay;
    L_INFO(_T("GetClientMonitorData Count %d, Primary %d\r\n"), *pNumMonitors, *pPrimaryMonitor);
        
    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWTSConnection::GetSecurityFilterCreds(int a1, int a2)
{
    DRIVER_ENTER();

    UNREFERENCED_PARAMETER(a1);
    UNREFERENCED_PARAMETER(a2);

    DRIVER_LEAVE();

    return E_NOTIMPL;
}

HRESULT CSparkWTSConnection::GetSecurityFilterClientCerts(int a1, int a2)
{
    DRIVER_ENTER();

    UNREFERENCED_PARAMETER(a1);
    UNREFERENCED_PARAMETER(a2);

    DRIVER_LEAVE();

    return E_NOTIMPL;
}

HRESULT CSparkWTSConnection::GetSecurityFilterClientToken(int a1)
{
    L_TRACE_ENTER();

    UNREFERENCED_PARAMETER(a1);

    L_TRACE_LEAVE();

    return E_NOTIMPL;
}

HRESULT CSparkWTSConnection::SendLogonErrorInfoToClient(int a1, int a2)
{
    DRIVER_ENTER();

    UNREFERENCED_PARAMETER(a1);
    UNREFERENCED_PARAMETER(a2);

    DRIVER_LEAVE();

    return E_NOTIMPL;
}

void CSparkWTSConnection::OnReadyHandle(IPacketBuffer* Buffer, DWORD Id, ICommunication* Param)
{
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(Id);

    IPipeClient* Client = dynamic_cast<IPipeClient*>(Param);
    CSparkWTSConnection* Connection = (CSparkWTSConnection*)Client->GetParam();

    DRIVER_INFO(_T("SparkService Preper Notify OnReady\r\n"));

    if (!Connection->m_pCallBack)
    {
        DRIVER_ERROR(_T("Connection do not has CallBack\r\n"));
        return;
    }

    //等5秒
    if (WaitForSingleObject(Connection->m_hWaitForOnReady, 5000) == WAIT_OBJECT_0)
    {
        ResetEvent(Connection->m_hWaitForOnReady);
    }

    DRIVER_INFO(_T("SparkService Go Notify OnReady\r\n"));

	Connection->m_pCallBack->OnReady();   
	return;
}

void CSparkWTSConnection::OnBrokenConnectionHandle(IPacketBuffer* Buffer, DWORD Id, ICommunication* Param)
{
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(Id);

    IPipeClient* Client = dynamic_cast<IPipeClient*>(Param);
    CSparkWTSConnection* Connection = (CSparkWTSConnection*)Client->GetParam();

    DRIVER_INFO(_T("SparkService Notify OnBrokenConnection\r\n"));

    if (!Connection->m_pCallBack)
    {
        DRIVER_ERROR(_T("Connection do not has CallBack\r\n"));
        return;
    }

    Connection->m_pCallBack->BrokenConnection(0, 0);
	Connection->m_pCallBack->Release();
	Connection->m_pCallBack = NULL;

    return;
}

void CSparkWTSConnection::OnRedrawRect(IPacketBuffer* Buffer, DWORD Id, ICommunication* Param)
{
    UNREFERENCED_PARAMETER(Id);

	IPipeClient* Client = dynamic_cast<IPipeClient*>(Param);
	CSparkWTSConnection* Connection = (CSparkWTSConnection*)Client->GetParam();

    if (Buffer->GetBufferLength() >= sizeof(SparkRedrawRectRequest))
	{
        PSparkRedrawRectRequest pRequest = (PSparkRedrawRectRequest)Buffer->GetData();
        if (Connection->m_pCallBack)
		{
			for (DWORD i = 0; i < pRequest->dwCount; i++)
			{

				WTS_SMALL_RECT wtsrc;
				wtsrc.Left   = (SHORT)pRequest->pRect[i].left;
                wtsrc.Right  = (SHORT)pRequest->pRect[i].right;
                wtsrc.Top    = (SHORT)pRequest->pRect[i].top;
                wtsrc.Bottom = (SHORT)pRequest->pRect[i].bottom;
				Connection->m_pCallBack->RedrawWindow(&wtsrc);
			}
		}
	}
}

void CSparkWTSConnection::OnReleaseHandle(IPacketBuffer* Buffer, DWORD Id, ICommunication* Param)
{
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(Id);

    IPipeClient* Client = dynamic_cast<IPipeClient*>(Param);
    CSparkWTSConnection* Connection = (CSparkWTSConnection*)Client->GetParam();

    DRIVER_INFO(_T("SparkService Notify OnRelease\r\n"));

    if (!Connection->m_pCallBack)
    {
        DRIVER_ERROR(_T("Connection do not has CallBack\r\n"));
        return;
    }

    Connection->m_pCallBack->Release();
    Connection->m_pCallBack = NULL;

    return;
}

void CSparkWTSConnection::DisConnectFromService(ICommunication* Param)
{
    IPipeClient* Client = dynamic_cast<IPipeClient*>(Param);
    CSparkWTSConnection* Connection = (CSparkWTSConnection*)Client->GetParam();

    if (Connection)
    {
        if (Connection->m_pCallBack)
        {
            Connection->m_pCallBack->BrokenConnection(0, 0);
            Connection->m_pCallBack->Release();
            Connection->m_pCallBack = NULL;
        }

        Client->SetParam(NULL);
        Connection->Release();
    }

    return;
}

BOOL CSparkWTSConnection::OnConnectThreadProcess(LPVOID param, HANDLE stopevent)
{
    HRESULT Result;
    IWTSProtocolConnectionCallback* ConnectionCallBack = NULL;
    CSparkWTSConnection* Connection = (CSparkWTSConnection*)param;
    HANDLE DoneEvent = NULL;
    IPacketBuffer* Buffer = NULL;
    IPacketBuffer* Reply = NULL;

    DRIVER_TRACE(_T("Call OnConnect\r\n"));

    if (Connection->m_pListenerCallBack)
    {
		L_INFO(_T("OnConnected  callback \r\n"));
        Result = Connection->m_pListenerCallBack->OnConnected(Connection, &ConnectionCallBack);
      
		if (Result != S_OK)
        {
            DRIVER_ERROR(_T("OnConnected Fail %x\r\n"), Result);
            //release for pipe
            Connection->Release();
            goto exit;
        }
    
        Connection->m_pCallBack = ConnectionCallBack;

        DRIVER_TRACE(_T("SetCallBack OK\r\n"));

        Connection->AddRef();
        Connection->m_pPipeClient->SetParam(Connection);

        if (!Connection->m_pPipeClient->Connect())
        {
            DRIVER_ERROR(_T("Pipe Connect Fail %d\r\n"), GetLastError());
            //release for pipe
            Connection->Release();
            Connection->m_pPipeClient->Release();
            Connection->m_pPipeClient = NULL;
            goto exit;
        }

        Buffer = CreateIBufferInstance(sizeof(SparkAcceptConnectionRequest));

        DoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        HANDLE h[2];
        h[0] = DoneEvent;
        h[1] = stopevent;

        PSparkAcceptConnectionRequest Request = (PSparkAcceptConnectionRequest)Buffer->GetData();
        PSparkAcceptConnectionRespone Respone = NULL;
        
        Request->stConnectionInfo.eType = SPARK_PROVIDER_WRDS;
        Request->stConnectionInfo.dwConnectionID = Connection->m_dwConnectionID;

        if (!Connection->m_pPipeClient->SendRequestWithRespone(SparkAcceptConnection, 
            Buffer, &Reply, DoneEvent))
        {
            DRIVER_ERROR(_T("Send Reques Fail %d\r\n"), GetLastError());
            //release for pipe
            Connection->m_pPipeClient->DisConnect();
            Connection->m_pPipeClient->Release();
            Connection->m_pPipeClient = NULL;
            goto exit;
        }

        DWORD Ret = WaitForMultipleObjects(2, h, FALSE, INFINITE);
        if (Ret != WAIT_OBJECT_0)
        {
            DRIVER_ERROR(_T("Wait Resp Fail %d\r\n"), Ret);
            Connection->m_pPipeClient->CancelIO();
            //release for pipe
            Connection->m_pPipeClient->DisConnect();
            Connection->m_pPipeClient->Release();
            Connection->m_pPipeClient = NULL;
            goto exit;
        }

        if (Reply != NULL)
        {
            Respone = (PSparkAcceptConnectionRespone)Reply->GetData();
            if (Respone->dwResult != ERROR_SUCCESS)
            {
                DRIVER_ERROR(_T("Respone Result Fail %d\r\n"), Respone->dwResult);
                //release for pipe
                Connection->m_pPipeClient->DisConnect();
                Connection->m_pPipeClient->Release();
                Connection->m_pPipeClient = NULL;
            }
            else
            {
                //pipe alive do not need release
                SetEvent(Connection->m_hServiceInitOK);
                DRIVER_INFO(_T("AcceptConnection OK\r\n"));
            }
        }
        else
        {
            DRIVER_ERROR(_T("Respone Result Timout\r\n"));
            //release for pipe
            Connection->m_pPipeClient->DisConnect();
            Connection->m_pPipeClient->Release();
            Connection->m_pPipeClient = NULL;
        }

        goto exit;
    }

exit:
    SetEvent(Connection->m_hOnConnectThreadStoped);
    if (DoneEvent)
    {
        CloseHandle(DoneEvent);
    }

    if (Buffer)
    {
        Buffer->Release();
    }

    if (Reply)
    {
        Reply->Release();
    }

    //release for thread
	Connection->Release();
    return FALSE;
}

void CSparkWTSConnection::StartConnect()
{
    m_pOnConnectThread->StartMainThread();
}