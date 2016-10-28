
#include "LogServer.h"
#include "acl/SockStream.h"
#include "acl/DateTime.h"
#include "acl/Dir.h"

//////////////////////////////////////////////////////////////////////////////////////////////
CLogServer* CLogServer::cpoInstance = NULL;
    
CLogServer* CLogServer::Instance(void)
{
    if (cpoInstance == NULL)
    {
        ACL_NEW_ASSERT(cpoInstance, CLogServer );
    }
    
    return cpoInstance;
}
    
void CLogServer::Release(void)
{
    ACL_DELETE(cpoInstance);
}

CLogServer::CLogServer(void)
    : mbIsClosed(true)
    , mpoReactor(NULL)
    , miEvents(0)
    , moBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE) /*5M*/
    , moMaxLogTime(900)
    , muMaxLogSize(1024*1024*5)
    , miLogLevel(4)
{
    apl_memset( this->macProcessName, 0, sizeof(this->macProcessName) );
    apl_memset( this->macLogFileName, 0, sizeof(this->macLogFileName) );
    apl_strncpy( this->macBackupDir, "LogBak", sizeof(this->macBackupDir) );
}
        
CLogServer::~CLogServer(void)
{
    this->Close();
}
    
apl_int_t CLogServer::Startup( acl::CReactor& aoReactor, char const* apcFileName )
{
    apl_int_t liRetCode = 0;
    
    this->mpoReactor = &aoReactor;

    if ( ( liRetCode = this->moLog.Open(apcFileName) ) != 0 )
    {
        return liRetCode;
    }
    
    if ( ( liRetCode = this->moHandlePair.Open(APL_AF_UNIX, APL_SOCK_STREAM) ) != 0)
    {
        return liRetCode;
    }
    
    this->moHandlePair.GetFirst().Disable(acl::CIOHandle::OPT_NONBLOCK);
    this->moHandlePair.GetSecond().Enable(acl::CIOHandle::OPT_NONBLOCK);
    
    apl_strncpy(this->macLogFileName, apcFileName, sizeof(this->macLogFileName) );
    this->mbIsClosed = false;
    
    if ( ( liRetCode = this->moThreadManager.Spawn(CLogServer::Svc, this) ) != 0 )
    {
        this->mbIsClosed = true;
    }
    
    if ( ( liRetCode = this->moThreadManager.Spawn(CLogServer::LogLevelSvc, this) ) != 0 )
    {
        this->mbIsClosed = true;
    }
    
    return liRetCode;
}
    
void CLogServer::Close(void)
{
    this->mbIsClosed = true;
    
    this->moHandlePair.GetFirst().Close();
    this->moHandlePair.GetSecond().Close();
    
    this->moThreadManager.WaitAll();
}

void CLogServer::SetMaxTime( acl::CTimeValue const& aoTime )
{
    if (aoTime.Nsec() > 0 )
    {
        this->moMaxLogTime = aoTime;
    }
}

void CLogServer::SetMaxLogSize( apl_size_t auSize )
{
    this->muMaxLogSize = auSize;
}

void CLogServer::SetBackupDir( char const* apcPath )
{
    apl_strncpy( this->macBackupDir, apcPath, sizeof(this->macBackupDir) );
}

void CLogServer::SetCacheSize( apl_size_t auSize )
{
    this->moBuffer.Resize(auSize);
    this->moBuffer.Reset();
}

void CLogServer::SetProcessName( char const* apcProcessName )
{
    apl_strncpy(this->macProcessName, apcProcessName, sizeof(this->macProcessName) );
}

void CLogServer::SetLevel( apl_int_t aiLevel )
{
    this->miLogLevel = aiLevel;
}

char const* CLogServer::GetProcessName(void)
{
    return this->macProcessName;
}

