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

        virtual LONG WINAPI AddRef();
        virtual LONG WINAPI Release();
    private:
        volatile LONG m_lRef;
    };
};

#endif