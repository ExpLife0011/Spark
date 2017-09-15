/**
* @file     WRdsLicenseConnection.cpp
* @author   wangxu.st@centerm.com
* @date     2016/1/27
* @version  1.0
* @brief    NEP协议授权连接器源文件
*/
#include "stdafx.h"
#include "Log\LogEx.h"
#include "WRdsLicenseConnection.h"

CNepWRdsLicenseConnection::CNepWRdsLicenseConnection()
{
    m_lRef = 1;
    DllAddRef();
}

CNepWRdsLicenseConnection::~CNepWRdsLicenseConnection()
{
    DRIVER_ENTER();

    DllRelease();

    DRIVER_LEAVE();
}

HRESULT CNepWRdsLicenseConnection::QueryInterface(const IID&  iid, VOID** ppv)
{
    DRIVER_ENTER();

    TCHAR guidString[512];

    //类型为IUnknow和IWRdsProtocolManager才返回自己
    if (iid == IID_IUnknown)
    {
        DRIVER_INFO(_T("CNepWRdsLicenseConnection::QueryInterface type IUnknow\r\n"));
        *ppv = this;
    }
    else if (iid == __uuidof(IWRdsProtocolLicenseConnection))
    {
        DRIVER_INFO(_T("CNepWRdsLicenseConnection::QueryInterface type IWRdsProtocolListener\r\n"));
        *ppv = this;
    }
    else
    {
        //其他一律返回E_NOINTERFACE
        DRIVER_ERROR(_T("CNepWRdsLicenseConnection::QueryInterface %s. Returning E_NOINTERFACE.\r\n"), GUIDToString(iid, guidString));
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    //获取一次则增加引用
    this->AddRef();

    DRIVER_LEAVE();

    return S_OK;
}

ULONG CNepWRdsLicenseConnection::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG CNepWRdsLicenseConnection::Release()
{
    ULONG Ref = InterlockedDecrement(&m_lRef);

    if (Ref == 0)
    {
        delete this;
        return 0;
    }
    return Ref;
}

HRESULT CNepWRdsLicenseConnection::RequestLicensingCapabilities(PWRDS_LICENSE_CAPABILITIES ppLicenseCapabilities, 
                                                         ULONG *pcbLicenseCapabilities)
{
    CHAR *clientName = "Xred test";

    DRIVER_ENTER();

    ppLicenseCapabilities->KeyExchangeAlg = WTS_KEY_EXCHANGE_ALG_RSA;
    ppLicenseCapabilities->ProtocolVer = WTS_LICENSE_CURRENT_PROTOCOL_VERSION;
    ppLicenseCapabilities->fAuthenticateServer = FALSE;
    ppLicenseCapabilities->CertType = WTS_CERT_TYPE_X509;
    ppLicenseCapabilities->cbClientName = (DWORD)(strlen(clientName) + 1);
    strcpy((CHAR *)ppLicenseCapabilities->rgbClientName, clientName);
    *pcbLicenseCapabilities = sizeof(WTS_LICENSE_CAPABILITIES);

    DRIVER_LEAVE();
    return S_OK;
}

HRESULT 
CNepWRdsLicenseConnection::SendClientLicense(PBYTE pClientLicense, ULONG cbClientLicense)
{
    DRIVER_ENTER();
    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CNepWRdsLicenseConnection::RequestClientLicense(PBYTE Reserve1, ULONG Reserve2, 
                                                         PBYTE ppClientLicense, ULONG *pcbClientLicense)
{
    DRIVER_ENTER();

    *pcbClientLicense = 0;

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT 
CNepWRdsLicenseConnection::ProtocolComplete(ULONG ulComplete)
{
    DRIVER_TRACE(L"License protocol: %d\n", ulComplete);
    return S_OK;
}
