#include "StdAfx.h"
#include "SocketBase.h"
#include "SocketHelper.h"

using namespace enlib;

CSocketBase::CSocketBase() : CCommunication()
{
    ZeroMemory(m_szDstAddress, 128);
    m_dwDstPort = 0;

    ZeroMemory(m_szSrcAddress, 128);
    m_dwSrcPort = 0;

    m_spCache = CreateICacheInstance(NULL);
    m_bAlive = FALSE;

    m_socket = INVALID_SOCKET;
}

CSocketBase::~CSocketBase()
{

}

BOOL WINAPI CSocketBase::Open()
{
    m_bAlive = TRUE;

    //set tcp no delay
    int flag = 1;
    setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

    //set non block mode
#ifdef WIN32
    flag = 1;
    ioctlsocket(m_socket, FIONBIO, (unsigned long *)&flag);
#else
    ioctl(m_socket, FIONBIO, 0);
#endif

    RegisterEndHandle(CSocketBase::SocketClear);
    StartCommunication();

    return TRUE;
}

void WINAPI CSocketBase::Close()
{
    m_bAlive = FALSE;
    StopCommunication();

    if (m_socket != INVALID_SOCKET)
    {
        shutdown(m_socket, SD_BOTH);
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

CObjPtr<IPacketBuffer> CSocketBase::RecvAPacket(HANDLE StopEvent)
{
    do
    {
        //read cache first
        BASE_PACKET_T* Packet;
        Packet = m_spCache->GetPacket();
        //if cache has a packet, return packet
        if (Packet != NULL)
        {
            CObjPtr<IPacketBuffer> Buffer = CreateIBufferInstanceEx((BYTE*)Packet->Data, Packet->Length - sizeof(BASE_PACKET_T));
            free(Packet);
            return Buffer;
        }

        //if cache not has a packet, read 4k byte to cache
        BYTE data[4096];
        DWORD read = 0;
        BOOL Ret = SocketRead(m_socket, data, 4096, &read, StopEvent);

        //read fail, stop recv
        if (Ret == FALSE)
        {
            return NULL;
        }

        m_spCache->AddData(data, read);

    } while (TRUE);

    return NULL;
}

BOOL CSocketBase::SendAPacket(CObjPtr<IPacketBuffer> Buffer, HANDLE StopEvent)
{
    if (!Buffer->DataPush(sizeof(BASE_PACKET_T)))
    {
        return FALSE;
    }

    BASE_PACKET_T* Packet = (BASE_PACKET_T*)Buffer->GetData();

    Packet->Length = Buffer->GetBufferLength();
    Packet->Type = 0;


    DWORD Len = Packet->Length;
    PBYTE Offset = Buffer->GetData();

    while (Len > 0)
    {
        DWORD Write = 0;
        if (SocketWrite(m_socket, Offset, Len, &Write, StopEvent))
        {
            Offset += Write;
            Len -= Write;
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}

void CSocketBase::SocketClear(CObjPtr<ICommunication> param)
{
    CObjPtr<CSocketBase> spSocket = NULL;
    spSocket = param;

    if (spSocket->m_bAlive)
    {
        spSocket->StopCommunication();

        if (spSocket->m_socket != INVALID_SOCKET)
        {
            shutdown(spSocket->m_socket, SD_BOTH);
            closesocket(spSocket->m_socket);
            spSocket->m_socket = INVALID_SOCKET;
        }

        spSocket->m_bAlive = FALSE;
    }
}

VOID CSocketBase::GetSrcPeer(CHAR* SrcAddress, DWORD BufferLen, WORD* SrcPort)
{
    strncpy(SrcAddress, m_szSrcAddress, BufferLen);

    *SrcPort = m_dwSrcPort;

    return;
}

VOID CSocketBase::GetDstPeer(CHAR* DstAddress, DWORD BufferLen, WORD* DstPort)
{
    strncpy(DstAddress, m_szDstAddress, BufferLen);

    *DstPort = m_dwDstPort;

    return;
}

