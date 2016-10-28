
#include "acl/Process.h"
#include "acl/SString.h"

ACL_NAMESPACE_START

///////////////////////////////////////////// ProcessOption ////////////////////////////////////
CProcessOption::CProcessOption(void)
    : mpcCommandLine(NULL)
    , mppcArgv(NULL)
    , muArgvIndex(0)
    , mpcEnvBuffer(NULL)
    , muEnvBufferIndex(0)
    , mppcEnv(NULL)
    , muEnvIndex(0)
    , mpcWorkingDir(NULL)
    , mpcProcessName(NULL)
    , miStdIn(ACL_INVALID_HANDLE)
    , miStdOut(ACL_INVALID_HANDLE)
    , miStdErr(ACL_INVALID_HANDLE)
{
    ACL_NEW_N_ASSERT(this->mpcCommandLine, char, MAX_CMDLINE_LEN);
    ACL_NEW_N_ASSERT(this->mppcArgv, char*, MAX_ARG_SIZE);
    
    apl_memset(this->mpcCommandLine, 0, MAX_CMDLINE_LEN);
    apl_memset(this->mppcArgv, 0, MAX_ARG_SIZE * sizeof(char*) );
}

CProcessOption::~CProcessOption(void)
{
    ACL_DELETE_N(this->mpcCommandLine);
    ACL_DELETE_N(this->mppcArgv);
    ACL_DELETE_N(this->mpcEnvBuffer);
    ACL_DELETE_N(this->mppcEnv);
    ACL_DELETE_N(this->mpcWorkingDir);
    ACL_DELETE_N(this->mpcProcessName);
}

apl_int_t CProcessOption::SetCmdLine( char const* apcFormat, ... )
{   
    va_list loArgp;
    va_start(loArgp, apcFormat);
    
    apl_vsnprintf(this->mpcCommandLine, MAX_CMDLINE_LEN, apcFormat, loArgp);
    
    va_end(loArgp);
    
    return 0;
}

char* const* CProcessOption::GetCmdLine(void) const
{
    return this->mppcArgv;
}

apl_int_t CProcessOption::SetProcName( char const* apcProcName )
{
    if (this->mpcProcessName == NULL)
    {
        ACL_NEW_N_ASSERT(this->mpcProcessName, char, APL_NAME_MAX);
    }
    
    apl_snprintf(this->mpcProcessName, APL_NAME_MAX, "%s", apcProcName );
    
    return 0;
}
    
char const* CProcessOption::GetProcName(void) const
{
    return this->mpcProcessName;
}

apl_int_t CProcessOption::SetHandle( apl_handle_t aiStdIn, apl_handle_t aiStdOut, apl_handle_t aiStdErr )
{
    if (aiStdIn == ACL_INVALID_HANDLE)
    {
        aiStdIn = APL_STDIN_FILENO;
    }
    if (aiStdOut == ACL_INVALID_HANDLE)
    {
        aiStdOut = APL_STDOUT_FILENO;
    }
    if (aiStdErr == ACL_INVALID_HANDLE)
    {
        aiStdErr = APL_STDERR_FILENO;
    }
    
    this->miStdIn  = apl_dup(aiStdIn);
    this->miStdOut = apl_dup(aiStdOut);
    this->miStdErr = apl_dup(aiStdErr);
    
    return 0;
}
    
apl_handle_t CProcessOption::GetStdIn(void) const
{
    return this->miStdIn;
}

apl_handle_t CProcessOption::GetStdOut(void) const
{
    return this->miStdOut;
}

apl_handle_t CProcessOption::GetStdErr(void) const
{
    return this->miStdErr;
}

void CProcessOption::PassHandle( apl_handle_t aiHandle )
{
    this->moHandleSet.push_back(aiHandle);
}

