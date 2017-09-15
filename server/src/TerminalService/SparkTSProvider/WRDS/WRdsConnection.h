/**
 * @file     WRdsConnection.h
 * @author   wangxu.st@centerm.com
 * @date     2016/1/22
 * @version  1.0
 * @brief    NEP协议连接器头文件
 */
#pragma once

#ifndef __WRDS_CONNECTION_H__
#define __WRDS_CONNECTION_H__

#include <Windows.h>
#include <wtsprotocol.h>
#include "Windows\IPipeClient.h"

class CRemoteSessionManager;


/**
 * @class CSparkWRdsConnection
 * @brief NEP协议监听器，COM组件
 *
 *
 * 用于CSparkWRdsConnection，由系统Terimin Service加载，并执行
 * 相关说明见：https://msdn.microsoft.com/en-us/library/hh707194(v=vs.85).aspx
 * 继承自IWRdsProtocolConnection接口，由于IWRdsProtocolConnection同时继续自IUnknow接口，所以需要实现大量函数
 * 各接口关系，与加载顺序见：https://msdn.microsoft.com/en-us/library/dd920049(v=vs.85).aspx
 */
class CSparkWRdsConnection : public IWRdsProtocolConnection, public IWRdsRemoteFXGraphicsConnection
{
public:
    /**
     * @brief 构造函数
     */
    CSparkWRdsConnection(DWORD ConnectionID, CRemoteSessionManager* Manager);

    /**
     * @brief 析构函数
     */
    ~CSparkWRdsConnection();

    /** < IUnknown类的接口 */
    /**
     * @brief 根据具体类型，来获取实际对象
     *        对于类型为IUnknow类与IWRdsProtocolManager类，可返回自己本身，他类则不支持
     * 定义见：https://msdn.microsoft.com/en-us/library/windows/desktop/ms682521(v=vs.85).aspx
     *
     * @param[in]  rrid   获取的类型
     * @param[out] ppvObj 返回的对象
     *
     * @return 返回S_OK表示获取成功，其他则表示失败
     */
    virtual HRESULT WINAPI QueryInterface(const IID& riid, VOID** ppvObj);

    /**
     * @brief 增加引用
     * 定义见：https://msdn.microsoft.com/en-us/library/windows/desktop/ms691379(v=vs.85).aspx
     *
     * @return 增加后的引用
     */
    virtual ULONG WINAPI AddRef();

    /**
     * @brief 减少引用，减少到0，自动析构
     * 定义见：https://msdn.microsoft.com/en-us/library/windows/desktop/ms682317(v=vs.85).aspx
     *
     * @return 增加后的引用
     */
    virtual ULONG WINAPI Release();

    virtual HRESULT WINAPI EnableRemoteFXGraphics(BOOL *pEnableRemoteFXGraphics);
        
    virtual HRESULT WINAPI GetVirtualChannelTransport(IUnknown **ppTransport);

    /** < IWRdsProtocolConnection类的接口 */
    /** 
     *  下列的接口，是新的会话连接的时候调用，顺序是函数声明顺序
     *  具体见：https://msdn.microsoft.com/en-us/library/dd919945(v=vs.85).aspx
     */
    /**
     * @brief 获取登陆错误的消息处理接口，暂时不实现，返回E_NOTIMPL
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707211(v=vs.85).aspx
     *
     * @param[out] ppLogonErrorRedir 登陆错误的消息处理接口
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI GetLogonErrorRedirector(IWRdsProtocolLogonErrorRedirector **ppLogonErrorRedir);

    /**
     * @brief 允许连接接入
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707201(v=vs.85).aspx
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI AcceptConnection();
        
    /**
     * @brief 获取客户端数据（连接配置）
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707207(v=vs.85).aspx
     *
     * @param[out] pClientData 获取到的客户端数据
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI GetClientData(WRDS_CLIENT_DATA *pClientData);

    /**
     * @brief 获取客户端显示器个数,与主显示器ID
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707207(v=vs.85).aspx
     *
     * @param[out] pNumMonitors    显示器个数
     * @param[out] pPrimaryMonitor 主显示器ID
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */ 
    virtual HRESULT WINAPI GetClientMonitorData(UINT *pNumMonitors, UINT *pPrimaryMonitor);
        
