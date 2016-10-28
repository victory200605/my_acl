
//#define AI_TEST_DEBUG 1;

#include "../include/CacheClient.h"
#include "../include/Utility.h"
#include "./AITest/TestCase.h"
#include "./AITest/TestAction.h"
#include "./AITest/TestParallel.h"
#include "./AITest/TestControl.h"
#include "./AITest/TestSnapshot.h"
#include <string.h>
#include <errno.h>
#include "AIProcBase.h"

int giTimeCount = 0;

//------------------------------------clsKey-------------------------------//
//use for gen message id
class clsKey
{
public:
    clsKey( int aiStart = 0, int aiStep = 1 ) : ciCount(aiStart),ciStep(aiStep)   {}
    virtual ~clsKey() {}
    virtual int GetKey( char* apsKeyBuff, size_t aiLen )
    {
        AI_MUTEX_LOCK( coLock );
        char lcTimeStr[15];
        FormatTime( 1214356201, lcTimeStr, "%d%02d%02d%02d%02d%02d" );
        snprintf( apsKeyBuff, aiLen, "%s%010d", lcTimeStr + 4, ciCount );
        ciCount += ciStep;
        AI_MUTEX_UNLOCK( coLock );
        return 0;
    }
    
    char* FormatTime(const time_t aiTimeValue, char* apsBuff, const char* apsFromat )
    {
    	struct tm tmT=*localtime(&aiTimeValue);
    	sprintf(apsBuff,apsFromat,tmT.tm_year+1900,tmT.tm_mon+1,tmT.tm_mday,
    		tmT.tm_hour,tmT.tm_min,tmT.tm_sec);
    	return apsBuff;
    }
    
protected:
    int ciCount;
    int ciStep;
    AIBC::AIMutexLock coLock;
};
//------------------------------------clsFileKey-------------------------------//
//use for gen message id from file
class clsFileKey : public clsKey
{
public:
    clsFileKey( const char* apsFileName )
    {
        cpFile = fopen( apsFileName, "r" );
        if ( cpFile == NULL ) printf( "Error:Open file %s fail - %s\n", apsFileName, strerror(errno) );
        assert ( cpFile != NULL );
    }
    virtual ~clsFileKey() 
    {
        fclose(cpFile);
    }
    virtual int GetKey( char* apsKeyBuff, size_t aiLen )
    {
        int liRet = 0;
        AI_MUTEX_LOCK( coLock );
        if ( fgets( apsKeyBuff, aiLen, cpFile ) == NULL )
        {
            liRet = -1;
        }
        if ( apsKeyBuff[strlen(apsKeyBuff) - 1] == '\n' ) 
        {
            apsKeyBuff[strlen(apsKeyBuff) - 1] = 0;
        }
        if ( apsKeyBuff[strlen(apsKeyBuff) - 1] == '\r' ) 
        {
            apsKeyBuff[strlen(apsKeyBuff) - 1] = 0;
        }
        AI_MUTEX_UNLOCK( coLock );
        return liRet;
    }

protected:
    FILE* cpFile;
    AIBC::AIMutexLock coLock;
};

//------------------------------------stParam-------------------------------//
//global param struct
struct stParam
{
    clsCacheClient coCacheClient;
    clsKey*     cpoKey;
    bool        cbIsTerminate;
};

class AIInsertTestCase : public AITest::AITestCase
{
public:
    virtual int DoIt( void* apParam )
    {
        char lcKey[40];
        char lcValue[256];
        memset( lcValue, 'A', 256 );
        AIBC::AIChunkEx loValue;
        
        stParam* lpoParam = (stParam*)apParam;
        if ( lpoParam->cpoKey->GetKey( lcKey, sizeof(lcKey) ) != 0 )
        {
            return -1;
        }
        size_t liLen = strlen(lcKey);
        memcpy( lcValue, lcKey, liLen );
        memcpy( lcValue + sizeof(lcValue) - liLen - 1, lcKey, liLen + 1 );
        
        int liRet = lpoParam->coCacheClient.Put( lcKey, lcValue, sizeof(lcValue) );
        if ( liRet != 0 )
        {
            if ( lpoParam->cbIsTerminate )
            {
                printf( "Insert data fail, MSGCODE:%d, [key=%s]\n", liRet, lcKey );
            }
            return -1;
        }
        
        return 0;
    }
};

