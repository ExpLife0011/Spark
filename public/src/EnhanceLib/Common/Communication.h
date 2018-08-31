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


#define HDR_DIRECT_MASK 0x1
#define HDR_DIRECT_REQ  0x0
#define HDR_DIRECT_RESP 0x1

namespace enlib {
    #pragma pack(1)

    typedef struct {
        DWORD Flags;
        DWORD Type;
        DWORD Id;
        DWORD Len;
        BYTE  Data[0];
    } DataHdr;

    typedef struct
    {
        DWORD  Id;
        HANDLE Event;
        CObjPtr<IPacketBuffer>* Buffer;
    } WaitResponeNode;

    typedef struct
    {
        HANDLE Event;
        CObjPtr<IPacketBuffer> Buffer;
    } SendNode;

    #pragma pack()

    class CCommunication : public virtual ICommunication
    {
    public:
        CCommunication();

        virtual ~CCommunication();

        virtual BOOL WINAPI StartCommunication();

        virtual VOID WINAPI StopCommunication();

        virtual BOOL WINAPI RegisterEndHandle(EndHandle Func);

        virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestPacketHandle Func);

        virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestDataHandle Func);

        virtual BOOL WINAPI SendRequest(DWORD Type, PBYTE Data, DWORD DataLen, HANDLE DoneEvent = NULL);

        virtual BOOL WINAPI SendRequest(DWORD Type, CObjPtr<IPacketBuffer> Buffer, HANDLE DoneEvent = NULL);

        virtual BOOL WINAPI SendRespone(DWORD Type, PBYTE Data, DWORD DataLen, DWORD Id, HANDLE DoneEvent = NULL);

        virtual BOOL WINAPI SendRespone(DWORD Type, CObjPtr<IPacketBuffer> Buffer, DWORD Id, HANDLE DoneEvent = NULL);

        virtual BOOL WINAPI SendRequestWithRespone(DWORD Type, CObjPtr<IPacketBuffer> Buffer, CObjPtr<IPacketBuffer>* Reply, HANDLE DoneEvent);

        virtual VOID WINAPI CancelIO();

        virtual DWORD WINAPI GetSendBufferLength();

    private:
        static BOOL RecvThreadProc(CObjPtr<CObject> Parameter, HANDLE StopEvent);

        static void RecvThreadEndProc(CObjPtr<CObject> Parameter);

        static BOOL SendThreadProc(CObjPtr<CObject> Parameter, HANDLE StopEvent);

        static void SendThreadEndProc(CObjPtr<CObject> Parameter);

        void SendPacket(CObjPtr<IPacketBuffer> Buffer, HANDLE DoneEvent);

        CObjPtr<IThread>                     m_spRecvThread;
        CObjPtr<IThread>                     m_spSendThread;
        WORD                                 m_dwRequestId;
        std::map<DWORD, RequestPacketHandle> m_ReqPacketList;
        std::map<DWORD, RequestDataHandle>   m_ReqDataList;
        std::map<DWORD, WaitResponeNode*>    m_WaitRespNodeList;
        std::list<EndHandle>                 m_EndList;
        std::list<SendNode>                  m_SendList;
        HANDLE                               m_hSendEvent;
        CRITICAL_SECTION                     m_csRequestIdLock;
        CRITICAL_SECTION                     m_csRecvListLock;
        CRITICAL_SECTION                     m_csSendListLock;
        DWORD                                m_dwSendBufferLength;
    };
};

#endif
