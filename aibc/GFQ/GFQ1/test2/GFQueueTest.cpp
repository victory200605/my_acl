
//#define AI_TEST_DEBUG 1;

#include "acl/DateTime.h"
#include "acl/SockAddr.h"
#include "acl/MemoryBlock.h"
#include "acl/stl/string.h"
#include "acl/IniConfig.h"
#include "gfq/GFQueueClient.h"
#include "./atf/TestCase.h"
#include "./atf/TestAction.h"
#include "./atf/TestParallel.h"
#include "./atf/TestControl.h"
#include "./atf/TestSnapshot.h"

bool             gbIsTerminate = false;
apl_int_t        giPort = 0;
apl_int_t        giWhenSec = 0;
apl_int_t        giConnNum = 1;
class CKey*      gpoKey = NULL;
std::string      goIpAddress("127.0.0.1");
std::string      goQueueName("test");
gfq::CGFQueueClient goGFQueueClient;

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
        acl::CDateTime loDate;
        loDate.Update();
        
        acl::TSmartLock<acl::CLock> loGuard(this->moLock);

        apl_snprintf( apcKeyBuff, auLen, "%s%010d", loDate.Format("%m%d%H%M%S"), this->miCount );
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
        acl::CMemoryBlock loValue(256);

        apl_memset( loValue.GetWritePtr(), '0', loValue.GetSize() );
        
        if (gpoKey->GetKey(loValue.GetWritePtr(), loValue.GetSize() ) != 0 )
        {
            return -1;
        }
        
        loValue.SetWritePtr(256);
        
        apl_int_t liRet = goGFQueueClient.Put(goQueueName.c_str(), loValue);
        if ( liRet != 0 )
        {
            //if ( lpoParam->cbIsTerminate )
            {
                printf( "Insert data fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
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
        acl::CMemoryBlock loValue;
        
        apl_int_t liRet = goGFQueueClient.Get(goQueueName.c_str(), &loValue, giWhenSec);
        if ( liRet != 0 )
        {
            //if ( lpoParam->cbIsTerminate )
            {
                printf( "Delete data fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
            }
            return -1;
        }
        else
        {
            if (gbIsTerminate )
            {
                printf( "Delete data success, [value=%s]\n", loValue.GetReadPtr() );
            }
        }
        
        return liRet;
    }
};

class AIGetStatTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        apl_int_t liSize = 0;
        apl_int_t liCapacity = 0;

        apl_int_t liRet = goGFQueueClient.GetStat(goQueueName.c_str(), &liSize, &liCapacity);
        if ( liRet != 0 )
        {
            //if ( lpoParam->cbIsTerminate )
            {
                printf( "Get queue stat fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
            }
            return -1;
        }
        else
        {
            printf( "Get queue stat success, (Size=%"APL_PRIdINT", Capacity=%"APL_PRIdINT")\n", liSize, liCapacity);
        
        }
        
        return 0;
    }
};

class AIGetModuleCountTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        apl_int_t liSize = 0;

        apl_int_t liRet = goGFQueueClient.GetModuleCount(&liSize);
        if ( liRet != 0 )
        {
            //if ( lpoParam->cbIsTerminate )
            {
                printf( "Get module count fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
            }
            return -1;
        }
        else
        {
            printf( "Get module count success, (Size=%"APL_PRIdINT")\n", liSize);
        
        }
        
        return 0;
    }
};

class AIPrintModulesTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        apl_int_t liRet = goGFQueueClient.PrintModules();
        if ( liRet != 0 )
        {
            //if ( lpoParam->cbIsTerminate )
            {
                printf( "Print modules info fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
            }
            return -1;
        }
        else
        {
            printf( "Print modules info success\n");
        
        }
        
        return 0;
    }
};

class AIPrintQueuesTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        apl_int_t liRet = goGFQueueClient.PrintQueues();
        if ( liRet != 0 )
        {
            //if ( lpoParam->cbIsTerminate )
            {
                printf( "Print queues info fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
            }
            return -1;
        }
        else
        {
            printf( "Print queues info success\n");
        
        }
        
        return 0;
    }
};

class AIReloadQueueInfoTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        apl_int_t liRet = goGFQueueClient.ReloadQueueInfo();
        if ( liRet != 0 )
        {
            //if ( lpoParam->cbIsTerminate )
            {
                printf( "Reload queues info fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
            }
            return -1;
        }
        else
        {
            printf( "Reload queue info success\n");
        
        }
        
        return 0;
    }
};

int main( apl_int_t argc, char* argv[] )
{
    apl_int_t liStatisticalTimeInterval = 0;
    apl_int_t liMaxUnitFlow = 0;
    apl_int_t liThreadCnt = 1;
    apl_int_t liCount = 0;
    apl_int_t liStartKey = 0;
    apl_int_t liOptChar;
    apl_int_t liStep = 1;
    std::string loOpt = "put";
    
    while( (liOptChar = getopt(argc, argv, "m:p:c:x:w:l:s:f:t:n:k:e:q:o" )) != EOF )
	{
		switch (liOptChar)
		{
            case 'm':
                goIpAddress = optarg;
                break;
            case 'p':
                giPort = atoi(optarg);
                break;
            case 'c':
                giConnNum = atoi(optarg);
                break;
			case 'x':
			    loOpt = optarg;
			    break;
			case 'w':
                giWhenSec = atoi(optarg);
				break;
			case 'o':
			    gbIsTerminate = true;
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
			case 'e':
			    liStep = atoi(optarg);
			    break;
            case 'q':
                goQueueName = optarg;
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

    acl::CSockAddr loRemote(giPort, goIpAddress.c_str() );
    if (goGFQueueClient.Initialize("test", loRemote, giConnNum, 10) != 0)
	{
	    printf( "Cache client initialize fail, may be connect rejected - %s\n", strerror(errno) );
	    return -1;
	}
	
    apl_sleep(APL_TIME_SEC);
	
    atf::CTestCase* lpCase = NULL;
    if (loOpt == "put")
    {
	    lpCase = new AIInsertTestCase;
    }
    else if (loOpt == "get")
    {
	    lpCase = new AIDelTestCase;
    }
    else if (loOpt == "get-stat")
    {
	    lpCase = new AIGetStatTestCase;
    }
    else if (loOpt == "get-modulecount")
    {
	    lpCase = new AIGetModuleCountTestCase;
    }
    else if (loOpt == "print-modules")
    {
	    lpCase = new AIPrintModulesTestCase;
    }
    else if (loOpt == "print-queues")
    {
	    lpCase = new AIPrintQueuesTestCase;
    }
    else if (loOpt == "reload-queueinfo")
    {
	    lpCase = new AIReloadQueueInfoTestCase;
    }
    else
    {
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

    lpTestAction->SetTestCase(lpCase);
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
	
    return 0;
}

