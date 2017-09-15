#include "Common/ICache.h"
#include "Common/Cache.h"

ICache* WINAPI CreateICacheInstance(HANDLE GetPacketEvent)
{
    return new CCache(GetPacketEvent);
}