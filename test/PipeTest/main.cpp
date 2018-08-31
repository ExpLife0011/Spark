#include <stdio.h>
#include <Windows.h>
#include <tchar.h>
#include "Windows\IPipeClient.h"
#include "Windows\IPipeServer.h"

using namespace enlib;

static void ServerRequestDataHandle(CObjPtr<IPacketBuffer> Buffer, DWORD Id, CObjPtr<ICommunication> Param)
{
    DWORD* index = (DWORD*)Buffer->GetData();
    wprintf(_T("Server Recv %d\r\n"), *index);


    Param->SendRequest(0, Buffer);
}


HANDLE DoneEvent = NULL;

static void ClientRequestDataHandle(CObjPtr<IPacketBuffer> Buffer, DWORD Id, CObjPtr<ICommunication> Param)
{
    DWORD* index = (DWORD*)Buffer->GetData();
    wprintf(_T("Client Recv %d\r\n"), *index);
    SetEvent(DoneEvent);
}


static void ClientRun()
{
    DWORD index = 0;

    DoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    while (TRUE)
    {
        
        CObjPtr<IPipeClient> spClient = CreateIPipeClientInstance(_T("\\\\.\\PIPE\\TestPipe"), 1000);
        spClient->RegisterRequestHandle(0, ClientRequestDataHandle);
        spClient->Connect();
        CObjPtr<IPacketBuffer> spBuffer = CreateIBufferInstanceEx((PBYTE)&index, sizeof(DWORD));

        ResetEvent(DoneEvent);
        spClient->SendRequest(0, spBuffer, NULL);
        WaitForSingleObject(DoneEvent, INFINITE);
        spClient->DisConnect();
        index++;
    }

    CloseHandle(DoneEvent);
}

static void ServerRun()
{
    HANDLE StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    CObjPtr<IPipeServerService> spService = CreateIPipeServerServiceInstance(_T("\\\\.\\PIPE\\TestPipe"), 1000, StopEvent);
    spService->RegisterRequestHandle(0, ServerRequestDataHandle);
    spService->StartMainService();
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