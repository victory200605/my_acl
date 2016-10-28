//=============================================================================
/**
* \file    CacheData.h
* \brief store file
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: CacheData.h,v 1.1 2011/01/05 08:34:16 daizh Exp $
*
* History
* 2008.06.18 first release by DZH
*/
//=============================================================================

#ifndef AI_CACHEDATA_H_2008
#define AI_CACHEDATA_H_2008

#include "StoreFile.h"

AI_CACHE_NAMESPACE_START

struct stDataNode
{
    stDataNode();
    
    size_t ciKeySize;
	size_t ciDataSize;
	int ciTimestamp;
	char ccMsgID[AI_MAX_KEY_LEN];
	char ccData[AI_MAX_VALUE_LEN];
};

class clsCacheData : public clsStoreFile<stDataNode>
{
public:
    typedef clsStoreFile<stDataNode> TBase;
public:
    int Create( const char* apsFileName, int aiCount, size_t aiSize );	
    int Open( const char* apsFileName, int aiCount, size_t aiSize );
    int Read( int aiIdx, AIChunkEx& aoKey, AIChunkEx& aoValue, int* apiTimestamp );
    int Write( int aiIdx, const AIChunkEx& aoKey, const AIChunkEx& aoValue, int aiTimestamp );
};

AI_CACHE_NAMESPACE_END

#endif // AI_CACHEDATA_H_2008
