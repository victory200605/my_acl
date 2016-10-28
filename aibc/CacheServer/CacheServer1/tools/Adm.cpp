
#include "cacheserver/CacheClient.h"
#include "../src/CacheServer/BillCache/BillCache.h"

int main( int argc, char* argv[] )
{
    AIBC::AIInitLOGHandler();
    AIBC::AIInitIniHandler();
    
    int liOptChar;
    int  liOpValue = -1;
    int  liCount = 0;
    int  liExportMode = 2;
    int  liPrintCount = 0;
        
    size_t liSize = 0;
    char lcDirPath[128];
	while( (liOptChar = getopt(argc, argv, "cn:s:d:rhe:p:" )) != EOF )
	{
	    switch( liOptChar )
	    {
	        case 'c':
	            liOpValue = 0;
	            break;
	        case 'r':
	            liOpValue  = 1;
	            break;
	        case 'n': 
	            liCount = atoi(optarg);
	            break;
	        case 's':
	            liSize = atoi(optarg);
	            break;
	        case 'd':
	            strcpy( lcDirPath, optarg );
	            break;
	        case 'p':
	            liPrintCount = atoi(optarg);
	            break;
	        case 'e':
	            liOpValue = 2;
	            if ( ::strcmp(optarg, "server") == 0 )
	            {
	                liExportMode = 1;
	            }
	            else if ( ::strcmp(optarg, "test") == 0 )
	            {
	                liExportMode = 2;
	            }
	            else
	            {
	                printf( "Invalid param %s\n", optarg );
	                return -1;
	            }
	            break;
	        case 'h':
	            printf( "-c create bill cache db file\n" );
	            printf( "-n <file count> for create bill cache store file\n" );
	            printf( "-s <file node count> for create bill cache store file\n" );
	            printf( "-d <directory> bill cache db file directory\n" );
	            printf( "-r <reset> reset(clean) bill cache db file\n" );
	            printf( "-e [server|test] export bill cache db record\n" );
	            return 0;
	    }
	}
	int liRet = 0;
	time_t liStart;
	cacheserver::clsBillCache loFileCache;
    
    switch( liOpValue )
    {
        case 0:
        {//create bill cache
	    
    	    printf( "Are you sure create db[Capacity=%d]/[Path=%s], "
    	            "existed db will be recover\nPress any key to coutinue and CTRL+C to exit", liCount * liSize, lcDirPath );
    	    getchar();
    	    
    	    time(&liStart);
    	    if ( ( liRet = loFileCache.Create( lcDirPath, liCount, liSize) ) != cacheserver::AI_NO_ERROR )
            {
                printf( "Create Cache Index fail - %s\n", cacheserver::StrError( liRet ) );
                return -1;
            }
            printf( "Create use time %lds\n", time(NULL) - liStart );
            return 0;
	    }
	    case 1:
    	{
    	    printf( "Are you sure reset db[Path=%s], all record will be erased\nPress any key to coutinue and CTRL+C to exit", lcDirPath );
    	    getchar();
    	    
    	    time(&liStart);
    	    if ( ( liRet = loFileCache.Open( lcDirPath ) ) != cacheserver::AI_NO_ERROR )
            {
                printf( "Open Cache Index fail - %s\n", cacheserver::StrError( liRet ) );
                return -1;
            }
            
            if ( ( liRet = loFileCache.Clean( ) ) != cacheserver::AI_NO_ERROR )
            {
                printf( "Clean Cache Index fail - %s\n", cacheserver::StrError( liRet ) );
                return -1;
            }
            printf( "Reset use time %lds\n", time(NULL) - liStart );
            return 0;
    	}
    	case 2:
    	{
    	    time(&liStart);
    	    if ( ( liRet = loFileCache.Open( lcDirPath ) ) != cacheserver::AI_NO_ERROR )
            {
                printf( "Open Cache Index fail - %s\n", cacheserver::StrError( liRet ) );
                return -1;
            }
            
            AIBC::AIChunkEx loKey;
            AIBC::AIChunkEx loValue;
            int             liTotalRecordCnt   = 0;
            int             liSuccessRecordCnt = 0;
            
            switch( liExportMode )
            {
                case 1:
                {
                    CacheClient loCacheClient;
                    
                    if ( loCacheClient.Init(NULL, 0, NULL, 0, "../config/CacheClient.ini") != 0 )
                	{
                	    printf( "Cache client initialize fail, may be connect rejected - %s\n", strerror(errno) );
                	    break;
                	}
                	
                	for ( cacheserver::TIterator loIter = loFileCache.Begin(); 
                        loIter != loFileCache.End(); ++loIter )
                    {
                        liTotalRecordCnt++;
                        
                        if ( ( liRet = loFileCache.Get( loIter, loKey, loValue ) ) != cacheserver::AI_NO_ERROR )
                        {
                            printf( "Get Cache record fail - %s\n", cacheserver::StrError( liRet ) );
                            break;
                        }
    
                        if ( ( liRet = loCacheClient.SaveData( loKey.BasePtr(), loValue.BasePtr(), loValue.GetSize() ) ) != 0 )
                        {
                            printf( "Insert data fail, MSGCODE:%d, [key=%s]\n", liRet, loKey.BasePtr() );
                            break;
                        }
                        
                        liSuccessRecordCnt++;
                    }

                    break;
                }
                case 2:
                {
                	for ( cacheserver::TIterator loIter = loFileCache.Begin(); 
                        loIter != loFileCache.End(); ++loIter )
                    {
                        liTotalRecordCnt++;
                        
                        if ( ( liRet = loFileCache.Get( loIter, loKey, loValue ) ) != cacheserver::AI_NO_ERROR )
                        {
                            printf( "Get Cache record fail - %s\n", cacheserver::StrError( liRet ) );
                            break;
                        }
                        
                        if ( liPrintCount-- > 0 )
                        {
                            printf( "Export data [Key=%s]/[Size=%d]/[Value=%s]\n", loKey.BasePtr(), loValue.GetSize(), loValue.BasePtr() );
                        }
                        liSuccessRecordCnt++;
                    }
                    break;
                }
            };
    
            printf( "Export data to complete, use timetime %lds\n", time(NULL) - liStart );
            printf( "=================================================================\n" );
            printf( "Total record count   = %d\n", liTotalRecordCnt );
            printf( "Success record count = %d\n", liSuccessRecordCnt );
            printf( "Reject record count  = %d\n", liTotalRecordCnt - liSuccessRecordCnt );
            printf( "Use time             = %lds\n", time(NULL) - liStart );
            printf( "=================================================================\n" );
            return 0;
	    }
    };
	
    return 0;
}

#if defined(AI_OS_AIX)
    //Make compiler happy on AIX env
#   include "AIProcBase.h"
    ProgOptParam moProgOptArray[1];
#endif

