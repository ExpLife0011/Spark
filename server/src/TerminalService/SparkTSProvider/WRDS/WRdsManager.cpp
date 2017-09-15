#include "stdafx.h"
#include "Log\LogEx.h"
#include "WRDS\WRdsManager.h"
#include "WRDS\WRdsListener.h"
#include "RemoteSessionManager.h"
#include "SessionHelper\SessionHelp.h"


CSparkWRDSManager::CSparkWRDSManager()
{
    m_lRef = 1;
    m_pSessionManager = new CRemoteSessionManager();
    DllAddRef();
}

CSparkWRDSManager::~CSparkWRDSManager()
{
	CSessionHelp::GetInstance()->Uninstall();
    m_pSessionManager->Release();
    DllRelease();
}

HRESULT CSparkWRDSManager::QueryInterface(const IID&  iid, VOID** ppv)
{
    DRIVER_ENTER();

    TCHAR guidString[512];

    if (iid == IID_IUnknown)
    {
        DRIVER_INFO(_T("CSparkWRDSManager::QueryInterface type IUnknow\r\n"));
        *ppv = this;
    }
    else if (iid == __uuidof(IWRdsProtocolManager))
    {
        DRIVER_INFO(_T("CSparkWRDSManager::QueryInterface type IWRdsProtocolManager\r\n"));
        *ppv = this;
    }
    else
    {
        DRIVER_ERROR(_T("CSparkWRDSManager::QueryInterface %s. Returning E_NOINTERFACE.\r\n"), GUIDToString(iid, guidString));
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();

    DRIVER_LEAVE();

    return S_OK;
}

ULONG CSparkWRDSManager::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG CSparkWRDSManager::Release()
{
    ULONG Ref = InterlockedDecrement(&m_lRef);

    if (Ref == 0)
    {
        delete this;
        return 0;
    }
    return Ref;
}

HRESULT CSparkWRDSManager::Initialize(IWRdsProtocolSettings *pIWRdsSettings, PWRDS_SETTINGS pWRdsSettings)
{
    UNREFERENCED_PARAMETER(pIWRdsSettings);
    UNREFERENCED_PARAMETER(pWRdsSettings);

    DRIVER_ENTER();
    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWRDSManager::Uninitialize()
{
    DRIVER_ENTER();
    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWRDSManager::CreateListener(WCHAR *wszListenerName, IWRdsProtocolListener **pProtocolListener)
{
    HRESULT Result = S_OK;

    DRIVER_ENTER();
    
    DRIVER_INFO(_T("Listener Name %s\n"), wszListenerName ? wszListenerName : _T("NULL"));

    CSparkWRdsListener* Listener = new CSparkWRdsListener(m_pSessionManager);
    if (Listener)
    {
        *pProtocolListener = Listener;
    }
    else
    {
        Result = E_OUTOFMEMORY;
    }

    DRIVER_LEAVE();

    return Result;
}

HRESULT CSparkWRDSManager::NotifyServiceStateChange(WRDS_SERVICE_STATE *pTSServiceStateChange)
{
    DRIVER_ENTER();

    if (pTSServiceStateChange)
    {
        DRIVER_INFO(L"RcmDrainState %d, RcmServiceState %d\n",
            pTSServiceStateChange->RcmDrainState, pTSServiceStateChange->RcmServiceState);
    }

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWRDSManager::NotifySessionOfServiceStart(WRDS_SESSION_ID *SessionId)
{
    UNREFERENCED_PARAMETER(SessionId);

    DRIVER_ENTER();
    DRIVER_LEAVE();

    return E_NOTIMPL;
}

HRESULT CSparkWRDSManager::NotifySessionOfServiceStop(WRDS_SESSION_ID *SessionId)
{
    UNREFERENCED_PARAMETER(SessionId);

    DRIVER_ENTER();
    DRIVER_LEAVE();

    return E_NOTIMPL;
}

HRESULT CSparkWRDSManager::NotifySessionStateChange(WRDS_SESSION_ID *SessionId, ULONG EventId)
{
    DRIVER_ENTER();

    TCHAR guidString[512];

    if (SessionId)
    {
        DRIVER_INFO(_T("Session state change: session %d(%s), EventId %d\n"),
            SessionId->SessionId,
            GUIDToString(SessionId->SessionUniqueGuid, guidString),
            EventId);

        if (EventId == WTS_SESSION_REMOTE_CONTROL)
        {
            m_pSessionManager->AddSession(SessionId->SessionId);
        }
        else if (EventId == WTS_SESSION_LOGOFF)
        {
            m_pSessionManager->RemoveSession(SessionId->SessionId);
        }
    }

    DRIVER_LEAVE();
    
    return S_OK;
}

HRESULT CSparkWRDSManager::NotifySettingsChange(PWRDS_SETTINGS pWRdsSettings)
{
    UNREFERENCED_PARAMETER(pWRdsSettings);

    DRIVER_ENTER();
    DRIVER_LEAVE();

    return E_NOTIMPL;
}

