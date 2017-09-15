#ifndef __IWTS_CONNECTION_RDP_H__
#define __IWTS_CONNECTION_RDP_H__

#include <Windows.h>

class __declspec(uuid("{047CB677-AF07-4C8D-962E-366C1988B251}")) IWTSConnectionRdp : public IUnknown
{
    virtual HRESULT WINAPI PrepareForAccept(void *a1, int a2, int a3, int a4) = 0;
    virtual HRESULT WINAPI GetClientMonitorData(UINT *pNumMonitors, UINT *pPrimaryMonitor) = 0;
    virtual HRESULT WINAPI GetSecurityFilterCreds(int a1, int a2) = 0;
    virtual HRESULT WINAPI GetSecurityFilterClientCerts(int a1, int a2) = 0;
    virtual HRESULT WINAPI GetSecurityFilterClientToken(int a1) = 0;
    virtual HRESULT WINAPI SendLogonErrorInfoToClient(int a1, int a2) = 0;
};

#endif
