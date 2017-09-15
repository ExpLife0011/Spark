/**-----------------------------------------------------------------------------
 * @file     utils.h
 *
 * @author   yangrz@centerm.com.cn
 *
 * @date     2011/8/30
 *
 * @brief    
 *
 * @version  
 *
 *----------------------------------------------------------------------------*/


HRESULT RegisterServer(HMODULE hModule,
                       const CLSID& clsid,
                       const char* szFriendlyName,
                       const char* szVerIndProgID,
                       const char* szProgID)
{
    //Get the Server location
    char szModule[512];
    DWORD dwResult = ::GetModuleFileName(hModule,szModule,sizeof(szModule)/sizeof(char));
    assert(dwResult!=0);

    //Convert the CLSID into a char
    char szCLSID[CLSID_STRING_SIZE];
    CLSIDtochar(clsid,szCLSID,sizeof(szCLSID));

    //Build the key CLSID\\{}
    char szKey[64];
    strcpy(szKey,"CLSID\\");
    strcat(szKey,szCLSID);

    //Add the CLSID to the registry
    setKeyAndValue(szKey,NULL,szFriendlyName);

    //Add the Server filename subkey under the CLSID key
    setKeyAndValue(szKey,"InprocServer32",szModule);

    setKeyAndValue(szKey,"ProgID",szProgID);

    setKeyAndValue(szKey,"VersionIndependentProgID",szVerIndProgID);

    //Add the version-independent ProgID subkey under HKEY_CLASSES_ROOT
    setKeyAndValue(szVerIndProgID,NULL,szFriendlyName);
    setKeyAndValue(szVerIndProgID,"CLSID",szCLSID);
    setKeyAndValue(szVerIndProgID,"CurVer",szProgID);

    //Add the versioned ProgID subkey under HKEY_CLASSES_ROOT
    setKeyAndValue(szProgID,NULL,szFriendlyName);
    setKeyAndValue(szProgID,"CLSID",szCLSID);
    return S_OK;
}

