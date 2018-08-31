#include "stdafx.h"
#include "Common/Communication.h"
#include "Common/Buffer.h"

using namespace enlib;

CCommunication::CCommunication() : CObject()
{
    m_dwRequestId = 0;
    m_dwSendBufferLength = 0;

    m_spRecvThread = NULL;
    m_spSendThread = NULL;
    m_spRecvThread = CreateIThreadInstanceEx(CCommunication::RecvThreadProc, CCommunication::RecvThreadEndProc);
    m_spSendThread = CreateIThreadInstanceEx(CCommunication::SendThreadProc, CCommunication::SendThreadEndProc);

    m_hSendEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    InitializeCriticalSection(&m_csSendListLock);
    InitializeCriticalSection(&m_csRecvListLock);
    InitializeCriticalSection(&m_csRequestIdLock);
}

CCommunication::~CCommunication()
{
    StopCommunication();

    CancelIO();

    m_spRecvThread = NULL;
    m_spSendThread = NULL;

    CloseHandle(m_hSendEvent);

    DeleteCriticalSection(&m_csSendListLock);
    DeleteCriticalSection(&m_csRecvListLock);
    DeleteCriticalSection(&m_csRequestIdLock);
}

BOOL CCommunication::StartCommunication()
{
    CObjPtr<CObject> spParam = NULL;
    spParam = this;
    if (!m_spSendThread->IsMainThreadRunning())
    {
        m_spSendThread->StartMainThread(spParam);
    }

    if (!m_spRecvThread->IsMainThreadRunning())
    {
        m_spRecvThread->StartMainThread(spParam);
    }

    return TRUE;
}

VOID CCommunication::StopCommunication()
{
    if (m_spSendThread->IsMainThreadRunning())
    {
        m_spSendThread->StopMainThread();

        while (m_spSendThread->IsMainThreadRunning())
        {
            Sleep(100);
        }
    }

    if (m_spRecvThread->IsMainThreadRunning())
    {
        m_spRecvThread->StopMainThread();
    }
}

DWORD CCommunication::GetSendBufferLength()
{
    DWORD Ret;
    EnterCriticalSection(&m_csSendListLock);
    Ret = m_dwSendBufferLength;
    LeaveCriticalSection(&m_csSendListLock);

    return Ret;
}

BOOL CCommunication::RegisterRequestHandle(DWORD Type, RequestDataHandle Func)
{
    EnterCriticalSection(&m_csRecvListLock);

    if (m_ReqDataList.find(Type) != m_ReqDataList.end()
        || m_ReqPacketList.find(Type) != m_ReqPacketList.end())
    {
        //重复注册返回失败
        LeaveCriticalSection(&m_csRecvListLock);
        return FALSE;
    }
    else
    {
        m_ReqDataList[Type] = Func;
    }
    LeaveCriticalSection(&m_csRecvListLock);

    return TRUE;
}

BOOL CCommunication::RegisterRequestHandle(DWORD Type, RequestPacketHandle Func)
{
    EnterCriticalSection(&m_csRecvListLock);

    if (m_ReqDataList.find(Type) != m_ReqDataList.end()
        || m_ReqPacketList.find(Type) != m_ReqPacketList.end())
    {
        LeaveCriticalSection(&m_csRecvListLock);
        return FALSE;
    }
    else
    {
        m_ReqPacketList[Type] = Func;
    }
    LeaveCriticalSection(&m_csRecvListLock);

    return TRUE;
}

BOOL CCommunication::RegisterEndHandle(EndHandle Func)
{
    EnterCriticalSection(&m_csRecvListLock);
    m_EndList.push_back(Func);
    LeaveCriticalSection(&m_csRecvListLock);

    return TRUE;
}

