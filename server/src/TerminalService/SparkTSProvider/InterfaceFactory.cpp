#include "stdafx.h"
#include "InterfaceFactory.h"
#include "WRDS\WRdsManager.h"
#include "WTS\WTSManager.h"
#include "Log\LogEx.h"

DEFINE_GUID(CLSID_SPARK_PROTOCOL_MANAGER,
    0xE99FA5E4, 0xA1C9, 0x4495, 0x8b, 0xf0, 0xd0, 0x97, 0x2d, 0x58, 0xe8, 0x41);

DEFINE_GUID(CLSID_SPARK_PROTOCOL_MANAGER_FOR_WIN7,
    0x5828227C, 0x20CF, 0x4408, 0xB7, 0x3F, 0x73, 0xAB, 0x70, 0xB8, 0x84, 0x9F);

static LONG gRefCount = 0;

CInterfaceFactory::CInterfaceFactory()
{
    m_lRef = 1;
}

CInterfaceFactory::~CInterfaceFactory()
{

}

ULONG CInterfaceFactory::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG CInterfaceFactory::Release()
{
    ULONG Ref = InterlockedDecrement(&m_lRef);

    if (Ref == 0)
    {
        delete this;
        return 0;
    }
    return Ref;
}

HRESULT WINAPI CInterfaceFactory::QueryInterface(const IID&  iid,
    void** ppv)
{
    DRIVER_ENTER();

    if (iid == IID_IUnknown)
    {
        DRIVER_TRACE(_T("CInterfaceFactory::QueryInterface Get IUnknow\r\n"));
        *ppv = this;
    }
    else if (iid == IID_IClassFactory)
    {
        DRIVER_TRACE(_T("CInterfaceFactory::QueryInterface Get IClassFactory\r\n"));
        *ppv = this;
    }
    else
    {
        DRIVER_TRACE(_T("CInterfaceFactory::QueryInterface not support\r\n"));
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    this->AddRef();

    DRIVER_LEAVE();

    return S_OK;
}

HRESULT WINAPI CInterfaceFactory::CreateInstance(LPUNKNOWN pUnkOuter, const IID& riid, LPVOID* ppvObject)
{
    HRESULT hr = E_FAIL;
    DRIVER_ENTER();

    //参数检测
    if (ppvObject == NULL)
    {
        DRIVER_ERROR(_T("CInterfaceFactory::CreateInstance Parameter Error\r\n"));
        return E_POINTER;
    }

    *ppvObject = NULL;

    //要聚合我们不支持
    if (pUnkOuter != NULL)
    {
        DRIVER_ERROR(_T("CInterfaceFactory::CreateInstance No Aggregation\r\n"));
        return CLASS_E_NOAGGREGATION;
    }

    if (riid == __uuidof(IWRdsProtocolManager))
    {
#if 1
        hr = E_OUTOFMEMORY;
#else
        //创建元素
        CSparkWRDSManager* Manager = new CSparkWRDSManager();

        if (Manager == NULL)
        {
            DRIVER_ERROR(_T("CInterfaceFactory::CreateInstance No Memory\r\n"));
            return E_OUTOFMEMORY;
        }

        //搞起
        hr = Manager->QueryInterface(riid, ppvObject);

        Manager->Release();
#endif
    }
    else if (riid == __uuidof(IWTSProtocolManager))
    {
        //创建元素
        CSparkWTSManager* Manager = new CSparkWTSManager();
        if (Manager == NULL)
        {
            DRIVER_ERROR(_T("CInterfaceFactory::CreateInstance No Memory\r\n"));
            return E_OUTOFMEMORY;
        }

        //搞起
        hr = Manager->QueryInterface(riid, ppvObject);

        Manager->Release();
    }

    DRIVER_LEAVE();

    return hr;
}

HRESULT WINAPI CInterfaceFactory::LockServer(BOOL bLock)
{
    if (bLock)
    {
        DllAddRef();
    }
    else
    {
        DllRelease();
    }

    return S_OK;
}

/**
 * @brief 增加全局对象引用个数
 */
void DllAddRef()
{
    InterlockedIncrement(&gRefCount);
}

/**
 * @brief 减少全局对象引用个数
 */
void DllRelease()
{
    InterlockedDecrement(&gRefCount);
}

HRESULT WINAPI DllCanUnloadNow()
{
    return (gRefCount > 0) ? S_FALSE : S_OK;
}

HRESULT WINAPI DllGetClassObject(REFCLSID clsid, REFIID iid, void** ppv)
{
    DRIVER_ENTER();
    //入参判断
    if (ppv == NULL)
    {
        return E_POINTER;
    }

    //先赋值为NULL
    *ppv = NULL;

    //只匹配OEMRENDER类型
    if (clsid != CLSID_SPARK_PROTOCOL_MANAGER
        && clsid != CLSID_SPARK_PROTOCOL_MANAGER_FOR_WIN7)
    {
        DRIVER_ERROR(_T("DllGetClassObject: Class not available!\r\n"));
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    //创建Class对象
    CInterfaceFactory* Factory = new CInterfaceFactory();
    if (Factory == NULL)
    {
        DRIVER_ERROR(_T("DllGetClassObject: Out of Memory!\r\n"));
        return E_OUTOFMEMORY;
    }

    //搞起
    HRESULT hrResult = Factory->QueryInterface(iid, ppv);
    Factory->Release();

    return hrResult;
}