
#include "acl/FileStream.h"
#include "acl/DateTime.h"
#include "acl/Singleton.h"
#include "aaf/DaemonServer.h"

AAF_NAMESPACE_START

CDaemonServer::CDaemonServer(void)
    : miArgc(0)
    , mppcArgv(NULL)
    , miLogLevel(4)
    , mbIsShutdown(false)
{
    apl_signal( APL_SIGINT,  HandleSignalShutdown );
    apl_signal( APL_SIGTERM, HandleSignalShutdown );
    apl_signal( APL_SIGPIPE, APL_SIG_IGN );
    apl_signal( APL_SIGHUP,  APL_SIG_IGN );
    
    this->macProcessName[0] = '\0';
}
    
CDaemonServer::~CDaemonServer(void)
{
}

apl_int_t CDaemonServer::Startup( char const* apcConfigFileName )
{
    if (this->mppcArgv == NULL)
    {
        apl_errprintf("DaemonServer startup fail, please set arg first\n");
        return -1;
    }
        
    acl::CGetOpt loOpt(this->miArgc, this->mppcArgv, "m:dsk", 0);
    acl::CTimestamp loTimestamp;
    apl_int_t    liOptVal = 0;
    bool         lbIsDaemon = false;
    
    char const* lpcProcessName = apl_strrchr(this->mppcArgv[0], '/');
    if (lpcProcessName == NULL)
    {
        apl_strncpy(this->macProcessName, this->mppcArgv[0], sizeof(this->macProcessName) );
    }
    else
    {
        apl_strncpy(this->macProcessName, ++lpcProcessName, sizeof(this->macProcessName) );
    }
    
    loOpt.LongOption("level",  'm', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("daemon", 'd');
    loOpt.LongOption("stop",   's');
    loOpt.LongOption("kill",   'k');

    while( (liOptVal = loOpt()) != -1 )
    {
        switch(liOptVal)
        {
            case 'm':
            {
                this->miLogLevel = apl_strtoi32(loOpt.OptArg(), NULL, 10);
                break;
            }
            case 'd':
            {
                lbIsDaemon = true;
                break;
            }
            case 's':
            {
                this->KillAll(this->macProcessName, APL_SIGTERM);
                apl_exit(0);
            }
            case 'k':
            {
                this->KillAll(this->macProcessName, APL_SIGKILL);
                apl_exit(0);
            }
            default:
            {
                /// Parser argument exception or do option -v
                apl_errprintf( "Usage : %s\n", this->mppcArgv[0] );
                apl_errprintf( " --level  -m : log level\n" );
                apl_errprintf( " --daemon -d : run by daemon\n" );
                apl_errprintf( " --stop   -s : stop all running process\n" );
                apl_errprintf( " --kill   -k : kill running process\n" );
                
                apl_exit(0);
            }
        };
    }
    
    if (this->KillAll(this->macProcessName, 0) > 0)
    {
        apl_errprintf("Process is Alive, cann't restart now\n");
        return -1;
    }
    
    this->UpdateShutdown(false);
    
    if (this->moLog.Open("monitor.log") != 0 )
    {
        apl_errprintf( "Open monitor log fail, [errno=%"APL_PRIdINT"]\n", apl_get_errno() );
        return -1;
    }

    if (lbIsDaemon)
    {
        this->moLog.Write(
            acl::LOG_LVL_INFO,
            "Parent %"APL_PRId64" running ...",
            (apl_int64_t)acl::CProcess::GetPid() );
            
        this->RunByDaemon();
        
        loTimestamp.Update();
        
        while( !IsShutdown() )
        {
            acl::CProcessOption loOption;
            apl_int_t           liExitCode = 0;
            
            this->SaveProcessID(this->macProcessName, APL_O_CREAT|APL_O_RDWR|APL_O_TRUNC);

            if ( this->Spawn(loOption) != 0 )
            {
                this->moLog.Write(
                    acl::LOG_LVL_ERROR,
                    "Spawn process fail, [errno=%"APL_PRIdINT"]\n",
                    apl_get_errno() );
                    
                apl_sleep(APL_TIME_SEC);
                continue;
            }
            
            if (this->GetChildID() == 0)
            {
                //Child
                this->SaveProcessID(this->macProcessName, APL_O_RDWR | APL_O_APPEND);
                
                this->moLog.Write(
                    acl::LOG_LVL_INFO,
                    "Child %"APL_PRId64" running ...",
                    (apl_int64_t)acl::CProcess::GetPid() );
                        
                return 0;
            }
            
            while( !IsShutdown() )
            {
                this->Wait(&liExitCode, acl::CTimeValue(0) );
                if ( this->Kill(0) != 0 )
                {
                    this->moLog.Write(
                        acl::LOG_LVL_INFO,
                        "Child process exited, [exit code=%"APL_PRIdINT"]\n",
                        liExitCode );
                    
                    acl::CTimestamp loDead;
                    loDead.Update();
                    if (loDead.Sec() - loTimestamp.Sec() < 30)
                    {
                        this->UpdateShutdown(true);
                    }
                    
                    break;
                }
                
                apl_sleep(APL_TIME_SEC);
            }
        }
        
        this->moLog.Write(
            acl::LOG_LVL_INFO,
            "Parent %"APL_PRId64" exiting ...",
            (apl_int64_t)acl::CProcess::GetPid() );
        
        this->ClearProcessID(this->macProcessName);
        
        apl_exit(0);
    }

    return 0;
}
    
apl_int_t CDaemonServer::Restart(void)
{
    return -1;
}
    
apl_int_t CDaemonServer::Shutdown(void)
{
    this->UpdateShutdown(true);
    return 0;
}

char const* CDaemonServer::GetProcessName(void)
{
    return this->macProcessName;
}

apl_int_t CDaemonServer::GetLogLevel(void)
{
    return this->miLogLevel;
}

void CDaemonServer::UpdateShutdown( bool abIsShutdown )
{
    this->mbIsShutdown = abIsShutdown;
}

bool CDaemonServer::IsShutdown(void)
{
    return this->mbIsShutdown;
}
    
void CDaemonServer::SetArg( apl_int_t argc, char* argv[] )
{
    this->miArgc = argc;
    this->mppcArgv = argv;
}

apl_ssize_t CDaemonServer::KillAll( apl_int_t aiSignal )
{
    return this->KillAll(this->macProcessName, aiSignal);
}

void CDaemonServer::RunByDaemon(void)
{
    switch( apl_fork() )
    {
        case 0:
        {
            break;
        }
        
        default: apl_exit(0);
    }
}

void CDaemonServer::SaveProcessID( const char* apcProcessName, apl_int_t aiFlag )
{
    acl::CFile loFile;
    char       lacBuffer[1024] = {0};
    
    apl_snprintf(lacBuffer, sizeof(lacBuffer), "%s.pid", apcProcessName);
    
    if ( loFile.Open(lacBuffer, aiFlag, 0755) != 0 )
    {
        apl_errprintf("Open file %s.pid fail - %s\n", apcProcessName, apl_strerror(apl_get_errno() ) );
    }
    else
    {
        acl::CDateTime loDateTime;
        
        loDateTime.Update();
        
        apl_snprintf(lacBuffer, sizeof(lacBuffer), "%"PRId64" %s\n", (apl_int64_t)this->GetPid(), loDateTime.Format() );
        
        if ( loFile.Write( lacBuffer, apl_strlen(lacBuffer) ) != (apl_ssize_t)apl_strlen(lacBuffer) )
        {
            apl_errprintf("Write file %s.pid fail - %s\n", apcProcessName, apl_strerror(apl_get_errno() ) );
        }
        
        loFile.Close();
    }
}

void CDaemonServer::ClearProcessID( const char* apcProcessName )
{
    acl::CFile loFile;
    char       lacBuffer[1024] = {0};
    
    apl_snprintf(lacBuffer, sizeof(lacBuffer), "%s.pid", apcProcessName);
    
    loFile.Open(lacBuffer, APL_O_RDWR|APL_O_TRUNC);
    loFile.Close();
}

apl_ssize_t CDaemonServer::KillAll( const char* apcProcessName, apl_int_t aiSignal )
{
    char lacBuffer[1024] = {0};
    apl_ssize_t liResult = 0;
    acl::CFileStream loFile;
    
    apl_snprintf(lacBuffer, sizeof(lacBuffer), "%s.pid", apcProcessName);
    
    if (loFile.Open(lacBuffer, APL_O_RDONLY) == 0)
    {
        while(loFile.ReadLine(lacBuffer, sizeof(lacBuffer) ) > 0)
        {
            if ( apl_strtoi32(lacBuffer, NULL, 10) > 0 )
            {
                if ( apl_kill( apl_strtoi32(lacBuffer, NULL, 10), aiSignal ) == 0 )
                {
                    liResult++;
                }
            }
        }
        
        loFile.Close();
    }
    
    return liResult;
}

void CDaemonServer::HandleSignalShutdown( int aiSignal )
{
    apl_signal( aiSignal, HandleSignalShutdown );

    acl::Instance<CDaemonServer>()->UpdateShutdown( true );
}

AAF_NAMESPACE_END
