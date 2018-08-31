#include "Base/ParamSet.h"
#include <list>
#include "SuperHash.h"

using namespace enlib;

CParamSet::CParamSet()
{
    m_ParamMap.clear();
    InitializeCriticalSection(&m_csParamLock);
#ifdef WIN32
    m_ComParamMap.clear();
    InitializeCriticalSection(&m_csComParamLock);
#endif
}

CParamSet::~CParamSet()
{
    std::map<UINT32, CObjPtr<CObject>>::iterator Itor;
    std::list<CObjPtr<CObject>> TmpList;
    std::list<CObjPtr<CObject>>::iterator TmpListItor;
    
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

#ifdef WIN32
    std::map<UINT32, CComPtr<IUnknown>>::iterator ComItor;
    std::list<CComPtr<IUnknown>> ComTmpList;
    std::list<CComPtr<IUnknown>>::iterator ComTmpListItor;
    
    EnterCriticalSection(&m_csComParamLock);
    for (ComItor = m_ComParamMap.begin(); ComItor != m_ComParamMap.end(); ComItor++)
    {
        if (ComItor->second != NULL)
        {
            ComTmpList.push_back(ComItor->second);
            ComItor->second = NULL;
        }
    }
    m_ComParamMap.clear();
    LeaveCriticalSection(&m_csComParamLock);

    for (ComTmpListItor = ComTmpList.begin(); ComTmpListItor != ComTmpList.end(); ComTmpListItor++)
    {
        (*ComTmpListItor) = NULL;
    }

    ComTmpList.clear();
    
    DeleteCriticalSection(&m_csComParamLock);
#endif
}

CObjPtr<CObject> CParamSet::GetParam(const CHAR* ParamKeyword)
{
    CObjPtr<CObject> spRet = NULL;
    UINT32 uHash = SuperFastHash(ParamKeyword, strlen(ParamKeyword), 1);
    std::map<UINT32, CObjPtr<CObject>>::iterator Itor;

    EnterCriticalSection(&m_csParamLock);

    Itor = m_ParamMap.find(uHash);
    if (Itor != m_ParamMap.end())
    {
        spRet = Itor->second;
    }

    LeaveCriticalSection(&m_csParamLock);

    return spRet;
}

VOID CParamSet::SetParam(const CHAR* ParamKeyword, CObjPtr<CObject> Param)
{
    UINT32 uHash = SuperFastHash(ParamKeyword, strlen(ParamKeyword), 1);
   
    EnterCriticalSection(&m_csParamLock);
    m_ParamMap[uHash] = Param;
    LeaveCriticalSection(&m_csParamLock);
    return;
}

VOID CParamSet::SetParam(const UINT32 uHash, CObjPtr<CObject> Param)
{
	EnterCriticalSection(&m_csParamLock);
    m_ParamMap[uHash] = Param;
    LeaveCriticalSection(&m_csParamLock);
    return;
}

#ifdef WIN32

CComPtr<IUnknown> CParamSet::GetComParam(const CHAR* ParamKeyword)
{
    CComPtr<IUnknown> spRet = NULL;
    UINT32 uHash = SuperFastHash(ParamKeyword, strlen(ParamKeyword), 1);
    std::map<UINT32, CComPtr<IUnknown>>::iterator Itor;

    EnterCriticalSection(&m_csComParamLock);

    Itor = m_ComParamMap.find(uHash);
    if (Itor != m_ComParamMap.end())
    {
        spRet = Itor->second;
    }

    LeaveCriticalSection(&m_csComParamLock);

    return spRet;
}

VOID CParamSet::SetParam(const CHAR* ParamKeyword, CComPtr<IUnknown> Param)
{
    UINT32 uHash = SuperFastHash(ParamKeyword, strlen(ParamKeyword), 1);
   
    EnterCriticalSection(&m_csComParamLock);
    m_ComParamMap[uHash] = Param;
    LeaveCriticalSection(&m_csComParamLock);
    return;
}

VOID CParamSet::SetParam(const UINT32 uHash, CComPtr<IUnknown> Param)
{
	EnterCriticalSection(&m_csComParamLock);
    m_ComParamMap[uHash] = Param;
    LeaveCriticalSection(&m_csComParamLock);
    return;
}

VOID CParamSet::CopyParam(CParamSet* Src)
{
    std::map<UINT32, CObjPtr<CObject>>::iterator ParamIterator;
    
    EnterCriticalSection(&m_csParamLock);

    for (ParamIterator = Src->m_ParamMap.begin(); ParamIterator != Src->m_ParamMap.end(); ParamIterator++)
    {
        this->SetParam(ParamIterator->first, ParamIterator->second);
    }   

    LeaveCriticalSection(&m_csParamLock);

#ifdef WIN32
    std::map<UINT32, CComPtr<IUnknown>>::iterator ComParamIterator;

    EnterCriticalSection(&m_csComParamLock);

    for (ComParamIterator = Src->m_ComParamMap.begin(); ComParamIterator != Src->m_ComParamMap.end(); ComParamIterator++)
    {
        this->SetParam(ComParamIterator->first, ComParamIterator->second);
    }   

    LeaveCriticalSection(&m_csComParamLock);
#endif

    return;
};

#endif