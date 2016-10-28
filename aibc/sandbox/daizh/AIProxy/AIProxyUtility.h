
#ifndef __AIPROXY__UTILITY_H__
#define __AIPROXY__UTILITY_H__

#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>

//For UStringToHex
#include <ctype.h>
#include <stdio.h>
#include "AIChunkEx.h"


#define START_AIPROXY_NAMESPACE namespace AIProxy {
#define END_AIPROXY_NAMESPACE }

#define AIPROXY AIProxy

START_AIPROXY_NAMESPACE

///start const variable define
//max name length
const int AI_MAX_NAME_LEN  = 64;

const int AI_IP_ADDR_LEN  = 20;

const int AI_MAX_PASSWORD_LEN  = 64;

//max path length
const int AI_MAX_PATH_LEN  = PATH_MAX;

//max event count for AIProxyChannel event
const int AI_MAX_EVENT_CNT = 10;

//max msgid length for AIProxyProtocol
const int AI_MAX_MSGID_LEN = 20;
///end const

///start errno
const int AI_NO_ERROR              = 0;
const int AI_ERROR_SERVER_NO_FOUND = -80301;
const int AI_ERROR_CONN_NO_FOUND   = -80302;
const int AI_ERROR_NO_ENOUGH_SPACE = -80303;
const int AI_ERROR_CONNECTE        = -80304;
const int AI_ERROR_AUTHENTICATE    = -80305;
const int AI_ERROR_SEND            = -80306;
const int AI_ERROR_RECV            = -80307;
const int AI_ERROR_TIMEOUT         = -80308;
const int AI_ERROR_MSGID_DUAL      = -80309;
const int AI_ERROR_MSGID_NO_FOUND  = -80310;
const int AI_ERROR_ROUTING         = -80311;
const int AI_ERROR_PTL_DECODE      = -80312; //PTL : protocol
const int AI_ERROR_PTL_ENCODE      = -80313; //PTL : protocol
const int AI_ERROR_SYSTEM          = -80314;
///end errno

#define AI_PROXY_TRACE( format, ... )
//fprintf( stderr, "TRACE: " format "\n", __VA_ARGS__ );
#define AI_PROXY_INFO( format, ... ) fprintf( stderr, "INFO: " format "\n", __VA_ARGS__ );
#define AI_PROXY_ERROR( format, ... ) fprintf( stderr, "ERROR: " format "\n", __VA_ARGS__ );

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

inline const char* UStringToHex( const char* aspBuff, size_t aiSize, AIChunkEx& aoOut, size_t aiColumn = 4, size_t aiGroup = 4 )
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
	unsigned const char*  lpsCurr = (unsigned const char*)aspBuff;
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

END_AIPROXY_NAMESPACE

#endif //__AIPROXY__UTILITY_H__
