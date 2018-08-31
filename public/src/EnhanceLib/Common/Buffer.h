#pragma once

#ifndef __ENLIB_PKT_BUFFER_H__
#define __ENLIB_PKT_BUFFER_H__

#include "Common/IBuffer.h"
#include "Base/BaseObject.h"

namespace enlib
{
    class CPacketBuffer : public IPacketBuffer
    {
    public:
        CPacketBuffer(DWORD Length);
        CPacketBuffer(BYTE* Buffer, DWORD Length);
        CPacketBuffer(BYTE* Buffer, DWORD Length, DWORD HeadRoom, DWORD TailRoom);

        virtual ~CPacketBuffer();

        virtual BOOL WINAPI DataPush(DWORD Length);
        virtual BOOL WINAPI DataPull(DWORD Length);
        virtual BOOL WINAPI TailPush(DWORD Length);
        virtual BOOL WINAPI TailPull(DWORD Length);

        virtual BYTE* WINAPI GetData();
        virtual BYTE* WINAPI GetTail();
        virtual DWORD WINAPI GetBufferLength();

    protected:
        BYTE* m_pHead;
        BYTE* m_pData;
        BYTE* m_pTail;
        BYTE* m_pEnd;
    };
};

#endif

