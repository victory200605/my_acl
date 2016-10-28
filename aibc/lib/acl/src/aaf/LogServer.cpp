
#include "acl/SockStream.h"
#include "acl/DateTime.h"
#include "acl/Dir.h"
#include "acl/FileStream.h"
#include "acl/StrAlgo.h"
#include "acl/IniConfig.h"
#include "aaf/LogServer.h"

AAF_NAMESPACE_START

//////////////////////////////////////////////////////////////////////////////////////////////
CLogServer::CLogServer(void)
    : mbIsShutdown(true)
    , mpoReactor(NULL)
    , miEvents(0)
    , moBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE) /*5M*/
    , moMaxBakInterval(900)
    , muMaxFileSize(1024*1024*5)
    , miLogLevel(4)
    , meStdErrLevel(acl::LOG_LVL_ERROR)
{
    apl_strncpy( this->macFileName, "LogServer.log", sizeof(this->macFileName) );
    apl_strncpy( this->macLevelFileName, "LogServer.level", sizeof(this->macLevelFileName) );
    apl_strncpy( this->macBackupDir, "LogBak", sizeof(this->macBackupDir) );
    
    this->moLastBakTime.Update(acl::CTimestamp::PRC_USEC);
}
        
CLogServer::~CLogServer(void)
{
    this->Shutdown();
}

apl_int_t CLogServer::Startup( char const* apcConfigFileName )
{
    acl::CIniConfig loConfig;
    
    if (loConfig.Open(apcConfigFileName) != 0)
    {
        return -1;
    }
    
    //Load log config
    this->muMaxFileSize = loConfig.GetValue("Log", "MaxFileSize", "5120000").ToInt();
    this->moMaxBakInterval = loConfig.GetValue("Log", "MaxBakInterval", "900").ToInt();
    apl_strncpy(
        this->macBackupDir,
        loConfig.GetValue("Log", "BakDir", "LogBak").ToString(),
        sizeof(this->macBackupDir) );
    if (!loConfig.GetValue("Log", "FileName").IsEmpty() )
    {
        apl_strncpy( this->macFileName, loConfig.GetValue("Log", "FileName").ToString(), sizeof(this->macFileName) );
    }
    
    if (!loConfig.GetValue("Log", "LevelFileName").IsEmpty() )
    {
        apl_strncpy( 
            this->macLevelFileName,
            loConfig.GetValue("Log", "LevelFileName").ToString(),
            sizeof(this->macLevelFileName) );
    }
    
    this->moBuffer.Resize( loConfig.GetValue("Log", "CacheSize", "5120000").ToInt() );
    //end

    if ( this->moLog.Open(this->macFileName) != 0 )
    {
        return -1;
    }
    
    this->mbIsShutdown = false;
    
    if (this->mpoReactor != NULL)
    {
        //Enable non-block write
        if ( this->moHandlePair.Open(APL_AF_UNIX, APL_SOCK_STREAM) != 0)
        {
            return -1;
        }
    
        this->moHandlePair.GetFirst().Disable(acl::CIOHandle::OPT_NONBLOCK);
        this->moHandlePair.GetSecond().Enable(acl::CIOHandle::OPT_NONBLOCK);
    }
    
    //redirect stderr
    if (this->moStdErrPair.Open(APL_AF_UNIX, APL_SOCK_STREAM) != 0)
    {
        return -1;
    }
    
    if (apl_dup2(this->moStdErrPair.GetSecond().GetHandle(), APL_STDERR_FILENO) == -1)
    {
        return -1;
    }
    
    //register handle
    if (this->moPollee.Initialize(2) != 0)
    {
        return -1;
    }
    
    if ( this->moPollee.AddHandle(this->moHandlePair.GetFirst().GetHandle(), APL_POLLIN) != 0
        || this->moPollee.AddHandle(this->moStdErrPair.GetFirst().GetHandle(), APL_POLLIN) != 0 )
    {
        return -1;
    }
    
    //start server
    if ( this->moThreadManager.Spawn(CLogServer::Svc, this) != 0 )
    {
        this->mbIsShutdown = true;
        return -1;
    }

    if ( this->moThreadManager.Spawn(CLogServer::LogLevelSvc, this) != 0 )
    {
        this->mbIsShutdown = true;
        return -1;
    }
    
    return 0;
}
    
apl_int_t CLogServer::Restart(void)
{
    return -1;
}
    
apl_int_t CLogServer::Shutdown(void)
{
    this->mbIsShutdown = true;
    
    this->moPollee.Close();
    
    this->moHandlePair.GetFirst().Close();
    this->moHandlePair.GetSecond().Close();
    
    this->moStdErrPair.GetFirst().Close();
    this->moStdErrPair.GetSecond().Close();
    
    this->moThreadManager.WaitAll();
    
    return 0;
}

void CLogServer::SetFileName( char const* apcFileName )
{
    apl_strncpy(this->macFileName, apcFileName, sizeof(this->macFileName) );
}

void CLogServer::SetLevelFileName( char const* apcFileName )
{
    apl_strncpy(this->macLevelFileName, apcFileName, sizeof(this->macLevelFileName) );
}

