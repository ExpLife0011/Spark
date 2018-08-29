#include "stdafx.h"
#include "Common/Thread.h"

#ifndef WIN32
#include <pthread.h>
#endif

#define INVALID_THREAD_ID ((DWORD)-1)

CThread::CThread(ThreadMainProc Func, LPVOID Param) : CBaseObject()
{
    Init(Func, Param, NULL, NULL);
}

CThread::CThread(ThreadMainProc MainFunc, LPVOID MainParam, ThreadEndProc EndFunc, LPVOID EndParam) : CBaseObject()
{
    Init(MainFunc, MainParam, EndFunc, EndParam);
}

void CThread::Init(ThreadMainProc MainFunc, LPVOID MainParam, ThreadEndProc EndFunc, LPVOID EndParam)
{
    m_hMainThread = NULL;
    m_dwMainThreadId = INVALID_THREAD_ID;

    m_hStopMainThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hMainThreadStopedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hMainThreadStartedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    SetEvent(m_hMainThreadStopedEvent);

    m_bContinueMainThread = FALSE;

    m_fnMainProc = MainFunc;
    m_pMainProcParam = MainParam;

    m_fnEndProc = EndFunc;
    m_pEndProcParam = EndParam;
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

BOOL CThread::StartMainThread()
{
    DWORD dwRet;
    ResetEvent(m_hStopMainThreadEvent);
    ResetEvent(m_hMainThreadStartedEvent);
    
    m_hMainThread = CreateThread(NULL, 0, CThread::MainThread, this, 0, &m_dwMainThreadId);

	dwRet = WaitForSingleObject(m_hMainThreadStartedEvent, 5000);

    return (dwRet == WAIT_OBJECT_0);
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
    CThread* Thread = (CThread*)Lp;
#ifndef WIN32
    pthread_detach(pthread_self());
#endif

    if (Thread)
    {
        Thread->AddRef();
    }
    else
    {
        return 0;
    }

    ResetEvent(Thread->m_hStopMainThreadEvent);
    ResetEvent(Thread->m_hMainThreadStopedEvent);
    Thread->m_bContinueMainThread = TRUE;
    SetEvent(Thread->m_hMainThreadStartedEvent);

    while (Thread->m_bContinueMainThread)
    {
        if (!Thread->m_fnMainProc(Thread->m_pMainProcParam, Thread->m_hStopMainThreadEvent))
        {
            break;
        }
    }

    EnterCriticalSection(&Thread->m_csEndLock);
    if (Thread->m_fnEndProc)
    {
        Thread->m_fnEndProc(Thread->m_pEndProcParam);
    }
    SetEvent(Thread->m_hMainThreadStopedEvent);
    LeaveCriticalSection(&Thread->m_csEndLock);

    Thread->Release();
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



