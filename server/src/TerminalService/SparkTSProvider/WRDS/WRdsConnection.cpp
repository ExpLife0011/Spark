/**
* @file     WRdsConnection.cpp
* @author   wangxu.st@centerm.com
* @date     2016/1/21
* @version  1.0
* @brief    NEP协议管理器源文件
*/
#include "stdafx.h"
#include "Log\LogEx.h"
#include "Common\IThread.h"
#include "ProviderControl.h"
#include "WRdsConnection.h"
#include "SessionHelp.h"
#include "WRdsLicenseConnection.h"
#include <WtsApi32.h>
#include "Windows\RegOperation.h"
#include "RemoteSessionManager.h"

#define TS_SERVICE_INIT_TIMEOUT          60000
#define TS_SERVICE_COMMUNICATION_TIMEOUT 10000

static void InitSetting(PWRDS_CONNECTION_SETTINGS_1 Setting)
{
    Setting->WRdsListenerSettings.WRdsListenerSettingLevel = WRDS_LISTENER_SETTING_LEVEL_1;
    Setting->WRdsListenerSettings.WRdsListenerSetting.WRdsListenerSettings1.MaxProtocolListenerConnectionCount = 8;
    Setting->WRdsListenerSettings.WRdsListenerSetting.WRdsListenerSettings1.SecurityDescriptorSize = 0;
    Setting->WRdsListenerSettings.WRdsListenerSetting.WRdsListenerSettings1.pSecurityDescriptor = NULL;
}

CSparkWRdsConnection::CSparkWRdsConnection(DWORD ConnectionId, CRemoteSessionManager* Manager)
{
    m_lRef = 1;
    this->mConnectionID = ConnectionId;
    this->mPipeClient = CreateIPipeClientInstance(NEPPROVIDER_PIPE_NAME, NEPPROVIDER_PIPE_TIMEOUT);
    this->mPipeClient->SetParam(this);
    this->mPipeClient->RegisterEndHandle(CSparkWRdsConnection::DisConnectFromService);
    this->mPipeClient->RegisterRequestHandle(NEPCTRL_CALLBACK_ON_READY, CSparkWRdsConnection::OnReadyHandle);
    this->mPipeClient->RegisterRequestHandle(NEPCTRL_CALLBACK_ON_BROKEN_CONNECTION, CSparkWRdsConnection::OnBrokenConnectionHandle);
    this->mPipeClient->RegisterRequestHandle(NEPCTRL_CALLBACK_RELEASE, CSparkWRdsConnection::OnReleaseHandle);
    this->mCallBack = NULL;
    this->mOnConnectThread = NULL;
    this->mOnConnectThreadStoped = CreateEvent(NULL, FALSE, FALSE, NULL);
    this->mServiceInitOK = CreateEvent(NULL, FALSE, FALSE, NULL);
    this->mAccpept = FALSE;
    this->mRemoteAppConnection = FALSE;
    this->mSessionManager = Manager;
    memset(this->mDomainName, 0, sizeof(WCHAR) * MAX_PATH);
    memset(this->mUserName, 0, sizeof(WCHAR) * MAX_PATH);
	mSessionID = -1;
	mbEnableAudio = TRUE;
    DllAddRef();
}

CSparkWRdsConnection::~CSparkWRdsConnection()
{
    DRIVER_ENTER();

    this->mPipeClient->DisConnect();
    DestoryIPipeClientInstance(this->mPipeClient);
    DllRelease();

    if (this->mOnConnectThread)
    {
        DestoryIThreadInstance(this->mOnConnectThread);
    }

	if (this->mAccpept)
	{
		CSessionHelp::GetInstance()->Disable();
	}

	if (mCallBack)
	{
		mCallBack->Release();
		mCallBack = NULL;
	}

    DRIVER_LEAVE();
}

