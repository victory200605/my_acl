
#ifndef AI_UTILITY_H_2008
#define AI_UTILITY_H_2008

#include "AILib.h"
#include <assert.h>
#include <string.h>
#include <inttypes.h>

//////////////////////////////////////////////////////////////////////////////
// Ai lib header file config
#include "AIMemAlloc.h"
#include "AIAsyncTcp.h"
#include "AIConfig.h"
#include "AILogSys.h"
#include "AIDualLink.h"
// define macro first
#if !defined(AI_NEW_ARRAY_ASSERT)
#   define AI_NEW_ARRAY_ASSERT AI_NEW_N_ASSERT
#endif

#if !defined(AI_NEW_ARRAY)
#   define AI_NEW_ARRAY AI_NEW_N_ASSERT
#endif

#if !defined(AI_DELETE_ARRAY)
#   define AI_DELETE_ARRAY AI_DELETE_N
#endif

#if !defined(AI_RETURN_IF)
#   define AI_RETURN_IF(ret, exp) if(exp) { return ret; }
#endif

#if !defined(AI_NEW_INIT_ASSERT)
#   define AI_NEW_INIT_ASSERT(ptr, type, ...)  {try{ptr=new type(__VA_ARGS__);}catch(...){ptr=NULL;}assert(NULL!=ptr);}
#endif

#if !defined(AILOG_LEVEL_TRACE)
#   define AILOG_LEVEL_TRACE  AILOG_LEVEL_ANY
#endif

#define AI_GFQ_NAMESPACE_START namespace gfq {
#define AI_GFQ_NAMESPACE_END }

#include "AIFile.h"
#include "AIDir.h"
#include "AIShlib.h"
#include "AIMapFile.h"
#include "GFQHashTable.h"
#include "AIChunkEx.h"
#include "AISynch.h"
#include "AISynch.h"
#include "AIModule.h"
#include "AIThrPool.h"
//#include "AITask.h"
#include "stl/vector.h"

#if defined(_ENABLE_TRAP)
#   include "StatAPI.h"
#endif

AI_GFQ_NAMESPACE_START

#include "GFQConfig.inc"

// const variable define
// max file name length
const size_t AI_MAX_NAME_LEN  = 128;
const size_t AI_MAX_FILENAME_LEN = AI_MAX_NAME_LEN;

// max ip address length
const size_t AI_IP_ADDR_LEN   = 20;

// max ip address length
const size_t AI_MAX_INT_LEN   = 25;

const size_t AI_MAX_DATA_LEN = GFQ_DATA_LEN;

// module name hash size
const size_t AI_MODULE_HASH_SIZE  = 100000;

// queue name hash size
const size_t AI_QUEUE_HASH_SIZE   = 100000;
// single file max size

const size_t AI_MAX_S_FILE_SIZE   = 0x7FFFFFFF; //2GB
// end variable define

//Error number
enum {
	AI_NO_ERROR = 0,
	AI_ERROR_UNKNOW               = -80100, // unknow error

	AI_ERROR_OPEN_DIR             = -80110, // open dir fail
	AI_ERROR_OPEN_FILE            = -80111, // open file fail
	AI_ERROR_READ_FILE            = -80112, // read file fail
	AI_ERROR_WRITE_FILE           = -80113, // open file fail
	AI_ERROR_SYNC_FILE            = -80114, // sync file fail
	AI_ERROR_MAP_FILE             = -80115, // map file fail
	AI_ERROR_INVALID_FILE         = -80116, // invalid file fail
	AI_ERROR_INVALID_DATA         = -80117, // invalid data
	AI_ERROR_LOAD_SHLIB           = -80118, // load share lib fail

	AI_ERROR_CONNECT_REJECT       = -80120, // connect reject
	AI_ERROR_SOCKET_SEND          = -80121, // socket send data fail
	AI_ERROR_SOCKET_RECV          = -80122, // socket rev data fail
	AI_ERROR_OPEN_SERVER          = -80123, // open socket server fail
	AI_ERROR_INVALID_REQUST       = -80124, // invalid request
	
