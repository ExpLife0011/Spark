#pragma once

#ifndef __ICOMMUNICATE_H__
#define __ICOMMUNICATE_H__

#ifdef WIN32
#include <windows.h>
#else
#include "Windef.h"
#endif

#include <map>
#include <list>

#include "DllExport.h"
#include "BasePacket.h"
#include "IBuffer.h"
#include "Base/BaseObject.h"

class ICommunication;

typedef void(*RequestPacketHandle) (IPacketBuffer* Buffer, DWORD Id, ICommunication* Param);

typedef void(*RequestDataHandle) (BYTE* Buffer, DWORD BufferLen, DWORD Id, ICommunication* Param);

typedef void(*EndHandle) (ICommunication* Param);

typedef void(*ConnectHandle) (ICommunication* Server);

class DLL_COMMONLIB_API ICommunication : public virtual CBaseObject
{
public:
    virtual BOOL WINAPI StartCommunication() = 0;

    virtual VOID WINAPI StopCommunication() = 0;

    virtual BOOL WINAPI RegisterEndHandle(EndHandle Func) = 0;

    virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestPacketHandle Func) = 0;

    virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestDataHandle Func) = 0;

    virtual BOOL WINAPI SendRequest(DWORD Type, PBYTE Data, DWORD DataLen, HANDLE DoneEvent = NULL) = 0;

    virtual BOOL WINAPI SendRequest(DWORD Type, IPacketBuffer* Buffer, HANDLE DoneEvent = NULL) = 0;

    virtual BOOL WINAPI SendRespone(DWORD Type, PBYTE Data, DWORD DataLen, DWORD Id, HANDLE DoneEvent = NULL) = 0;

    virtual BOOL WINAPI SendRespone(DWORD Type, IPacketBuffer* Buffer, DWORD Id, HANDLE DoneEvent = NULL) = 0;

    virtual BOOL WINAPI SendRequestWithRespone(DWORD Type, IPacketBuffer* Buffer, IPacketBuffer** Reply, HANDLE DoneEvent) = 0;

    virtual VOID WINAPI CancelIO() = 0;

    virtual DWORD WINAPI GetSendBufferLength() = 0;

protected:

    virtual IPacketBuffer* RecvAPacket(HANDLE StopEvent) = 0;

    virtual BOOL SendAPacket(IPacketBuffer* Buffer, HANDLE StopEvent) = 0;
};

#endif
