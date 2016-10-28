
#include "acl/SockConnector.h"
#include "acl/Timestamp.h"
#include "acl/GetOpt.h"
#include "../protocol/CountServerProtocol.h"
#include "../api/CountServerApi.h"
#include "./AITest/TestCase.h"
#include "./AITest/TestAction.h"
#include "./AITest/TestParallel.h"
#include "./AITest/TestControl.h"
#include "./AITest/TestSnapshot.h"

struct CTestParam
{
    CTestParam(void)
        : miNum(0)
    {
        apl_strncpy(this->macKey, "INVALIDSP", sizeof(this->macKey) );
    }
    
    CCountServerApi moApi;
    char macKey[64];
    apl_int_t miNum;
};

class AICounterTestCase : public AITest::AITestCase
{
public:
    virtual int DoIt( void* apParam )
    {
        CTestParam* lpoParam = (CTestParam*)apParam;

        return lpoParam->moApi.AcquireConnToken( lpoParam->macKey, lpoParam->miNum, 2 );
    }
};

class AISpeederTestCase : public AITest::AITestCase
{
public:
    virtual int DoIt( void* apParam )
    {
        CTestParam* lpoParam = (CTestParam*)apParam;

        return lpoParam->moApi.AcquireSpeedToken( lpoParam->macKey, lpoParam->miNum, 2 );
    }
};

int main( int argc, char* argv[] )
{
    apl_int_t liOpValue = 0;
    apl_int_t liStatisticalTimeInterval = 0;
    apl_int_t liMaxUnitFlow = 0;
    apl_int_t liThreadCnt = 1;
    apl_int_t liCount = 0;
    apl_int_t liOptChar;
    char lacIpAddress[20] = "10.3.18.170";
    apl_uint16_t li16Port = 1234;
    
    acl::CGetOpt loOpt(argc, argv, "i:p:l:s:n:t:x:y:z:");
    
    loOpt.LongOption("ip",       'i', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("port",     'p', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("loop",     'l', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("statistical", 's', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("unitflow", 'n', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("parallel", 't', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("counter",  'x', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("speeder",  'y', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("num",      'z', acl::CGetOpt::ARG_REQUIRED);
    
    CTestParam loParam;
    
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
		    case 'l':
		        liCount = apl_strtoi32(loOpt.OptArg(), NULL, 10);
		        break;
			case 's':
				liStatisticalTimeInterval = apl_strtoi32(loOpt.OptArg(), NULL, 10);
				break;
			case 't':
			    liThreadCnt = apl_strtoi32(loOpt.OptArg(), NULL, 10);
			    break;
			case 'n':
			    liMaxUnitFlow = apl_strtoi32(loOpt.OptArg(), NULL, 10);
			    break;
			case 'x':
			    apl_strncpy(loParam.macKey, loOpt.OptArg(), sizeof(loParam.macKey) );
			    liOpValue = 0;
			    break;
			case 'y':
			    apl_strncpy(loParam.macKey, loOpt.OptArg(), sizeof(loParam.macKey) );
			    liOpValue = 1;
				break;
		    case 'z':
		        loParam.miNum = apl_strtoi32(loOpt.OptArg(), NULL, 10);
				break;
			default:
                apl_errprintf( "invalid option : %c\n", liOptChar );
			    /// Parser argument exception or do option -v
                apl_errprintf( "Usage : %s\n", argv[0] );
                apl_errprintf( " --ip            -i <string>   : ip address\n" );
                apl_errprintf( " --port          -p <num>      : server port\n" );
                apl_errprintf( " --loop          -l <num>      : loop\n" );
                apl_errprintf( " --statistical   -s <num/s>    : kill running process\n" );
                apl_errprintf( " --unitflow      -n <num/s>    : log level\n" );
                apl_errprintf( " --parallel      -t <num>      : parallel thread count\n" );
                apl_errprintf( " --counter       -x            : acquire counter\n" );
                apl_errprintf( " --speeder       -y            : acquire speeder\n" );
                apl_errprintf( " --num           -z <num>      : acquire num\n" );
			    return 0;
		}
	}

    if ( loParam.moApi.Initialize(lacIpAddress, li16Port, 3) != 0)
    {
        printf("Connect fail\n");
        return -1;
    }
    
	AITest::AITestCase* lpCase = NULL;
	switch ( liOpValue )
	{
	    case 0:
	        lpCase = new AICounterTestCase;
	        break;
	    case 1:
	        lpCase = new AISpeederTestCase;
	        break;
	};
	
	lpCase->SetRepeatCount(liCount);

	AITest::AITestAction* lpTestAction = NULL;
	if ( liThreadCnt > 1 )
	{
	    lpTestAction = new AITest::AITestParallel(liThreadCnt);
	}
	else
	{
	    lpTestAction = new AITest::AITestAction;
	}
	AITest::AITestSnapshot* lpSnapshot = NULL;
	if ( liStatisticalTimeInterval > 0 )
	{
	    lpSnapshot = new AITest::AITestSnapshot( lpTestAction,  liStatisticalTimeInterval );
	    lpTestAction = lpSnapshot;
	}
	
	if ( liMaxUnitFlow > 0 )
	{
	    lpTestAction = new AITest::AITestControl( lpTestAction,  1, liMaxUnitFlow );
	}

    lpTestAction->SetTestCase( lpCase );
	lpTestAction->Run( &loParam );
    
    printf( "--------------------------Complete Test--------------------------------\n" );
	printf( " Total Record count              = %d\n",   lpCase->GetSuccess() + lpCase->GetReject() );
	printf( " Success Record count            = %d\n",   lpCase->GetSuccess() );
	printf( " Reject Record count             = %d\n",   lpCase->GetReject() );
	printf( " Total use time(s)               = %lf\n",   lpCase->GetUseTime() );
	if ( lpSnapshot != NULL )
	{
	    printf( " Avg delay time(us)              = %d\n", lpSnapshot->GetAvgDelayTime() );
	    printf( " Max delay time(us)              = %d\n", lpSnapshot->GetMaxDelayTime() );
	    printf( " Min delay time(us)              = %d\n", lpSnapshot->GetMinDelayTime() );
	}
	printf( " Record count per                = %lf/s\n", lpCase->GetFrequency() );

    return 0;
}
