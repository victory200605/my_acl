
//#define AI_TEST_DEBUG 1;

#include "acl/DateTime.h"
#include "acl/MemoryBlock.h"
#include "acl/SockAddr.h"
#include "acl/stl/string.h"
#include "acl/IniConfig.h"
#include "cacheserver/CacheClient.h"
#include "./atf/TestCase.h"
#include "./atf/TestAction.h"
#include "./atf/TestParallel.h"
#include "./atf/TestControl.h"
#include "./atf/TestSnapshot.h"

#define CONFIG_GET_INT( val, config, field, name ) \
    { \
        acl::CIniConfig::KeyIterType loIter = config.GetValue(field, name); \
        if (loIter.IsEmpty() ) \
        { \
            apl_errprintf("CacheClient Config field (%s.%s) unexisted\n", field, name); \
            return -1; \
        } \
        val = loIter.ToInt(); \
    }
#define CONFIG_GET_STR( val, config, field, name ) \
    { \
        acl::CIniConfig::KeyIterType loIter = config.GetValue(field, name); \
        if (loIter.IsEmpty() ) \
        { \
            apl_errprintf("CacheClient Config field (%s.%s) unexisted\n", field, name); \
            return -1; \
        } \
        val = loIter.ToString(); \
    }

cacheserver::CCacheClient goCacheClient;
class CKey*                gpoKey = NULL;
bool                       gbIsTerminate = false;

//------------------------------------CKey-------------------------------//
//use for gen message id
class CKey
{
public:
    CKey( apl_int_t aiStart = 0, apl_int_t aiStep = 1 ) 
        : miCount(aiStart)
        , miStep(aiStep)
    {
    }

    virtual ~CKey(void) 
    {
    }

    virtual apl_int_t GetKey( char* apcKeyBuff, apl_size_t auLen )
    {
        acl::TSmartLock<acl::CLock> loGuard(this->moLock);

        apl_snprintf( apcKeyBuff, auLen, "0625091001%010d", this->miCount );
        this->miCount += this->miStep;
        return 0;
    }
    
protected:
    apl_int_t miCount;
    apl_int_t miStep;
    acl::CLock moLock;
};

//------------------------------------CFileKey-------------------------------//
//use for gen message id from file
class CFileKey : public CKey
{
public:
    CFileKey( const char* apcFileName )
    {
        this->mpFile = fopen( apcFileName, "r" );
        if (this->mpFile == NULL)
        {
            apl_errprintf( "Error:Open file %s fail - %s\n", apcFileName, apl_strerror( apl_get_errno() ) );
        }

        ACL_ASSERT(this->mpFile != NULL);
    }
    
    virtual ~CFileKey(void)
    {
        fclose(this->mpFile);
    }

    virtual apl_int_t GetKey( char* apcKeyBuff, apl_size_t auLen )
    {
        apl_int_t liRet = 0;
        acl::TSmartLock<acl::CLock> loGuard(this->moLock);

        if ( fgets(apcKeyBuff, auLen, this->mpFile) == NULL )
        {
            liRet = -1;
        }

        if ( apcKeyBuff[apl_strlen(apcKeyBuff) - 1] == '\n' ) 
        {
            apcKeyBuff[apl_strlen(apcKeyBuff) - 1] = 0;
        }

        if ( apcKeyBuff[apl_strlen(apcKeyBuff) - 1] == '\r' ) 
        {
            apcKeyBuff[apl_strlen(apcKeyBuff) - 1] = 0;
        }
        
        return liRet;
    }

protected:
    FILE* mpFile;
    acl::CLock moLock;
};

class AIInsertTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        char lacKey[40];
        char lacValue[256];
        apl_memset(lacValue, 'A', 256);
        
        if (gpoKey->GetKey(lacKey, sizeof(lacKey) ) != 0 )
        {
            return -1;
        }

        apl_size_t luLen = apl_strlen(lacKey);
        apl_memcpy( lacValue, lacKey, luLen );
        apl_memcpy( lacValue + sizeof(lacValue) - luLen - 1, lacKey, luLen + 1 );
        
        apl_int_t liRet = goCacheClient.Put( lacKey, lacValue, sizeof(lacValue), 10 );
        if ( liRet != 0 )
        {
            //if ( lpoParam->cbIsTerminate )
            {
                apl_errprintf( "Insert data fail, MSGCODE:%"APL_PRIdINT", [key=%s]\n", liRet, lacKey );
            }
            return -1;
        }
        
        return 0;
    }
};

class AIDelTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        char lacKey[40];
        acl::CMemoryBlock loValue;
        
        if (gpoKey->GetKey(lacKey, sizeof(lacKey) ) != 0 )
        {
            return -1;
        }
        
        apl_int_t liRet = goCacheClient.Get(lacKey, &loValue, 10 );
        if ( liRet != 0 )
        {
            //if ( lpoParam->cbIsTerminate )
            {
                apl_errprintf( "Delete data fail, MSGCODE:%"APL_PRIdINT", [key=%s]\n", liRet, lacKey );
            }

            return -1;
        }
        else
        {
            if ( gbIsTerminate )
            {
                printf( "Delete data success, [key=%s]/[value=%s]\n", lacKey, loValue.GetReadPtr() );
            }
        }
        
        return liRet;
    }
};

class AIDelTimeoutTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        acl::CMemoryBlock loKey;
        acl::CMemoryBlock loValue;
        
        apl_int_t liRet = goCacheClient.GetTimeout(&loKey, &loValue, 10);
        if ( liRet != 0 )
        {
            if ( gbIsTerminate )
            {
                apl_errprintf( "Delete data fail, MSGCODE:%"APL_PRIdINT", [key=%s]\n", liRet, loKey.GetReadPtr() );
            }

            return -1;
        }
        else
        {
            if ( gbIsTerminate )
            {
                printf( "Delete data success, [key=%s]/[value=%s]\n", loKey.GetReadPtr(), loValue.GetReadPtr() );
            }
        }
        
        return liRet;
    }
};

