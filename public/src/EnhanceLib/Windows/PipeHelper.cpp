#include "stdafx.h"
#include "Windows\PipeHelper.h"
#include "Common\Buffer.h"

using namespace enlib;

static BOOL PipeReadNBytes(
    HANDLE PipeHandle,
    PBYTE  Data,
    DWORD  Length,
    DWORD  Timeout,
    HANDLE StopEvent)
{
    BOOL            Ret = FALSE;
    DWORD           BytesRead = 0;
    OVERLAPPED      OverLapped;
    PBYTE           Position;
    HANDLE          ReadEvent;
    DWORD           Error;
    DWORD           WaitRet;
    HANDLE          Handles[2];
    DWORD           HandleCount;
    BOOL            ReadRet;

    memset(&OverLapped, 0, sizeof(OVERLAPPED));

    ReadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    OverLapped.hEvent = ReadEvent;

    if (OverLapped.hEvent == NULL)
    {
        return FALSE;
    }

    Position = Data;
    while (Length > 0)
    {
		SetLastError(ERROR_SUCCESS);

        ReadRet = ReadFile(PipeHandle,
                           Position, 
                           Length, 
                           &BytesRead,
                           &OverLapped);
        if (ReadRet)
        {
            Position += BytesRead;
            Length -= BytesRead;
            ResetEvent(ReadEvent);
        }
        else
        {
			Error = GetLastError();
            if (GetLastError() == ERROR_IO_PENDING)
            {
                HandleCount = 1;
                Handles[0] = ReadEvent;
                if (StopEvent != NULL)
                {
                    Handles[1] = StopEvent;
                    HandleCount++;
                }

                WaitRet = WaitForMultipleObjects(HandleCount, Handles, FALSE, Timeout);
                if (WaitRet == WAIT_OBJECT_0)
                {
                    ReadRet = GetOverlappedResult(PipeHandle, &OverLapped, &BytesRead, TRUE );
                    if (ReadRet == FALSE)
                    {
                        goto exit;
                    }

                    Position += BytesRead;
                    Length -= BytesRead;
                    ResetEvent(ReadEvent);
                }
                else
                {
                    if (CancelIo(PipeHandle) == FALSE)
                    {
            			if (GetLastError() != ERROR_INVALID_HANDLE)
						{
							CloseHandle(PipeHandle);
						}
                    }
                    else
                    {
                        GetOverlappedResult(PipeHandle, &OverLapped, &BytesRead, TRUE);
                    }
                    goto exit;
                }
            }
            else
            {
			    goto exit;
            }
        }
    }
    Ret = TRUE;

exit:
    if (ReadEvent)
    {
        CloseHandle(ReadEvent);
    }
    return Ret;
}

BOOL PipeWriteNBytes(
    HANDLE PipeHandle,
    PBYTE  Data,
    DWORD  Length,
    DWORD  Timeout,
    HANDLE StopEvent)
{
    BOOL            Ret = FALSE;
    BOOL            WriteRet;
    OVERLAPPED      OverLapped;
    PBYTE           Position;
    DWORD           BytesWrite;
    HANDLE          WriteEvent;
    DWORD           Error;
    DWORD           WaitRet;
    HANDLE          Handles[2];
    DWORD           HandleCount;

    memset(&OverLapped, 0, sizeof(OverLapped));
    WriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    OverLapped.hEvent = WriteEvent;

    if (OverLapped.hEvent == NULL)
    {
        return FALSE;
    }

    Position = Data;
    while (Length > 0)
    {
        //异步写入
        WriteRet = WriteFile(PipeHandle,        
                             Position,
                             Length,
                             &BytesWrite,
                             &OverLapped);
        if (WriteRet)
        {
            Position += BytesWrite;
            Length -= BytesWrite;
            ResetEvent(WriteEvent);
        }
        else
        {
            Error = GetLastError();
            if (Error == ERROR_IO_PENDING)
            {
                HandleCount = 1;
                Handles[0] = WriteEvent;
                if (StopEvent != NULL)
                {
                    Handles[1] = StopEvent;
                    HandleCount++;
                }

                WaitRet = WaitForMultipleObjects(HandleCount, Handles, FALSE, Timeout);
                if (WaitRet == WAIT_OBJECT_0)
                {
                    WriteRet = GetOverlappedResult(PipeHandle, &OverLapped, &BytesWrite, TRUE );
                    if (WriteRet == FALSE)
                    {
                        goto exit;
                    }

                    Position += BytesWrite;
                    Length -= BytesWrite;
                    ResetEvent(WriteEvent);
                }
                else
                {
                    if (CancelIo(PipeHandle) == FALSE)
                    {
                		if (GetLastError() != ERROR_INVALID_HANDLE)
						{
							CloseHandle(PipeHandle);
						}
                    }
                    else
                    {
                        GetOverlappedResult(PipeHandle, &OverLapped, &BytesWrite, TRUE);
                    }
					
                    goto exit;
                }
            }
            else
            {				
                goto exit;
            }
        }
    }
    Ret = TRUE;

exit:
    if (WriteEvent)
    {
        CloseHandle(WriteEvent);
    }
    return Ret;
}

