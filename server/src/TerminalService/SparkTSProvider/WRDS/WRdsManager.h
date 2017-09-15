#pragma once

#ifndef __WRDS_MANAGER_H__
#define __WRDS_MANAGER_H__

#include <Windows.h>
#include <wtsprotocol.h>

class CRemoteSessionManager;

class CSparkWRDSManager : public IWRdsProtocolManager
{
public:
    CSparkWRDSManager();

    ~CSparkWRDSManager();

    virtual HRESULT WINAPI QueryInterface(const IID& riid, VOID** ppvObj);

    virtual ULONG WINAPI AddRef();

    virtual ULONG WINAPI Release();

	virtual HRESULT WINAPI Initialize(IWRdsProtocolSettings *pIWRdsSettings, PWRDS_SETTINGS pWRdsSettings);

    virtual HRESULT WINAPI Uninitialize(void);

	virtual HRESULT WINAPI CreateListener(WCHAR *wszListenerName, IWRdsProtocolListener **pProtocolListener);

	virtual HRESULT WINAPI NotifyServiceStateChange(WRDS_SERVICE_STATE *pTSServiceStateChange);

    virtual HRESULT WINAPI NotifySessionStateChange(WRDS_SESSION_ID *SessionId, ULONG EventId);

    virtual HRESULT WINAPI NotifySettingsChange(PWRDS_SETTINGS pWRdsSettings);

	virtual HRESULT WINAPI NotifySessionOfServiceStart(WRDS_SESSION_ID *SessionId);

	virtual HRESULT WINAPI NotifySessionOfServiceStop(WRDS_SESSION_ID *SessionId);

private:
    LONG                m_lRef;
    CRemoteSessionManager* m_pSessionManager;
};

#endif