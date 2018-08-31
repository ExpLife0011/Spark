#pragma once

#ifndef __ENLIB_PARAM_SET_H__
#define __ENLIB_PARAM_SET_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include <map>
#include "DllExport.h"
#include "Base/BaseObject.h"

namespace enlib
{
    #ifdef WIN32
    template class __declspec(dllexport) std::map<UINT32, CObjPtr<CObject>>;  
    #endif

    class DLL_COMMONLIB_API CParamSet
    {
    public:
        CParamSet();

        virtual ~CParamSet();

        virtual CObjPtr<CObject> WINAPI GetParam(const CHAR* ParamKeyword);

        virtual VOID WINAPI SetParam(const CHAR* ParamKeyword, CObjPtr<CObject> Param);

        virtual VOID WINAPI SetParam(const UINT32 uHash, CObjPtr<CObject> Param);

    private:
        std::map<UINT32, CObjPtr<CObject>> m_ParamMap;
        CRITICAL_SECTION                   m_csParamLock;
    };
};


#endif