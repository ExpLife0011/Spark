#include <stdio.h>
#include <Windows.h>
#include <tchar.h>
#include "Windows\IPipeClient.h"
#include "Windows\IPipeServer.h"

using namespace enlib;

static void ServerRequestDataHandle(CObjPtr<IPacketBuffer> Buffer, DWORD Id, CObjPtr<ICommunication> Param)
{
    DWORD* index = (DWORD*)Buffer->GetData();
    if ((*index % 100) == 0)
    {
        wprintf(_T("Server Recv %d\r\n"), *index);
    }
}

static void ClientRun()
{
    DWORD index = 0;

    while (TRUE)
    {
        HANDLE Done = CreateEvent(NULL, TRUE, FALSE, NULL);
        CObjPtr<IPipeClient> spClient = CreateIPipeClientInstance(_T("\\\\.\\PIPE\\TestPipe"), 1000);
        spClient->Connect();
        CObjPtr<IPacketBuffer> spBuffer = CreateIBufferInstanceEx((PBYTE)&index, sizeof(DWORD));
        spClient->SendRequest(0, spBuffer, Done);
        WaitForSingleObject(Done, INFINITE);
        spClient->DisConnect();
        CloseHandle(Done);
        index++;
    }
}

static void ServerRun()
{
    HANDLE StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    CObjPtr<IPipeServerService> Service = CreateIPipeServerServiceInstance(_T("\\\\.\\PIPE\\TestPipe"), 1000, StopEvent);
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