#include "stdafx.h"
#include "Common/IBuffer.h"
#include "Common/Buffer.h"

using namespace enlib;

CObjPtr<IPacketBuffer> WINAPI CreateIBufferInstance(DWORD len)
{
    CObjPtr<IPacketBuffer> spBuffer = NULL;
    IPacketBuffer* pBuffer = NULL;
    pBuffer = new CPacketBuffer(len);
    if (pBuffer)
    {
        spBuffer = pBuffer;
        pBuffer->Release();
        pBuffer = NULL;
    }

    return spBuffer;
}

CObjPtr<IPacketBuffer> WINAPI CreateIBufferInstanceEx(BYTE* buffer, DWORD len)
{
    CObjPtr<IPacketBuffer> spBuffer = NULL;
    IPacketBuffer* pBuffer = NULL;
    pBuffer = new CPacketBuffer(buffer, len);
    if (pBuffer)
    {
        spBuffer = pBuffer;
        pBuffer->Release();
        pBuffer = NULL;
    }

    return spBuffer;
}

CObjPtr<IPacketBuffer> WINAPI CreateIBufferInstanceEx2(BYTE* buffer, DWORD len, DWORD HeadRoom, DWORD TailRoom)
{
    CObjPtr<IPacketBuffer> spBuffer = NULL;
    IPacketBuffer* pBuffer = NULL;
    pBuffer = new CPacketBuffer(buffer, len, HeadRoom, TailRoom);
    if (pBuffer)
    {
        spBuffer = pBuffer;
        pBuffer->Release();
        pBuffer = NULL;
    }

    return spBuffer;
}

