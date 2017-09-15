#include "StdAfx.h"
#include "Windows\GlobalEvent.h"

HANDLE WINAPI CreateGlobalEventW(WCHAR* pEventName)
{
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    HANDLE hEvent = NULL;

    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = &sd;

    hEvent = CreateEventW(&sa, TRUE, TRUE, pEventName);

    return hEvent;
}


HANDLE WINAPI CreateGlobalEventA(CHAR* pEventName)
{
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    HANDLE hEvent = NULL;

    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = &sd;

    hEvent = CreateEventA(&sa, TRUE, TRUE, pEventName);

    return hEvent;
}


HANDLE WINAPI CreateGlobalAutoEventA(CHAR* pEventName)
{
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    HANDLE hEvent = NULL;

    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = &sd;

    hEvent = CreateEventA(&sa, FALSE, TRUE, pEventName);

    return hEvent;
}

HANDLE WINAPI CreateGlobalAutoEventW(WCHAR* pEventName)
{
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    HANDLE hEvent = NULL;

    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = &sd;

    hEvent = CreateEventW(&sa, TRUE, TRUE, pEventName);

    return hEvent;
}