#ifndef __SESSION_HELP_H__
#define __SESSION_HELP_H__

#include "IniFile.h"

class CSessionHelp;

typedef int (CSessionHelp::*FUNC1_T)(DWORD *arg);
typedef BOOL(WINAPI *PROCPTR)(HANDLE, DWORD, DWORD, LPVOID, DWORD, LPDWORD);

typedef BOOL(_stdcall *ENUMPROCESSMODULES)
(HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded);
typedef DWORD(_stdcall *GETMODULEFILENAMEEX)
(HANDLE hProcess, HMODULE hModule, LPTSTR lpFilename, DWORD nSize);

typedef struct
{
	union
	{
		struct
		{
			WORD Minor;
			WORD Major;
		} wVersion;
		DWORD dwVersion;
	};
	WORD Release;
	WORD Build;
} FILE_VERSION;

HMODULE GetCurrentModule();

class CSessionHelp
{
public:
    void Enable();
    void Disable();
    void Install(BYTE *base);
	void InstallBefore();
    void Uninstall();
    static CSessionHelp* GetInstance();
    DWORD HookHelp1(DWORD *arg);
	void Install();
	BOOL SetProcess(BOOL bPermitConnect);
	DWORD FindModlueAddr(DWORD dwProcessId, TCHAR *TempSMPFileName, BOOL bPermitConnect);
	BOOL GetDefPolicyOffsetAndCode();

	
	BOOL GetModuleVersion(TCHAR *lptstrModuleName, FILE_VERSION *FileVersion);
	
	DWORD INIReadDWordHex(INI_FILE *IniFile, char *Sect, char *VariableName, uintptr_t Default);
private:
    CSessionHelp();
    ~CSessionHelp();

private:
    static CSessionHelp s_instance;

    FUNC1_T m_func1;
    BOOL m_hookInstalled;
    CRITICAL_SECTION m_lock;
    volatile long m_enableRef;

	uintptr_t  m_OffsetPtr;

    INI_VAR_BYTEARRAY m_WriteCodePatch;
	
    BYTE    m_OriReadMem[50];
	BOOL    m_HasBackUpOriMem;
	BOOL	m_HasWriteMemory;
};

#endif
