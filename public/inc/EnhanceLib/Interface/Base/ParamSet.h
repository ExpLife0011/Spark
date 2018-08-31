#pragma once

#ifndef __ENLIB_PARAM_SET_H__
#define __ENLIB_PARAM_SET_H__

#ifdef WIN32
#include <Windows.h>
#include <atlbase.h>
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
    template class __declspec(dllexport) std::map<UINT32, CComPtr<IUnknown>>;
    #endif

    class DLL_COMMONLIB_API CParamSet
    {
    public:
        CParamSet();

        virtual ~CParamSet();

        virtual CObjPtr<CObject> WINAPI GetParam(const CHAR* ParamKeyword);

        virtual VOID WINAPI SetParam(const CHAR* ParamKeyword, CObjPtr<CObject> Param);

        virtual VOID WINAPI SetParam(const UINT32 uHash, CObjPtr<CObject> Param);

#ifdef WIN32
        virtual CComPtr<IUnknown> WINAPI GetComParam(const CHAR* ParamKeyword);

        virtual VOID WINAPI SetParam(const CHAR* ParamKeyword, CComPtr<IUnknown> Param);

        virtual VOID WINAPI SetParam(const UINT32 uHash, CComPtr<IUnknown> Param);
#endif

        virtual VOID CopyParam(CParamSet* Src);

    private:
        std::map<UINT32, CObjPtr<CObject>> m_ParamMap;
        CRITICAL_SECTION                   m_csParamLock;

#ifdef WIN32
        std::map<UINT32, CComPtr<IUnknown>> m_ComParamMap;
        CRITICAL_SECTION                    m_csComParamLock;
#endif
    };
};


#endif