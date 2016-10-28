#include "acl/Timestamp.h"
#include "acl/Synch.h"
#include "acl/stl/string.h"
#include "acl/IniConfig.h"
#include "acl/GetOpt.h"
#include "acl/SString.h"
#include "anf/IoSockAcceptor.h"
#include "anf/IoHandler.h"
#include "anf/filter/BencodingMessageFilter.h"
#include "gfq2/GFQueueClient.h"

//api env
gfq2::CGFQueueClient goGFQueueClient;
gfq2::CQueuePoll goQueuePoll(goGFQueueClient);
std::string      goIpAddress("127.0.0.1");
apl_int_t        giPort = 0;
apl_int_t        giTimeOut = 100;

//source queue and dest queue
std::string goInQueueName;
std::string goOutQueueName;

//stat. info
acl::TNumber<apl_int_t, acl::CLock> giSuccess;
acl::TNumber<apl_int_t, acl::CLock> giReject;
apl_int_t giLastSuccess = 0;
apl_int_t giLastReject = 0;
apl_int_t giTimeInterval = 1;

void* Worker( void* apvParam)
{
    std::string loValue;
    std::vector<std::string> loResult;
    
    while(true) 
    {
        if (goQueuePoll.Poll(giTimeOut, &loResult) > 0)
        {
            while(true)
            {
                apl_int_t liRetCode = goGFQueueClient.Get(goOutQueueName.c_str(), &loValue);
                if (liRetCode == 0)
                {
                    liRetCode = goGFQueueClient.Put(goInQueueName.c_str(), loValue);
                    if ( liRetCode != 0 )
                    {
                        giReject++;

                        apl_errprintf("Worker put data into dest queue fail, %s (RetCode=%"APL_PRIdINT")\n", 
                            gfq2::StrError(liRetCode), 
                            liRetCode );

                        break;
                    }
                    else
                    {
                        giSuccess++;
                    }

                    continue;
                }
                else if (liRetCode != gfq2::ERROR_END_OF_QUEUE)
                {
                    apl_errprintf("Worker get data from source queue fail, %s (RetCode=%"APL_PRIdINT")\n", 
                        gfq2::StrError(liRetCode),
                        liRetCode );
                }

                break;
            }
        }
    }

    return NULL;
}

void* StatInfoSrv( void* apvParam)
{
    while(true)
    {
        apl_sleep( APL_TIME_SEC * giTimeInterval);

        apl_int_t liCurrSuccess = giSuccess;
        apl_int_t liCurrReject = giReject;

        apl_errprintf(" %"APL_PRIdINT"sec process  %"APL_PRIdINT"  %"APL_PRIdINT" fail\n",
                giTimeInterval, 
                liCurrSuccess - giLastSuccess, 
                liCurrReject - giLastReject );

        giLastSuccess = liCurrSuccess;
        giLastReject = liCurrReject;
    }

    return NULL;
}

void PrintUsage(void)
{
    printf("Usage:\n");
    printf("    -i --ip=<ip address>: master server ip address\n");
    printf("    -p --port=<port> : master server port\n");
    printf("    -c --connection=<num> : parallel connection\n");
    printf("    -t --thread=<num> : parallel thread count\n");
    printf("    -d --dest=<name> : the queuname send to \n");
    printf("    -s --source=<name> : source queue to get from\n");
    printf("    -o --otime=<timevalue> : time to wait every poll(default 10sec)\n");
    printf("    -v --interval=<timevalue> : speed print time interval(default 1sec)\n");
}

int main(int argc, char* argv[] )
{
    acl::CGetOpt loOpt(argc, argv, "i:p:c:t:s:d:o::v::");
    loOpt.LongOption("ip", 'i', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("port", 'p', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("conn", 'c', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("thread", 't', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("source", 's', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("dest", 'd', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("otime", 'o', acl::CGetOpt::ARG_OPTIONAL);
    loOpt.LongOption("interval", 'v', acl::CGetOpt::ARG_OPTIONAL);
    
    apl_int_t liThreadCnt = 1;
    apl_int_t liConnCnt = 1;
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
                liConnCnt = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
                break;
            case 't':
                liThreadCnt = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
                break;
            case 'd':
                goInQueueName = loOpt.OptArg();
                break;
            case 's':
                goOutQueueName = loOpt.OptArg();
                break;
            case 'o':
                giTimeOut = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
                break;
            case 'v':
                giTimeInterval = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
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

    acl::CSockAddr loRemote(giPort, goIpAddress.c_str() );
    if (goGFQueueClient.Initialize("test", loRemote, liConnCnt, 10, false) != 0)
    {
        apl_errprintf( "Client initialize fail, may be connect rejected - %s\n", strerror(errno) );
        return -1;
    }

    goGFQueueClient.SetReadBufferSize(102400);
    
    //Interested queue
    goQueuePoll.Add(goOutQueueName.c_str() );

    acl::CThreadManager loThreadManager;

    if (loThreadManager.Spawn(StatInfoSrv, NULL) != 0 )
    { 
        apl_errprintf( "Can't startup the info thread!\n");
    }
    
    if (loThreadManager.SpawnN(liThreadCnt, Worker, NULL) != 0 )
    {
        apl_errprintf( "Can't startup %"APL_PRIdINT" threads.\n",liThreadCnt);
        return -1;
    }

    loThreadManager.WaitAll();

    return 0; 
}
