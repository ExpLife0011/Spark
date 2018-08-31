#pragma once

#ifndef __ENLIB_ITHREAD_H__
#define __ENLIB_ITHREAD_H__

#ifdef WIN32
#include <Windows.h>
#include <list>
#else
#include <winpr/wtypes.h>
#include <winpr/synch.h>
#endif

#include "DllExport.h"
#include "Base/BaseObject.h"

typedef BOOL (*ThreadMainProc)(enlib::CObjPtr<enlib::CObject> param, HANDLE stopevent);

typedef void (*ThreadEndProc)(enlib::CObjPtr<enlib::CObject> param);

namespace enlib
{
    class DLL_COMMONLIB_API IThread : public virtual CObject
    {
    public:
        virtual BOOL WINAPI StartMainThread(CObjPtr<CObject> Param) = 0;

        virtual void WINAPI StopMainThread() = 0;

        virtual BOOL WINAPI IsMainThreadRunning() = 0;
    };
};

DLL_COMMONLIB_API enlib::CObjPtr<enlib::IThread> WINAPI CreateIThreadInstance(ThreadMainProc Func);

DLL_COMMONLIB_API enlib::CObjPtr<enlib::IThread> WINAPI CreateIThreadInstanceEx(ThreadMainProc MainFunc, ThreadEndProc Endfunc);


#endif
