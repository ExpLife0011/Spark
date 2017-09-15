#pragma once

#ifndef __REMOTE_SESSION_MANAGER_H__
#define __REMOTE_SESSION_MANAGER_H__

#include <Windows.h>
#include <list>
#include "Base/BaseObject.h"

class IPipeClient;

class CRemoteSession : public CBaseObject
{
public:
    CRemoteSession(DWORD SessionId);

    ~CRemoteSession();

	void ReplaceToSpark();

    void Update();

    DWORD GetSessionID();

    const WCHAR* GetUserName();

    const WCHAR* GetDomainName();

    const WCHAR* GetSessionName();

    BOOL MatchUser(const WCHAR* Domain, const WCHAR* Username);

    BOOL IsSparkSession(BOOL* RemoteApp);

    BOOL IsConsole();

private:
    WCHAR                m_szUserName[MAX_PATH];
    WCHAR                m_szDomainName[MAX_PATH];
    WCHAR                m_szSessionName[MAX_PATH];
    DWORD                m_dwSessionId;
};

class CRemoteSessionManager : public CBaseObject
{
public:
    CRemoteSessionManager();
    ~CRemoteSessionManager();

    void AddSession(DWORD SessionId);
    void RemoveSession(DWORD SessionId);

	void ReplaceToSpark(DWORD SessionID);

    DWORD FindSparkConnectedSessionID(const WCHAR* Domain, const WCHAR* UserName, BOOL RemoteAppConnection);

    void NotifyServiceConnected();
private:
    void SendPacketToService(CRemoteSession* Session, BOOL Add);

    std::list<CRemoteSession*> m_SessionList;
    CRITICAL_SECTION           m_csLock;
};

#endif