
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "AILogSys.h"
#include "AISocket.h"
#include "AISynch.h"
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
int  giConnBackLog    = 1000;
int  giCommonTimeout  = 10;
int  giTranWinCount   = 1;
int  giUnitSize       = 1024;

struct stConnParam
{
    int          ciSocket;
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
    
    int        liRetCode = 0;

    AILOG_DEBUG( "Process [%d] Thread [%d] entry", giCurrProcessIdx, pthread_self() );
    
    loState.Increase( loState[giCurrProcessIdx].ciCurrConnCnt );
    
    do
    {
        if ( ( liRetCode = lpoParam->cpoProtocol->RecvConnectRequest(lpoParam->ciSocket, lpoChannel) ) != 0 )
        {
            ai_socket_close(lpoParam->ciSocket);
            
            loState.Increase( loState[giCurrProcessIdx].ciTotalErrConnCnt );
            AILOG_ERROR( "Process [%d] Thread [%d] recv connect request fail, MSGCODE:%d", giCurrProcessIdx, pthread_self(), liRetCode );
            break;
        }
        else
        {
            AILOG_DEBUG( "Process [%d] Thread [%d] recv connect request success", giCurrProcessIdx, pthread_self() );
        }
        
        while( !gbShutdown )
        {
            if ( lpoParam->coFrequency.GetFrequency() == 0 )
            {
                sleep(1);
                continue;
            }
            else
            {
                lpoParam->coFrequency.Get();
            }

TIMEOUT_AGAIN:
            liRetCode = lpoChannel->RecvRequest( lpoParam->ciSocket, giCommonTimeout );
            if( liRetCode == AIProtocol::ERROR_RECV )
            {
                ai_socket_close(lpoParam->ciSocket);
                
                loState.Increase( loState[giCurrProcessIdx].ciTotalErrRevCnt );
                AILOG_ERROR( "Process [%d] Thread [%d] recv fail, MSGCODE:%d", giCurrProcessIdx, pthread_self(), liRetCode );
    
                break;
            }
            else if( liRetCode == AIProtocol::ERROR_SEND )
            {
                ai_socket_close(lpoParam->ciSocket);
                
                loState.Increase( loState[giCurrProcessIdx].ciTotalErrSendCnt );
                AILOG_ERROR( "Process [%d] Thread [%d] send fail, MSGCODE:%d", giCurrProcessIdx, pthread_self(), liRetCode );
                break;
            }
            else if( liRetCode == AIProtocol::ERROR_REQUEST )
            {
                loState.Increase( loState[giCurrProcessIdx].ciTotalErrRequestCnt );
                AILOG_ERROR( "Process [%d] Thread [%d] request fail, MSGCODE:%d", giCurrProcessIdx, pthread_self(), liRetCode );
            }
            else if ( liRetCode == AIProtocol::ERROR_TIMEOUT )
            {
                goto TIMEOUT_AGAIN;
            }
            
            loState.Increase( loState[giCurrProcessIdx].ciTotalRevCnt );
            loState.Increase( loState[giCurrProcessIdx].ciTotalSendCnt );
            loState.Increase( loState[giCurrProcessIdx].ciTotalRequestCnt );
        }
    }
    while(false);
    
    loState.Decrease( loState[giCurrProcessIdx].ciCurrConnCnt );
    
    lpoParam->cpoProtocol->ReleaseChannel(lpoChannel);
    delete lpoParam;
    
    return NULL;
}

void* AcceptThread( void* apParam )
{
    pthread_detach(pthread_self());
    
    stConnParam* lpoParam = (stConnParam*)apParam;
    clsMPShareState* lpoState = lpoParam->cpoState;
        
    char lsClientIp[20];
    int  liPort;
    int  liClientSocket;

    pthread_t  liThrd;
    AIFileLock loFileLock("server_conn");
    while( !gbShutdown )
    {
        loFileLock.Lock();
        AILOG_DEBUG( "Process [%d] Thread [%d] start accept connection ... ", giCurrProcessIdx, pthread_self() );
        if ( ( liClientSocket = ai_socket_accept( lpoParam->ciSocket, lsClientIp, &liPort ) ) <= 0 )
        {
            AILOG_ERROR( "Process [%d] Thread [%d] accept socket fail, MSGCODE:%d", giCurrProcessIdx, pthread_self(), liClientSocket );
            continue;
        }
        AILOG_DEBUG( "Process [%d] Thread [%d] accept connection success", giCurrProcessIdx, pthread_self() );
        loFileLock.Unlock();
        stConnParam* lpoTmpParam = new stConnParam;
        lpoTmpParam->ciSocket    = liClientSocket;
        lpoTmpParam->cpoState    = lpoState;
        lpoTmpParam->cpoProtocol = lpoParam->cpoProtocol;
        lpoTmpParam->coFrequency.Set(lpoParam->coFrequency.GetFrequency());
        assert( pthread_create( &liThrd, NULL, ConnProcessThread, (void*)lpoTmpParam ) == 0 );
    }
    
    delete lpoParam;
    
    return NULL;
}

