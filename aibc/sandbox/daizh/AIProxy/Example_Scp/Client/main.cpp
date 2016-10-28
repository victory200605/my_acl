

#include "../../Api/AIProxyApi.h"
#include "../Proxy/ScpProtocol.h"
#include "./AITest/TestCase.h"
#include "./AITest/TestAction.h"
#include "./AITest/TestParallel.h"
#include "./AITest/TestControl.h"
#include "./AITest/TestSnapshot.h"

using namespace AIPROXY;

class MyProxyApi : public AIProxyApi
{
public:
    MyProxyApi( AIProtocol* apoProtocol ) : AIProxyApi(apoProtocol) {};
    
    virtual int InputRequest( const char* apcData, size_t aiSize )
    {
        printf( "<<Recv request success [MsgID=%s]/[Size=%d]\n", apcData, aiSize );
        printf( "<<Send response now\n" );
        return this->Response( apcData, aiSize );
    }
};

struct stParam
{
    stParam(): coProxyApi( new clsScpInnerProtocol ), coBuffer(0),coResponse(0) {}
    
    MyProxyApi  coProxyApi;
    AIChunkEx   coBuffer;
    AIChunkEx   coResponse;
};

class MyTestCase : public AITest::AITestCase
{
public:
    virtual int DoIt( void* apParam )
    {
        stParam* lpoParam = (stParam*)apParam;
        if ( lpoParam->coProxyApi.Request( lpoParam->coBuffer.BasePtr(), lpoParam->coBuffer.GetSize(), lpoParam->coResponse, 10 ) != 0 )
        {
            printf( "Request fail,[MsgID=%s]\n", "xxx" );
        }
        else
        {
            //printf( ">>Request success [ResponseData=%s]/[ResponseSize=%d]\n", loResponse.BasePtr(), loResponse.GetSize() );
        }
        return 0;
    }
};

void* ThreadFunc( void* apvParam )
{
	for ( int liN = 0; liN < 500000; liN++ )
    {
        stParam* lpoParam = (stParam*)apvParam;
		AIChunkEx loResponse;
        if ( lpoParam->coProxyApi.Request( lpoParam->coBuffer.BasePtr(), lpoParam->coBuffer.GetSize(), loResponse, 10 ) != 0 )
        {
            printf( "Request fail,[MsgID=%s]\n", "xxx" );
        }
        else
        {
            //printf( ">>Request success [ResponseData=%s]/[ResponseSize=%d]\n", loResponse.BasePtr(), loResponse.GetSize() );
        }     
     }   
     return NULL;
}

int main( int argc, char* argv[] )
{
    stParam loParam;
    int     liOptChar;
    int     liInsertCount = 1;
    int     liThreadCount = 1;
    while( (liOptChar = getopt(argc, argv, "n:t:" )) != EOF )
	{
		switch (liOptChar)
		{
			case 'n':
			    liInsertCount = atoi(optarg);
			    break;
			case 't':
			    liThreadCount = atoi(optarg);
			    break;
		}
	}
    
    const char* lpcXml = "<gw-scp> "
                         "<sm-chg-req sp-id=\"8888\" "
                         "service-id=\"tpcq315513505000030\" "
                         "msg-id=\"061815020010050065535\" "
                         "src-gw=\"001001\" "
                         "gw-id=\"002001\" "
                         "smc-addr=\"13907551234\" pri=\"0\"> "
                         "<dst><addr val=\"13611111111\"></addr></dst > "
                         "<chg-info	msid=\"13812345678\" rate=\"50\" type=\"1\"> "
                         "</chg-info> "
                         "</sm-chg-req> "
                         "</gw-scp>";
                         
    loParam.coProxyApi.Initialize();
    loParam.coProxyApi.Connect( "10.3.18.99", 8012, 1, 10 );
    
    SCPMsgHeader loMsgHeader( MSG_TYPE_REQUEST );
    
    loMsgHeader.GeneralHeader()->SetCSeq( 0 );
    loMsgHeader.GeneralHeader()->SetFrom( "10.3.18.99" );
    loMsgHeader.GeneralHeader()->SetTo( "10.3.3.108" );
    loMsgHeader.GeneralHeader()->SetServiceKey( 12 );
    loMsgHeader.SetXmlMsg(lpcXml);
    loMsgHeader.Encode(loParam.coBuffer);
   /*
    pthread_t thrd, thrd1,thrd2;
   
    pthread_create( &thrd, NULL, ThreadFunc, &loParam ); 
    //pthread_create( &thrd1, NULL, ThreadFunc, &loParam1 );
    AITimeMeter loTimeMeter;
    pthread_join(thrd, NULL ); 
    //pthread_join(thrd1, NULL ); 
    loTimeMeter.Snapshot();
    printf( "Complete ... use %fs, %f/sec\n", loTimeMeter.GetTime(), 1000000/loTimeMeter.GetTime() );
   return 0;*/
    /*
    AITimeMeter loTimeMeter;
    for( int liN = 0; liN < 10000; liN++ )
    {        
        if ( loProxyApi.Request( loBuffer.BasePtr(), loBuffer.GetSize(), loResponse, 20 ) != 0 )
        {
            printf( "Request fail,[MsgID=%s]\n", "xxx" );
        }
        else
        {
            //printf( ">>Request success [ResponseData=%s]/[ResponseSize=%d]\n", loResponse.BasePtr(), loResponse.GetSize() );
        }
        //getchar();
    }
    loTimeMeter.Snapshot();
    printf( "Complete ... use %fs, %f/sec\n", loTimeMeter.GetTime(), 10000/loTimeMeter.GetTime() );
    */
    MyTestCase loTestCase;
    AITest::AITestAction* lpTestAction = NULL;
        
    loTestCase.SetRepeatCount( liInsertCount );
    lpTestAction = new AITest::AITestParallel(liThreadCount);
        
    lpTestAction->SetTestCase( &loTestCase );
	lpTestAction->Run( &loParam );
	
    printf( " Total use time(s)               = %lf\n",   loTestCase.GetUseTime() );
	printf( " Record count per                = %lf/s\n", loTestCase.GetFrequency() );
    
    return 0;
}
