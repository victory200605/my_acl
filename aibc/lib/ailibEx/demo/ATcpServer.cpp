#include "AIAsyncTcp.h"
#include "AISocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


int                 giCount = 0;
int                 giShutDown = 0;
pthread_mutex_t     gtLock = PTHREAD_MUTEX_INITIALIZER;
clsAsyncServer      *gpoServer = NULL;


int RecvCallback(const ASYNCSOCK *apoSocket, char *apcData, const int aiSize)
{
    pthread_mutex_lock(&gtLock);
        giCount += 1;
    pthread_mutex_unlock(&gtLock);

    fprintf(stderr, "%s\r\n", apcData);
    
    // return  (ERROR_ASYNC_LOCALBUSY);
    return  (0);
}

void RecvCallbackEx(const ASYNCMATCH *apoMatch, char *apcData, const int aiSize)
{
    pthread_mutex_lock(&gtLock);
        giCount += 1;
    pthread_mutex_unlock(&gtLock);

    fprintf(stderr, "%s\r\n", apcData);
    gpoServer->SendResult(apoMatch, apcData, aiSize, 0);
}

void ShutDown(int aiSignal)
{
    giShutDown = 1;
    signal(aiSignal, SIG_IGN);
}

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, ShutDown);
    signal(SIGTERM, ShutDown);
    
    gpoServer = new clsAsyncServer(NULL, 3333);

    // gpoServer->SetCallback(RecvCallback);
    gpoServer->SetCallbackEx(RecvCallbackEx);

    gpoServer->StartDaemon();
    for(int liIt = 0; !giShutDown && liIt < 30; liIt++)
    {
        gpoServer->ListConnection();
        sleep(2);
    }
    
    fprintf(stderr, "ShutDown 3 ... %ld\r\n", time(NULL));
    gpoServer->ShutDown();
    fprintf(stderr, "ShutDown 4 ... %ld\r\n", time(NULL));
    
    delete gpoServer;
    gpoServer = NULL;
}