	AI_ERROR_OUT_OF_RANGE         = -80130, // out of range
	AI_ERROR_NO_FOUND_QUEUE       = -80132, // queue unexisted
	AI_ERROR_NO_FOUND_MODULE      = -80133, // module unexisted
	AI_ERROR_DUP_MODULE           = -80134, // dup module
	AI_ERROR_DUP_QUEUE            = -80135, // dup queue
	AI_ERROR_NO_ENOUGH_SPACE      = -80136, // queue no enough space
	AI_ERRRO_INVALID_QUEUE        = -80138,

	AI_ERROR_THREAD_POOL_FULL     = -80140, // thread pool full
	AI_ERROR_SYSTEM               = -80141,  // system error
	
	AI_ERROR_QUEUEINFO_LOAD_FUNC_UNDEFINED = -80150, // queue info load func undefined
	AI_ERROR_PERMISION_FUNC_UNDEFINED      = -80151, // permision func undefined
	AI_ERROR_QUEUEINFO_LOAD                = -80152, // queue info load fail
	AI_ERROR_HEARTBEAT_TIMEOUT             = -80153,
	AI_ERROR_SERVER_DENIED                 = -80154,
	
	//warning
	AI_ERROR_EMPTY_OF_SEGMENT     = -80200, // empty of segment
	AI_ERROR_END_OF_SEGMENT       = -80201, // end of segment
	AI_ERROR_END_OF_QUEUE         = -80202, // end of queue
	AI_ERROR_PERMISION_DENIED     = -80203, // permision denied
	AI_ERROR_NO_FOUND_OUTTIME     = -80204  // no found outtime node
};

inline bool IsWarning( int aiErrNo )
{
    if ( AI_ERROR_NO_FOUND_QUEUE == aiErrNo || aiErrNo / 100 == -802 )
    {
        return true;
    }
    return false;
}

inline const char* StrError( int aiErrNo )
{
    switch(aiErrNo)
    {
        ///
        case AI_NO_ERROR:                            return "No Error";
        case AI_ERROR_UNKNOW:                        return "Unknow error";
        ///
	    case AI_ERROR_OPEN_FILE:                     return "Open file fail";
	    case AI_ERROR_OPEN_DIR:                      return "Open dir fail";
	    case AI_ERROR_WRITE_FILE:                    return "Write file fail";
	    case AI_ERROR_READ_FILE:                     return "Read file fail";
	    case AI_ERROR_SYNC_FILE:                     return "Sync file fail";
	    case AI_ERROR_MAP_FILE:                      return "Map file fail";
	    case AI_ERROR_LOAD_SHLIB:                    return "Load share lib fail";
	    case AI_ERROR_INVALID_FILE:                  return "Invalid file";
	    case AI_ERROR_INVALID_DATA:                  return "Invalid data";
	    ////
		case AI_ERROR_CONNECT_REJECT:                return "Connect rejected";
		case AI_ERROR_SOCKET_SEND:                   return "Send data fail";
		case AI_ERROR_SOCKET_RECV:                   return "Rev data fail";
		case AI_ERROR_OPEN_SERVER:                   return "Open socket server fail";
		case AI_ERRRO_INVALID_QUEUE:                 return "Invalid queue create request";
		///
	    case AI_ERROR_END_OF_QUEUE:                  return "End of queue";
	    case AI_ERROR_END_OF_SEGMENT:                return "End of segment";
	    case AI_ERROR_EMPTY_OF_SEGMENT:              return "Empty of segment";
	    case AI_ERROR_NO_FOUND_OUTTIME:              return "No found timeout record";
	    case AI_ERROR_NO_FOUND_QUEUE:                return "Queue unexisted";
	    case AI_ERROR_NO_FOUND_MODULE:               return "Module unexisted";
	    case AI_ERROR_OUT_OF_RANGE:                  return "Out of range";
	    case AI_ERROR_DUP_MODULE:                    return "Store Module existed";
	    case AI_ERROR_PERMISION_DENIED:              return "Permision denied";
	    case AI_ERROR_DUP_QUEUE:                     return "Queue existed";
	    case AI_ERROR_NO_ENOUGH_SPACE:               return "No enough space";
	    case AI_ERROR_INVALID_REQUST:                return "Invalid protocol request";
	    ////
	    case AI_ERROR_QUEUEINFO_LOAD_FUNC_UNDEFINED: return "Queue info load function undefined";
	    case AI_ERROR_PERMISION_FUNC_UNDEFINED:      return "Permision function undefined";
	    case AI_ERROR_QUEUEINFO_LOAD:                return "Queue info load fail";
	    case AI_ERROR_HEARTBEAT_TIMEOUT:             return "Heart beat timeout";
	    case AI_ERROR_SERVER_DENIED:                 return "Server denied";
	        
	    ////
	    case AI_ERROR_THREAD_POOL_FULL:              return "Thread pool full";
	    case AI_ERROR_SYSTEM:                        return "System error";
	    
	    default:                                     return "Unknow error";
    }
}

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

