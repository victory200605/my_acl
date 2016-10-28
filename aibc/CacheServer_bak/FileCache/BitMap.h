
#ifndef __AI_BITMAP_H__
#define __AI_BITMAP_H__

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "AIMapFile.h"
#include "../include/Utility.h"

// namespace define 
AI_CACHE_NAMESPACE_START

class clsBitMap
{
public:
	struct stHeader
	{
		stHeader();
		bool IsInvalid();
		
		size_t ciCurrAllocPos;
		size_t ciUsingNum;
		size_t ciSize;
	};
	
public:
	clsBitMap();
	~clsBitMap();
	
	// create bit map
	int Create( const char* apsFileName, size_t aiSize );
	
	int Open( const char* apsFileName );
	
	void Format();
	
	void AlignUsingSize( size_t aiSize );
	void SetFree( int aiIdx );
	void SetUsing( int aiIdx );
	int GetFree();
	void Alloc( int aiIdx );
	void Dealloc( int aiIdx );

	bool IsUsing( int aiIdx );
	
	size_t GetSize()                    { return cpoHeader->ciSize; }
	size_t GetFreeSize()                { return cpoHeader->ciSize - cpoHeader->ciUsingNum; }
	size_t GetUsingSize()               { return cpoHeader->ciUsingNum; }
	size_t GetCurrAllocPos()            { return cpoHeader->ciCurrAllocPos; }
	
protected:
	void Initialize( char* apBitMap, size_t aiSize );
	size_t CalcBitMapSize( size_t aiSize );
	int SanFree( int aiStart, int aiEnd, int aiBkcnt );
	
protected:
	unsigned char* cpBitMap;
	stHeader* cpoHeader;
	
	AIBC::AIMapFile coMapFile;
protected:
	static const unsigned char scmask = 0x80;
};

AI_CACHE_NAMESPACE_END

#endif //AI_BITMAP_H_2008
