
#include <assert.h>
//#include <stdint.h>
#include "AITime.h"
#include "AIString.h"
#include "AILogSys.h"
#include "AITcpSocket.h"
#include "AIDir.h"
#include "AIPacketTrace.h"
#include "AIModule.h"

///compatible aimodule last version
////////////////////////////////////////////////////////////////////////////////////
///// Variables for global
///start namespace
AIBC_NAMESPACE_START

volatile int  giGlobalReLoad = 0;
volatile int  giGlobalShutDown = 0;
volatile int  giGlobalLogLevel = 0;
char gsGlobalModuleID[128] = {0,};
char gsGlobalProgName[128] = {0,};
char gsGlobalStartTime[24] = {0,};

///end namespace
AIBC_NAMESPACE_END
////

namespace AIM {
    
////
TCommandHookRegFunc    __gfAIModuleParentHookRegFunc = NULL;
TCommandHookRegFunc    __gfAIModuleChildHookRegFunc  = NULL;
TArgumentOptionRegFunc __gfArgumentOptionRegFunc  = DefaultArgumentOption;
TInitializeFunc        __gfInitializeFunc = NULL;
TFinallyFunc           __gfFinallyFunc    = NULL;
TReloadFunc            gpfReLoad = NULL;
////

AIModuleLog* AIModuleLog::scpoInstance = NULL;
    
AIModuleLog::AIModuleLog() 
    : ciFileHandle(-1)
{
}

AIModuleLog::~AIModuleLog()
{
    this->Close();
}

AIModuleLog* AIModuleLog::Instance()
{
    if ( scpoInstance == NULL ) 
    {
        AI_NEW_ASSERT( scpoInstance, AIModuleLog);
    }
    return scpoInstance;
}

void AIModuleLog::Release()
{
    AI_DELETE( scpoInstance );
}

int AIModuleLog::Open( const char* apcFileName )
{
    //Close first
    this->Close();
    
    this->ciFileHandle = ::open( apcFileName, O_CREAT|O_WRONLY|O_APPEND, 0600 );
    if ( this->ciFileHandle < 0 )
    {
        return -1;
    }
    
    this->coLock.Unlock();
    
    return 0;
}

void AIModuleLog::Close()
{
    if ( this->ciFileHandle >= 0 )
    {
        ::close(this->ciFileHandle);
        this->ciFileHandle = -1;
    }
}
    
int AIModuleLog::Write( const char *apcFormat, ... )
{
    va_list loVaList;
    char    lsBuffer[1024] = {0};
    char    *lpcCurPtr = NULL;
    size_t  liRetSize = 0;
    
    AIBC::GetTimeString( lsBuffer, 0, 1 );
    lpcCurPtr  = lsBuffer + strlen(lsBuffer);
    *lpcCurPtr++ = ':';
    
    AIBC::AISmartLock loSmartLock(coLock);
    
    ::va_start( loVaList, apcFormat );
        
    //HP-UX will return -1 when buffer is no enough
    liRetSize = vsnprintf( lpcCurPtr, sizeof(lsBuffer) - (lpcCurPtr - lsBuffer), apcFormat, loVaList );
    if ( liRetSize < 0 //HP-UNIX
        || liRetSize > sizeof(lsBuffer) - (lpcCurPtr - lsBuffer) - 1 )
    {
        lpcCurPtr += strlen(lpcCurPtr);
    }
    else
    {
        lpcCurPtr += liRetSize;
    }
    
    ::va_end( loVaList );

    if( ( lpcCurPtr-lsBuffer ) > 0 && *(lpcCurPtr -1) != '\n' )
    {
        *lpcCurPtr++ = '\n';
        *lpcCurPtr = '\0';
    }

    write( this->ciFileHandle, lsBuffer, lpcCurPtr - lsBuffer );
    
    return 0;
}

////////////////////////////////////////// *ProcessManager* //////////////////////////////////////
AIProcessManager::AIProcessManager( const char* apcProcessName, size_t aiChildCnt ) : 
    cpoProcessNode(NULL),ciProcCnt(0),ciCapacity(0)
{
    ::snprintf( csFileName, sizeof(csFileName), "%s.pid", apcProcessName );
    this->SetMaxChildCnt( aiChildCnt );
}

AIProcessManager::~AIProcessManager()
{
    AI_FREE( this->cpoProcessNode );
}

void AIProcessManager::SetMaxChildCnt( size_t aiChildCnt )
{
    if ( aiChildCnt > this->ciCapacity )
    {
        AI_REALLOC_ASSERT( this->cpoProcessNode, stProcessNode, 
            aiChildCnt*sizeof(stProcessNode) );
		::memset( this->cpoProcessNode + this->ciCapacity, 0, 
		    sizeof(stProcessNode)*(aiChildCnt - this->ciCapacity) );
    }
    this->ciCapacity = aiChildCnt;
}

int AIProcessManager::LoadFromFile()
{
    char lsLine[AIM_MAX_LINE_LEN];
     
    FILE* lpFile = ::fopen( csFileName, "r" );
    if ( lpFile == NULL )
    {
        return -1;
    }

    while( fgets( lsLine, sizeof(lsLine), lpFile ) )
    {
        if ( this->ciProcCnt >= this->ciCapacity )
        {
            this->SetMaxChildCnt( this->ciProcCnt * 2 );
        }
        
        if ( atoi(lsLine) > 0 )
        {
            cpoProcessNode[this->ciProcCnt].ctPHandle    = (AI_PHANDLE)atoi(lsLine);
            cpoProcessNode[this->ciProcCnt++].ciBookTime = time(NULL);
        
            assert( AIM_MAX_CHILD_CNT > this->ciProcCnt );
        }
    }
    
    ::fclose( lpFile );
    
    return 0;
}

int AIProcessManager::PersistentToFile()
{
    char lsTimeString[100];
    
    FILE* lpFile = ::fopen( csFileName, "w+" );
    if ( lpFile == NULL )
    {
        return -1;
    }
    
    AIBC::GetTimeString( lsTimeString, 0, 1 );
    ::fprintf( lpFile, "%u %s\n\n", ::getpid(), lsTimeString );

    for( size_t liIt = 0; liIt < this->ciCapacity; liIt++ )
    {
        if ( !IS_VALID_PHANDLE( this->cpoProcessNode[liIt].ctPHandle ) )
        {
            continue;
        }
        AIBC::GetTimeString( lsTimeString, 0, 1 );
        ::fprintf( lpFile, "%u %s\n", this->cpoProcessNode[liIt].ctPHandle, lsTimeString );
    }
    
    ::fclose( lpFile );
    
    return 0;
}

int AIProcessManager::WaitAll()
{
    while( ::waitpid( (AI_PHANDLE)-1, NULL, WUNTRACED ) > 0 )
    {
        continue;
    }
    return 0;
}

int AIProcessManager::StopAll()
{
    ///Handle stop signal on all process here
    for ( size_t liN = 0; liN < this->ciCapacity; liN++ )
    {
        if ( IS_VALID_PHANDLE( this->cpoProcessNode[liN].ctPHandle ) )
        {
            this->HandleStop( this->cpoProcessNode[liN].ctPHandle );
        }
    }
    return 0;
}

int AIProcessManager::KillAll()
{
    ///Handle kill signal on all process here
    for ( size_t liN = 0; liN < this->ciCapacity; liN++ )
    {
        if ( IS_VALID_PHANDLE( this->cpoProcessNode[liN].ctPHandle ) )
        {
            this->HandleKill( this->cpoProcessNode[liN].ctPHandle );
        }
    }
    return 0;
}

int AIProcessManager::ReloadAll()
{
    ///Handle reload signal on all process here
    for ( size_t liN = 0; liN < this->ciCapacity; liN++ )
    {
        if ( IS_VALID_PHANDLE( this->cpoProcessNode[liN].ctPHandle ) )
        {
            this->HandleReload( this->cpoProcessNode[liN].ctPHandle );
        }
    }
    return 0;
}
    
//// operation
int AIProcessManager::GetFreeNo(void)
{
    return this->Find( (AI_PHANDLE)0 );
}

int AIProcessManager::Find( AI_PHANDLE atPHandle )
{
    for ( size_t liN = 0; liN < this->ciCapacity; liN++ )
    {
        if ( this->cpoProcessNode[liN].ctPHandle == atPHandle )
        {
            return liN;
        }
    }
    
    return -1;
}

void AIProcessManager::ProcBook( size_t aiNo, AI_PHANDLE atPHandle )
{
    this->cpoProcessNode[aiNo].ctPHandle   = atPHandle;
    this->cpoProcessNode[aiNo].ciBookTime  = time(NULL);
    this->cpoProcessNode[aiNo].ciLoginTime = 0;
    
    {
        AIBC::AISmartLock loLock(coLock);
        this->ciProcCnt++;
        assert( AIM_MAX_CHILD_CNT > this->ciProcCnt );
    }
}

void AIProcessManager::ProcLogin( AI_PHANDLE atPHandle )
{
    size_t liSlot = 0;
    if( ( liSlot = Find( atPHandle ) ) >= 0 
        && atPHandle == this->cpoProcessNode[liSlot].ctPHandle )
    {
        this->cpoProcessNode[liSlot].ciLoginTime = time(NULL);
    }
}

void AIProcessManager::ProcLogout( AI_PHANDLE atPHandle )
{
    size_t liSlot = 0;
    if( ( liSlot = Find( atPHandle ) ) >= 0 
        && atPHandle == this->cpoProcessNode[liSlot].ctPHandle )
    {
        this->cpoProcessNode[liSlot].ciLogoutTime = time(NULL);
        
        {
            AIBC::AISmartLock loLock(coLock);
            this->ciProcCnt--;
        }
    }
}

void AIProcessManager::RecycleRubbish()
{
    for ( size_t liN = 0; liN < this->ciCapacity; liN++ )
    {
        if ( !IS_VALID_PHANDLE( this->cpoProcessNode[liN].ctPHandle )
            || this->cpoProcessNode[liN].ciLogoutTime > 0 )
        {
            continue;
        }
        ///Exception exit child
        if ( this->cpoProcessNode[liN].ciLoginTime > 0 
            && !this->IsProcessExisted(this->cpoProcessNode[liN].ctPHandle) )
        {
            memset( &this->cpoProcessNode[liN], 0, sizeof(stProcessNode) );
            
            {
                AIBC::AISmartLock loLock(coLock);
                this->ciProcCnt--;
            }
            
            continue;
        }
        ///TODO igrone login timeout child here
    }
}

AIProcessManager::CHILD_STATE AIProcessManager::DetectState( int aiTimeOut )
{
    int liTotalCnt   = 0;
    int liLogoutCnt  = 0;
    int liTimeOutCnt = 0;
    
    for ( size_t liN = 0; liN < this->ciCapacity; liN++ )
    {
        if ( !IS_VALID_PHANDLE( this->cpoProcessNode[liN].ctPHandle ) )
        {
            continue;
        }
        
        liTotalCnt++;
        
        if ( this->cpoProcessNode[liN].ciLoginTime == 0 
            && time(NULL) - this->cpoProcessNode[liN].ciBookTime > aiTimeOut )
        {
            liTimeOutCnt++;
        }

        if ( this->cpoProcessNode[liN].ciLogoutTime > 0 )
        {
            liLogoutCnt++;
        }
    }
    
    if ( liTotalCnt == 0 ) return AIProcessManager::NORMAL;

    if ( liTimeOutCnt == liTotalCnt )
    {
        return AIProcessManager::TIMEOUT;
    }
    if ( liLogoutCnt == liTotalCnt )
    {
        return AIProcessManager::LOGOUT;
    }
    
    return AIProcessManager::NORMAL;
}

AI_PHANDLE AIProcessManager::operator [] ( size_t aiN )
{
    assert ( this->ciCapacity > aiN );

    return this->cpoProcessNode[aiN].ctPHandle;
}
    
size_t AIProcessManager::GetCount(void)
{
    return this->ciProcCnt;
}

//// Handle function
bool AIProcessManager::IsProcessExisted( AI_PHANDLE atPHandle )
{
    ::waitpid( atPHandle, NULL, WNOHANG );
    return ::kill( atPHandle, 0 ) == 0 ? true : false;
}

int  AIProcessManager::HandleStop( AI_PHANDLE atPHandle )
{
    return ::kill( atPHandle, SIGTERM );
}

int  AIProcessManager::HandleKill( AI_PHANDLE atPHandle )
{
    return ::kill( atPHandle, SIGKILL );
}

int  AIProcessManager::HandleReload( AI_PHANDLE atPHandle )
{
    return ::kill( atPHandle, SIGUSR1 );
}

////////////////////////////////////////// *Config* /////////////////////////////////////////////
AIModuleConfig::stIntIterator::stIntIterator( 
    AIBC::AIConfig* apoConfig, const char* apcField, const char* apcList, int aiDefault ) :
    cpoConfig(apoConfig),
    cpcField(apcField),
    ciDefault(aiDefault)
{
    if ( apcList == NULL )
    {
        this->cpcFirst = "";
        this->cpcLast  = cpcFirst;
    }
    else
    {
        ///Skip '|'
        while( *apcList == '|' ) apcList++;
        this->cpcFirst = apcList;
        this->cpcLast  = apcList + strlen(apcList);
    }
}

void AIModuleConfig::stIntIterator::operator ++ ( int )
{
    const char* lpcNext = ::strchr(cpcFirst,'|');
    if ( lpcNext != NULL )
    {
        while( *lpcNext == '|' ) lpcNext++;
        
        this->cpcFirst = lpcNext;
    }
    else
    {
        this->cpcFirst = cpcLast;
    }
}

int AIModuleConfig::stIntIterator::operator * ()
{
    return this->cpoConfig->GetIniInt( cpcField, this->CurrKey(), ciDefault );
}

const char* AIModuleConfig::stIntIterator::CurrKey()
{
    static char lsTmpName[AIM_MAX_NAME_LEN];
    const char* lpcNext = ::strchr(cpcFirst,'|');
    char* lpcCurr = lsTmpName;
    
    if ( lpcNext == NULL ) lpcNext = this->cpcLast;
        
    assert( AIM_MAX_NAME_LEN > lpcNext - this->cpcFirst );
    
    *lpcCurr   = '\0';
    for ( const char* lpcFirst = cpcFirst; lpcFirst < lpcNext; lpcFirst++ )
    {
        *lpcCurr++ = *lpcFirst;
        *lpcCurr   = '\0';
    }
    
    return lsTmpName;
}

///AIModuleConfig
AIModuleConfig::AIModuleConfig() : 
    ciChildCnt(0),
    ciListenPort(0)
{
    memset( csModuleID, 0, sizeof(csModuleID) );
    memset( csListenIpAddr, 0, sizeof(csListenIpAddr) );
    memset( csPassword, 0, sizeof(csPassword) );
}

AIModuleConfig* AIModuleConfig::Instance()
{
    static AIModuleConfig* slpoConfig = NULL;
    if (slpoConfig == NULL) AI_NEW_ASSERT( slpoConfig, AIModuleConfig );
    return slpoConfig;
}

int AIModuleConfig::Load( const char* apcPath, const char* apcField )
{
    if ( this->coConfig.LoadINI( apcPath ) < 0 )
    {
        //TODO ERR
        return -1;
    }
    
    this->ciChildCnt   = this->GetIntFromConfig( apcField, "MaxChildCount", 1 );
    this->ciListenPort = this->GetIntFromConfig( apcField, "AdminListenPort", -1 );
    
    this->GetStrFromConfig( apcField, "ModuleID", this->csModuleID, sizeof(this->csModuleID) );
    this->GetStrFromConfig( apcField, "AdminListenIpAddr", this->csListenIpAddr, sizeof(this->csListenIpAddr) );
    this->GetStrFromConfig( apcField, "AdminListenPassword", this->csPassword, sizeof(this->csPassword) );
    
    ///compatible aimodule last version
    AIBC::StringCopy( AIBC::gsGlobalModuleID, this->csModuleID, sizeof(AIBC::gsGlobalModuleID) );
    ///
    
    return 0;
}

int AIModuleConfig::GetIntFromConfig( const char* apcField, const char* apcKey, int aiDefault )
{
    return this->coConfig.GetIniInt( apcField, apcKey, aiDefault );
}

int AIModuleConfig::GetStrFromConfig( const char* apcField, const char* apcKey, char* apcValue, size_t aiSize )
{
    return this->coConfig.GetIniString( apcField, apcKey, apcValue, aiSize );
}

AIModuleConfig::TIntIter AIModuleConfig::GetIntListFromConfig( const char* apcField, const char* apcListKey, int aiDefault )
{
    return stIntIterator( &this->coConfig, apcField, apcListKey, aiDefault );
}

////////////////////////////////////////// *Protocol* ////////////////////////////////////////
////
AIModuleProtocol::AIModuleProtocol()
{
}
AIModuleProtocol::~AIModuleProtocol()
{
}
int AIModuleProtocol::Decode( const char* apcData, size_t aiSize )
{
    const char* lpcLast  = apcData;
    const char* lpcNext  = NULL;
    
    while ( (lpcLast = (char*)::memchr( lpcLast, '\r', aiSize - (lpcLast - apcData) ) ) != NULL )
    {
        if ( lpcLast + 1 < apcData + aiSize && *(lpcLast+1) == '\n' )
        {
            break;
        }
        
        lpcLast++;
    }
    
    if ( lpcLast == NULL )
    {
        return -1;
    }
    
    this->coCommandLine.Resize( lpcLast - apcData + 2 /* end */ );
    ::memcpy( this->coCommandLine.BasePtr(), apcData, lpcLast - apcData );
    ::memcpy( this->coCommandLine.BasePtr() + (lpcLast - apcData), " ", 2 );
    
    this->ciArgc = 0;
    this->coAppendedData.Resize(0);
    while( (lpcNext = ::strchr( this->coCommandLine.ReadPtr(), ' ' )) != NULL )
    {
        if ( this->ciArgc >= AIM_MAX_PTL_ARGC ) return -2;
        
        if ( ::strncmp(this->coCommandLine.ReadPtr(), AIBC::AICmdLineChannel::STAG, strlen(AIBC::AICmdLineChannel::STAG)) == 0 )
        {
            size_t liAppendedSize = atoi(this->coCommandLine.ReadPtr() + strlen(AIBC::AICmdLineChannel::STAG));
            if ( liAppendedSize != aiSize - (lpcLast - apcData) - 2 /*\r\n*/ )
            {
                return -2;
            }
            this->coAppendedData.Resize( liAppendedSize );
            ::memcpy( this->coAppendedData.BasePtr(), lpcLast + 2, liAppendedSize );
        }
        else
        {
            this->coArgv[this->ciArgc].Resize( lpcNext - this->coCommandLine.ReadPtr() + 1 /*end*/ );
            ::memcpy( this->coArgv[this->ciArgc].BasePtr(), this->coCommandLine.ReadPtr(), lpcNext - this->coCommandLine.ReadPtr() );
            ::memset( this->coArgv[this->ciArgc].BasePtr() + this->coArgv[this->ciArgc].GetSize() - 1, 0, 1 );
            this->ciArgc++;
        }
        
        /// Skip blank
        while( *lpcNext == ' ' ) lpcNext++;
        
        this->coCommandLine.ReadPtr( const_cast<char*>(lpcNext) );
    }
    
    ///complete
    ::memset( this->coCommandLine.BasePtr() + this->coCommandLine.GetSize() - 2, 0, 1 ); //Erase end blank
    this->coCommandLine.ResetRead();

    return 0;
}

int AIModuleProtocol::Encode( AIBC::AIChunkEx& aoBuffer, const AIBC::AIChunkEx* apoAppendedData, const char* apcFormat, ... )
{
    ///Check protocol format, end by \r\n
    const char* lpcLast = ::strstr( apcFormat, "\r\n" );
    if ( lpcLast != NULL && lpcLast[3] != '\0' )
    {
        return -1;
    }
    
    aoBuffer.Resize(AIM_MAX_LINE_LEN);
    aoBuffer.Reset();
    
    ///va all param value
    va_list loVl;
    va_start( loVl, apcFormat );
    vsnprintf( aoBuffer.BasePtr(), aoBuffer.GetSize(), apcFormat, loVl );
    va_end( loVl );
    
    ///resize buffer
    size_t liLen = strlen(aoBuffer.BasePtr());
    if ( lpcLast != NULL ) liLen -= 2;
    aoBuffer.WritePtr( liLen );

    if ( apoAppendedData == NULL || apoAppendedData->GetSize() <= 0 )
    {
        ///End of encode
        aoBuffer.Resize( liLen + 2 );
        ::memcpy( aoBuffer.WritePtr(), "\r\n", 2 );
        aoBuffer.WritePtr(2);
    }
    else
    {
        ///Appended data
        size_t liRemainSize = AIM_MAX_LINE_LEN - ( aoBuffer.BasePtr() - aoBuffer.WritePtr() );
        ::snprintf( aoBuffer.WritePtr(), liRemainSize, " APPENDED=%lu\r\n", (long unsigned int)apoAppendedData->GetSize() );
        liLen = strlen(aoBuffer.BasePtr());
        aoBuffer.Resize( liLen + apoAppendedData->GetSize() );
        aoBuffer.WritePtr( aoBuffer.BasePtr() + liLen );
        ::memcpy( aoBuffer.WritePtr(), apoAppendedData->BasePtr(), apoAppendedData->GetSize() );
    }
    
    return 0;
}
    
bool AIModuleProtocol::IsCommand( const char* apcCmd ) const
{
    return ::strcmp( this->coArgv[0].BasePtr(), apcCmd ) == 0? true : false;
}

bool AIModuleProtocol::IsHasAppended() const
{
    return coAppendedData.GetSize() > 0 ? true : false;
}

const char* AIModuleProtocol::GetCommandLine() const
{
    return this->coCommandLine.BasePtr();
}

const char* AIModuleProtocol::GetCommand() const
{
    return this->coArgv[0].BasePtr();
}

const char* AIModuleProtocol::GetParam( size_t aiN ) const
{
    return this->coArgv[aiN + 1].BasePtr();
}

size_t AIModuleProtocol::GetParamCnt() const
{
    return this->ciArgc - 1;
}

const AIBC::AIChunkEx& AIModuleProtocol::GetAppendedData() const
{
    return this->coAppendedData;
}
    
////////////////////////////////////////// *ModuleEnv* //////////////////////////////////////
AIModuleEnv::AIModuleEnv() : 
    cbIsShutdown(false),
    cbIsReload(false),
    cpoFirst(NULL),
    cpoLast(NULL)
{
    ::getcwd( this->csRootDirectory, sizeof(this->csRootDirectory) );
    ::getcwd( this->csHomeDirectory, sizeof(this->csHomeDirectory) );
}

AIModuleEnv::~AIModuleEnv()
{
}

AIModuleEnv* AIModuleEnv::Instance()
{
    static AIModuleEnv* slpoModuleEnv = NULL;
    if (slpoModuleEnv == NULL) AI_NEW_ASSERT( slpoModuleEnv, AIModuleEnv);
    return slpoModuleEnv;
}

int AIModuleEnv::Initialize( const char* apcProcessName )
{
    char lsLevelFile[AIM_MAX_NAME_LEN];
    
    snprintf( this->csProcessName, sizeof(this->csProcessName), "%s", apcProcessName );
    
    ///Make base directory
    AIBC::AIDir::Make( AIM_LOCKFILE_PATH, 0777 );
    
    snprintf( lsLevelFile, sizeof(lsLevelFile), "%s.level", this->csProcessName );
    remove( lsLevelFile );
    
    return 0;
}

int AIModuleEnv::RunByDaemon()
{
    switch( ::fork() )
    {
        case 0:
        {
            ::setsid();
            break;
        }
        default: ::exit(0);
    }
    
    return 0;
}

void AIModuleEnv::UpdateShutdown( bool aiFlag )
{
    this->cbIsShutdown = aiFlag;
    ///compatible aimodule last version
    AIBC::giGlobalShutDown   = aiFlag;
    ///
}

void AIModuleEnv::UpdateReload( bool aiFlag )
{
    this->cbIsReload = aiFlag;
    ///compatible aimodule last version
    AIBC::giGlobalReLoad   = aiFlag;
    ///
}

void AIModuleEnv::ClearHook()
{
    this->coHookNode.Clear();
}

void AIModuleEnv::RegisterCommandHook( const char* apcCommand, void* afHook )
{
    assert( AIM_MAX_COMMAND_HOOK_CNT > this->coHookNode.Size() );
    
    size_t liIdx = this->coHookNode.Size();
    ::snprintf( coHookNode[liIdx].csCommand, sizeof(coHookNode[liIdx].csCommand), "%s", apcCommand );
    this->coHookNode[liIdx].cfHookFunc = afHook;
    this->coHookNode.Insert(liIdx);
}

void* AIModuleEnv::FindCommandHook( const char* apcCommand )
{
    stHookNode* lpoNode = coHookNode.Find( apcCommand );
    if ( lpoNode != NULL )
    {
        return lpoNode->cfHookFunc;
    }
    return NULL;
}

bool AIModuleEnv::IsShutdown()
{
    return this->cbIsShutdown;
}

bool AIModuleEnv::IsReload()
{
    return this->cbIsReload;
}

void AIModuleEnv::SetRootDirectory( const char* apcDirectory /* = NULL */ )
{
    if ( apcDirectory == NULL )
    {
        ::getcwd( this->csRootDirectory, sizeof(this->csRootDirectory) );
    }
    else
    {
        ::snprintf( this->csRootDirectory, sizeof(this->csRootDirectory), "%s", apcDirectory );
    }
}

void AIModuleEnv::SetHomeDirectory( const char* apcDirectory /* = NULL */ )
{
    if ( apcDirectory != NULL )
    {
        AIBC::AIDir::Make(apcDirectory);
        chdir( apcDirectory );
    }
    ::getcwd( this->csHomeDirectory, sizeof(this->csHomeDirectory) );
    
    ///compatible aimodule last version
    AIBC::StringCopy( AIBC::gsConfigDirectory, "../../config", sizeof(AIBC::gsConfigDirectory) );
    ///
}

const char* AIModuleEnv::GetRootDirectory() const
{
    return this->csRootDirectory;
}

const char* AIModuleEnv::GetHomeDirectory() const
{
    return this->csHomeDirectory;
}

const char* AIModuleEnv::GetProcessName() const
{
    return this->csProcessName;
}

void AIModuleEnv::AddModuleVersion( const char* apcModuleName, const char* apcVersion, const char* apcTime )
{
    assert( apcModuleName != NULL && apcVersion != NULL && apcTime != NULL );
    
    stModuleVersion* lpoVersion = NULL;
    AI_NEW_ASSERT( lpoVersion, stModuleVersion );
    ::snprintf( lpoVersion->csModuleName, sizeof(lpoVersion->csModuleName), "%s", apcModuleName );
    ::snprintf( lpoVersion->csVersion, sizeof(lpoVersion->csVersion), "%s", apcVersion );
    ::snprintf( lpoVersion->csTime, sizeof(lpoVersion->csModuleName), "%s", apcTime );
    lpoVersion->cpoNext = NULL;
    
    if ( this->cpoLast == NULL )
    {
        this->cpoFirst = lpoVersion;
        this->cpoLast  = lpoVersion;
    }
    else
    {
        this->cpoLast->cpoNext = lpoVersion;
        this->cpoLast = lpoVersion;
    }
}

AIModuleEnv::TVersionHandle AIModuleEnv::FirstVersion()
{
    return this->cpoFirst;
}

////////////////////////////////////////// *ModuleConnServer* //////////////////////////////////////
AIModuleConnServer* AIModuleConnServer::Instance()
{
    static AIModuleConnServer* slpoModuleConnServer = NULL;
    if (slpoModuleConnServer == NULL) AI_NEW_ASSERT( slpoModuleConnServer, AIModuleConnServer);
    return slpoModuleConnServer;
}

AIModuleConnServer::AIModuleConnServer()
{
}

AIModuleConnServer::~AIModuleConnServer()
{
}

void AIModuleConnServer::AddListenPort( const char* apcName, int aiPort )
{
    assert( AIM_MAX_LISTEN_SOCKET_CNT > coServerNode.Size() );
    
    size_t liIdx = coServerNode.Size();
    ::snprintf( coServerNode[liIdx].csName, sizeof(coServerNode[liIdx].csName), "%s", apcName );
    coServerNode[liIdx].ciPort   = aiPort;
    coServerNode[liIdx].ciSocket = -1;
    coServerNode.Insert(liIdx);
}

int AIModuleConnServer::ListenAll( int* apiNodeId )
{
    for ( size_t liN = 0; liN < coServerNode.Size(); liN++ )
    {
        if( (coServerNode[liN].ciSocket = AIBC::ai_socket_listen( NULL, coServerNode[liN].ciPort, 5 )) <= 0 )
        {
            this->Close();
            
            if ( apiNodeId != NULL ) *apiNodeId = liN;
                
            return -1;
        }
    }
    return 0;
}

void AIModuleConnServer::Close()
{
    for ( size_t liN = 0; liN < coServerNode.Size(); liN++ )
    {
        AIBC::ai_socket_close( coServerNode[liN].ciSocket );
    }
}

AI_HANDLE AIModuleConnServer::GetListenSocket( const char* apcName )
{
    stServerNode* lpoNode = coServerNode.Find( apcName );
    if ( lpoNode != NULL )
    {
        return lpoNode->ciSocket;
    }
    
    return (AI_HANDLE)-1;
}

AI_HANDLE AIModuleConnServer::AcceptNewConn( const char* apcName, char* apcIpAddr, int* apiPort )
{
    AI_HANDLE liListenSocket;
    AI_HANDLE liClientSocket;
    char      lsClientIp[AIM_IP_ADDR_LEN];
    int       liClientPort;
    char      lsLockName[AIM_MAX_PATH_LEN];
    
    stServerNode* lpoNode = coServerNode.Find( apcName );
    if ( lpoNode == NULL || !IS_VALID_SOCKET( (liListenSocket = lpoNode->ciSocket) ) )
    {
        //TODO ERR
        AIM_ERROR( "Search server node by name [%s] fail", apcName );
        return (AI_HANDLE)-1;
    }
    
    //Create File lock
    snprintf( lsLockName, sizeof(lsLockName), 
        "%s/%s/%s.lck", AIModuleEnv::Instance()->GetRootDirectory(), AIM_LOCKFILE_PATH, apcName );
    AIBC::AIFileLock loFileLock( lsLockName );
    
    //Lock scope
    loFileLock.Lock(); //Mutex for multi-procsess
        lpoNode->coLock.Lock(); //Mutex for multi-thread
        {
            liClientSocket = AIBC::ai_socket_accept( liListenSocket, lsClientIp, &liClientPort );
        }
        lpoNode->coLock.Unlock();
    loFileLock.Unlock();
    
    if( !IS_VALID_SOCKET(liClientSocket) )
    {
        //TODO ERR
        AIM_ERROR( "Accept connection from %s fail, RETCODE:%d", apcName, liClientSocket );
        return (AI_HANDLE)-3;
    }
    
    if( apcIpAddr != NULL )
    {
        strcpy( apcIpAddr, lsClientIp );
    }

    if( apiPort != NULL )
    {
        *apiPort = liClientPort;
    }
     
    return  liClientSocket;
}

////////////////////////////////////////// *Parent* //////////////////////////////////////
// ServerChannel
AIServerChannel::AIServerChannel() : 
    ciRole(-1),cpoModule(NULL)
{
}

void AIServerChannel::SetModule( AIModuleParent* apModule )
{
    this->cpoModule = apModule;
}

int AIServerChannel::Input( const char* apcData, size_t aiSize )
{
    assert( this->cpoModule != NULL );
    
    //TODO External conntion command
    AIModuleProtocol loProtocol;
    
    if ( loProtocol.Decode(apcData, aiSize) != 0 )
    {
        // ERR invalid protocol
        AIM_ERROR( "%s", "Server channel recv invalid protocol request, ignore it here" );
        return -1;
    }
    
    switch( ciRole )
    {
        case 0:///Child
        {
            this->HandleChildInput( loProtocol );
            break;
        }
        case 1:///External
        {
            this->HandleExtInput( loProtocol );
            break;
        }
        default:
        {
            if ( loProtocol.IsCommand(AIM_COMMAND_HELLI) )
            {
                ciRole = 0;
                AIModuleParent::HandleChildLogin( this->cpoModule, this, &loProtocol );
                break;
            }
            if ( loProtocol.IsCommand(AIM_COMMAND_HELLO) )
            {
                ciRole = 1;
                AIModuleParent::HandleExtLogin( this->cpoModule, this, &loProtocol );
                break;
            }
        }
    };
    
    return 0;
}

int AIServerChannel::Error( AI_SYNCHANDLE atHandle, int aiErrno )
{
    AIM_ERROR( "Server channel ignore error here, [Handle:%d]/[ERRNO:%d]", atHandle, aiErrno );
    //TODO error
    //Noting here
    return 0;
}

int AIServerChannel::Exit()
{
    AIServerChannel* lpoChannel = this;

    if ( ciRole == 0 )
    {
        AIModuleParent::HandleChildLogout( this->cpoModule, lpoChannel, NULL );
    }
    else
    {
        AIModuleParent::HandleExtLogout( this->cpoModule, lpoChannel, NULL );
    }

    //Recycle here, Dominate my life by myself ^_^
    AI_DELETE( lpoChannel );
    
    return 0;
}

int AIServerChannel::HandleExtInput( AIModuleProtocol& aoProtocol )
{
    AIModuleParent::THandleFunc lpfHookFunc = 
            (AIModuleParent::THandleFunc)AIModuleEnv::Instance()->FindCommandHook( aoProtocol.GetCommand() );
    if ( lpfHookFunc == NULL )
    {
        AIM_INFO( "Server channel[%s:%d] recv invalid command [%s], deliver to childs immediately", 
            this->GetRemoteIp(), this->GetRemotePort(), aoProtocol.GetCommand() );
        AIModuleParent::HandleRequest( this->cpoModule, this, &aoProtocol );
        return -1;
    }
    
    AIM_INFO( "Server channel[%s:%d] recv command [%s] and do it now", 
        this->GetRemoteIp(), this->GetRemotePort(), aoProtocol.GetCommand() );
    
    return (*lpfHookFunc)( this->cpoModule, this, &aoProtocol );
}

int AIServerChannel::HandleChildInput( AIModuleProtocol& aoProtocol )
{
    //TODO child input
    
    do
    {
        if ( aoProtocol.IsCommand(AIM_COMMAND_TRACE) )
        {
            AIModuleParent::HandleChildTrace( this->cpoModule, this, &aoProtocol );
            break;
        }
        
        if ( aoProtocol.IsCommand(AIM_COMMAND_LOGOUT) )
        {
            AIModuleParent::HandleChildLogout( this->cpoModule, this, &aoProtocol );
            break;
        }
        
    }while(false);
    
    return 0;
}

// Module
AIModuleParent* AIModuleParent::Instance()
{
    static AIModuleParent* slpoModule = NULL;
    if ( slpoModule == NULL ) AI_NEW_ASSERT( slpoModule, AIModuleParent );
    return slpoModule;
}

AIModuleParent::AIModuleParent() : 
    cpoProcessManager(NULL)
{
    ::memset( this->cpoaChildChannel, 0, sizeof(this->cpoaChildChannel) );
    ::memset( this->cpoaExtChannel,   0, sizeof(this->cpoaExtChannel) );
    ::memset( this->cpoaTraceChannel, 0, sizeof(this->cpoaTraceChannel) );
        
    ///TODO signal
    ::signal( SIGALRM, SIG_IGN );
    ::signal( SIGPIPE, SIG_IGN );
    ::signal( SIGUSR2, SIG_IGN );
    ::signal( SIGHUP,  SIG_IGN );
    ::signal( SIGQUIT, SIG_IGN );

    ::signal( SIGUSR1, HandleSignalReload );
    ::signal( SIGINT,  HandleSignalShutdown );
    ::signal( SIGTERM, HandleSignalShutdown );
        
    ///TODO register hook function
    AIModuleEnv::Instance()->ClearHook();
    AIModuleEnv::Instance()->RegisterCommandHook( "SHUTDOWN",  (void*)HandleShutdown );
    AIModuleEnv::Instance()->RegisterCommandHook( "RELOAD",    (void*)HandleReload );
    AIModuleEnv::Instance()->RegisterCommandHook( "GETSOCKET", (void*)HandleRequestForResponse );
    AIModuleEnv::Instance()->RegisterCommandHook( "TRACE",     (void*)HandleExtTrace );
    ///Udf hook function
    if ( __gfAIModuleParentHookRegFunc != NULL )
    {
        (*__gfAIModuleParentHookRegFunc)( AIModuleEnv::Instance() );
    }
}

AIModuleParent::~AIModuleParent()
{
    AI_DELETE( this->cpoProcessManager );
}

int AIModuleParent::Initialize()
{
    if ( __gfInitializeFunc != NULL )
    {
        return (*__gfInitializeFunc)();
    }
    else
    {
        return 0;
    }
}

void AIModuleParent::Finally()
{
    if ( __gfFinallyFunc != NULL )
    {
        (*__gfFinallyFunc)();
    }
}

int AIModuleParent::Run( const AIBC::AIArgument& aoArgument, AIModule* apoModule, const char* apcPortList )
{ 
    AI_PHANDLE      ltPHandle;
    int             liRetCode = 0;
    int             liChildNo = 0;
    char            lsConfigName[PATH_MAX];
    AIModuleConfig* lpoConfig = AIModuleConfig::Instance();
    AIModuleConnServer* lpoConnServer = AIModuleConnServer::Instance();
    
    //TODO Process config
    snprintf( lsConfigName, sizeof(lsConfigName), "../config/%s.ini", aoArgument.GetProcessName() );
    if ( lpoConfig->Load( lsConfigName, "SERVER" ) != 0 )
    {
        fprintf( stderr, "ERROR: Load config file %s fail, please checking file is existed\n", lsConfigName );
        return -1;
    }
    
    for ( AIModuleConfig::TIntIter loIter = lpoConfig->GetIntListFromConfig( "SERVER", apcPortList, -1);
          !loIter.IsEof(); loIter++ )
    {
        //TODO list all listen port
        if ( *loIter <= 0 )
        {
            fprintf( stderr, "INFO: Listen port [%s = %d] Disabled\n", loIter.CurrKey(), (int)*loIter);
            continue;
        }
        
        lpoConnServer->AddListenPort( loIter.CurrKey(), *loIter );
    }
    
    //TODO Start admin server
    if ( this->Startup( lpoConfig->GetListenIpAddr(), lpoConfig->GetListenPort() ) != 0 )
    {
        //Error here
        fprintf( stderr, "ERROR: Startup admin server [%s:%d] fail, maybe listen port unuseable\n", 
            lpoConfig->GetListenIpAddr(), lpoConfig->GetListenPort() );
        return -1;
    }
    
    //TODO Start Module conn server
    int liNodeId = 0;
    if ( ( liRetCode = lpoConnServer->ListenAll( &liNodeId ) ) != 0 )
    {
        //Error here
        fprintf( stderr, "ERROR: Startup Module conn server [%s:%d] fail, maybe listen port unuseable\n", 
            lpoConnServer->GetName(liNodeId), lpoConnServer->GetPort(liNodeId) );
        return -1;
    }
    
    ///Create processmanager
    AI_NEW_INIT_ASSERT( this->cpoProcessManager, 
        AIProcessManager, aoArgument.GetProcessName(), lpoConfig->GetChildCount() );
        
    while( !AIModuleEnv::Instance()->IsShutdown() )
    {
        // Handle signal command
        if ( AIModuleEnv::Instance()->IsReload() )
        {
            this->HandleReload( this, NULL, NULL );
            AIModuleEnv::Instance()->UpdateReload( false );
        }
        
        this->cpoProcessManager->RecycleRubbish();
        
        if ( this->cpoProcessManager->DetectState( 30 /*sec*/ ) != AIProcessManager::NORMAL )
        {
            //exception here, shutdown now
            AIModuleEnv::Instance()->UpdateShutdown( true );
            continue;
        }
        
        // Check process child limit
        if ( ( liChildNo = this->cpoProcessManager->GetFreeNo() ) < 0 )
        {
            AIBC::AISleepFor( AI_TIME_SEC );
            continue;
        }
        
        // Add child here
        switch( ltPHandle = ::fork() )
        {
            case  0:
            {
                //TODO recycle parent
                TBase::Close(); // parent listen adm port
                
                //TODO for my child
                char          lsMyChildHome[PATH_MAX];
                int           liRetCode = 0;

                //TODO set child env
                snprintf( lsMyChildHome, sizeof(lsMyChildHome), "./CHILD_%d", liChildNo );
                AIModuleEnv::Instance()->SetRootDirectory();
                AIModuleEnv::Instance()->SetHomeDirectory( lsMyChildHome );
                
                AIM_LOG_INIT( "monitor.log" );
                AIBC::AIInitIniHandler();
                AIBC::AIInitLOGHandler();
                
                AIM_INFO( "%s", "======================================================" );
                AIM_INFO( "Start to run child [%d] ...", ::getpid() );
                    liRetCode = AIModuleChild::Instance()->Run( aoArgument, apoModule );
                    AIModuleChild::Instance()->Close();
                AIM_INFO( "Child [%d] exit now...", ::getpid() );
                AIM_INFO( "%s", "======================================================" );
                
                AIBC::AICloseLOGHandler();
                AIBC::AICloseIniHandler();
                
                //Exit now
                ::exit(0);
                    
                break;
            }
            case -1:
            {
                //TODO oh, abort
                
            }
            default:
            {
                //TODO parent
                this->cpoProcessManager->ProcBook( liChildNo, ltPHandle );
                this->cpoProcessManager->PersistentToFile();
                break;
            }
        }
    }
    
    this->HandleShutdown( this, NULL, NULL );
    this->cpoProcessManager->WaitAll();

    return 0;
}

void AIModuleParent::Close()
{
    //Close connection server
    AIModuleConnServer::Instance()->Close();
    
    //Close server
    TBase::Close();
    
    while( true )  
    {//Close external connection
        bool lbIsExit = true;
        {
            AIBC::AISmartLock loLock( this->coHandleLock );
            for ( size_t liN = 0; liN < AIM_MAX_EXT_CNT; liN++ )
            {
                if ( this->cpoaExtChannel[liN] != NULL )
                {
                    this->cpoaExtChannel[liN]->CloseHandle();
                    lbIsExit = false;
                }
            }
        }
        if ( lbIsExit ) break;
        
        AIBC::AISleepFor( AI_TIME_MSEC * 100 );
    }
}

int AIModuleParent::Accept( AIBC::AIBaseChannel* apoChannel )
{
    //printf( "accept %p\n", apoChannel );
    static_cast<AIServerChannel*>(apoChannel)->SetModule( this );
    return 0;
}

int AIModuleParent::PutChannel( AIServerChannel** apoChannelArray, size_t aiCount, AIServerChannel* apoChannel )
{
    for ( size_t liN = 0; liN < aiCount; liN++ )
    {
        if ( apoChannelArray[liN] == NULL )
        {
            apoChannelArray[liN] = apoChannel;
            return static_cast<int>(liN);
        }
        else if ( apoChannelArray[liN] == apoChannel )
        {
            return static_cast<int>(liN);
        }
    }
    return -1;
}

int AIModuleParent::EraseChannel(  AIServerChannel** apoChannelArray, size_t aiCount, AIServerChannel* apoChannel )
{
    AIServerChannel** lpoHit  = NULL;
    AIServerChannel** lpoLast = apoChannelArray + aiCount;
    
    for ( size_t liN = 0; liN < aiCount; liN++ )
    {
        if ( apoChannelArray[liN] == apoChannel )
        {
            lpoHit = &apoChannelArray[liN];
            break;
        }
        else if ( apoChannelArray[liN] == NULL )
        {
            break;
        }
    }
    
    if ( lpoHit != NULL )
    {
        ::memmove( lpoHit, lpoHit + 1, (lpoLast - lpoHit - 1)*sizeof(AIServerChannel*) );
        *(lpoLast-1) = NULL;
        
        return static_cast<int>(lpoHit - apoChannelArray);
    }
    
    return -1;
}

int AIModuleParent::RequestToChild( const AIBC::AIChunkEx& aoRequest )
{
    size_t liChildCnt = AIModuleConfig::Instance()->GetChildCount();

    for ( size_t liN = 0; liN < liChildCnt; liN++ )
    {
        AIBC::AIBaseChannel* lpoChannel = cpoaChildChannel[liN];
        if ( lpoChannel == NULL )
        {
            continue;
        }
        
        if ( lpoChannel->Request( aoRequest.BasePtr(), aoRequest.GetSize() ) )
        {
            ///Request fail
            continue;
        }
    }
        
    return 0;
}

int AIModuleParent::RequestToChild( const AIBC::AIChunkEx& aoRequest, AIBC::AIChunkEx& aoResponse )
{
    AIBC::AIChunkExOut     loOutput(aoResponse);
    AIBC::AIChunkEx        loChildResp;
    AIModuleProtocol loProtocol;
    size_t           liChildCnt = AIModuleConfig::Instance()->GetChildCount();
    
    for ( size_t liN = 0; liN < liChildCnt; liN++ )
    {
        AIBC::AIBaseChannel* lpoChannel = cpoaChildChannel[liN];
        if ( lpoChannel == NULL )
        {
            continue;
        }
        
        if ( lpoChannel->Request( aoRequest.BasePtr(), aoRequest.GetSize(), loChildResp ) != 0 )
        {
            ///Request fail
            continue;
        }
        loProtocol.Decode( loChildResp.BasePtr(), loChildResp.GetSize() );
        loOutput.PutMem( loProtocol.GetAppendedData().BasePtr(), loProtocol.GetAppendedData().GetSize() );
    }

    return 0;
}

//TODO Handle funtion
int AIModuleParent::HandleChildLogin( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol )
{
    ///TODO HELLI <child process pid>\r\n
    if ( apoProtocol == NULL || apoProtocol->GetParamCnt() <= 0 )
    {
        //TODO ERR
        AIM_ERROR( "[Channel %s:%d] Child login fail, expect <process id>", 
            apoChannel->GetRemoteIp(), apoChannel->GetRemotePort() );
        apoChannel->CloseHandle();
        return -1;
    }

    if ( apoModule->ChildLogin( apoChannel, (AI_PHANDLE)atoi(apoProtocol->GetParam(0)) ) != 0 )
    {
        apoChannel->CloseHandle();
        return -1;
    }
    
    return 0;
}

int AIModuleParent::HandleChildLogout( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol )
{
    ///TODO LOGOUT <child process pid>\r\n
    if ( apoProtocol == NULL || apoProtocol->GetParamCnt() <= 0 )
    {
        return apoModule->ChildLogout( apoChannel, (AI_PHANDLE)0 );;
    }
    else
    {
        return apoModule->ChildLogout( apoChannel, (AI_PHANDLE)atoi(apoProtocol->GetParam(0)) );
    }
}

int AIModuleParent::HandleExtLogin( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol )
{
    AIBC::AIChunkEx loResponse;
    int       liRetCode = 0;
    
    do
    {
        ///TODO HELLO <password>\r\n
        if ( apoProtocol == NULL || apoProtocol->GetParamCnt() <= 0 )
        {
            //TODO ERR
            AIM_ERROR( "[Channel %s:%d] External usr login fail, expect <password>", 
                apoChannel->GetRemoteIp(), apoChannel->GetRemotePort() );
                
            AIModuleProtocol::Encode( loResponse, NULL, "%s permision deny", AIM_COMMAND_LRESPFAIL );
            
            liRetCode = -1;
            
            break;
        }
        
        if ( ( liRetCode = apoModule->ExtLogin( apoChannel, apoProtocol->GetParam(0) ) ) == 0 )
        {
            AIModuleProtocol::Encode( loResponse, NULL, "%s", AIM_COMMAND_LRESPOK );
        }
        else
        {
            AIModuleProtocol::Encode( loResponse, NULL, "%s permision deny", AIM_COMMAND_LRESPFAIL );
        }
    }
    while(false);
    
    if ( apoChannel->Response( loResponse.BasePtr(), loResponse.GetSize() ) != 0 )
    {
        ///TODO ERR
        AIM_ERROR( "[Channel %s:%d] External user Login response fail, channel exception", 
            apoChannel->GetRemoteIp(), apoChannel->GetRemotePort() );
        liRetCode = -1;
    }
    
    if ( liRetCode != 0 ) apoChannel->CloseHandle();
    
    return liRetCode;
}

int AIModuleParent::HandleExtLogout( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol )
{
    AIM_INFO( "[Channel %s:%d] External usr logout, channel closed", 
        apoChannel->GetRemoteIp(), apoChannel->GetRemotePort() );
    return apoModule->ExtLogout( apoChannel );
}

int AIModuleParent::HandleExtTrace( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol )
{
    if ( apoModule->PutTraceChannel( apoChannel ) != 0 )
    {
        apoChannel->CloseHandle();
        return -1;
    }
    
    if ( apoModule->HandleRequest( apoModule, apoChannel, apoProtocol ) != 0 )
    {
        apoChannel->CloseHandle();
        return -1;
    }

    return 0;
}

int AIModuleParent::HandleChildTrace( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol )
{   
    ///TODO TRACE <TYPE>\r\n
    if ( apoProtocol == NULL || apoProtocol->GetParamCnt() <= 0 )
    {
        //TODO ERR
        AIM_ERROR( "[Channel %s:%d] Child TRACE data invalid, expect TRACE <TYPE>", 
            apoChannel->GetRemoteIp(), apoChannel->GetRemotePort() );
        return -1;
    }
    
    return apoModule->ResponseTrace( apoProtocol->GetParam(0)/*Trace type*/, apoProtocol->GetAppendedData() );
}

int AIModuleParent::HandleReload( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol )
{
    //TODO variable declare
    AIBC::AIChunkEx loRequest(0);
   
    /// encode request
    AIModuleProtocol::Encode( loRequest, NULL, "%s", AIM_COMMAND_RELOAD );

    apoModule->DoRequest( loRequest );
    
    return 0;
}

int AIModuleParent::HandleShutdown( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol )
{
    //TODO variable declare
    AIBC::AIChunkEx loRequest(0);
   
    /// encode request
    AIModuleProtocol::Encode( loRequest, NULL, "%s", AIM_COMMAND_SHUTDOWN );
    
    apoModule->DoRequest( loRequest );
    
    ///Parent Shutdown here
    AIModuleEnv::Instance()->UpdateShutdown( true );
    
    return 0;
}

int AIModuleParent::HandleRequest( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol )
{
    //TODO variable declare
    AIBC::AIChunkEx loRequest(0);
    
    /// encode request
    AIModuleProtocol::Encode( loRequest, NULL, "%s", apoProtocol->GetCommandLine() );
    
    return apoModule->DoRequest( loRequest );
}

int AIModuleParent::HandleRequestForResponse( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol )
{
    //TODO variable declare
    AIBC::AIChunkEx    loRequest(0);
    AIBC::AIChunkEx    loResponse(0);
    AIBC::AIChunkEx    loSocketInfo(0);
    AIBC::AIChunkExOut loOutput(loSocketInfo);
   
    /// encode request
    AIModuleProtocol::Encode( loRequest, NULL, "%s", apoProtocol->GetCommandLine() );
    
    char const* const   lsBEGIN = "<ModuleList>\r\n";
    char const* const   lsEND   = "</ModuleList>\r\n";

    loOutput.PutMem( lsBEGIN, strlen(lsBEGIN) );
    
    apoModule->DoRequest( loRequest, loSocketInfo );
    
    loOutput.PutMem( lsEND, strlen(lsEND) );
    
    AIModuleProtocol::Encode( loResponse, &loSocketInfo, "%s", apoProtocol->GetCommand() );
    
    if ( apoChannel->Response( loResponse.BasePtr(), loResponse.GetSize() ) != 0 )
    {
        ///TODO ERR
        AIM_ERROR( "[Channel %s:%d] GetSocket response fail, channel exception", 
            apoChannel->GetRemoteIp(), apoChannel->GetRemotePort() );
        apoChannel->CloseHandle();
        return -1;
    }
    
    return 0;
}

///TODO for signal
void AIModuleParent::HandleSignalShutdown( int aiSignal )
{
    ::signal( aiSignal, HandleSignalShutdown );
        
    AIModuleEnv::Instance()->UpdateShutdown( true );
}

void AIModuleParent::HandleSignalReload( int aiSignal )
{
    ::signal( aiSignal, HandleSignalReload );
    
    AIModuleEnv::Instance()->UpdateReload( true );
}
////

////
int AIModuleParent::ChildLogin( AIServerChannel* apoChannel, AI_PHANDLE atPHandle )
{
    if ( this->cpoProcessManager->Find(atPHandle) < 0 )
    {
        AIM_ERROR( "[Channel %s:%d] Child [%d] login fail, unacceptable child", 
            apoChannel->GetRemoteIp(), apoChannel->GetRemotePort(), atPHandle );
        return -1;
    }
    
    {///Lock scope
        AIBC::AISmartLock loLock( this->coHandleLock );
        if ( this->PutChannel( this->cpoaChildChannel, AIM_MAX_CHILD_CNT, apoChannel ) < 0 )
        {
            AIM_ERROR( "[Channel %s:%d] Child [%d] login fail, maybe channel slot full", 
                apoChannel->GetRemoteIp(), apoChannel->GetRemotePort(), atPHandle );
            return -1;
        }
        
        ///Child login
        cpoProcessManager->ProcLogin( atPHandle );
    }
    
    AIM_INFO( "[Channel %s:%d] Child [%d] login success", 
                apoChannel->GetRemoteIp(), apoChannel->GetRemotePort(), atPHandle );
    return 0;
}

int AIModuleParent::ChildLogout( AIServerChannel* apoChannel, AI_PHANDLE atPHandle )
{
    //Ignore retcode
    {///Lock scope
        AIBC::AISmartLock loLock( this->coHandleLock );
        if ( this->EraseChannel( this->cpoaChildChannel, AIM_MAX_CHILD_CNT, apoChannel ) == -1
            && !IS_VALID_PHANDLE( atPHandle ) )
        {
            return 0;
        }
        
        if ( IS_VALID_PHANDLE( atPHandle ) )
        {
            AIM_INFO( "[Channel %s:%d] Child logout, child process exited", 
                apoChannel->GetRemoteIp(), apoChannel->GetRemotePort() );
            this->cpoProcessManager->ProcLogout( atPHandle );
        }
        else
        {
            AIM_INFO( "[Channel %s:%d] Child [%d] logout, maybe channel exception", 
                apoChannel->GetRemoteIp(), apoChannel->GetRemotePort(), atPHandle );
        }
    }
    
    return 0;
}

int AIModuleParent::ExtLogin( AIServerChannel* apoChannel, const char* apcPassword )
{
    if ( strcmp( apcPassword, AIModuleConfig::Instance()->GetListenPassword() ) != 0 )
    {
        ///Permision deny
        AIM_ERROR( "[Channel %s:%d] External usr login fail, permision deny", 
            apoChannel->GetRemoteIp(), apoChannel->GetRemotePort() );
        return -1;
    }
    
    
    {///Lock scope
        AIBC::AISmartLock loLock( this->coHandleLock );
        if ( this->PutChannel( this->cpoaExtChannel, AIM_MAX_EXT_CNT, apoChannel ) < 0 )
        {
            AIM_ERROR( "[Channel %s:%d] External usr login fail, channel slot full", 
                apoChannel->GetRemoteIp(), apoChannel->GetRemotePort() );
            return -1;
        }
    }
    
    AIM_INFO( "[Channel %s:%d] External usr login success", 
                apoChannel->GetRemoteIp(), apoChannel->GetRemotePort() );
                
    return 0;
}

int AIModuleParent::ExtLogout( AIServerChannel* apoChannel )
{
    //Ignore retcode
    {///Lock scope

        AIBC::AISmartLock loLock( this->coHandleLock );
        this->EraseChannel( this->cpoaExtChannel, AIM_MAX_EXT_CNT, apoChannel );
        if ( this->EraseChannel( this->cpoaTraceChannel, AIM_MAX_EXT_CNT, apoChannel ) != -1 
            && this->cpoaTraceChannel[0] == NULL )
        {
            ///TRACE channel exit
            AIBC::AIChunkEx loRequest;
            AIModuleProtocol::Encode( loRequest, NULL, "%s", AIM_COMMAND_TRACE );
            this->RequestToChild(loRequest);
        }
    }
    
    return 0;
}

int AIModuleParent::PutTraceChannel( AIServerChannel* apoChannel )
{
    {///Lock scope
        AIBC::AISmartLock loLock( this->coHandleLock );
        if ( this->PutChannel( this->cpoaTraceChannel, AIM_MAX_EXT_CNT, apoChannel ) < 0 )
        {
            ///TODO ERR
            AIM_ERROR( "[Channel %s:%d] External usr trace fail, trace channel slot full", 
                apoChannel->GetRemoteIp(), apoChannel->GetRemotePort() );
            return -1;
        }
    }
    
    return 0;
}

int AIModuleParent::ResponseTrace( const char* apcType, const AIBC::AIChunkEx& aoBuffer )
{
    AIBC::AIChunkEx loResponse(0);
    
    AIModuleProtocol::Encode( loResponse, &aoBuffer, "%s %s", AIM_COMMAND_TRACE, apcType );
        
    {///Lock scope
        AIBC::AISmartLock loLock( this->coHandleLock );
        for ( AIServerChannel** lpoChannel = this->cpoaTraceChannel; *lpoChannel != NULL; lpoChannel++ )
        {
            (*lpoChannel)->Response( loResponse.BasePtr(), loResponse.GetSize() );
        }
    }
    
    return 0;
}

int AIModuleParent::DoRequest( const AIBC::AIChunkEx& aoRequest )
{
    AIBC::AISmartLock loLock( this->coHandleLock );
    
    return this->RequestToChild( aoRequest );
}

int AIModuleParent::DoRequest( const AIBC::AIChunkEx& aoRequest, AIBC::AIChunkEx& aoResponse )
{
    AIBC::AISmartLock loLock( this->coHandleLock );
    
    return this->RequestToChild( aoRequest, aoResponse );
}

///////////////////////////////////////// *Child* ////////////////////////////////////////
// ClientChannel
AIClientChannel::AIClientChannel():cpoModule(NULL)
{
}

void AIClientChannel::SetModule( AIModuleChild* apModule )
{
    this->cpoModule = apModule;
}

int AIClientChannel::Input( const char* apcData, size_t aiSize )
{
    //TODO command from server
    assert( this->cpoModule != NULL );
    
    //TODO External conntion command
    AIModuleProtocol loProtocol;
    
    if ( loProtocol.Decode(apcData, aiSize) != 0 )
    {
        // ERR invalid protocol
        AIM_ERROR( "%s", "Client channel recv invalid protocol request, ignore it here" );
        return -1;
    }
    
    AIModuleChild::THandleFunc lpfHookFunc = 
        (AIModuleChild::THandleFunc)AIModuleEnv::Instance()->FindCommandHook( loProtocol.GetCommand() );
    if ( lpfHookFunc == NULL )
    {
        AIM_ERROR( "Child channel recv invalid command [%s]", loProtocol.GetCommand() );
        return -1;
    }
    
    AIM_INFO( "Client channel recv command [%s] and do it now", loProtocol.GetCommand() );
    
    return (*lpfHookFunc)( this->cpoModule, this, &loProtocol );
}

int AIClientChannel::Error( AI_SYNCHANDLE atHandle, int aiErrno )
{
    this->CloseHandle();
    return 0;
}

int AIClientChannel::Exit()
{   
    if ( !AIModuleEnv::Instance()->IsShutdown() )
    {
        AIModuleChild::HandleReRegister( this->cpoModule );
    }
    
    return 0;
}

///////////////////////////////////// *ModuleChild* //////////////////////////////////
AIModuleChild::AIModuleChild() : ctTraceServerThread(0)
{
    ::setsid( );
    
    ::signal( SIGALRM, SIG_IGN );
    ::signal( SIGPIPE, SIG_IGN );
    ::signal( SIGUSR2, SIG_IGN );
    ::signal( SIGHUP,  SIG_IGN );
    ::signal( SIGQUIT, SIG_IGN );
    ::signal( SIGCHLD, SIG_IGN );
    
    ::signal( SIGUSR1, SIG_IGN );
    ::signal( SIGINT,  SIG_IGN );
    ::signal( SIGTERM, SIG_IGN );
        
    ///TODO register hook function
    AIModuleEnv::Instance()->ClearHook();
    AIModuleEnv::Instance()->RegisterCommandHook( "SHUTDOWN",  (void*)HandleShutdown );
    AIModuleEnv::Instance()->RegisterCommandHook( "RELOAD",    (void*)HandleReload );
    AIModuleEnv::Instance()->RegisterCommandHook( "GETSOCKET", (void*)HandleGetSocket );
    AIModuleEnv::Instance()->RegisterCommandHook( "TRACE",     (void*)HandleTrace );
    ///Udf hook function
    if ( __gfAIModuleChildHookRegFunc != NULL )
    {
        (*__gfAIModuleChildHookRegFunc)( AIModuleEnv::Instance() );
    }
}
AIModuleChild::~AIModuleChild()
{
    this->Close();
}

AIModuleChild* AIModuleChild::Instance()
{
    static AIModuleChild* slpoModule = NULL;
    if ( slpoModule == NULL ) AI_NEW_ASSERT( slpoModule, AIModuleChild );
    return slpoModule;
}
    
int AIModuleChild::Run( const AIBC::AIArgument& aoArgument, AIModule* apoModule )
{
    //TODO variable declare here
    int liRetCode = 0;
    AIModuleConfig* lpoConfig = AIModuleConfig::Instance();
    
    AIModuleEnv::Instance()->UpdateShutdown( false );

    do
    {
        if ( this->RegisterTo( lpoConfig->GetListenIpAddr(), lpoConfig->GetListenPort() ) != 0 )
        {
            liRetCode = -1;
            break;
        }
        
        ///Start trace server thread
        if ( ::pthread_create( &ctTraceServerThread, NULL, TraceServerThread, this ) != 0 )
        {
            AIM_ERROR( "Child create traceserver thread fail - %s", ::strerror(errno) );
            liRetCode = -1;
            break;
        }
        
        ///Start dynamic log level server thread
        AI_THANDLE ltDynLogLevelServerThread;
        if ( ::pthread_create( &ltDynLogLevelServerThread, NULL, DynLogLevelServerThread, this ) != 0 )
        {
            AIM_ERROR( "Child dynamic log level server thread fail - %s", ::strerror(errno) );
            liRetCode = -1;
            break;
        }
        
        liRetCode = apoModule->Main( aoArgument.Argc(), aoArgument.Argv() );
    }
    while(false);
    
    AIModuleEnv::Instance()->UpdateShutdown( true );
    
    ///Recycle server thread
    ::pthread_join( ctTraceServerThread, NULL );

    return liRetCode;
}

void AIModuleChild::Close()
{
    AIBC::AIChunkEx loRequest;
    
    AIModuleProtocol::Encode( loRequest, NULL, "%s %d", AIM_COMMAND_LOGOUT, ::getpid() );
    this->RequestToParent( loRequest );
    
    this->coChannel.Close();
    
    //recycle all listen port
    AIModuleConnServer::Instance()->Close();
}

int AIModuleChild::RegisterTo( const char* apcServerIp, int aiPort )
{
    int       liRetCode = 0;
    AIBC::AIChunkEx loRequest;

    AIBC::AISmartLock loLock(coHandleLock);

    this->coChannel.Close();
    this->coChannel.SetModule( this );
    
    //TODO connecte to admin
    if ( ( liRetCode = this->coClient.Connect( apcServerIp, aiPort, this->coChannel ) ) != 0 )
    {
        //TODO error here
        AIM_ERROR( "Child connecte to [%s:%d] fail, SYNCCODE:%d", apcServerIp, aiPort, liRetCode );
        return -1;
    }

    //TODO login here
    AIModuleProtocol::Encode( loRequest, NULL, "%s %d", AIM_COMMAND_HELLI, ::getpid() );
    if ( ( liRetCode = this->coChannel.Request( loRequest.BasePtr(), loRequest.GetSize() ) ) != 0 )
    {
        //TODO error here
        AIM_ERROR( "Child send register request to [%s:%d] fail, SYNCCODE:%d", 
            apcServerIp, aiPort, liRetCode );
        this->coChannel.CloseHandle();
        return -1;
    }
    
    return 0;
}

int AIModuleChild::RequestToParent( AIBC::AIChunkEx& aoRequest )
{
    AIBC::AISmartLock loLock(coHandleLock);
    
    return this->coChannel.Request( aoRequest.BasePtr(), aoRequest.GetSize() );
}

void* AIModuleChild::TraceServerThread( void* apvParam )
{
    AIModuleChild* lpoModule = static_cast<AIModuleChild*>(apvParam);
    AIBC::AIChunkEx      loRequest;
    AIBC::AIChunkEx      loTraceInfo;
    
    while( !AIModuleEnv::Instance()->IsShutdown() )
    {
        AIBC::AISmartDataPtr loDataPtr = AIBC::AIPacketFilterGetData();
        if ( loDataPtr == NULL )
        {
            AIBC::AISleepFor( AI_TIME_SEC * 2 );
            continue;
        }
        
        loTraceInfo.Resize( loDataPtr.GetSize() );
        ::memcpy( loTraceInfo.BasePtr(), loDataPtr.GetData(), loDataPtr.GetSize() );
        AIModuleProtocol::Encode( loRequest, &loTraceInfo, "%s %s", AIM_COMMAND_TRACE, loDataPtr.GetType() );
        lpoModule->RequestToParent( loRequest );
    }
    
    return NULL;
}

void* AIModuleChild::DynLogLevelServerThread( void* apvParam )
{
    char lsFileName[AIM_MAX_PATH_LEN];

    pthread_detach(pthread_self());
    
    // Action delay 60 seconds
    AIBC::AISleepFor( AI_TIME_SEC * 60 );

    ::snprintf( lsFileName, sizeof(lsFileName), "%s/%s", 
        AIModuleEnv::Instance()->GetRootDirectory(), 
        AIModuleEnv::Instance()->GetProcessName() );
    
    while( true )
    {
        AIBC::AIDynChangeLOGLevel( lsFileName );
        AIBC::AISleepFor( AI_TIME_SEC * 5 );
    }
    
    return NULL;
}

////
int AIModuleChild::HandleGetSocket( AIModuleChild* apoModule, AIClientChannel* apoChannel, const AIModuleProtocol* apoProtocol )
{
    //TODO variable declare
    int       liRetCode = 0;
    AIBC::AIChunkEx loSockInfo(0);
    AIBC::AIChunkEx loBuffer(0);
    
    //TODO GETSOCKET
    AIBC::clsBaseSocket::GetAllConnectionInfo(&loSockInfo);
    
    /// encode request
    AIModuleProtocol::Encode( loBuffer, &loSockInfo, "%s", AIM_COMMAND_GETSOCKET );

    if ( ( liRetCode = apoChannel->Response( loBuffer.BasePtr(), loBuffer.GetSize() ) ) != 0 )
    {
        ///TODO error here
        AIM_ERROR( "Child response socket info to [%s:%d] fail, SYNCCODE:%d", 
            apoChannel->GetRemoteIp(), apoChannel->GetRemotePort(), liRetCode );
        return -1;
    }
    
    return 0;
}

int AIModuleChild::HandleTrace( AIModuleChild* apoModule, AIClientChannel* apoChannel, const AIModuleProtocol* apoProtocol )
{
    AIM_INFO( "Child update filter for trace now ...,[exp=%s]", apoProtocol->GetParam(0) );
    //TODO TRACE
    AIBC::AIPacketFilterUpdate( apoProtocol->GetParam(0) );

    return 0;
}

int AIModuleChild::HandleReload( AIModuleChild* apoModule, AIClientChannel* apoChannel, const AIModuleProtocol* apoProtocol )
{
    //TODO variable declare
    AIModuleEnv::Instance()->UpdateReload( true );
    
    ///compatible aimodule last version
    if( gpfReLoad )  
    {
        int liRetCode = gpfReLoad( );
        AIM_INFO( "Execute RELOAD ... %d", liRetCode );
                
        AIModuleEnv::Instance()->UpdateReload( false );
    }
    ///
    
    return 0;
}

int AIModuleChild::HandleShutdown( AIModuleChild* apoModule, AIClientChannel* apoChannel, const AIModuleProtocol* apoProtocol )
{
    //TODO variable declare
    AIModuleEnv::Instance()->UpdateShutdown( true );
    AIModuleConnServer::Instance()->Close();
    return 0;
}

int AIModuleChild::HandleReRegister( AIModuleChild* apoModule )
{
    //TODO re register
    AIModuleConfig* lpoConfig = AIModuleConfig::Instance();
    if ( apoModule->RegisterTo( lpoConfig->GetListenIpAddr(), lpoConfig->GetListenPort() ) != 0 )
    {
        ///Fail Shutdown it
        AIM_ERROR( "Child re register to [%s:%d] fail, Shutdown now...",
            lpoConfig->GetListenIpAddr(), lpoConfig->GetListenPort() );
            
        AIModuleEnv::Instance()->UpdateShutdown( true );
        
        return -1;
    }
    
    return 0;
}

///////////////////////////////////// *AIModule* //////////////////////////////////
int  AIModule::Initialize()
{
    return 0;
}

void AIModule::Finally()
{
}

int AIModule::Run( int argc, char* argv[], const char* apcPortList )
{
    // Argument
    AIBC::AIArgument loArgument( argc, argv );
    __gfArgumentOptionRegFunc(loArgument);
    
    //TODO Parser command argument
    if ( loArgument.Parser() != 0 || loArgument.IsHasOpt( 'v' ) )
    {
        /// Parser argument exception or do option -v
        printf( "Usage : %s\n", loArgument.GetProcessName() );
        printf( " [stop|kill|reload] : command option\n" );
        loArgument.PrintOption();
        
        int liIdx = 0;
        printf( "\nModule version detail:\n" );
		for( AIModuleEnv::TVersionHandle lpoHandle = AIModuleEnv::Instance()->FirstVersion(); 
		     lpoHandle != NULL ; lpoHandle = lpoHandle->Next() )
		{
			fprintf( stderr, "%2d: %s - %s - %s\n", liIdx++, lpoHandle->csModuleName, 
				lpoHandle->csVersion, lpoHandle->csTime );
		}
		
        return 0;
    }
    
    {
        // Load pid from file
        AIProcessManager loProcessMgr( loArgument.GetProcessName() );
        loProcessMgr.LoadFromFile();
        
        // Preprocess command
        if ( loArgument.IsHasOpt( "stop" ) )
        {
            return loProcessMgr.StopAll();
        }
        else if ( loArgument.IsHasOpt( "kill" ) )
        {
            return loProcessMgr.KillAll();
        }
        else if ( loArgument.IsHasOpt( "reload" ) )
        {
            return loProcessMgr.ReloadAll();
        }
        
        //TODO Check current process
        if ( loProcessMgr.GetCount() > 0 && AIProcessManager::IsProcessExisted(loProcessMgr[0]) )
        {
            // Process existed
            fprintf( stderr, "ERROR:  %s Alive, cann't restart now\n", loArgument.GetProcessName() );
            return 0;
        }
    }
    
    ///compatible aimodule last version
    AIBC::StringCopy( AIBC::gsGlobalProgName, loArgument.GetProcessName(), sizeof(AIBC::gsGlobalProgName) );
    AIBC::GetTimeString( AIBC::gsGlobalStartTime, 0, 1 );
    AIBC::giGlobalLogLevel = loArgument.GetIntValue( 'm', 5 );
    ///

    if ( AIModuleLog::Instance()->Open( "monitor.log" ) != 0 )
    {
        // Process existed
        fprintf( stderr, "ERROR:  Open log file monitor.log fail - %s\n", ::strerror(errno) );
        return -1;
    }

    if ( loArgument.IsHasOpt('d') )
    {
        //TODO If daemon
        AIModuleEnv::RunByDaemon();
    }
    
    ///Initialize module env
    AIModuleEnv::Instance()->Initialize( loArgument.GetProcessName() );
    AIModuleEnv::Instance()->SetRootDirectory();
    AIModuleEnv::Instance()->SetHomeDirectory();
    
    ///Initialize process hook
    if ( this->Initialize() != 0 )
    {
        fprintf( stderr, "ERROR:  Initialize hook function fail\n" );
        return -1;
    }
    
    AIM_INFO( "%s", "======================================================" );
    AIM_INFO( "Start to run parent [%d]...", ::getpid() );
        AIModuleParent::Instance()->Run( loArgument, this, apcPortList );
        AIModuleParent::Instance()->Close();
    AIM_INFO( "Parent [%d] exit now...", ::getpid() );
    AIM_INFO( "%s", "======================================================" );
    
    ///Finally process hook
    this->Finally();
    
    return 0;
}

///AISimpleModule
int AISimpleModule::Run( int argc, char* argv[], const char* apcPortList )
{
    // Argument
    AIBC::AIArgument loArgument( argc, argv );
    __gfArgumentOptionRegFunc(loArgument);
    
    //TODO Parser command argument
    if ( loArgument.Parser() != 0 || loArgument.IsHasOpt( 'v' ) )
    {
        /// Parser argument exception or do option -v
        loArgument.PrintUsage();
        return 0;
    }

    ///compatible aimodule last version
    AIBC::StringCopy( AIBC::gsGlobalProgName, loArgument.GetProcessName(), sizeof(AIBC::gsGlobalProgName) );
    AIBC::GetTimeString( AIBC::gsGlobalStartTime, 0, 1 );
    ///
    
    if ( AIModuleLog::Instance()->Open( "monitor.log" ) != 0 )
    {
        // Process existed
        fprintf( stderr, "ERROR:  Open log file monitor.log fail - %s\n", ::strerror(errno) );
        return -1;
    }

    if ( loArgument.IsHasOpt('d') )
    {
        //TODO If daemon
        AIModuleEnv::RunByDaemon();
    }
    
    AIModuleEnv::Instance()->Initialize( loArgument.GetProcessName() );
    AIModuleEnv::Instance()->SetRootDirectory();
    AIModuleEnv::Instance()->SetHomeDirectory();
    
    ///Initialize process hook
    if ( this->Initialize() != 0 )
    {
        fprintf( stderr, "ERROR:  Initialize hook function fail\n" );
        return -1;
    }
    
    AIM_INFO( "%s", "======================================================" );
    AIM_INFO( "Start to run process [%d]...", ::getpid() );
        AIModuleChild::Instance()->Run( loArgument, this );
        AIModuleChild::Instance()->Close();
    AIM_INFO( "Process [%d] exit now...", ::getpid() );
    AIM_INFO( "%s", "======================================================" );
    
    ///Finally process hook
    this->Finally();
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////
void DefaultArgumentOption( AIBC::AIArgument& aoArgument )
{
    aoArgument.AddOption( 'd', 0, "daemon", "Run by daemon environment" );
    aoArgument.AddOption( 'm', 1, "level",  "log level" );
    aoArgument.AddOption( 'v', 0, "help",   "help" );
}

//////////////////////////////////////// global function ///////////////////////////////
///AIMoudleEnv
bool IsShutdown()
{
    return AIModuleEnv::Instance()->IsShutdown();
}

void UpdateShutdown( bool aiFlag )
{
    AIModuleEnv::Instance()->UpdateShutdown( aiFlag );
}

bool IsReload()
{
    return AIModuleEnv::Instance()->IsReload();
}

void UpdateReload( bool aiFlag )
{
    AIModuleEnv::Instance()->UpdateReload(aiFlag);
}

void SetReLoadHook( TReloadFunc apfFunc )
{
    gpfReLoad = apfFunc;
}

const char* GetProcessName()
{
    return AIModuleEnv::Instance()->GetProcessName();
}

const char* GetRootDir()
{
    return AIModuleEnv::Instance()->GetRootDirectory();
}

const char* GetHomeDir()
{
    return AIModuleEnv::Instance()->GetHomeDirectory();
}

///AIModuleConfig
const char* GetModuleID()
{
    return AIModuleConfig::Instance()->GetModuleID();
}

int GetChildCount()
{
    return AIModuleConfig::Instance()->GetChildCount();
}

///AIModuleConnServer
int AcceptNewConn( const char *apcListenName, char *apcIpAddr, int *apiPort )
{
    return AIModuleConnServer::Instance()->AcceptNewConn( apcListenName, apcIpAddr, apiPort );
}
////////////

};

///start namespace
AIBC_NAMESPACE_START

///compatible aimodule last version
int SetReLoad( AIM::TReloadFunc apfFunc )
{
    AIM::gpfReLoad = apfFunc;
    return 0;
}

int GetListenSocket( const char *apcListenName )
{
    return AIM::AIModuleConnServer::Instance()->GetListenSocket( apcListenName );
}

int GetNewConnection( const char *apcListenName, char *apcIpAddr, int *apiPort )
{
    return AIM::AIModuleConnServer::Instance()->AcceptNewConn( apcListenName, apcIpAddr, apiPort );
}
///

///end namespace
AIBC_NAMESPACE_END