VOID CCommunication::CancelIO()
{
    std::list<SendNode>::iterator EndIterator;
    std::map<DWORD, WaitResponeNode*>::iterator WaitRespIterator;
    WaitResponeNode* RecvNode = NULL;

    EnterCriticalSection(&m_csSendListLock);

    for (EndIterator = m_SendList.begin(); EndIterator != m_SendList.end(); EndIterator++)
    {
        if (EndIterator->Event)
        {
            SetEvent(EndIterator->Event);
        }

        if (EndIterator->Buffer)
        {
            DataHdr* Block = (DataHdr*)EndIterator->Buffer->GetData();
            m_dwSendBufferLength -= Block->Len;
            EndIterator->Buffer = NULL;
        }
    }

    m_SendList.clear();

    LeaveCriticalSection(&m_csSendListLock);

    EnterCriticalSection(&m_csRecvListLock);

    for (WaitRespIterator = m_WaitRespNodeList.begin(); WaitRespIterator != m_WaitRespNodeList.end(); WaitRespIterator++)
    {
        RecvNode = WaitRespIterator->second;
        *RecvNode->Buffer = NULL;

        if (RecvNode->Event)
        {
            SetEvent(RecvNode->Event);
        }

        free(RecvNode);
    }

    m_WaitRespNodeList.clear();

    LeaveCriticalSection(&m_csRecvListLock);
}

void CCommunication::SendPacket(CObjPtr<IPacketBuffer> Buffer, HANDLE DoneEvent)
{
    SendNode Node;

    DataHdr *Header = (DataHdr *)Buffer->GetData();

    Node.Event = DoneEvent;

    if (Node.Event)
    {
        ResetEvent(Node.Event);
    }

    Node.Buffer = Buffer;

    //插入队列
    EnterCriticalSection(&m_csSendListLock);
    m_dwSendBufferLength += Header->Len;
    m_SendList.push_back(Node);
    SetEvent(m_hSendEvent);
    LeaveCriticalSection(&m_csSendListLock);

    return;
}

BOOL CCommunication::SendRequestWithRespone(DWORD Type, CObjPtr<IPacketBuffer> Buffer, CObjPtr<IPacketBuffer>* Reply, HANDLE DoneEvent)
{
    WaitResponeNode* Node = NULL;

    DWORD Len = Buffer->GetBufferLength();

    if (DoneEvent == NULL || Reply == NULL)
    {
        return FALSE;
    }

    if (!Buffer->DataPush(sizeof(DataHdr)))
    {
        return FALSE;
    }

    DataHdr *Header = (DataHdr *)Buffer->GetData();

    EnterCriticalSection(&m_csRequestIdLock);
    Header->Id = m_dwRequestId++;
    LeaveCriticalSection(&m_csRequestIdLock);
    Header->Flags = HDR_DIRECT_REQ;
    Header->Len = Len;
    Header->Type = Type;

    Node = (WaitResponeNode*)malloc(sizeof(WaitResponeNode));

    Node->Event = DoneEvent;
    Node->Id = Header->Id;
    Node->Buffer = Reply;

    EnterCriticalSection(&m_csRecvListLock);
    if (m_WaitRespNodeList.find(Node->Id) != m_WaitRespNodeList.end())
    {
        free(Node);
        LeaveCriticalSection(&m_csRecvListLock);
        return FALSE;
    }
    else
    {
        m_WaitRespNodeList[Node->Id] = Node;
    }
    LeaveCriticalSection(&m_csRecvListLock);

    SendPacket(Buffer, NULL);

    return TRUE;
}

BOOL CCommunication::SendRequest(DWORD Type, PBYTE Data, DWORD DataLen, HANDLE DoneEvent)
{
    CObjPtr<IPacketBuffer> Packet = CreateIBufferInstanceEx(Data, DataLen);

    BOOL Ret = SendRequest(Type, Packet, DoneEvent);

    return Ret;
}

BOOL CCommunication::SendRequest(DWORD Type, CObjPtr<IPacketBuffer> Buffer, HANDLE DoneEvent)
{
    DWORD Len = Buffer->GetBufferLength();

    if (!Buffer->DataPush(sizeof(DataHdr)))
    {
        return FALSE;
    }

    DataHdr *Header = (DataHdr *)Buffer->GetData();

    EnterCriticalSection(&m_csRequestIdLock);
    Header->Id = m_dwRequestId++;
    LeaveCriticalSection(&m_csRequestIdLock);
    Header->Flags = HDR_DIRECT_REQ;
    Header->Len = Len;
    Header->Type = Type;

    SendPacket(Buffer, DoneEvent);

    return TRUE;
}

BOOL CCommunication::SendRespone(DWORD Type, PBYTE Data, DWORD DataLen, DWORD Id, HANDLE DoneEvent)
{
    CObjPtr<IPacketBuffer> Packet = CreateIBufferInstanceEx(Data, DataLen);

    BOOL Ret = SendRespone(Type, Packet, Id, DoneEvent);

    return Ret;
}

