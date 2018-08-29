#pragma once

#ifndef __PIPE_SERVER_H__
#define __PIPE_SERVER_H__

#include "Common/Communication.h"
#include "Windows/IPipeServer.h"
#include "DllExport.h"
#include <map>

class CPipeServer : public IPipeServer, public CCommunication
{
public:
    CPipeServer(HANDLE hPipe);

    virtual ~CPipeServer();

    virtual BOOL WINAPI Start();

    virtual void WINAPI Stop();

    virtual BOOL WINAPI IsConnected();

private:
    virtual IPacketBuffer* RecvAPacket(HANDLE StopEvent);

    virtual BOOL SendAPacket(IPacketBuffer* Packet, HANDLE StopEvent);

    static void PipeClear(ICommunication* param);

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

    virtual VOID WINAPI SetParam(const CHAR* ParamKeyword, CBaseObjPtr<CBaseObject> Param);

private:
    static BOOL ServiceMainThreadProc(LPVOID Parameter, HANDLE StopEvent);
    
    static VOID ServiceMainThreadEndProc(LPVOID Parameter);
    
    void InitalizeServer(CPipeServer* Server);

    void InitSyncLock();

    IThread*                                         m_pMainThread;

    TCHAR*                                           m_szPipeName;
    DWORD                                            m_dwTimeout;
    HANDLE                                           m_hStopEvent;
    CRITICAL_SECTION                                 m_csLock;
    std::map<DWORD, RequestPacketHandle>             m_ReqPacketList;
    std::map<DWORD, RequestDataHandle>               m_ReqDataList;
    std::list<EndHandle>                             m_EndList;
    std::list<ConnectHandle>                         m_ConnectList;
    std::map<UINT32, CBaseObjPtr<CBaseObject>>       m_ParamMap;
    HANDLE                                           m_hSyncLocker;
};

#endif