void CLogServer::SetLevel( apl_int_t aiLevel )
{
    this->miLogLevel = aiLevel;
}

void CLogServer::SetStdErrLevel( apl_int_t aiLevel )
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

char const* CLogServer::GetFileName(void)
{
    return this->macFileName;
}

char const* CLogServer::GetLevelFileName(void)
{
    return this->macLevelFileName;
}

apl_int_t CLogServer::GetLevel(void)
{
    return this->miLogLevel;
}

acl::ELLType CLogServer::GetStdErrLevel(void)
{
    return this->meStdErrLevel;
}

void CLogServer::SetReactor( acl::CReactor* apoReactor )
{
    this->mpoReactor = apoReactor;
}

apl_int_t CLogServer::Write( acl::ELLType aeLevel, char const* apcFormat, ... )
{
    va_list    loVaList;
    char       lacBuffer[4096] = {0};
    apl_size_t liRetSize = 0;
    
    if (aeLevel > this->GetLevel() )
    {
        return 0;
    }
    
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    this->CheckBackup();
    
    va_start(loVaList, apcFormat);

    liRetSize = this->moFormat( aeLevel, apcFormat, loVaList, lacBuffer, sizeof(lacBuffer) );

    va_end(loVaList);
    
    return this->moLog.Write(lacBuffer, liRetSize);
}

apl_int_t CLogServer::WriteHex( acl::ELLType aeLevel, char const* apcPtr, apl_size_t auLength )
{
    acl::CHexFormatter loHexFormatter;

    return this->Write( aeLevel, "\n%s", loHexFormatter.Format(apcPtr, auLength) );
}

apl_int_t CLogServer::WriteNonBlock( acl::ELLType aeLevel, char const* apcFormat, ... )
{
    va_list    loVaList;
    char       lacBuffer[4096] = {0};
    apl_size_t liRetSize = 0;
    
    if (aeLevel > this->GetLevel() )
    {
        return 0;
    }
    
    if (this->mpoReactor == NULL)
    {
        return -1;
    }
    
    va_start(loVaList, apcFormat);

    liRetSize = this->moFormat( aeLevel, apcFormat, loVaList, lacBuffer, sizeof(lacBuffer) );

    va_end(loVaList);
    
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    if ( this->moBuffer.GetSpace() < liRetSize )
    {
        return -1;
    }
    else
    {
        if (this->moBuffer.GetLength() > 0 && ACL_BIT_DISABLED(this->miEvents, acl::IEventHandler::WRITE_MASK) )
        {
            if ( this->mpoReactor->RegisterHandler(
                this->moHandlePair.GetSecond().GetHandle(),
                this,
                acl::IEventHandler::WRITE_MASK ) != 0 )
            {
                //LOG HERE: Can only use blocking mode
                AAF_LOG_ERROR(
                    "LogServer register handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
                    apl_strerror(apl_get_errno()),
                    apl_get_errno(),
                    acl::IEventHandler::WRITE_MASK );
            }
            else
            {
                ACL_SET_BITS(this->miEvents, acl::IEventHandler::WRITE_MASK);
            }
        }
        
        apl_memcpy(this->moBuffer.GetWritePtr(), lacBuffer, liRetSize);
        this->moBuffer.SetWritePtr(liRetSize);
    }
    
    return 0;
}

apl_int_t CLogServer::WriteNonBlockHex( acl::ELLType aeLevel, char const* apcPtr, apl_size_t auLength )
{
    acl::CHexFormatter loHexFormatter;

    return this->WriteNonBlock( aeLevel, "\n%s", loHexFormatter.Format(apcPtr, auLength) );
}

apl_int_t CLogServer::HandleOutput( apl_handle_t aiHandle )
{
    apl_ssize_t      liResult = 0;
    acl::CSockStream loPeer;
    
    loPeer.SetHandle(aiHandle);

    if (this->moBuffer.GetLength() == 0)
    {
        this->moBuffer.Reset();

        if ( this->mpoReactor->RemoveHandler(aiHandle, acl::IEventHandler::WRITE_MASK) != 0 )
        {
            AAF_LOG_ERROR_NB(
                "LogServer remove handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                apl_get_errno(),
                acl::IEventHandler::WRITE_MASK );
        }
        else
        {
            ACL_CLR_BITS(this->miEvents, acl::IEventHandler::WRITE_MASK);
        }
        
        return 0;
    }
    
    liResult = loPeer.Send( this->moBuffer.GetReadPtr(), this->moBuffer.GetLength() );
    if (liResult > 0)
    {
        this->moBuffer.SetReadPtr(liResult);
        
        apl_size_t luLength = this->moBuffer.GetLength();
        
        apl_memmove(this->moBuffer.GetBase(), this->moBuffer.GetReadPtr(), luLength);
            
        this->moBuffer.Reset();
        this->moBuffer.SetWritePtr(luLength);
    }
    else
    {
        //LOG HERE: Can only use blocking mode
        AAF_LOG_ERROR(
            "LogServer Sock pair exception, write fail, [result=%"APL_PRIdINT"][errno=%"APL_PRIdINT"]/[size=%"APL_PRIuINT"]",
            liResult,
            apl_get_errno(),
            this->moBuffer.GetLength() );
    }
    
    return 0;
}

