#ifdef WIN32
#include "StdAfx.h"
#else
#include <winpr/wtypes.h>
#include <winpr/file.h>
#endif

#include "Log/LogExBase.h"

CHAR gModuleNameArrayA[MAX_PATH] = {'\0'};
static CHAR* gModuleNameA = gModuleNameArrayA;

CLogWriter gLogWriter;

static BOOL GetFullPathByHandle(const CHAR *filename, HMODULE hModule, CHAR* FullPath)
{
    memset(FullPath, 0, sizeof(CHAR) * MAX_PATH);
    GetModuleFileNameA(hModule, FullPath, MAX_PATH);

    for (int i = MAX_PATH; i >= 0; i--)
    {
        if (FullPath[i] == '\\')
        {
            strcpy(&FullPath[i + 1], filename);
            return TRUE;
        }
    }

    return FALSE;
}

void CLogEx::LogInit(HMODULE DllModule, CHAR* FileName, DWORD level)
{
    CHAR FullPath[MAX_PATH + 1];
    GetFullPathByHandle(FileName, DllModule, FullPath);
    gLogWriter.LogInit(level, FullPath);
}

void CLogEx::LogInit(CHAR* ConfigFullPath, DWORD level)
{
    gLogWriter.LogInit(level, ConfigFullPath);
}

void CLogEx::SetModuleName(CHAR* ModuleName)
{
    strcpy(gModuleNameArrayA, ModuleName);
}

CHAR* CLogEx::GetModuleName()
{
    return gModuleNameA;
}

void CLogEx::LogDone()
{
    gLogWriter.LogClose();
}

void CLogEx::Dump(unsigned char* Buffer, unsigned int Length)
{
    unsigned char* p = Buffer;
    unsigned char tmp[16];
    L_TRACE("---------------------------------------------------\n");
    for (unsigned int i = 0; i < Length / 16; i++)
    {
        L_TRACE("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
        p += 16;
    }

    memset(tmp, 0, 16);
    memcpy(tmp, p, Length % 16);
    p = tmp;
    L_TRACE("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
    L_TRACE("---------------------------------------------------\n");
}
