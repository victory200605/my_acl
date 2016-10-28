
//#define AI_TEST_DEBUG 1;

#include "gfq/GFQueueApi.h"
#include "gfq/GFQUtility.h"
#include "./AITest/TestCase.h"
#include "./AITest/TestAction.h"
#include "./AITest/TestParallel.h"
#include "./AITest/TestControl.h"
#include "./AITest/TestSnapshot.h"
#include "AICrypt.h"
#include "AITime.h"
#include <string.h>
#include <errno.h>

extern int optind;

struct ISMGProtocols
{
    //cmpp
    uint8_t cmpp_version;           //cmpp版本，2表示CMPP20，3表示CMPP30
    uint8_t pk_total;       
    uint8_t pk_number;
    char    msg_id[8+1];
    char    msg_src[12+1];
    char    service_id[10+1];
    char    source_addr[21+1];
    char    destination_addr[21+1];
    uint8_t fee_ter_type;               //计费号码类型：0：真实号码；1：伪码
    char    fee_ter[21+1];              //计费号码
    char    fee_ter_pseudo[32];         //计费号码伪码
    char    fee_ter_acode[5+1];         //计费号码区号
    uint8_t fee_ter_usertype;           //计费号码用户类型：0：全球通；1：神州行    （30FWD中使用）
    uint8_t phone_ter_type;             //手机号码类型：0：真实号码；1：伪码    （MT为目的号码，MO为源号码）
    char    phone_ter_pseudo[32];       //手机号码伪码（MT为目的号码，MO为源号码）
    uint8_t phone_usertype;             //手机号码用户类型：0：全球通；1：神州行    （30FWD中使用）
    uint8_t fee_usertype;               //计费用户类型字段：0：对目的终端MSISDN计费；1：对源终端MSISDN计费；2：对SP计费；3：表示本字段无效，对谁计费参见fee_ter字段
    char    fee_type[2+1];                  
    char    fee_code[6+1];
    char    src_gwid[6+1];
    char    dest_gwid[6+1];
    char    link_id[20+1];
    uint8_t fwd_type;
    uint8_t node_count;
    uint8_t priority;               //msg_level
    uint8_t tp_pid;
    uint8_t tp_udhi;
    char    at_time[17+1];
    char    valid_time[17+1];
    uint8_t registered_delivery;
    uint8_t msg_fmt;
    uint8_t destusr_tl;
    uint8_t sm_length;
    char    short_message[160];
    
    //for status report
    char    sr_msgid[48+1]; 
    char    sr_status[7+1];
    char    sr_donetime[10+1];
    
    //extra
    time_t  ciRecvTime;                 //短信接收时间,对应话单的“申请时间”字段
    int     ciResendCount;              //重发次数
    int     ciSMCSeq;                   //for MO stat
    char    csServiceNo[21+1];          //SP服务号码
    char    csLastProcessModule[15+1];  //最后一次处理该短信的模块
    time_t  ciLastProcessTime;          //最后一次处理该短信的时间
    char    csLastStatus[7+1];          //最后一次处理短信的状态
    uint8_t ccMsgType;
};
void PaserMsgId(char* apcMsgId, int* apiSegNo, int* apiTime, char* apcIsmgNo );
void SetFromIni(ISMGProtocols* apoData, AIConfig* apoIni, char const* apcSection, char* apcMsgID, size_t aiSize );
void EncodeCMPPMsgID(char* apcMsgID, int aiMsgID, char const* apcTime, char const* apcISMGNo);

//------------------------------------clsValue-------------------------------//
//use for gen message id
class clsValue
{
public:
    clsValue( int aiStart = 0, int aiStep = 1 ) : 
        ciCount(aiStart),ciStep(aiStep),ciRevTimeInterval(0),cbIsmgProtocol(false),ciTime(-1)
    {}
    virtual ~clsValue()
    {}
    
