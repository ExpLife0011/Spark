/**-----------------------------------------------------------------------------
 * @file     SessionHelp.cpp
 *
 * @author   yangrz@centerm.com.cn
 *
 * @date     2014/12/11
 *
 * @brief    
 *
 * @version  
 *
 *----------------------------------------------------------------------------*/
#include "stdafx.h"
#include "SessionHelp.h"
#include "mhook.h"
#include <TlHelp32.h>

CSessionHelp CSessionHelp::s_instance;


template <class T1, class T2>
T1 union_cast(T2 v)
{
    union UT {T1 t1; T2 t2;} u;
    u.t2 = v;
    return u.t1;
}

CSessionHelp::CSessionHelp()
{
    m_func1 = NULL;
    m_hookInstalled = FALSE;
    m_enableRef = 0;
	m_HasWriteMemory = FALSE;
	m_OffsetPtr = 0;
	memset(&m_WriteCodePatch, 0, sizeof(INI_VAR_BYTEARRAY));
	memset(m_OriReadMem, 0, sizeof(m_OriReadMem));
	m_HasBackUpOriMem = FALSE;
    InitializeCriticalSection(&m_lock);
}

CSessionHelp::~CSessionHelp()
{
    DeleteCriticalSection(&m_lock);
}

void CSessionHelp::Enable()
{
    EnterCriticalSection(&m_lock);
    m_enableRef++;
    LeaveCriticalSection(&m_lock);
}

void CSessionHelp::Disable()
{
    EnterCriticalSection(&m_lock);
    m_enableRef--;
    if ( m_enableRef < 0 )
    {
        m_enableRef = 0;
    }
    LeaveCriticalSection(&m_lock);
}

CSessionHelp *
CSessionHelp::GetInstance()
{
    return &s_instance;
}

DWORD
CSessionHelp::HookHelp1(DWORD *arg)
{
    // L_TRACE_ENTER();
    CSessionHelp *self = GetInstance();
    DWORD *v = (DWORD *)this;
    DWORD *v1;
    DWORD *v2;

#ifdef _WIN64
    v1 = v + 401;
    v2 = v + 398;
#else
    v1 = v + 203;
    v2 = v + 200;
#endif

    // L_TRACE(L"m_enableRef %d\n", self->m_enableRef);
    if ( self->m_enableRef > 0 )
    {
        *arg = *v1;
        *v2 = 256;
        return 0;
    }
    else
    {
        *v2 = 0x1;
        FUNC1_T func1 = self->m_func1;
        return (this->*func1)(arg);
    }
}

void 
CSessionHelp::Install(BYTE *base)
{
    if ( !m_hookInstalled )
    {
        OSVERSIONINFOEX osvi;
        PVOID func1;

        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        GetVersionEx ((OSVERSIONINFO *) &osvi);
        if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 )
        {
            int index;

            if ( osvi.wServicePackMajor == 0 )
            {
                index = 15;
            }
            else
            {
                index = 14;
            }

            // L_TRACE(L"base %p\n", base);
#ifdef _WIN64
            func1 = (PVOID)(*(UINT64 *)(*((UINT64 *)(*(UINT64 *)(base + 1608))) + index * 8));
            // L_TRACE(L"func1 %p\n", func1);
#else
            func1 = (PVOID)(*(DWORD *)(*((DWORD *)(*(DWORD *)((*(DWORD *)(base + 8)) + 808))) + index * 4));
            // L_TRACE(L"func1 %p\n", func1);
#endif
            Mhook_SetHook(&func1, union_cast<void *>(&CSessionHelp::HookHelp1));
            m_func1 = union_cast<FUNC1_T >(func1);
            m_hookInstalled = TRUE;
        }
    }
}

void 
CSessionHelp::Uninstall()
{
    if ( m_hookInstalled )
    {
        PVOID func1;

        func1 = union_cast<void *>(m_func1);
        Mhook_Unhook(&func1);
        m_hookInstalled = FALSE;
    }
	if (m_HasWriteMemory)
	{
		SetProcess(FALSE);
	}
}


void
CSessionHelp::Install()
{
	GetDefPolicyOffsetAndCode();
	SetProcess(TRUE);
}

BOOL CSessionHelp::SetProcess(BOOL bPermitConnect)
{
	DWORD       dwRet = NO_ERROR;
	HANDLE         hProcessSnap = NULL;
	PROCESSENTRY32 pe32 = { 0 };

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap == (HANDLE)-1)
	{
		return FALSE;
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hProcessSnap, &pe32))
	{
		do
		{
			if (0 == _tcscmp(pe32.szExeFile, TEXT("svchost.exe")))
			{
				dwRet = FindModlueAddr(pe32.th32ProcessID, TEXT("termsrv.dll"), bPermitConnect);
			}
		} while (Process32Next(hProcessSnap, &pe32));
	}
	else
	{
		return FALSE;
	}

	CloseHandle(hProcessSnap);
	return TRUE;
}

