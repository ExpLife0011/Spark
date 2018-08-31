#include "Common/ICache.h"
#include "Common/Cache.h"

using namespace enlib;

CObjPtr<ICache> WINAPI CreateICacheInstance(HANDLE GetPacketEvent)
{
    CObjPtr<ICache> spCache = NULL;
    ICache* pCache = NULL;

    pCache = new CCache(GetPacketEvent);

    if (pCache)
    {
        spCache = pCache;
        pCache->Release();
        pCache = NULL;
    }

    return spCache;
}