class AIUpdateTestCase : public AITest::AITestCase
{
public:
    virtual int DoIt( void* apParam )
    {
        char lcKey[40];
        char lcValue[256];
        memset( lcValue, 'B', 256 );
        AIBC::AIChunkEx loValue;
        
        stParam* lpoParam = (stParam*)apParam;
        if ( lpoParam->cpoKey->GetKey( lcKey, sizeof(lcKey) ) != 0 )
        {
            return -1;
        }
        
        size_t liLen = strlen(lcKey);
        memcpy( lcValue, lcKey, liLen );
        memcpy( lcValue + sizeof(lcValue) - liLen - 1, lcKey, liLen + 1 );
        
        int liRet = lpoParam->coCacheClient.Update( lcKey, lcValue, sizeof(lcValue) );
        if ( liRet != 0 )
        {
            if ( lpoParam->cbIsTerminate )
            {
                printf( "Update data fail, MSGCODE:%d, [key=%s]\n", liRet, lcKey );
            }
            return -1;
        }
        
        return 0;
    }
};

class AIDelTestCase : public AITest::AITestCase
{
public:
    virtual int DoIt( void* apParam )
    {
        char lcKey[40];
        AIBC::AIChunkEx loValue;
        
        stParam* lpoParam = (stParam*)apParam;
        if ( lpoParam->cpoKey->GetKey( lcKey, sizeof(lcKey) ) != 0 )
        {
            return -1;
        }
        
        int liRet = lpoParam->coCacheClient.Get( lcKey, loValue );
        if ( liRet != 0 )
        {
            if ( lpoParam->cbIsTerminate )
            {
                printf( "Delete data fail, MSGCODE:%d, [key=%s]\n", liRet, lcKey );
            }
            return -1;
        }
        else
        {
            if ( lpoParam->cbIsTerminate )
            {
                printf( "Delete data success, [key=%s]/[value=%s]\n", lcKey, loValue.GetPointer() );
            }
        }
        
        return liRet;
    }
};

class AIDelTimeoutTestCase : public AITest::AITestCase
{
public:
    virtual int DoIt( void* apParam )
    {
        AIBC::AIChunkEx loKey;
        AIBC::AIChunkEx loValue;
        
        stParam* lpoParam = (stParam*)apParam;
        int liRet = lpoParam->coCacheClient.GetTimeout( loKey, loValue );
        if ( liRet != 0 )
        {
            if ( lpoParam->cbIsTerminate )
            {
                printf( "Delete data fail, MSGCODE:%d, [key=%s]\n", liRet, loKey.BasePtr() );
            }
            return -1;
        }
        else
        {
            if ( lpoParam->cbIsTerminate )
            {
                printf( "Delete data success, [key=%s]/[value=%s]\n", loKey.BasePtr(), loValue.GetPointer() );
            }
        }
        
        return liRet;
    }
};

class AIStatTestCase : public AITest::AITestCase
{
public:
    virtual int DoIt( void* apParam )
    {
        int liRet = 0;
        int liCapacity = 0;
        int liSize = 0;
        int liFileCount = 0;
                
        stParam* lpoParam = (stParam*)apParam;
        
        for ( int liN = 0; liN < lpoParam->coCacheClient.GetServerCount(); liN++ )
        {
            liRet = lpoParam->coCacheClient.GetStat( liN, &liCapacity, &liSize, &liFileCount );
            if ( liRet != 0 )
            {
                printf( "Getstat data fail, MSGCODE:%d\n", liRet );
                return -1;
            }
            else
            {
                 printf( "Getstat data success, [server=%d]/[capacity=%d]/[size=%d]/[filecount=%d]\n",
                    liN, liCapacity, liSize, liFileCount );
            }
        }
        
        return liRet;
    }
};