////////////////////////////////////////////////////////////////////////////////////////////////
// ailib config
// macro define
#define AI_MUTEX_LOCK( mutex ) mutex.Lock();
#define AI_MUTEX_UNLOCK( mutex ) mutex.Unlock();
#define AI_CONDITION_WAIT( cond, lock ) cond.WaitUntil(lock, AI_TIME_FOREVER);
#define AI_CONDITION_WAIT_UNTIL( cond, lock, time ) cond.WaitUntil(lock, time);

/////////////////////////////// start utility for log //////////////////////////////////
// for trace log
#define AI_GFQ_TRACE( ... )  \
        if ( giGlobalLogLevel >= AILOG_LEVEL_TRACE ) \
            AIWriteLOG( NULL, AILOG_LEVEL_TRACE, __VA_ARGS__ );

// for err log
#define AI_GFQ_ERROR( ... )  \
    AIWriteLOG( NULL, AILOG_LEVEL_ERROR, __VA_ARGS__ );

// for debug log
#define AI_GFQ_DEBUG( ... )  \
    AIWriteLOG( NULL, AILOG_LEVEL_DEBUG, __VA_ARGS__ );

// for info log
#define AI_GFQ_INFO( ... )  \
    AIWriteLOG( NULL, AILOG_LEVEL_INFO, __VA_ARGS__ );

// for level log
#define AI_GFQ_LOG( ret, ... )  \
    AIWriteLOG( NULL, ( IsWarning(ret) ? AILOG_LEVEL_DEBUG : AILOG_LEVEL_ERROR ), __VA_ARGS__ );
/// end log define

///start timetrace define
struct stTimeTrace
{
    static stTimeTrace* Instance()
    {
        static stTimeTrace* slpoTimeTrace = NULL;
        if (slpoTimeTrace == NULL)
        {
            AI_NEW_ASSERT( slpoTimeTrace, stTimeTrace );
        }
        return slpoTimeTrace;
    }
    
    stTimeTrace() : ciInterval(0), ciFirstLevel(0), ciLastLevel(0) {}
    
    int ciInterval;
    int ciFirstLevel;
    int ciLastLevel;
};

