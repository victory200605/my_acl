
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
#include "../DevpollReactor.h"

#if defined(_SINGLE_PROCESS)
#   define __fork() 0
#else
#   define __fork() fork()
#endif

/////////////////////////////// global variable ///////////////////////////////////
bool gbShutdown = false;
bool gbIsSoftSetFileHandle = false;
int  giCurrProcessIdx = 0;
int  giConnBackLog    = 1000;
int  giCommonTimeout  = 10;
int  giMaxFileHandle  = 1024;
int  giTranWinCount   = 1;
int  giUnitSize       = 1024;

class IOEventHandler;

clsMPShareState*  gpoState = NULL;
AIDevpollReactor* gpoDevpollReactor = NULL;
AIMutexLock*      gpoDevpollLock = NULL;
IOEventHandler*   gpoEventHandler = NULL;

AIChunkEx* gpoWriteBuffer = NULL;
AIChunkEx* gpoReadBuffer = NULL;

///////////////////////////////////////////////////////////////////////////////////
class IOEventHandler : public AIEventHandler
{
public:
    virtual int HandleInput( int aiHandle )
    {
        AIChunkEx& loReadBuffer = gpoReadBuffer[aiHandle];
        
        int liRecvCount  = loReadBuffer.GetSize() / giUnitSize;
        int liRemainSize = loReadBuffer.GetCapacity() - loReadBuffer.GetSize();
        
        int liRetCode = recv( aiHandle, loReadBuffer.WritePtr(), liRemainSize, 0 );
        //printf( "Handle = %d, revc size = %d, remain=%d\n", aiHandle, liRetCode, liRemainSize );
        if ( liRetCode == 0 )
        {
            gpoState->Decrease( (*gpoState)[giCurrProcessIdx].ciCurrConnCnt );
            
            gpoDevpollReactor->RemoveHandler( aiHandle, AIEventHandler::ALL_EVENTS_MASK );
                
            ai_socket_close(aiHandle);
        }
        else if ( liRetCode < 0 )
        {
            gpoState->Increase( (*gpoState)[giCurrProcessIdx].ciTotalErrRevCnt );
            gpoState->Decrease( (*gpoState)[giCurrProcessIdx].ciCurrConnCnt );
            
            gpoDevpollReactor->RemoveHandler( aiHandle, AIEventHandler::ALL_EVENTS_MASK );
            
            ai_socket_close(aiHandle);
        }
        else
        {
            loReadBuffer.WritePtr(liRetCode);
            loReadBuffer.Resize( loReadBuffer.GetSize() + liRetCode );
            
            liRecvCount  = loReadBuffer.GetSize() / giUnitSize - liRecvCount;
            
            (*gpoState)[giCurrProcessIdx].ciTotalRevCnt += liRecvCount;
            
            if ( loReadBuffer.GetSize() >= loReadBuffer.GetCapacity() )
            {
                gpoDevpollReactor->RemoveHandler( aiHandle, AIEventHandler::READ_MASK );
            }
            
            gpoDevpollReactor->RegisterHandler( aiHandle, gpoEventHandler, AIEventHandler::WRITE_MASK );
            
            //HandleOutput( aiHandle );
        }
        
        return 0;
    }
    
    virtual int HandleOutput( int aiHandle )
    {
        AIChunkEx& loReadBuffer = gpoReadBuffer[aiHandle];
        AIChunkEx& loWriteBuffer = gpoWriteBuffer[aiHandle];
        
        size_t liRemainSize = loWriteBuffer.GetCapacity() - loWriteBuffer.GetSize();
            
        liRemainSize = (loReadBuffer.GetSize() < liRemainSize ? loReadBuffer.GetSize() : liRemainSize );
        if ( liRemainSize > 0 )
        {
            if ( loReadBuffer.GetSize() >= loReadBuffer.GetCapacity() )
            {
                gpoDevpollReactor->RegisterHandler( aiHandle, gpoEventHandler, AIEventHandler::READ_MASK );
            }
            
            ::memcpy( loWriteBuffer.WritePtr(), loReadBuffer.BasePtr(), liRemainSize );
                
            ///reset read buffer
            ::memmove(
                loReadBuffer.BasePtr(), 
                loReadBuffer.BasePtr() + liRemainSize,
                loReadBuffer.GetSize() - liRemainSize );
            loReadBuffer.Reset();
            loReadBuffer.Resize( loReadBuffer.GetSize() - liRemainSize );
            loReadBuffer.WritePtr( loReadBuffer.GetSize() );
            
            ///reset write buffer
            loWriteBuffer.WritePtr( liRemainSize );
            loWriteBuffer.Resize( loWriteBuffer.GetSize() + liRemainSize );
        }

        int liRetCode = send( aiHandle, loWriteBuffer.BasePtr(), loWriteBuffer.GetSize(), 0 );
        //printf( "Handle = %d,send size = %d\n", aiHandle, liRetCode );
        if ( liRetCode < 0 )
        {
            gpoState->Increase( (*gpoState)[giCurrProcessIdx].ciTotalErrSendCnt );
            
            gpoDevpollReactor->RemoveHandler( aiHandle, AIEventHandler::ALL_EVENTS_MASK );
                
            ai_socket_close(aiHandle);
        }
        else
        {
            ///reset read buffer
            ::memmove( 
                loWriteBuffer.BasePtr(), 
                loWriteBuffer.BasePtr() + liRetCode,
                loWriteBuffer.GetSize() - liRetCode );
            loWriteBuffer.Reset();
            loWriteBuffer.Resize( loWriteBuffer.GetSize() - liRetCode );
            loWriteBuffer.WritePtr( loWriteBuffer.GetSize() );
            
            if ( loWriteBuffer.GetSize() <= 0 )
            {
                gpoDevpollReactor->RemoveHandler( aiHandle, AIEventHandler::WRITE_MASK );
            }
        }
        
        return 0;
    }
    