apl_int_t CLogServer::GetLevel(void)
{
    return this->miLogLevel;
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
    
    va_start(loVaList, apcFormat);

    liRetSize = this->moFormat( aeLevel, apcFormat, loVaList, lacBuffer, sizeof(lacBuffer) );

    va_end(loVaList);
    
    return this->moLog.Write(lacBuffer, liRetSize);
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
    
    va_start(loVaList, apcFormat);

    liRetSize = this->moFormat( aeLevel, apcFormat, loVaList, lacBuffer, sizeof(lacBuffer) );

    va_end(loVaList);
    
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
                CNTSRV_LOG_ERROR(
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

char* CLogServer::CharToHex( unsigned char acCh, char* apcRet, apl_size_t auSize )
{
	unsigned char lcChLow = acCh & 0x0F;
	unsigned char lcChHigh = acCh >> 4;
	
	if (auSize <= 2)
	{
	    apcRet[0] = '\0';
	}
    else
    {
    	lcChLow  += lcChLow  < 10 ? '0' : ('A' - 10);
    	lcChHigh += lcChHigh < 10 ? '0' : ('A' - 10);
    
    	*(apcRet++) = lcChHigh;
    	*(apcRet++) = lcChLow;
    	*(apcRet)   = '\0';
    }
    
	return apcRet;
}

const char* CLogServer::UStringToHex(
    char const* apcBuff,
    apl_size_t auSize,
    acl::CMemoryBlock& aoOut,
    apl_size_t auColumn,
    apl_size_t auGroup )
{
	apl_size_t luColumn    = auColumn;
	apl_size_t luGroup     = auGroup;
	apl_size_t luDelta     = auSize % ( luColumn * luGroup );
	apl_size_t luRow       = auSize / ( luColumn * luGroup ) + (luDelta? 1 : 0);
	apl_size_t luRowLenght = 9 /*header adder*/ + luColumn * ( luGroup * 2 + 1) + 2 /* del char*/ + luGroup * luColumn /* printable char */ + 1 /*\n*/;
	apl_size_t luNeedSize  = luRowLenght * luRow + 1 /* 1 end */;
	apl_size_t luStep      = luColumn * luGroup;

	aoOut.Resize( luNeedSize );
	aoOut.Reset();
	
	apl_memset( aoOut.GetBase(), ' ', aoOut.GetSize() );

	char*  lpcCurrRow       = aoOut.GetBase();
	unsigned char*  lpcCurr = (unsigned char*)apcBuff;
	apl_size_t luCurrPos    = 0;
	apl_size_t luRowIdx     = 0;
	char*  lpcHeader   = NULL;
	char*  lpcHex      = NULL;
	char*  lpcPrint    = NULL;

	for ( apl_size_t luIt = 0; luIt < luRow; luIt++ )
	{
		lpcHeader = lpcCurrRow;
		lpcHex    = lpcHeader + 9;
		lpcPrint  = lpcHeader + 9 /*header adder*/ + luColumn * ( luGroup * 2 + 1) + 2 /* del char*/;

		apl_snprintf( lpcHeader, 10, "%08X:", luRowIdx );

		for ( apl_size_t luCol = 0; luCol < luColumn; luCol++ )
		{
			*lpcHex++ = ' ';

			for ( apl_size_t luIdx = 0; luIdx < luGroup; luIdx++ )
			{
				if ( luCurrPos < auSize )
				{
					lpcHex = CharToHex( lpcCurr[luCurrPos], lpcHex, 3 );

					*lpcPrint++ = ( apl_isprint(lpcCurr[luCurrPos]) ? lpcCurr[luCurrPos] : '.' );
				}
				else
				{
					lpcHex = CharToHex( 0, lpcHex, 3 );
					*lpcPrint++ = '.';
				}

				luCurrPos++;
			}

			if ( luCurrPos >= auSize )
			{
			    break;
			}
		}
		
		*lpcHex     = ' ';
		*lpcPrint++ = '\n';
		
		lpcCurrRow = lpcPrint;
		luRowIdx   = luRowIdx + luStep;
	}

	*lpcPrint = '\0';

	return aoOut.GetBase();
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
            CNTSRV_LOG_ERROR_NB(
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
        CNTSRV_LOG_ERROR(
            "Sock pair exception, write fail, [result=%"APL_PRIdINT"][errno=%"APL_PRIdINT"]/[size=%"APL_PRIuINT"]",
            liResult,
            apl_get_errno(),
            this->moBuffer.GetLength() );
    }
    
    return 0;
}

void* CLogServer::Svc( void* apvServer )
{
    CLogServer*      lpoServer = static_cast<CLogServer*>(apvServer);
    apl_ssize_t      liResult = 0;
    acl::CSockStream loPeer;
    acl::CTimestamp  loCurrTime;
    acl::CTimestamp  loLastTime;
    char             lacBuffer[4096] = {0};
    char             lacBackupName[APL_PATH_MAX] = {0};
    
    loPeer.SetHandle( lpoServer->moHandlePair.GetFirst().GetHandle() );
    
    acl::CDir::Make(lpoServer->macBackupDir);
        
    loLastTime.Update(acl::CTimestamp::PRC_SEC);
        
    while(!lpoServer->mbIsClosed)
    {
        liResult = loPeer.Recv(lacBuffer, sizeof(lacBuffer));
        if (liResult > 0)
        {
            lpoServer->moLog.Write(lacBuffer, liResult);
            
            loCurrTime.Update(acl::CTimestamp::PRC_SEC);
            
            if ( (loCurrTime.Sec() - loLastTime.Sec() ) > lpoServer->moMaxLogTime.Sec()
                || ( lpoServer->moLog.GetSize() > (apl_ssize_t)lpoServer->muMaxLogSize
                    && (loCurrTime.Sec() - loLastTime.Sec() ) > 1 ) )
            {
				acl::CDateTime loCurrDateTime;
				loCurrDateTime.Update(loCurrTime);
				
                apl_snprintf(lacBackupName, sizeof(lacBackupName), "%s/%s[%s]", 
                    lpoServer->macBackupDir, lpoServer->macLogFileName, loCurrDateTime.Format("%Y%m%d%H%M%S") );
                
                lpoServer->moLog.Backup(lacBackupName);
                
                loLastTime.Update(acl::CTimestamp::PRC_SEC);
            }
        }
        else
        {
            //LOG HERE: Can only use blocking mode
            CNTSRV_LOG_ERROR(
                "Sock pair exception, recv fail, [result=%"APL_PRIdINT"][errno=%"APL_PRIdINT"]",
                liResult,
                apl_get_errno() );
        }
    }
    
    return NULL;
}

void* CLogServer::LogLevelSvc( void* apvServer )
{
    CLogServer* lpoServer = static_cast<CLogServer*>(apvServer);
    FILE*       lpoFile = NULL;
    char        lsFileName[128] = {0};

    apl_snprintf(lsFileName, sizeof(lsFileName), "%s.level", lpoServer->GetProcessName() );
    
    while(!lpoServer->mbIsClosed)
    {
        if ( ( lpoFile = fopen(lsFileName, "r") ) != NULL )
        {
            char lsBuffer[64] = {0};
            
            if ( fgets(lsBuffer, sizeof(lsBuffer), lpoFile ) )
            {
                lpoServer->SetLevel( apl_strtoi32(lsBuffer, NULL, 10) );
                CNTSRV_LOG_INFO("Change log level to %"APL_PRIdINT, lpoServer->GetLevel() );
            }
            
            fclose(lpoFile);
            
            acl::CFile::Remove(lsFileName);
        }

        apl_sleep(APL_TIME_SEC);
    }
    
    return NULL;
}