    void SetIsmgProtocol()
    {
        AIConfig* lpoIni = AIGetIniHandler("ISMGProtocols.ini");
        if ( lpoIni == NULL )
        {
            printf( "Get ISMGProtocol ini fail\n" );
            return;
        }
        SetFromIni( &coProtocol, lpoIni, "default", csMsgID, sizeof(csMsgID) );
        PaserMsgId( csMsgID, &ciSeqNo, &ciTime, csIsmgNo );
        ciRevTimeInterval = coProtocol.ciRecvTime;
        cbIsmgProtocol = true;
        
    }
    virtual int GetValue( AIChunk& aoChunk )
    {
        int  liMsgID;
        char lsTime[32] = {0};
        
        if ( ciTime < 0 )
        {
            AIFormatLocalTime(lsTime, sizeof(lsTime), "%m%d%H%M%S", AICurTime());
        }
        
        if ( cbIsmgProtocol )
        {
            AISmartLock loLock( coLock );
            
            if ( ciSeqNo < 0 )
            {
                liMsgID = ciCount;
            }
            
            if ( ciRevTimeInterval == 0 )
            {
                coProtocol.ciRecvTime = time(NULL);
            }
            else if ( ciRevTimeInterval < 0 )
            {
                coProtocol.ciRecvTime = time(NULL) + ciRevTimeInterval;
            }
            else
            {
                coProtocol.ciRecvTime = ciRevTimeInterval;
            }
            
            EncodeCMPPMsgID( coProtocol.msg_id, liMsgID, lsTime, csIsmgNo );
            memcpy( aoChunk.GetPointer(), &coProtocol, sizeof(coProtocol) );
            
            ciCount += ciStep;
        }
        else
        {
            AISmartLock loLock( coLock );
            ::snprintf(aoChunk.GetPointer(), aoChunk.GetSize(), "%010d", ciCount++ );
        }

        return 0;
    }
    
    char* FormatTime(const time_t aiTimeValue, char* apsBuff, const char* apsFromat )
    {
    	struct tm tmT=*localtime(&aiTimeValue);
    	sprintf(apsBuff,apsFromat,tmT.tm_year+1900,tmT.tm_mon+1,tmT.tm_mday,
    		tmT.tm_hour,tmT.tm_min,tmT.tm_sec);
    	return apsBuff;
    }
    
protected:
    int ciCount;
    int ciStep;
    int ciRevTimeInterval;
    int cbIsmgProtocol;
    char csMsgID[32];
    int ciSeqNo;
    int ciTime;
    char csIsmgNo[32];
    
    ISMGProtocols coProtocol;
    AIMutexLock coLock;
};
//------------------------------------clsFileValue-------------------------------//
//use for gen message id from file
class clsFileValue : public clsValue
{
public:
    clsFileValue( const char* apsFileName, bool abIsBinary = false )
    {
        cpFile = fopen( apsFileName, "r" );
        if ( cpFile == NULL ) printf( "Error:Open file %s fail - %s\n", apsFileName, strerror(errno) );
        assert ( cpFile != NULL );
        
        cbIsBinary = abIsBinary;
    }
    virtual ~clsFileValue() 
    {
        fclose(cpFile);
    }
    virtual int GetValue( AIChunk& aoChunk )
    {
        int liRet = 0;
        char* apsValueBuff = aoChunk.GetPointer();
        int aiLen = aoChunk.GetSize();
        
        AISmartLock loLock( coLock );
        if ( fgets( apsValueBuff, aiLen, cpFile ) == NULL )
        {
            liRet = -1;
        }
        if ( apsValueBuff[strlen(apsValueBuff) - 1] == '\n' ) 
        {
            apsValueBuff[strlen(apsValueBuff) - 1] = 0;
        }
        if ( apsValueBuff[strlen(apsValueBuff) - 1] == '\r' ) 
        {
            apsValueBuff[strlen(apsValueBuff) - 1] = 0;
        }
        
        if ( cbIsBinary )
        {
            AIChunk loChunk;
            AscStringToBinary( apsValueBuff, loChunk );
            loChunk.Swap(aoChunk);
        }
        else
        {
            AIChunk loChunk( (size_t)strlen(apsValueBuff) );
            memcpy( loChunk.GetPointer(), apsValueBuff, loChunk.GetSize() );
            loChunk.Swap(aoChunk);
        }
        
        return liRet;
    }
protected:
    char AscCharToBinary( char aCh )
    {
        if ( aCh >= '0' &&  aCh <= '9' )
    	{
    		return aCh - '0' ;
    	}
    	else if ( aCh >= 'a' &&  aCh <= 'f' )
    	{
    		return aCh - 'a' + 10 ;
    	}
    	else if ( aCh >= 'A' && aCh <= 'F' )
    	{
    		return aCh - 'A' + 10 ;
    	}
    	else 
    	{				
    		assert( false );
    	}
    	return 0;
    }
    void AscStringToBinary( const char* apcSrc, AIChunk& aoRet )
    {
    	size_t i = 0;
    	size_t j = 0;
    	size_t liSrcLen    = strlen(apcSrc) + 1;
    	size_t liNewSrcLen = 0;
    	char* lpsTmpSrc = new char[ liSrcLen ];
    	memset( lpsTmpSrc, 0, liSrcLen );
    	
    	// Skip blank
    	while( *apcSrc != '\0' )
        {
            if ( *(apcSrc) != ' ' ) lpsTmpSrc[liNewSrcLen++] = *(apcSrc++);
        }

    	assert ( liNewSrcLen % 2 == 0 );
    	aoRet.Resize(liNewSrcLen / 2);
    	
    	unsigned char* lpsRet = (unsigned char*)aoRet.GetPointer();
    
    	for ( ; j < liNewSrcLen; i++, j += 2 )
    	{
    		lpsRet[i] = AscCharToBinary( lpsTmpSrc[j] );
    		
    		lpsRet[i] <<= 4;
    		
    		lpsRet[i] |= AscCharToBinary( lpsTmpSrc[j + 1] );			
    	}
    	
    	delete[] lpsTmpSrc;
    }
protected:
    FILE* cpFile;
    AIMutexLock coLock;
    bool  cbIsBinary;
};

