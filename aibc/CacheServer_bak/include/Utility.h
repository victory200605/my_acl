
#ifndef __AI_UTILITY_H__
#define __AI_UTILITY_H__

#include <assert.h>
#include <string.h>
#include "../include/Config.h"
#include "../include/Errno.h"

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
    AIBC::AIWriteLOG( NULL, ( AICache::IsWarning(ret) ? AILOG_LEVEL_DEBUG : AILOG_LEVEL_ERROR ), __VA_ARGS__ );
/// end log define

AI_CACHE_NAMESPACE_START

/// Define for put/get flag
enum
{
    OVERWRITE    = 1,
    NO_OVERWRITE = 2,
    F_DELETE     = 3,
    NO_DELETE    = 4
};

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

		sprintf( lpsHeader, "%08X:", liRowIdx );

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

#define DEFINE_CACHE_MAIN( name, cache )                                 \
int name( int argc, char *argv[], pid_t atParentPID )                    \
{                                                                        \
    char lsConfigFile[AI_CACHE_NAMESPACE::AI_MAX_NAME_LEN] = {0};        \
    ::snprintf( lsConfigFile, sizeof(lsConfigFile),                      \
        "../../config/%s.ini", AIM::GetProcessName() );                  \
    if ( AI_CACHE_NAMESPACE::cache::Instance()->Startup(lsConfigFile) != 0 ) \
	{                                                                    \
		fprintf(stderr, "Fail to startup server\n");                     \
		return 1;                                                        \
	}                                                                    \
	                                                                     \
	fprintf(stderr, "Server startup success\n");                         \
	                                                                     \
	while ( !AIM::IsShutdown() )                                         \
    {                                                                    \
        AIBC::AISleepFor( AI_TIME_SEC );                                       \
                                                                         \
     	AI_CACHE_NAMESPACE::cache::Instance()->PrintStat();              \
    }                                                                    \
                                                                         \
    AI_CACHE_NAMESPACE::cache::Instance()->Shutdown();                   \
                                                                         \
    fprintf(stderr, "Server exit......\n");                              \
                                                                         \
    return 0;                                                            \
}

AI_CACHE_NAMESPACE_END

#endif //__AI_UTILITY_H__
