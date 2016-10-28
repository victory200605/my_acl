
#include "acl/GetOpt.h"
#include "acl/SString.h"
#include "gfq2/GFQueueClient.h"

void PrintUsage(void)
{
    printf("Usage:\n");
    printf("    -i --ip=<ip address>: master server ip address\n");
    printf("    -p --port=<port> : master server port\n");
    printf("    -q --queue=<queue> : queue name\n");
    printf("    -g --group=<nodegroup> : server node group name\n");
    printf("    -h --help\n");
}

apl_int_t    giPort = 0;
std::string  goIpAddress("127.0.0.1");

int main( int argc, char* argv[] )
{
    std::vector<std::string> loQueueList;
    acl::CGetOpt loOpt(argc, argv, "i:p:q:n:c:h");
    loOpt.LongOption("ip", 'i', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("port", 'p', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("queue", 'q', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("group", 'g', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("help", 'h', acl::CGetOpt::NO_ARG);
    
    apl_int_t liOptChar = 0;
    apl_int_t liRetCode = 0;
    std::string loQueueName;
    std::string loGroupName;

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
                loQueueName = loOpt.OptArg();
                break;
            case 'g':
                loGroupName = loOpt.OptArg();
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
    
    if ( (liRetCode = loClient.BindNodeGroup(loQueueName.c_str(), loGroupName.c_str() ) ) != 0)
    {
        printf("Bind node group fail, (QueueName=%s,GroupName=%s,RetCode=%"APL_PRIdINT")\n",
            loQueueName.c_str(),
            loGroupName.c_str(),
            liRetCode );

        return -1;
    }
    else
    {
        printf("Bind node group success\n");

        return 0;
    }
}
