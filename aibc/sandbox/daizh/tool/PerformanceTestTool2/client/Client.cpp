
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "AILogSys.h"
#include "AISocket.h"
#include "AISynch.h"
#include "AITime.h"
#include "../State.h"
#include "../Protocol.h"
#include "../AILog.h"
#include "../Frequency.h"

#if defined(_SINGLE_PROCESS)
#   define __fork() 0
#else
#   define __fork() fork()
#endif

bool gbShutdown = false;
int  giCurrProcessIdx = 0;
int  giCommonTimeout  = 10;
int  giTranWinCount   = 1;
int  giUnitSize       = 1024;

struct stConnParam
{ 
    char         csServerIp[20];
    int          ciPort;
    clsFrequency coFrequency;
    clsMPShareState* cpoState;
    AIProtocol*  cpoProtocol;
};

void* ConnProcessThread( void* apParam )
{
    pthread_detach(pthread_self());
    
    stConnParam* lpoParam = (stConnParam*)apParam;
    clsMPShareState& loState = *lpoParam->cpoState;
    AIChannel*   lpoChannel = lpoParam->cpoProtocol->CreateChannel();
    int  liRetCode = 0;
    int  liSocket  = 0;
    
    AILOG_DEBUG( "Process [%d] Thread [%d] entry", giCurrProcessIdx, pthread_self() );
    
    while( !gbShutdown )
    {
        if ( liSocket <= 0 )
        {
            AILOG_DEBUG( "Process [%d] Thread [%d] start connect to ...", giCurrProcessIdx, pthread_self() );
            if ( ( liSocket = ai_socket_connect( lpoParam->csServerIp, lpoParam->ciPort, 10 ) ) <= 0 )
            {
                AILOG_ERROR( "Process [%d] Thread [%d] connect fail, MSGCODE:%d", giCurrProcessIdx, pthread_self(), liSocket );
                if ( liSocket < 0 )
                {
                    loState.Increase( loState[giCurrProcessIdx].ciTotalErrConnCnt );
                }
                sleep(1);
                continue;
            }
            AILOG_DEBUG( "Process [%d] Thread [%d] connect success", giCurrProcessIdx, pthread_self() );
            if ( (liRetCode=lpoParam->cpoProtocol->SendConnectRequest( liSocket, lpoChannel )) != 0 )
            {
                AILOG_ERROR( "Process [%d] Thread [%d] connect request fail, MSGCODE:%d", giCurrProcessIdx, pthread_self(), liRetCode );
                ai_socket_close(liSocket);
                loState.Increase( loState[giCurrProcessIdx].ciTotalErrConnCnt );
                sleep(1);
                continue;
            }
            AILOG_DEBUG( "Process [%d] Thread [%d] connect request success", giCurrProcessIdx, pthread_self() );
            loState.Increase( loState[giCurrProcessIdx].ciCurrConnCnt );
        }

        if ( lpoParam->coFrequency.GetFrequency() == 0 )
        {
            sleep(1);
            continue;
        }
        else
        {
            lpoParam->coFrequency.Get();
        }

        if ( (liRetCode=lpoChannel->SendRequest( liSocket, giCommonTimeout )) == 0 )
        {
            loState.Increase( loState[giCurrProcessIdx].ciTotalSendCnt );
            loState.Increase( loState[giCurrProcessIdx].ciTotalRevCnt );
            loState.Increase( loState[giCurrProcessIdx].ciTotalRequestCnt );
        
            continue;
        }
        
        if ( liRetCode == AIProtocol::ERROR_SEND )
        {
            ai_socket_close(liSocket);
            
            loState.Increase( loState[giCurrProcessIdx].ciTotalErrSendCnt );
            loState.Decrease( loState[giCurrProcessIdx].ciCurrConnCnt );
            AILOG_ERROR( "Process [%d] Thread [%d] send fail, MSGCODE:%d", giCurrProcessIdx, pthread_self(), liRetCode );
        }
        else if ( liRetCode == AIProtocol::ERROR_RECV )
        {
            ai_socket_close(liSocket);
            
            loState.Decrease( loState[giCurrProcessIdx].ciCurrConnCnt );
            loState.Increase( loState[giCurrProcessIdx].ciTotalErrRevCnt );
            AILOG_ERROR( "Process [%d] Thread [%d] recv fail, MSGCODE:%d", giCurrProcessIdx, pthread_self(), liRetCode );
        }
        else if ( liRetCode == AIProtocol::ERROR_REQUEST )
        {
            loState.Increase( loState[giCurrProcessIdx].ciTotalErrRequestCnt );
            AILOG_ERROR( "Process [%d] Thread [%d] request fail, MSGCODE:%d", giCurrProcessIdx, pthread_self(), liRetCode );
        }
    }
    
    lpoParam->cpoProtocol->ReleaseChannel(lpoChannel);
    delete lpoParam;
    
    return NULL;
}

////////////////////////////////////////////////////////////////////////
void ChildMain( const char* apcServerIp, int aiPort, int aiThreadCnt, int aiFrequency, clsMPShareState* apoState, AIProtocol* apoProtocol )
{
    for( int liN = 0; liN < aiThreadCnt; liN++ )
    {
        stConnParam* lpConnParam = new stConnParam;
        strncpy( lpConnParam->csServerIp, apcServerIp, sizeof(lpConnParam->csServerIp) );
        lpConnParam->coFrequency.Set(aiFrequency);
        lpConnParam->ciPort      = aiPort;
        lpConnParam->cpoState    = apoState;
        lpConnParam->cpoProtocol = apoProtocol;
        pthread_t liThrd;
        assert( pthread_create( &liThrd, NULL, ConnProcessThread, lpConnParam ) == 0 );
    }
    
    while( !gbShutdown )
    {
        sleep(1);
    }
}

