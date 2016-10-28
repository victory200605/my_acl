#define _TRACE
#include "SegmentServer.h"
#include <time.h>

using namespace gfq;

#define SEGMENT_SERVER_CONFIG "../../config/SegmentServer.ini"
#define MASTER_CLIENT_CONFIG "../../config/GFQClient.ini"

int ChildMain( int argc, char *argv[], pid_t atParentPID )
{
    clsSegmentServer loSegmentServer(0);
    
    if ( loSegmentServer.Initialize( SEGMENT_SERVER_CONFIG, MASTER_CLIENT_CONFIG ) != 0 )
    {
        printf( "Fail to Initialize Segment Server!\n" );
        return 1;
    }
    
    pthread_t liThrd;
    if ( pthread_create(&liThrd, NULL, TimeTraceThreadFunc, argv[0]) != 0 )
    {
        printf( "Fail to start timetrace Server!\n" );
        return 1;
    }
    
    if ( loSegmentServer.Startup() != 0)
    {
        fprintf(stderr, "Fail to startup Segment Server\n");
        return 1;
    }
    
    fprintf(stderr, "Segment Server exit......\n");
    
    return 0;
}
    
HANDLE_MAIN_MACRO( ChildMain, "|SegmentServerPort|" )

#if defined(AI_OS_AIX)
    //Make compiler happy on AIX env
#   include "AIProcBase.h"
    ProgOptParam moProgOptArray[1];
#endif

