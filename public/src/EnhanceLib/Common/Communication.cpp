#include "stdafx.h"
#include "Common/Communication.h"
#include "Common/Buffer.h"

CCommunication::CCommunication() : CBaseObject()
{
    m_dwRequestId = 0;
    m_dwSendBufferLength = 0;

	m_pRecvThread = NULL;
	m_pSendThread = NULL;
    
    AddRef();
    m_pRecvThread = CreateIThreadInstanceEx(CCommunication::RecvThreadProc, (LPVOID)this, CCommunication::RecvThreadEndProc, (LPVOID)this);

    AddRef();
    m_pSendThread = CreateIThreadInstanceEx(CCommunication::SendThreadProc, (LPVOID)this, CCommunication::SendThreadEndProc, (LPVOID)this);

    m_hSendEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    InitializeCriticalSection(&m_csSendListLock);
    InitializeCriticalSection(&m_csRecvListLock);
    InitializeCriticalSection(&m_csRequestIdLock);
}

CCommunication::~CCommunication()
{
    StopCommunication();

    m_pRecvThread->Release();
    m_pSendThread->Release();
	m_pRecvThread = NULL;
	m_pSendThread = NULL;

	CloseHandle(m_hSendEvent);

    DeleteCriticalSection(&m_csSendListLock);
    DeleteCriticalSection(&m_csRecvListLock);
    DeleteCriticalSection(&m_csRequestIdLock);
}

BOOL CCommunication::StartCommunication()
{
    m_pRecvThread->StartMainThread();
    m_pSendThread->StartMainThread();

    return TRUE;
}

