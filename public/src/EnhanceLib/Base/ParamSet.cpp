#include "Base/ParamSet.h"
#include <list>
#include "SuperHash.h"

CParamSet::CParamSet()
{
    m_ParamMap.clear();
    InitializeCriticalSection(&m_csParamLock);
}

CParamSet::~CParamSet()
{
    std::map<UINT32, CBaseObjPtr<CBaseObject>>::iterator Itor;
    std::list<CBaseObjPtr<CBaseObject>> TmpList;
    std::list<CBaseObjPtr<CBaseObject>>::iterator TmpListItor;
    
    EnterCriticalSection(&m_csParamLock);
    for (Itor = m_ParamMap.begin(); Itor != m_ParamMap.end(); Itor++)
    {
        if (Itor->second != NULL)
        {
            TmpList.push_back(Itor->second);
            Itor->second = NULL;
        }
    }
    m_ParamMap.clear();
    LeaveCriticalSection(&m_csParamLock);

    for (TmpListItor = TmpList.begin(); TmpListItor != TmpList.end(); TmpListItor++)
    {
        (*TmpListItor) = NULL;
    }

    TmpList.clear();

    DeleteCriticalSection(&m_csParamLock);
}

CBaseObjPtr<CBaseObject> CParamSet::GetParam(const CHAR* ParamKeyword)
{
    CBaseObjPtr<CBaseObject> spRet = NULL;
    UINT32 uHash = SuperFastHash(ParamKeyword, strlen(ParamKeyword), 1);
    std::map<UINT32, CBaseObjPtr<CBaseObject>>::iterator Itor;

    EnterCriticalSection(&m_csParamLock);

    Itor = m_ParamMap.find(uHash);
    if (Itor != m_ParamMap.end())
    {
        spRet = Itor->second;
    }

    LeaveCriticalSection(&m_csParamLock);

    return spRet;
}

VOID CParamSet::SetParam(const CHAR* ParamKeyword, CBaseObjPtr<CBaseObject> Param)
{
    UINT32 uHash = SuperFastHash(ParamKeyword, strlen(ParamKeyword), 1);
   
    EnterCriticalSection(&m_csParamLock);
    m_ParamMap[uHash] = Param;
    LeaveCriticalSection(&m_csParamLock);
    return;
}

VOID CParamSet::SetParam(const UINT32 uHash, CBaseObjPtr<CBaseObject> Param)
{
	EnterCriticalSection(&m_csParamLock);
    m_ParamMap[uHash] = Param;
    LeaveCriticalSection(&m_csParamLock);
    return;
}