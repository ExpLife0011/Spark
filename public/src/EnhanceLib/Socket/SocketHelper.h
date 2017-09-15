#pragma once

#ifndef __SOCKET_HELP_H__
#define __SOCKET_HELP_H__

#ifdef WIN32
#include <windows.h>
#else
#include "Windef.h"
#endif

BOOL SocketRead(SOCKET s, BYTE* pBuffer, DWORD dwBufferSize, DWORD* pdwReaded, HANDLE hStopEvent);
BOOL SocketWrite(SOCKET s, BYTE* pBuffer, DWORD dwBufferSize, DWORD* pdwWritten, HANDLE hStopEvent);

#endif