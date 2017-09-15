#include "stdafx.h"
#include "Log\LogEx.h"
#include "WTS\WTSListener.h"
#include "WTS\WTSConnection.h"
#include "SparkControl.h"
#include "Windows\IPipeClient.h"
#include "Common\IThread.h"
#include "RemoteSessionManager.h"

#define CONNECT_INTERVAL 3000

CSparkWTSListener::CSparkWTSListener(CRemoteSessionManager* Manager)
{
    m_lRef = 1;

    m_pPipeClient = CreateIPipeClientInstance(SPARK_PROVIDER_PIPE_NAME, SPARK_PROVIDER_PIPE_TIMEOUT);

    //thread ref add
    AddRef();
    m_pConnectThread = CreateIThreadInstanceEx(CSparkWTSListener::ConnectThreadProcess, this, CSparkWTSListener::ConnectThreadEndProcess, this);

    m_pSessionManager = Manager;
    if (m_pSessionManager)
    {
        m_pSessionManager->AddRef();
    }

    DllAddRef();
}

CSparkWTSListener::~CSparkWTSListener()
{
    DRIVER_ENTER();

    StopListen();

    if (m_pConnectThread)
    {
        m_pConnectThread->Release();
        m_pConnectThread = NULL;
    }

    if (m_pPipeClient)
    {
        m_pPipeClient->Release();
        m_pPipeClient = NULL;
    }

    if (m_pSessionManager)
    {
        m_pSessionManager->Release();
    }

    DRIVER_LEAVE();

    DllRelease();
}

HRESULT CSparkWTSListener::QueryInterface(const IID&  iid, VOID** ppv)
{
    DRIVER_ENTER();

    TCHAR guidString[512];

    if (iid == IID_IUnknown)
    {
        DRIVER_INFO(_T("CSparkWTSListener::QueryInterface type IUnknow\r\n"));
        *ppv = this;
    }
    else if (iid == __uuidof(IWRdsProtocolListener))
    {
        DRIVER_INFO(_T("CSparkWTSListener::QueryInterface type IWRdsProtocolListener\r\n"));
        *ppv = this;
    }
    else
    {
        DRIVER_ERROR(_T("CSparkWTSListener::QueryInterface %s. Returning E_NOINTERFACE.\r\n"), GUIDToString(iid, guidString));
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();

    DRIVER_LEAVE();

    return S_OK;
}

ULONG CSparkWTSListener::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG CSparkWTSListener::Release()
{
    ULONG Ref = InterlockedDecrement(&m_lRef);

    if (Ref == 0)
    {
        delete this;
        return 0;
    }
    return Ref;
}

HRESULT CSparkWTSListener::StartListen(IWTSProtocolListenerCallback *pCallback)
{
    DRIVER_ENTER();

    m_pPipeClient->RegisterRequestHandle(SparkInitListener, CSparkWTSListener::OnConnectHandle);
    AddRef();
    m_pPipeClient->SetParam(this);

    m_pListenerCallBack = pCallback;
    if (m_pListenerCallBack)
    {
        m_pListenerCallBack->AddRef();
    }

    m_pConnectThread->StartMainThread();

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWTSListener::StopListen()
{
    DRIVER_ENTER();

    m_pPipeClient->SetParam(NULL);
    Release();
    m_pPipeClient->DisConnect();

    m_pConnectThread->StopMainThread();

    if (m_pListenerCallBack)
    {
        m_pListenerCallBack->Release();
        m_pListenerCallBack = NULL;
    }

    DRIVER_LEAVE();
    return S_OK;
}

BOOL CSparkWTSListener::ConnectThreadProcess(LPVOID param, HANDLE stopevent)
{
    DWORD Ret = WaitForSingleObject(stopevent, CONNECT_INTERVAL);
    CSparkWTSListener* Listener = (CSparkWTSListener*)param;

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

                    Listener->m_pSessionManager->NotifyServiceConnected();
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
}

VOID CSparkWTSListener::ConnectThreadEndProcess(LPVOID param)
{
    CSparkWTSListener* Listener = (CSparkWTSListener*)param;
    Listener->Release();

    Listener->m_pPipeClient->DisConnect();
}

void CSparkWTSListener::OnConnectHandle(IPacketBuffer* Buffer, DWORD Id, ICommunication* Param)
{
    IPipeClient* Client = dynamic_cast<IPipeClient*>(Param);
    CSparkWTSListener* Listener = (CSparkWTSListener*)Client->GetParam();
    PSparkCallbackOnConnectedRequest Request = (PSparkCallbackOnConnectedRequest)Buffer->GetData();

    UNREFERENCED_PARAMETER(Id);
    
    DRIVER_ENTER();
    DRIVER_INFO(_T("SparkService Notify OnConnect ID %d\r\n"), Request->stConnectionInfo.dwConnectionID);

    if (Listener == NULL)
    {
        DRIVER_ERROR(_T("Listener is null\r\n"));
        return;
    }

    if (!Listener->m_pListenerCallBack)
    {
        DRIVER_ERROR(_T("Listener do not has CallBack\r\n"));
        return;
    }

    CSparkWTSConnection *Connection = new CSparkWTSConnection(Request->stConnectionInfo.dwConnectionID,
        Listener->m_pSessionManager, Listener->m_pListenerCallBack);

    Connection->StartConnect();

    Connection->Release();

    DRIVER_LEAVE();

    return;
}