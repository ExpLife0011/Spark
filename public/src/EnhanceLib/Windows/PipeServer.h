#pragma once

#ifndef __PIPE_SERVER_H__
#define __PIPE_SERVER_H__

#include "Common/Communication.h"
#include "Windows/IPipeServer.h"
#include "DllExport.h"
#include <map>

namespace enlib
{
    class CPipeServer : public IPipeServer, public CCommunication
    {
    public:
        CPipeServer(HANDLE hPipe);

        virtual ~CPipeServer();

        virtual BOOL WINAPI Start();

        virtual void WINAPI Stop();

        virtual BOOL WINAPI IsConnected();

    private:
        virtual CObjPtr<IPacketBuffer> RecvAPacket(HANDLE StopEvent);

        virtual BOOL SendAPacket(CObjPtr<IPacketBuffer> Buffer, HANDLE StopEvent);

        static void PipeClear(CObjPtr<ICommunication> Param);

        HANDLE m_hPipe;
    };

    class CPipeServerService : public IPipeServerService
    {
    public:
        CPipeServerService(TCHAR* PipeName, DWORD Timeout, HANDLE StopEvent);

        ~CPipeServerService();

        virtual BOOL WINAPI StartMainService();

        virtual void WINAPI StopMainService();

        virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestPacketHandle Func);

        virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestDataHandle Func);

        virtual void WINAPI RegisterEndHandle(EndHandle Func);

        virtual void WINAPI RegisterConnectHandle(ConnectHandle Func);

    private:
        static BOOL ServiceMainThreadProc(CObjPtr<CObject> Parameter, HANDLE StopEvent);

        void InitalizeServer(CObjPtr<CPipeServer> Server);

        void InitSyncLock();

        CObjPtr<IThread>                                 m_spMainThread;

        TCHAR*                                           m_szPipeName;
        DWORD                                            m_dwTimeout;
        HANDLE                                           m_hStopEvent;
        CRITICAL_SECTION                                 m_csLock;
        std::map<DWORD, RequestPacketHandle>             m_ReqPacketList;
        std::map<DWORD, RequestDataHandle>               m_ReqDataList;
        std::list<EndHandle>                             m_EndList;
        std::list<ConnectHandle>                         m_ConnectList;
        HANDLE                                           m_hSyncLocker;
    };
};

#endif