BOOL CCommunication::SendRespone(DWORD Type, CObjPtr<IPacketBuffer> Buffer, DWORD Id, HANDLE DoneEvent)
{
    DWORD Len = Buffer->GetBufferLength();

    if (!Buffer->DataPush(sizeof(DataHdr)))
    {
        return FALSE;
    }

    DataHdr *Header = (DataHdr *)Buffer->GetData();

    Header->Id = Id;
    Header->Flags = HDR_DIRECT_RESP;
    Header->Len = Len;
    Header->Type = Type;

    SendPacket(Buffer, DoneEvent);

    return TRUE;
}

BOOL CCommunication::RecvThreadProc(CObjPtr<CObject> Parameter, HANDLE StopEvent)
{
    BASE_PACKET_T *Package = NULL;
    CObjPtr<IPacketBuffer> Buffer = NULL;
    DataHdr  *Header = NULL;
    CObjPtr<CCommunication> spCommunicate = NULL;
    CObjPtr<ICommunication> spICommunicate = NULL;
    WaitResponeNode* RecvNode = NULL;
    RequestPacketHandle PacketFunc;
    RequestDataHandle DataFunc;
    std::map<DWORD, WaitResponeNode*>::iterator WaitRespIterator;
    std::map<DWORD, RequestPacketHandle>::iterator ReqPacketIterator;
    std::map<DWORD, RequestDataHandle>::iterator ReqDataIterator;
    std::list<EndHandle>::iterator end_itor;

    spCommunicate = Parameter;
    spICommunicate = spCommunicate;

    Buffer = spCommunicate->RecvAPacket(StopEvent);
    if (Buffer == NULL)
    {
        return FALSE;
    }

    Package = (BASE_PACKET_T*)Buffer->GetData();

    if (!Buffer->DataPull(sizeof(BASE_PACKET_T)))
    {
        return FALSE;
    }

    Header = (DataHdr *)Buffer->GetData();

    if ((Header->Flags & HDR_DIRECT_MASK) == HDR_DIRECT_REQ)
    {
        EnterCriticalSection(&spCommunicate->m_csRecvListLock);
        ReqPacketIterator = spCommunicate->m_ReqPacketList.find(Package->Type);
        ReqDataIterator = spCommunicate->m_ReqDataList.find(Package->Type);

        if (ReqPacketIterator != spCommunicate->m_ReqPacketList.end())
        {
            PacketFunc = ReqPacketIterator->second;
            if (Buffer->DataPull(sizeof(DataHdr)))
            {
                if (PacketFunc)
                {
                    PacketFunc(Buffer, Header->Id, spICommunicate);
                }
            }
        }
        else if (ReqDataIterator != spCommunicate->m_ReqDataList.end())
        {
            DataFunc = ReqDataIterator->second;
            if (Buffer->DataPull(sizeof(DataHdr)))
            {
                DataFunc(Buffer->GetData(), Buffer->GetBufferLength(), Header->Id, spICommunicate);
            }
        }
        LeaveCriticalSection(&spCommunicate->m_csRecvListLock);
    }
    else if ((Header->Flags & HDR_DIRECT_MASK) == HDR_DIRECT_RESP)
    {
        EnterCriticalSection(&spCommunicate->m_csRecvListLock);
        WaitRespIterator = spCommunicate->m_WaitRespNodeList.find(Header->Id);
        if (WaitRespIterator != spCommunicate->m_WaitRespNodeList.end())
        {
            RecvNode = WaitRespIterator->second;
            if (Buffer->DataPull(sizeof(DataHdr)))
            {
                *(RecvNode->Buffer) = Buffer;
            }
            else
            {
                *(RecvNode->Buffer) = NULL;
            }
            spCommunicate->m_WaitRespNodeList.erase(WaitRespIterator);

            if (RecvNode->Event)
            {
                SetEvent(RecvNode->Event);
            }
        }
        LeaveCriticalSection(&spCommunicate->m_csRecvListLock);
    }

    return TRUE;
}

