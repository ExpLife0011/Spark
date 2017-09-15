#pragma once

#ifndef __WRDS_LISTENER_H__
#define __WRDS_LISTENER_H__

#include <Windows.h>
#include <wtsprotocol.h>

class IPacketBuffer;
class IThread;
class IPipeClient;
class ICommunication;
class CRemoteSessionManager;

class CSparkWRdsListener : public IWRdsProtocolListener
{
public:
    CSparkWRdsListener(CRemoteSessionManager* Manager);

    ~CSparkWRdsListener();

    virtual HRESULT WINAPI QueryInterface(const IID& riid, VOID** ppvObj);

    virtual ULONG WINAPI AddRef();

    virtual ULONG WINAPI Release();

    virtual HRESULT WINAPI GetSettings(WRDS_LISTENER_SETTING_LEVEL WRdsListenerSettingLevel,
        PWRDS_LISTENER_SETTINGS pWRdsListenerSettings);

    virtual HRESULT WINAPI StartListen(IWRdsProtocolListenerCallback *pCallback);

    virtual HRESULT WINAPI StopListen();

private:
    static BOOL ConnectThreadProcess(LPVOID param, HANDLE stopevent);

    static VOID ConnectThreadEndProcess(LPVOID param);

    static void OnConnectHandle(IPacketBuffer* Buffer, DWORD Id, ICommunication* Param);

    static void DisConnectFromService(ICommunication* Param);

    LONG                            m_lRef;
    IPipeClient*                    m_pPipeClient;
    IThread*                        m_pConnectThread;
    IWRdsProtocolListenerCallback*  m_pListenerCallBack;
    CRemoteSessionManager*          m_pSessionManager;
};

#endif