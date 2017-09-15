#pragma once

#ifndef __BASE_OBJECT_H__
#define __BASE_OBJECT_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "DllExport.h"

class DLL_COMMONLIB_API CBaseObject
{
public:
    CBaseObject();
    virtual ~CBaseObject();

    virtual LONG WINAPI AddRef();
    virtual LONG WINAPI Release();
private:
    volatile LONG m_lRef;
};

#endif