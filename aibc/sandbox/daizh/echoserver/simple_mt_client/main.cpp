
#include "anf/protocol/SimpleAsyncMessage.h"
#include "acl/SockConnector.h"
#include "acl/SockAddr.h"
#include "acl/SockStream.h"
#include "acl/ThreadManager.h"
#include "acl/Number.h"
#include "acl/GetOpt.h"

acl::TNumber<apl_int64_t, acl::CLock> goTraffic(0);
acl::TNumber<apl_int64_t, acl::CLock> goCount(0);
apl_int16_t gi16Port = 5555;

void* Run(void* apcIp)
{
    acl::CSockAddr loAddr(gi16Port, (char*)apcIp);
    acl::CSockConnector loConnector;
    acl::CSockStream loPeer;
    acl::CTimeValue  loTimeout(10);
    anf::CSimpleAsyncMessage  loRequest;
    anf::CSimpleAsyncMessage  loResponse;
    acl::CMemoryBlock loBlock(2048);
    char lacBuffer[1024] = {'A'};
    apl_ssize_t liResult = 0;
    
    loRequest.GetBody().Resize(1024);
    loRequest.GetBody().Write(lacBuffer, sizeof(lacBuffer) );
    
    if (loConnector.Connect(loPeer, loAddr, loTimeout) != 0)
    {
        printf("Connect fail %s\n", strerror(errno) );
        return NULL;
    }

	loPeer.SetOption(acl::CSockHandle::OPT_TCPNODELAY, 1);
    
    while(true)
    {
        loBlock.Reset();
        
        loRequest.Encode(loBlock);
        
        if ( (liResult = loPeer.Send(loBlock.GetReadPtr(), loBlock.GetLength() ) ) != (apl_ssize_t)loBlock.GetLength() )
        {
            printf("Send fail %s\n", strerror(errno) );
            return NULL;
        }
        
        goTraffic += liResult;

        if ( (liResult = loPeer.Recv(loBlock.GetReadPtr(), loBlock.GetLength() ) ) != (apl_ssize_t)loBlock.GetLength() )
        {
            printf("Recv fail %s\n", strerror(errno) );
            return NULL;
        }
        
        loResponse.Decode(loBlock);
        
        //goTraffic += liResult;

        ++goCount;
    }
    
    return NULL;
}

void* Stat(void*)
{
    apl_int64_t li64CurrTraffic = 0;
    apl_int64_t li64PrevTraffic = 0;
    apl_int64_t li64CurrCount = 0;
    apl_int64_t li64PrevCount = 0;
    
    while(true)
    {
        apl_sleep(APL_TIME_SEC);
        
        li64CurrTraffic = goTraffic;
        li64CurrCount = goCount;
         
        printf(
            "Curr traffic = %lfMB, %lfper\n",
            float(li64CurrTraffic - li64PrevTraffic) / (1024 * 1024),
            float(li64CurrCount - li64PrevCount) );
        
        li64PrevCount = li64CurrCount;
        li64PrevTraffic = li64CurrTraffic;
    }
    
    return NULL;
}

int main(int argc, char* argv[])
{
    apl_int_t liOptChar;
    char lacIpAddress[12] = "127.0.0.1";
    apl_int_t liThreadCount = 1;
    acl::CGetOpt loOpt(argc, argv, "i:p:t:");
    while( (liOptChar = loOpt() ) != EOF )
	{
		switch (liOptChar)
		{
		    case 'i':
			    apl_strncpy(lacIpAddress, loOpt.OptArg(), sizeof(lacIpAddress) );
			    break;
			case 'p':
			    gi16Port = apl_strtoi32(loOpt.OptArg(), NULL, 10);
				break;
		    case 't':
		        liThreadCount = apl_strtoi32(loOpt.OptArg(), NULL, 10);
		        break;
        }
    }

    acl::CThreadManager loThreadManager;
    
    loThreadManager.Spawn(Stat, NULL);
    
    for (apl_int_t liN = 0; liN < liThreadCount; liN++)
    {
        loThreadManager.Spawn(Run, lacIpAddress);
    }
    
    loThreadManager.WaitAll();
    
    return 0;
}
