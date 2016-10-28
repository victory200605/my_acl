#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "AIAdvFileQueue.h"

int AddRecord( clsAdvFileQueue *apoQueue )
{
    int                 liRetCode;
    char                lsMesg[900];

    memset( lsMesg, 'A', sizeof(lsMesg) );
    for( int liIt = 0; liIt < 200000; liIt++ )
    {
        sprintf( lsMesg+580, "Message %d", liIt );
        if( (liRetCode = apoQueue->AddRecord( "XCZ", lsMesg, 600 )) )
        {
            fprintf( stderr, "%s:  AddRecord ... Fail = %d\r\n", \
                __FUNCTION__, liRetCode );
            break;
        }
    }
    
    return  (0);
}

int DelRecord( clsAdvFileQueue *apoQueue )
{
    int                 liCount;
    int                 liRetCode;
    int                 liRetSize;
    int                 liGlobalID;
    char                lsMesg[900];

    liCount = 0;
    liRetSize = (int)sizeof(lsMesg);
        
    while( !(liRetCode = apoQueue->GetRecord( "XCZ", liGlobalID, lsMesg, liRetSize )) )
    {
        if( (liRetCode = apoQueue->RemoveRecord( liGlobalID )) )
        {
            fprintf( stderr, "%s:  RemoveRecord( %d ) ... Fail = %d\r\n", \
                __FUNCTION__, liGlobalID, liRetCode );
        }
            
        liCount += 1;
        liRetSize = (int)sizeof(lsMesg);
            
        if( liCount % 1000 == 0 )
        {
            fprintf( stderr, "%s\r\n", lsMesg );
        }
    }

    fprintf( stderr, "%s:  GetRecord ... Fail = %d\r\n", \
        __FUNCTION__, liRetCode );
    return  (0);
}

int main( int argc, char *argv[] )
{
    int                 liRetCode;
    clsAdvFileQueue     loFileQueue;

    fprintf( stderr, "TIME1:  %ld\r\n", time(NULL) );   
    if( (liRetCode = loFileQueue.LoadFileQueue( "XCZ.DAT", 800, 600000 )) )
    {
        fprintf( stderr, "%s:  LoadFileQueue ... Fail = %d\r\n", \
            __FUNCTION__, liRetCode );
        return  (0);
    }
    
    fprintf( stderr, "TIME2:  %ld\r\n", time(NULL) );   
    if( argc > 1 && strcmp( argv[1], "add" ) == 0 )
    {
        AddRecord( &loFileQueue );
    }
        
    if( argc > 1 && strcmp( argv[1], "del" ) == 0 )
    {
        DelRecord( &loFileQueue );
    }

    if( argc > 1 && strcmp( argv[1], "loop" ) == 0 )
    {
        DelRecord( &loFileQueue );
        
        for( int liIt = 0; liIt < 100; liIt++ )
        {
            AddRecord( &loFileQueue );
            DelRecord( &loFileQueue );
        }   
    }
    
    fprintf( stderr, "TIME3:  %ld\r\n", time(NULL) );   
    loFileQueue.ShutDown();

    fprintf( stderr, "TIME4:  %ld\r\n", time(NULL) );   
    return  (0);
}
