#pragma once

#ifndef __ISOCKET_BASE_H__
#define __ISOCKET_BASE_H__

#include "Common/ICommunication.h"
#include "DllExport.h"
#include "Base/ParamSet.h"

namespace enlib
{
    class DLL_COMMONLIB_API ISocketBase : public virtual ICommunication, public virtual CParamSet
    {
    public:
        virtual void WINAPI Close() = 0;

        virtual VOID GetSrcPeer(CHAR* SrcAddress, DWORD BufferLen, WORD* SrcPort) = 0;

        virtual VOID GetDstPeer(CHAR* DstAddress, DWORD BufferLen, WORD* DstPort) = 0;
    };
};

#endif