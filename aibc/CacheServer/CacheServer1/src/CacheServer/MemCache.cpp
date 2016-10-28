#include "MemCache.h"

AI_CACHE_NAMESPACE_START

clsMemCache::clsMemCache()
{
}

clsMemCache::~clsMemCache()
{
}

int clsMemCache::Open( const char* apsDirPath )
{
    return 0;
}

void clsMemCache::Close(void)
{
}

int clsMemCache::Put( const AIChunkEx& aoKey, const AIChunkEx& aoValue, int aiFlag )
{
    AISmartLock loGuard(this->moLock);

    std::string loKey(aoKey.BasePtr(), aoKey.GetSize() );

    std::string& loValue = this->moMap[loKey];
    if (loValue.size() > 0 && aiFlag == DONT_OVERWRITE)
    {
        return AI_ERROR_DUP_RECORD;
    }

    loValue.assign(aoValue.BasePtr(), aoValue.GetSize() );

    return 0;
}

int clsMemCache::Get( const AIChunkEx& aoKey, AIChunkEx& aoValue, int* apiTimestamp, int aiFlag )
{
    AISmartLock loGuard(this->moLock);

    std::string loKey(aoKey.BasePtr(), aoKey.GetSize() );

    std::map<std::string, std::string>::iterator loIter = this->moMap.find(loKey);
    if (loIter == this->moMap.end() )
    {
        return AI_WARN_NO_FOUND;
    }

    aoValue.Resize(loIter->second.size() );
    memcpy(aoValue.BasePtr(), loIter->second.c_str(), aoValue.GetSize() );

    if (aiFlag == DO_DELETE)
    {
        this->moMap.erase(loIter);
    }

    return 0;
}

int clsMemCache::Put( const char* apsKey, const AIChunkEx& aoValue, int aiFlag )
{
    return AI_ERROR_UNCOMPATIBLE_VERSION;
}

int clsMemCache::Get( const char* apsKey, AIChunkEx& aoValue, int* apiTimestamp, int aiFlag )
{
    return AI_ERROR_UNCOMPATIBLE_VERSION;
}

int clsMemCache::Update( const AIChunkEx& aoKey, const AIChunkEx& aoValue )
{
    return AI_ERROR_UNCOMPATIBLE_VERSION;
}

int clsMemCache::DelTimeOut( int aiTimestamp, AIChunkEx& aoKey, AIChunkEx& aoValue, int* apiTimestamp )
{
    return AI_ERROR_UNCOMPATIBLE_VERSION;
}

void clsMemCache::GetStat( clsCacheStat& aoStat )
{
    aoStat.SetSize(this->moMap.size() );
    aoStat.SetCapacity(1000000);
    aoStat.SetMaxCapacity(1000000);
    aoStat.SetNodeCount(1);
    aoStat.SetNodeCapacity(1000000);
}

AI_CACHE_NAMESPACE_END

