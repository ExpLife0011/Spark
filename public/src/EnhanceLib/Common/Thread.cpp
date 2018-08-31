#include "stdafx.h"
#include "Common/Thread.h"

#ifndef WIN32
#include <pthread.h>
#endif

using namespace enlib;

#define INVALID_THREAD_ID ((DWORD)-1)

CThread::CThread(ThreadMainProc Func) : CObject()
{
    Init(Func, NULL);
}

CThread::CThread(ThreadMainProc MainFunc, ThreadEndProc EndFunc) : CObject()
{
    Init(MainFunc, EndFunc);
}

void CThread::Init(ThreadMainProc MainFunc, ThreadEndProc EndFunc)
{
    m_hMainThread = NULL;
    m_dwMainThreadId = INVALID_THREAD_ID;

    m_hStopMainThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hMainThreadStopedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hMainThreadStartedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    SetEvent(m_hMainThreadStopedEvent);

    m_bContinueMainThread = FALSE;

    m_fnMainProc = MainFunc;
    m_spParam = NULL;

    m_fnEndProc = EndFunc;
    InitializeCriticalSection(&m_csEndLock);
}

CThread::~CThread(void)
{
    StopMainThread();

    if (m_hStopMainThreadEvent)
    {
        CloseHandle(m_hStopMainThreadEvent);
        m_hStopMainThreadEvent = NULL;
    }

    if (m_hMainThreadStopedEvent)
    {
        CloseHandle(m_hMainThreadStopedEvent);
        m_hMainThreadStopedEvent = NULL;
    }

    if (m_hMainThreadStartedEvent)
    {
        CloseHandle(m_hMainThreadStartedEvent);
        m_hMainThreadStopedEvent = NULL;
    }

    if (m_hMainThread)
    {
        CloseHandle(m_hMainThread);
        m_hMainThread = NULL;
        m_dwMainThreadId = INVALID_THREAD_ID;
    }
    DeleteCriticalSection(&m_csEndLock);
}

BOOL CThread::StartMainThread(CObjPtr<CObject> Param)
{
    DWORD dwRet;
    ResetEvent(m_hStopMainThreadEvent);
    ResetEvent(m_hMainThreadStartedEvent);
    
    m_spParam = Param;

    m_hMainThread = CreateThread(NULL, 0, CThread::MainThread, this, 0, &m_dwMainThreadId);

	dwRet = WaitForSingleObject(m_hMainThreadStartedEvent, 5000);

    if (dwRet == WAIT_OBJECT_0)
    {
        return TRUE;
    }
    else
    {
        m_spParam = NULL;
        return FALSE;
    }
}

void CThread::StopMainThread()
{
    if (!IsMainThreadRunning())
    {
        if (m_hMainThread)
        {
            CloseHandle(m_hMainThread);
            m_hMainThread = NULL;
            m_dwMainThreadId = INVALID_THREAD_ID;
        }
        return;
    }

    if (m_hMainThread == NULL)
    {
        return;
    }
    
    m_bContinueMainThread = FALSE;
    SetEvent(m_hStopMainThreadEvent);
}

DWORD WINAPI CThread::MainThread(LPVOID Lp)
{
    CObjPtr<CThread> spThread = NULL;
    spThread = (CThread*)Lp;
#ifndef WIN32
    pthread_detach(pthread_self());
#endif

    if (spThread == NULL)
    {
        return 0;
    }

    ResetEvent(spThread->m_hStopMainThreadEvent);
    ResetEvent(spThread->m_hMainThreadStopedEvent);
    spThread->m_bContinueMainThread = TRUE;
    SetEvent(spThread->m_hMainThreadStartedEvent);

    while (spThread->m_bContinueMainThread)
    {
        if (!(spThread->m_fnMainProc(spThread->m_spParam, spThread->m_hStopMainThreadEvent)))
        {
            break;
        }
    }

    EnterCriticalSection(&spThread->m_csEndLock);
    if (spThread->m_fnEndProc)
    {
        spThread->m_fnEndProc(spThread->m_spParam);
    }
    SetEvent(spThread->m_hMainThreadStopedEvent);
    LeaveCriticalSection(&spThread->m_csEndLock);

    spThread->m_spParam = NULL;

    return 0;
}

BOOL CThread::IsMainThreadRunning()
{
    DWORD ret = WaitForSingleObject(m_hMainThreadStopedEvent, 0);
    if (ret == WAIT_TIMEOUT)
    {
        return TRUE;
    }
    return FALSE;
}



