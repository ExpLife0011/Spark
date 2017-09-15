#include "Common/IOpini.h"
#include "Common/Opini.h"

IFileIni* WINAPI CreateIFileIniInstance(const CHAR* inifile)
{
    return new CFileIni(inifile);
}