//------------------------------------stParam-------------------------------//
//global param struct
struct stParam
{
    clsGFQueueApi coGFQueueApi;
    clsValue*     cpoValue;
    char          ccQueueName[AI_MAX_NAME_LEN];
    bool          cbIsTerminate;
    bool          cbIsBinary;
    bool          cbIsCheck;
    int           ciEndOfQueueSleep;
    int           ciWhenSec;
};

class AIPutTestCase : public AITest::AITestCase
{
public:
    virtual int DoIt( void* apParam )
    {
        AIChunk loValue(sizeof(ISMGProtocols));
	
        stParam* lpoParam = (stParam*)apParam;
        if ( lpoParam->cpoValue->GetValue( loValue ) != 0 )
        {
            return -1;
        }
        
        int liRet = lpoParam->coGFQueueApi.Put( lpoParam->ccQueueName, loValue );
        if ( liRet != 0 )
        {
            if ( lpoParam->cbIsTerminate )
            {
                printf( "Put data fail, MSGCODE:%d, [Data=%s]\n", liRet, loValue.GetPointer() );
            }
            return -1;
        }
        else
        {
        	if ( lpoParam->cbIsTerminate )
            {
                if ( lpoParam->cbIsBinary )
                {
                    char lcBuff[10];
                    printf( "Put data success, [Data=" );
                    for( size_t liIt = 0; liIt < loValue.GetSize(); liIt++ )
                    {
                        printf( "%s ", CharToBinary( ((unsigned char*)loValue.GetPointer())[liIt], lcBuff, 10 ) );
                    }
                    printf( "]\n" );
                }
                else 
                {
                    printf( "Put data success, [Data=%s]\n", loValue.GetPointer() );
                }
            }
        }
        
        return 0;
    }
};

class AIGetTestCase : public AITest::AITestCase
{
public:
    virtual int DoIt( void* apParam )
    {
        AIChunk loValue(1024);
		stParam* lpoParam = (stParam*)apParam;
        int liRet = lpoParam->coGFQueueApi.Get( lpoParam->ccQueueName, loValue, lpoParam->ciWhenSec );
        if ( liRet != 0 )
        {
            if ( lpoParam->cbIsTerminate )
            {
                printf( "Get data fail, MSGCODE:%d, [Data=%s]\n", liRet, loValue.GetPointer() );
            }
            
            if ( lpoParam->ciEndOfQueueSleep > 0 && liRet == AI_ERROR_END_OF_QUEUE ) 
            {
            	printf( "Get data fail - End of queue, Sleep %ds now ... \n", lpoParam->ciEndOfQueueSleep );
            	sleep(lpoParam->ciEndOfQueueSleep);
            }
            
            return -1;
        }
        else
        {
            if (lpoParam->cbIsCheck)
            {
                AIChunk loTmp(1024);
                if ( lpoParam->cpoValue->GetValue( loTmp ) != 0 )
                {
                    printf( "Check Get value fail\n" );
                    return -1;
                }
                
                if ( strcmp(loTmp.GetPointer(), loValue.GetPointer()) != 0 )
                {
                    printf( "Check fail [%s!=%s]\n", loTmp.GetPointer(), loValue.GetPointer() );
                    return -1;
                }
            }
                
        	if ( lpoParam->cbIsTerminate )
            {
                if ( lpoParam->cbIsBinary )
                {
                    char lcBuff[10];
                    printf( "Get data success, [Data=" );
                    for( size_t liIt = 0; liIt < loValue.GetSize(); liIt++ )
                    {
                        printf( "%s ", CharToBinary( ((unsigned char*)loValue.GetPointer())[liIt], lcBuff, 10 ) );
                    }
                    printf( "]\n" );
                }
                else 
                {
                    printf( "Get data success, [Data=%s]\n", loValue.GetPointer() );
                }
            }
        }
        return 0;
    }
};

