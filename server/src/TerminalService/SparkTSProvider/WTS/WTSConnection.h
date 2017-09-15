#pragma once

#ifndef __WTS_CONNECTION_H__
#define __WTS_CONNECTION_H__

#include <Windows.h>
#include <wtsprotocol.h>
#include "Windows\IPipeClient.h"
#include "Common\IThread.h"
#include "IWTSConnectionRdp.h"
#include "SparkControl.h"

class CRemoteSessionManager;

class CSparkWTSConnection: public IWTSProtocolConnection, IWTSConnectionRdp
{
public:
    CSparkWTSConnection(DWORD ConnectionID, CRemoteSessionManager* Manager, IWTSProtocolListenerCallback* ListenerCallback);

    ~CSparkWTSConnection();

    virtual HRESULT WINAPI QueryInterface(const IID& riid, VOID** ppvObj);

    virtual ULONG WINAPI AddRef();

    virtual ULONG WINAPI Release();

public:
    virtual HRESULT WINAPI GetLogonErrorRedirector(IWTSProtocolLogonErrorRedirector **ppLogonErrorRedir);

    virtual HRESULT WINAPI SendPolicyData(WTS_POLICY_DATA *pPolicyData);

    virtual HRESULT WINAPI AcceptConnection(void);

    virtual HRESULT WINAPI GetClientData(WTS_CLIENT_DATA *pClientData);

    virtual HRESULT WINAPI GetUserCredentials(WTS_USER_CREDENTIAL *pUserCreds);

    virtual HRESULT WINAPI GetLicenseConnection(IWTSProtocolLicenseConnection **ppLicenseConnection);

    virtual HRESULT WINAPI AuthenticateClientToSession(WTS_SESSION_ID *SessionId);

    virtual HRESULT WINAPI NotifySessionId(WTS_SESSION_ID *SessionId);

    virtual HRESULT WINAPI GetProtocolHandles(HANDLE_PTR *pKeyboardHandle,
                                      HANDLE_PTR *pMouseHandle,
                                      HANDLE_PTR *pBeepHandle,
                                      HANDLE_PTR *pVideoHandle);

    virtual HRESULT WINAPI ConnectNotify(ULONG SessionId);

    virtual HRESULT WINAPI IsUserAllowedToLogon(ULONG SessionId,
                                        HANDLE_PTR UserToken,
                                        WCHAR *pDomainName,
                                        WCHAR *pUserName);

    virtual HRESULT WINAPI SessionArbitrationEnumeration(HANDLE_PTR hUserToken,
                                                 BOOL bSingleSessionPerUserEnabled,
                                                 ULONG *pSessionIdArray,
                                                 ULONG *pdwSessionIdentifierCount);

    virtual HRESULT WINAPI LogonNotify(HANDLE_PTR hClientToken, WCHAR *wszUserName, WCHAR *wszDomainName,
                               WTS_SESSION_ID *SessionId);

    virtual HRESULT WINAPI GetUserData(WTS_POLICY_DATA *pPolicyData, WTS_USER_DATA *pClientData);

    virtual HRESULT WINAPI DisconnectNotify(void);

    virtual HRESULT WINAPI Close(void);

    virtual HRESULT WINAPI GetProtocolStatus(WTS_PROTOCOL_STATUS *pProtocolStatus);

    virtual HRESULT WINAPI GetLastInputTime(ULONG64 *pLastInputTime);

    virtual HRESULT WINAPI SetErrorInfo(ULONG ulError);

    virtual HRESULT WINAPI SendBeep(ULONG Frequency, ULONG Duration);

    virtual HRESULT WINAPI CreateVirtualChannel(CHAR *szEndpointName,
                                        BOOL bStatic,
                                        ULONG RequestedPriority,
                                        ULONG_PTR *phChannel);

    virtual HRESULT WINAPI QueryProperty(GUID QueryType, ULONG ulNumEntriesIn,
                                 ULONG ulNumEntriesOut, PWTS_PROPERTY_VALUE pPropertyEntriesIn,
                                 PWTS_PROPERTY_VALUE pPropertyEntriesOut);

    virtual HRESULT WINAPI GetShadowConnection(IWTSProtocolShadowConnection **ppShadowConnection);

    virtual HRESULT WINAPI PrepareForAccept(void *a1, int a2, int a3, int a4);
    virtual HRESULT WINAPI GetClientMonitorData(UINT *pNumMonitors, UINT *pPrimaryMonitor);
    virtual HRESULT WINAPI GetSecurityFilterCreds(int a1, int a2);
    virtual HRESULT WINAPI GetSecurityFilterClientCerts(int a1, int a2);
    virtual HRESULT WINAPI GetSecurityFilterClientToken(int a1);
    virtual HRESULT WINAPI SendLogonErrorInfoToClient(int a1, int a2);

    void StartConnect();

private:
    static BOOL OnConnectThreadProcess(LPVOID param, HANDLE stopevent);

    static void OnReadyHandle(IPacketBuffer* Buffer, DWORD Id, ICommunication* Param);

    static void OnBrokenConnectionHandle(IPacketBuffer* Buffer, DWORD Id, ICommunication* Param);

    static void OnReleaseHandle(IPacketBuffer* Buffer, DWORD Id, ICommunication* Param);

    static void DisConnectFromService(ICommunication* Param);

    static void OnRedrawRect(IPacketBuffer* Buffer, DWORD Id, ICommunication* Param);

    LONG                             m_lRef;                   /** < 引用计数                         */
    IPipeClient*                     m_pPipeClient;            /** < 管道客户端                       */
    DWORD                            m_dwConnectionID;         /** < 连接ID                           */
    IWTSProtocolConnectionCallback*  m_pCallBack;              /** < Teriminal Service 提供的回调函数 */
    IThread*                         m_pOnConnectThread;       /** < 连接线程                         */
    HANDLE                           m_hOnConnectThreadStoped; /** < 线程是否结束                     */
    HANDLE                           m_hServiceInitOK;         /** < 是否成功                         */
    HANDLE                           m_hWaitForOnReady;        /** < 是否开始等待OnRead事件           */
    IWTSProtocolListenerCallback*    m_pListenerCallBack;      /** < 监听器回调，只用一次             */
    BOOL                             m_bAccept;                /** < 是否已经accept连接               */
    DWORD                            m_dwSessionID;            /** < 会话ID                           */
    CRemoteSessionManager*           m_pSessionManager;        /** < 会话管理器                       */
    SparkClientInfo                  m_stClientInfo;           /** < 客户端发送过来的所有消息         */
};


#endif
