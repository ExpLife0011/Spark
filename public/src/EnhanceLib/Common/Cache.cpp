#include "stdafx.h"
#include "Common/Cache.h"

CCache::CCache(HANDLE GetPacketEvent) : CBaseObject()
{
    InitializeCriticalSection(&m_csPacketListLock);
    m_hGetPacketEvent = GetPacketEvent;
    InitBuffer();
}

CCache::~CCache()
{
    std::list<BASE_PACKET_T*>::iterator Itertor;
    EnterCriticalSection(&m_csPacketListLock);
    for (Itertor = m_PacketList.begin(); Itertor != m_PacketList.end(); Itertor++)
    {
        free(*Itertor);
    }
    m_PacketList.clear();
    LeaveCriticalSection(&m_csPacketListLock);

    if (m_pBufferBase)
    {
        free(m_pBufferBase);
    }
    DeleteCriticalSection(&m_csPacketListLock);
}

BOOL CCache::InitBuffer()
{
    m_dwDataLength = 0;
    m_dwBufferLength = sizeof(BASE_PACKET_T);
    m_pBufferBase = (BYTE *)malloc(m_dwBufferLength);
    if (m_pBufferBase == NULL)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

BOOL CCache::Check()
{
    if (m_dwDataLength < m_dwBufferLength)
    {
        return TRUE;
    }

    if (m_dwBufferLength == sizeof(BASE_PACKET_T))
    {
        DWORD PacketLen;
        BYTE  *tmp;

        PacketLen = *(DWORD*)m_pBufferBase;
        if (PacketLen < sizeof(BASE_PACKET_T))
        {
            return FALSE;
        }

        tmp = (BYTE *)malloc(PacketLen);
        if (tmp == NULL)
        {
            return FALSE;
        }
        memcpy(tmp, m_pBufferBase, sizeof(BASE_PACKET_T));
        free(m_pBufferBase);

        m_pBufferBase = tmp;
        m_dwBufferLength = PacketLen;
        m_dwDataLength = sizeof(BASE_PACKET_T);
    }

    if (m_dwDataLength == m_dwBufferLength)
    {
        BASE_PACKET_T* pkg = (BASE_PACKET_T *)m_pBufferBase;
        EnterCriticalSection(&m_csPacketListLock);
       
        m_PacketList.push_back(pkg);
        LeaveCriticalSection(&m_csPacketListLock);

        if (m_hGetPacketEvent != NULL && m_hGetPacketEvent != INVALID_HANDLE_VALUE)
        {
            SetEvent(m_hGetPacketEvent);
        }

        return this->InitBuffer();
    }
    return TRUE;
}

BOOL CCache::AddData(BYTE *Data, DWORD Length)
{
    DWORD AddLength;
    BYTE *Pos = Data;

    while (Length > 0)
    {
        AddLength = Length < ((int)(m_dwBufferLength - m_dwDataLength)) ?
        Length : ((int)(m_dwBufferLength - m_dwDataLength));

        memcpy(m_pBufferBase + m_dwDataLength, Pos, AddLength);
        m_dwDataLength += AddLength;
        Length -= AddLength;
        Pos += AddLength;

        if (!Check())
        {
            return FALSE;
        }
    }
    return TRUE;
}

BASE_PACKET_T* CCache::GetPacket()
{
    BASE_PACKET_T* pkg;

    EnterCriticalSection(&m_csPacketListLock);
    if (!m_PacketList.empty())
    {
        pkg = m_PacketList.front();
        m_PacketList.pop_front();
    }
    else
    {
        pkg = NULL;
    }
    LeaveCriticalSection(&m_csPacketListLock);

    return pkg;
}