class AIGetStatTestCase : public AITest::AITestCase
{
public:
    virtual int DoIt( void* apParam )
    {
        int liSize    = 0;
        int liCapaciy = 0;
		stParam* lpoParam = (stParam*)apParam;
		
        int liRet = lpoParam->coGFQueueApi.GetStat( lpoParam->ccQueueName, &liSize, &liCapaciy );
        if ( liRet != 0 )
        {
            if ( lpoParam->cbIsTerminate )
            {
                printf( "Get stat fail, MSGCODE:%d\n", liRet );
            }
            return -1;
        }
        else
        {
        	printf( "Get stat success, [size=%d]/[capacity=%d]\n", liSize, liCapaciy );
        }
        return 0;
    }
};

int ParseCmd( const char* apsCmd, char* argv[], int aiMax )
{
	int liIdx = 1;
	int liPos = 0;
	
#define IS_CMD_END( cmd ) ( *apsCmd == '\0' || *apsCmd == '\n' )
	
	while( !IS_CMD_END(apsCmd) )
	{
		// Skip blank
		while( !IS_CMD_END(apsCmd) && *apsCmd == ' ' ) apsCmd++;
		
		if ( liIdx >= aiMax ) return -1;

		while( !IS_CMD_END(apsCmd) && *apsCmd != ' ' )
		{
			argv[liIdx][liPos++] = *apsCmd++;
		}
		argv[liIdx++][liPos] = '\0';
		liPos = 0;
	}
	
	return liIdx;
}

int _Main( int argc, char* argv[], stParam& aoParam );

int main( int argc, char* argv[] )
{
    AIInitIniHandler();
    AIInitLOGHandler();
    
	stParam loParam;
	if ( loParam.coGFQueueApi.Initialize( "../config/GFQClient.ini" ) != 0 )
	{
	    printf( "GFQ client initialize fail, may be connect rejected - %s\n", strerror(errno) );
	    return -1;
	}
	
	if ( argc > 1 )
	{
		_Main( argc, argv, loParam );
	}
	else
	{
    	char lsCmdBuff[4096];
    	char** lppArgv = new char*[100];
    	memset( lsCmdBuff, 0, sizeof(lsCmdBuff) );
    	memset( lppArgv, 0, sizeof(char*) * 100 );
    	for( int liIt = 0; liIt < 100; liIt++ )
    	{
    		lppArgv[liIt] = new char[128];
    		memset( lppArgv[liIt], 0, 128 );
    	}
    	strcpy( lppArgv[0], argv[0] );
    	
    	while( true )
    	{
    		fprintf( stdout, "%s>", lppArgv[0] );
    		fgets( lsCmdBuff, sizeof(lsCmdBuff), stdin );
    		
    		int liArgc = ParseCmd( lsCmdBuff, lppArgv, 100 );
    		if ( liArgc == 1 ) continue;
    		if ( liArgc == 2 && ( strcmp( lppArgv[1], "q" ) == 0 || strcmp( lppArgv[1], "Q" ) == 0 ) )
    		{
    			break;
    		}
    		_Main( liArgc, lppArgv, loParam );
    	}
    }
    
	loParam.coGFQueueApi.Shutdown();
	
	AICloseLOGHandler();
    AICloseIniHandler();
    
	return 0;
}

