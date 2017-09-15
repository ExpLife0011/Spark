#include "stdafx.h"
#include "Log\LogEx.h"
#include "WTS\WTSManager.h"
#include "WTS\WTSListener.h"
#include "SessionHelper\SessionHelp.h"
#include "RemoteSessionManager.h"

CSparkWTSManager::CSparkWTSManager()
{
    m_lRef = 1;
    m_pSessionManager = new CRemoteSessionManager();
    DllAddRef();
}

CSparkWTSManager::~CSparkWTSManager()
{
    CSessionHelp::GetInstance()->Uninstall();
    m_pSessionManager->Release();
    DllRelease();
}

HRESULT CSparkWTSManager::QueryInterface(const IID&  iid, VOID** ppv)
{
    DRIVER_ENTER();

    TCHAR guidString[512];

    if (iid == IID_IUnknown)
    {
        DRIVER_INFO(_T("CSparkWTSManager::QueryInterface type IUnknow\r\n"));
        *ppv = this;
    }
    else if (iid == __uuidof(IWTSProtocolManager))
    {
        DRIVER_INFO(_T("CSparkWTSManager::QueryInterface type IWTSProtocolManager\r\n"));
        *ppv = this;
    }
    else
    {
        DRIVER_ERROR(_T("CSparkWTSManager::QueryInterface %s. Returning E_NOINTERFACE.\r\n"), GUIDToString(iid, guidString));
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();

    DRIVER_LEAVE();

    return S_OK;
}

ULONG CSparkWTSManager::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG CSparkWTSManager::Release()
{
    ULONG Ref = InterlockedDecrement(&m_lRef);

    if (Ref == 0)
    {
        delete this;
        return 0;
    }
    return Ref;
}


HRESULT CSparkWTSManager::CreateListener(WCHAR *wszListenerName, IWTSProtocolListener **pProtocolListener)
{
    HRESULT Result = S_OK;

    DRIVER_ENTER();
    
    DRIVER_INFO(_T("Listener Name %s\n"), wszListenerName ? wszListenerName : _T("NULL"));
	
    CSparkWTSListener* Listener = new CSparkWTSListener(m_pSessionManager);
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

HRESULT CSparkWTSManager::NotifyServiceStateChange(WTS_SERVICE_STATE *pTSServiceStateChange)
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

HRESULT CSparkWTSManager::NotifySessionOfServiceStart(WTS_SESSION_ID *SessionId)
{
    UNREFERENCED_PARAMETER(SessionId);

    DRIVER_ENTER();
    DRIVER_LEAVE();

    return E_NOTIMPL;
}

HRESULT CSparkWTSManager::NotifySessionOfServiceStop(WTS_SESSION_ID *SessionId)
{
    UNREFERENCED_PARAMETER(SessionId);

    DRIVER_ENTER();
	
    DRIVER_LEAVE();

    return E_NOTIMPL;
}

HRESULT CSparkWTSManager::NotifySessionStateChange(WTS_SESSION_ID *SessionId, ULONG EventId)
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