int main( int argc, char* argv[] )
{
    AIBC::AIInitIniHandler();
    AIBC::AIInitLOGHandler();
    
    int liOpValue = 0;
    int liStatisticalTimeInterval = 0;
    int liMaxUnitFlow = 0;
    int liThreadCnt = 1;
    int liCount = 0;
    int liStartKey = 0;
    int liOptChar;
    int liStep = 1;
    
    stParam loParam;
    loParam.cpoKey = NULL;
    loParam.cbIsTerminate = false;
    while( (liOptChar = getopt(argc, argv, "idl:s:f:t:n:k:poe:uq" )) != EOF )
	{
		switch (liOptChar)
		{
			case 'i':
			    liOpValue = 0;
			    break;
			case 'd':
			    liOpValue = 1;
				break;
		    case 'u':
			    liOpValue = 2;
			    break;
			case 'o':
			    liOpValue = 3;
				break;
		    case 'q':
			    liOpValue = 4;
				break;
			case 's':
				liStatisticalTimeInterval = atoi(optarg);
				break;
			case 'f':
			    loParam.cpoKey = new clsFileKey(optarg);
			    break;
			case 't':
			    liThreadCnt = atoi(optarg);
			    break;
			case 'l':
			    liCount = atoi(optarg);
			    break;
			case 'n':
			    liMaxUnitFlow = atoi(optarg);
			    break;
			case 'k':
			    liStartKey = atoi(optarg);
			    break;
			case 'p':
			    loParam.cbIsTerminate = true;
			    break;
			case 'e':
			    liStep = atoi(optarg);
			    break;
			default:
			    printf("x");
			    break;
		}
	}
	if ( loParam.cpoKey == NULL )
	{
	    loParam.cpoKey = new clsKey( liStartKey, liStep );
	}
	
	if ( loParam.coCacheClient.Initialize() != 0 )
	{
	    printf( "Cache client initialize fail, may be connect rejected - %s\n", strerror(errno) );
	    return -1;
	}
	
	AITest::AITestCase* lpCase = NULL;
	switch ( liOpValue )
	{
	    case 0:
	        lpCase = new AIInsertTestCase;
	        break;
	    case 1:
	        lpCase = new AIDelTestCase;
	        break;
	    case 2:
	        lpCase = new AIUpdateTestCase;
	        break;
	    case 3:
	        lpCase = new AIDelTimeoutTestCase;
	        break;
	    case 4:
	        lpCase = new AIStatTestCase;
	        break;
	};
	
	lpCase->SetRepeatCount(liCount);
	
	
	AITest::AITestAction* lpTestAction = NULL;
	if ( liThreadCnt > 1 )
	{
	    lpTestAction = new AITest::AITestParallel(liThreadCnt);
	}
	else
	{
	    lpTestAction = new AITest::AITestAction;
	}
	AITest::AITestSnapshot* lpSnapshot = NULL;
	if ( liStatisticalTimeInterval > 0 )
	{
	    lpSnapshot = new AITest::AITestSnapshot( lpTestAction,  liStatisticalTimeInterval );
	    lpTestAction = lpSnapshot;
	}
	
	if ( liMaxUnitFlow > 0 )
	{
	    lpTestAction = new AITest::AITestControl( lpTestAction,  1, liMaxUnitFlow );
	}

    lpTestAction->SetTestCase( lpCase );
	lpTestAction->Run( &loParam );
    
    printf( "--------------------------Complete Test--------------------------------\n" );
	printf( " Total Record count              = %d\n",   lpCase->GetSuccess() + lpCase->GetReject() );
	printf( " Success Record count            = %d\n",   lpCase->GetSuccess() );
	printf( " Reject Record count             = %d\n",   lpCase->GetReject() );
	printf( " TimeOut Record count            = %d\n",   giTimeCount );
	printf( " Total use time(s)               = %lf\n",   lpCase->GetUseTime() );
	if ( lpSnapshot != NULL )
	{
	    printf( " Avg delay time(us)              = %d\n", lpSnapshot->GetAvgDelayTime() );
	    printf( " Max delay time(us)              = %d\n", lpSnapshot->GetMaxDelayTime() );
	    printf( " Min delay time(us)              = %d\n", lpSnapshot->GetMinDelayTime() );
	}
	printf( " Record count per                = %lf/s\n", lpCase->GetFrequency() );
	
	AIBC::AICloseLOGHandler();
    AIBC::AICloseIniHandler();

    return 0;
}

AIBC_NAMESPACE_START
DECLARE_CMDOPT_BEGIN()
DECLARE_CMDOPT_END()
AIBC_NAMESPACE_END