void ShutDown( int aiSignal )
{
    gbShutdown = true;
    signal(aiSignal, SIG_IGN);
}

void KillChild( pid_t apiPid[], int aiCount )
{
    for ( int liN = 0; liN < aiCount; liN++ )
    {
        kill( apiPid[liN], SIGTERM );
    }
}
     
int main( int argc, char* argv[] )
{
    AIInitIniHandler();
    AIInitLOGHandler();
    
    int  liChildCnt = 1;
    char lsIPAddr[15] = "127.0.0.1";
    char lsProtocol[15] = "cmpp";
    int  liPort = 6500;
    int  liInterval;
    int  liFrequency;
    int  liThreadCnt;
    int  liLogLevel;
    AIProtocol* lpoProtocol = NULL;
    pid_t liPid[100];

    signal(SIGINT, ShutDown);
    signal(SIGTERM, ShutDown);
    signal(SIGPIPE, SIG_IGN);
        
    AIArgument loArgument(argc, argv);
    loArgument.AddOption( 's', 1, "serverip", "server ip" );
    loArgument.AddOption( 'p', 1, "serverport", "server port" );
    loArgument.AddOption( 'c', 1, "child", "child count" );
    loArgument.AddOption( 't', 1, "thread", "thread count" );
    loArgument.AddOption( 'l', 1, "log", "log level" );
    loArgument.AddOption( 'i', 1, "interval", "print state time interval" );
    loArgument.AddOption( 'f', 1, "frequency", "request frequency" );
    loArgument.AddOption( 'w', 1, "tranwin", "tranfer window count" );
    loArgument.AddOption( 'u', 1, "unit", "tranfer window unit size" );
    loArgument.AddOption( 'v', 1, "protocol", "protocol exp:[cmpp|cmppex|defualt]" );
    loArgument.AddOption( 'o', 1, "timeout", "common time out" );
    loArgument.AddOption( 'h', 0, "help", "help" );
    
    //TODO Parser command argument
    if ( loArgument.Parser() != 0 || loArgument.IsHasOpt( 'h' ) )
    {
        /// Parser argument exception or do option -v
        loArgument.PrintUsage();
        return 0;
    }
    
    loArgument.GetStrValue( 's', lsIPAddr, sizeof(lsIPAddr), "127.0.0.1" );
    loArgument.GetStrValue( 'v', lsProtocol, sizeof(lsProtocol), "defualt" );
    liPort      = loArgument.GetIntValue('p', 6500);
    liChildCnt  = loArgument.GetIntValue('c', 1);
    liInterval  = loArgument.GetIntValue('i', 5);
    liThreadCnt = loArgument.GetIntValue('t', 1);
    liFrequency = loArgument.GetIntValue('f', -1);
    liLogLevel  = loArgument.GetIntValue('l', 1);
    giCommonTimeout = loArgument.GetIntValue('o', 10);
    giTranWinCount  = loArgument.GetIntValue('w', 1);
    giUnitSize      = loArgument.GetIntValue('u', 1024);
    
    if ( strcmp( lsProtocol, "cmpp" ) == 0 )
    {
        lpoProtocol = new AICmppProtocol;
    }
    else if ( strcmp( lsProtocol, "cmppex" ) == 0 )
    {
        lpoProtocol = new AICmppProtocolEx;
    }
    else
    {
        lpoProtocol = new AIDefualtProtocol(giTranWinCount, giUnitSize);
    }
    
    ///log
    AILog::Instance()->SetLevel( liLogLevel );
    
    if ( lpoProtocol->Initialize() != 0 )
    {
        printf( "ERROR : protocol initialize fail\n" );
        return -1;
    }
    
    {
        clsMPShareState loShareState;
        
        if ( loShareState.CreateShareState(liChildCnt) != 0 )
        {
            printf( "ERROR : Create share state fail\n" );
            return -1;
        }
    }
    
    for( int liN = 0; liN < liChildCnt; liN++ )
    {
        if( ( liPid[liN] = __fork() ) == 0 )
        {
            giCurrProcessIdx = liN;
            clsMPShareState loShareState;
            if ( loShareState.OpenShareState() != 0 )
            {
                printf( "ERROR : Open share state fail, proc exit\n" );
                return -1;
            }
            
            ::ChildMain( lsIPAddr, liPort, liThreadCnt, liFrequency, &loShareState, lpoProtocol );
            
            printf( "Exit proc [%d] now ...\n", giCurrProcessIdx );
            
            return 0;
        }
        else if ( liPid[liN] < 0 )
        {
            printf( "fork process fail - error %s\n", strerror(errno) );
            liChildCnt = liN;
            gbShutdown = true;
            break;
        }
        printf( "Start proc [%d] success ...\n", liN );
    }
    
    while( !gbShutdown )
    {
        PrintState( liChildCnt );
        sleep(liInterval);
    }
    
    KillChild( liPid, liChildCnt );
    
    printf( "Exit now ...\n" );
    
    return 0;
}
