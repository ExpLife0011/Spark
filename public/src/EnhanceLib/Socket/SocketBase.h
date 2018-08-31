#ifndef __SOCKET_BASE_H__
#define __SOCKET_BASE_H__

#include "Common/Communication.h"
#include "Common/Cache.h"
#include "Socket/ISocketBase.h"

namespace enlib
{
    class CSocketBase : public virtual CCommunication, public virtual ISocketBase
    {
    public:
        CSocketBase();

        ~CSocketBase();

        virtual void WINAPI Close();

        virtual VOID GetSrcPeer(CHAR* SrcAddress, DWORD BufferLen, WORD* SrcPort);

        virtual VOID GetDstPeer(CHAR* DstAddress, DWORD BufferLen, WORD* DstPort);

    protected:
        BOOL WINAPI Open();

        SOCKET m_socket;

        CHAR   m_szDstAddress[128];
        WORD   m_dwDstPort;

        CHAR   m_szSrcAddress[128];
        WORD   m_dwSrcPort;

    private:
        virtual CObjPtr<IPacketBuffer> RecvAPacket(HANDLE StopEvent);

        virtual BOOL SendAPacket(CObjPtr<IPacketBuffer> Buffer, HANDLE StopEvent);

        static void SocketClear(CObjPtr<ICommunication> param);

        BOOL            m_bAlive;

        CObjPtr<CCache> m_spCache;
    };
};

#endif