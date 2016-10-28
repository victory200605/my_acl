
#include "acl/GetOpt.h"
#include "acl/SString.h"
#include "gfq2/GFQueueClient.h"

void PrintUsage(void)
{
    printf("Usage:\n");
    printf("    -i --ip=<ip address>: master server ip address\n");
    printf("    -p --port=<port> : master server port\n");
    printf("    -q --queue=<queue [|queue ...]> : queue name\n");
    printf("    -n --interval=<second> : display interval\n");
    printf("    -c --count=<num> : display count\n");
    printf("    -h --help\n");
}

apl_int_t    giPort = 0;
apl_int_t    giInterval = 0;
apl_int_t    giCount = 1;
std::string  goIpAddress("127.0.0.1");

int main( int argc, char* argv[] )
{
    std::vector<std::string> loQueueList;
    acl::CGetOpt loOpt(argc, argv, "i:p:q:n:c:h");
    loOpt.LongOption("ip", 'i', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("port", 'p', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("queue", 'q', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("interval", 'n', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("count", 'c', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("help", 'h', acl::CGetOpt::NO_ARG);
    
    apl_int_t liOptChar = 0;
    apl_int_t liRetCode = 0;

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
            case 'q':
            {
                acl::CSpliter loSpliter("|");

                loSpliter.Parse(loOpt.OptArg() );

                for (apl_size_t luN = 0; luN < loSpliter.GetSize(); luN++)
                {
                    loQueueList.push_back(loSpliter.GetField(luN) );
                }

                break;
            }
            case 'n':
            {
                giInterval = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
                
                if (giCount == 0)
                {
                    giCount = APL_INT_MAX;
                }

                break;
            }
            case 'c':
                giCount = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
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
    gfq2::CGFQueueClient loClient;
    if (loClient.Initialize("main", loRemote, 1, 10) != 0)
	{
	    printf( "Client initialize fail, may be connect rejected - %s\n", apl_strerror(apl_get_errno() ) );
	    return -1;
	}
    
    for (apl_int_t i = 0; i < giCount; i++)
    {
        std::vector<gfq2::CQueueInfo> loResult;
        
        if (loQueueList.size() > 0)
        {
            gfq2::CQueueInfo loQueueInfo;

            for (std::vector<std::string>::iterator loIter = loQueueList.begin();
                 loIter != loQueueList.end(); ++loIter)
            {
                if ( (liRetCode = loClient.GetQueueInfo(loIter->c_str(), loQueueInfo) ) != 0)
                {
                    printf( "Get %s queue info fail, MSGCODE:%"APL_PRIdINT"\n", loIter->c_str(), liRetCode);
                    return -1;
                }

                loResult.push_back(loQueueInfo);
            }
        }
        else
        {
            if ( (liRetCode = loClient.GetAllQueueInfo(loResult) ) != 0)
            {
                printf( "Get all queue info fail, MSGCODE:%"APL_PRIdINT"\n", liRetCode);
                return -1;
            }
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

        if (giInterval > 0 && i != giCount - 1)
        {
            apl_sleep(giInterval * APL_TIME_SEC);
        }
    }

    return 0;
}
