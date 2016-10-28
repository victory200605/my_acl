
#include "anf/util/AsyncClient.h"
#include "anf/protocol/SimpleAsyncMessage.h"
#include "anf/filter/SimpleAsyncMessageFilter.h"
#include "acl/Number.h"
#include "acl/GetOpt.h"

//global variable define
typedef anf::TAsyncClient<anf::CSimpleAsyncMessage> SyncClientType;
acl::TNumber<apl_int64_t, acl::CLock> goTraffic(0);
acl::TNumber<apl_int64_t, acl::CLock> goCount(0);
apl_int16_t gi16Port = 5555;
std::string goIpAddress = "127.0.0.1";
apl_size_t  giWriteBufferSize = 1024;

//worker thread
void* Worker(void* apoClient)
{
    SyncClientType* lpoClient = static_cast<SyncClientType*>(apoClient);
    anf::CSimpleAsyncMessage  loRequest;
    anf::CSimpleAsyncMessage* lpoResponse = NULL;
    acl::CTimeValue loTimeout(10);
    apl_int_t liRetCode = 0;
    
    loRequest.GetBody().Resize(giWriteBufferSize - 12);
    loRequest.GetBody().SetWritePtr(giWriteBufferSize - 12);

    while(true)
    {
        if ( (liRetCode = lpoClient->Request(loRequest, &lpoResponse, loTimeout) ) < 0)
        {
            printf("Request fail %d\n", liRetCode );
            return NULL;
        }
        
        ACL_DELETE(lpoResponse);
        
        goTraffic += giWriteBufferSize;
        ++goCount;
    }
    
    return NULL;
}

//stat thread
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
    apl_int_t liThreadCount = 1;
    apl_int_t liConnCount = 1;
    acl::CGetOpt loOpt(argc, argv, "i:p:c:t:w:");
    while( (liOptChar = loOpt() ) != EOF )
	{
		switch (liOptChar)
		{
		    case 'i':
			    goIpAddress = loOpt.OptArg();
			    break;
			case 'p':
			    gi16Port = apl_strtoi32(loOpt.OptArg(), NULL, 10);
				break;
            case 'c':
			    liConnCount = apl_strtoi32(loOpt.OptArg(), NULL, 10);
                break;
		    case 't':
		        liThreadCount = apl_strtoi32(loOpt.OptArg(), NULL, 10);
		        break;
            case 'w':
                giWriteBufferSize = apl_strtoi32(loOpt.OptArg(), NULL, 10);
        }
    }
    
    //set remote address
    acl::CSockAddr loRemoteAddress(gi16Port, goIpAddress.c_str() );
    SyncClientType loClient;
    
    //config sync client
    loClient.GetFilterChain()->AddFirst("simple-async-message", new anf::CSimpleAsyncMessageFilter);
    loClient.GetConfiguration()->SetProcessorPoolSize(liConnCount);
    loClient.GetConfiguration()->SetWriteBufferSize(giWriteBufferSize);
    loClient.GetConfiguration()->SetReadBufferSize( 10 * giWriteBufferSize);
    
    //initialize client
    if (loClient.Initialize(1000, 10) != 0)
    {
        printf("Client initialize fail\n");
        return -1;
    }
    if (loClient.Connect("test", loRemoteAddress, liConnCount) <= 0)
    {
        printf("Client connect fail\n");
        return -1;
    }

    acl::CThreadManager loThreadManager;
    
    //start stat. thread
    loThreadManager.Spawn(Stat, NULL);
    
    //start work thread
    for (apl_int_t liN = 0; liN < liThreadCount; liN++)
    {
        loThreadManager.Spawn(Worker, &loClient);
    }
    
    loThreadManager.WaitAll();
    
    return 0;
}
