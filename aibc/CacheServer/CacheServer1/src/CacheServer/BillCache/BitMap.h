//=============================================================================
/**
* \file    BitMap.h
* \brief store file
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: BitMap.h,v 1.1 2011/01/05 08:34:16 daizh Exp $
*
* History
* 2008.06.18 first release by DZH
*/
//=============================================================================
#ifndef AI_BITMAP_H_2008
#define AI_BITMAP_H_2008

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include "CacheDefine.h"

AI_CACHE_NAMESPACE_START

class clsBitMap
{
public:
	clsBitMap();
	~clsBitMap();
	
	int32_t  Create( size_t aiSize );
	void SetFree( int32_t aiIdx );
	void SetUsing( int32_t aiIdx );
	int32_t Alloc();
	size_t GetFreeSize()                { return ciSize - ciUsingNum; }
	int32_t GetCurrAllocPoint()         { return ciCurrAllocPoint; }

protected:
	int32_t SanFree( int32_t aiStart, int32_t aiEnd, int32_t aiBkcnt );
	
protected:
	int32_t ciCurrAllocPoint;
	size_t ciSize;
	size_t ciUsingNum;
	unsigned char* cpBitMap;

protected:
	static const unsigned char scmask = 0x80;
};

AI_CACHE_NAMESPACE_END

#endif //AI_BITMAP_H_2008
