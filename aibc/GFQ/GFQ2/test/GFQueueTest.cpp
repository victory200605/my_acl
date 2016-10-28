
//#define AI_TEST_DEBUG 1;

#include "acl/DateTime.h"
#include "acl/stl/string.h"
#include "acl/IniConfig.h"
#include "acl/GetOpt.h"
#include "acl/SString.h"
#include "anf/IoSockAcceptor.h"
#include "anf/IoHandler.h"
#include "anf/filter/BencodingMessageFilter.h"
#include "gfq2/GFQueueClient.h"
#include "./atf/TestCase.h"
#include "./atf/TestAction.h"
#include "./atf/TestParallel.h"
#include "./atf/TestControl.h"
#include "./atf/TestSnapshot.h"

bool             gbIsTerminate = false;
apl_int_t        giPort = 0;
apl_int_t        giTimedout = 0;
apl_int_t        giConnNum = 1;
apl_int_t        giDelNum = 1;
class CKey*      gpoKey = NULL;
std::string      goIpAddress("127.0.0.1");
std::string      goQueueName("test");
std::string      goNodeGroup("default");
gfq2::CGFQueueClient goGFQueueClient;

// Auto gen data key
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

    virtual void MakeKey( char* apcKeyBuff, apl_size_t auLen )
    {
        acl::CDateTime loDate;
        apl_int_t liCount = 0;
        
        loDate.Update();
        
        {
            acl::TSmartLock<acl::CLock> loGuard(this->moLock);
            liCount = this->miCount;
            this->miCount += this->miStep;
        }

        apl_int_t liLen = apl_snprintf( apcKeyBuff, auLen, "%s%010d", loDate.Format("%m%d%H%M%S"), liCount );
        apl_memset(apcKeyBuff + liLen, 'A', auLen - liLen);
    }
    
protected:
    apl_int_t miCount;
    apl_int_t miStep;
    acl::CLock moLock;
};

//All test case
class AIInsertTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        char lacValue[1024];
        
        gpoKey->MakeKey(lacValue, sizeof(lacValue) );
        
        apl_int_t liRet = goGFQueueClient.Put(goQueueName.c_str(), lacValue, sizeof(lacValue) );
        if ( liRet != 0 )
        {
            if (gbIsTerminate )
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
        std::string loValue;
        
        apl_int_t liRet = goGFQueueClient.Get(goQueueName.c_str(), &loValue);
        if ( liRet != 0 )
        {
            if (gbIsTerminate)
            {
                printf( "Delete data fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
            }

            return -1;
        }
        else
        {
            if (gbIsTerminate)
            {
                printf( "Delete data success, [value=%s]\n", loValue.c_str() );
            }
        }
        
        return liRet;
    }
};

class AIMDelTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        std::vector<std::string> loValues;
        
        apl_int_t liRet = goGFQueueClient.Get(goQueueName.c_str(), giDelNum, &loValues);
        if ( liRet < 0 )
        {
            if (gbIsTerminate)
            {
                printf( "Delete data fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
            }

            return -1;
        }
        else
        {
            if (gbIsTerminate)
            {
                for (std::vector<std::string>::iterator iter = loValues.begin(); iter != loValues.end(); ++iter)
                {
                    printf( "Delete data success, [value=%s]\n", iter->c_str() );
                }
            }
        
            return 0;
        }
    }
};

class AIPollDelTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        std::string loValue;

        //Queue poll resultset
        std::vector<std::string> loResult;

        //Queue poll
        gfq2::CQueuePoll loQueuePoll(goGFQueueClient);

        //Interested queue
        loQueuePoll.Add(goQueueName.c_str() );

        if (loQueuePoll.Poll(100, &loResult) > 0)
        {
            printf("Result set (size=%d):\n", loResult.size() );

            for(std::vector<std::string>::iterator loIter = loResult.begin();
                loIter != loResult.end();
                ++loIter)
            {
                printf("\t%s\n", loIter->c_str() );

                apl_int_t liRet = goGFQueueClient.Get(loIter->c_str(), &loValue);
                if ( liRet != 0 )
                {
                    if (gbIsTerminate )
                    {
                        printf( "Delete data fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
                    }
                    return -1;
                }
                else
                {
                    if (gbIsTerminate )
                    {
                        printf( "Delete data success, [value=%s]\n", loValue.c_str() );
                    }
                }
            }
            
            return 0;
        }
        else
        {
            printf("Poll timeout\n");
            
            return -1;
        }
    }
};

class AIDelTimeoutTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        acl::CMemoryBlock loValue;
        
        apl_int_t liRet = goGFQueueClient.GetTimedout(goQueueName.c_str(), giTimedout, &loValue );
        if ( liRet != 0 )
        {
            if (gbIsTerminate)
            {
                printf( "Delete data fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
            }
            return -1;
        }
        else
        {
            if (gbIsTerminate)
            {
                printf( "Delete data success, [value=%s]\n", loValue.GetReadPtr() );
            }
        }
        
        return liRet;
    }
};

class AIQueueListTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        std::vector<gfq2::CQueueInfo> loResult;
        
        apl_int_t liRet = goGFQueueClient.GetAllQueueInfo(loResult);
        if ( liRet != 0 )
        {
            printf( "Get queue list fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
            return -1;
        }

        printf("----------------------------------------------------------------------\n" );
        printf("|%-20s|%-15s|%-15s|%-15s|\n", "queue name", "capacity", "size", "segment count" );
        printf("----------------------------------------------------------------------\n" );
        for (std::vector<gfq2::CQueueInfo>::iterator loIter = loResult.begin();
             loIter != loResult.end(); ++loIter)
        {
            printf("|%-20s|%-15"APL_PRIuINT"|%-15"APL_PRIuINT"|%-15"APL_PRIuINT"|\n", 
                loIter->GetQueueName(), 
                loIter->GetCapacity(), 
                loIter->GetSize(), 
                loIter->GetSegmentCount() );
        }
        printf("----------------------------------------------------------------------\n" );
        
        return liRet;
    }
};

class AINodeListTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        std::vector<gfq2::CServerNodeInfo> loResult;
        
        apl_int_t liRet = goGFQueueClient.GetAllServerNodeInfo(loResult);
        if ( liRet != 0 )
        {
            printf( "Get node list fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
            return -1;
        }

        printf("--------------------------------------------------------------------------------------------------\n" );
        printf("|%-20s|%-15s|%-15s|%-10s|%-10s|%-10s|%-10s|\n",
            "node name", "server ip", "server port", "free", "used", "conns", "tps" );
        printf("--------------------------------------------------------------------------------------------------\n" );
        for (std::vector<gfq2::CServerNodeInfo>::iterator loIter = loResult.begin();
             loIter != loResult.end(); ++loIter)
        {
            printf("|%-20s|%-15s|%-15"APL_PRIuINT"|%-10"APL_PRIuINT"|%-10"APL_PRIuINT"|%-10"APL_PRIdINT"|%-10"APL_PRIuINT"|\n", 
                loIter->GetServerNodeName(), 
                loIter->GetServerIp(), 
                (apl_size_t)loIter->GetServerPort(), 
                loIter->GetFreeSegmentCount(),
                loIter->GetUsedSegmentCount(),
                (apl_int_t)loIter->GetConnectionCount(),
                (apl_size_t)(loIter->GetReadTimes() + loIter->GetWriteTimes() ) );
        }
        printf("---------------------------------------------------------------------------------------------------\n" );

        return 0;
    }
};

