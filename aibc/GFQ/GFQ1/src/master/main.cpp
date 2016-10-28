#define _TRACE
#include "MasterServer.h"
#include <time.h>

using namespace gfq;

clsMasterServer* gpoMasterServer = NULL;

//////////////////////////////////////////////////////////////////////////////////////
// global reload func
int ReloadFunc( void )
{
    AIChunkEx                       loChunk;
    clsRequestReloadQueueInfo       loRequest;
    clsResponseReloadQueueInfo      loResponse;
    
    loRequest.SetUserName( "Admin" );
    if ( gpoMasterServer != NULL )
    {
        gpoMasterServer->DoRequest( loRequest, loResponse );
    }
    
    return 0;
}

#if defined(_AI_LIB_EX_OLD)
    #define ADVFQUEUE_CONFIG "../config/Master.ini"

    int MyMainInit( int argc, char *argv[], int nLogLevel, int nPID )
    {
        AIConfig *lpoIni = NULL;
    	if ((lpoIni = AIGetIniHandler( ADVFQUEUE_CONFIG )) == NULL)
    	{
    		AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "Fail to load ini %s", ADVFQUEUE_CONFIG);
    		printf( "Fail to load ini %s", ADVFQUEUE_CONFIG);
    		return 1;
    	}
    	
    	int liListenPort = lpoIni->GetIniInt("Master", "ListenPort", 0);
    	
        AI_NEW_INIT_ASSERT( gpoMasterServer, clsMasterServer, liListenPort );
        
        if ( gpoMasterServer->Initialize( ADVFQUEUE_CONFIG ) != 0 )
        {
            printf( "Fail to Initialize Master server!\n" );
            return 1;
        }
        
        if ( gpoMasterServer->Startup() != 0)
    	{
    		fprintf(stderr, "Fail to startup Master server\n");
    		return 1;
    	}
        
        fprintf(stderr, "Master server exit......\n");
        
    	return 0;
    }
    
    void SafeExitFun(int aiSignal)
    {
        if ( gpoMasterServer != NULL )
    	{
    	    gpoMasterServer->Shutdown();
    	}
    }
    
    HANDLE_MAIN_MACRO(MyMainInit, NULL, SafeExitFun, NULL, NULL)
#else
    #define ADVFQUEUE_CONFIG "../../config/Master.ini"

    int ChildMain( int argc, char *argv[], pid_t atParentPID )
    {
        AI_NEW_INIT_ASSERT( gpoMasterServer, clsMasterServer, 0 );
    
        if ( gpoMasterServer->Initialize( ADVFQUEUE_CONFIG ) != 0 )
        {
            printf( "Fail to Initialize Master server!\n" );
            return 1;
        }
        
        // Set queue info reload function
        SetReLoad( ReloadFunc );
        
        if ( gpoMasterServer->Startup() != 0)
    	{
    		fprintf(stderr, "Fail to startup Master server\n");
    		return 1;
    	}
        
        fprintf(stderr, "Master server exit......\n");
        
	    return 0;
    }
    HANDLE_MAIN_MACRO( ChildMain, "|MasterServerPort|" )
#endif

#if defined(AI_OS_AIX)
    //Make compiler happy on AIX env
#   include "AIProcBase.h"
    ProgOptParam moProgOptArray[1];
#endif

