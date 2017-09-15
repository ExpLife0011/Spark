#pragma once

#ifndef __WRDS_LICENSE_CONNECTION_H__
#define __WRDS_LICENSE_CONNECTION_H__

#include <Windows.h>
#include <wtsprotocol.h>
#include "Windows\IPipeClient.h"

class CSparkWTSLicenseConnection : public IWTSProtocolLicenseConnection
{
public:
    CSparkWTSLicenseConnection();

    ~CSparkWTSLicenseConnection();

    virtual HRESULT WINAPI QueryInterface(const IID& riid, VOID** ppvObj);

    virtual ULONG WINAPI AddRef();

    virtual ULONG WINAPI Release();

    virtual HRESULT WINAPI RequestLicensingCapabilities(PWRDS_LICENSE_CAPABILITIES ppLicenseCapabilities, ULONG *pcbLicenseCapabilities);
  
    virtual HRESULT WINAPI SendClientLicense(PBYTE pClientLicense, ULONG cbClientLicense);
   
    virtual HRESULT WINAPI RequestClientLicense(PBYTE Reserve1, ULONG Reserve2, PBYTE ppClientLicense, ULONG *pcbClientLicense);
   
    virtual HRESULT WINAPI ProtocolComplete(ULONG ulComplete);

private:
    LONG                             m_lRef;
};

#endif