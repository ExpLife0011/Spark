#pragma once

#ifndef __ENLIB_ICACHE_H__
#define __ENLIB_ICACHE_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/winpr.h>
#endif

#include <list>
#include "DllExport.h"
#include "BasePacket.h"
#include "Base/BaseObject.h"

namespace enlib
{
    class ICache : public virtual CObject
    {
    public:
        virtual BOOL WINAPI AddData(BYTE *Data, DWORD Length) = 0;

        virtual BASE_PACKET_T* WINAPI GetPacket() = 0;
    };
};

DLL_COMMONLIB_API enlib::CObjPtr<enlib::ICache> WINAPI CreateICacheInstance(HANDLE GetPacketEvent);


#endif
