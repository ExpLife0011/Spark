#include "Windows/IPipeClient.h"
#include "Windows/IPipeServer.h"
#include "Windows/PipeClient.h"
#include "Windows/PipeServer.h"

using namespace enlib;

CObjPtr<IPipeClient> WINAPI CreateIPipeClientInstance(TCHAR *PipeName, DWORD Timeout)
{
    CObjPtr<IPipeClient> spRet = NULL;
    IPipeClient* pClient = NULL;
    
    pClient = new CPipeClient(PipeName, Timeout);

    if (pClient)
    {
        spRet = pClient;
        pClient->Release();
        pClient = NULL;
    }

    return spRet;
}

CObjPtr<IPipeServerService> WINAPI CreateIPipeServerServiceInstance(TCHAR* PipeName, DWORD Timeout, HANDLE StopEvent)
{
    CObjPtr<IPipeServerService> spRet = NULL;
    IPipeServerService* pService = NULL;
    
    pService = new CPipeServerService(PipeName, Timeout, StopEvent);

    if (pService)
    {
        spRet = pService;
        pService->Release();
        pService = NULL;
    }

    return spRet;
}
