
#ifndef __AI_FILECACHESERVER_H__
#define __AI_FILECACHESERVER_H__

#include <stdio.h>
#include <unistd.h>
#include "cacheserver/CacheUtility.h"
#include "../common/Protocol.h"
#include "BillCache/BillCache.h"
#include "Cache.h"
#include "AITask.h"

AI_CACHE_NAMESPACE_START

class clsCacheServer : public AIBC::clsAsyncServer
{
public:
	// Define for thread param
	struct stThreadParam
	{
		stThreadParam( clsCacheServer* apoServer, const AIBC::ASYNCMATCH* apoMatch, char* apsData, size_t aiSize );
		clsCacheServer* cpoServer;
		AIBC::ASYNCMATCH          coMatch;
		AIBC::AIChunkEx     coData;
	};
	
public:
    static clsCacheServer* Instance();
    ~clsCacheServer();

    int Startup( const char *apcConfigFile );
    void Shutdown();
    
    void PrintStat();
    
protected:
    clsCacheServer();
    virtual void Callback( const AIBC::ASYNCMATCH *apoMatch, char *apcData, const int aiSize );
    
    //thread operator
    static void* HandleRequestEntry( void* apvParam );
    static void* AutoDelTimeoutEntry( void* apvParam );

	void HandleRequestThread( AIBC::ASYNCMATCH *apoMatch, AIBC::AIChunkEx& aoChunk );

    //request operator
    int DoRequest( clsRequestPut& aoRequest, clsResponsePut& aoResponse );
    int DoRequest( clsRequestGet& aoRequest, clsResponseGet& aoResponse );
    int DoRequest( clsRequestPutOW& aoRequest, clsResponsePutOW& aoResponse );
    int DoRequest( clsRequestGetND& aoRequest, clsResponseGetND& aoResponse );
    int DoRequest( clsRequestUpdate& aoRequest, clsResponseUpdate& aoResponse );
    int DoRequest( clsRequestDelTimeOut& aoRequest, clsResponseDelTimeOut& aoResponse );
    int DoRequest( clsRequestStat& aoRequest, clsResponseStat& aoResponse );

    //send response operator
    void SendResponse( const AIBC::ASYNCMATCH *apoMatch, int aiCmd, clsBody* apoBody );
    
//attr member
protected:
    AITask* cpoThreadPool;
    
    //cache object
    ICache* cpoCache;
    
    //config member
    int ciShutDownTimeout;
    int ciListenPort;
    int ciPrintStatInterval;
    int ciTimeoutInterval;
    int ciClearTimeoutInterval;
};

AI_CACHE_NAMESPACE_END

#endif //__AI_CACHESERVER_H__
