#include "Common/IOpini.h"
#include "Common/Opini.h"

using namespace enlib;

CObjPtr<IFileIni> WINAPI CreateIFileIniInstance(const CHAR* inifile)
{
    CObjPtr<IFileIni> spRet = NULL;
    IFileIni* pIni = NULL;
    pIni = new CFileIni(inifile);

    if (pIni)
    {
        spRet = pIni;
        pIni->Release();
        pIni = NULL;
    }

    return spRet;
}