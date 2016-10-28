//=============================================================================
/**
* \file    AIChunkEx.h
* \brief chunk operator, and chunk for stream
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: AIChunkEx.h,v 1.6 2009/03/02 13:39:50 daizh Exp $
*
* History
* 2008.01.18 first release by DZH
*/
//=============================================================================
#ifndef __AILIBEX__AICHUNKEX_H__
#define __AILIBEX__AICHUNKEX_H__

#include <stdlib.h>
#include <string.h>
#include "AILib.h"
#include <assert.h>

///start namespace
AIBC_NAMESPACE_START

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
/**
 * @brief This is class AIChunkEx.
 */
class AIChunkEx
{
public:
    AIChunkEx( const AIChunkEx& aoOther );
    AIChunkEx( const void* apvPtr, size_t aiSize );
    AIChunkEx( size_t aiSize = 100 );
    AIChunkEx& operator = ( const AIChunkEx& aoOther );

    ~AIChunkEx();
    
    /**
     * @brief Change AIChunkEx size according to the specified size.
     *
     * @param aiSize        IN - the specified size
     *
     * @return 0 if success.
     */
    int Resize( size_t aiSize );
    
    /**
     * @brief Get base pointer.
     */
    char* BasePtr() const;
    char* BasePtr();

    /**
     * @brief Get base pointer.
     */
    char* GetPointer();

    /**
     * @brief Swap two AIChunkEx.
     */
    void Swap( AIChunkEx& aoOther );
    
    /** 
     * @brief Reset read & write pointer.
     */
    void Reset();

    /**
     * @brief Reset read pointer.
     */
    void ResetRead();

    /**
     * @brief Reset write pointer.
     */
    void ResetWrite();
    
    /**
     * @brief Get the read pointer.
     */
    char* ReadPtr( void ) const;
    
    /**
     * @brief Set the read pointer ahead <aiN> bytes.
     */
    void ReadPtr( size_t aiN );
    
    /**
     * @brief Set the read pointer to <apPtr> bytes.
     */
    void ReadPtr( char* apPtr );
    
    /**
     * @brief Get the write pointer.
     */
    char* WritePtr( void ) const;

    /**
     * @brief Set the write pointer to <apPtr>.
     */
    void WritePtr( char* apPtr );

    /**
     * @brief Set the write pointer ahead <aiN> bytes.  This is used to compute the <length> of a message.
     */
    void WritePtr( size_t aiN );

    /**
     * @brief Get chunk size.
     */
    size_t GetSize() const;

    /**
     * @brief Get chunk capacity.
     */
    size_t GetCapacity() const;

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



/**
 * @brief This is class AIChunkExIn.
 */
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
    int16_t ToHInt16();
    int32_t ToHInt32();
    int     ToHInt16( int16_t& aiN );
    int     ToHInt32( int32_t& aiN );
    
    /**
     * @brief Copy chunk to apvPtr.
     *
     * @param apvPtr      OUT - the pointer to the output buffer
     * @param aiSize      IN - the max size to copy from chunk
     *
     * @return 0 if success;-1 if error
     */
    int  ToMem( void* apvPtr, size_t aiSize );

    /**
     * @brief Copy string to apcStr,end by '\0'.
     *
     * @param apcStr      OUT - the pointer to the output string buffer
     * @param aiSize      IN - the max size to copy from chunk
     *
     * @return 0 if success;-1 if error
     */
    int  ToCStr( char* apcStr, size_t aiSize );

    /**
     * @brief Copy line to apcStr, end by "\r\n".
     *
     * @param apcStr      OUT - the pointer to the output buffer
     * @param aiSize      IN - the max size to copy from chunk
     *
     * @return 0 if success;-1 if error
     */
    int  ToLine( char* apcStr, size_t aiSize );

protected:
    AIChunkEx& coChunk;
};

template< class T >
AIChunkExIn& operator >> ( AIChunkExIn& aoIn, T& atVl );

template<>
AIChunkExIn& operator >><int8_t> ( AIChunkExIn& aoIn, int8_t& atVl );

template<>
AIChunkExIn& operator >><int16_t> ( AIChunkExIn& aoIn, int16_t& atVl );

template<>
AIChunkExIn& operator >><int32_t> ( AIChunkExIn& aoIn, int32_t& atVl );

/**
 * @brief This is class AIChunkExOut.
 */
class AIChunkExOut
{
public:
    AIChunkExOut( AIChunkEx& aoChunk );
    
    //raw outstream
    void PutInt8( int8_t aiN );
    void PutInt16( int16_t aiN );
    void PutInt32( int32_t aiN );

    //change to network sequence from host sequence
    void PutNInt16( int16_t aiN );
    void PutNInt32( int32_t aiN );
    
    /**
     * @brief Copy from buffer to chunk.
     *
     * @param apvPtr       IN - the pointer to the input buffer
     * @param aiSize       IN - the max size to copy  
     */  
    void PutMem( const void* apvPtr, size_t aiSize );

    /**
     * @brief Copy from string to chunk.
     *
     * @param apcStr       IN - the pointer to the input string
     */  
    void PutCStr( char const* apcStr);

protected:
    AIChunkEx& coChunk;
};

template< class T >
AIChunkExOut& operator << ( AIChunkExOut& aoOut, T atVl );

template< >
AIChunkExOut& operator << <int8_t> ( AIChunkExOut& aoOut, int8_t atVl );

template< >
AIChunkExOut& operator << <int16_t> ( AIChunkExOut& aoOut, int16_t atVl );

template< >
AIChunkExOut& operator << <int32_t> ( AIChunkExOut& aoOut, int32_t atVl );

///end namespace
AIBC_NAMESPACE_END

#endif //__AILIBEX__AICHUNKEX__