DWORD CSessionHelp::FindModlueAddr(DWORD dwProcessId, TCHAR *TempSMPFileName, BOOL bPermitConnect)
{
	HMODULE hModule = NULL;
	HMODULE hMods[1024];
	DWORD   cbNeeded = 0;
	TCHAR    szModName[MAX_PATH];
	int     i = 0;
	DWORD dwret = 0;
	int iret = 0;
	SIZE_T bw = 0;
	HANDLE hProcess = NULL;

	hModule = LoadLibrary(_T("psapi.dll"));

	if (hModule)
	{
		ENUMPROCESSMODULES pEnumProcessModules = (ENUMPROCESSMODULES)GetProcAddress(hModule, "EnumProcessModules");

		if (pEnumProcessModules)
		{
#ifdef UNICODE
			GETMODULEFILENAMEEX pGetModuleFileNameEx = (GETMODULEFILENAMEEX)GetProcAddress(hModule, "GetModuleFileNameExW");
#else
			GETMODULEFILENAMEEX pGetModuleFileNameEx = (GETMODULEFILENAMEEX)GetProcAddress(hModule, "GetModuleFileNameExA");
#endif

			if (pGetModuleFileNameEx)
			{
				//加载所有的进程中包含termsrv.dll模块的，然后进行writememory

				hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwProcessId);

				if (hProcess)
				{
					if (pEnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
					{
						//枚举成功
						for (i = 0; i <= (int)(cbNeeded / sizeof(HMODULE)); i++)
						{
							if (pGetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName)))
							{
								//OutputDebugString(szModName);
								if (_tcsstr(szModName, TempSMPFileName))
								{
									uintptr_t uTerminalBaseAddress = (uintptr_t)hMods[i];
									BYTE    bReadMemTemp[50] = { NULL };
									char szReadMemory[MAX_PATH] = { NULL };
									memset(bReadMemTemp, 0, sizeof(bReadMemTemp));
									SIZE_T readSize = m_WriteCodePatch.ArraySize;
									SIZE_T outReadSize = 0;
									BOOL bret = TRUE;
									bret = ReadProcessMemory(hProcess, (LPVOID)(uTerminalBaseAddress + m_OffsetPtr), bReadMemTemp, sizeof(bReadMemTemp), &outReadSize);


									if (bret)
									{
#if 1
										OutputDebugString(TEXT("6666"));
										TCHAR szTestBuff[MAX_PATH];
										wsprintf(szTestBuff, TEXT("AREA1:%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x"),
											bReadMemTemp[0], bReadMemTemp[1], bReadMemTemp[2], bReadMemTemp[3], bReadMemTemp[4],
											bReadMemTemp[5], bReadMemTemp[6], bReadMemTemp[7], bReadMemTemp[8], bReadMemTemp[9],
											bReadMemTemp[10], bReadMemTemp[11], bReadMemTemp[12], bReadMemTemp[13], bReadMemTemp[14]);
										OutputDebugString(szTestBuff);
#endif


										if ((m_HasBackUpOriMem == FALSE) && ((bReadMemTemp[0]) != (((BYTE)m_WriteCodePatch.Value[0]))) && ((bReadMemTemp[1]) != (((BYTE)m_WriteCodePatch.Value[1]))))
										{
											//备份原始值
											//L_TRACE(_T(" will back memory g_WriteCodePatch.ArraySize=%d\r\n"), m_WriteCodePatch.ArraySize);
											for (int i = 0; i < m_WriteCodePatch.ArraySize; i++)
											{
												m_OriReadMem[i] = bReadMemTemp[i];
												m_HasBackUpOriMem = TRUE;
											}
										}
										if (bPermitConnect)
										{
											//需要进行修改
											if ((bReadMemTemp[0]) == (((BYTE)m_WriteCodePatch.Value[0])))
											{
												////STDBG((_T("has write will return \r\n")));
											}
											else
											{
												////STDBG((_T("will WriteProcessMemory hMods[i]=0x%x g_OffsetPtr=0x%x PatchCodes is 0x%2x 0x%2x 0x%2x 0x%2x\r\n"), hMods[i], m_OffsetPtr, (BYTE)m_WriteCodePatch.Value[0], m_WriteCodePatch.Value[1], m_WriteCodePatch.Value[2], m_WriteCodePatch.Value[3]));
												bret = WriteProcessMemory(hProcess, (LPVOID)(uTerminalBaseAddress + m_OffsetPtr), m_WriteCodePatch.Value, m_WriteCodePatch.ArraySize, &bw);
												if (bret == FALSE)
												{
													//STDBG((_T(" WriteProcessMemory  fail error %d\r\n"), GetLastError()));
												}
												else
												{
													m_HasWriteMemory = TRUE;
												}

											}
										}
										else
										{
											//停止服务或者其他操作，需要将内存修改回来。
											//判定是否备份了内存值
											if (m_HasBackUpOriMem)
											{
												if (((bReadMemTemp[0]) == (m_OriReadMem[0])) && ((bReadMemTemp[1]) == (m_OriReadMem[1])))
												{
													//STDBG((_T("memory is ori, not need to write \r\n")));
												}
												else
												{
													//STDBG((_T("will WriteProcessMemory  ori 0x%2x 0x%2x 0x%2x 0x%2x\r\n"), m_OriReadMem[0],m_OriReadMem[1],m_OriReadMem[2],m_OriReadMem[3]));
													bret = WriteProcessMemory(hProcess, (LPVOID)(uTerminalBaseAddress + m_OffsetPtr), m_OriReadMem, m_WriteCodePatch.ArraySize, &bw);
													if (bret == FALSE)
													{
														//STDBG((_T(" Write ori ProcessMemory  fail error %d\r\n"),GetLastError()));
													}
													else
													{
														m_HasWriteMemory = FALSE;
													}

												}
											}

										}


									}
								}
							}
						}
					}
				}
				else
				{
					dwret = GetLastError();
					//STDBG((_T("OpenProcess %d\r\n"), dwret));
				}
			}
			else
			{
				dwret = GetLastError();
				//STDBG((_T("GetProcAddress error GetModuleFileNameEx %d\r\n"), dwret));
										}

									}
		else
		{
			dwret = GetLastError();
			//STDBG((_T("GetProcAddress error EnumProcessModules %d\r\n"), dwret));
		}
								}
	else
	{
		dwret = GetLastError();

		//STDBG((_T("load psspai.dll error %d \r\n"), dwret));
	}

	if (hProcess)
	{
		CloseHandle(hProcess);
		hProcess = NULL;
	}

	if (hModule)
	{
		FreeLibrary(hModule);
		hModule = NULL;
	}

	return dwret;
}


