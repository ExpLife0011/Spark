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

namespace enlib
{
    class ICommunication;

    typedef void(*RequestPacketHandle) (CObjPtr<IPacketBuffer> Buffer, DWORD Id, CObjPtr<ICommunication> Param);

    typedef void(*RequestDataHandle) (BYTE* Buffer, DWORD BufferLen, DWORD Id, CObjPtr<ICommunication> Param);

    typedef void(*EndHandle) (CObjPtr<ICommunication> Param);

    typedef void(*ConnectHandle) (CObjPtr<ICommunication> Server);

    class DLL_COMMONLIB_API ICommunication : public virtual CObject
    {
    public:
        virtual BOOL WINAPI StartCommunication() = 0;

        virtual VOID WINAPI StopCommunication() = 0;

        virtual BOOL WINAPI RegisterEndHandle(EndHandle Func) = 0;

        virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestPacketHandle Func) = 0;

        virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestDataHandle Func) = 0;

        virtual BOOL WINAPI SendRequest(DWORD Type, PBYTE Data, DWORD DataLen, HANDLE DoneEvent = NULL) = 0;

        virtual BOOL WINAPI SendRequest(DWORD Type, CObjPtr<IPacketBuffer> Buffer, HANDLE DoneEvent = NULL) = 0;

        virtual BOOL WINAPI SendRespone(DWORD Type, PBYTE Data, DWORD DataLen, DWORD Id, HANDLE DoneEvent = NULL) = 0;

        virtual BOOL WINAPI SendRespone(DWORD Type, CObjPtr<IPacketBuffer> Buffer, DWORD Id, HANDLE DoneEvent = NULL) = 0;

        virtual BOOL WINAPI SendRequestWithRespone(DWORD Type, CObjPtr<IPacketBuffer> Buffer, CObjPtr<IPacketBuffer>* Reply, HANDLE DoneEvent) = 0;

        virtual VOID WINAPI CancelIO() = 0;

        virtual DWORD WINAPI GetSendBufferLength() = 0;

    protected:
        virtual CObjPtr<IPacketBuffer> RecvAPacket(HANDLE StopEvent) = 0;

        virtual BOOL SendAPacket(CObjPtr<IPacketBuffer> Buffer, HANDLE StopEvent) = 0;
    };
};

#endif
