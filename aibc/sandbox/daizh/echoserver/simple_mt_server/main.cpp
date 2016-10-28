
#include "acl/ThreadManager.h"
#include "acl/SockAcceptor.h"
#include "acl/SockStream.h"
#include "acl/SockAddr.h"
#include "acl/GetOpt.h"

void* worker(void* apParam)
{
    acl::CSockStream loPeer;
    loPeer.SetHandle((apl_handle_t)(apParam));
    char lacBuffer[2048];
    apl_ssize_t liResult = 0;

	ACL_ASSERT(loPeer.SetOption(acl::CSockHandle::OPT_TCPNODELAY, 1) == 0);
    
    while(true)
    {
        if ( (liResult = loPeer.Recv(lacBuffer, sizeof(lacBuffer) ) ) < 0 )
        {
            break;
        }
        
        if ( loPeer.Send(lacBuffer, liResult) != liResult)
        {
            break;
        }
    }
    
    printf("worker %d exited\n", (int)pthread_self());
    
    return NULL;
}

int main( int argc, char* argv[] )
{
    apl_int_t liOptChar;
    apl_int16_t li16Port = 5555;
    char lacIpAddress[12] = "127.0.0.1";
    acl::CGetOpt loOpt(argc, argv, "i:p:");
    while( (liOptChar = loOpt() ) != EOF )
	{
		switch (liOptChar)
		{
		    case 'i':
			    apl_strncpy(lacIpAddress, loOpt.OptArg(), sizeof(lacIpAddress) );
			    break;
			case 'p':
			    li16Port = apl_strtoi32(loOpt.OptArg(), NULL, 10);
				break;
        }
    }
    
    acl::CThreadManager loThreadManager;
    acl::CSockAcceptor loAcceptor;
    acl::CSockAddr loAddr(li16Port, lacIpAddress);
    
    if (loAcceptor.Open(loAddr) != 0 )
    {
        printf("startup fail\n");
        return -1;
    }
    
    while(true)
    {
        acl::CSockStream loPeer;
        
        if (loAcceptor.Accept(loPeer) != 0)
        {
            printf("accept fail\n");
            return -1;
        }
        
        if( loThreadManager.Spawn(worker, (void*)loPeer.GetHandle() ) != 0)
        {
            printf("spawn worker fail\n");
            return -1;
        }
    }
    
    return 0;
}