//获取需要修改的地址偏移量和修改的数据
BOOL CSessionHelp::GetDefPolicyOffsetAndCode()
{
	FILE_VERSION FV;
	char *Sect;
	INI_VAR_STRING PatchName;
	INI_FILE *IniFile;
	TCHAR ConfigFile[MAX_PATH] = { 0x00 };
	bool Bool = false;

	//获取termsrv.dll的版本号
	if (GetModuleVersion(_T("termsrv.dll"), &FV))
	{
		if ((((BYTE)FV.wVersion.Minor) | (((BYTE)FV.wVersion.Major << 8))) == 0)
		{

			OutputDebugString(_T("Error: Failed to detect Terminal Services version1\r\n"));
			return FALSE;
		}
	}
	else
	{
		OutputDebugString(_T("Error: Failed to detect Terminal Services version2\r\n"));

		return FALSE;
	}

	//读取配置文件

	//STDBG((_T("Loading configuration...\r\n")));

	GetModuleFileName(GetCurrentModule(), ConfigFile, MAX_PATH);
	//GetModuleFileName(NULL, ConfigFile, MAX_PATH);
	OutputDebugString(ConfigFile);

	for (DWORD i = _tcslen(ConfigFile); i > 0; i--)
	{
		if (ConfigFile[i] == _T('\\'))
		{
			memset(&ConfigFile[i + 1], 0x00, ((MAX_PATH - (i + 1))) * sizeof(TCHAR));
			memcpy(&ConfigFile[i + 1], _T("ctss.ini"), _tcslen(_T("ctss.ini")) * sizeof(TCHAR));
			break;
		}
	}

	OutputDebugString(ConfigFile);
	IniFile = new INI_FILE(ConfigFile);

	// TODO: implement this
	if (IniFile == NULL)
	{
		OutputDebugString(_T("Error: Failed to load configuration\r\n"));
		return FALSE;
	}

	Sect = new char[256];
	memset(Sect, 0x00, 256);
	wsprintfA(Sect, "%d.%d.%d.%d", FV.wVersion.Major, FV.wVersion.Minor, FV.Release, FV.Build);

	//STDBG((_T("version is %d.%d.%d.%d\r\n"), FV.wVersion.Major, FV.wVersion.Minor, FV.Release, FV.Build));
	OutputDebugStringA(Sect);

	if (IniFile->SectionExists(Sect))
	{
#ifdef _WIN64

		if (!(IniFile->GetVariableInSection(Sect, "DefPolicyPatch.x64", &Bool)))
		{
			Bool = false;
			OutputDebugString(_T("GetVariableInSection _WIN64 fail\r\n"));
		}

#else

		if (!(IniFile->GetVariableInSection(Sect, "DefPolicyPatch.x86", &Bool)))
		{
			Bool = false;
			OutputDebugString(_T("GetVariableInSection _WIN32 fail\r\n"));
		}

#endif

		if (Bool)
		{
			OutputDebugString(_T("Patch CDefPolicy::Query\r\n"));
			Bool = false;
#ifdef _WIN64
			m_OffsetPtr = (uintptr_t)(INIReadDWordHex(IniFile, Sect, "DefPolicyOffset.x64", 0));
			Bool = IniFile->GetVariableInSection(Sect, "DefPolicyCode.x64", &PatchName);
#else
			m_OffsetPtr = (uintptr_t)(INIReadDWordHex(IniFile, Sect, "DefPolicyOffset.x86", 0));

			Bool = IniFile->GetVariableInSection(Sect, "DefPolicyCode.x86", &PatchName);

#endif
			//STDBG((_T("offset is 0x%x\r\n"), m_OffsetPtr));
			OutputDebugStringA(PatchName.Name);
			OutputDebugStringA(PatchName.Value);

			if (Bool)
			{
				Bool = IniFile->GetVariableInSection("PatchCodes", PatchName.Value, &m_WriteCodePatch);
			}
			//STDBG((_T("PatchCodes is 0x%x 0x%x 0x%x 0x%x\r\n"), m_WriteCodePatch.Value[0], m_WriteCodePatch.Value[1], m_WriteCodePatch.Value[2], m_WriteCodePatch.Value[3]));
		}


	}
	else
	{
		OutputDebugString(_T("SectionExists fail \r\n"));

	}

	delete[] Sect;
	return TRUE;
}

