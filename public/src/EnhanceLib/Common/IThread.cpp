#include "Common/IThread.h"
#include "Common/Thread.h"

using namespace enlib;

CObjPtr<IThread> WINAPI CreateIThreadInstance(ThreadMainProc Func)
{
    CObjPtr<IThread> spThread = NULL;
    IThread* pThread = NULL;
    pThread = new CThread(Func);

    if (pThread)
    {
        spThread = pThread;
        pThread->Release();
        pThread = NULL;
    }

    return spThread;
}

CObjPtr<IThread> WINAPI CreateIThreadInstanceEx(ThreadMainProc MainFunc, ThreadEndProc EndFunc)
{
    CObjPtr<IThread> spThread = NULL;
    IThread* pThread = NULL;
    pThread = new CThread(MainFunc, EndFunc);

    if (pThread)
    {
        spThread = pThread;
        pThread->Release();
        pThread = NULL;
    }

    return spThread;
}
