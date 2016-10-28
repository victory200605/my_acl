
#include "acl/SockStream.h"
#include "acl/FileStream.h"
#include "acl/DateTime.h"
#include "acl/Dir.h"
#include "acl/StrAlgo.h"
#include "acl/Poll.h"
#include "acl/IniConfig.h"
#include "aaf/LogService.h"

AAF_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////////////////////////
class CLogHandlerFactory
{
public:
    CLogHandler* CreateByType( char const* apcType )
    {
        CLogHandler* lpoHandler = NULL;

        if (apl_strcmp(apcType, "file") == 0)
        {
            ACL_NEW_ASSERT(lpoHandler, CLogFileHandler);
        }
        else if (apl_strcmp(apcType, "terminal") == 0)
        {
            ACL_NEW_ASSERT(lpoHandler, CLogTerminalHandler);
        }

        return lpoHandler;
    }

    void Destroy( CLogHandler* apoHandler )
    {
        apoHandler->Destroy();
        ACL_DELETE(apoHandler);
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
CLogHandler::CLogHandler(void)
    : muBackupSize(1024*1024*5)
    , moBackupTimeInterval(900)
    , moBackupDir("LogBak")
    , miTriggerLevel(-1)
{
}

CLogHandler::~CLogHandler(void)
{
}

void CLogHandler::SetBufferSize( apl_size_t auBufferSize )
{
    this->moBuffer.Resize(auBufferSize);
}

acl::CMemoryBlock& CLogHandler::GetBuffer(void)
{
    return this->moBuffer;
}

void CLogHandler::SetBackupSize( apl_size_t auBackupSize )
{
    this->muBackupSize = auBackupSize;
}

apl_size_t CLogHandler::GetBackupSize(void)
{
    return this->muBackupSize;
}

void CLogHandler::SetBackupTimeInterval( acl::CTimeValue const& aoInterval )
{
    this->moBackupTimeInterval = aoInterval;
}

acl::CTimeValue const& CLogHandler::GetBackupTimeInterval(void)
{
    return this->moBackupTimeInterval;
}

void CLogHandler::SetBackupDir( char const* apcBackupDir )
{
    this->moBackupDir = apcBackupDir;
}

char const* CLogHandler::GetBackupDir(void)
{
    return this->moBackupDir.c_str();
}

void CLogHandler::SetTriggerLevel( apl_int_t aiLevel )
{
    this->miTriggerLevel = aiLevel;
}

apl_int_t CLogHandler::GetTriggerLevel(void)
{
    return this->miTriggerLevel;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLogFileHandler::CLogFileHandler(void)
    : mpoService(NULL)
    , miTimerID(-1)
{
}

CLogFileHandler::~CLogFileHandler(void)
{
    if (this->miTimerID > 0)
    {
        this->mpoService->GetTimerQueueAdapter().Cancel(this->miTimerID, NULL);
    }
}

apl_int_t CLogFileHandler::Init( CLogService* apoService, char const* apcName )
{
    acl::CDir::Make(this->GetBackupDir() );

    this->mpoService = apoService;
    this->moName = apcName;
    
    this->miTimerID = this->mpoService->GetTimerQueueAdapter().Schedule(
        this, 
        NULL, 
        this->GetBackupTimeInterval() );

    return this->moFileLog.Open(apcName);
}

apl_int_t CLogFileHandler::Destroy(void)
{
    this->HandleFlush();
    this->moFileLog.Close();
    return 0;
}

apl_int_t CLogFileHandler::HandleWrite( acl::ELLType aeLevel, char const* apcBuffer, apl_size_t auLen )
{
    do
    {
        if (this->GetTriggerLevel() == -1)
        {
            if (aeLevel > this->mpoService->GetLevel() )
            {
                break;
            }
        }
        else 
        {
            if (this->GetTriggerLevel() != aeLevel)
            {
                break;
            }
        }

        acl::TSmartLock<acl::CLock> loGuard(this->moLock);

        acl::CMemoryBlock& loBuffer = this->GetBuffer();

        while(true)
        {
            if (loBuffer.GetSize() == 0)
            {
                return this->Write(apcBuffer, auLen);
            }
            else if (loBuffer.GetSpace() > auLen)
            {
                apl_memcpy(loBuffer.GetWritePtr(), apcBuffer, auLen);
                loBuffer.SetWritePtr(auLen);
                break;
            }
            else if (loBuffer.GetLength() > 0)
            {
                if (this->FlushAll() != 0)
                {
                    return -1;
                }
            }
            else
            {
                return this->Write(apcBuffer, auLen);
            }
        }
    }
    while(false);

    return 0;
}

apl_int_t CLogFileHandler::HandleTimeout( acl::CTimestamp const& aoCurrentTime,void const* apvAct )
{
    acl::CTimeValue loAfter;

    if (aoCurrentTime.Sec() - this->moLastBackupTime.Sec() >= this->GetBackupTimeInterval().Sec() )
    {
        acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        if( this->moFileLog.GetSize() > 0 )
        {
            this->Backup(aoCurrentTime);
        }

        loAfter = this->GetBackupTimeInterval();
    }
    else
    {
        loAfter = this->GetBackupTimeInterval().Sec() - (aoCurrentTime.Sec() - this->moLastBackupTime.Sec() );
    }

    this->miTimerID = this->mpoService->GetTimerQueueAdapter().Schedule(
        this, 
        NULL, 
        loAfter );

    return 0;
}

apl_int_t CLogFileHandler::HandleFlush(void)
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    this->FlushAll();

    return 0;
}

apl_int_t CLogFileHandler::Write( char const* apcBuffer, apl_size_t auLen )
{
    if (auLen == 0)
    {
        return 0;
    }
    else
    {
        apl_ssize_t liRetsult = this->moFileLog.Write(apcBuffer, auLen);
        if (liRetsult != (apl_ssize_t)auLen)
        {
            return -1;
        }

        if (this->moFileLog.GetSize() >= (apl_ssize_t)this->GetBackupSize() )
        {
            acl::CTimestamp loCurrTime;
            loCurrTime.Update(acl::CTimestamp::PRC_SEC);

            if (loCurrTime.Sec() - this->moLastBackupTime.Sec() > 0)
            {
                this->Backup(loCurrTime);
            }
        }
        
        return 0;
    }
}

apl_int_t CLogFileHandler::Backup( acl::CTimestamp const& aoTimestamp )
{
    char lacTemp[APL_NAME_MAX];
    acl::CDateTime loDate;

    loDate.Update(aoTimestamp);

    apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s[%s]", 
        this->GetBackupDir(),
        this->moName.c_str(), 
        loDate.Format("%Y%m%d%H%M%S") ); 

    this->moLastBackupTime.Update();

    return this->moFileLog.Backup(lacTemp);
}

apl_int_t CLogFileHandler::FlushAll(void)
{
    acl::CMemoryBlock& loBuffer = this->GetBuffer();

    if (this->Write(loBuffer.GetReadPtr(), loBuffer.GetLength() ) != 0)
    {
        return -1;
    }
    
    loBuffer.Reset();

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLogTerminalHandler::~CLogTerminalHandler(void)
{
}

apl_int_t CLogTerminalHandler::Init( CLogService* apoService, char const* apcName )
{
    this->mpoService = apoService;
    return 0;
}

apl_int_t CLogTerminalHandler::Destroy(void)
{
    return 0;
}

apl_int_t CLogTerminalHandler::HandleWrite( acl::ELLType aeLevel, char const* apcBuffer, apl_size_t auLen )
{
    if (this->GetTriggerLevel() == -1
        && aeLevel <= this->mpoService->GetLevel() )
    {
        apl_write(APL_STDOUT_FILENO, apcBuffer, auLen);
    }
    else if (this->GetTriggerLevel() == aeLevel)
    {
        apl_write(APL_STDOUT_FILENO, apcBuffer, auLen);
    }

    return 0;
}

apl_int_t CLogTerminalHandler::HandleTimeout( acl::CTimestamp const& aoCurrentTime,void const* apvAct )
{
    return 0;
}

apl_int_t CLogTerminalHandler::HandleFlush(void)
{
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLogService::CLogService(void)
    : mbIsShutdown(true)
    , moLevelFileName("LogService.level")
    , miLogLevel(4)
    , meStdErrLevel(acl::LOG_LVL_ERROR)
    , moDefaultName("LogService.log")
    , muDefaultBufferSize(0)
    , muDefaultBackupSize(1024*1024*5)
    , moDefaultBackupTimeInterval(900)
    , moDefaultBackupDir("LogBak")
    , miDefaultTriggerType(TRIGGER_SELF)
    , moTimerQueueAdapter(1024)
{
}
        
CLogService::~CLogService(void)
{
    this->Shutdown();
}

apl_int_t CLogService::Startup( char const* apcConfigFileName )
{
    if (apcConfigFileName != NULL)
    {
        acl::CIniConfig loConfig;
        
        if (loConfig.Open(apcConfigFileName) != 0)
        {
            return -1;
        }

        //Load log config
        for(acl::CIniConfig::FieldIterType loField = loConfig.Begin("^Log$"); loField != loConfig.End(); ++loField)
        {
            this->muDefaultBufferSize = loField.GetValue("BufferSize", "5120000").ToInt();
            this->muDefaultBackupSize = loField.GetValue("BackupSize", "5120000").ToInt();
            this->moDefaultBackupTimeInterval = loField.GetValue("BackupTimeInterval", "900").ToInt();
            this->moDefaultBackupDir = loField.GetValue("BackupDir", "LogBak").ToString();
            this->moDefaultName = loField.GetValue("Name", this->moDefaultName.c_str() ).ToString();
            this->moLevelFileName = loField.GetValue("LevelFileName", this->moLevelFileName.c_str() ).ToString();
        }
        
        //Load log config, if has LogService field config and replace the lastest
        for(acl::CIniConfig::FieldIterType loField = loConfig.Begin("^LogService$"); loField != loConfig.End(); ++loField)
        {
            this->muDefaultBufferSize = loField.GetValue("BufferSize", "5120000").ToInt();
            this->muDefaultBackupSize = loField.GetValue("BackupSize", "5120000").ToInt();
            this->moDefaultBackupTimeInterval = loField.GetValue("BackupTimeInterval", "900").ToInt();
            this->moDefaultBackupDir = loField.GetValue("BackupDir", "LogBak").ToString();
            this->moDefaultName = loField.GetValue("Name", this->moDefaultName.c_str() ).ToString();
            this->moLevelFileName = loField.GetValue("LevelFileName", this->moLevelFileName.c_str() ).ToString();
        }
        
        std::string loName;
        std::string loType;
        std::string loBackupDir(this->moDefaultBackupDir);
        apl_size_t  luBufferSize(this->muDefaultBufferSize);
        apl_size_t  luBackupSize(this->muDefaultBackupSize);
        apl_int_t   liTriggerLevel = -1;
        apl_int_t   liTriggerType  = TRIGGER_SELF;
        acl::CTimeValue loBackupTimeInterval(this->moDefaultBackupTimeInterval);

        //Load log handler config
        for(acl::CIniConfig::FieldIterType loField = loConfig.Begin("^LogHandler[0-9]+$"); loField != loConfig.End(); ++loField)
        {
            if (loField.GetValue("Name", NULL).IsEmpty() )
            {
                return -1;
            }

            if (!loField.GetValue("BufferSize", NULL).IsEmpty() )
            {
                luBufferSize = loField.GetValue("BufferSize", NULL).ToInt();
            }

            if(!loField.GetValue("BackupSize", NULL).IsEmpty() )
            {
                luBackupSize = loField.GetValue("BackupSize", NULL).ToInt();
            }

            if (!loField.GetValue("BackupTimeInterval", NULL).IsEmpty() )
            {
                loBackupTimeInterval = loField.GetValue("BackupTimeInterval", NULL).ToInt();
            }

            if (!loField.GetValue("BackupDir", NULL).IsEmpty() )
            {
                loBackupDir = loField.GetValue("BackupDir", NULL).ToString();
            }

            loName = loField.GetValue("Name", this->moDefaultName.c_str() ).ToString();
            loType = loField.GetValue("Type", "file").ToString();
            liTriggerLevel = loField.GetValue("TriggerLevel", "-1").ToInt();
            for( char const* lpcFirst = loField.GetValue("TriggerType", this->moDefaultName.c_str() ).ToString(); *lpcFirst != '\0'; ++lpcFirst)
            {
                switch(*lpcFirst)
                {
                    case 's': ACL_SET_BITS(liTriggerType, TRIGGER_SELF); break;
                    case 'b': ACL_SET_BITS(liTriggerType, TRIGGER_ALL); break;//broadcast
                    case 'a': ACL_SET_BITS(liTriggerType, TRIGGER_ANY); break;
                };
            }
            
            if (this->AddHandler(
                loName.c_str(),
                loType.c_str(),
                liTriggerLevel,
                liTriggerType,
                luBufferSize,
                luBackupSize,
                loBackupTimeInterval,
                loBackupDir.c_str() ) != 0)
            {
                return -1;
            }
        }
    }
    //end

    if (this->moAllHandlers.size() == 0)
    {
        if (this->AddHandler(
            this->moDefaultName.c_str(),
            "file",
            -1,
            TRIGGER_SELF|TRIGGER_ALL,
            this->muDefaultBufferSize,
            this->muDefaultBackupSize,
            this->moDefaultBackupTimeInterval,
            this->moDefaultBackupDir.c_str() ) != 0)
        {
            if (apl_get_errno() == 0)
            {
                apl_errprintf("LogService add handler fail, maybe default file name conflict\n");
            }

            return -1;
        }
    }

    this->mbIsShutdown = false;
    
    //redirect stderr
    if (this->moStdErrPair.Open(APL_AF_UNIX, APL_SOCK_STREAM) != 0)
    {
        return -1;
    }
    
    if (apl_dup2(this->moStdErrPair.GetSecond().GetHandle(), APL_STDERR_FILENO) == -1)
    {
        return -1;
    }
    
    //start server
    if ( this->moThreadManager.Spawn(CLogService::StdErrorSvc, this) != 0 )
    {
        this->mbIsShutdown = true;
        return -1;
    }

    if ( this->moThreadManager.Spawn(CLogService::LogLevelSvc, this) != 0 )
    {
        this->mbIsShutdown = true;
        return -1;
    }

    if ( this->moTimerQueueAdapter.Activate() != 0)
    {
        this->mbIsShutdown = true;
        return -1;
    }
    
    return 0;
}
    
apl_int_t CLogService::Restart(void)
{
    return -1;
}
    
apl_int_t CLogService::Shutdown(void)
{
    if(!this->mbIsShutdown)
    {
        this->mbIsShutdown = true;
        
        this->moStdErrPair.GetFirst().Close();
        this->moStdErrPair.GetSecond().Close();
        
        this->moThreadManager.WaitAll();

        this->moTimerQueueAdapter.Deactivate();
    
        acl::TSmartWLock<acl::CRWLock> loGuard(this->moLock);

        for (HandlerMapType::iterator loIter = this->moHandlers.begin(); loIter != this->moHandlers.end(); ++loIter)
        {
            acl::Instance<CLogHandlerFactory>()->Destroy(loIter->second);
        }

        this->moHandlers.clear();
        this->moAllHandlers.clear();
        this->moAnyHandlers.clear();
    }

    return 0;
}

bool CLogService::IsHandlerExisted( char const* apcName )
{
    acl::TSmartWLock<acl::CRWLock> loGuard(this->moLock);
    
    if (this->moHandlers.find(apcName) != this->moHandlers.end() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

apl_int_t CLogService::AddHandler(
    char const* apcName,
    CLogHandler* apoHandler,
    apl_int_t aiTriggerLevel,
    apl_int_t aiTriggerType,
    apl_size_t auBufferSize,
    apl_size_t auBackupSize,
    acl::CTimeValue const& aoBackupTimeInterval,
    char const* apcBackupDir )
{
    apoHandler->SetBufferSize(auBufferSize);
    apoHandler->SetBackupSize(auBackupSize);
    apoHandler->SetBackupTimeInterval(aoBackupTimeInterval);
    apoHandler->SetBackupDir(apcBackupDir);
    apoHandler->SetTriggerLevel(aiTriggerLevel);

    acl::TSmartWLock<acl::CRWLock> loGuard(this->moLock);
    
    if (this->moHandlers.find(apcName) != this->moHandlers.end() )
    {
        return -1;
    }
    
    if (apoHandler->Init(this, apcName) != 0)
    {
        return -1;
    }

    this->moHandlers[apcName] = apoHandler;

    if (ACL_BIT_ENABLED(aiTriggerType, TRIGGER_ANY) )
    {
        this->moAnyHandlers.push_back(apoHandler);
        this->moAllHandlers.push_back(apoHandler);
    }
    else if (ACL_BIT_ENABLED(aiTriggerType, TRIGGER_ALL) )
    {
        this->moAllHandlers.push_back(apoHandler);
    }
    
    return 0;
}

apl_int_t CLogService::AddHandler(
    char const* apcName,
    char const* apcType,
    apl_int_t aiTriggerLevel,
    apl_int_t aiTriggerType,
    apl_size_t auBufferSize,
    apl_size_t auBackupSize,
    acl::CTimeValue const& aoBackupTimeInterval,
    char const* apcBackupDir )
{
    CLogHandler* lpoHandler = acl::Instance<CLogHandlerFactory>()->CreateByType(apcType);
    if (lpoHandler == NULL)
    {
        return -1;
    }

    if (this->AddHandler(
        apcName,
        lpoHandler,
        aiTriggerLevel,
        aiTriggerType,
        auBufferSize,
        auBackupSize,
        aoBackupTimeInterval,
        apcBackupDir ) != 0)
    {
        acl::Instance<CLogHandlerFactory>()->Destroy(lpoHandler);
        return -1;
    }

    return 0;
}

void CLogService::RemoveHandler( char const* apcName )
{
    acl::TSmartWLock<acl::CRWLock> loGuard(this->moLock);
    
    HandlerMapType::iterator loIter = this->moHandlers.find(apcName);

    if (loIter == this->moHandlers.end() )
    {
        return;
    }

    for (HandlerListType::iterator loTempIter = this->moAllHandlers.begin(); 
         loTempIter != this->moAllHandlers.end(); ++loTempIter)
    {
        if (*loTempIter == loIter->second)
        {
            this->moAllHandlers.erase(loTempIter);
            break;
        }
    }
    
    for (HandlerListType::iterator loTempIter = this->moAnyHandlers.begin(); 
         loTempIter != this->moAnyHandlers.end(); ++loTempIter)
    {
        if (*loTempIter == loIter->second)
        {
            this->moAnyHandlers.erase(loTempIter);
            break;
        }
    }
}

void CLogService::SetLevelFileName( char const* apcName )
{
    this->moLevelFileName = apcName;
}

void CLogService::SetLevel( apl_int_t aiLevel )
{
    this->miLogLevel = aiLevel;
}

void CLogService::SetStdErrLevel( apl_int_t aiLevel )
{
    switch(aiLevel)
    {
        case acl::LOG_LVL_SYSTEM: this->meStdErrLevel = acl::LOG_LVL_SYSTEM; break;
        case acl::LOG_LVL_ERROR:  this->meStdErrLevel = acl::LOG_LVL_ERROR; break;
        case acl::LOG_LVL_INFO:   this->meStdErrLevel = acl::LOG_LVL_INFO; break;
        case acl::LOG_LVL_WARN:   this->meStdErrLevel = acl::LOG_LVL_WARN; break;
        case acl::LOG_LVL_DEBUG:  this->meStdErrLevel = acl::LOG_LVL_DEBUG; break;
        case acl::LOG_LVL_TRACE:  this->meStdErrLevel = acl::LOG_LVL_TRACE; break;
        case acl::LOG_LVL_ANY:    this->meStdErrLevel = acl::LOG_LVL_ANY; break;
    };
}

acl::ELLType CLogService::GetStdErrLevel(void)
{
    return this->meStdErrLevel;
}

void CLogService::RedirectStdErr( char const* apcName )
{
    this->moStdErrName = apcName;
}

char const* CLogService::GetLevelFileName(void)
{
    return this->moLevelFileName.c_str();
}

apl_int_t CLogService::GetLevel(void)
{
    return this->miLogLevel;
}

void CLogService::SetDefaultName( char const* apcName )
{
    this->moDefaultName = apcName;
}

void CLogService::SetDefaultBufferSize( apl_size_t auBufferSize )
{
    this->muDefaultBufferSize = auBufferSize;
}

void CLogService::SetDefaultBackupSize( apl_size_t auBackupSize )
{
    this->muDefaultBackupSize = auBackupSize;
}

void CLogService::SetDefaultBackupTimeInterval( acl::CTimeValue const& aoBackupTimeInterval )
{
    this->moDefaultBackupTimeInterval = aoBackupTimeInterval;
}

void CLogService::SetDefaultBackupDir( char const* apcBackupDir )
{
    this->moDefaultBackupDir = apcBackupDir;
}

void CLogService::SetDefaultTriggerType( apl_int_t aiTriggerType )
{
    this->miDefaultTriggerType = aiTriggerType;
}

acl::CTimerQueueAdapter& CLogService::GetTimerQueueAdapter(void)
{
    return this->moTimerQueueAdapter;
}

apl_int_t CLogService::Write( char const* apcName, acl::ELLType aeLevel, char const* apcBuffer, apl_size_t auLen )
{
    CLogHandler* lpoHandler = NULL;
    apl_int_t    liRetCode  = -1;

    while(true)
    {
        do
        {
            acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

            if (apcName != NULL)
            {
                HandlerMapType::iterator loIter = this->moHandlers.find(apcName);
                if (loIter != this->moHandlers.end() )
                {
                    lpoHandler = loIter->second;
                    if (lpoHandler->HandleWrite(aeLevel, apcBuffer, auLen) == 0)
                    {
                        liRetCode = 0;
                    }
                }
                else
                {
                    //target handler unexist
                    break;
                }
                
                for (apl_size_t luN = 0; luN < this->moAnyHandlers.size(); luN++)
                {
                    if (this->moAnyHandlers[luN] != lpoHandler)
                    {
                        if (this->moAnyHandlers[luN]->HandleWrite(aeLevel, apcBuffer, auLen) == 0)
                        {
                            liRetCode = 0;
                        }
                    }
                }
            }
            else
            {
                //Broadcast all handlers
                for (apl_size_t luN = 0; luN < this->moAllHandlers.size(); luN++)
                {
                    if (this->moAllHandlers[luN]->HandleWrite(aeLevel, apcBuffer, auLen) == 0)
                    {
                        liRetCode = 0;
                    }
                }
            }

            return liRetCode;
        }
        while(false);
        
        if (this->AddHandler(
            apcName,
            "file",
            -1,
            this->miDefaultTriggerType,
            this->muDefaultBufferSize,
            this->muDefaultBackupSize,
            this->moDefaultBackupTimeInterval,
            this->moDefaultBackupDir.c_str() ) != 0 && !this->IsHandlerExisted(apcName) )
        {
            return -1;
        }
    }
    
    return 0;
}

void* CLogService::StdErrorSvc( void* apvService )
{
    CLogService*  lpoService = static_cast<CLogService*>(apvService);
    acl::CSockStream loPeer;
    acl::CTimeValue  loTimeout(1);
    char lacBuffer[1024] = {0};

    loPeer.SetHandle(lpoService->moStdErrPair.GetFirst().GetHandle() );

    while(!lpoService->mbIsShutdown)
    {
        apl_ssize_t liResult = acl::CPoll::Poll(loPeer.GetHandle(), APL_POLLIN, NULL, loTimeout);
        if (liResult <= 0)
        {
            continue;
        }
        
        liResult = loPeer.RecvUntil(lacBuffer, sizeof(lacBuffer) - 1, "\n", loTimeout);
        if (liResult > 0)
        {
            lacBuffer[liResult] = '\0';
            
            if (lpoService->moStdErrName.length() > 0)
            {
                lpoService->Write(
                    lpoService->moStdErrName.c_str(), 
                    acl::CDefaultFormatter<1024>(lpoService->GetStdErrLevel(), 
                        "%s", lacBuffer ) );
            }
            else
            {
                lpoService->Write(
                    NULL, 
                    acl::CDefaultFormatter<1024>(lpoService->GetStdErrLevel(), 
                        "%s", lacBuffer ) );
            }
        }
    }
    
    return NULL;
}

void* CLogService::LogLevelSvc( void* apvService )
{
    CLogService* lpoService = static_cast<CLogService*>(apvService);
    acl::CFileStream loFile;

    while(!lpoService->mbIsShutdown)
    {
        if ( loFile.Open(lpoService->GetLevelFileName(), APL_O_RDONLY) == 0)
        {
            char lacBuffer[64] = {0};
            
            if ( loFile.ReadLine(lacBuffer, sizeof(lacBuffer) ) > 0)
            {
                char* apcNext = NULL;
                lpoService->SetLevel(apl_strtoi32(lacBuffer, &apcNext, 10) );
                AAF_LOG_INFO("LogService change log level to %"APL_PRIdINT, lpoService->GetLevel() );
                
                if (apl_strtoi32(apcNext, NULL, 10) > 0)
                {
                    lpoService->SetStdErrLevel(apl_strtoi32(apcNext, NULL, 10) );
                    AAF_LOG_INFO("LogService change stderr log level to %"APL_PRIdINT, lpoService->GetStdErrLevel() );
                }
            }
            
            loFile.Close();
            
            acl::CFile::Remove(lpoService->GetLevelFileName());
        }
        
        {
            acl::TSmartRLock<acl::CRWLock> loGuard(lpoService->moLock);
            
            //Broadcast all handlers
            for (std::map<std::string, CLogHandler*>::iterator loIter = lpoService->moHandlers.begin(); 
                 loIter != lpoService->moHandlers.end(); ++loIter)
            {
                loIter->second->HandleFlush();
            }
        }

        apl_sleep(APL_TIME_SEC);
    }
    
    return NULL;
}

AAF_NAMESPACE_END

