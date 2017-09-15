/**
 * @file     IThread.cpp
 * @author   4680414@qq.com
 * @date     2014/12/24
 * @version  1.0
 * @brief    线程控制类源文件
 */
#include "Common/IThread.h"
#include "Common/Thread.h"

IThread* WINAPI CreateIThreadInstance(ThreadMainProc Func, LPVOID Param)
{
    return new CThread(Func, Param);
}

IThread* WINAPI CreateIThreadInstanceEx(ThreadMainProc MainFunc, LPVOID MainParam, ThreadEndProc EndFunc, LPVOID EndParam)
{
    return new CThread(MainFunc, MainParam, EndFunc, EndParam);
}
