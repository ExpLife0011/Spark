#include <stdio.h>
#include <Windows.h>
#include <tchar.h>
#include "Windows\IPipeClient.h"
#include "Windows\IPipeServer.h"

static void ClientRequestDataHandle(IPacketBuffer* Buffer, DWORD Id, ICommunication* Param)
{
    DWORD index = *(DWORD*)Buffer->GetData();
    if (index % 1000 == 0)
    {
        wprintf(_T("Client Recv %d\r\n"), index);
    }

    Buffer->AddRef();
    Param->SendRequest(0, Buffer, NULL);
    Buffer->Release();
}

static void ServerRequestDataHandle(IPacketBuffer* Buffer, DWORD Id, ICommunication* Param)
{
    DWORD* index = (DWORD*)Buffer->GetData();
    if (*index % 1000 == 0)
    {
        wprintf(_T("Server Recv %d\r\n"), *index);
    }
    (*index)++;

    Param->SendRequest(1, Buffer, NULL);
}

static void ClientRun()
{
    DWORD index = 0;

    IPipeClient* Client = CreateIPipeClientInstance(_T("\\\\.\\PIPE\\TestPipe"), 1000);
    Client->Connect();
    Client->RegisterRequestHandle(1, ClientRequestDataHandle);
    IPacketBuffer* Buffer = CreateIBufferInstanceEx((PBYTE)&index, sizeof(DWORD));
    Client->SendRequest(0, Buffer, NULL);
    Buffer->Release();
}

static void ServerRun()
{
    HANDLE StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    IPipeServerService* Service = CreateIPipeServerServiceInstance(_T("\\\\.\\PIPE\\TestPipe"), 1000, StopEvent);
    Service->RegisterRequestHandle(0, ServerRequestDataHandle);
    Service->StartMainService();
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        wprintf(_T("argment count is %d\r\n"), argc);
    }
    else
    {
        if (stricmp(argv[1], "-c") == 0)
        {
            wprintf(_T("Client mode\r\n"));
            ClientRun();
        }
        else if (stricmp(argv[1], "-s") == 0)
        {
            wprintf(_T("Server mode\r\n"));
            ServerRun();
        }
        else
        {
            wprintf(_T("unknow mode\r\n"));
        }
    }

    getchar();
}