apl_int_t CProcessOption::SetEnv( char const* apcFormat, ... )
{
    if (this->mpcEnvBuffer == NULL)
    {
        ACL_NEW_N_ASSERT(this->mpcEnvBuffer, char, MAX_ENVBUFFER_LEN);
        ACL_NEW_N_ASSERT(this->mppcEnv, char*, MAX_ENV_SIZE);
        this->muEnvBufferIndex = 0;
        this->muEnvIndex = 0;
    }
    
    if (this->muEnvIndex >= MAX_ENV_SIZE 
        || this->muEnvBufferIndex >= MAX_ENVBUFFER_LEN)
    {
        return -1;
    }
    
    va_list loArgp;
    va_start(loArgp, apcFormat);
    
    apl_vsnprintf(
        this->mpcEnvBuffer + this->muEnvBufferIndex,
        MAX_ENVBUFFER_LEN - this->muEnvBufferIndex,
        apcFormat,
        loArgp);
    
    va_end(loArgp);

    this->mppcEnv[this->muEnvIndex++] = this->mpcEnvBuffer + this->muEnvBufferIndex;
    
    this->muEnvBufferIndex += strlen(this->mpcEnvBuffer + this->muEnvBufferIndex);
    
    this->mpcEnvBuffer[this->muEnvBufferIndex++] = '\0';
    this->mppcEnv[this->muEnvIndex] = NULL;
    
    return 0;
}

char* const* CProcessOption::GetEnv(void) const
{
    return this->mppcEnv;
}

apl_int_t CProcessOption::SetWorkingDir( char const* apcDirectory )
{
    if (this->mpcWorkingDir == NULL)
    {
        ACL_NEW_N_ASSERT(this->mpcWorkingDir, char, APL_PATH_MAX);
    }
    
    apl_snprintf(this->mpcWorkingDir, APL_PATH_MAX, "%s", apcDirectory );
    
    return 0;
}

char const* CProcessOption::GetWorkingDir(void) const
{
    return this->mpcWorkingDir;
}

void CProcessOption::Prepare(void)
{
    CTokenizer loParse(" ", true);
    apl_size_t liLen = apl_strlen(this->mpcCommandLine);
    apl_size_t liBufferLen = MAX_CMDLINE_LEN - liLen;
    char* lpcBuffer = this->mpcCommandLine + liLen;

    for (std::vector<apl_handle_t>::iterator loIter = this->moHandleSet.begin();
         loIter != this->moHandleSet.end(); loIter++ )
    {
        apl_snprintf( lpcBuffer, liBufferLen, " +H %"PRId64, (apl_int64_t)(*loIter) );
        liLen = apl_strlen(lpcBuffer);
        liBufferLen = liBufferLen - liLen;
        lpcBuffer = lpcBuffer + liLen;
    }

    loParse.Parse(this->mpcCommandLine);
    for ( apl_size_t liN = 0; 
        liN < loParse.GetSize() && this->muArgvIndex < MAX_ARG_SIZE; liN++ )
    {
        if (this->muArgvIndex < MAX_ARG_SIZE)
        {
            this->mppcArgv[this->muArgvIndex++] = const_cast<char*>( loParse.GetField(liN) );
        }
        else
        {
            apl_errprintf( 
                "CProcessOption::Prepare: Too many param, max=%"PRId32", curr=%"PRId32", param=%s\n",
                (apl_int32_t)MAX_ARG_SIZE,
                (apl_int32_t)this->muArgvIndex,
                loParse.GetField(liN) );
        }
    }
    this->mppcArgv[this->muArgvIndex] = NULL;
}

/////////////////////////////////////////////// Process //////////////////////////////////////////
CProcess::CProcess(void)
    : miChildID(-1)
    , miHandle(ACL_INVALID_HANDLE)
{
}

CProcess::~CProcess(void)
{
}