apl_int_t Initialize( char const* apcConfig, cacheserver::CCacheClient* apoClient )
{
    acl::CIniConfig loConfig;

    if (loConfig.Open(apcConfig) != 0)
    {
        return -1;
    }

    //Load cache server info
    for (acl::CIniConfig::FieldIterType loField = loConfig.Begin("CacheServer[0-9]+");
         loField != loConfig.End(); ++loField )
    {
        apl_int_t   liWindSize = 0;
        apl_int_t   liTimeout = 0;
        std::string loIpAddress;
        apl_int_t   liPort = 0;
        apl_int_t   liConnNum = 0;
        apl_int_t   liProcessorPoolSize = 0;
        apl_int_t   liHashLower = 0;
        apl_int_t   liHashUpper = 0;
        
        CONFIG_GET_INT(liWindSize, loConfig, loField.GetFieldName(), "WindSize");
        CONFIG_GET_INT(liTimeout, loConfig, loField.GetFieldName(), "ConnTimeout");
        CONFIG_GET_INT(liConnNum, loConfig, loField.GetFieldName(), "ConnNum");
        CONFIG_GET_INT(liProcessorPoolSize, loConfig, loField.GetFieldName(), "ProcessorPoolSize");
        CONFIG_GET_STR(loIpAddress, loConfig, loField.GetFieldName(), "IpAddress");
        CONFIG_GET_INT(liPort, loConfig, loField.GetFieldName(), "Port");
        CONFIG_GET_INT(liHashLower, loConfig, loField.GetFieldName(), "HashLower");
        CONFIG_GET_INT(liHashUpper, loConfig, loField.GetFieldName(), "HashUpper");


        apoClient->AddServer(
             acl::CSockAddr(liPort, loIpAddress.c_str() ), 
             liConnNum, 
             liProcessorPoolSize, 
             liHashLower, 
             liHashUpper, 
             liTimeout, 
             liWindSize );
    }

    return apoClient->Initialize();
}
int main( int argc, char* argv[] )
{
    apl_int_t liOpValue = 0;
    apl_int_t liStatisticalTimeInterval = 0;
    apl_int_t liMaxUnitFlow = 0;
    apl_int_t liThreadCnt = 1;
    apl_int_t liCount = 0;
    apl_int_t liStartKey = 0;
    apl_int_t liOptChar;
    apl_int_t liStep = 1;
    
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
			    gpoKey = new CFileKey(optarg);
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
			    gbIsTerminate = true;
			    break;
			case 'e':
			    liStep = atoi(optarg);
			    break;
			default:
			    printf("x");
			    break;
		}
	}
	if (gpoKey == NULL )
	{
	    gpoKey = new CKey( liStartKey, liStep );
	}
	
	if (Initialize("../config/CacheClientV3.ini", &goCacheClient) != 0 )
	{
	    printf( "Cache client initialize fail, may be connect rejected - %s\n", strerror(errno) );
	    return -1;
	}
	
    apl_sleep(APL_TIME_SEC);
	
    atf::CTestCase* lpCase = NULL;
	switch ( liOpValue )
	{
	    case 0:
	        lpCase = new AIInsertTestCase;
	        break;
	    case 1:
	        lpCase = new AIDelTestCase;
	        break;
        case 2:
            apl_errprintf("unimplement\n");
            return -1;
	    case 3:
	        lpCase = new AIDelTimeoutTestCase;
	        break;
        case 4:
            apl_errprintf("unimplement\n");
            return -1;
	};
	
	lpCase->SetRepeatCount(liCount);
	
	
	atf::CTestAction* lpTestAction = NULL;
	if ( liThreadCnt > 1 )
	{
	    lpTestAction = new atf::CTestParallel(liThreadCnt);
	}
	else
	{
	    lpTestAction = new atf::CTestAction;
	}
	atf::CTestSnapshot* lpSnapshot = NULL;
	if ( liStatisticalTimeInterval > 0 )
	{
	    lpSnapshot = new atf::CTestSnapshot( lpTestAction,  liStatisticalTimeInterval );
	    lpTestAction = lpSnapshot;
	}
	
	if ( liMaxUnitFlow > 0 )
	{
	    lpTestAction = new atf::CTestControl( lpTestAction,  1, liMaxUnitFlow );
	}

    lpTestAction->SetTestCase( lpCase );
	lpTestAction->Run(NULL);
    
    printf( "--------------------------Complete Test--------------------------------\n" );
	printf( " Total Record count              = %"APL_PRIdINT"\n",   lpCase->GetSuccess() + lpCase->GetReject() );
	printf( " Success Record count            = %"APL_PRIdINT"\n",   lpCase->GetSuccess() );
	printf( " Reject Record count             = %"APL_PRIdINT"\n",   lpCase->GetReject() );
	printf( " Total use time(s)               = %lf\n",   lpCase->GetUseTime() );
	if ( lpSnapshot != NULL )
	{
	    printf( " Avg delay time(us)              = %"APL_PRIdINT"\n", lpSnapshot->GetAvgDelayTime() );
	    printf( " Max delay time(us)              = %"APL_PRIdINT"\n", lpSnapshot->GetMaxDelayTime() );
	    printf( " Min delay time(us)              = %"APL_PRIdINT"\n", lpSnapshot->GetMinDelayTime() );
	}
	printf( " Record count per                = %lf/s\n", lpCase->GetFrequency() );
	
    goCacheClient.Close();

    return 0;
}

