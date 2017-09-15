#include "stdafx.h"
#include "Log\LogEx.h"

TCHAR* GUIDToString(const GUID &guid, TCHAR* guid_str)
{
#ifdef UNICODE
    wsprintf(guid_str, _T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
#else
    sprintf(guid_str, _T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
#endif
        
    return guid_str;
}

BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD dwReason, void *)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        CLogEx::LogInit(hinstDll, _T("SparkProvider_logconf.ini"));
        CLogEx::SetModuleName(_T("SparkProvider"));
        CLogEx::AddOutputModule(SERVER_MODULE);
        CLogEx::AddOutputModule(SERVER_DRIVER);
        CLogEx::SetLogLevel(LOG_LEVEL_TRACE);
        CLogEx::SetOutputType(LOG_OUTPUT_DBGPORT);

        DRIVER_INFO(_T(">>>>> Spark terminal service plugin loaded, build at %S, %S\r\n"), __DATE__, __TIME__);
        
        DisableThreadLibraryCalls(hinstDll);
        break;
    case DLL_PROCESS_DETACH:
        DRIVER_INFO(_T(">>>>> Spark terminal service plugin unload\r\n"));
        CTLogEx_done();
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}