apl_int_t CProcess::Spawn( CProcessOption& aoOption )
{
    aoOption.Prepare();

    this->miChildID = apl_fork();
    
    switch (this->miChildID)
    {
        case -1:
        {
            return -1;
        }
        case 0:
        {
            //Child process
            if ( aoOption.GetStdIn() != ACL_INVALID_HANDLE
                && apl_dup2( aoOption.GetStdIn(), APL_STDIN_FILENO ) == -1 )
            {
                apl_errprintf( 
                    "CProcess::Spawn: apl_dup2(%"PRId32", %"PRId32") failed, errno=%"PRId32"\n",
                    (apl_int32_t)aoOption.GetStdIn(),
                    (apl_int32_t)APL_STDIN_FILENO,
                    (apl_int32_t)apl_get_errno() );
                apl_exit( apl_get_errno() );
            }
            if ( aoOption.GetStdOut() != ACL_INVALID_HANDLE
                && apl_dup2( aoOption.GetStdOut(), APL_STDOUT_FILENO ) == -1 )
            {
                apl_errprintf( 
                    "CProcess::Spawn: apl_dup2(%"PRId32", %"PRId32") failed, errno=%"PRId32"\n",
                    (apl_int32_t)aoOption.GetStdIn(),
                    (apl_int32_t)APL_STDOUT_FILENO,
                    (apl_int32_t)apl_get_errno() );
                apl_exit( apl_get_errno() );
            }
            if ( aoOption.GetStdErr() != ACL_INVALID_HANDLE
                && apl_dup2( aoOption.GetStdErr(), APL_STDERR_FILENO ) == -1 )
            {
                apl_errprintf( 
                    "CProcess::Spawn: apl_dup2(%"PRId32", %"PRId32") failed, errno=%"PRId32"\n",
                    (apl_int32_t)aoOption.GetStdIn(),
                    (apl_int32_t)APL_STDERR_FILENO,
                    (apl_int32_t)apl_get_errno() );
                apl_exit( apl_get_errno() );
            }
            
            apl_close( aoOption.GetStdIn() );
            apl_close( aoOption.GetStdOut() );
            apl_close( aoOption.GetStdErr() );
            
            if ( aoOption.GetWorkingDir() != NULL
                && apl_chdir( aoOption.GetWorkingDir() ) )
            {
                apl_errprintf( 
                    "CProcess::Spawn: apl_chdir(%s) failed, errno=%"PRId32"\n",
                    aoOption.GetWorkingDir(),
                    (apl_int32_t)apl_get_errno() );
                apl_exit( apl_get_errno() );
            }
            
            this->Child();
            
            if ( aoOption.GetProcName() != NULL
                && apl_exec( aoOption.GetProcName(), aoOption.GetCmdLine(), aoOption.GetEnv() ) != 0 )
            {
                apl_errprintf( 
                    "CProcess::Spawn: apl_exec( \"%s\", ..., ... ) failed, errno=%"PRId32"\n",
                    aoOption.GetProcName(),
                    (apl_int32_t)apl_get_errno() );
                    
                //errprintf
                apl_errprintf("Param:\n");
                for ( apl_size_t liN = 0; aoOption.GetCmdLine()[liN] != NULL ; liN++ )
                {
                    apl_errprintf( "%s\t", aoOption.GetCmdLine()[liN] );
                }
                //errprintf
                apl_errprintf("\nEnv:\n");
                for ( apl_size_t liN = 0; aoOption.GetEnv()[liN] != NULL ; liN++ )
                {
                    apl_errprintf( "%s\t", aoOption.GetEnv()[liN] );
                }
                apl_errprintf("\n");
                
                apl_exit( apl_get_errno() );
            }
            
            return 0;
        }
        default:
        {
            //Parent process
            this->Parent();
            
            return 0;
        }
    };
}

apl_int_t CProcess::Wait( apl_int_t* apiExitCode,  CTimeValue const& aoTimeout )
{
    apl_int_t liExitCode = 0;
    apl_int_t liRetCode = 0;
    
    if ( aoTimeout == CTimeValue::MAXTIME || aoTimeout.Nsec() < 0 )
    {
        liRetCode = apl_waitpid(this->miChildID, &liExitCode, 0);

    }
    else if ( aoTimeout == CTimeValue::ZERO )
    {
        liRetCode = apl_waitpid(this->miChildID, &liExitCode, APL_WNOHANG);
    }
    else
    {
        CTimeValue loTimeout(aoTimeout);
        
        while(loTimeout.Sec() > 0)
        {
            liRetCode = apl_waitpid(this->miChildID, &liExitCode, APL_WNOHANG);
            if (liRetCode != 0)
            {
                break;
            }
            
            apl_sleep(APL_TIME_SEC);
            
            --loTimeout;
        }
    }
    
    if (liRetCode != -1 && apiExitCode != NULL)
    {
        *apiExitCode = liExitCode;
    }
    
    return liRetCode;
}

apl_int_t CProcess::Kill( apl_int_t aiSignal ) const
{
    return apl_kill(this->miChildID, aiSignal);
}

apl_pid_t CProcess::GetChildID(void)
{
    return this->miChildID;
}

apl_pid_t CProcess::GetPid(void)
{
    return apl_getpid();
}

apl_pid_t CProcess::GetPpid(void)
{
    return apl_getppid();
}

void CProcess::Parent(void)
{
    //NULL
}

void CProcess::Child(void)
{
    //NULL
}

ACL_NAMESPACE_END
