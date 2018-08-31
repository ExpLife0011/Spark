#ifndef __ENLIB_IOPINI_H__
#define __ENLIB_IOPINI_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "DllExport.h"
#include "Base/BaseObject.h"

namespace enlib
{
    class IFileIni : public virtual CObject
    {
    public:
        virtual BOOL ReadOption(const CHAR* segment, const CHAR* name, CHAR* value) = 0;

        virtual BOOL ReadOption(const CHAR* segment, const CHAR* name, DWORD* value) = 0;

        virtual BOOL DeleteOption(const CHAR* segment, const CHAR* name) = 0;

        virtual BOOL WriteOption(const CHAR* segment, const CHAR* name, CHAR* value) = 0;

        virtual BOOL WriteOption(const CHAR* segment, const CHAR* name, DWORD value) = 0;
    };
};

DLL_COMMONLIB_API enlib::CObjPtr<enlib::IFileIni> WINAPI CreateIFileIniInstance(const CHAR* inifile);


#endif
