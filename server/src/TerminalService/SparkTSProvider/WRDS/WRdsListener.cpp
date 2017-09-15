#include "stdafx.h"
#include "Log\LogEx.h"
#include "WRdsListener.h"
#include "WRdsConnection.h"
#include "SparkControl.h"
#include "Windows\IPipeClient.h"
#include "Common\IThread.h"
#include "RemoteSessionManager.h"

#define CONNECT_INTERVAL 3000

CSparkWRdsListener::CSparkWRdsListener(CRemoteSessionManager* Manager)
{
    m_lRef = 1;

    //pipe ref add
    AddRef();
    m_pPipeClient = CreateIPipeClientInstance(SPARK_PROVIDER_PIPE_NAME, SPARK_PROVIDER_PIPE_TIMEOUT);
    m_pPipeClient->RegisterRequestHandle(SparkCallbackOnConnected, CSparkWRdsListener::OnConnectHandle);
    m_pPipeClient->RegisterEndHandle(CSparkWRdsListener::DisConnectFromService);
    m_pPipeClient->SetParam(this);

    //thread ref add
    AddRef();
    m_pConnectThread = CreateIThreadInstanceEx(CSparkWRdsListener::ConnectThreadProcess, this, CSparkWRdsListener::ConnectThreadEndProcess, this);
    
    m_pSessionManager = Manager;
    if (m_pSessionManager)
    {
        m_pSessionManager->AddRef();
    }

    DllAddRef();
}

CSparkWRdsListener::~CSparkWRdsListener()
{
    DRIVER_ENTER();

    m_pConnectThread->StopMainThread();
    m_pPipeClient->DisConnect();

    m_pConnectThread->Release();
    m_pPipeClient->Release();

    if (m_pSessionManager)
    {
        m_pSessionManager->AddRef();
    }


    DRIVER_LEAVE();

    DllRelease();
}

HRESULT CSparkWRdsListener::QueryInterface(const IID&  iid, VOID** ppv)
{
    DRIVER_ENTER();

    TCHAR guidString[512];

    if (iid == IID_IUnknown)
    {
        DRIVER_INFO(_T("NepProtocolManager::QueryInterface type IUnknow\r\n"));
        *ppv = this;
    }
    else if (iid == __uuidof(IWRdsProtocolListener))
    {
        DRIVER_INFO(_T("NepProtocolManager::QueryInterface type IWRdsProtocolListener\r\n"));
        *ppv = this;
    }
    else
    {
        //其他一律返回E_NOINTERFACE
        DRIVER_ERROR(_T("NepProtocolManager::QueryInterface %s. Returning E_NOINTERFACE.\r\n"), GUIDToString(iid, guidString));
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    //获取一次则增加引用
    this->AddRef();

    DRIVER_LEAVE();

    return S_OK;
}

ULONG CSparkWRdsListener::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG CSparkWRdsListener::Release()
{
    ULONG Ref = InterlockedDecrement(&m_lRef);

    if (Ref == 0)
    {
        delete this;
        return 0;
    }
    return Ref;
}

HRESULT CSparkWRdsListener::GetSettings(WRDS_LISTENER_SETTING_LEVEL WRdsListenerSettingLevel,
    PWRDS_LISTENER_SETTINGS pWRdsListenerSettings)
{
    UNREFERENCED_PARAMETER(WRdsListenerSettingLevel);
    UNREFERENCED_PARAMETER(pWRdsListenerSettings);

    DRIVER_ENTER();

    return E_NOTIMPL;
}

HRESULT CSparkWRdsListener::StartListen(IWRdsProtocolListenerCallback *pCallback)
{
    DRIVER_ENTER();

    m_pListenerCallBack = pCallback;
    if (m_pListenerCallBack)
    {
        m_pListenerCallBack->AddRef();
    }

    m_pConnectThread->StartMainThread();

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWRdsListener::StopListen()
{
    DRIVER_ENTER();

    m_pConnectThread->StopMainThread();
    m_pPipeClient->DisConnect();

    if (m_pListenerCallBack)
    {
        m_pListenerCallBack->Release();
    }

    DRIVER_LEAVE();
    return S_OK;
}

BOOL CSparkWRdsListener::ConnectThreadProcess(LPVOID param, HANDLE stopevent)
{
    DWORD Ret = WaitForSingleObject(stopevent, CONNECT_INTERVAL);
    CSparkWRdsListener* Listener = (CSparkWRdsListener*)param;

    switch (Ret)
    {
        case WAIT_TIMEOUT:
            if (!Listener->m_pPipeClient->IsConnected())
            {
                DRIVER_INFO(_T("Disconnect, Try to Connect To Service\r\n"));
                if (Listener->m_pPipeClient->Connect())
                {
                    IPacketBuffer* Buffer = CreateIBufferInstance(sizeof(SparkInitListenerRequest));
                    PSparkInitListenerRequest Request = (PSparkInitListenerRequest)Buffer->GetData();

                    memset(Request, 0, sizeof(SparkInitListenerRequest));
                    Request->eType = SPARK_PROVIDER_WTS;

                    Listener->m_pPipeClient->SendRequest(SparkInitListener, Buffer, NULL);
                    DRIVER_INFO(_T("Connect To Service OK\r\n"));

                    Listener->m_pSessionManager->NotifyServiceConnected(Listener->m_pPipeClient);
                }
                else
                {
                    DRIVER_INFO(_T("Connect To Service Fail\r\n"));
                }
            }

            return TRUE;
        default:
            return FALSE;
    }

    return FALSE;
}

VOID CSparkWRdsListener::ConnectThreadEndProcess(LPVOID param)
{
    CSparkWRdsListener* Listener = (CSparkWRdsListener*)param;
    //thread ref release
    Listener->Release();

    //pipe disconnect
    Listener->m_pPipeClient->DisConnect();
    //pipe ref release
    Listener->Release();
}

void CSparkWRdsListener::OnConnectHandle(IPacketBuffer* Buffer, DWORD Id, ICommunication* Param)
{
    IPipeClient* Client = dynamic_cast<IPipeClient*>(Param);
    CSparkWRdsListener* Listener = (CSparkWRdsListener*)Client->GetParam();
    PSparkCallbackOnConnectedRequest Request = (PSparkCallbackOnConnectedRequest)Buffer->GetData();

    UNREFERENCED_PARAMETER(Id);
    
    DRIVER_ENTER();
    DRIVER_INFO(_T("NepService Notify OnConnect\r\n"));

    if (!Listener->m_pListenerCallBack)
    {
        DRIVER_ERROR(_T("Listener do not has CallBack\r\n"));
        return;
    }

    CSparkWRdsConnection *Connection = new CSparkWRdsConnection(Request->stConnectionInfo.dwConnectionID,
        Listener->m_pSessionManager, Listener->m_pListenerCallBack);

    Connection->StartConnect();

    Connection->Release();

    DRIVER_LEAVE();

    return;
}

void CSparkWRdsListener::DisConnectFromService(ICommunication* Param)
{
    IPipeClient* Client = dynamic_cast<IPipeClient*>(Param);
    CSparkWRdsListener* Listener = (CSparkWRdsListener*)Client->GetParam();
    Listener->mSessionManager->NotifyServiceDisconnect();
}
