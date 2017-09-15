/**
 * @file     Buffer.cpp
 * @author   4680414@qq.com
 * @date     2017/1/25
 * @version  1.0
 * @brief    缓冲报文类源文件
 */
#include <string.h>
#include "Common/Buffer.h"

CPacketBuffer::CPacketBuffer(DWORD Length) : CBaseObject()
{
    m_pHead = (BYTE*)malloc(PACKET_BUFFER_HEADROOM_DEFAULT + Length + PACKET_BUFFER_TAILROOM_DEFAULT);
    m_pData = m_pHead + PACKET_BUFFER_HEADROOM_DEFAULT;
    m_pTail = m_pData + Length;
    m_pEnd = m_pTail + PACKET_BUFFER_TAILROOM_DEFAULT;
}

CPacketBuffer::CPacketBuffer(BYTE* Buffer, DWORD Length) : CBaseObject()
{
    m_pHead = (BYTE*)malloc(PACKET_BUFFER_HEADROOM_DEFAULT + Length + PACKET_BUFFER_TAILROOM_DEFAULT);
    m_pData = m_pHead + PACKET_BUFFER_HEADROOM_DEFAULT;
    m_pTail = m_pData + Length;
    m_pEnd = m_pTail + PACKET_BUFFER_TAILROOM_DEFAULT;

    memcpy(m_pData, Buffer, Length);
}

CPacketBuffer::CPacketBuffer(BYTE* Buffer, DWORD Length, DWORD HeadRoom, DWORD TailRoom) : CBaseObject()
{
    m_pHead = (BYTE*)malloc(HeadRoom + Length + TailRoom);
    m_pData = m_pHead + HeadRoom;
    m_pTail = m_pData + Length;
    m_pEnd = m_pTail + TailRoom;

    memcpy(m_pData, Buffer, Length);
}

CPacketBuffer::~CPacketBuffer()
{
    free(m_pHead);
}

BOOL CPacketBuffer::DataPush(DWORD Length)
{
    //向前到达缓冲区顶端，说明头部空间不足
    if (m_pData - Length < m_pHead)
    {
        //返回失败
        return FALSE;
    }

    m_pData -= Length;
    return TRUE;
}

BOOL CPacketBuffer::DataPull(DWORD Length)
{
    //向后到达数据区尾部，说明尾部空间不足
    if (m_pData + Length > m_pTail)
    {
        //返回失败
        return FALSE;
    }

    m_pData += Length;
    return TRUE;
}

BYTE* CPacketBuffer::GetData()
{
    return m_pData;
}

BOOL CPacketBuffer::TailPush(DWORD Length)
{
    //向前达到数据区头部，说明头部空间不足
    if (m_pTail - Length < m_pData)
    {
        //返回失败
        return FALSE;
    }

    m_pTail -= Length;
    return TRUE;
}

BOOL CPacketBuffer::TailPull(DWORD Length)
{
    //向后达到缓冲区尾部，说明尾部空间不足
    if (m_pTail + Length > m_pEnd)
    {
        //返回失败
        return FALSE;
    }

    m_pTail += Length;
    return TRUE;
}

BYTE* CPacketBuffer::GetTail()
{
    return m_pTail;
}

DWORD CPacketBuffer::GetBufferLength()
{
    if (m_pTail < m_pData)
    {
        return 0;
    }

    return (DWORD)(m_pTail - m_pData);
}