VOID CCommunication::StopCommunication()
{
    if (m_pRecvThread->IsMainThreadRunning())
    {
        m_pRecvThread->StopMainThread();
    }

    if (m_pSendThread->IsMainThreadRunning())
    {
        m_pSendThread->StopMainThread();
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

BOOL CCommunication::RegisterRequestHandle(DWORD Type, RequestPacketHandle Func)
{
    EnterCriticalSection(&m_csRecvListLock);

    if (m_ReqPacketList.find(Type) != m_ReqPacketList.end())
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
            EndIterator->Buffer->Release();
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
    }

    m_WaitRespNodeList.clear();

    LeaveCriticalSection(&m_csRecvListLock);
}

void CCommunication::SendPacket(IPacketBuffer* Buffer, HANDLE DoneEvent)
{
    SendNode Node;

	DataHdr *Header = (DataHdr *)Buffer->GetData();

    Node.Event = DoneEvent;

    if (Node.Event)
    {
        ResetEvent(Node.Event);
    }

    Buffer->AddRef();
    Node.Buffer = Buffer;

    //插入队列
    EnterCriticalSection(&m_csSendListLock);
    m_dwSendBufferLength += Header->Len;
    m_SendList.push_back(Node);
    SetEvent(m_hSendEvent);
	LeaveCriticalSection(&m_csSendListLock);

    return;
}

BOOL CCommunication::SendRequestWithRespone(DWORD Type, IPacketBuffer* Buffer, IPacketBuffer** Reply, HANDLE DoneEvent)
{
    WaitResponeNode Node;

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

    Node.Event = DoneEvent;
    Node.Id = Header->Id;
    Node.Buffer = Reply;

    EnterCriticalSection(&m_csRecvListLock);
    if (m_WaitRespNodeList.find(Node.Id) != m_WaitRespNodeList.end())
    {
        LeaveCriticalSection(&m_csRecvListLock);
        return FALSE;
    }
    else
    {
        m_WaitRespNodeList[Node.Id] = &Node;
    }
    LeaveCriticalSection(&m_csRecvListLock);

    SendPacket(Buffer, NULL);

    return TRUE;
}

BOOL CCommunication::SendRequest(DWORD Type, IPacketBuffer* Buffer, HANDLE DoneEvent)
{
    DWORD Len = Buffer->GetBufferLength();
 
	if (!Buffer->DataPush(sizeof(DataHdr)))
	{
        return FALSE;
	}

	DataHdr *Header = (DataHdr *)Buffer->GetData();

    EnterCriticalSection(&m_csRequestIdLock);
    Header->Id    = m_dwRequestId++;
    LeaveCriticalSection(&m_csRequestIdLock);
    Header->Flags = HDR_DIRECT_REQ;
    Header->Len   = Len;
    Header->Type  = Type;

    SendPacket(Buffer, DoneEvent);

    return TRUE;
}

BOOL CCommunication::SendRespone(DWORD Type, IPacketBuffer* Buffer, WORD Id, HANDLE DoneEvent)
{
    DWORD Len = Buffer->GetBufferLength();

	if (!Buffer->DataPush(sizeof(DataHdr)))
	{
        return FALSE;
	}

	DataHdr *Header = (DataHdr *)Buffer->GetData();

    Header->Id    = Id;
    Header->Flags = HDR_DIRECT_RESP;
    Header->Len   = Len;
    Header->Type  = Type;

    SendPacket(Buffer, DoneEvent);

    return TRUE;
}

BOOL CCommunication::RecvThreadProc(LPVOID Parameter, HANDLE StopEvent)
{
    BASE_PACKET_T *Package = NULL;
	IPacketBuffer* Buffer;
    DataHdr  *Header = NULL;
    CCommunication *Communicate = (CCommunication *)Parameter;
    WaitResponeNode* RecvNode = NULL;
    RequestPacketHandle PacketFunc;
    std::map<DWORD, WaitResponeNode*>::iterator WaitRespIterator;
    std::map<DWORD, RequestPacketHandle>::iterator ReqPacketIterator;
    std::list<EndHandle>::iterator end_itor;

    Buffer = Communicate->RecvAPacket(StopEvent);
    if (Buffer == NULL)
    {
        return FALSE;
    }

	Package = (BASE_PACKET_T*)Buffer->GetData();
   
	if (!Buffer->DataPull(sizeof(BASE_PACKET_T)))
	{
        Buffer->Release();
		return FALSE;
	}

	Header = (DataHdr *)Buffer->GetData();
    
    if ((Header->Flags & HDR_DIRECT_MASK) == HDR_DIRECT_REQ) 
    {
        EnterCriticalSection(&Communicate->m_csRecvListLock);
        ReqPacketIterator = Communicate->m_ReqPacketList.find(Package->Type);
        if (ReqPacketIterator != Communicate->m_ReqPacketList.end()) 
        {
            PacketFunc = ReqPacketIterator->second;
			if (Buffer->DataPull(sizeof(DataHdr)))
			{
				if (PacketFunc)
				{
					PacketFunc(Buffer, Header->Id, (ICommunication*)Communicate);
				}
			}
        }
        LeaveCriticalSection(&Communicate->m_csRecvListLock);
    }
    else if ((Header->Flags & HDR_DIRECT_MASK) == HDR_DIRECT_RESP)
    {
        EnterCriticalSection(&Communicate->m_csRecvListLock);
        WaitRespIterator = Communicate->m_WaitRespNodeList.find(Header->Id);
        if (WaitRespIterator != Communicate->m_WaitRespNodeList.end()) 
        {
            RecvNode = WaitRespIterator->second;
			if (Buffer->DataPull(sizeof(DataHdr)))
			{
                Buffer->AddRef();
				*(RecvNode->Buffer) = Buffer;
			}
			else
			{
				*(RecvNode->Buffer) = NULL;
			}
			Communicate->m_WaitRespNodeList.erase(WaitRespIterator);

            if (RecvNode->Event)
            {
                SetEvent(RecvNode->Event);
            }
        }
        LeaveCriticalSection(&Communicate->m_csRecvListLock);
    }

    Buffer->Release();

    return TRUE;
}

void CCommunication::RecvThreadEndProc(LPVOID Parameter)
{
    CCommunication *Communicate = (CCommunication *)Parameter;
    std::map<DWORD, WaitResponeNode*>::iterator WaitRespIterator;
    std::list<EndHandle>::iterator EndIterator;
    WaitResponeNode* RecvNode = NULL;

    EnterCriticalSection(&Communicate->m_csRecvListLock);
    for (WaitRespIterator = Communicate->m_WaitRespNodeList.begin(); WaitRespIterator != Communicate->m_WaitRespNodeList.end(); WaitRespIterator++)
    {
        RecvNode = WaitRespIterator->second;
        *RecvNode->Buffer = NULL;

        if (RecvNode->Event)
        {
            SetEvent(RecvNode->Event);
        }
    }

    Communicate->m_WaitRespNodeList.clear();

    for (EndIterator = Communicate->m_EndList.begin(); EndIterator != Communicate->m_EndList.end(); EndIterator++)
    {
		if ((*EndIterator))
		{
			(*EndIterator)(Communicate);
		}
    }

    LeaveCriticalSection(&Communicate->m_csRecvListLock);

    Communicate->Release();
}

BOOL CCommunication::SendThreadProc(LPVOID Parameter, HANDLE StopEvent)
{
    BASE_PACKET_T *Packet;
    CCommunication *Communicate = (CCommunication *)Parameter;

	HANDLE hEvents[2] = {Communicate->m_hSendEvent, StopEvent};
	DWORD ret;
	
    ret = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

	switch (ret)
	{
	case WAIT_OBJECT_0:		

        EnterCriticalSection(&Communicate->m_csSendListLock);
        if (!Communicate->m_SendList.empty())
		{
            DWORD Ret;
            SendNode Node = Communicate->m_SendList.front();
            IPacketBuffer* Buffer = Node.Buffer;
            DataHdr* Block = (DataHdr*)Buffer->GetData();

            Communicate->m_dwSendBufferLength -= Block->Len;
			Communicate->m_SendList.pop_front();
            
			LeaveCriticalSection(&Communicate->m_csSendListLock);

			if (!Buffer->DataPush(sizeof(BASE_PACKET_T)))
			{
                if (Node.Event)
                {
                    SetEvent(Node.Event);
                }

                Buffer->Release();
                return FALSE;
			}

			Packet = (BASE_PACKET_T*)Buffer->GetData();
			Packet->Type = Block->Type;
            Packet->Length  = sizeof(BASE_PACKET_T) + sizeof(DataHdr) + Block->Len;
            
            Ret = Communicate->SendAPacket(Buffer, StopEvent);

            if (Node.Event)
            {
                SetEvent(Node.Event);
            }

            Buffer->Release();

            if (Ret == FALSE)
            {
                return FALSE;
            }
		}
		else
		{
			ResetEvent(Communicate->m_hSendEvent);
            LeaveCriticalSection(&Communicate->m_csSendListLock);
		}
				
		break;
	case (WAIT_OBJECT_0 + 1):
        return FALSE;
	case WAIT_TIMEOUT:
		break;
    default:
        return FALSE;
	}
    
    return TRUE;
}

void CCommunication::SendThreadEndProc(LPVOID Parameter)
{
    CCommunication *Communicate = (CCommunication *)Parameter;
    std::list<SendNode>::iterator EndIterator;
   
    EnterCriticalSection(&Communicate->m_csSendListLock);
    for (EndIterator = Communicate->m_SendList.begin(); EndIterator != Communicate->m_SendList.end(); EndIterator++)
    {
        if (EndIterator->Event)
        {
            SetEvent(EndIterator->Event);
        }

        if (EndIterator->Buffer)
        {
            DataHdr* Block = (DataHdr*)EndIterator->Buffer->GetData();
            Communicate->m_dwSendBufferLength -= Block->Len;
            EndIterator->Buffer->Release();
        }
    }

    Communicate->m_SendList.clear();

    LeaveCriticalSection(&Communicate->m_csSendListLock);

    Communicate->Release();

    return;
}


