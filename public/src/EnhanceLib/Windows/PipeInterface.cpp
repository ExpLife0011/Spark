#include "Windows/IPipeClient.h"
#include "Windows/IPipeServer.h"
#include "Windows/PipeClient.h"
#include "Windows/PipeServer.h"

IPipeClient* WINAPI CreateIPipeClientInstance(TCHAR *PipeName, DWORD Timeout)
{
    return new CPipeClient(PipeName, Timeout);
}

IPipeServerService* WINAPI CreateIPipeServerServiceInstance(TCHAR* PipeName, DWORD Timeout, HANDLE StopEvent)
{
    return new CPipeServerService(PipeName, Timeout, StopEvent);
}
