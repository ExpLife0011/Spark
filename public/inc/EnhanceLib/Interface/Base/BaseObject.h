#pragma once

#ifndef __ENLIB_OBJECT_H__
#define __ENLIB_OBJECT_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "DllExport.h"
#include "Base/BaseObjPtr.h"

namespace enlib
{
    class DLL_COMMONLIB_API CObject
    {
    public:
        CObject();
        virtual ~CObject();

        virtual ULONG WINAPI AddRef();
        virtual ULONG WINAPI Release();
    private:
        volatile ULONG m_lRef;
    };
};

#endif