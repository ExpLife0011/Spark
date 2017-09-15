#ifdef WIN32
#include "StdAfx.h"
#else
#include <winpr/wtypes.h>
#include <winpr/file.h>
#endif

#include "Log/LogExBase.h"

WCHAR gModuleNameArrayW[MAX_PATH] = {'\0'};
static WCHAR* gModuleNameW = gModuleNameArrayW;

CHAR gModuleNameArrayA[MAX_PATH] = {'\0'};
static CHAR* gModuleNameA = gModuleNameArrayA;

static DWORD gModuleMask = 0;

#ifdef WIN32

static BOOL GetFullPathByHandle(const TCHAR *filename, HMODULE hModule, TCHAR* FullPath)
{
    memset(FullPath, 0, sizeof(TCHAR) * MAX_PATH);
    GetModuleFileName(hModule, FullPath, MAX_PATH);

    for (int i = MAX_PATH; i >= 0; i--)
    {
        if (FullPath[i] == '\\')
        {
            _tcscpy(&FullPath[i + 1], filename);
            return TRUE;
        }
    }

    return FALSE;
}

void CLogEx::LogInitW(HMODULE DllModule, WCHAR* FileName)
{
    TCHAR FullPath[MAX_PATH + 1];
    GetFullPathByHandle(FileName, DllModule, FullPath);
    CTLogEx_init(FullPath);
}

void CLogEx::LogInitW(WCHAR* ConfigFullPath)
{
    CTLogEx_initW(ConfigFullPath);
}

#endif

void CLogEx::LogInitA(CHAR* ConfigFullPath)
{
    CTLogEx_initA(ConfigFullPath);
}

void CLogEx::SetLogLevel(DWORD Level)
{
    CTLogEx_setLogLevel(Level);
}

void CLogEx::SetOutputType(DWORD Level)
{
    CTLogEx_setOutput(Level);
}

void CLogEx::AddOutputModule(DWORD Module)
{
    gModuleMask |= (1 << Module);
}

void CLogEx::RemoveOutputModule(DWORD Module)
{
    gModuleMask &= ~(1 << Module);
}

DWORD CLogEx::GetModuleMask()
{
    return gModuleMask;
}

#ifdef WIN32
void CLogEx::SetModuleNameW(WCHAR* ModuleName)
{
    wcscpy(gModuleNameArrayW, ModuleName);
}

WCHAR* CLogEx::GetModuleNameW()
{
    return gModuleNameW;
}
#endif

void CLogEx::SetModuleNameA(CHAR* ModuleName)
{
    strcpy(gModuleNameArrayA, ModuleName);
}

CHAR* CLogEx::GetModuleNameA()
{
    return gModuleNameA;
}

void CLogEx::LogDone()
{
    CTLogEx_done();
}

void CLogEx::Dump(unsigned char* Buffer, unsigned int Length)
{
    unsigned char* p = Buffer;
    unsigned char tmp[16];
    L_TRACE(_T("---------------------------------------------------\n"));
    for (unsigned int i = 0; i < Length / 16; i++)
    {
        L_TRACE(_T("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
        p += 16;
    }

    memset(tmp, 0, 16);
    memcpy(tmp, p, Length % 16);
    p = tmp;
    L_TRACE(_T("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
    L_TRACE(_T("---------------------------------------------------\n"));
}
