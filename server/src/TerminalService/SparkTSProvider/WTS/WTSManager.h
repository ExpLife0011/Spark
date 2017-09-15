#pragma once

#ifndef __WTS_MANAGER_H__
#define __WTS_MANAGER_H__

#include <wtsprotocol.h>

class CRemoteSessionManager;

class CSparkWTSManager: public IWTSProtocolManager
{
public:
    CSparkWTSManager();

    ~CSparkWTSManager();

    virtual HRESULT WINAPI QueryInterface(const IID& riid, VOID** ppvObj);

    virtual ULONG WINAPI AddRef();

    virtual ULONG WINAPI Release();

    virtual HRESULT WINAPI CreateListener(WCHAR *wszListenerName, IWTSProtocolListener **pProtocolListener);

	virtual HRESULT WINAPI NotifyServiceStateChange(WTS_SERVICE_STATE *pTSServiceStateChange);

    virtual HRESULT WINAPI NotifySessionStateChange(WTS_SESSION_ID *SessionId, ULONG EventId);

	virtual HRESULT WINAPI NotifySessionOfServiceStart(WTS_SESSION_ID *SessionId);

	virtual HRESULT WINAPI NotifySessionOfServiceStop(WTS_SESSION_ID *SessionId);

private:
    LONG                   m_lRef;
    CRemoteSessionManager* m_pSessionManager;
};

#endif
