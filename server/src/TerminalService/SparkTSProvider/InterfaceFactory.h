#pragma once

#ifndef __INTERFACE_FACTORY_H__
#define __INTERFACE_FACTORY_H__

#include <Windows.h>

class CInterfaceFactory : public IClassFactory
{
public:
    CInterfaceFactory();

    ~CInterfaceFactory();

    virtual HRESULT WINAPI QueryInterface(const IID& riid, VOID** ppvObj);

    virtual ULONG WINAPI AddRef();

    virtual ULONG WINAPI Release();

    virtual HRESULT WINAPI CreateInstance(LPUNKNOWN  pUnkOuter,
        const IID& riid,
        LPVOID*    ppvObject);

    virtual HRESULT WINAPI LockServer(BOOL bLock);

private:
    DWORD m_lRef;
};




#endif
