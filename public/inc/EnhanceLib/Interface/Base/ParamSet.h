#pragma once

#ifndef __PARAM_SET_H__
#define __PARAM_SET_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include <map>
#include "DllExport.h"
#include "Base/BaseObject.h"

#ifdef WIN32
template class __declspec(dllexport) std::map<UINT32, CBaseObjPtr<CBaseObject>>;  
#endif

class DLL_COMMONLIB_API CParamSet
{
public:
    CParamSet();

    virtual ~CParamSet();

    virtual CBaseObjPtr<CBaseObject> WINAPI GetParam(const CHAR* ParamKeyword);

    virtual VOID WINAPI SetParam(const CHAR* ParamKeyword, CBaseObjPtr<CBaseObject> Param);

    virtual VOID WINAPI SetParam(const UINT32 uHash, CBaseObjPtr<CBaseObject> Param);

private:
    std::map<UINT32, CBaseObjPtr<CBaseObject>> m_ParamMap;
    CRITICAL_SECTION                           m_csParamLock;
};


#endif