void CCommunication::RecvThreadEndProc(CObjPtr<CObject> Parameter)
{
    CObjPtr<CCommunication> spCommunicate = NULL;
    CObjPtr<ICommunication> spICommunicate = NULL;
    std::map<DWORD, WaitResponeNode*>::iterator WaitRespIterator;
    std::list<EndHandle>::iterator EndIterator;
    WaitResponeNode* RecvNode = NULL;

    spCommunicate = Parameter;
    spICommunicate = spCommunicate;

    EnterCriticalSection(&spCommunicate->m_csRecvListLock);
    for (WaitRespIterator = spCommunicate->m_WaitRespNodeList.begin(); WaitRespIterator != spCommunicate->m_WaitRespNodeList.end(); WaitRespIterator++)
    {
        RecvNode = WaitRespIterator->second;
        *RecvNode->Buffer = NULL;

        if (RecvNode->Event)
        {
            SetEvent(RecvNode->Event);
        }

        free(RecvNode);
    }

    spCommunicate->m_WaitRespNodeList.clear();

    for (EndIterator = spCommunicate->m_EndList.begin(); EndIterator != spCommunicate->m_EndList.end(); EndIterator++)
    {
        if ((*EndIterator))
        {
            (*EndIterator)(spICommunicate);
        }
    }

    LeaveCriticalSection(&spCommunicate->m_csRecvListLock);
}

BOOL CCommunication::SendThreadProc(CObjPtr<CObject> Parameter, HANDLE StopEvent)
{
    BASE_PACKET_T *Packet;
    CObjPtr<CCommunication> spCommunicate = NULL;

    spCommunicate = Parameter;

    HANDLE hEvents[2] = {spCommunicate->m_hSendEvent, StopEvent };
    DWORD ret;

    ret = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

    switch (ret)
    {
    case WAIT_OBJECT_0:

        EnterCriticalSection(&spCommunicate->m_csSendListLock);
        if (!(spCommunicate->m_SendList.empty()))
        {
            DWORD Ret;
            SendNode Node = spCommunicate->m_SendList.front();
            CObjPtr<IPacketBuffer> Buffer = Node.Buffer;
            DataHdr* Block = (DataHdr*)Buffer->GetData();

            spCommunicate->m_dwSendBufferLength -= Block->Len;
            spCommunicate->m_SendList.pop_front();

            LeaveCriticalSection(&spCommunicate->m_csSendListLock);

            if (!Buffer->DataPush(sizeof(BASE_PACKET_T)))
            {
                if (Node.Event)
                {
                    SetEvent(Node.Event);
                }

                return FALSE;
            }

            Packet = (BASE_PACKET_T*)Buffer->GetData();
            Packet->Type = Block->Type;
            Packet->Length = sizeof(BASE_PACKET_T) + sizeof(DataHdr) + Block->Len;

            Ret = spCommunicate->SendAPacket(Buffer, StopEvent);

            if (Node.Event)
            {
                SetEvent(Node.Event);
            }

            if (Ret == FALSE)
            {
                return FALSE;
            }
        }
        else
        {
            ResetEvent(spCommunicate->m_hSendEvent);
            LeaveCriticalSection(&spCommunicate->m_csSendListLock);
        }

        break;
    case (WAIT_OBJECT_0 + 1) :
        return FALSE;
    case WAIT_TIMEOUT:
        break;
    default:
        return FALSE;
    }

    return TRUE;
}

void CCommunication::SendThreadEndProc(CObjPtr<CObject> Parameter)
{
    std::list<SendNode>::iterator EndIterator;
    CObjPtr<CCommunication> spCommunicate = NULL;

    spCommunicate = Parameter;

    EnterCriticalSection(&spCommunicate->m_csSendListLock);
    for (EndIterator = spCommunicate->m_SendList.begin(); EndIterator != spCommunicate->m_SendList.end(); EndIterator++)
    {
        if (EndIterator->Event)
        {
            SetEvent(EndIterator->Event);
        }

        if (EndIterator->Buffer)
        {
            DataHdr* Block = (DataHdr*)EndIterator->Buffer->GetData();
            spCommunicate->m_dwSendBufferLength -= Block->Len;
            EndIterator->Buffer = NULL;
        }
    }

    spCommunicate->m_SendList.clear();

    LeaveCriticalSection(&spCommunicate->m_csSendListLock);

    return;
}


