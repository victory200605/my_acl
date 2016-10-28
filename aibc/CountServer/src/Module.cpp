
#include "Module.h"
#include "acl/File.h"
#include "acl/DateTime.h"

CModuleEnv* CModuleEnv::Instance(void)
{
    static CModuleEnv* slpoInstance = NULL;
    
    if (slpoInstance == NULL)
    {
        ACL_NEW_ASSERT(slpoInstance, CModuleEnv);
    }
    
    return slpoInstance;
}

void CModuleEnv::UpdateShutdown( bool abIsShutdown )
{
    this->mbIsShutdown = abIsShutdown;
}

bool CModuleEnv::IsShutdown(void)
{
    return this->mbIsShutdown;
}

//////////////////////////////////////////////////////////////////////////////////////
CModule::CModule(void)
    : miLogLevel(5)
    , miArgc(0)
    , mppcArgv(0)
{
    apl_signal( APL_SIGINT,  HandleSignalShutdown );
    apl_signal( APL_SIGTERM, HandleSignalShutdown );
    apl_signal( APL_SIGPIPE, APL_SIG_IGN );
    apl_signal( APL_SIGHUP,  APL_SIG_IGN );
    
    apl_memset( this->macProcessName, 0, sizeof(this->macProcessName) );
}
    
CModule::~CModule(void)
{
}
    
apl_int_t CModule::Run( apl_int_t argc, char* argv[] )
{
    acl::CGetOpt loOpt(argc, argv, "m:dsk", 0);
    acl::CTimestamp loTimestamp;
    apl_int_t    liOptVal = 0;
    bool         lbIsDaemon = false;
    
    this->miArgc = argc;
    this->mppcArgv = argv;
    
    apl_strncpy(this->macProcessName, this->mppcArgv[0], sizeof(this->macProcessName) );
    
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
                return 0;
            }
            case 'k':
            {
                this->KillAll(this->macProcessName, APL_SIGKILL);
                return 0;
            }
            default:
            {
                /// Parser argument exception or do option -v
                apl_errprintf( "Usage : %s\n", this->mppcArgv[0] );
                apl_errprintf( " --level  -m : log level\n" );
                apl_errprintf( " --daemon -d : run by daemon\n" );
                apl_errprintf( " --stop   -s : stop all running process\n" );
                apl_errprintf( " --kill   -k : kill running process\n" );
                
                return 0;
            }
        };
    }
    
    if (this->KillAll(this->macProcessName, 0) > 0)
    {
        apl_errprintf("Process is Alive, cann't restart now\n");
        return -1;
    }
    
    UpdateShutdown(false);
    
    if (this->moLog.Open("monitor.log") != 0 )
    {
        apl_errprintf( "Open monitor log fail, [errno=%"APL_PRIdINT"]\n", apl_get_errno() );
        return -1;
    }
    
    this->moLog.Write(acl::LOG_LVL_INFO, "Parent %"APL_PRId64" running ...", (apl_int64_t)acl::CProcess::GetPid() );
    
    if (lbIsDaemon)
    {
        this->RunByDaemon();
        
        while( !IsShutdown() )
        {
            acl::CProcessOption loOption;
            apl_int_t           liExitCode = 0;
            
            this->SaveProcessID(this->macProcessName, APL_O_CREAT|APL_O_RDWR|APL_O_TRUNC);
            
            loTimestamp.Update();
            if ( this->Spawn(loOption) != 0 )
            {
                this->moLog.Write(acl::LOG_LVL_ERROR, "Spawn process fail, [errno=%"APL_PRIdINT"]\n", apl_get_errno() );
                    
                apl_sleep(APL_TIME_SEC);
                continue;
            }
            
            while( !IsShutdown() )
            {
                this->Wait(&liExitCode, acl::CTimeValue(0) );
                if ( this->Kill(0) != 0 )
                {
                    this->moLog.Write(acl::LOG_LVL_INFO, "Child process exited, [exit code=%"APL_PRIdINT"]\n", liExitCode );
                    
                    acl::CTimestamp loDead;
                    loDead.Update();
                    if (loDead.Sec() - loTimestamp.Sec() < 30)
                    {
                        UpdateShutdown(true);
                    }
                    
                    break;
                }
                
                apl_sleep(APL_TIME_SEC);
            }
        }
        
    }
    else
    {
        this->Main( argc, argv );
    }
    
    this->moLog.Write(acl::LOG_LVL_INFO, "Parent %"APL_PRId64" exiting ...", (apl_int64_t)acl::CProcess::GetPid() );
    
    return 0;
}

void CModule::RunByDaemon(void)
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

void CModule::SaveProcessID( const char* apcProcessName, apl_int_t aiFlag )
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

apl_ssize_t CModule::KillAll( const char* apcProcessName, apl_int_t aiSignal )
{
    char lacBuffer[1024] = {0};
    apl_ssize_t liResult = 0;
    
    apl_snprintf(lacBuffer, sizeof(lacBuffer), "%s.pid", apcProcessName);
    
    FILE* lpoFile = ::fopen( lacBuffer, "r" );
    if ( lpoFile != NULL )
    {
        while( fgets( lacBuffer, sizeof(lacBuffer), lpoFile ) )
        {
            if ( apl_strtoi32(lacBuffer, NULL, 10) > 0 )
            {
                if ( apl_kill( apl_strtoi32(lacBuffer, NULL, 10), aiSignal ) == 0 )
                {
                    liResult++;
                }
            }
        }
        
        ::fclose( lpoFile );
    }
    
    return liResult;
}

void CModule::HandleSignalShutdown( int aiSignal )
{
    apl_signal( aiSignal, HandleSignalShutdown );

    UpdateShutdown( true );
}
    
void CModule::Child(void)
{
    this->SaveProcessID(this->macProcessName, APL_O_RDWR | APL_O_APPEND);
    
    this->moLog.Write(acl::LOG_LVL_INFO, "Child %"APL_PRId64" running ...", (apl_int64_t)acl::CProcess::GetPid() );
    this->Main(this->miArgc, this->mppcArgv);
    this->moLog.Write(acl::LOG_LVL_INFO, "Child %"APL_PRId64" eixting ...", (apl_int64_t)acl::CProcess::GetPid() );
    apl_exit( apl_get_errno() );
}

char const* CModule::GetProcessName(void)
{
    return this->macProcessName;
}

apl_int_t CModule::GetLogLevel(void)
{
    return this->miLogLevel;
}

///////////////////////////////////////////////////////////////////////////////////////////
void UpdateShutdown( bool abIsShutdown )
{
    CModuleEnv::Instance()->UpdateShutdown(abIsShutdown);
}

bool IsShutdown(void)
{
    return CModuleEnv::Instance()->IsShutdown();
}
