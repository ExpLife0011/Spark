#pragma once

#ifndef __PIPE_SERVER_H__
#define __PIPE_SERVER_H__

#include "Common/Communication.h"
#include "Windows/IPipeServer.h"
#include "DllExport.h"

class CPipeServer : public IPipeServer, public CCommunication
{
public:
    CPipeServer(HANDLE hPipe);

    virtual ~CPipeServer();

    virtual BOOL WINAPI Start();

    virtual void WINAPI Stop();

    virtual BOOL WINAPI IsConnected();

    virtual PVOID WINAPI GetParam();

	virtual VOID WINAPI SetParam(PVOID Param);

private:
    virtual IPacketBuffer* RecvAPacket(HANDLE StopEvent);

    virtual BOOL SendAPacket(IPacketBuffer* Packet, HANDLE StopEvent);

    static void PipeClear(ICommunication* param);

    BOOL   m_bAlive;
    HANDLE m_hPipe;
    PVOID  m_pParam;
};

class CPipeServerService : public IPipeServerService
{
public:
    CPipeServerService(TCHAR* PipeName, DWORD Timeout, HANDLE StopEvent);

    ~CPipeServerService();

    virtual BOOL WINAPI StartMainService();

    virtual void WINAPI StopMainService();

    virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestPacketHandle Func);

    virtual void WINAPI RegisterEndHandle(EndHandle Func);

    virtual VOID WINAPI SetParam(PVOID Param);

private:
    static BOOL ServiceMainThreadProc(LPVOID Parameter, HANDLE StopEvent);
    
    void InitalizeServer(CPipeServer* Server);

    void InitSyncLock();

    IThread*                             m_pMainThread;

    TCHAR*                               m_szPipeName;
    DWORD                                m_dwTimeout;
    HANDLE                               m_hStopEvent;
    CRITICAL_SECTION                     m_csLock;
    std::map<DWORD, RequestPacketHandle> m_ReqPacketList;
    std::list<EndHandle>                 m_EndList;
    PVOID                                m_pParam;
    HANDLE                               m_hSyncLocker;
};

#endif