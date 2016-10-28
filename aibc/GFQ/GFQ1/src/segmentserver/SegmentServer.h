//=============================================================================
/**
 * \file    SegmentServer.h
 * \brief 
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: SegmentServer.h,v 1.3 2011/03/01 06:39:54 daizh Exp $
 *
 * History
 * 2008.04.04 first release by DZH
 */
//=============================================================================
#ifndef AI_SEGMENTSERVER_H_2008
#define AI_SEGMENTSERVER_H_2008

#include "gfq/GFQUtility.h"
#include "../common/AITask.h"
#include "gfq/GFQProtocol.h"
#include "SegmentStore.h"

AI_GFQ_NAMESPACE_START

class clsSegmentServer : public clsAsyncServer
{
public:
	// Define for thread param
	struct stThreadParam
	{
		stThreadParam( clsSegmentServer* apoServer, const ASYNCMATCH* apoMatch, char* apsData, size_t aiSize );
		clsSegmentServer* cpoServer;
		ASYNCMATCH        coMatch;
		AIChunkEx         coData;
		int               ciCmd;
	};
	
	// Statistic information
	struct stRequestStat
	{
	    stRequestStat() : ciRequestCnt(0),ciTimestamp(time(NULL)) {};
	    int  Frequency()    { int liInterval = time(NULL) - ciTimestamp; return ciRequestCnt / ( liInterval > 0 ? liInterval : 1 ); }
	    void Reset()        { ciRequestCnt = 0; ciTimestamp = time(NULL); }
	    void Request()      { ciRequestCnt++; }
	    
	    int ciRequestCnt;
	    int ciTimestamp;
	};
	
public:
    clsSegmentServer( const unsigned short aiBindPort );
    ~clsSegmentServer();

    int Initialize( const char *apcConfigFile, const char* apsClientConfig );
    
    int Startup(void);
    
    bool IsShutdown();
    
protected:
    virtual void Callback( const ASYNCMATCH *apoMatch, char *apcData, const int aiSize );
    
    int DelModule(void);
    int HeartBeat(void);
    int RegisterToMaster( const char* apsModuleName, const char* apsServerIP, int aiServerPort );
    
    //thread operator
    static void* HandleRequestEntry( void *apvParam );

	void HandleRequestThread( ASYNCMATCH *apoMatch, int aiCmd, AIChunkEx& aoChunk );
	
    //request operator
    int DoRequest( clsRequestAlloc& aoRequest, clsResponseAlloc& aoResponse );
    int DoRequest( clsRequestFree& aoRequest, clsResponseFree& aoResponse );
    
    int DoRequest( clsRequestPut& aoRequest, clsResponsePut& aoResponse );
    int DoRequest( clsRequestGet& aoRequest, clsResponseGet& aoResponse );
    int DoRequest( clsRequestSegmentSize& aoRequest, clsResponseSegmentSize& aoResponse );
    
    void PrintStat();
    
    //send response operator
    void SendResponse( const ASYNCMATCH *apoMatch, int aiCmd, clsBody* apoBody ); 
    
//attr member
protected:
    AITask* cpoThreadPool;
    int ciThreadCount;
        
    int   ciMasterPort;
    char  ccMasterIp[AI_IP_ADDR_LEN];
    clsAsyncClient* cpoMaster;

    //adv file queue
    IBaseStore* cpoSegmentStore;
    //clsSegmentStore coSegmentStore;
    
    //config member
    int ciShutDownTimeout;
    int ciPrintInterval;
    int ciHeartBeatInterval;
    int ciRegisterIdlePercent;
    double cdTrapLimitPercent;
    
    char ccModuleName[AI_MAX_NAME_LEN];
    char ccServerIp[AI_IP_ADDR_LEN];
    int  ciServerPort;

    // Statistic stat
    stRequestStat coRequestStat;
    
    bool cbIsService;
};

AI_GFQ_NAMESPACE_END

#endif //AI_SEGMENTSERVER_H_2008