    /**
     * @brief 获取用户认证信息，用于用户登录，只支持用户名密码登陆，不实现，直接返回E_NOTIMPL，由WinLogon的Provider来实现以支持票据登陆
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707214(v=vs.85).aspx
     *
     * @param[out] pUserCreds 用户认证信息
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI GetUserCredentials(WRDS_USER_CREDENTIAL *pUserCreds);
        
    /**
     * @brief 获取授权信息接口
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707210(v=vs.85).aspx
     *
     * @param[out] ppLicenseConnection授权接口
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI GetLicenseConnection(IWRdsProtocolLicenseConnection **ppLicenseConnection);
       
    /**
     * @brief 适用与客户端和会话绑定的场景，获取该客户端应该认证的会话，不实现，直接返回E_NOTIMPL
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707202(v=vs.85).aspx
     *
     * @param[out] SessionId 会话信息
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI AuthenticateClientToSession(WRDS_SESSION_ID *SessionId);
        
    /**
     * @brief 通知会话创建成功
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707218(v=vs.85).aspx
     *
     * @param[in] SessionId     会话ID
     * @param[in] SessionHandle 会话句柄
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI NotifySessionId(WRDS_SESSION_ID *SessionId, HANDLE_PTR SessionHandle);

    /**
     * @brief 获取输入设备句柄
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707208(v=vs.85).aspx
     *
     * @param[out] pKeyboardHandle 键盘句柄
     * @param[out] pMouseHandle    鼠标句柄
     * @param[out] pBeepHandle     蜂鸣器句柄
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */ 
    virtual HRESULT WINAPI GetInputHandles(HANDLE_PTR *pKeyboardHandle, HANDLE_PTR *pMouseHandle, HANDLE_PTR *pBeepHandle);
        
    /**
     * @brief 获取输出设备句柄
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707215(v=vs.85).aspx
     *
     * @param[out] pVideoHandle 显卡小端口驱动句柄
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */ 
    virtual HRESULT WINAPI GetVideoHandle(HANDLE_PTR *pVideoHandle);
        
    /**
     * @brief 通知会话初始化成功
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707204(v=vs.85).aspx
     *
     * @param[out] SessionId 会话ID
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */ 
    virtual HRESULT WINAPI ConnectNotify(ULONG SessionId);

    /**
     * @brief 通知WinLogin已经创建于初始化成功
     * 定义见：https://msdn.microsoft.com/en-us/library/jj553607(v=vs.85).aspx
     *
     * @param[out] SessionId 会话ID
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */                 
    virtual HRESULT WINAPI NotifyCommandProcessCreated(ULONG SessionId);
    
    /**
     * @brief 决定是否可以让这个用户登录
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707216(v=vs.85).aspx
     *
     * @param[in] SessionId   会话ID
     * @param[in] UserToken   用户Token
     * @param[in] pDomainName 域名
     * @param[in] pUserName   用户名
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */ 
    virtual HRESULT WINAPI IsUserAllowedToLogon(ULONG SessionId, HANDLE_PTR UserToken, WCHAR *pDomainName, WCHAR *pUserName);
        
    /**
     * @brief 这个不知道干嘛的，暂时不实现，返回E_NOTIMPL
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707220(v=vs.85).aspx
     *
     * @param[in] hUserToken                 
     * @param[in] bSingleSessionPerUserEnabled
     * @param[in] pSessionIdArray
     * @param[in] pdwSessionIdentifierCount
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */ 
    virtual HRESULT WINAPI SessionArbitrationEnumeration(HANDLE_PTR hUserToken, 
        BOOL bSingleSessionPerUserEnabled, ULONG *pSessionIdArray, ULONG *pdwSessionIdentifierCount);
        
    /**
     * @brief 通知已经登陆
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707220(v=vs.85).aspx
     *
     * @param[in] hClientToken  用户TOKEN               
     * @param[in] wszUserName   用户名
     * @param[in] wszDomainName 域名
     * @param[in] SessionId     会话ID
     * @param[inout] pWRdsConnectionSettings 连接配置
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */ 
    virtual HRESULT WINAPI LogonNotify( 
            HANDLE_PTR hClientToken,
            WCHAR *wszUserName,
            WCHAR *wszDomainName,
            WRDS_SESSION_ID *SessionId,
            PWRDS_CONNECTION_SETTINGS pWRdsConnectionSettings);

