#include "gfq2/GFQueueClient.h"
#include "acl/ThreadManager.h"

void* Worker(void* apvParam)
{
    gfq2::CQueuePoll* lpoQueuePoll = 
        static_cast<gfq2::CQueuePoll*>(apvParam);

    //Create poll result set
    std::vector<std::string> loResult;

    while(true)
    {
        //Send poll request to server
        apl_ssize_t liN = lpoQueuePoll->Poll(10/*timeout*/, &loResult);
        if (liN < 0)
        {
            //TODO ERROR
            apl_errprintf("QueuePoll fail,%s\n", gfq2::StrError(liN) );
        }
        else
        {
            //Handle queue readable event
            for (std::vector<std::string>::iterator loIter = loResult.begin();
                loIter != loResult.end(); ++loIter)
            {
                std::string loValue;
    
                //Read all in once
                while(true)
                {
                    apl_int_t liRetCode = lpoQueuePoll->GetClient()->Get(loIter->c_str(), &loValue);
                    if (liRetCode != 0)
                    {
                        apl_errprintf("GFQ client get data fail,%s\n", gfq2::StrError(liN) );

                        break;
                    }
                    else
                    {
                        apl_errprintf("GFQ client get data (%s)\n", loValue.c_str() );
                    }
                }
            }
        }
    }

    return APL_NULL;
}

int main( int argc, char* arpv[] )
{
    apl_int_t liRetCode = 0;

    //Create GFQ client
    gfq2::CGFQueueClient loClient;

    //Set client env
    loClient.SetReadBufferSize(2048*10);

    //Initialize client
    if ( (liRetCode = loClient.Initialize(
        "test", //Client identify name
        acl::CSockAddr(8012, "127.0.0.1"), //remote server address
        10, //parallel connection count
        10 //timedout
        ) ) != 0)
    {
        apl_errprintf(
            "GFQ Client connect to remote server fail,%s\n",
            gfq2::StrError(liRetCode) );

        return -1;
    }

    gfq2::CQueuePoll loQueuePoll(loClient);

    //Add interest queue
    loQueuePoll.Add("queue1");
    loQueuePoll.Add("queue2");
    loQueuePoll.Add("queue3");

    acl::CThreadManager loThreadManager;

    loThreadManager.SpawnN(10, Worker, &loQueuePoll);

    loThreadManager.WaitAll();
    
    return 0;
}

