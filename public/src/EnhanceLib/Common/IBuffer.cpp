#include "Common/IBuffer.h"
#include "Common/Buffer.h"

IPacketBuffer* WINAPI CreateIBufferInstance(DWORD len)
{
    return new CPacketBuffer(len);
}

IPacketBuffer* WINAPI CreateIBufferInstanceEx(BYTE* buffer, DWORD len)
{
    return new CPacketBuffer(buffer, len);
}

IPacketBuffer* WINAPI CreateIBufferInstanceEx2(BYTE* buffer, DWORD len, DWORD HeadRoom, DWORD TailRoom)
{
    return new CPacketBuffer(buffer, len, HeadRoom, TailRoom);
}

void WINAPI DestoryIBufferInstance(IPacketBuffer* Buffer)
{
    delete((CPacketBuffer*)Buffer);
}
