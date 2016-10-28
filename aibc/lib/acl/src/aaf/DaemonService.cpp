
#include "acl/FileStream.h"
#include "acl/DateTime.h"
#include "acl/Singleton.h"
#include "aaf/DaemonService.h"

AAF_NAMESPACE_START

CDaemonService::CDaemonService(void)
    : miArgc(0)
    , mppcArgv(NULL)
    , mbIsParseArgFail(false)
    , miLogLevel(4)
    , mbIsShutdown(false)
{
    apl_signal( APL_SIGINT,  HandleSignalShutdown );
    apl_signal( APL_SIGTERM, HandleSignalShutdown );
    apl_signal( APL_SIGPIPE, APL_SIG_IGN );
    apl_signal( APL_SIGHUP,  APL_SIG_IGN );
    
    this->macProcessName[0] = '\0';
    
    this->SetOption('m', true, "level");
    this->SetOption('d', false, "daemon");
    this->SetOption('s', false, "stop");
    this->SetOption('k', false, "kill");
}
    
CDaemonService::~CDaemonService(void)
{
    if (!this->IsHasOption('d') )
    {
        this->Finalize();
    }
}


apl_int_t CDaemonService::Startup( char const* apcConfigFileName )
{
    if (this->mppcArgv == NULL)
    {
        apl_errprintf("DaemonService startup fail, please set arg first\n");
        return -1;
    }
        
    acl::CTimestamp loTimestamp;
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

    if (this->mbIsParseArgFail)
    {
        /// Parser argument exception or do option -v
        this->PrintUsage();
        apl_exit(0);
    }

    if (this->IsHasOption('s') )
    {
        this->KillAll(this->macProcessName, APL_SIGTERM);
        apl_exit(0);
    }

    if (this->IsHasOption('k') )
    {
        this->KillAll(this->macProcessName, APL_SIGKILL);
        apl_exit(0);
    }

    if (this->IsHasOption('m') )
    {
        this->miLogLevel = apl_strtoi32(this->GetOptArg('m'), NULL, 10);
    }
    
    if (this->IsHasOption('d') )
    {
        lbIsDaemon = true;
    }

    if (this->KillAll(this->macProcessName, 0) > 0)
    {
        apl_errprintf("Process is Alive, cann't restart now\n");
        return -1;
    }
    
    this->UpdateShutdown(false);
    
    if (this->moLog.Open("monitor.log") != 0 )
    {
        apl_errprintf("Open monitor log fail, [errno=%"APL_PRIdINT"]\n", apl_get_errno() );
        return -1;
    }

    if (lbIsDaemon)
    {
        this->RunByDaemon();
        
        this->moLog.Write(
            acl::LOG_LVL_INFO,
            "Parent %"APL_PRId64" running ...",
            (apl_int64_t)acl::CProcess::GetPid() );
        
        loTimestamp.Update();

        if (this->Initialize() != 0)
        {
            apl_errprintf("Hook initialize fail, [errno=%"APL_PRIdINT"]\n", apl_get_errno() );
            return -1;
        }
        
        while( !IsShutdown() )
        {
            acl::CProcessOption loOption;
            apl_int_t liExitCode = 0;
            
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
            
            while(true)
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
        
        this->Finalize();
        
        this->moLog.Write(
            acl::LOG_LVL_INFO,
            "Parent %"APL_PRId64" exiting ...",
            (apl_int64_t)acl::CProcess::GetPid() );
        
        this->ClearProcessID(this->macProcessName);
        
        apl_exit(0);
    }
    else
    {
        if (this->Initialize() != 0)
        {
            apl_errprintf("Hook initialize fail, [errno=%"APL_PRIdINT"]\n", apl_get_errno() );
            return -1;
        }
    }

    return 0;
}
    
apl_int_t CDaemonService::Restart(void)
{
    return -1;
}
    
apl_int_t CDaemonService::Shutdown(void)
{
    this->UpdateShutdown(true);
    return 0;
}

void CDaemonService::MakeOptionString( std::string& aoOptString )
{
    for (UdfOptionMapType::iterator loIter = this->moUdfOptions.begin();
         loIter != this->moUdfOptions.end(); ++loIter)
    {
        aoOptString += (char)loIter->first;
        if (loIter->second.mbIsHasValue)
        {
            aoOptString += ':';
        }
    }
}

void CDaemonService::SetLongOption( acl::CGetOpt& aoOpt )
{
    for (UdfOptionMapType::iterator loIter = this->moUdfOptions.begin();
         loIter != this->moUdfOptions.end(); ++loIter)
    {
        aoOpt.LongOption(
            loIter->second.moLongOption.c_str(), 
            loIter->first, 
            (loIter->second.mbIsHasValue ? acl::CGetOpt::ARG_REQUIRED : acl::CGetOpt::NO_ARG ) );
    }
}

bool CDaemonService::CheckAppendOption( apl_int_t aiOpt, char* apcOptArg )
{
    UdfOptionMapType::iterator loIter = this->moUdfOptions.find(aiOpt);
    if (loIter != this->moUdfOptions.end() )
    {
        if (loIter->second.mbIsHasValue)
        {
            loIter->second.moValue = apcOptArg;
        }

        loIter->second.mbIsHited = true;

        return true;
    }
    else
    {
        return false;
    }
}

void CDaemonService::PrintUsage(void)
{
    /// Parser argument exception or do option -v
    apl_errprintf( "Usage : %s\n", this->macProcessName );
    apl_errprintf( " %-10s -%c <val>: %s\n", "--level", 'm', "log level" );
    apl_errprintf( " %-10s -%c      : %s\n", "--daemon", 'd', "run by daemon" );
    apl_errprintf( " %-10s -%c      : %s\n", "--stop", 's', "stop all running process" );
    apl_errprintf( " %-10s -%c      : %s\n", "--kill", 'k', "kill running process" );

    for (UdfOptionMapType::iterator loIter = this->moUdfOptions.begin();
         loIter != this->moUdfOptions.end(); ++loIter)
    {
        switch(loIter->first)
        {
            case 'm':
            case 's':
            case 'd':
            case 'k': continue;
        };

        std::string loLongOption;
        std::string loValue;

        if (loIter->second.moLongOption.length() > 0)
        {
            loLongOption = "--" + loIter->second.moLongOption;
        }

        if (loIter->second.mbIsHasValue)
        {
            loValue = "<val>";
        }
        
        apl_errprintf( " %-10s -%c %-5s: %s\n", loLongOption.c_str(), (char)loIter->first, loValue.c_str(), loIter->second.moDesc.c_str() );
    }
}

bool CDaemonService::SetOption( char acOpt, bool abIsHasValue, char const* apcLongOpt, char const* apcDesc )
{
    if (this->moUdfOptions.find(acOpt) != this->moUdfOptions.end() )
    {
        return false;
    }
    
    CUdfOption& loOpt = this->moUdfOptions[acOpt];
    loOpt.mbIsHasValue = abIsHasValue;
    loOpt.mbIsHited = false;
    loOpt.moValue = "";
    loOpt.moLongOption = apcLongOpt == NULL ? "" : apcLongOpt;
    loOpt.moDesc = apcDesc == NULL ? "" : apcDesc;

    return true;
}

bool CDaemonService::IsHasOption( char acOpt )
{
    UdfOptionMapType::iterator loIter = this->moUdfOptions.find(acOpt);
    if (loIter != this->moUdfOptions.end() && loIter->second.mbIsHited )
    {
        return true;
    }
    else
    {
        return false;
    }
}

char const* CDaemonService::GetOptArg( char acOpt )
{
    UdfOptionMapType::iterator loIter = this->moUdfOptions.find(acOpt);
    if (loIter != this->moUdfOptions.end() && loIter->second.mbIsHited )
    {
        return loIter->second.moValue.c_str();
    }
    else
    {
        return NULL;
    }

}

char const* CDaemonService::GetProcessName(void)
{
    return this->macProcessName;
}

apl_int_t CDaemonService::GetLogLevel(void)
{
    return this->miLogLevel;
}

void CDaemonService::UpdateShutdown( bool abIsShutdown )
{
    this->mbIsShutdown = abIsShutdown;
}

bool CDaemonService::IsShutdown(void)
{
    return this->mbIsShutdown;
}
    
void CDaemonService::SetArg( apl_int_t argc, char* argv[] )
{
    this->miArgc = argc;
    this->mppcArgv = argv;
    this->mbIsParseArgFail = false;
    
    this->MakeOptionString(this->moOptString);
    acl::CGetOpt loOpt(this->miArgc, this->mppcArgv, this->moOptString.c_str(), 1, true, acl::CGetOpt::RETURN_IN_ORDER );
    apl_int_t    liOptVal = 0;

    this->SetLongOption(loOpt);

    while( (liOptVal = loOpt()) != -1 )
    {
        if (liOptVal == 1)
        {
            break;
        }

        if (!this->CheckAppendOption(liOptVal, loOpt.OptArg() ) )
        {
            this->mbIsParseArgFail = true;
        }
    }
}

apl_ssize_t CDaemonService::KillAll( apl_int_t aiSignal )
{
    return this->KillAll(this->macProcessName, aiSignal);
}

void CDaemonService::RunByDaemon(void)
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

void CDaemonService::SaveProcessID( const char* apcProcessName, apl_int_t aiFlag )
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

void CDaemonService::ClearProcessID( const char* apcProcessName )
{
    acl::CFile loFile;
    char       lacBuffer[1024] = {0};
    
    apl_snprintf(lacBuffer, sizeof(lacBuffer), "%s.pid", apcProcessName);
    
    loFile.Open(lacBuffer, APL_O_RDWR|APL_O_TRUNC);
    loFile.Close();
}

apl_ssize_t CDaemonService::KillAll( const char* apcProcessName, apl_int_t aiSignal )
{
    char lacBuffer[1024] = {0};
    apl_ssize_t liResult = 0;
    acl::CFileStream loFile;
    
    apl_snprintf(lacBuffer, sizeof(lacBuffer), "%s.pid", apcProcessName);
    
    if ( loFile.Open(lacBuffer, APL_O_RDONLY) == 0)
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

void CDaemonService::HandleSignalShutdown( int aiSignal )
{
    apl_signal( aiSignal, HandleSignalShutdown );

    acl::Instance<CDaemonService>()->UpdateShutdown( true );
}

apl_int_t CDaemonService::Initialize(void)
{
    return 0;
}

void CDaemonService::Finalize(void)
{
}

AAF_NAMESPACE_END
