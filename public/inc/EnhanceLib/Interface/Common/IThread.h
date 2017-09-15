#pragma once

#ifndef __ITHREAD_H__
#define __ITHREAD_H__

#ifdef WIN32
#include <Windows.h>
#include <list>
#else
#include <winpr/wtypes.h>
#include <winpr/synch.h>
#endif

#include "DllExport.h"
#include "Base/BaseObject.h"

typedef BOOL(*ThreadMainProc)(LPVOID param, HANDLE stopevent);

typedef void(*ThreadEndProc)(LPVOID param);

class DLL_COMMONLIB_API IThread : public virtual CBaseObject
{
public:
    virtual void WINAPI StartMainThread() = 0;

    virtual void WINAPI StopMainThread() = 0;

    virtual BOOL WINAPI IsMainThreadRunning() = 0;
};

extern "C"
{
    DLL_COMMONLIB_API IThread* WINAPI CreateIThreadInstance(ThreadMainProc Func, LPVOID Param);
 
    DLL_COMMONLIB_API IThread* WINAPI CreateIThreadInstanceEx(ThreadMainProc MainFunc, LPVOID MainParam, ThreadEndProc Endfunc, LPVOID Endparam);
}

#endif
