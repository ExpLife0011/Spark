#include <Windows.h>
#include "Windows/RegOperation.h"

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);
    UNREFERENCED_PARAMETER(hModule);

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            WSADATA WSAData;

            WSAStartup(MAKEWORD(2, 0), &WSAData);
            break;
        }
        case DLL_PROCESS_DETACH:
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        
            break;
    }
    return TRUE;
}