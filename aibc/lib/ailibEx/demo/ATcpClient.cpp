#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "AISocket.h"
#include "AIAsyncTcp.h"


int                 giCount = 0;
int                 giShutDown = 0;
pthread_mutex_t     gtLock = PTHREAD_MUTEX_INITIALIZER;
clsAsyncClient      goClient( "10.3.2.35", 3333, 5, 15 );

#define  MAX_TEST_SEND_COUNT    (80000)

void *SendThread( void *avOpt )
{
    char        lsBuffer[1024];
    
    pthread_detach( pthread_self() );
    memset( lsBuffer, 0, sizeof(lsBuffer) );
    
    while( !giShutDown )
    {
        pthread_mutex_lock( &gtLock );
        if( giCount++, giCount <= MAX_TEST_SEND_COUNT )
        {
            sprintf( lsBuffer, "giCount = %d, %ld", giCount, time(NULL) );
            pthread_mutex_unlock( &gtLock );
            
            if( goClient.SendRequest( lsBuffer, 600, NULL ) )
            {
                fprintf( stderr, "%ld: goClient.SendRequest ... Fail\r\n", (long)pthread_self() );

                if( giShutDown == 0 )
                {
                    sleep( 1 );
                    continue;
                }

                break;
            }
            
            if( goClient.RecvResponse( ) )
            {
                fprintf( stderr, "%ld: goClient.RecvResponse ... Fail\r\n", (long)pthread_self() );

                if( giShutDown == 0 )
                {
                    sleep( 1 );
                    continue;
                }

                break;
            }
            
            continue;
        }
        
        pthread_mutex_unlock( &gtLock );
        break;
    }   
    
    return (NULL);
}

void ShutDown( int aiSignal )
{
    giShutDown = 1;
    signal(aiSignal, SIG_IGN);
}

int main( int argc, char *argv[] )
{
    time_t      ltBeginTime;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, ShutDown);
    signal(SIGTERM, ShutDown);
    
    goClient.StartDaemon(1, 1);
    ltBeginTime = time(NULL);
    for( int liIt = 0; liIt < 50; liIt++ )
    {
        pthread_t   tid;
        pthread_create( &tid, NULL, SendThread, NULL );
    }   
    
    while( !giShutDown && giCount < MAX_TEST_SEND_COUNT )  sleep(1);
    fprintf( stderr, "= %ld Seconds\r\n", time(NULL)-ltBeginTime );

    fprintf( stderr, "ShutDown 3 ... %ld\r\n", time(NULL) );
    goClient.ShutDown();
    fprintf( stderr, "ShutDown 4 ... %ld\r\n", time(NULL) );
}

