
#ifndef AI_UTILITY_H_2008
#define AI_UTILITY_H_2008
 
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "AIChunkEx.h"
#include "../include/Config.h"
#include "../../GlobalFileQueue/include/Errno.h"

AI_GFQ_NAMESPACE_START

typedef unsigned long ptrdiff_t;    // evc3, PocketPC don't defined ptrdiff_t

#define atoll _atoi64

inline const char* IntToString( int64_t aiValue, char* apsRet, size_t aiSize )
{
	assert( aiSize > 2 );
	memset( apsRet, 0, aiSize );

	size_t liCurr = aiSize - 2;
	bool   lbIsNegative = aiValue < 0 ? true : false;

	do
	{
		int32_t aiTmp = aiValue % 10;
		aiTmp = abs( aiTmp );
		
		apsRet[liCurr--] = aiTmp + '0';

		aiValue = aiValue / 10;
	}while( liCurr > 0 && aiValue != 0 );

	assert( aiValue == 0 );

	if ( lbIsNegative ) apsRet[liCurr] = '-';
	else liCurr++;

	memmove( apsRet, apsRet + liCurr, aiSize - liCurr );

	return apsRet;
}

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

inline const char* UStringToHex( char* aspBuff, size_t aiSize, AIChunkEx& aoOut, size_t aiColumn = 4, size_t aiGroup = 4 )
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

inline const char* CharToBinary( unsigned char aiCh, char* apsRet, size_t aiSize )
{
	assert( aiSize > 8 );
	memset( apsRet, 0, aiSize );

	int32_t liLen = 8;
	memset( apsRet, '0', liLen );
	do 
	{
		char lcTmp = aiCh % 2;
		apsRet[--liLen] = lcTmp + 48;
		aiCh = aiCh / 2;
	}while( aiCh != 0 );

	return apsRet;
}

inline size_t HashString( const char* apsString, size_t aiLength )
{
	assert( apsString != NULL );
	//unsigned long llH = 0; 
	unsigned int	liRetValue = 1;
	unsigned int	liTempValue = 4;

	while( aiLength-- ) /* from MySQL */
	{ 
		liRetValue ^= (((liRetValue & 63)+liTempValue)*((unsigned int)(unsigned char) *apsString++))+ (liRetValue << 8);
		liTempValue += 3; 
	} 
	
	return liRetValue; 
}

inline void StringCopy(char* apcDestStr, char const* apcSrcStr, size_t aiSize)
{
    const char  *lpcPtr1 = apcSrcStr;
    char        *lpcPtr2 = apcDestStr;

    *lpcPtr2=0;
    while ((*lpcPtr1) && (lpcPtr2 - apcDestStr) < (ptrdiff_t)(aiSize-1) )
    {
        *lpcPtr2++ = *lpcPtr1++;
        *lpcPtr2 = 0;
    }
}

#define AI_DUAL_UNLINK( node )                         \
	if ( node.cpPrev != NULL && node.cpNext != NULL )  \
	{                                                  \
		node.cpPrev->cpNext = node.cpNext;             \
    	node.cpNext->cpPrev = node.cpPrev;             \
    	node.cpPrev = NULL;                            \
    	node.cpNext = NULL;                            \
	}

/* Define retry count */
#define AI_OP_RETRY  2

#define OP_RETRY_N( op, retrynum )                     \
    do                                                 \
    {                                                  \
        for( int liIt = 0; liIt < retrynum; liIt++ )   \
        {                                              \
            if ( !(op) ) break;                        \
        }                                              \
    }while(false)

#define OP_RETRY( op )  OP_RETRY_N( op, AI_OP_RETRY )

// Define AI_RETURN_IF
#ifndef AI_RETURN_IF
#   define AI_RETURN_IF( ret, op ) if(op) { return -1; }
#endif

AI_GFQ_NAMESPACE_END

#endif //AI_UTILITY_H_2008
