#pragma once

#ifndef __WTS_LISTENER_H__
#define __WTS_LISTENER_H__

#include <wtsprotocol.h>

class IPacketBuffer;
class IThread;
class IPipeClient;
class ICommunication;
class CRemoteSessionManager;

class CSparkWTSListener: public IWTSProtocolListener
{
public:
    CSparkWTSListener(CRemoteSessionManager* SessionManager);

    ~CSparkWTSListener();

    virtual HRESULT WINAPI QueryInterface(const IID& riid, VOID** ppvObj);

    virtual ULONG WINAPI AddRef();

    virtual ULONG WINAPI Release();

    virtual HRESULT WINAPI StartListen(IWTSProtocolListenerCallback *pCallback);

    virtual HRESULT WINAPI StopListen();

private:
    static BOOL ConnectThreadProcess(LPVOID param, HANDLE stopevent);

    static VOID ConnectThreadEndProcess(LPVOID param);

    static void OnConnectHandle(IPacketBuffer* Buffer, DWORD Id, ICommunication* Param);

    LONG                            m_lRef;
    IThread*                        m_pConnectThread;
    IWTSProtocolListenerCallback*   m_pListenerCallBack;
    CRemoteSessionManager*          m_pSessionManager;
    IPipeClient*                    m_pPipeClient;
};


#endif
