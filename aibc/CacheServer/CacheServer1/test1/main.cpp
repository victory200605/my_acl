
//#define AI_TEST_DEBUG 1;

#include "cacheserver/CacheClient.h"
#include "AILib.h"
#include "AILogSys.h"
#include "AIConfig.h"
#include "AISynch.h"
#include "./AITest/TestCase.h"
#include "./AITest/TestAction.h"
#include "./AITest/TestParallel.h"
#include "./AITest/TestControl.h"
#include "./AITest/TestSnapshot.h"
#include <string.h>
#include <errno.h>

int giTimeCount = 0;

int TimeoutCBFun(const char *apcKey, const void *apvValue, const int aiValueSize, void *apvParam)
{
    giTimeCount++;
	AIWriteLOG( NULL, AILOG_LEVEL_INFO, "Delete %s/%s", apcKey, (char*)apvValue );
	return 0;
}

//------------------------------------clsKey-------------------------------//
//use for gen message id
class clsKey
{
public:
    clsKey( int aiStart = 0, int aiStep = 1 ) : ciCount(aiStart),ciStep(aiStep)   {}
    virtual ~clsKey() {}
    virtual int GetKey( char* apsKeyBuff, size_t aiLen )
    {
        coLock.Lock();
        char lcTimeStr[15];
        FormatTime( 1214356201, lcTimeStr, "%d%02d%02d%02d%02d%02d" );
        snprintf( apsKeyBuff, aiLen, "%s%010d", lcTimeStr + 4, ciCount );
        ciCount += ciStep;
        coLock.Unlock();
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
    AIMutexLock coLock;
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
        coLock.Lock();
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
        coLock.Unlock();
        return liRet;
    }

protected:
    FILE* cpFile;
    AIMutexLock coLock;
};

//------------------------------------stParam-------------------------------//
//global param struct
struct stParam
{
    CacheClient coCacheClient;
    clsKey*     cpoKey;
    bool        cbIsTerminate;
};

class AIInsertTestCase : public AITest::AITestCase
{
public:
    virtual int DoIt( void* apParam )
    {
        char lcKey[40];
        char lcValue[512];
        memset( lcValue, 'A', 512 );
        AIChunk loValue;
        
        stParam* lpoParam = (stParam*)apParam;
        if ( lpoParam->cpoKey->GetKey( lcKey, sizeof(lcKey) ) != 0 )
        {
            return -1;
        }
        
        int liLen = strlen(lcKey);
        memcpy( lcValue, lcKey, liLen );
        memcpy( lcValue + 512 - liLen - 1, lcKey, liLen + 1 );
        
        int liRet = lpoParam->coCacheClient.SaveData( lcKey, lcValue, sizeof(lcValue) );
        if ( liRet != 0 )
        {
            if ( lpoParam->cbIsTerminate )
            {
                printf( "Insert data fail, MSGCODE:%d, [key=%s]\n", liRet, lcKey );
            }
            return -1;
        }
        else
        {
            if ( lpoParam->cbIsTerminate )
            {
                printf( "Insert data success, [key=%s]/[value=%s]\n", lcKey, lcValue );
            }
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
        char lcValue[512];
        memset( lcValue, 'A', 512 );
        AIChunk loValue;
        
        stParam* lpoParam = (stParam*)apParam;
        if ( lpoParam->cpoKey->GetKey( lcKey, sizeof(lcKey) ) != 0 )
        {
            return -1;
        }
        
        int liRet = lpoParam->coCacheClient.DeleteData( lcKey, &loValue );
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

int main( int argc, char *argv[] )
{
    
    bool lbInsert = true;
    bool lbDeleteTimeOut = false;
    int liStatisticalTimeInterval = 0;
    int liMaxUnitFlow = 0;
    int liThreadCnt = 1;
    int liCount = 0;
    int liStartKey = 0;
    int liOptChar;
    int liStep = 1;
    
    AIInitIniHandler();
    AIInitLOGHandler();

    stParam loParam;
    loParam.cpoKey = NULL;
    loParam.cbIsTerminate = false;
    while( (liOptChar = getopt(argc, argv, "idl:s:f:t:n:k:poe:" )) != EOF )
	{
		switch (liOptChar)
		{
			case 'i':
			    lbInsert = true;
			    break;
			case 'd':
			    lbInsert = false;
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
			case 'o':
			    lbDeleteTimeOut = true;
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
	
	if ( loParam.coCacheClient.Init( ( lbDeleteTimeOut ? TimeoutCBFun : NULL ), CACHE_TYPE_SM, NULL, 0, "../config/CacheClientV1.ini") != 0 )
	{
	    printf( "Cache client initialize fail, may be connect rejected - %s\n", strerror(errno) );
        return -1;
	}
	
	AITest::AITestCase* lpCase = NULL;
	if ( lbInsert )
	{
	    lpCase = new AIInsertTestCase;
	}
	else
	{
	    lpCase = new AIDelTestCase;
	}
	
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
    return 0;
}

#if defined(AI_OS_AIX)
    //Make compiler happy on AIX env
#   include "AIProcBase.h"
    ProgOptParam moProgOptArray[1];
#endif