    /** 
     *  下列的接口，是会话断开的时候调用，顺序是函数声明顺序
     */       
    /**
     * @brief 连接准备断开
     * 定义见：https://msdn.microsoft.com/en-us/library/hh972697(v=vs.85).aspx
     *
     * @param[in] DisconnectReason 断开原因
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI PreDisconnect(ULONG DisconnectReason);
 
    /**
     * @brief 通知已经断开    
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707206(v=vs.85).aspx
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI DisconnectNotify();
  
    /**
     * @brief 关闭连接，用于清理
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707203(v=vs.85).aspx
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI Close();

    /** 
     *  下列的接口，随时都有可能调用
     */  
    /**
     * @brief 获取连接状态，包括收发统计什么的，暂时不实现，返回E_NOTIMPL
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707212(v=vs.85).aspx
     *
     * @param[out] pProtocolStatus 协议状态
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI GetProtocolStatus(WRDS_PROTOCOL_STATUS *pProtocolStatus);
    
    /**
     * @brief 获取最后一次操作距离限制多久，可能和自动锁屏有关，可以由客户端来实现，这里暂时不实现，返回E_NOTIMPL
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707209(v=vs.85).aspx
     *
     * @param[out] pLastInputTime 举例现在的时间，单位ms
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI GetLastInputTime(ULONG64 *pLastInputTime);
    
    /**
     * @brief 设置错误代码，暂时不实现，返回E_NOTIMPL
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707221(v=vs.85).aspx
     *
     * @param[out] ulError 错误代码
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI SetErrorInfo(ULONG ulError);
    
    /**
     * @brief 创建虚拟通道，NEP协议的通道由另外的方案实现，这里不实现，返回E_NOTIMPL
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707205(v=vs.85).aspx
     *
     * @param[in] szEndpointName    通道名字
     * @param[in] bStatic           是否静态通道
     * @param[in] RequestedPriority 优先级
     * @param[out] phChannel        通道指针
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI CreateVirtualChannel(CHAR *szEndpointName, BOOL bStatic, ULONG RequestedPriority, ULONG_PTR *phChannel);
        
    /**
     * @brief 获取协议属性，暂时不实现，返回E_NOTIMPL
     * 定义见：    https://msdn.microsoft.com/en-us/library/hh707219(v=vs.85).aspx
     *
     * @param[in] QueryType                获取的属性类型
     * @param[in] ulNumEntriesIn           入参大小
     * @param[in] ulNumEntriesOut          出参大小
     * @param[in] pPropertyEntriesIn      入参
     * @param[out] pPropertyEntriesIn      出参
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI QueryProperty(GUID QueryType, ULONG ulNumEntriesIn, ULONG ulNumEntriesOut, 
        PWRDS_PROPERTY_VALUE pPropertyEntriesIn, PWRDS_PROPERTY_VALUE pPropertyEntriesOut);
        
    /**
     * @brief 获取影子连接（什么鬼）,暂时不实现，返回E_NOTIMPL
     * 定义见：https://msdn.microsoft.com/en-us/library/hh707213(v=vs.85).aspx
     *
     * @param[in] ppShadowConnection 影子连接，好中二
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */

    virtual HRESULT WINAPI GetShadowConnection(IWRdsProtocolShadowConnection **ppShadowConnection);

    /**
     * @brief 设置回调
     *
     * @param[in] CallBack 回调接口
     */
    void SetCallBack(IWRdsProtocolConnectionCallback* CallBack);

    /**
     * @brief 连接线程
     *
     * @param[in] OnConnectThread 连接线程
     */
    void SetOnConnectThread(IThread* OnConnectThread, IWRdsProtocolListenerCallback* ListenrCallBack);

    /**
     * @brief ONCONNECT线程处理函数
     *
     * @param[in] param     参数，Listener对象
     * @param[in] stopevent 停止参数
     *
     * @return 是否继续处理
     */
    static BOOL OnConnectThreadProcess(LPVOID param, HANDLE stopevent);

private:
    /**
     * @brief 就绪通告处理函数
     *
     * @param[in] Buffer    数据包
     * @param[in] BufferLen 数据包长度
     * @param[in] Id        数据包ID
     * @param[in] Param     接收到的ICommunictian对象
     *
     * @return 无
     */
    static void OnReadyHandle(BYTE* Buffer, DWORD BufferLen, DWORD Id, ICommunication* Param);

    /**
     * @brief 断开通告处理函数
     *
     * @param[in] Buffer    数据包
     * @param[in] BufferLen 数据包长度
     * @param[in] Id        数据包ID
     * @param[in] Param     接收到的ICommunictian对象
     *
     * @return 无
     */
    static void OnBrokenConnectionHandle(BYTE* Buffer, DWORD BufferLen, DWORD Id, ICommunication* Param);

    /**
     * @brief 释放通告处理函数
     *
     * @param[in] Buffer    数据包
     * @param[in] BufferLen 数据包长度
     * @param[in] Id        数据包ID
     * @param[in] Param     接收到的ICommunictian对象
     *
     * @return 无
     */
    static void OnReleaseHandle(BYTE* Buffer, DWORD BufferLen, DWORD Id, ICommunication* Param);

    /**
     * @brief 管道端口处理函数
     *
     * @param[in] Param 管道
     */
    static void DisConnectFromService(ICommunication* Param);

    LONG                             m_lRef;                   /** < 引用计数                         */
    IPipeClient*                     mPipeClient;            /** < 管道客户端                       */
    DWORD                            mConnectionID;          /** < 连接ID                           */
    IWRdsProtocolConnectionCallback* mCallBack;              /** < Teriminal Service 提供的回调函数 */
    IThread*                         mOnConnectThread;       /** < 连接线程                         */
    HANDLE                           mOnConnectThreadStoped; /** < 线程是否结束                     */
    HANDLE                           mServiceInitOK;         /** < 是否成功                         */
    IWRdsProtocolListenerCallback*   mListenrCallBack;       /** < 监听器回调，只用一次             */
    BOOL                             mAccpept; 
	DWORD                            mSessionID;             /** < 会话ID                           */
    WCHAR                            mDomainName[MAX_PATH];  /** < 域名                             */
    WCHAR                            mUserName[MAX_PATH];    /** < 用户名                           */
    CRemoteSessionManager*           mSessionManager;        /** < 会话管理器                       */
    BOOL                             mbEnableAudio;          /** < 是否启用声音                     */
    BOOL                             mRemoteAppConnection;   /** < 远程应用会话                     */
};

#endif