int _Main( int argc, char* argv[], stParam& aoParam )
{
    int liOpValue = 0;
    bool lbDeleteTimeOut = false;
    bool lbFileKey = false;
    bool lbISMGProtocol = false;
    int liStatisticalTimeInterval = 0;
    int liMaxUnitFlow = 0;
    int liThreadCnt = 1;
    int liCount = 0;
    int liStartValue = 0;
    int liOptChar;
    int liStep = 1;
    
    char lsFileName[128];
   
    aoParam.cpoValue = NULL;
    aoParam.cbIsTerminate = false;
    aoParam.cbIsBinary = false;
    aoParam.cbIsCheck = false;
    aoParam.ciEndOfQueueSleep = 0;
    aoParam.ciWhenSec = 0;
    strcpy( aoParam.ccQueueName, "MyQueue" );
    optind = 1;
    while( (liOptChar = getopt(argc, argv, "id:l:s:f:t:n:k:poe:gq:bx:yc" )) != EOF )
	{
		switch (liOptChar)
		{
			case 'i':
			    liOpValue = 0;
			    break;
			case 'd':
			    liOpValue = 1;
			    aoParam.ciWhenSec = atoi(optarg);
				break;
			case 'g':
			    liOpValue = 2;
				break;
			case 's':
				liStatisticalTimeInterval = atoi(optarg);
				break;
			case 'f':
			    lbFileKey = true;
			    strcpy( lsFileName, optarg );
			    break;
			case 't':
			    liThreadCnt = atoi(optarg);
			    break;
			case 'l':
			    liCount = atoi(optarg);
			    break;
			case 'n':
			    liMaxUnitFlow = atoi(optarg);
			    break;
			case 'k':
			    liStartValue = atoi(optarg);
			    break;
			case 'p':
			    aoParam.cbIsTerminate = true;
			    break;
			case 'o':
			    lbDeleteTimeOut = true;
			    break;
			case 'e':
			    liStep = atoi(optarg);
			    break;
			case 'q':
			 	strcpy( aoParam.ccQueueName, optarg );
			 	break;
			case 'b':
			 	aoParam.cbIsBinary = true;
			 	break;
			case 'x':
				aoParam.ciEndOfQueueSleep = atoi(optarg);
				break;
		    case 'y':
		        lbISMGProtocol = true;
				break;
		    case 'c':
		        aoParam.cbIsCheck = true;
		        break;
			default:
			    printf("usage invalid param\n");
			    break;
		}
	}
	if ( lbFileKey )
	{
	    aoParam.cpoValue = new clsFileValue(lsFileName, aoParam.cbIsBinary);
	}
	else
	{
	    aoParam.cpoValue = new clsValue( liStartValue, liStep );
	    if (lbISMGProtocol) aoParam.cpoValue->SetIsmgProtocol();
	}

	AITest::AITestCase* lpCase = NULL;
	switch( liOpValue )
	{
	case 1:
		lpCase = new AIGetTestCase;
		break;
	case 2:
	    lpCase = new AIGetStatTestCase;
	    liCount = 1;
	    break;
	default:
	    lpCase = new AIPutTestCase;
	}
	
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
	lpTestAction->Run( &aoParam );
    
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
	
	// Release resource
	delete aoParam.cpoValue;
	delete lpCase;
	delete lpTestAction;
	
	aoParam.cpoValue = NULL;
	lpCase = NULL;
	lpTestAction = NULL;

    return 0;
}

DECLARE_CMDOPT_BEGIN()
DECLARE_CMDOPT_END()

void EncodeCMPPMsgID(char* apcMsgID, int aiMsgID, char const* apcTime, char const* apcISMGNo)
{
    char    liChar = 0;
    int     liValue = 0;
    char    *lpcValue = (char *)&liValue;
    unsigned long liTime = strtoul(apcTime, NULL, 10);

    liChar = (liTime/100000000%100) & 0x0F;
    apcMsgID[0] = (liChar << 4);

    liChar = ((liTime/1000000)%100) & 0x1F;
    apcMsgID[0] = apcMsgID[0] | ((liChar >> 1) & 0x0F);
    apcMsgID[1] = ((liChar & 0x01) << 7);

    liChar = ((liTime/10000)%100) & 0x1F;
    apcMsgID[1] = apcMsgID[1] | ((liChar << 2) & 0x7C);

    liChar = ((liTime/100)%100) & 0x3F;
    apcMsgID[1] = apcMsgID[1] | ((liChar >> 4) & 0x03);
    apcMsgID[2] = ((liChar & 0x0F) << 4);

    liChar = (liTime%100) & 0x3F;
    apcMsgID[2] = apcMsgID[2] | ((liChar >> 2) & 0x0F);
    apcMsgID[3] = ((liChar & 0x03) << 6);
    
    liValue = htonl(strtoul( apcISMGNo, (char**)NULL, 10 ) & 0x003FFFFF);
    apcMsgID[3] = apcMsgID[3] | (lpcValue[1] & 0x3F);
    memcpy( apcMsgID+4, lpcValue+2, 2 );

    liValue = htonl(aiMsgID & 0x0000FFFF);
    memcpy( apcMsgID+6, lpcValue+2, 2 );
}

