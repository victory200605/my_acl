#include "cacheserver/CacheUtility.h"
#include "CacheServer.h"

using namespace cacheserver;

int __Main( int argc, char *argv[], pid_t atParentPID )
{
    char lsConfigFile[AI_CACHE_NAMESPACE::AI_MAX_NAME_LEN] = {0};
    ::snprintf( lsConfigFile, sizeof(lsConfigFile),
        "../../config/%s.ini", AIM::GetProcessName() );
    if ( clsCacheServer::Instance()->Startup(lsConfigFile) != 0 )
	{ 
		fprintf(stderr, "Fail to startup server\n");
        exit(0);
		return 1;
	}

	fprintf(stderr, "Server startup success\n");

	while ( !AIM::IsShutdown() )
    {
        AIBC::AISleepFor( AI_TIME_SEC );
     	clsCacheServer::Instance()->PrintStat();
    } 
    
    clsCacheServer::Instance()->Shutdown();
    
    fprintf(stderr, "Server exit......\n");
    
    return 0;
}

HANDLE_MAIN_MACRO( __Main, "|CacheServerPort|" )

#if defined(AI_OS_AIX)
    //Make compiler happy on AIX env
#   include "AIProcBase.h"
    ProgOptParam moProgOptArray[1];
#endif

