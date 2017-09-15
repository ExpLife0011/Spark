#pragma once

#ifndef __PIPE_HELPER_H__
#define __PIPE_HELPER_H__

#include <windows.h>
#include "Common\IBuffer.h"
#include "Common\BasePacket.h"

#ifdef UNICODE
    #define PipeConnect PipeConnectW
#else
    #define PipeConnect PipeConnectA
#endif

extern BOOL PipeReadNBytes(
    HANDLE PipeHandle,
    PBYTE  Data,
    DWORD  Length,
    DWORD  Timeout,
    HANDLE StopEvent);

extern BOOL PipeWriteNBytes(
    HANDLE PipeHandle,
    PBYTE  Data,
    DWORD  Length,
    DWORD  Timeout,
    HANDLE StopEvent);

extern IPacketBuffer* PipeRecvAPacket(
    HANDLE PipeHandle,
    DWORD  Timeout,
    HANDLE StopEvent);

extern BOOL PipeSendAPacket(HANDLE PipeHandle,
    IPacketBuffer* Packet,
    DWORD  Type,
    DWORD  Timeout,
    HANDLE StopEvent);

extern HANDLE PipeConnectA(PCHAR PipeName, DWORD Timeout);

extern HANDLE PipeConnectW(PWCHAR PipeName, DWORD Timeout);

extern void PipeDisconnect(HANDLE PipeHandle);

#endif