HRESULT CSparkWRdsConnection::QueryInterface(const IID&  iid, VOID** ppv)
{
    DRIVER_ENTER();

    TCHAR guidString[512];

    //类型为IUnknow和IWRdsProtocolManager才返回自己
    if (iid == IID_IUnknown)
    {
        DRIVER_INFO(_T("CSparkWRdsConnection::QueryInterface type IUnknow\r\n"));
        *ppv = this;
    }
    else if (iid == __uuidof(IWRdsProtocolConnection))
    {
        DRIVER_INFO(_T("CSparkWRdsConnection::QueryInterface type IWRdsProtocolListener\r\n"));
        *ppv = (IWRdsProtocolConnection*)this;
    }
    else if (iid == _uuidof(IWRdsRemoteFXGraphicsConnection))
    {
        DRIVER_INFO(_T("CSparkWRdsConnection::QueryInterface type IWRdsRemoteFXGraphicsConnection\r\n"));
        *ppv = (IWRdsRemoteFXGraphicsConnection*)this;
    }
    else
    {
        //其他一律返回E_NOINTERFACE
        DRIVER_ERROR(_T("CSparkWRdsConnection::QueryInterface %s. Returning E_NOINTERFACE.\r\n"), GUIDToString(iid, guidString));
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    //获取一次则增加引用
    this->AddRef();

    DRIVER_LEAVE();

    return S_OK;
}

ULONG CSparkWRdsConnection::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG CSparkWRdsConnection::Release()
{
    ULONG Ref = InterlockedDecrement(&m_lRef);

    if (Ref == 0)
    {
        delete this;
        return 0;
    }
    return Ref;
}

HRESULT CSparkWRdsConnection::GetLogonErrorRedirector(IWRdsProtocolLogonErrorRedirector **ppLogonErrorRedir)
{
    DRIVER_ENTER();

    *ppLogonErrorRedir = NULL;

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWRdsConnection::AcceptConnection()
{
    DRIVER_ENTER();

    if (!this->mAccpept)
    {
        DRIVER_INFO(_T("Wait OnConnect\r\n"));
        WaitForSingleObject(this->mOnConnectThreadStoped, INFINITE);

        if (WaitForSingleObject(this->mServiceInitOK, 0) != WAIT_OBJECT_0)
        {
            DRIVER_INFO(_T("Service Init Fail"));
            DRIVER_LEAVE();
            return E_FAIL;
        }

        DRIVER_INFO(_T("Service Init OK"));
        this->mAccpept = TRUE;
		CSessionHelp::GetInstance()->Enable();
    }
    DRIVER_LEAVE();
    return S_OK;
}
        
HRESULT CSparkWRdsConnection::GetClientData(WRDS_CLIENT_DATA *pClientData)
{
    NEPCTRL_GET_CLIENTINFO_REQUEST Request;
    PNEPCTRL_GET_CLIENTINFO_RESPONE Respone = NULL;
    DWORD RespLen;
    HRESULT Result;

    DRIVER_ENTER();

    Request.ConnectionInfo.ProviderType = NEP_PROVIDER_WRDS;
    Request.ConnectionInfo.ConnectionID = this->mConnectionID;

    if (!this->mPipeClient->SendRequestWithRespone(NEPCTRL_GET_CLIENTINFO, (PBYTE)&Request,
        sizeof(NEPCTRL_GET_CLIENTINFO_REQUEST), (PBYTE*)&Respone, &RespLen, TS_SERVICE_COMMUNICATION_TIMEOUT))
    {
        DRIVER_ERROR(_T("Send Reques Fail %d\r\n"), GetLastError());
        return E_FAIL;
    }

    if (Respone != NULL)
    {
        if (Respone->Result != ERROR_SUCCESS)
        {
            DRIVER_ERROR(_T("Respone Result Fail %d\r\n"), Respone->Result);
            Result = E_FAIL;
        }
        else
        {
			L_INFO(_T("enable audio %d \r\n"), !Respone->ClientData.fNoAudioPlayback);
			this->mbEnableAudio = !Respone->ClientData.fNoAudioPlayback;
            this->mRemoteAppConnection = Respone->RemoteAppConnection;
            memcpy(pClientData, &Respone->ClientData, sizeof(WRDS_CLIENT_DATA));
            DRIVER_INFO(_T("GetClientData OK\r\n"));
            Result = S_OK;
        }
        this->mPipeClient->ReleaseReply((PBYTE)Respone);
    }
    else
    {
        DRIVER_ERROR(_T("Respone Result Timout\r\n"));
        Result = E_FAIL;
    }

	DRIVER_INFO(_T("fDisableCtrlAltDel            :%d\r\n"), pClientData->fDisableCtrlAltDel);
	DRIVER_INFO(_T("fDoubleClickDetect            :%d\r\n"), pClientData->fDoubleClickDetect);
	DRIVER_INFO(_T("fEnableWindowsKey             :%d\r\n"), pClientData->fEnableWindowsKey);
	DRIVER_INFO(_T("fHideTitleBar                 :%d\r\n"), pClientData->fHideTitleBar);
	DRIVER_INFO(_T("fInheritAutoLogon             :%d\r\n"), pClientData->fInheritAutoLogon);
	DRIVER_INFO(_T("fPromptForPassword            :%d\r\n"), pClientData->fPromptForPassword);
	DRIVER_INFO(_T("fUsingSavedCreds              :%d\r\n"), pClientData->fUsingSavedCreds);
	DRIVER_INFO(_T("Domain                        :%s\r\n"), pClientData->Domain);
	DRIVER_INFO(_T("UserName                      :%s\r\n"), pClientData->UserName);
	DRIVER_INFO(_T("Password                      :%s\r\n"), pClientData->Password);
	DRIVER_INFO(_T("fPasswordIsScPin              :%d\r\n"), pClientData->fPasswordIsScPin);
	DRIVER_INFO(_T("fInheritInitialProgram        :%d\r\n"), pClientData->fInheritInitialProgram);
	DRIVER_INFO(_T("WorkDirectory                 :%s\r\n"), pClientData->WorkDirectory);
	DRIVER_INFO(_T("InitialProgram                :%s\r\n"), pClientData->InitialProgram);
	DRIVER_INFO(_T("fMaximizeShell                :%d\r\n"), pClientData->fMaximizeShell);
	DRIVER_INFO(_T("EncryptionLevel               :%d\r\n"), pClientData->EncryptionLevel);
	DRIVER_INFO(_T("PerformanceFlags              :%d\r\n"), pClientData->PerformanceFlags);
	DRIVER_INFO(_T("ProtocolName                  :%s\r\n"), pClientData->ProtocolName);
	DRIVER_INFO(_T("ProtocolType                  :%d\r\n"), pClientData->ProtocolType);
	DRIVER_INFO(_T("fInheritColorDepth            :%d\r\n"), pClientData->fInheritColorDepth);
	DRIVER_INFO(_T("HRes                          :%d\r\n"), pClientData->HRes);
	DRIVER_INFO(_T("VRes                          :%d\r\n"), pClientData->VRes);
	DRIVER_INFO(_T("ColorDepth                    :%d\r\n"), pClientData->ColorDepth);
	DRIVER_INFO(_T("DisplayDriverName             :%s\r\n"), pClientData->DisplayDriverName);
	DRIVER_INFO(_T("DisplayDeviceName             :%s\r\n"), pClientData->DisplayDeviceName);
	DRIVER_INFO(_T("fMouse                        :%d\r\n"), pClientData->fMouse);
	DRIVER_INFO(_T("KeyboardLayout                :%d\r\n"), pClientData->KeyboardLayout);
	DRIVER_INFO(_T("KeyboardType                  :%d\r\n"), pClientData->KeyboardType);
	DRIVER_INFO(_T("KeyboardSubType               :%d\r\n"), pClientData->KeyboardSubType);
	DRIVER_INFO(_T("KeyboardFunctionKey           :%d\r\n"), pClientData->KeyboardFunctionKey);
	DRIVER_INFO(_T("imeFileName                   :%s\r\n"), pClientData->imeFileName);
	DRIVER_INFO(_T("ActiveInputLocale             :%d\r\n"), pClientData->ActiveInputLocale);
	DRIVER_INFO(_T("fNoAudioPlayback              :%d\r\n"), pClientData->fNoAudioPlayback);
	DRIVER_INFO(_T("fRemoteConsoleAudio           :%d\r\n"), pClientData->fRemoteConsoleAudio);
	DRIVER_INFO(_T("AudioDriverName               :%s\r\n"), pClientData->AudioDriverName);

	DRIVER_INFO(_T("ClientName                    :%s\r\n"), pClientData->ClientName);
	DRIVER_INFO(_T("SerialNumber                  :%d\r\n"), pClientData->SerialNumber);
	DRIVER_INFO(_T("ClientAddressFamily           :%d\r\n"), pClientData->ClientAddressFamily);
	DRIVER_INFO(_T("ClientAddress                 :%s\r\n"), pClientData->ClientAddress);
	DRIVER_INFO(_T("ClientDirectory               :%s\r\n"), pClientData->ClientDirectory);
	DRIVER_INFO(_T("ClientBuildNumber             :%d\r\n"), pClientData->ClientBuildNumber);
	DRIVER_INFO(_T("ClientProductId               :%d\r\n"), pClientData->ClientProductId);
	DRIVER_INFO(_T("OutBufCountHost               :%d\r\n"), pClientData->OutBufCountHost);
	DRIVER_INFO(_T("OutBufCountClient             :%d\r\n"), pClientData->OutBufCountClient);
	DRIVER_INFO(_T("OutBufLength                  :%d\r\n"), pClientData->OutBufLength);
	DRIVER_INFO(_T("ClientSessionId               :%d\r\n"), pClientData->OutBufLength);
	DRIVER_INFO(_T("ClientDigProductId            :%s\r\n"), pClientData->ClientDigProductId);
	DRIVER_INFO(_T("fDisableCpm            :%d\r\n"), pClientData->fDisableCpm);
	DRIVER_INFO(_T("fDisableCdm            :%d\r\n"), pClientData->fDisableCdm);
	DRIVER_INFO(_T("fDisableCcm            :%d\r\n"), pClientData->fDisableCcm);
	DRIVER_INFO(_T("fDisableLPT            :%d\r\n"), pClientData->fDisableLPT);
	DRIVER_INFO(_T("fDisableClip            :%d\r\n"), pClientData->fDisableClip);
	DRIVER_INFO(_T("fDisablePNP            :%d\r\n"), pClientData->fDisablePNP);
	

	//swprintf(pClientData->DisplayDeviceName, L"NcVideo");
    DRIVER_LEAVE();

    return Result;
}

HRESULT CSparkWRdsConnection::GetClientMonitorData(UINT *pNumMonitors, UINT *pPrimaryMonitor)
{
    NEPCTRL_GET_DISPLAYINFO_REQUEST Request;
    PNEPCTRL_GET_DISPLAYINFO_RESPONE Respone = NULL;
    DWORD RespLen;
    HRESULT Result;

    DRIVER_ENTER();

    Request.ConnectionInfo.ProviderType = NEP_PROVIDER_WRDS;
    Request.ConnectionInfo.ConnectionID = this->mConnectionID;

    if (!this->mPipeClient->SendRequestWithRespone(NEPCTRL_GET_DISPLAYINFO, (PBYTE)&Request,
        sizeof(NEPCTRL_GET_DISPLAYINFO_REQUEST), (PBYTE*)&Respone, &RespLen, TS_SERVICE_COMMUNICATION_TIMEOUT))
    {
        DRIVER_ERROR(_T("Send Reques Fail %d\r\n"), GetLastError());
        return E_FAIL;
    }

    if (Respone != NULL)
    {
        if (Respone->Result != ERROR_SUCCESS)
        {
            DRIVER_ERROR(_T("Respone Result Fail %d\r\n"), Respone->Result);
            Result = E_FAIL;
        }
        else
        {
            *pNumMonitors = Respone->DisplayCount;
            *pPrimaryMonitor = Respone->PrimaryDisplay;
            DRIVER_INFO(_T("GetClientMonitorData Count %d, Primary %d\r\n"), *pNumMonitors, *pPrimaryMonitor);
            Result = S_OK;
        }
        this->mPipeClient->ReleaseReply((PBYTE)Respone);
    }
    else
    {
        DRIVER_ERROR(_T("Respone Result Timout\r\n"));
        Result = E_FAIL;
    }

    DRIVER_LEAVE();

    return Result;
}
        
HRESULT CSparkWRdsConnection::GetUserCredentials(WRDS_USER_CREDENTIAL *pUserCreds)
{
    NEPCTRL_GET_USERINFO_REQUEST Request;
    PNEPCTRL_GET_USERINFO_RESPONE Respone = NULL;
    DWORD RespLen;
    HRESULT Result = E_NOTIMPL; 

    DRIVER_ENTER();

    Request.ConnectionInfo.ProviderType = NEP_PROVIDER_WRDS;
    Request.ConnectionInfo.ConnectionID = this->mConnectionID;

    if (!this->mPipeClient->SendRequestWithRespone(NEPCTRL_GET_USERINFO, (PBYTE)&Request,
        sizeof(NEPCTRL_GET_USERINFO_REQUEST), (PBYTE*)&Respone, &RespLen, TS_SERVICE_COMMUNICATION_TIMEOUT))
    {
        DRIVER_ERROR(_T("Send Reques Fail %d\r\n"), GetLastError());
        return E_FAIL;
    }

    if (Respone != NULL)
    {
        if (Respone->Result != ERROR_SUCCESS)
        {
            DRIVER_ERROR(_T("Respone Result Fail %d\r\n"), Respone->Result);
            Result = E_FAIL;
        }
        else
        {
            wcscpy(pUserCreds->Domain, Respone->Domain);
            wcscpy(pUserCreds->UserName, Respone->Username);
            wcscpy(pUserCreds->Password, Respone->Password);
            DRIVER_INFO(_T("GetUserCredentials Domain %s, Username %s Password %s\r\n"), pUserCreds->Domain, pUserCreds->UserName, pUserCreds->Password);
            Result = S_OK;
        }
        this->mPipeClient->ReleaseReply((PBYTE)Respone);
    }
    else
    {
        DRIVER_ERROR(_T("Respone Result Timout\r\n"));
        Result = E_FAIL;
    }

    DRIVER_LEAVE();

    return Result;
}
        
HRESULT CSparkWRdsConnection::GetLicenseConnection(IWRdsProtocolLicenseConnection **ppLicenseConnection)
{
    DRIVER_ENTER();

	CSessionHelp::GetInstance()->Install();

    CNepWRdsLicenseConnection *License = new CNepWRdsLicenseConnection();

    *ppLicenseConnection = License;

    DRIVER_LEAVE();

    return S_OK;
}
       
HRESULT CSparkWRdsConnection::AuthenticateClientToSession(WRDS_SESSION_ID *SessionId)
{
	
    DRIVER_ENTER();
	
    DRIVER_LEAVE();

	return E_NOTIMPL;
}
        
HRESULT CSparkWRdsConnection::NotifySessionId(WRDS_SESSION_ID *SessionId, HANDLE_PTR SessionHandle)
{
	NEPCTRL_NOTIFY_SESSION_ID_REQUEST Request;
    DRIVER_ENTER();
	this->mSessionID = SessionId->SessionId;
	DRIVER_INFO(_T("Connect to Session %d\r\n"), SessionId->SessionId);

	Request.ConnectionInfo.ProviderType = NEP_PROVIDER_WTS;
	Request.ConnectionInfo.ConnectionID = this->mConnectionID;
	Request.SessionID = this->mSessionID;
	L_INFO(_T("notify session id %d\r\n"), Request.SessionID);
	this->mPipeClient->SendRequest(NEPCTRL_NOTIFY_SESSIONID, (PBYTE)&Request, sizeof(NEPCTRL_NOTIFY_SESSION_ID_REQUEST));
    DRIVER_LEAVE();

    return S_OK;
}
 
HRESULT CSparkWRdsConnection::GetInputHandles(HANDLE_PTR *pKeyboardHandle, HANDLE_PTR *pMouseHandle, HANDLE_PTR *pBeepHandle)
{
    NEPCTRL_GET_DRIVERINFO_REQUEST Request;
    PNEPCTRL_GET_DRIVERINFO_RESPONE Respone = NULL;
    DWORD RespLen;
    HRESULT Result;

    DRIVER_ENTER();

    Request.Pid = GetCurrentProcessId();
    Request.ConnectionInfo.ProviderType = NEP_PROVIDER_WRDS;
    Request.ConnectionInfo.ConnectionID = this->mConnectionID;

    if (!this->mPipeClient->SendRequestWithRespone(NEPCTRL_GET_DRIVERINFO, (PBYTE)&Request,
        sizeof(NEPCTRL_GET_DRIVERINFO_REQUEST), (PBYTE*)&Respone, &RespLen, TS_SERVICE_COMMUNICATION_TIMEOUT))
    {
        DRIVER_ERROR(_T("Send Reques Fail %d\r\n"), GetLastError());
        return E_FAIL;
    }

    if (Respone != NULL)
    {
        if (Respone->Result != ERROR_SUCCESS)
        {
            DRIVER_ERROR(_T("Respone Result Fail %d\r\n"), Respone->Result);
            Result = E_FAIL;
        }
        else
        {
            *pKeyboardHandle = (HANDLE_PTR)Respone->KeyBroadHandle;
            *pMouseHandle = (HANDLE_PTR)Respone->MouseHandle;
			*pBeepHandle = (HANDLE_PTR)NULL;//Respone->BeepHandle;
            //CloseHandle((HANDLE)Respone->VideoHandle);
            DRIVER_INFO(_T("GetInputHandles Keybroad %x, Mouse %x Beep %x\r\n"), *pKeyboardHandle, *pMouseHandle, *pBeepHandle);
            Result = S_OK;
        }
        this->mPipeClient->ReleaseReply((PBYTE)Respone);
    }
    else
    {
        DRIVER_ERROR(_T("Respone Result Timout\r\n"));
        Result = E_FAIL;
    }

    DRIVER_LEAVE();

    return Result;
}

HANDLE OpenDevice(WCHAR *DeviceName)
{
	return CreateFile(DeviceName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
}

HRESULT CSparkWRdsConnection::GetVideoHandle(HANDLE_PTR *pVideoHandle)
{
    NEPCTRL_GET_DRIVERINFO_REQUEST Request;
    PNEPCTRL_GET_DRIVERINFO_RESPONE Respone = NULL;
    DWORD RespLen;
    HRESULT Result;

    DRIVER_ENTER();

#if 0
    *pVideoHandle = NULL;
    DRIVER_INFO(_T("GetVideoHandle %x\r\n"), *pVideoHandle);
    Result = S_OK;

    DRIVER_LEAVE();

    return Result;

#else
    Request.Pid = GetCurrentProcessId();
    Request.ConnectionInfo.ProviderType = NEP_PROVIDER_WRDS;
    Request.ConnectionInfo.ConnectionID = this->mConnectionID;

    if (!this->mPipeClient->SendRequestWithRespone(NEPCTRL_GET_DRIVERINFO, (PBYTE)&Request,
        sizeof(NEPCTRL_GET_DRIVERINFO_REQUEST), (PBYTE*)&Respone, &RespLen, TS_SERVICE_COMMUNICATION_TIMEOUT))
    {
        DRIVER_ERROR(_T("Send Reques Fail %d\r\n"), GetLastError());
        return E_FAIL;
    }

    if (Respone != NULL)
    {
        if (Respone->Result != ERROR_SUCCESS)
        {
            DRIVER_ERROR(_T("Respone Result Fail %d\r\n"), Respone->Result);
            Result = E_FAIL;
        }
        else
        {
            CloseHandle((HANDLE)Respone->KeyBroadHandle);
            CloseHandle((HANDLE)Respone->MouseHandle);
			*pVideoHandle = NULL;// (HANDLE_PTR)OpenDevice(TEXT("\\\\.\\XSTA0")); //(HANDLE_PTR)Respone->VideoHandle;
            DRIVER_INFO(_T("GetVideoHandle %x\r\n"), *pVideoHandle);
            Result = S_OK;
        }
        this->mPipeClient->ReleaseReply((PBYTE)Respone);
    }
    else
    {
        DRIVER_ERROR(_T("Respone Result Timout\r\n"));
        Result = E_FAIL;
    }

    DRIVER_LEAVE();

    return Result;
#endif
}
        
HRESULT CSparkWRdsConnection::ConnectNotify(ULONG SessionId)
{
    DRIVER_ENTER();

    DRIVER_INFO(_T("ConnectNotify %d\r\n"), SessionId);

    DRIVER_LEAVE();

    return S_OK;
}
               
HRESULT CSparkWRdsConnection::NotifyCommandProcessCreated(ULONG SessionId)
{
    DRIVER_ENTER();

    DRIVER_INFO(_T("NotifyCommandProcessCreated %d\r\n"), SessionId);

    DRIVER_LEAVE();

    return S_OK;
}
    
HRESULT CSparkWRdsConnection::IsUserAllowedToLogon(ULONG SessionId, HANDLE_PTR UserToken, WCHAR *pDomainName, WCHAR *pUserName)
{
    DRIVER_ENTER();
    DRIVER_LEAVE();

    wcsncpy(this->mDomainName, pDomainName, MAX_PATH);
    wcsncpy(this->mUserName, pUserName, MAX_PATH);

    DRIVER_INFO(_T("Login Check OK %s/%s Session %d \r\n"), pDomainName, pUserName, SessionId);

    return S_OK;
}
        
HRESULT CSparkWRdsConnection::SessionArbitrationEnumeration(HANDLE_PTR hUserToken,
    BOOL bSingleSessionPerUserEnabled, ULONG *pSessionIdArray, ULONG *pdwSessionIdentifierCount)
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

    DRIVER_INFO(_T("Current Name %s/%s\r\n"), this->mDomainName, this->mUserName);

    Session = this->mSessionManager->FindNeptuneConnectedSessionID(this->mDomainName, this->mUserName, this->mRemoteAppConnection);

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
			this->mSessionManager->ReplaceToIvy(Session);
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
        
HRESULT CSparkWRdsConnection::LogonNotify(
    HANDLE_PTR hClientToken,
    WCHAR *wszUserName,
    WCHAR *wszDomainName,
    WRDS_SESSION_ID *SessionId,
    PWRDS_CONNECTION_SETTINGS pWRdsConnectionSettings)
{
    DRIVER_ENTER();
    DRIVER_LEAVE();

    return S_OK;
}


HRESULT CSparkWRdsConnection::PreDisconnect(ULONG DisconnectReason)
{
    DRIVER_ENTER();

    DRIVER_INFO(_T("PreDisconnect %d\r\n"), DisconnectReason);

    DRIVER_LEAVE();

    return S_OK;
}
 
HRESULT CSparkWRdsConnection::DisconnectNotify()
{
    DRIVER_ENTER();
    DRIVER_LEAVE();

    return S_OK;
}
  
HRESULT CSparkWRdsConnection::Close()
{
    DRIVER_ENTER();
    
    NEPCTRL_CLOSE_CONNECTION_REQUEST Request;
    Request.ConnectionInfo.ProviderType = NEP_PROVIDER_WRDS;
    Request.ConnectionInfo.ConnectionID = this->mConnectionID;

    this->mPipeClient->SendRequest(NEPCTRL_CLOSE_CONNECTION, (PBYTE)&Request, sizeof(NEPCTRL_GET_DRIVERINFO_REQUEST));

	if (mCallBack)
	{
		mCallBack->Release();
		mCallBack = NULL;
	}

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWRdsConnection::GetProtocolStatus(WRDS_PROTOCOL_STATUS *pProtocolStatus)
{

	ZeroMemory(pProtocolStatus, sizeof(WRDS_PROTOCOL_STATUS));
	return S_OK;
  
}
    
HRESULT CSparkWRdsConnection::GetLastInputTime(ULONG64 *pLastInputTime)
{
    DRIVER_ENTER();

    *pLastInputTime = 1000;
    
    DRIVER_LEAVE();
    return S_OK;
}
    
HRESULT CSparkWRdsConnection::SetErrorInfo(ULONG ulError)
{
    DRIVER_ENTER();

    DRIVER_ERROR(_T("Error 0x%08x\r\n"), ulError);

    DRIVER_LEAVE();

    return S_OK;
}
    
HRESULT CSparkWRdsConnection::CreateVirtualChannel(CHAR *szEndpointName, BOOL bStatic, ULONG RequestedPriority, ULONG_PTR *phChannel)
{
    DRIVER_ENTER();
    DRIVER_LEAVE();

    return E_NOTIMPL;
}
        
HRESULT CSparkWRdsConnection::QueryProperty(GUID QueryType, ULONG ulNumEntriesIn, ULONG ulNumEntriesOut,
    PWRDS_PROPERTY_VALUE pPropertyEntriesIn, PWRDS_PROPERTY_VALUE pPropertyEntriesOut)
{
    TCHAR GuidStr[512];
    HRESULT Result = E_NOTIMPL;
    DRIVER_ENTER();

    GUIDToString(QueryType, GuidStr);

    DRIVER_INFO(_T("QueryType %s %d %d"), GuidStr, ulNumEntriesIn, ulNumEntriesOut);

    if ( QueryType == WRDS_QUERY_ALLOWED_INITIAL_APP )
    {
        DRIVER_TRACE(L"WTS_QUERY_ALLOWED_INITIAL_APP\r\n");
    }
    else if ( QueryType == WRDS_QUERY_LOGON_SCREEN_SIZE )
    {
        DRIVER_TRACE(L"WTS_QUERY_LOGON_SCREEN_SIZE\r\n");
    }
    else if ( QueryType == WRDS_QUERY_AUDIOENUM_DLL )
    {
        DRIVER_TRACE(L"WTS_QUERY_AUDIOENUM_DLL\r\n");
		if (pPropertyEntriesOut)
		{
			DWORD Length = MAX_PATH;
			pPropertyEntriesOut->Type = WTS_VALUE_TYPE_STRING;
			pPropertyEntriesOut->u.strVal.pstrVal = (WCHAR*)LocalAlloc(0, MAX_PATH * sizeof(WCHAR));

			memset(pPropertyEntriesOut->u.strVal.pstrVal, 0, MAX_PATH * sizeof(WCHAR));
			if (mbEnableAudio)
			{
				CRegOperation::RegReadString(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\Ivy"),
					_T("AudioEnumeratorDll"), pPropertyEntriesOut->u.strVal.pstrVal, &Length, _T(""));
			}
			

			pPropertyEntriesOut->u.strVal.size = MAX_PATH;
			Result = S_OK;
		}
    }
    else if ( QueryType == WRDS_QUERY_MF_FORMAT_SUPPORT )
    {
        DRIVER_TRACE(L"WTS_QUERY_MF_FORMAT_SUPPORT\r\n");
    }

    DRIVER_LEAVE();

    return Result;
}

HRESULT CSparkWRdsConnection::GetShadowConnection(IWRdsProtocolShadowConnection **ppShadowConnection)
{
    DRIVER_ENTER();

    *ppShadowConnection = NULL;

    DRIVER_LEAVE();

    return E_NOTIMPL;
}

void CSparkWRdsConnection::SetCallBack(IWRdsProtocolConnectionCallback* CallBack)
{
    this->mCallBack = CallBack;
}

void CSparkWRdsConnection::SetOnConnectThread(IThread* OnConnectThread, IWRdsProtocolListenerCallback* ListenrCallBack)
{
    this->mOnConnectThread = OnConnectThread;
    this->mListenrCallBack = ListenrCallBack;
}

void CSparkWRdsConnection::OnReadyHandle(BYTE* Buffer, DWORD BufferLen, DWORD Id, ICommunication* Param)
{
    IPipeClient* Client = dynamic_cast<IPipeClient*>(Param);
    CSparkWRdsConnection* Connection = (CSparkWRdsConnection*)Client->GetParam();

    DRIVER_INFO(_T("NepService Notify OnReady\r\n"));

    if (!Connection->mCallBack)
    {
        DRIVER_ERROR(_T("Connection do not has CallBack\r\n"));
        return;
    }

    Connection->mCallBack->OnReady();

    return;
}

void CSparkWRdsConnection::OnBrokenConnectionHandle(BYTE* Buffer, DWORD BufferLen, DWORD Id, ICommunication* Param)
{
    IPipeClient* Client = dynamic_cast<IPipeClient*>(Param);
    CSparkWRdsConnection* Connection = (CSparkWRdsConnection*)Client->GetParam();

    DRIVER_INFO(_T("NepService Notify OnBrokenConnection\r\n"));

    if (!Connection->mCallBack)
    {
        DRIVER_ERROR(_T("Connection do not has CallBack\r\n"));
        return;
    }

    Connection->mCallBack->BrokenConnection(0, 0);
	Connection->mCallBack->Release();
	Connection->mCallBack = NULL;
    return;
}

void CSparkWRdsConnection::OnReleaseHandle(BYTE* Buffer, DWORD BufferLen, DWORD Id, ICommunication* Param)
{
    IPipeClient* Client = dynamic_cast<IPipeClient*>(Param);
    CSparkWRdsConnection* Connection = (CSparkWRdsConnection*)Client->GetParam();

    DRIVER_INFO(_T("NepService Notify OnRelease\r\n"));

    if (!Connection->mCallBack)
    {
        DRIVER_ERROR(_T("Connection do not has CallBack\r\n"));
        return;
    }

    Connection->mCallBack->Release();
    Connection->mCallBack = NULL;

    return;
}

void CSparkWRdsConnection::DisConnectFromService(ICommunication* Param)
{
    IPipeClient* Client = dynamic_cast<IPipeClient*>(Param);
    CSparkWRdsConnection* Connection = (CSparkWRdsConnection*)Client->GetParam();

    //如果通道突然断开，直接通知TS释放
    if (!Connection->mCallBack)
    {
        DRIVER_ERROR(_T("Connection do not has CallBack\r\n"));
        return;
    }

    Connection->mCallBack->BrokenConnection(0, 0);
    Connection->mCallBack->Release();
    Connection->mCallBack = NULL;

    return;
}

HRESULT CSparkWRdsConnection::EnableRemoteFXGraphics(BOOL *pEnableRemoteFXGraphics)
{
    DRIVER_ENTER();

    *pEnableRemoteFXGraphics = FALSE;

    DRIVER_LEAVE();

    return S_OK;
}
        
HRESULT CSparkWRdsConnection::GetVirtualChannelTransport(IUnknown **ppTransport)
{
    DRIVER_ENTER();
    DRIVER_LEAVE();

    return E_NOTIMPL;
}

BOOL CSparkWRdsConnection::OnConnectThreadProcess(LPVOID param, HANDLE stopevent)
{
    HRESULT Result;
    IWRdsProtocolConnectionCallback* ConnectionCallBack = NULL;
    CSparkWRdsConnection* Connection = (CSparkWRdsConnection*)param;
    WRDS_CONNECTION_SETTINGS Setting;
    Setting.WRdsConnectionSettingLevel = WRDS_CONNECTION_SETTING_LEVEL_1;
    memset(&Setting.WRdsConnectionSetting, 0, sizeof(WRDS_CONNECTION_SETTINGS_1));
    InitSetting(&Setting.WRdsConnectionSetting.WRdsConnectionSettings1);

    DRIVER_TRACE(_T("Call OnConnect\r\n"));

    if (Connection->mListenrCallBack)
    {

		WRDS_CONNECTION_SETTINGS* pWRdsConnectionSettings = &Setting;
		DRIVER_INFO(_T("WRdsConnectionSettingLevel %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSettingLevel);
		DRIVER_INFO(_T("WRdsConnectionSetting:  \r\n"));
		DRIVER_INFO(_T("fInheritInitialProgram: %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fInheritInitialProgram);
		DRIVER_INFO(_T("fInheritColorDepth:     %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fInheritColorDepth);
		DRIVER_INFO(_T("fHideTitleBar:          %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fHideTitleBar);
		DRIVER_INFO(_T("fInheritAutoLogon:      %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fInheritAutoLogon);
		DRIVER_INFO(_T("fMaximizeShell:         %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fMaximizeShell);
		DRIVER_INFO(_T("fDisablePNP:            %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fDisablePNP);
		DRIVER_INFO(_T("fPasswordIsScPin:       %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fPasswordIsScPin);
		DRIVER_INFO(_T("fPromptForPassword:     %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fPromptForPassword);
		DRIVER_INFO(_T("fDisableCpm:            %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fDisableCpm);
		DRIVER_INFO(_T("fDisableCdm:            %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fDisableCdm);
		DRIVER_INFO(_T("fDisableCcm:            %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fDisableCcm);
		DRIVER_INFO(_T("fDisableLPT:            %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fDisableLPT);
		DRIVER_INFO(_T("fDisableClip:           %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fDisableClip);
		DRIVER_INFO(_T("fResetBroken:           %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fResetBroken);
		DRIVER_INFO(_T("fDisableEncryption:     %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fDisableEncryption);
		DRIVER_INFO(_T("fDisableAutoReconnect:  %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fDisableAutoReconnect);
		DRIVER_INFO(_T("fDisableCtrlAltDel:     %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fDisableCtrlAltDel);
		DRIVER_INFO(_T("fDoubleClickDetect:     %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fDoubleClickDetect);
		DRIVER_INFO(_T("fEnableWindowsKey:      %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fEnableWindowsKey);
		DRIVER_INFO(_T("fUsingSavedCreds:       %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fUsingSavedCreds);
		DRIVER_INFO(_T("fMouse:                 %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fMouse);
		DRIVER_INFO(_T("fNoAudioPlayback:       %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fNoAudioPlayback);
		DRIVER_INFO(_T("fRemoteConsoleAudio:    %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.fRemoteConsoleAudio);
		DRIVER_INFO(_T("EncryptionLevel:        %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.EncryptionLevel);
		DRIVER_INFO(_T("ColorDepth:             %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.ColorDepth);
		DRIVER_INFO(_T("ProtocolType:           %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.ProtocolType);
		DRIVER_INFO(_T("HRes:                   %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.HRes);
		DRIVER_INFO(_T("VRes:                   %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.VRes);
		DRIVER_INFO(_T("ClientProductId:        %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.ClientProductId);
		DRIVER_INFO(_T("OutBufCountHost:        %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.OutBufCountHost);
		DRIVER_INFO(_T("OutBufCountClient:      %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.OutBufCountClient);
		DRIVER_INFO(_T("OutBufLength:           %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.OutBufLength);
		DRIVER_INFO(_T("KeyboardLayout:         %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.KeyboardLayout);
		DRIVER_INFO(_T("MaxConnectionTime:      %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.MaxConnectionTime);
		DRIVER_INFO(_T("MaxDisconnectionTime:   %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.MaxDisconnectionTime);
		DRIVER_INFO(_T("MaxIdleTime:            %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.MaxIdleTime);
		DRIVER_INFO(_T("PerformanceFlags:       %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.PerformanceFlags);
		DRIVER_INFO(_T("KeyboardType:           %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.KeyboardType);
		DRIVER_INFO(_T("KeyboardSubType:        %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.KeyboardSubType);
		DRIVER_INFO(_T("KeyboardFunctionKey:    %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.KeyboardFunctionKey);
		DRIVER_INFO(_T("ActiveInputLocale:      %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.ActiveInputLocale);
		DRIVER_INFO(_T("SerialNumber:           %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.SerialNumber);
		DRIVER_INFO(_T("ClientAddressFamily:    %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.ClientAddressFamily);
		DRIVER_INFO(_T("ClientBuildNumber:      %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.ClientBuildNumber);
		DRIVER_INFO(_T("ClientSessionId:        %d\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.ClientSessionId);

		DRIVER_INFO(_T("WorkDirectory:          %s\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.WorkDirectory);
		DRIVER_INFO(_T("InitialProgram:         %s\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.InitialProgram);
		DRIVER_INFO(_T("UserName:               %s\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.UserName);
		DRIVER_INFO(_T("Domain:                 %s\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.Domain);
		DRIVER_INFO(_T("Password:               %s\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.Password);
		DRIVER_INFO(_T("ProtocolName:           %s\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.ProtocolName);
		DRIVER_INFO(_T("DisplayDriverName:      %s\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.DisplayDriverName);
		DRIVER_INFO(_T("DisplayDeviceName:      %s\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.DisplayDeviceName);
		DRIVER_INFO(_T("imeFileName:            %s\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.imeFileName);
		DRIVER_INFO(_T("AudioDriverName:        %s\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.AudioDriverName);
		DRIVER_INFO(_T("ClientName:             %s\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.ClientName);
		DRIVER_INFO(_T("ClientAddress:          %s\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.ClientAddress);
		DRIVER_INFO(_T("ClientDirectory:        %s\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.ClientDirectory);
		DRIVER_INFO(_T("ClientDigProductId:     %s\r\n"), pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.ClientDigProductId);
		DRIVER_INFO(_T("Hook GetSettings MaxProtocolListenerConnectionCount %d %x %x %x\r\n"),
			pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.WRdsListenerSettings.WRdsListenerSettingLevel,
			pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.WRdsListenerSettings.WRdsListenerSetting.WRdsListenerSettings1.MaxProtocolListenerConnectionCount,
			pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.WRdsListenerSettings.WRdsListenerSetting.WRdsListenerSettings1.pSecurityDescriptor,
			pWRdsConnectionSettings->WRdsConnectionSetting.WRdsConnectionSettings1.WRdsListenerSettings.WRdsListenerSetting.WRdsListenerSettings1.SecurityDescriptorSize);


        Result = Connection->mListenrCallBack->OnConnected(Connection, &Setting, &ConnectionCallBack);
		
        if (Result != S_OK)
        {
           DRIVER_ERROR(_T("OnConnected Fail %x\r\n"), Result);
            Connection->Release();
            return FALSE;
        }
    
        Connection->SetCallBack(ConnectionCallBack);

        DRIVER_TRACE(_T("SetCallBack OK\r\n"));

        //发送连接消息
        NEPCTRL_ACCEPT_CONNECTION_REQUEST Request;
        PNEPCTRL_ACCEPT_CONNECTION_RESPONE Respone = NULL;
        DWORD ResponeLen;

        if (!Connection->mPipeClient->Connect())
        {
            DRIVER_ERROR(_T("Pipe Connect Fail %d\r\n"), GetLastError());
            goto exit;
        }

        Request.ConnectionInfo.ProviderType = NEP_PROVIDER_WRDS;
        Request.ConnectionInfo.ConnectionID = Connection->mConnectionID;

        if (!Connection->mPipeClient->SendRequestWithRespone(NEPCTRL_ACCEPT_CONNECTION, 
            (PBYTE)&Request, sizeof(NEPCTRL_ACCEPT_CONNECTION_REQUEST),
            (PBYTE*)&Respone, &ResponeLen, TS_SERVICE_INIT_TIMEOUT))
        {
            DRIVER_ERROR(_T("Send Reques Fail %d\r\n"), GetLastError());
            goto exit;
        }

        if (Respone != NULL)
        {
            if (Respone->Result != ERROR_SUCCESS)
            {
                DRIVER_ERROR(_T("Respone Result Fail %d\r\n"), Respone->Result);
            }
            else
            {
                SetEvent(Connection->mServiceInitOK);
                DRIVER_INFO(_T("AcceptConnection OK\r\n"));
            }
            Connection->mPipeClient->ReleaseReply((PBYTE)Respone);
        }
        else
        {
            DRIVER_ERROR(_T("Respone Result Timout\r\n"));
        }

        goto exit;
    }

exit:
    SetEvent(Connection->mOnConnectThreadStoped);
	Connection->Release();
    return FALSE;
}