// monitor thread func
inline void* TimeTraceThreadFunc( void* apvParam )
{
    FILE*  lpoFile = NULL;
    char   lsFileName[128] = {0};
    
    pthread_detach(pthread_self());
    
    if (apvParam == NULL)
    {
        snprintf(lsFileName, sizeof(lsFileName), "%s.ctrace", (const char*)apvParam);
    }
    else
    {
        snprintf(lsFileName, sizeof(lsFileName), "SegmentServer.ctrace");
    }
    
    while(true)
    {
        if ( ( lpoFile = fopen(lsFileName, "r") ) != NULL )
        {
            char lsBuffer[64] = {0};
            
            if ( fgets(lsBuffer, sizeof(lsBuffer), lpoFile ) )
            {
                do
                {   
                    char* lpcFirstLevel = strchr(lsBuffer, ' ');
                    if ( lpcFirstLevel != NULL 
                        && lpcFirstLevel[1] >= '0'
                        && lpcFirstLevel[1] <= '9' )
                    {
                        *lpcFirstLevel++ = '\0';
                        
                        char* lpcLastLevel = strchr(lpcFirstLevel, ' ');
                        if ( lpcLastLevel != NULL
                            && lpcLastLevel[1] >= '0'
                            && lpcLastLevel[1] <= '9' )
                        {
                            *lpcLastLevel++ = '\0';
    
                            stTimeTrace::Instance()->ciFirstLevel = atoi(lpcFirstLevel);
                            stTimeTrace::Instance()->ciLastLevel = atoi(lpcLastLevel);
                            stTimeTrace::Instance()->ciInterval = atoi(lsBuffer);
                            
                            AI_GFQ_INFO( "Set TimeTrace interval = %d, level = [%d, %d)", 
                                stTimeTrace::Instance()->ciInterval,
                                stTimeTrace::Instance()->ciFirstLevel,
                                stTimeTrace::Instance()->ciLastLevel );
                            
                            break;
                        }
                    }
                    
                    AI_GFQ_INFO( "Set TimeTrace format invalid" );
                }
                while(false);
            }
            
            fclose(lpoFile);
            
            remove(lsFileName);
        }

        sleep(2);
    }
    
    return NULL;
}

#define AI_GFQ_TIMETRACE( op, level, format, ... )  \
    { \
        stTimeTrace* lpoTimeTrace = stTimeTrace::Instance(); \
        if (lpoTimeTrace->ciInterval > 0) \
        { \
            AITime_t liStart = AICurTime(); \
            op; \
            int32_t  liUseTime = AICurTime() - liStart; \
            if ( liUseTime > lpoTimeTrace->ciInterval \
                && level >= lpoTimeTrace->ciFirstLevel \
                && level < lpoTimeTrace->ciLastLevel ) \
            { \
                AI_GFQ_INFO( "[time=%"PRId32"us] "format, liUseTime, __VA_ARGS__ ); \
            } \
        } \
        else \
        { \
            op; \
        } \
    }
///end timetrace define

///start trap define
#if defined(_ENABLE_TRAP)
    inline StatAPI* GetStatApi(void)
    {
        static StatAPI* slpoStatApi = NULL;
        if (slpoStatApi == NULL)
        {
            AI_NEW_ASSERT(slpoStatApi, StatAPI);
        }
        
        return slpoStatApi;
    }
    
#   define AI_GFQ_INITIALIZE_TRAP() GetStatApi()->Initial("../../config/StatAPI.ini", "../../config/StatTable.ini")
#   define AI_GFQ_TRAP( moduleid, type, title, ... )                              \
        {                                                                         \
            char lsBuffer[1024] = {0};                                            \
            snprintf(lsBuffer, sizeof(lsBuffer), __VA_ARGS__);                    \
            ColumnSet loWarnSet;                                                  \
            loWarnSet.SetColumnStr("sProgramName", moduleid, strlen(moduleid));   \
            loWarnSet.SetColumnInt("nType", type);                                \
            loWarnSet.SetColumnInt("cLevel", 0);                                  \
            loWarnSet.SetColumnStr("sTitle", title, strlen(title));               \
            loWarnSet.SetColumnStr("sContent", lsBuffer, strlen(lsBuffer));       \
            loWarnSet.SetColumnInt("nCounter", 1);                                \
                                                                                  \
            GetStatApi()->SendWarning(loWarnSet);                                 \
        }
#   define AI_GFQ_CLOSE_TRAP() GetStatApi()->ShutDown()
#else
#   define AI_GFQ_INITIALIZE_TRAP() 0
#   define AI_GFQ_TRAP( moduleid, type, title, ... )
#   define AI_GFQ_CLOSE_TRAP()
#endif
///end trap

AI_GFQ_NAMESPACE_END

#endif //AI_UTILITY_H_2008
