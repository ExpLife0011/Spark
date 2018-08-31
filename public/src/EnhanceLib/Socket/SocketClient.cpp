#include "StdAfx.h"
#include "SocketClient.h"
#include "SocketHelper.h"

using namespace enlib;

CSocketClient::CSocketClient(const CHAR *address, WORD port) : CSocketBase()
{
    strncpy(m_szDstAddress, address, 128);
    m_dwDstPort = port;
}

CSocketClient::~CSocketClient()
{

}

BOOL WINAPI CSocketClient::Connect()
{
    struct sockaddr_in  DstAddress;
    struct sockaddr_in  SrcAddress;

    int Ret;
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        return FALSE;
    }

    memset(&DstAddress, 0, sizeof(sockaddr_in));
    DstAddress.sin_family = AF_INET;
    DstAddress.sin_port = htons(m_dwDstPort);
    DstAddress.sin_addr.s_addr = inet_addr(m_szDstAddress);

    Ret = connect(sock, (struct sockaddr *)&DstAddress, sizeof(sockaddr_in));
    if (Ret < 0)
    {
        closesocket(sock);
        return FALSE;
    }

    m_socket = sock;

    int len = sizeof(struct sockaddr_in);
    if (getpeername(m_socket, (struct sockaddr*)&SrcAddress, &len) >= 0)
    {
        m_dwSrcPort = SrcAddress.sin_port;
        strcpy(m_szSrcAddress, inet_ntoa(SrcAddress.sin_addr));
    }

    Open();

    return TRUE;
}
