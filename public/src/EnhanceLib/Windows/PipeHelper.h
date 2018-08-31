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

BOOL PipeWriteNBytes(
    HANDLE PipeHandle,
    PBYTE  Data,
    DWORD  Length,
    DWORD  Timeout,
    HANDLE StopEvent);

enlib::CObjPtr<enlib::IPacketBuffer> PipeRecvAPacket(
    HANDLE PipeHandle,
    DWORD  Timeout,
    HANDLE StopEvent);

HANDLE PipeConnectA(PCHAR PipeName, DWORD Timeout);

HANDLE PipeConnectW(PWCHAR PipeName, DWORD Timeout);

void PipeDisconnect(HANDLE PipeHandle);


#endif
