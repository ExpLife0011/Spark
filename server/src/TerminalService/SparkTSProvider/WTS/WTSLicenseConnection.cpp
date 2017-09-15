#include "stdafx.h"
#include "Log\LogEx.h"
#include "WTS\WTSLicenseConnection.h"

CSparkWTSLicenseConnection::CSparkWTSLicenseConnection()
{
    m_lRef = 1;
    DllAddRef();
}

CSparkWTSLicenseConnection::~CSparkWTSLicenseConnection()
{
    DRIVER_ENTER();

    DllRelease();

    DRIVER_LEAVE();
}

HRESULT CSparkWTSLicenseConnection::QueryInterface(const IID&  iid, VOID** ppv)
{
    DRIVER_ENTER();

    TCHAR guidString[512];

    if (iid == IID_IUnknown)
    {
        DRIVER_INFO(_T("CSparkWTSLicenseConnection::QueryInterface type IUnknow\r\n"));
        *ppv = this;
    }
    else if (iid == __uuidof(IWTSProtocolLicenseConnection))
    {
        DRIVER_INFO(_T("CSparkWTSLicenseConnection::QueryInterface type IWTSProtocolListener\r\n"));
        *ppv = this;
    }
    else
    {
        DRIVER_ERROR(_T("CSparkWTSLicenseConnection::QueryInterface %s. Returning E_NOINTERFACE.\r\n"), GUIDToString(iid, guidString));
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    this->AddRef();

    DRIVER_LEAVE();

    return S_OK;
}

ULONG CSparkWTSLicenseConnection::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG CSparkWTSLicenseConnection::Release()
{
    ULONG Ref = InterlockedDecrement(&m_lRef);

    if (Ref == 0)
    {
        delete this;
        return 0;
    }
    return Ref;
}

HRESULT CSparkWTSLicenseConnection::RequestLicensingCapabilities(PWRDS_LICENSE_CAPABILITIES ppLicenseCapabilities, 
                                                         ULONG *pcbLicenseCapabilities)
{
    CHAR *clientName = "SparkClient";

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

HRESULT CSparkWTSLicenseConnection::SendClientLicense(PBYTE pClientLicense, ULONG cbClientLicense)
{
    DRIVER_ENTER();

    UNREFERENCED_PARAMETER(pClientLicense);
    UNREFERENCED_PARAMETER(cbClientLicense);

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWTSLicenseConnection::RequestClientLicense(PBYTE Reserve1, ULONG Reserve2, 
                                                         PBYTE ppClientLicense, ULONG *pcbClientLicense)
{
    DRIVER_ENTER();

    UNREFERENCED_PARAMETER(Reserve1);
    UNREFERENCED_PARAMETER(Reserve2);
    UNREFERENCED_PARAMETER(ppClientLicense);

    *pcbClientLicense = 0;

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT CSparkWTSLicenseConnection::ProtocolComplete(ULONG ulComplete)
{
    DRIVER_TRACE(L"License protocol: %d\n", ulComplete);
    return S_OK;
}
