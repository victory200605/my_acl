
#include "../src/segmentserver/SegmentStore.h"
#include "gfq/GFQueueApi.h"

using namespace gfq;

int main( int argc, char* argv[] )
{
    AIInitIniHandler();
    AIInitLOGHandler();
    
    int  liOptChar;
    int  liOpValue = 0;
    char liOptValue = 0;
    int liCount = 0;
    int liSegmentCnt = 0;
    size_t liSize = 0;
    char lcDirPath[128];
    memset( lcDirPath, 0, sizeof(lcDirPath) );
    while( (liOptChar = getopt(argc, argv, "cn:m:s:d:rlhp:" )) != EOF )
    {
        switch( liOptChar )
        {
            case 'c':
                liOpValue = 0;
                break;
            case 'm': 
                liSegmentCnt = atoi(optarg);
                break;
            case 'n': 
                liCount = atoi(optarg);
                break;
            case 's':
                liSize = atoi(optarg);
                break;
            case 'd':
                strcpy( lcDirPath, optarg );
                lcDirPath[strlen(lcDirPath)] = '/';
                break;
            case 'r':
                liOpValue = 1;
                break;
            case 'l':
                liOpValue = 2;
                break;
            case 'p':
                liOpValue = 3;
                liOptValue = *optarg;
                break;
            case 'h':
                printf( "-c create segment server db file\n" );
                printf( "-r reset(clean) segment server db file\n" );
                printf( "-l notify master reload queue info\n" );
                printf( "-p <q|m> notify master to print queue or module info, q:queue, m:module\n" );
                printf( "-n <file count> for create segment server store file\n" );
                printf( "-m <single file segment count> for create segment server store file\n" );
                printf( "-s <segment node count> for create segment server store file\n" );
                printf( "-d <directory> segment server db file directory\n" );
                return 0;
        }
    }
    int liRet = 0;
    time_t liStart;
    clsSegmentStore loSegmentStore(lcDirPath);

    switch( liOpValue )
    {
        case 0:
        {
            time(&liStart);
            if ( ( liRet = loSegmentStore.Create( lcDirPath, liCount, liSegmentCnt, liSize ) ) != AI_NO_ERROR )
            {
                printf( "Create segment server store db fail - %s\n", StrError( liRet ) );
                return -1;
            }
            printf( "Create use time %lds\n", time(NULL) - liStart );
            return 0;
        }
        case 1:
        {
            time(&liStart);
            if ( ( liRet = loSegmentStore.Clear( lcDirPath ) ) != AI_NO_ERROR )
            {
                printf( "Reset segment server store db fail - %s\n", StrError( liRet ) );
                return -1;
            }
            printf( "Reset use time %lds\n", time(NULL) - liStart );
            return 0;
        }
        case 2:
        {
            time(&liStart);
            clsGFQueueApi loApi;
            if ( loApi.Initialize( "../config/GFQClient.ini" ) != 0 )
        	{
        	    printf( "GFQ client initialize fail, may be connect rejected - %s\n", strerror(errno) );
        	    return -1;
        	}
        	
            if ( ( liRet = loApi.ReloadQueueInfo() ) != AI_NO_ERROR )
            {
                printf( "Reload queue info fail - %s\n", StrError( liRet ) );
                return -1;
            }
            printf( "Reload use time %lds\n", time(NULL) - liStart );
            return 0;
        }
        case 3:
        {
            time(&liStart);
            clsGFQueueApi loApi;
            if ( loApi.Initialize( "../config/GFQClient.ini" ) != 0 )
        	{
        	    printf( "GFQ client initialize fail, may be connect rejected - %s\n", strerror(errno) );
        	    return -1;
        	}
        	switch( liOptValue )
        	{
        	    case 'q':
        	    {
        	        if ( ( liRet = loApi.PrintQueues() ) != AI_NO_ERROR )
                    {
                        printf( "Print queue info fail - %s\n", StrError( liRet ) );
                        return -1;
                    }
                    break;
        	    }
        	    case 'm':
        	    {
        	        if ( ( liRet = loApi.PrintModules() ) != AI_NO_ERROR )
                    {
                        printf( "Print module info fail - %s\n", StrError( liRet ) );
                        return -1;
                    }
                    break;
        	    }
        	    default:
        	    {
        	        printf( "Print option [%c] invalid\n", liOptValue );
        	    }
        	}
            
            printf( "Print use time %lds\n", time(NULL) - liStart );
            return 0;
        }
    }
    
    AICloseLOGHandler();
    AICloseIniHandler();
    
    return 0;
}

#if defined(AI_OS_AIX)
    //Make compiler happy on AIX env
#   include "AIProcBase.h"
    ProgOptParam moProgOptArray[1];
#endif

