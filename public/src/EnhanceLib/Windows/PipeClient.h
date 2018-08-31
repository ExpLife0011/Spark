#pragma once

#ifndef __PIPE_CLIENT_H__
#define __PIPE_CLIENT_H__

#include "Windows/IPipeClient.h"
#include "Common/Communication.h"
#include "Base/BaseObjPtr.h"

namespace enlib
{
    class CPipeClient : public IPipeClient, public CCommunication
    {
    public:
        CPipeClient(TCHAR *PipeName, DWORD Timeout);

        virtual ~CPipeClient();

        virtual BOOL WINAPI Connect();

        virtual void WINAPI DisConnect();

        virtual BOOL WINAPI IsConnected();

    private:
        virtual CObjPtr<IPacketBuffer> RecvAPacket(HANDLE StopEvent);

        virtual BOOL SendAPacket(CObjPtr<IPacketBuffer> Buffer, HANDLE StopEvent);

        static void PipeClear(CObjPtr<ICommunication> Param);

        HANDLE InitSyncLock();

        HANDLE                                     m_hPipe;
        TCHAR*                                     m_szPipeName;
        DWORD                                      m_dwTimeout;
    };
};

#endif