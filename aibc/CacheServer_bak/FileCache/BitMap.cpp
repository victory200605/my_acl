
#include "BitMap.h"

// namespace define 
AI_CACHE_NAMESPACE_START

//----------------------------------------Header-----------------------------------//
clsBitMap::stHeader::stHeader()
{
	memset( this, 0, sizeof(stHeader) );
}

bool clsBitMap::stHeader::IsInvalid()
{
	if ( this->ciSize <= 0 ) return true;
	 
	if ( this->ciCurrAllocPos < 0 || this->ciCurrAllocPos > ciSize ) return true;
	
	if ( this->ciUsingNum < 0 || this->ciUsingNum > this->ciSize ) return true;
	
	return false;
}

const unsigned char clsBitMap::scmask;
    
clsBitMap::clsBitMap()
    : cpBitMap(NULL)
	, cpoHeader(NULL)
{
}

clsBitMap::~clsBitMap()
{
    this->coMapFile.Unmap();
}

int clsBitMap::Create( const char* apsFileName, size_t aiSize )
{
	AIBC::AIFile loFile;
	char*  lpBitMap = NULL;
	
    if ( loFile.Open( apsFileName, O_RDWR | O_CREAT | O_TRUNC, 0766 ) != AI_NO_ERROR )
    {
        //Can't not open file, are you sure the path is right
        return AI_ERROR_OPEN_FILE;
    }
    
	AI_NEW_N_ASSERT( lpBitMap, char, this->CalcBitMapSize(aiSize) );
	this->Initialize( lpBitMap, aiSize );
	
	// write bitmap
	if ( (size_t)loFile.Write( lpBitMap, CalcBitMapSize(aiSize) ) != this->CalcBitMapSize(aiSize) )
    {
        //oh,no. please check file system
    	AI_DELETE_N(lpBitMap);
        return AI_ERROR_WRITE_FILE;
    }
    
    AI_DELETE_N(lpBitMap);
	
	return AI_NO_ERROR;
}

int clsBitMap::Open( const char* apsFileName )
{
	if ( this->coMapFile.Map( apsFileName, PROT_READ | PROT_WRITE, MAP_SHARED ) != AIBC::AIMapFile::AI_NO_ERROR )
	{
		return AI_ERROR_MAP_FILE;
	}

	this->cpoHeader = (stHeader*)coMapFile.GetAddr();

	if ( this->cpoHeader->IsInvalid() || this->coMapFile.GetSize() != this->CalcBitMapSize(cpoHeader->ciSize) )
	{
		return AI_ERROR_INVALID_DATA;
	}
	
	// Sure, it right
	unsigned char* lpCheckCh = (unsigned char*)this->coMapFile.GetAddr() + this->CalcBitMapSize(cpoHeader->ciSize) - 1;
	if ( *lpCheckCh != scmask )
	{
		return AI_ERROR_INVALID_DATA;
	}
	
	this->cpBitMap = (unsigned char*)this->coMapFile.GetAddr() + sizeof(stHeader);
	
	return AI_NO_ERROR;
}

void clsBitMap::Format()
{
	assert( cpBitMap != NULL );
	
	//Reset bitmap
	this->Initialize( (char*)(cpoHeader), cpoHeader->ciSize );
}

void clsBitMap::AlignUsingSize( size_t aiSize )
{
	this->cpoHeader->ciUsingNum = aiSize;
}

void clsBitMap::SetFree( int aiIdx )
{
	assert( this->cpBitMap != NULL && (size_t)aiIdx < this->cpoHeader->ciSize && aiIdx >= 0 );
	
	if ( this->IsUsing( aiIdx ) )
    {
        this->cpBitMap[ aiIdx / 8 ] ^= ( scmask >> aiIdx % 8 );
    }
}

void clsBitMap::SetUsing( int aiIdx )
{
	assert( this->cpBitMap != NULL && (size_t)aiIdx < this->cpoHeader->ciSize && aiIdx >= 0 );
	
	this->cpBitMap[ aiIdx / 8 ] |= ( scmask >> aiIdx % 8 );
}

int clsBitMap::GetFree()
{
	assert( cpBitMap != NULL );
	
	//sorry, i have no enough space
	if ( this->cpoHeader->ciUsingNum >= cpoHeader->ciSize ) return -1;
			
	int liFree = this->SanFree( this->cpoHeader->ciCurrAllocPos, this->cpoHeader->ciSize, 1 );
	if ( liFree == -1 )
	{
		//second san bitmap
		liFree = this->SanFree( 0, this->cpoHeader->ciCurrAllocPos, 1 );
	}
	return liFree;
}

void clsBitMap::Alloc( int aiIdx )
{
    assert( !this->IsUsing(aiIdx) );
    
	this->SetUsing( aiIdx );
	
	//increace
	this->cpoHeader->ciCurrAllocPos = aiIdx + 1;
	this->cpoHeader->ciCurrAllocPos = this->cpoHeader->ciCurrAllocPos % this->cpoHeader->ciSize;
	
	this->cpoHeader->ciUsingNum++;
}

void clsBitMap::Dealloc( int aiIdx )
{
    assert( this->IsUsing(aiIdx) );
    
	this->SetFree( aiIdx );
	
	this->cpoHeader->ciUsingNum--;
}

bool clsBitMap::IsUsing( int aiIdx )
{
	return ( this->cpBitMap[ aiIdx / 8 ] & ( scmask >> aiIdx % 8 ) ) ? true : false;
}

void clsBitMap::Initialize( char* apBitMap, size_t aiSize )
{
	stHeader loHeader;
	::memset( &loHeader, 0, sizeof(stHeader) );
	loHeader.ciSize = aiSize;
	
	::memcpy( apBitMap, &loHeader, sizeof(stHeader) );
	
	::memset( apBitMap + sizeof(stHeader), 0, ( aiSize / 8 + 1 ) );
	
	::memcpy( apBitMap + this->CalcBitMapSize( aiSize ) - 1, &scmask, sizeof(scmask) );
}

size_t clsBitMap::CalcBitMapSize( size_t aiSize )
{
	return sizeof(stHeader) + ( aiSize / 8 + 1 ) + 1;
}

int clsBitMap::SanFree( int aiStart, int aiEnd, int aiBkcnt )
{
	assert( cpBitMap != NULL );
    int liCnt  = 0;
    for ( int liIdx = aiStart; liIdx < aiEnd; liIdx++ )
    {
        if ( this->cpBitMap[ liIdx / 8 ] & ( scmask >> liIdx % 8 ) )
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
