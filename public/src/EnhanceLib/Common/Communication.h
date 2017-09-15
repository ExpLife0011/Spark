#pragma once

#ifndef __COMMUNICATE_H__
#define __COMMUNICATE_H__

#ifdef WIN32
#include <windows.h>
#else
#include "Windef.h"
#endif

#include <map>
#include <list>

#include "Common/ICommunication.h"
#include "Common/IThread.h"
#include "Base/BaseObject.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

#define HDR_DIRECT_MASK 0x1
#define HDR_DIRECT_REQ  0x0
#define HDR_DIRECT_RESP 0x1

typedef struct {
    WORD  Flags;
    WORD  Id;
    DWORD Type;
    DWORD Len;
    BYTE  Data[1];
} DataHdr;

typedef struct 
{
    DWORD  Id;
    HANDLE Event;
	IPacketBuffer** Buffer;
} WaitResponeNode;

typedef struct
{
    HANDLE Event;
    IPacketBuffer* Buffer;
} SendNode;

#pragma pack()

#ifdef __cplusplus
}
#endif

class CCommunication : public virtual ICommunication
{
public:
    CCommunication();

    virtual ~CCommunication();

    virtual BOOL WINAPI StartCommunication();

    virtual VOID WINAPI StopCommunication();

    virtual BOOL WINAPI RegisterEndHandle(EndHandle Func);

    virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestPacketHandle Func);

    virtual BOOL WINAPI SendRequest(DWORD Type, IPacketBuffer* Buffer, HANDLE DoneEvent);

    virtual BOOL WINAPI SendRespone(DWORD Type, IPacketBuffer* Buffer, WORD Id, HANDLE DoneEvent);

    virtual BOOL WINAPI SendRequestWithRespone(DWORD Type, IPacketBuffer* Buffer, IPacketBuffer** Reply, HANDLE DoneEvent);

    virtual VOID WINAPI CancelIO();

    virtual DWORD WINAPI GetSendBufferLength();

private:
    static BOOL RecvThreadProc(LPVOID Parameter, HANDLE StopEvent);

    static void RecvThreadEndProc(LPVOID Parameter);

    static BOOL SendThreadProc(LPVOID Parameter, HANDLE StopEvent);

    static void SendThreadEndProc(LPVOID Parameter);

    void SendPacket(IPacketBuffer* Buffer, HANDLE DoneEvent);
    
    IThread*                             m_pRecvThread;
    IThread*                             m_pSendThread;
    WORD                                 m_dwRequestId;
    std::map<DWORD, RequestPacketHandle> m_ReqPacketList;
    std::map<DWORD, WaitResponeNode*>    m_WaitRespNodeList;
    std::list<EndHandle>                 m_EndList;
    std::list<SendNode>                  m_SendList;
    HANDLE                               m_hSendEvent;
    CRITICAL_SECTION                     m_csRequestIdLock;
    CRITICAL_SECTION                     m_csRecvListLock;
    CRITICAL_SECTION                     m_csSendListLock;
    DWORD                                m_dwSendBufferLength;
};

#endif