class AIBindDefaultAllocateNodeTestCase : public atf::CTestCase
{
public:
    virtual apl_int_t DoIt( void* apParam )
    {
        apl_int_t liRet = goGFQueueClient.BindNodeGroup(goQueueName.c_str(), goNodeGroup.c_str());
        if ( liRet != 0 )
        {
            if (gbIsTerminate)
            {
                printf( "Bind default allocate node fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
            }
            return -1;
        }
        else
        {
            printf( "Bind default allocate node success.\n");
        }
        
        return 0;
    }
};

void PrintUsage(void)
{
    printf("Usage:\n");
    printf("    -i --ip=<ip address>: master server ip address\n");
    printf("    -p --port=<port> : master server port\n");
    printf("    -c --connection=<num> : connection count\n");
    printf("    -e --exec=<get|put|get-timedout|poll-get|queue-list|node-list|bind-node|service> : execute operator\n");
    printf("    -l --loop=<num> : repeat times\n");
    printf("    -s --stat=<second> : statistical snapshot\n");
    printf("    -t --thread=<num> : parallel thread count\n");
    printf("    -f --fps=<num> : flows per second\n");
    printf("    -q --queue=<queue name> : queue name\n");
    printf("    -b --bind=<node name> : default server node name\n");
    printf("    -k --key=<start:step> : autogen key start point and step separating by ':'\n");
    printf("    -v --view : view running info\n");
    printf("--exec=get-timeout option\n");
    printf("    -o --timedout=<timevalue> : timedout\n");
    printf("--exec=get option\n");
    printf("    -n --num=<num> : get max record count per times\n");
    printf("--exec=service option\n");
    printf("    -m --service=<ip:port> : put operator proxy service, will put the received data into GFQ\n");
}

class CServiceHandler : public anf::IoHandler
{
public:
    virtual void Exception( anf::CThrowable const& aoCause )
    {
        apl_errprintf("Server exception - %s\n", aoCause.GetMessage() );
    }
    
    virtual void SessionException( anf::SessionPtrType& aoSession, anf::CThrowable const& aoCause )
    {
        apl_errprintf("Session exception - %s\n", aoCause.GetMessage() );
    }

    virtual void SessionOpened( anf::SessionPtrType& aoSession )
    {
        apl_errprintf(
            "Create a new session [%s:%"APL_PRIdINT"]\n",
            aoSession->GetRemoteAddress().GetIpAddr(),
            (apl_int_t)aoSession->GetRemoteAddress().GetPort() );
    }

    virtual void SessionClosed( anf::SessionPtrType& aoSession )
    {
        apl_errprintf(
            "Close session [%s:%"APL_PRIdINT"]\n",
            aoSession->GetRemoteAddress().GetIpAddr(),
            (apl_int_t)aoSession->GetRemoteAddress().GetPort() );
    }

    virtual void SessionIdle( anf::SessionPtrType& aoSession, apl_int_t aiStatus )
    {
        switch(aiStatus)
        {
            case anf::SESSION_READ_IDLE:
                apl_errprintf("Session read idle\n");
                break;
            case anf::SESSION_WRITE_IDLE:
                apl_errprintf("Session write idle\n");
                break;
        };
    }

    virtual void MessageReceived( anf::SessionPtrType& aoSession, acl::CAny const& aoMessage )
    {
        acl::CIndexDict* lpoRecved = NULL;

        ACL_ASSERT(aoMessage.CastTo<acl::CIndexDict*>(lpoRecved) );

        char const* lpcQueueName = lpoRecved->Get(0,"");
        acl::CIndexDict::ValueType loValue = lpoRecved->GetStr(1,"");
        
        apl_int_t liRet = goGFQueueClient.Put(lpcQueueName, loValue.GetStr(), loValue.GetLength() );
        if ( liRet != 0 )
        {
            if (gbIsTerminate )
            {
                printf( "Insert data fail, MSGCODE:%"APL_PRIdINT"\n", liRet);
            }
        }
    }

