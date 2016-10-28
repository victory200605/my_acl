//=============================================================================
/**
* \file    BitMap.cpp
* \brief store file
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: BitMap.cpp,v 1.1 2011/01/05 08:34:16 daizh Exp $
*
* History
* 2008.06.18 first release by DZH
*/
//=============================================================================

#include "BitMap.h"
#include "AILib.h"

AI_CACHE_NAMESPACE_START

clsBitMap::clsBitMap() : 
	ciCurrAllocPoint(0),
	ciSize(0),
	ciUsingNum(0),
	cpBitMap(NULL)
{}

clsBitMap::~clsBitMap()
{
	AI_DELETE_ARRAY(cpBitMap);
}

int32_t clsBitMap::Create( size_t aiSize )
{
	int32_t liByteSize = ( aiSize / 8 + 1 );

	AI_NEW_ARRAY_ASSERT( cpBitMap, unsigned char, liByteSize );
	memset( cpBitMap, 0, liByteSize );
	
	ciSize = aiSize;
	
	return 0;
}

void clsBitMap::SetFree( int32_t aiIdx )
{
	assert( (size_t)aiIdx < ciSize && aiIdx >= 0 );
	
	if ( cpBitMap[ aiIdx / 8 ] & ( scmask >> aiIdx % 8 ) )
    {
        cpBitMap[ aiIdx / 8 ] ^= ( scmask >> aiIdx % 8 );
    }
    
    if ( ciUsingNum > 0 ) ciUsingNum--;
}

void clsBitMap::SetUsing( int32_t aiIdx )
{
	assert( (size_t)aiIdx < ciSize && aiIdx >= 0 );
	
	cpBitMap[ aiIdx / 8 ] |= ( scmask >> aiIdx % 8 );
	
	ciCurrAllocPoint = aiIdx + 1;
	ciCurrAllocPoint = ciCurrAllocPoint % ciSize;
	
	ciUsingNum++;
}

int32_t clsBitMap::Alloc()
{
	//sorry, i have no enough space
	if ( ciUsingNum >= ciSize ) return -1;
			
	int32_t liFree = SanFree( ciCurrAllocPoint, ciSize, 1 );
	if ( liFree == -1 )
	{
		//second san bitmap
		liFree = SanFree( 0, ciCurrAllocPoint, 1 );
	}
	
	return liFree;
}

int32_t clsBitMap::SanFree( int32_t aiStart, int32_t aiEnd, int32_t aiBkcnt )
{
	assert( cpBitMap != NULL );
    int32_t liCnt  = 0;
    for ( int32_t liIdx = aiStart; liIdx < aiEnd; liIdx++ )
    {
        if ( cpBitMap[ liIdx / 8 ] & ( scmask >> liIdx % 8 ) )
        {
             liCnt = 0;
        }
        else 
        {
             liCnt++;
        }

        if ( liCnt == aiBkcnt )
        {
             return liIdx - aiBkcnt + 1;
        }
    }
    
    return -1;
}

AI_CACHE_NAMESPACE_END