//通过模块名字获取模块版本号
BOOL  CSessionHelp::GetModuleVersion(TCHAR *lptstrModuleName, FILE_VERSION *FileVersion)
{
	typedef struct
	{
		WORD             wLength;
		WORD             wValueLength;
		WORD             wType;
		WCHAR            szKey[16];
		WORD             Padding1;
		VS_FIXEDFILEINFO Value;
		WORD             Padding2;
		WORD             Children;
	} VS_VERSIONINFO;

	//获取文件版本号，需要先进行加载，否则无法获取
	HMODULE hTermSrv = NULL;
	BOOL bret = FALSE;
	hTermSrv = LoadLibrary(lptstrModuleName);

	if (hTermSrv == NULL)
	{
		//STDBG((_T("Error: Failed to load Terminal Services library %d\r\n"), GetLastError()));
		bret = FALSE;
		return bret;
	}

	HMODULE hMod = GetModuleHandle(lptstrModuleName);

	if (hMod)
	{
		HRSRC hResourceInfo = FindResource(hMod, (LPCWSTR)1, (LPCWSTR)0x10);

		if (hResourceInfo)
		{
			VS_VERSIONINFO *VersionInfo = (VS_VERSIONINFO *)LoadResource(hMod, hResourceInfo);

			if (VersionInfo)
			{
				FileVersion->dwVersion = VersionInfo->Value.dwFileVersionMS;
				FileVersion->Release = (WORD)(VersionInfo->Value.dwFileVersionLS >> 16);
				FileVersion->Build = (WORD)VersionInfo->Value.dwFileVersionLS;
				bret = TRUE;
			}
			else
			{
				//STDBG((_T("LoadResource error %d lptstrModuleName=%s\r\n"), GetLastError(), lptstrModuleName));
				bret = FALSE;
			}
		}
		else
		{
			//STDBG((_T("FindResource error %d lptstrModuleName=%s\r\n"), GetLastError(), lptstrModuleName));
			bret = FALSE;
		}
	}
	else
	{
		//STDBG((_T("GetModuleHandle error %d lptstrModuleName=%s\r\n"), GetLastError(), lptstrModuleName));
		bret = FALSE;
	}

	if (hTermSrv)
	{
		FreeLibrary(hTermSrv);
		hTermSrv = NULL;
	}

	return bret;
}

HMODULE GetCurrentModule()
{
	HMODULE hModule = NULL;

	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (TCHAR *)GetCurrentModule, &hModule);
	return hModule;
}

DWORD CSessionHelp::INIReadDWordHex(INI_FILE *IniFile, char *Sect, char *VariableName, uintptr_t Default)
{
	INI_VAR_DWORD Variable;

	if (IniFile->GetVariableInSection(Sect, VariableName, &Variable))
	{
		return Variable.ValueHex;
	}

	return Default;
}

