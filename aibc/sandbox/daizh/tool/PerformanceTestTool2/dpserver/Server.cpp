
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
#include "../DevpollReactor.h"

#if defined(_SINGLE_PROCESS)
#   define __fork() 0
#else
#   define __fork() fork()
#endif

class IOEventHandler;

/////////////////////////////// global variable ///////////////////////////////////
bool gbShutdown = false;
bool gbIsSoftSetFileHandle = false;
int  giCurrProcessIdx = 0;
int  giConnBackLog    = 1000;
int  giCommonTimeout  = 10;
int  giSuspendHandle  = -1;
int  giMaxFileHandle  = 1024;
int  giTranWinCount   = 1;
int  giUnitSize       = 1024;

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
            AILOG_DEBUG( "Process [%d] Thread [%d] Decrease connection %d... ", 
                giCurrProcessIdx, pthread_self(), (*gpoState)[giCurrProcessIdx].ciCurrConnCnt );
            (*gpoState)[giCurrProcessIdx].ciCurrConnCnt--;
            if ( giSuspendHandle > 0 )
            {
                AILOG_DEBUG( "Process [%d] Thread [%d] ResumeHandler %d... ", 
                    giCurrProcessIdx, pthread_self(), giSuspendHandle );
                gpoDevpollReactor->ResumeHandler(giSuspendHandle);
                giSuspendHandle = -1;
            }
            AILOG_DEBUG( "Process [%d] Thread [%d] Decrease connection complete ... ", 
                    giCurrProcessIdx, pthread_self() );
            
            gpoDevpollReactor->RemoveHandler( aiHandle, AIEventHandler::ALL_EVENTS_MASK );
            ai_socket_close(aiHandle);
            
            return -1;
        }
        else if ( liRetCode < 0 )
        {
            (*gpoState)[giCurrProcessIdx].ciTotalErrRevCnt++;
            AILOG_ERROR( "Process [%d] Thread [%d] recv error, retcode=%d, errno=%d, msg=%s",
                giCurrProcessIdx, pthread_self(), liRetCode, errno, strerror(errno) );
            
            (*gpoState)[giCurrProcessIdx].ciCurrConnCnt--;
            if ( giSuspendHandle > 0 )
            {
                AILOG_DEBUG( "Process [%d] Thread [%d] ResumeHandler %d... ", 
                    giCurrProcessIdx, pthread_self(), giSuspendHandle );
                gpoDevpollReactor->ResumeHandler(giSuspendHandle);
                giSuspendHandle = -1;
            }
            
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
            (*gpoState)[giCurrProcessIdx].ciTotalErrSendCnt++;
            return -1;
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

class AcceptEventHandler : public AIEventHandler
{
public:
    AcceptEventHandler()
        : coFileLock("server_conn")
    {
    }
    virtual ~AcceptEventHandler()
    {
    }
    
    virtual int HandleInput( int aiHandle )
    {
        int  liClientSocket = 0;
        struct sockaddr_in  fromAddrIn;
        socklen_t           liSockAddrLen;
        
        liSockAddrLen = sizeof(fromAddrIn);
        memset(&fromAddrIn, 0, liSockAddrLen);

        coFileLock.Lock();
        AILOG_DEBUG( "Process [%d] Thread [%d] start accept connection ... ", giCurrProcessIdx, pthread_self() );
        int liResult = ai_socket_wait_events( aiHandle, POLLIN, NULL, 0 );
        if ( liResult > 0 )
        {
            liClientSocket = accept(aiHandle, (struct sockaddr *)&fromAddrIn, &liSockAddrLen);
            if (!IS_VALID_SOCKET(liClientSocket))
            {
                if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    return 1;
                }
                AILOG_ERROR( "Process [%d] Thread [%d] accept socket fail, MSG=%s", giCurrProcessIdx, pthread_self(), strerror(errno) );
                return 0;
            }
            else
            {
                int liFlags = fcntl(liClientSocket, F_GETFL, 0);
                fcntl(liClientSocket, F_SETFL, liFlags | O_NONBLOCK);
        
                gpoDevpollReactor->RegisterHandler( liClientSocket, gpoEventHandler, AIEventHandler::READ_MASK );
                
                (*gpoState)[giCurrProcessIdx].ciCurrConnCnt++;
                
                if ( (*gpoState)[giCurrProcessIdx].ciCurrConnCnt > giMaxFileHandle )
                {
                    gpoDevpollReactor->SuspendHandler( aiHandle );
                    giSuspendHandle = aiHandle;
                }
            }
        }
        AILOG_DEBUG( "Process [%d] Thread [%d] accept connection success", giCurrProcessIdx, pthread_self() );
        coFileLock.Unlock();
        
        return liResult;
    }
    
    virtual int HandleClose( int aiHandle, int aiMask )
    {
        ai_socket_close(aiHandle);        
        return 0;
    }

protected:    
    AIFileLock coFileLock;
};

///////////////////////////////////////////////////////////////////////////////////
int StartServer( const char* apcIp, int aiPort )
{
    return ::ai_socket_listen( apcIp, aiPort, giConnBackLog );
}

void ChildMain( int aiSocket, clsMPShareState* apoState, int aiFrequency, AIProtocol* apoProtocol )
{
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
        gpoWriteBuffer[liN].Resize(0);
    }
    
    assert( gpoDevpollReactor->Open( giMaxFileHandle ) == 0 );
    
    AcceptEventHandler loAcceptEventHandler;
    gpoDevpollReactor->RegisterHandler( aiSocket, &loAcceptEventHandler, AIEventHandler::READ_MASK );
    
    AITimeMeter loMeter;
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
    loArgument.AddOption( 'm', 1, "maxfilehandle", "file descriptor" );
    loArgument.AddOption( 'z', 0, "softset", "softset file descriptor to maxfilehandle" );
    loArgument.AddOption( 'f', 1, "frequency", "request frequency" );
    loArgument.AddOption( 'w', 1, "tranwin", "tranfer window count" );
    loArgument.AddOption( 'u', 1, "unit", "tranfer window unit size" );
    loArgument.AddOption( 'x', 1, "backlog", "connection backlog" );
    loArgument.AddOption( 'v', 1, "protocol", "protocol exp:[cmpp|cmpp_r]" );
    loArgument.AddOption( 'h', 0, "help", "help" );
    
    //TODO Parser command argument
    if ( loArgument.Parser() != 0 || loArgument.IsHasOpt( 'h' ) )
    {
        /// Parser argument exception or do option -v
        loArgument.PrintUsage();
        return 0;
    }
    
    loArgument.GetStrValue( 's', lsIPAddr, sizeof(lsIPAddr), "127.0.0.1" );
    loArgument.GetStrValue( 'v', lsProtocol, sizeof(lsProtocol), "cmpp" );
    liPort      = loArgument.GetIntValue('p', 6500);
    liChildCnt  = loArgument.GetIntValue('c', 1);
    liInterval  = loArgument.GetIntValue('i', 5);
    liLogLevel  = loArgument.GetIntValue('l', 1);
    liFrequency = loArgument.GetIntValue('f', -1);
    giConnBackLog = loArgument.GetIntValue('x', 1000);
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
        printf( "ERROR : invalid protocol\n" );
        return -1;
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