void PaserMsgId(char* apcMsgId, int* apiSegNo, int* apiTime, char* apcIsmgNo )
{
    char* lpcTime;
    char* lpcIsmgNo;
    char* lpcSeqNo;

    lpcTime = apcMsgId;

    lpcIsmgNo = strchr(lpcTime, ':');
    if (NULL == lpcIsmgNo)
    {
        return;
    }
  
    *lpcIsmgNo = '\0'; 
    ++lpcIsmgNo;

    lpcSeqNo = strchr(lpcIsmgNo, ':');
    if (NULL == lpcSeqNo)
    {
        return;
    }

    *lpcSeqNo = '\0';
    ++lpcSeqNo;
    
    *apiSegNo = StringAtoi(lpcSeqNo);
    *apiTime  = StringAtoi(lpcTime);
    strcpy( apcIsmgNo, lpcIsmgNo );
}

void SetFromIni(ISMGProtocols* apoData, AIConfig* apoIni, char const* apcSection, char* apcMsgID, size_t aiSize )
{
    char    lsBase64[256];

    #define GET_INT(field) apoData->field = apoIni->GetIniInt(apcSection, #field, apoData->field)
    #define GET_STR(field) apoIni->GetIniString(apcSection, #field, apoData->field, sizeof(apoData->field))

    memset(apoData, 0, sizeof(*apoData));
    memset(lsBase64, 0, sizeof(lsBase64));

    GET_INT(cmpp_version);
    GET_INT(pk_total);
    GET_INT(pk_number);
    
    apoIni->GetIniString(apcSection, "msg_id", apcMsgID, aiSize );
    
    GET_STR(msg_src);
    GET_STR(service_id);
    GET_STR(source_addr);
    GET_STR(destination_addr);
    GET_INT(fee_ter_type);
    GET_STR(fee_ter);
    GET_STR(fee_ter_pseudo);
    GET_STR(fee_ter_acode);
    GET_INT(fee_ter_usertype);
    GET_INT(phone_ter_type);
    GET_STR(phone_ter_pseudo);
    GET_INT(phone_usertype);
    GET_INT(fee_usertype);
    GET_STR(fee_type);
    GET_STR(fee_code);
    GET_STR(src_gwid);
    GET_STR(dest_gwid);
    GET_STR(link_id);
    GET_INT(fwd_type);
    GET_INT(node_count);
    GET_INT(priority);
    GET_INT(tp_pid);
    GET_INT(tp_udhi);
    GET_STR(at_time);
    GET_STR(valid_time);
    GET_INT(registered_delivery);
    GET_INT(msg_fmt);
    GET_INT(destusr_tl);

    apoIni->GetIniString(apcSection, "short_message(BASE64)", lsBase64, sizeof(lsBase64));
    StringTrim(lsBase64);
    if (strlen(lsBase64) == 0)
    { // read from sm_length and short_message
        GET_INT(sm_length);
        GET_STR(short_message);
    }
    else
    { // decode from short_message(BASE64)
        ssize_t liRetSize = BASE64Decode(lsBase64, strlen(lsBase64), apoData->short_message, sizeof(apoData->short_message));
        if (liRetSize < 0)
        {
            printf("WARN: failed to decode base64 data, short_message is undefine!\n");
            apoData->sm_length = 0;
        }
        else
        {
            apoData->sm_length = liRetSize;
        }
    }

    GET_STR(sr_msgid);
    GET_STR(sr_status);
    GET_STR(sr_donetime);
    GET_INT(ciRecvTime);
    GET_INT(ciResendCount);
    GET_INT(ciSMCSeq);
    GET_STR(csServiceNo);
    GET_STR(csLastProcessModule);
    GET_INT(ciLastProcessTime);
    GET_STR(csLastStatus);
    GET_INT(ccMsgType);

    #undef GET_STR
    #undef GET_INT
}

#if defined(AI_OS_AIX)
    //Make compiler happy on AIX env
#   include "AIProcBase.h"
    ProgOptParam moProgOptArray[1];
#endif

