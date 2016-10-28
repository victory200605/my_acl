#include <stdio.h>

#include "AIChunkEx.h"

// Define AI_RETURN_IF
#ifndef AI_RETURN_IF
#	define AI_RETURN_IF( ret, op ) if(op) { return -1; }
#endif

AIChunkEx::AIChunkEx( size_t aiSize /* = 100 */ ) : 
    cpsData(NULL),
    ciSize(0),
    ciCapacity(0),
    ciReadPos(0),
    ciWritePos(0)
{
    Resize( aiSize );
}

AIChunkEx::AIChunkEx( const AIChunkEx& aoOther ) : 
    cpsData(NULL),
    ciSize(0),
    ciCapacity(0),
    ciReadPos(aoOther.ciReadPos),
    ciWritePos(aoOther.ciWritePos)
{
    Resize( aoOther.ciSize );
    
    memcpy( cpsData, aoOther.cpsData, aoOther.ciSize );
}

AIChunkEx::AIChunkEx( const void* apvPtr, size_t aiSize ) : 
    cpsData(NULL),
    ciSize(0),
    ciCapacity(0),
    ciReadPos(0),
    ciWritePos(aiSize)
{
    Resize( aiSize );
    
    memcpy( cpsData, apvPtr, aiSize );
}

AIChunkEx::~AIChunkEx()
{
    delete[] cpsData;
    cpsData = NULL;
}

AIChunkEx& AIChunkEx::operator = ( const AIChunkEx& aoOther )
{
    Resize( aoOther.ciSize );
    
    memcpy( cpsData, aoOther.cpsData, aoOther.ciSize );
    
    ciReadPos = aoOther.ciReadPos;
    ciWritePos = aoOther.ciWritePos;
	return *this;
}

int AIChunkEx::Resize( size_t aiSize )
{
    assert( aiSize >= 0 );
        
    if ( aiSize <= ciCapacity )
    {
        ciSize = aiSize;
        return 0;
    }
    
    //reallocate and initialize memory
    char* lpsTmp = new char[aiSize];
    assert( lpsTmp != NULL );
    memset( lpsTmp, 0, aiSize );
    
    //append original data
    if ( cpsData != NULL )
    {
        memcpy( lpsTmp, cpsData, ciSize );
    }
    //free old data chunk
    delete[] cpsData;
    
    cpsData    = lpsTmp;
    ciSize     = aiSize;
    ciCapacity = aiSize;
    return 0;
}

char* AIChunkEx::BasePtr() const
{
    return cpsData;
}

char* AIChunkEx::BasePtr()
{
    return cpsData;
}

char* AIChunkEx::GetPointer()
{
    return BasePtr();
}

void AIChunkEx::Swap( AIChunkEx& aoOther )
{
	Swap( cpsData,    aoOther.cpsData );
	Swap( ciSize,     aoOther.ciSize );
	Swap( ciCapacity, aoOther.ciCapacity );
	Swap( ciReadPos,  aoOther.ciReadPos );
	Swap( ciWritePos, aoOther.ciWritePos );
}

void AIChunkEx::Reset()
{
    ciReadPos = 0;
    ciWritePos = 0;
}

void AIChunkEx::ResetRead()
{
	ciReadPos = 0;
}
void AIChunkEx::ResetWrite()
{
	ciWritePos = 0;
}

char* AIChunkEx::ReadPtr( void ) const
{
    return BasePtr() + ciReadPos;
}

void AIChunkEx::ReadPtr( size_t aiN )
{
    ciReadPos += aiN;
}

void AIChunkEx::ReadPtr( char* apPtr )
{
    ciReadPos = apPtr - BasePtr();
}

char* AIChunkEx::WritePtr( void ) const
{
    return BasePtr() + ciWritePos;
}

void AIChunkEx::WritePtr( char* apPtr )
{
    ciWritePos = apPtr - BasePtr();
}

void AIChunkEx::WritePtr( size_t aiN )
{
    ciWritePos += aiN;
}

size_t AIChunkEx::GetSize() const
{
    return ciSize;
}

size_t AIChunkEx::GetSize()
{
    return ciSize;
}

size_t AIChunkEx::GetCapacity() const
{
    return ciCapacity;
}

size_t AIChunkEx::GetCapacity()
{
    return ciCapacity;
}

//---------------------------------AIChunkExIN---------------------------------//
AIChunkExIn::AIChunkExIn( AIChunkEx& aoChunk ) : coChunk(aoChunk)
{
//    coChunk.ResetRead();
}

//raw instream
#define AI_CHUNK_TO_TYPE( type, name ) \
    type AIChunkExIn::To##name() \
    {\
        type liN = 0;\
        ToMem( &liN, sizeof( type ) );\
        return liN;\
    }\
    int AIChunkExIn::To##name( type& aiN )\
    {\
        return ToMem( &aiN, sizeof( type ) );\
    }

AI_CHUNK_TO_TYPE( int8_t, Int8 )
AI_CHUNK_TO_TYPE( int16_t, Int16 )
AI_CHUNK_TO_TYPE( int32_t, Int32 )

