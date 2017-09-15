#include "stdafx.h"
#include "Windows/IService.h"

static ServiceBaseInfo*      gServiceInfo    = NULL;
static HANDLE                gTerminateEvent = NULL;
static SERVICE_STATUS_HANDLE gServiceStatusHandle = NULL;
static DWORD                 gCurrentState;

static BOOL 
SendStatusToSCM(DWORD dwCurrentState,
                DWORD dwWin32ExitCode, 
                DWORD dwServiceSpecificExitCode,
                DWORD dwCheckPoint,
                DWORD dwWaitHint)
{
    BOOL success;
    SERVICE_STATUS serviceStatus;

    gCurrentState = dwCurrentState;
    
    // Fill in all of the SERVICE_STATUS fields
    serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    serviceStatus.dwCurrentState = dwCurrentState;

    /*
     * If in the process of doing something, then accept
     * no control events, else accept anything
     */
    if ( dwCurrentState == SERVICE_START_PENDING )
    {
        serviceStatus.dwControlsAccepted = 0;
    }
    else
    {
        serviceStatus.dwControlsAccepted =  SERVICE_ACCEPT_STOP |
                                            SERVICE_ACCEPT_SHUTDOWN;
    }

    /*
     * if a specific exit code is defined, set up
     * the win32 exit code properly
     */
    if ( dwServiceSpecificExitCode == 0 )
    {
        serviceStatus.dwWin32ExitCode = dwWin32ExitCode;
    }
    else
    {
        serviceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
    }

    serviceStatus.dwServiceSpecificExitCode = dwServiceSpecificExitCode;
    serviceStatus.dwCheckPoint = dwCheckPoint;
    serviceStatus.dwWaitHint = dwWaitHint;

    // Pass the status record to the SCM
    success = SetServiceStatus(gServiceStatusHandle, &serviceStatus);
    return success;
}

/*
 * 服务事件处理函数
 */
static DWORD WINAPI 
HandlerEx(DWORD controlCode,
          DWORD dwEventType,
          LPVOID lpEventData,
          LPVOID lpContext)
{
    UNREFERENCED_PARAMETER(dwEventType);
    UNREFERENCED_PARAMETER(lpEventData);
    UNREFERENCED_PARAMETER(lpContext);

    switch ( controlCode )
    {
        /*  
         *  SERVICE_CONTROL_SHUTDOWN means turn off the machine
         *  Do nothing in a shutdown. Could do cleanup
         *  here but it must be very quick.
         */
        case SERVICE_CONTROL_SHUTDOWN:
            SendStatusToSCM( SERVICE_STOP_PENDING,
                             NO_ERROR, 0, 1, 1000 );
            SetEvent(gTerminateEvent);
            return NO_ERROR;

        // Stop the service
        case SERVICE_CONTROL_STOP:
            SendStatusToSCM( SERVICE_STOP_PENDING,
                             NO_ERROR, 0, 1, 5000 );
            SetEvent(gTerminateEvent);
            return NO_ERROR;

        case SERVICE_CONTROL_INTERROGATE:
            SendStatusToSCM(gCurrentState, NO_ERROR, 0, 0, 0);
            return NO_ERROR;

        default:
            SendStatusToSCM(gCurrentState, NO_ERROR, 0, 0, 0);
            return ERROR_CALL_NOT_IMPLEMENTED;
    }
}

static VOID WINAPI ServiceMainFunction(DWORD argc, LPTSTR * argv) 
{
    BOOL ret;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    gServiceStatusHandle = RegisterServiceCtrlHandlerEx(gServiceInfo->ServiceName, HandlerEx, NULL);
    if (!gServiceStatusHandle)
    {
        goto exit;
    }

    ret = SendStatusToSCM(SERVICE_START_PENDING, NO_ERROR, 0, 1, 5000);
    if (!ret)
    {
        goto exit;
    }

    gTerminateEvent = CreateEvent(0, TRUE, FALSE, NULL);
    if (!gTerminateEvent)
    {
        goto exit;
    }

    if (gServiceInfo->StartFun)
    {
        (gServiceInfo->StartFun)();
    }

    ret = SendStatusToSCM( SERVICE_RUNNING, NO_ERROR, 0, 0, 0 );
    if ( !ret )
    {
        goto end;
    }

    // 等待服务停止
    WaitForSingleObject(gTerminateEvent, INFINITE);

end:
    if (gServiceInfo->StopFun)
    {
        (gServiceInfo->StopFun)();
    }

exit:
    if (gTerminateEvent)
    {
        CloseHandle(gTerminateEvent);
    }

    if (gServiceStatusHandle)
    {
        SendStatusToSCM(SERVICE_STOPPED, 0, 0, 0, 0);
    }
}

static BOOL WINAPI HandlerConsoleCtrl(DWORD dwCtrlType)
{
    if ( dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_CLOSE_EVENT )
    {
        SetEvent(gTerminateEvent);
        return TRUE;
    }

    return FALSE;
}

static int ServiceMainProcess(ServiceBaseInfo *service)
{
    WORD            wVersionRequested;
    WSADATA         wsaData;
    BOOL            bConsole = FALSE;

    gServiceInfo = service;
    
    wVersionRequested = MAKEWORD(2, 2);  
    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        goto EXIT;
    }

    if (!service->ConsoloMode)
    {
        WCHAR ServiceName[MAX_PATH];

        wcscpy(ServiceName, service->ServiceName);

        SERVICE_TABLE_ENTRY serviceTable[] = { 
            {ServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMainFunction},
            {NULL, NULL}
        };

        StartServiceCtrlDispatcher(serviceTable);
    }
    else
    {
        //控制台模式
        gTerminateEvent = CreateEvent(0, TRUE, FALSE, NULL);
        if (gTerminateEvent == NULL )
        {
            goto EXIT;
        }

        if (service->StartFun)
        {
            (service->StartFun)();
        }

        SetConsoleCtrlHandler(HandlerConsoleCtrl, TRUE);
        WaitForSingleObject(gTerminateEvent, INFINITE);

        if (service->StopFun)
        {
            (service->StopFun)();
        }
        CloseHandle(gTerminateEvent);
    }

EXIT:

    WSACleanup();

    return 0;
}

int WINAPI ServiceMainLib(ServiceBaseInfo *ServiceInfo)
{
    ServiceMainProcess(ServiceInfo);
    return 0;
}