////////////////////////////////////////////////////////////////////////
int StartServer( const char* apcIp, int aiPort )
{
    return ::ai_socket_listen( apcIp, aiPort, giConnBackLog );
}

void ChildMain( int aiSocket, clsMPShareState* apoState, int aiFrequency, AIProtocol* apoProtocol )
{
    stConnParam* lpoParam = new stConnParam;
    lpoParam->ciSocket = aiSocket;
    lpoParam->cpoState = apoState;
    lpoParam->coFrequency.Set(aiFrequency);
    lpoParam->cpoProtocol = apoProtocol;
        
    pthread_t liThrd;
    assert( pthread_create( &liThrd, NULL, AcceptThread, lpoParam ) == 0 );
    
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
    int  liInterval = 5;
    int  liLogLevel;
    int  liFrequency;
    AIProtocol* lpoProtocol = NULL;
    pid_t liPid[100];

	signal(SIGINT, ShutDown);
	signal(SIGTERM, ShutDown);
	signal(SIGPIPE, SIG_IGN);
	
    AIArgument loArgument(argc, argv);
    loArgument.AddOption( 's', 1, "serverip", "server ip" );
    loArgument.AddOption( 'p', 1, "serverport", "server port" );
    loArgument.AddOption( 'c', 1, "child", "child count" );
    loArgument.AddOption( 'i', 1, "interval", "print state time interval" );
    loArgument.AddOption( 'l', 1, "log", "log level" );
    loArgument.AddOption( 'o', 1, "timeout", "common time out" );
    loArgument.AddOption( 'f', 1, "frequency", "request frequency" );
    loArgument.AddOption( 'w', 1, "tranwin", "tranfer window count" );
    loArgument.AddOption( 'u', 1, "unit", "tranfer window unit size" );
    loArgument.AddOption( 'x', 1, "backlog", "connection backlog" );
    loArgument.AddOption( 'v', 1, "protocol", "protocol exp:[cmpp|cmppex|default]" );
    loArgument.AddOption( 'h', 0, "help", "help" );
    
    //TODO Parser command argument
    if ( loArgument.Parser() != 0 || loArgument.IsHasOpt( 'h' ) )
    {
        /// Parser argument exception or do option -v
        loArgument.PrintUsage();
        return 0;
    }
    
    loArgument.GetStrValue( 's', lsIPAddr, sizeof(lsIPAddr), "127.0.0.1" );
    loArgument.GetStrValue( 'v', lsProtocol, sizeof(lsProtocol), "default" );
    liPort      = loArgument.GetIntValue('p', 6500);
    liChildCnt  = loArgument.GetIntValue('c', 1);
    liInterval  = loArgument.GetIntValue('i', 5);
    liLogLevel  = loArgument.GetIntValue('l', 1);
    liFrequency = loArgument.GetIntValue('f', -1);
    giConnBackLog = loArgument.GetIntValue('x', 1000);
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
    
    int liSocket = ::StartServer( lsIPAddr, liPort );
    if ( liSocket <= 0 )
    {
        printf( "Start server fail\n" );
        return -1;
    }

    if ( lpoProtocol == NULL )
    {
        lpoProtocol = new AICmppProtocol;
    }
    
    {
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

                ::ChildMain( liSocket, &loShareState, liFrequency, lpoProtocol );
                
                printf( "Exit proc [%d] now ...\n", giCurrProcessIdx );
                ai_socket_close( liSocket );
                
                return 0;
            }
        }
    }
    
    while( !gbShutdown )
    {
        PrintState( liChildCnt );
        sleep(liInterval);
    }
    
    KillChild( liPid, liChildCnt );
    
    printf( "Exit now ...\n" );
    ai_socket_close( liSocket );
    
    return 0;
}
