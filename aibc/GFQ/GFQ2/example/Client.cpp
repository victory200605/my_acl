#include "gfq2/GFQueueClient.h"

int main( int argc, char* argv[] )
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

    //Save data to 'queue1' GFQ queue 
    std::string data("test data");
    if ( (liRetCode = loClient.Put("queue1", data) ) != 0)
    {
        apl_errprintf(
            "GFQ Client put data to remote server fail,%s\n",
            gfq2::StrError(liRetCode) );

        loClient.Close();//Fail and close

        return -1;
    }

    //Get data from 'queue1' GFQ queue
    std::string data1;
    if ( (liRetCode = loClient.Get("queue1", &data1) ) != 0)
    {
        apl_errprintf("GFQ Client get data from remote server fail,%s\n", gfq2::StrError(liRetCode) );

        loClient.Close();//Fail and close

        return -1;
    }
    else
    {
        apl_errprintf("GFQ Client get data(%s) from remote server\n", data1.c_str() );
    }

    loClient.Close();//Close

    return 0;
}

