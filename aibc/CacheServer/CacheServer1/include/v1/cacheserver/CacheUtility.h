
#ifndef __AI_UTILITY_H__
#define __AI_UTILITY_H__

#include <assert.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////
// ailib config
// Ai lib header file config
#include "AILib.h"
#include "AIAsyncTcp.h"
#include "AIConfig.h"
#include "AILogSys.h"
#include "AIDualLink.h"

// Common header file
#include "AIFile.h"
#include "AIDir.h"
#include "AIMapFile.h"
#include "AIChunkEx.h"
#include "AIQueue.h"
#include "AIString.h"
#include "AISynch.h"
#include "AIModule.h"
#include "AIThrPool.h"

#define AI_CACHE_NAMESPACE cacheserver
#define AI_CACHE_NAMESPACE_START namespace AI_CACHE_NAMESPACE {
#define AI_CACHE_NAMESPACE_END };

/////////////////////////////// File cache log //////////////////////////////////
// for trace log
#define AI_CACHE_TRACE( ... )  \
    if ( AIBC::giGlobalLogLevel >= AILOG_LEVEL_TRACE ) \
        AIBC::AIWriteLOG( NULL, AILOG_LEVEL_TRACE, __VA_ARGS__ );

// for err log
#define AI_CACHE_ERROR( ... )  \
    AIBC::AIWriteLOG( NULL, AILOG_LEVEL_ERROR, __VA_ARGS__ );

// for debug log
#define AI_CACHE_DEBUG( ... )  \
    AIBC::AIWriteLOG( NULL, AILOG_LEVEL_DEBUG, __VA_ARGS__ );

// for info log
#define AI_CACHE_INFO( ... )  \
    AIBC::AIWriteLOG( NULL, AILOG_LEVEL_INFO, __VA_ARGS__ );

// for level log
#define AI_CACHE_LOG( ret, ... )  \
    AIBC::AIWriteLOG( NULL, ( AI_CACHE_NAMESPACE::IsWarning(ret) ? AILOG_LEVEL_DEBUG : AILOG_LEVEL_ERROR ), __VA_ARGS__ );

/// end log define

AI_CACHE_NAMESPACE_START

#include "CacheConfig.inc"

// const variable define
// ip addr length
const size_t AI_IP_ADDR_LEN       = 20;

//max message key length
//const size_t AI_MAX_KEY_LEN     = 256;

//max data buffer length
//const size_t AI_MAX_VALUE_LEN      = 2048;

//max file name length
const size_t AI_MAX_FILENAME_LEN  = 128;

const size_t AI_MAX_NAME_LEN      = 128;

//hash size
const size_t AI_HASH_SIZE         = 25165843;

const size_t AI_SERVER_HASH_SIZE  = 100;

// single file max size
const size_t AI_MAX_S_FILE_SIZE   = 0x7FFFFFFF; //2GB
// end variable define

const size_t AI_MAX_VERSION_LEN   = 40;
/// Define for put/get flag

enum
{
    DO_OVERWRITE    = 1,
    DONT_OVERWRITE  = 2,
    DO_DELETE       = 3,
    DONT_DELETE     = 4
};

// error number define
enum
{
    AI_NO_ERROR                   =  0,
    
    AI_WARN_NO_FOUND              =  70001,
    AI_ERROR_OPEN_FILE            = -70001,
    AI_ERROR_WRITE_FILE           = -70002,
    AI_ERROR_READ_FILE            = -70003,
    AI_ERROR_SYNC_FILE            = -70004,
    AI_ERROR_INVALID_FILE         = -70005,
    AI_ERROR_OUT_OF_RANGE         = -70006,
    AI_ERROR_DEL_FREE_NODE        = -70007,
    AI_ERROR_INVALID_DATA         = -70008,
    AI_ERROR_NO_ENOUGH_SPACE      = -70009,
    AI_ERROR_UNOPENED             = -70010,
    AI_ERROR_DUP_RECORD           = -70011,
    AI_ERROR_UNCOMPATIBLE_VERSION = -70012,
    AI_ERROR_MAP_FILE             = -70013,
    
    // Network
    AI_ERROR_SOCKET_SEND          = -70014,
    AI_ERROR_SOCKET_RECV          = -70015,
    AI_ERROR_INVALID_REQUST       = -70016,
    AI_ERROR_INVALID_RESPONSE     = -70017,
    
    // System
    AI_ERROR_INVALID_PARAM        = -70017,  //
    AI_ERROR_THREAD_POOL_FULL     = -70018,  // thread pool full
	AI_ERROR_SYSTEM               = -70020   // system error
};

inline bool IsWarning( int aiErrno ) { return ( AI_WARN_NO_FOUND == aiErrno )? true : false; };
// end define