CObjPtr<IPacketBuffer> PipeRecvAPacket(
    HANDLE PipeHandle,
    DWORD  Timeout,
    HANDLE StopEvent)
{
    CObjPtr<IPacketBuffer> spPacket = NULL;
    BASE_PACKET_T*   Header = NULL;

    if (PipeHandle == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    Header = (BASE_PACKET_T*)malloc(sizeof(BASE_PACKET_T));
    memset(Header, 0, sizeof(BASE_PACKET_T));

    if (PipeReadNBytes(PipeHandle, (BYTE *)Header, sizeof(BASE_PACKET_T), Timeout, StopEvent) == FALSE )
    {
        free(Header);
        return NULL;
    }

    spPacket = CreateIBufferInstance(Header->Length - sizeof(BASE_PACKET_T));

	if (Header->Length > sizeof(BASE_PACKET_T))
    {
        if (PipeReadNBytes(PipeHandle, (BYTE *)spPacket->GetData(), spPacket->GetBufferLength(), Timeout, StopEvent) == FALSE )
        {
            free(Header);
	        return NULL;
        }
    }

    if (spPacket->DataPush(sizeof(BASE_PACKET_T)) == FALSE)
    {
        free(Header);
	    return NULL;
    }

    memcpy(spPacket->GetData(), Header, sizeof(BASE_PACKET_T));
    free(Header);

    return spPacket;
}

HANDLE PipeConnectW(PWCHAR PipeName, DWORD Timeout)
{
    BOOL Ret;
    HANDLE PipeHandle;

    Ret = WaitNamedPipeW(PipeName, Timeout);
    if (Ret == FALSE)
    {
        return INVALID_HANDLE_VALUE;
    }

    PipeHandle = CreateFileW(
        PipeName,
        GENERIC_READ|GENERIC_WRITE, 
        0,
        NULL,
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL | SECURITY_DELEGATION | FILE_FLAG_OVERLAPPED,
        NULL);

    if (PipeHandle == INVALID_HANDLE_VALUE || PipeHandle == NULL)
    {
        return INVALID_HANDLE_VALUE;
    }

    return PipeHandle;
}

HANDLE PipeConnectA(
    PCHAR PipeName,
    DWORD Timeout)
{
	BOOL Ret;
	HANDLE PipeHandle;

	Ret = WaitNamedPipeA(PipeName, Timeout);
	if (Ret == FALSE)
	{
		return INVALID_HANDLE_VALUE;
	}

	PipeHandle = CreateFileA(
        PipeName,
        GENERIC_READ|GENERIC_WRITE, 
		0,
        NULL,
        OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL | SECURITY_DELEGATION | FILE_FLAG_OVERLAPPED,
		NULL);

	if (PipeHandle == INVALID_HANDLE_VALUE || PipeHandle == NULL)
	{
		return INVALID_HANDLE_VALUE;
	}

	return PipeHandle;
}

void PipeDisconnect(HANDLE PipeHandle)
{
    if (PipeHandle != NULL && PipeHandle != INVALID_HANDLE_VALUE)
    {
        FlushFileBuffers(PipeHandle);
        DisconnectNamedPipe(PipeHandle);
        CloseHandle(PipeHandle);
    }
}