void CLogServer::CheckBackup(void)
{
    acl::CTimestamp loCurrTime;
    char            lacBackupName[APL_PATH_MAX] = {0};
    apl_ssize_t     liFileSize;
    
    loCurrTime.Update(acl::CTimestamp::PRC_SEC);

    liFileSize = this->moLog.GetSize();

    if ( (liFileSize > 0)
        && ((loCurrTime.Sec() - this->moLastBakTime.Sec() ) > this->moMaxBakInterval.Sec()
            || ( liFileSize > (apl_ssize_t)this->muMaxFileSize
                && (loCurrTime.Sec() - this->moLastBakTime.Sec() ) > 1 ) ) )
    {
		acl::CDateTime loCurrDateTime;
		loCurrDateTime.Update(loCurrTime);
		
        apl_snprintf(lacBackupName, sizeof(lacBackupName), "%s/%s[%s]", 
            this->macBackupDir, this->macFileName, loCurrDateTime.Format("%Y%m%d%H%M%S") );
        
        this->moLog.Backup(lacBackupName);
        
        moLastBakTime.Update(acl::CTimestamp::PRC_SEC);
    }
}

void* CLogServer::Svc( void* apvServer )
{
    CLogServer*      lpoServer = static_cast<CLogServer*>(apvServer);
    apl_ssize_t      liResult = 0;
    acl::CSockStream loPeer;
    acl::CTimeValue  loTimeout(2);
    char             lacBuffer[4096] = {0};

    acl::CDir::Make(lpoServer->macBackupDir);
        
    while(!lpoServer->mbIsShutdown)
    {
        liResult = lpoServer->moPollee.Poll(loTimeout);
        if (liResult == 0)
        {
            continue;
        }
        else if (liResult < 0)
        {
            AAF_LOG_ERROR(
                "LogServer pollee sock pair exception, [result=%"APL_PRIdINT"][errno=%"APL_PRIdINT"]",
                liResult,
                apl_get_errno() );
                
            apl_sleep(APL_TIME_SEC);
            
            continue;
        }
        else if (lpoServer->mbIsShutdown)
        {
            break;
        }
        
        for ( acl::CPollee::IteratorType loIter = lpoServer->moPollee.Begin();
            loIter != lpoServer->moPollee.End(); ++loIter )
        {
            loPeer.SetHandle(loIter.Handle() );
            
            if (lpoServer->moStdErrPair.GetFirst().GetHandle() == loIter.Handle() )
            {
                liResult = loPeer.RecvUntil(lacBuffer, sizeof(lacBuffer) - 1, "\n", loTimeout);
            }
            else
            {
                liResult = loPeer.Recv(lacBuffer, sizeof(lacBuffer) - 1);
            }
            
            if (liResult > 0)
            {
                acl::TSmartLock<acl::CLock> loGuard(lpoServer->moLock);
                
                if (loIter.Handle() == lpoServer->moStdErrPair.GetFirst().GetHandle() )
                {
                    lacBuffer[liResult] = '\0';
                    lpoServer->moLog.Write(lpoServer->GetStdErrLevel(), "%s", lacBuffer);
                }
                else
                {
                    lpoServer->moLog.Write(lacBuffer, liResult);
                }
                
                lpoServer->CheckBackup();
            }
            else
            {
                //LOG HERE: Can only use blocking mode
                AAF_LOG_ERROR(
                    "LogServer Sock pair exception, recv fail, [result=%"APL_PRIdINT"][errno=%"APL_PRIdINT"]",
                    liResult,
                    apl_get_errno() );
            }
        }
    }
    
    return NULL;
}

void* CLogServer::LogLevelSvc( void* apvServer )
{
    CLogServer* lpoServer = static_cast<CLogServer*>(apvServer);
    acl::CFileStream loFile;

    while(!lpoServer->mbIsShutdown)
    {
        if ( loFile.Open(lpoServer->GetLevelFileName(), APL_O_RDONLY) == 0)
        {
            char lsBuffer[64] = {0};
            
            if ( loFile.ReadLine(lsBuffer, sizeof(lsBuffer) ) > 0)
            {
                char* apcNext = NULL;
                lpoServer->SetLevel( apl_strtoi32(lsBuffer, &apcNext, 10) );
                AAF_LOG_INFO("LogServer change log level to %"APL_PRIdINT, lpoServer->GetLevel() );
                
                if (apl_strtoi32(apcNext, NULL, 10) > 0)
                {
                    lpoServer->SetStdErrLevel(apl_strtoi32(apcNext, NULL, 10) );
                    AAF_LOG_INFO("LogServer change stderr log level to %"APL_PRIdINT, lpoServer->GetStdErrLevel() );
                }
            }
            
            loFile.Close();
            
            acl::CFile::Remove(lpoServer->GetLevelFileName());
        }

        apl_sleep(APL_TIME_SEC);
    }
    
    return NULL;
}

AAF_NAMESPACE_END
