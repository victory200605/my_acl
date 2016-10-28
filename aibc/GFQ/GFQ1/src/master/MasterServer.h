//=============================================================================
/**
 * \file    MasterServer.h
 * \brief 
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: MasterServer.h,v 1.2 2010/12/20 08:19:04 daizh Exp $
 *
 * History
 * 2008.04.04 first release by DZH
 */
//=============================================================================
#ifndef AI_MasterServer_H_2008
#define AI_MasterServer_H_2008

#include <stdio.h>
#include <unistd.h>
#include "GFQueue.h"
#include "../common/AITask.h"
#include "gfq/GFQUtility.h"
#include "gfq/GFQProtocol.h"

AI_GFQ_NAMESPACE_START

class clsMasterServer : public clsAsyncServer
{
public:
	// Define for thread param
	struct stThreadParam
	{
		stThreadParam( clsMasterServer* apoServer, const ASYNCMATCH* apoMatch, char* apsData, size_t aiSize );
		clsMasterServer* cpoServer;
		ASYNCMATCH       coMatch;
		AIChunkEx        coData;
		int              ciCmd;
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
    clsMasterServer( const unsigned short aiBindPort );
    
    ~clsMasterServer();

    virtual void Callback( const ASYNCMATCH *apoMatch, char *apcData, const int aiSize );
    
    int Initialize( const char *apcConfigFile );
    
    int Startup();
    
    bool IsShutdown();
    
    void PrintStat();
    
    //thread operator
    static void* HandleRequestEntry( void *apvParam );

	void HandleRequestThread( ASYNCMATCH *apoMatch, int aiCmd, AIChunkEx& aoChunk );

    //request operator
    int DoRequest( clsRequestAddModule& aoRequest, clsResponseAddModule& aoResponse );
    int DoRequest( clsRequestDelModule& aoRequest, clsResponseDelModule& aoResponse );
    int DoRequest( clsRequestAddSeg& aoRequest, clsResponseAddSeg& aoResponse );
    int DoRequest( clsRequestAddSegTry& aoRequest, clsResponseAddSeg& aoResponse );
    
    int DoRequest( clsRequestWriteSeg& aoRequest, clsResponseWriteSeg& aoResponse );
    int DoRequest( clsRequestWriteSegSecond& aoRequest, clsResponseWriteSegSecond& aoResponse );
    int DoRequest( clsRequestReadSeg& aoRequest, clsResponseReadSeg& aoResponse );
    int DoRequest( clsRequestReadSegSecond& aoRequest, clsResponseReadSegSecond& aoResponse );
    
    int DoRequest( clsRequestQueueStat& aoRequest, clsResponseQueueStat& aoResponse );
    int DoRequest( clsRequestPrintQueue& aoRequest, clsResponsePrintQueue& aoResponse );
    int DoRequest( clsRequestPrintModule& aoRequest, clsResponsePrintModule& aoResponse );
    int DoRequest( clsRequestModuleCount& aoRequest, clsResponseModuleCount& aoResponse );
    
    int DoRequest( clsRequestReloadQueueInfo& aoRequest, clsResponseReloadQueueInfo& aoResponse );
    
    int DoRequest( clsRequestHeartBeat& aoRequest, clsResponseHeartBeat& aoResponse );
    
    //send response operator
    void SendResponse( const ASYNCMATCH *apoMatch, int aiCmd, clsBody* apoBody );
    
//attr member
protected:
    AITask* cpoThreadPool;
    int ciThreadCount;

    //adv file queue
    clsGFQueue coGFQueue;
    
    //config member
    int ciShutDownTimeout;
    int ciListenPort;
    int ciPrintInterval;
    int ciHeartBeatInterval;
    double cdTrapLimitPercent;
    
    // Statistic stat
    stRequestStat coRequestStat;
};

AI_GFQ_NAMESPACE_END

#endif //AI_MasterServer_H_2008
