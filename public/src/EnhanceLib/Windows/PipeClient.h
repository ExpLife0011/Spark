#pragma once

#ifndef __PIPE_CLIENT_H__
#define __PIPE_CLIENT_H__

#include "Windows/IPipeClient.h"
#include "Common/Communication.h"

class CPipeClient : public IPipeClient, public CCommunication
{
public:
    CPipeClient(TCHAR *PipeName, DWORD Timeout);

    virtual ~CPipeClient();

    virtual BOOL WINAPI Connect();

    virtual void WINAPI DisConnect();
    
    virtual BOOL WINAPI IsConnected();

    virtual PVOID WINAPI GetParam();

	virtual VOID WINAPI SetParam(PVOID Param);

private:
    virtual IPacketBuffer* RecvAPacket(HANDLE StopEvent);

    virtual BOOL SendAPacket(IPacketBuffer* Buffer, HANDLE StopEvent);

    static void PipeClear(ICommunication* param);

    HANDLE InitSyncLock();
    
    HANDLE m_hPipe;
    TCHAR* m_szPipeName;
    DWORD  m_dwTimeout;
	PVOID  m_pParam;
    BOOL   m_bAlive;
};

#endif