inline const char* StrError( int aiErrNo )
{
    switch(aiErrNo)
    {
        case AI_NO_ERROR:                            return "No Error";
	    case AI_WARN_NO_FOUND:                       return "Record unexisted";
	    case AI_ERROR_OPEN_FILE:                     return "Open file fail";
	    case AI_ERROR_WRITE_FILE:                    return "Write file fail";
	    case AI_ERROR_READ_FILE:                     return "Read file fail";
	    case AI_ERROR_SYNC_FILE:                     return "Sync file fail";
	    case AI_ERROR_INVALID_FILE:                  return "Invalid file";
	    case AI_ERROR_INVALID_DATA:                  return "Invalid data node";
	    case AI_ERROR_MAP_FILE:                      return "Map file fail";
	    case AI_ERROR_UNCOMPATIBLE_VERSION:          return "uncompatible version";
	        
	    case AI_ERROR_OUT_OF_RANGE:                  return "Out of range";
	    case AI_ERROR_DEL_FREE_NODE:                 return "Can't delete free node";
	    case AI_ERROR_NO_ENOUGH_SPACE:               return "No enough space";
	    case AI_ERROR_UNOPENED:                      return "Cache unopened";
	    case AI_ERROR_DUP_RECORD:                    return "Record existed";
	    case AI_ERROR_SOCKET_SEND:                   return "Send data fail";
		case AI_ERROR_SOCKET_RECV:                   return "Rev data fail";
		case AI_ERROR_INVALID_REQUST:                return "Invalid protocol request";
		case AI_ERROR_INVALID_RESPONSE:              return "Invalid protocol response";
		    
	    case AI_ERROR_THREAD_POOL_FULL:              return "Thread pool full";
	    case AI_ERROR_SYSTEM:                        return "System error";
	    default:                                     return "UnKnow Error";
    }
}

//////////////////////////////////////// utility function ///////////////////////////////////////////////
inline char* CharToHex( unsigned char aiCh, char* apsRet, size_t aiSize )
{
	assert( aiSize > 2 );

	unsigned char lcChLow;
	unsigned char lcChHigh;

	lcChLow  = aiCh & 0x0F;
	lcChHigh = aiCh >> 4;

	lcChLow  += lcChLow  < 10 ? '0' : ('A' - 10);
	lcChHigh += lcChHigh < 10 ? '0' : ('A' - 10);

	*(apsRet++) = lcChHigh;
	*(apsRet++) = lcChLow;
	*(apsRet)   = '\0';

	return apsRet;
}

inline const char* UStringToHex( const char* aspBuff, size_t aiSize, AIBC::AIChunkEx& aoOut, size_t aiColumn = 4, size_t aiGroup = 4 )
{
	size_t liColumn    = aiColumn;
	size_t liGroup     = aiGroup;
	size_t liDelta     = aiSize % ( liColumn * liGroup );
	size_t liRow       = aiSize / ( liColumn * liGroup ) + (liDelta? 1 : 0);
	size_t liRowLenght = 9 /*header adder*/ + liColumn * ( liGroup * 2 + 1) + 2 /* del char*/ + liGroup * liColumn /* printable char */ + 1 /*\n*/;
	size_t liNeedSize  = liRowLenght * liRow + 1 /* 1 end */;
	size_t liStep      = liColumn * liGroup;

	aoOut.Resize( liNeedSize );
	memset( aoOut.BasePtr(), ' ', aoOut.GetSize() );

	char*  lpsCurrRow       = aoOut.BasePtr();
	unsigned char*  lpsCurr = (unsigned char*)aspBuff;
	size_t liCurrPos   = 0;
	size_t liRowIdx    = 0;
	char*  lpsHeader   = NULL;
	char*  lpsHex      = NULL;
	char*  lpsPrint    = NULL;

	for ( size_t liIt = 0; liIt < liRow; liIt++ )
	{
		lpsHeader = lpsCurrRow;
		lpsHex    = lpsHeader + 9;
		lpsPrint  = lpsHeader + 9 /*header adder*/ + liColumn * ( liGroup * 2 + 1) + 2 /* del char*/;

		sprintf( lpsHeader, "%08X:", (int)liRowIdx );

		for ( size_t liCol = 0; liCol < liColumn; liCol++ )
		{
			*lpsHex++ = ' ';

			for ( size_t liIdx = 0; liIdx < liGroup; liIdx++ )
			{
				if ( liCurrPos < aiSize )
				{
					lpsHex = CharToHex( lpsCurr[liCurrPos], lpsHex, 3 );

					*lpsPrint++ = ( isprint(lpsCurr[liCurrPos]) ? lpsCurr[liCurrPos] : '.' );
				}
				else
				{
					lpsHex = CharToHex( 0, lpsHex, 3 );
					*lpsPrint++ = '.';
				}

				liCurrPos++;
			}

			if ( liCurrPos >= aiSize ) break;
		}
		
		*lpsHex     = ' ';
		*lpsPrint++ = '\n';
		
		lpsCurrRow = lpsPrint;
		liRowIdx   = liRowIdx + liStep;
	}

	*lpsPrint = '\0';

	return aoOut.BasePtr();
}

inline size_t HashString( const char* apcString, size_t aiLength )
{
	//unsigned long llH = 0; 
	unsigned int	liRetValue = 1;
	unsigned int	liTempValue = 4;

	while( aiLength-- ) /* from MySQL */
	{ 
		liRetValue ^= (((liRetValue & 63)+liTempValue)*((unsigned int)(unsigned char) *apcString++))+ (liRetValue << 8);
		liTempValue += 3; 
	} 
	
	return liRetValue; 
}

inline size_t SubHashString( const char* apsPtr, size_t aiLength )
{
	const char *lpsEnd = apsPtr + aiLength; 
	unsigned int loHash; 
	for ( loHash = 0; apsPtr < lpsEnd; apsPtr++ ) 
	{ 
		loHash *= 16777619; 
		loHash ^= (unsigned int) *(unsigned char*) apsPtr; 
	} 
	return (loHash); 
}

AI_CACHE_NAMESPACE_END

#endif //__AI_UTILITY_H__
