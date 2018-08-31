#pragma once

#ifndef __ENLIB_THREAD_H__
#define __ENLIB_THREAD_H__

#include "Common/IThread.h"
#include "Base/BaseObject.h"

namespace enlib
{
    class CThread : public IThread
    {
    public:
        CThread(ThreadMainProc Func);
        CThread(ThreadMainProc MainFunc, ThreadEndProc Endfunc);

        virtual ~CThread(void);
    public:
        virtual BOOL WINAPI StartMainThread(CObjPtr<CObject> Param);
        virtual void WINAPI StopMainThread();
        virtual BOOL WINAPI IsMainThreadRunning();

    private:
        void Init(ThreadMainProc MainFunc, ThreadEndProc Endfunc);

        static DWORD WINAPI MainThread(LPVOID Lp);

        HANDLE           m_hMainThread;
        DWORD            m_dwMainThreadId;
        HANDLE           m_hMainThreadStartedEvent;
        HANDLE           m_hStopMainThreadEvent;
        HANDLE           m_hMainThreadStopedEvent;
        BOOL             m_bContinueMainThread;

        ThreadMainProc   m_fnMainProc;
        CObjPtr<CObject> m_spParam;
        ThreadEndProc    m_fnEndProc;
        CRITICAL_SECTION m_csEndLock;
    };
};

#endif
