//=============================================================================
/**
* \file    AIChunkEx.h
* \brief chunk operator, and chunk for stream
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: AIChunkEx.h,v 1.1 2010/11/23 06:35:53 daizh Exp $
*
* History
* 2008.01.18 first release by DZH
*/
//=============================================================================
#ifndef __AILIBEX__AICHUNKEX__
#define __AILIBEX__AICHUNKEX__

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if !defined(WIN32)
#	include <sys/types.h>
#	include <netinet/in.h>
#	include <inttypes.h>
#else
#	include <Winsock2.h>

// int8_t
#	if !defined(int8_t)
#		define int8_t    char
#	endif

// int16_t
#	if !defined(int16_t)
#		define int16_t   short
#	endif

// int32_t
#	if !defined(int32_t)
#		define int32_t   int
#	endif

// int64_t
#	if !defined(int64_t)
#		define int64_t   long long
#	endif
#endif

class AIChunkEx
{
public:
    AIChunkEx( const AIChunkEx& aoOther );
    AIChunkEx( const void* apvPtr, size_t aiSize );
    AIChunkEx( size_t aiSize = 100 );
    AIChunkEx& operator = ( const AIChunkEx& aoOther );

    ~AIChunkEx();
    
    int Resize( size_t aiSize );
    
    char* BasePtr() const;
    char* BasePtr();
    char* GetPointer();
    void Swap( AIChunkEx& aoOther );
    
    // Reset read & write pointer
    void Reset();
    void ResetRead();
    void ResetWrite();
    
    // Get the read pointer
    char* ReadPtr( void ) const;
    
    // Set the read pointer ahead <aiN> bytes
    void ReadPtr( size_t aiN );
    
    // Set the read pointer to <apPtr> bytes
    void ReadPtr( char* apPtr );
    
    /// Get the write pointer.
    char* WritePtr( void ) const;

    /// Set the write pointer to <apPtr>.
    void WritePtr( char* apPtr );

    /// Set the write pointer ahead <aiN> bytes.  This is used to compute
    /// the <length> of a message.
    void WritePtr( size_t aiN );

    size_t GetSize() const;
    size_t GetSize();
    size_t GetCapacity() const;
    size_t GetCapacity();

protected:
    template< class T >
    void Swap( T& l, T& r )
    {
        T ltTemp(l);
        l = r;
        r = ltTemp;
    }
    
protected:
    char* cpsData;
    size_t ciSize;
    size_t ciCapacity;
    size_t ciReadPos;
    size_t ciWritePos;
};

class AIChunkExIn
{
public:
    AIChunkExIn( AIChunkEx& aoChunk );
    
    //raw instream
    int8_t  ToInt8();
    int16_t ToInt16();
    int32_t ToInt32();
    int     ToInt8( int8_t& aiN );
    int     ToInt16( int16_t& aiN );
    int     ToInt32( int32_t& aiN );
    
    //change from network sequence to host sequence
    int8_t  ToHInt8();
    int16_t ToHInt16();
    int32_t ToHInt32();
    int     ToHInt8( int8_t& aiN );
    int     ToHInt16( int16_t& aiN );
    int     ToHInt32( int32_t& aiN );
    
    int  ToMem( void* apvPtr, size_t aiSize );

    int  ToCStr( char* apvStr, size_t aiSize );

protected:
    AIChunkEx& coChunk;
};

template< class T >
AIChunkExIn& operator >> ( AIChunkExIn& aoIn, T& atVl );

template<>
extern AIChunkExIn& operator >><int8_t> ( AIChunkExIn& aoIn, int8_t& atVl );

template<>
extern AIChunkExIn& operator >><int16_t> ( AIChunkExIn& aoIn, int16_t& atVl );

template<>
extern AIChunkExIn& operator >><int32_t> ( AIChunkExIn& aoIn, int32_t& atVl );

class AIChunkExOut
{
public:
    AIChunkExOut( AIChunkEx& aoChunk );
    
    //raw outstream
    void PutInt8( int8_t aiN );
    void PutInt16( int16_t aiN );
    void PutInt32( int32_t aiN );

    //change to network sequence from host sequence
    void PutNInt8( int8_t aiN );
    void PutNInt16( int16_t aiN );
    void PutNInt32( int32_t aiN );
    
    void PutMem( const void* apvPtr, size_t aiSize );

    void PutCStr( char const* apcStr);

protected:
    AIChunkEx& coChunk;
};

template< class T >
AIChunkExOut& operator << ( AIChunkExOut& aoOut, T atVl );

template< >
extern AIChunkExOut& operator << <int8_t> ( AIChunkExOut& aoOut, int8_t atVl );

template< >
extern AIChunkExOut& operator << <int16_t> ( AIChunkExOut& aoOut, int16_t atVl );

template< >
extern AIChunkExOut& operator << <int32_t> ( AIChunkExOut& aoOut, int32_t atVl );

#endif //__AILIBEX__AICHUNKEX__