    virtual int HandleClose( int aiHandle, int aiMask )
    {
        return 0;
    }
};

////////////////////////////////////////////////////////////////////////
void ChildMain( const char* apcServerIp, int aiPort, int aiThreadCnt, int aiFrequency, clsMPShareState* apoState, AIProtocol* apoProtocol )
{
    AITimeMeter loMeter;
    struct rlimit loLimit;
    loLimit.rlim_cur = giMaxFileHandle;
    loLimit.rlim_max = giMaxFileHandle;
    
    if ( gbIsSoftSetFileHandle && setrlimit( RLIMIT_NOFILE, &loLimit ) != 0 )
    {
        printf( "setrlimit fail\n" );
        return;
    }
    
    gpoDevpollLock    = new AIMutexLock;
    gpoDevpollReactor = new AIDevpollReactor;
    gpoEventHandler   = new IOEventHandler;
    gpoWriteBuffer    = new AIChunkEx[giMaxFileHandle];
    gpoReadBuffer     = new AIChunkEx[giMaxFileHandle];
    gpoState          = apoState;
    
    for ( int liN = 0; liN < giMaxFileHandle; liN++ )
    {
        gpoReadBuffer[liN].Resize(giTranWinCount*giUnitSize);
        gpoWriteBuffer[liN].Resize(giTranWinCount*giUnitSize);
        
        gpoReadBuffer[liN].Resize(0);
        //gpoWriteBuffer[liN].Resize(0);
    }
    
    assert( gpoDevpollReactor->Open( giMaxFileHandle ) == 0 );
    
    int liSocket = 0;
    int liConnCnt = aiThreadCnt;
    struct sockaddr_in  server_addr;
    int liOptValue = 0;
    while( liConnCnt > 0 )
    {
        liOptValue = 0;
        if ( ai_set_sockaddr_in(&server_addr, apcServerIp, aiPort) < 0 ) goto ERROR;
        
        liOptValue++;
        if ( (liSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) goto ERROR;
        
        liOptValue++;
        if ( (connect(liSocket, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0 )
        {
            goto ERROR;
        }
        
        int liFlags = fcntl(liSocket, F_GETFL, 0);
        fcntl(liSocket, F_SETFL, liFlags | O_NONBLOCK);
        
        apoState->Increase( (*apoState)[giCurrProcessIdx].ciCurrConnCnt );
        
        gpoDevpollReactor->RegisterHandler( 
            liSocket, gpoEventHandler, AIEventHandler::WRITE_MASK | AIEventHandler::READ_MASK );
        
        liConnCnt--;
    }

    while( !gbShutdown )
    {
        //sleep(1);
        loMeter.Reset();
        gpoDevpollReactor->HandleEvent( 1 );
        loMeter.Snapshot();
        if ( loMeter.GetTime() > 10 )
        {
            printf( "Loop use time = %lf\n", loMeter.GetTime() );
        }
    }
    
    return;

ERROR:
    printf( "Error: Opt = %d, MSG=%s\n", liOptValue, strerror(errno) );
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
    loArgument.AddOption( 'z', 0, "softset", "softset file descriptor to maxfilehandle" );
    loArgument.AddOption( 'm', 1, "maxfilehandle", "file descriptor" );
    loArgument.AddOption( 'w', 1, "tranwin", "tranfer window count" );
    loArgument.AddOption( 'u', 1, "unit", "tranfer window unit size" );
    loArgument.AddOption( 'v', 1, "protocol", "protocol exp:[cmpp|defualt]" );
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
    giMaxFileHandle = loArgument.GetIntValue('m', 1024);
    giTranWinCount  = loArgument.GetIntValue('w', 1);
    giUnitSize      = loArgument.GetIntValue('u', 1024);
    
    if ( loArgument.IsHasOpt( 'z' ) )
    {
        gbIsSoftSetFileHandle = true;
    }
    
    if ( strcmp( lsProtocol, "cmpp" ) == 0 )
    {
        lpoProtocol = new AICmppProtocol;
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