    virtual void MessageFinished( anf::SessionPtrType& aoSession, anf::CWriteFuture::PointerType& aoWriteFuture )
    {
    };
};

void StartService( std::string const& aoService )
{
    acl::CSpliter loSpliter(":");
    anf::CIoSockAcceptor loAcceptor;

    loSpliter.Parse(aoService);

    if (loSpliter.GetSize() != 2)
    {
        PrintUsage();
        apl_exit(0);
    }

    loAcceptor.SetHandler(new CServiceHandler);
    loAcceptor.GetFilterChain()->AddFirst("bencoding-message", new anf::CBencodingMessageFilter);

    loAcceptor.Bind( acl::CSockAddr(apl_strtoi32(loSpliter.GetField(1), APL_NULL, 10), loSpliter.GetField(0) ) );

    printf("Service startup\n");

    while(true)
    {
        apl_sleep(APL_TIME_SEC);
    }
}

int main(int argc, char* argv[] )
{
    acl::CGetOpt loOpt(argc, argv, "i:p:c:e:o:vl:s:t:f:q:b:k:m:n:");
    loOpt.LongOption("ip", 'i', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("port", 'p', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("connection", 'c', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("exec", 'e', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("timedout", 'o', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("loop", 'l', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("stat", 's', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("thread", 't', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("fps", 'f', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("queue", 'q', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("bind", 'b', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("key", 'k', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("service", 'm', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("num", 'n', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("view", 'v', acl::CGetOpt::NO_ARG);

    apl_int_t liStatisticalTimeInterval = 0;
    apl_int_t liMaxUnitFlow = 0;
    apl_int_t liThreadCnt = 1;
    apl_int_t liCount = 0;
    apl_int_t liStartKey = 0;
    apl_int_t liStep = 1;
    std::string loService;
    std::string loExec = "put";
    apl_int_t liOptChar;
    
    while( (liOptChar = loOpt()) != -1)
	{
		switch (liOptChar)
		{
            case 'i':
                goIpAddress = loOpt.OptArg();
                break;
            case 'p':
                giPort = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
                break;
            case 'c':
                giConnNum = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
                break;
			case 'e':
			    loExec = loOpt.OptArg();
			    break;
			case 'o':
                giTimedout = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
				break;
			case 'v':
			    gbIsTerminate = true;
				break;
			case 's':
				liStatisticalTimeInterval = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
				break;
			case 'n':
				giDelNum = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
				break;
			case 't':
			    liThreadCnt = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
			    break;
			case 'l':
			    liCount = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
			    break;
			case 'f':
			    liMaxUnitFlow = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
			    break;
			case 'k':
            {
                acl::CSpliter loSpliter(":");

                loSpliter.Parse(loOpt.OptArg() );

			    liStartKey = apl_strtoi32(loSpliter.GetField(0), APL_NULL, 10);
			    liStep = apl_strtoi32(loSpliter.GetField(1), APL_NULL, 10);

			    break;
            }
			case 'm':
                loService = loOpt.OptArg();
			    break;
            case 'q':
                goQueueName = loOpt.OptArg();
                break;
            case 'b':
                goNodeGroup = loOpt.OptArg();
                break;
			default:
                PrintUsage();
			    return 0;
		}
	}

    if (giPort == 0 || goIpAddress.length() == 0)
    {
        PrintUsage();
        return 0;
    }
    
	gpoKey = new CKey( liStartKey, liStep );

    acl::CSockAddr loRemote(giPort, goIpAddress.c_str() );
    if (goGFQueueClient.Initialize("test", loRemote, giConnNum, 10, false) != 0)
	{
	    printf( "Client initialize fail, may be connect rejected - %s\n", strerror(errno) );
	    return -1;
	}

    goGFQueueClient.SetReadBufferSize(giDelNum * 1024 + 100);
	
    atf::CTestCase* lpCase = NULL;
    if (loExec == "put")
    {
	    lpCase = new AIInsertTestCase;
    }
    else if (loExec == "get")
    {
        if (giDelNum > 1)
        {
	        lpCase = new AIMDelTestCase;
        }
        else
        {
	        lpCase = new AIDelTestCase;
        }
    }
    else if (loExec == "get-timedout")
    {
	    lpCase = new AIDelTimeoutTestCase;
    }
    else if (loExec == "poll-get")
    {
	    lpCase = new AIPollDelTestCase;
    }
    else if (loExec == "queue-list")
    {
	    lpCase = new AIQueueListTestCase;
    }
    else if (loExec == "node-list")
    {
	    lpCase = new AINodeListTestCase;
    }
    else if (loExec == "bind-node")
    {
	    lpCase = new AIBindDefaultAllocateNodeTestCase;
    }
    else if (loExec == "service")
    {
        StartService(loService);
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
	
    goGFQueueClient.Close();

    return 0;
}