//change from network sequence to host sequence
#define AI_CHUNK_TO_TYPE_H( type, name ) \
    type AIChunkExIn::ToH##name() \
    {\
        type liN = 0;\
        ToMem( &liN, sizeof( type ) );\
        liN = ntohl(liN); \
        return liN;\
    }\
    int AIChunkExIn::ToH##name( type& aiN )\
    {\
        int liRet = ToMem( &aiN, sizeof( type ) );\
        if ( liRet == 0 ) \
            aiN = ntohl(aiN);\
        return liRet; \
    }

//AI_CHUNK_TO_TYPE_H( int8_t, Int8 )
//AI_CHUNK_TO_TYPE_H( int16_t, Int16 )
AI_CHUNK_TO_TYPE_H( int32_t, Int32 )
    
int AIChunkExIn::ToMem( void* apvPtr, size_t aiSize )
{
    if ( (size_t)(coChunk.WritePtr() - coChunk.ReadPtr()) < aiSize )
    {
        //ASSERT( AIChunkEx_to_mem_out_of_rangle, true );
        return -1;
    }
    
    memcpy( apvPtr, coChunk.ReadPtr(), aiSize );
    
    coChunk.ReadPtr( aiSize );
    
    return 0;
}


int AIChunkExIn::ToCStr(char* apcStr, size_t aiSize)
{
    size_t liStrLen;
    char const* lpcStrEnd = (char const*)memchr(coChunk.ReadPtr(), '\0', coChunk.WritePtr() - coChunk.ReadPtr());
    
    AI_RETURN_IF(-1, (NULL == lpcStrEnd)); 
     
    liStrLen = lpcStrEnd - (char const*)coChunk.ReadPtr() + 1;

    if ( aiSize < liStrLen )
    {
        return -1;
    }

    memcpy(apcStr, coChunk.ReadPtr(), liStrLen);

    return 0;
}


template<>
AIChunkExIn& operator >><int8_t> ( AIChunkExIn& aoIn, int8_t& atN )
{
	atN = aoIn.ToInt8();
	return aoIn;
}

template<>
AIChunkExIn& operator >><int16_t> ( AIChunkExIn& aoIn, int16_t& atN )
{
	atN = aoIn.ToInt16();
	return aoIn;
}

template<>
AIChunkExIn& operator >><int32_t> ( AIChunkExIn& aoIn, int32_t& atN )
{
	atN = aoIn.ToInt32();
	return aoIn;
}

//--------------------------------AIChunkExOut-----------------------------//
AIChunkExOut::AIChunkExOut( AIChunkEx& aoChunk ) : coChunk(aoChunk)
{
//	coChunk.ResetWrite();
}

//raw outstream
void AIChunkExOut::PutInt8( int8_t aiN )
{
    PutMem( &aiN, sizeof( int8_t ) );
}

void AIChunkExOut::PutInt16( int16_t aiN )
{
    PutMem( &aiN, sizeof( int16_t ) );
}

void AIChunkExOut::PutInt32( int32_t aiN )
{
    PutMem( &aiN, sizeof( int32_t ) );
}

//change to network sequence from host sequence
void AIChunkExOut::PutNInt8( int8_t aiN )
{
    aiN = htonl( aiN );
    PutMem( &aiN, sizeof( int8_t ) );
}

void AIChunkExOut::PutNInt16( int16_t aiN )
{
    aiN = htonl( aiN );
    PutMem( &aiN, sizeof( int16_t ) );
}

void AIChunkExOut::PutNInt32( int32_t aiN )
{
    aiN = htonl( aiN );
    PutMem( &aiN, sizeof( int32_t ) );
}

void AIChunkExOut::PutMem( const void* apvPtr, size_t aiSize )
{
    size_t liNewSize = coChunk.WritePtr() - coChunk.BasePtr() + aiSize;
    if ( liNewSize > coChunk.GetSize() ) 
    {
        coChunk.Resize( liNewSize );
    }
    memcpy( coChunk.WritePtr(), apvPtr, aiSize );
    coChunk.WritePtr( aiSize );
}

void AIChunkExOut::PutCStr(char const* apcStr)
{
    size_t liStrLen = strlen(apcStr) + 1;
    size_t liNewSize = coChunk.WritePtr() - coChunk.BasePtr() + liStrLen;

    if ( liNewSize > coChunk.GetSize() )
    {
        coChunk.Resize( liNewSize );
    }

    memcpy(coChunk.WritePtr(), apcStr, liStrLen);

    coChunk.WritePtr( liStrLen );
}


template< >
AIChunkExOut& operator << <int8_t> ( AIChunkExOut& aoOut, int8_t atN )
{
	aoOut.PutInt8( atN );
	return aoOut;
}

template< >
AIChunkExOut& operator << <int16_t> ( AIChunkExOut& aoOut, int16_t atN )
{
	aoOut.PutInt16( atN );
	return aoOut;
}

template< >
AIChunkExOut& operator << <int32_t> ( AIChunkExOut& aoOut, int32_t atN )
{
	aoOut.PutInt32( atN );
	return aoOut;
}
