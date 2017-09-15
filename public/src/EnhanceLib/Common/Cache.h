#pragma once

#ifndef __CACHE_H__
#define __CACHE_H__

#include "Common/ICache.h"
#ifndef WIN32
#include <winpr/synch.h>
#endif

#include "Base/BaseObject.h"

class CCache : public ICache
{
public:
    CCache(HANDLE GetPacketEvent);

    virtual ~CCache();

    virtual BOOL WINAPI AddData(BYTE *Data, DWORD Length);

    virtual BASE_PACKET_T* WINAPI GetPacket();
private:
    BOOL InitBuffer();

    BOOL Check();

    BYTE*                     m_pBufferBase;
    DWORD                     m_dwBufferLength;
    DWORD                     m_dwDataLength;
    HANDLE                    m_hGetPacketEvent;
    std::list<BASE_PACKET_T*> m_PacketList;
    CRITICAL_SECTION          m_csPacketListLock;
};

#endif
