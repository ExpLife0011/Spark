#pragma once

#ifndef __ICACHE_H__
#define __ICACHE_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/winpr.h>
#endif

#include <list>
#include "DllExport.h"
#include "BasePacket.h"
#include "Base/BaseObject.h"

class ICache : public virtual CBaseObject
{
public:
    virtual BOOL WINAPI AddData(BYTE *Data, DWORD Length) = 0;

    virtual BASE_PACKET_T* WINAPI GetPacket() = 0;
};

extern "C"
{
    DLL_COMMONLIB_API ICache* WINAPI CreateICacheInstance(HANDLE GetPacketEvent);
}

#endif
