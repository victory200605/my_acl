
#include "acl/IniConfig.h"
#include "acl/SockConnector.h"
#include "aaf/TrapService.h"

AAF_NAMESPACE_START

//////////////////////////////////////////////////////////////////////////////////////////////
CTrapService::CTrapService(void)
    : mbIsShutdown(true)
    , mpoReactor(NULL)
    , miEvents(0)
    , moBuffer(1024 * 1024 * 5) /*5M*/
{
}
        
CTrapService::~CTrapService(void)
{
    this->Shutdown();
}

apl_int_t CTrapService::Startup( char const* apcConfigFileName )
{
    apl_int_t       liRetCode = 0;
    acl::CIniConfig loConfig;

    if (loConfig.Open(apcConfigFileName) != 0)
    {
        apl_errprintf(
            "TrapService open config %s fail, %s\n",
            apcConfigFileName,
            apl_strerror(apl_get_errno()) );
            
        return -1;
    }
    
    //Load Trap config
    if ( loConfig.GetValue("Trap", "ServiceIp").IsEmpty() )
    {
        apl_errprintf( "TrapService's server ip is empty and shall be disabled\n" );
            
        return 0;
    }
    else
    {
        if ( loConfig.GetValue("Trap", "ServicePort").IsEmpty()
            || loConfig.GetValue("Trap", "TableName").IsEmpty()
            || loConfig.GetValue("Trap", "ProgramName").IsEmpty()
            || loConfig.GetValue("Trap", "Type").IsEmpty()
            || loConfig.GetValue("Trap", "Level").IsEmpty()
            || loConfig.GetValue("Trap", "CommandID").IsEmpty() )
        {
            apl_errprintf( "TrapService is enabled and CommandID/ServicePort/TableName/ProgramName/Type/Level can't default\n" );
            
            return -1;
        }
        
        this->muCommandID = loConfig.GetValue("Trap", "CommandID").ToInt();
        this->moServiceIp = loConfig.GetValue("Trap", "ServiceIp").ToString();
        this->mu16ServicePort = loConfig.GetValue("Trap", "ServicePort").ToInt();
        this->moTableName = loConfig.GetValue("Trap", "TableName").ToString();
        this->moProgramName = loConfig.GetValue("Trap", "ProgramName").ToString();
        this->moType = loConfig.GetValue("Trap", "Type").ToString();
        this->moLevel = loConfig.GetValue("Trap", "Level").ToString();
        this->moCounter = loConfig.GetValue("Trap", "Counter", "1").ToString();
        this->moDelResult = loConfig.GetValue("Trap", "DelResult", "").ToString();
        this->moWarningStat = loConfig.GetValue("Trap", "WarningStat", "0").ToString();
        this->muInvokeID = loConfig.GetValue("Trap", "InvokeID", "1").ToInt();
    
        this->moBuffer.Resize( loConfig.GetValue("Trap", "CacheSize", "5120000").ToInt() );
    }
    //end
    
    if ( (liRetCode = this->ConnectToService() ) != 0 )
    {
        //Ignore
        //return -1;
    }
    
    this->mbIsShutdown = false;
    
    if (this->mpoReactor != NULL)
    {
        //Enable non-block write
        if ( this->moHandlePair.Open(APL_AF_UNIX, APL_SOCK_STREAM) != 0)
        {
            apl_errprintf( "TrapService open sockpair fail, %s\n", apl_strerror(apl_get_errno()) );
            
            return -1;
        }
    
        this->moHandlePair.GetFirst().Disable(acl::CIOHandle::OPT_NONBLOCK);
        this->moHandlePair.GetSecond().Enable(acl::CIOHandle::OPT_NONBLOCK);
            
        if ( this->moThreadManager.Spawn(CTrapService::Svc, this) != 0 )
        {
            this->mbIsShutdown = true;
            
            apl_errprintf( "TrapService spawn svc fail, %s\n", apl_strerror(apl_get_errno()) );
                
            return -1;
        }
    }
    
    return 0;
}
    
apl_int_t CTrapService::Restart(void)
{
    return -1;
}
    
apl_int_t CTrapService::Shutdown(void)
{
    this->mbIsShutdown = true;
    
    this->moHandlePair.GetFirst().Close();
    this->moHandlePair.GetSecond().Close();
    
    this->moThreadManager.WaitAll();
    
    return 0;
}

void CTrapService::SetReactor( acl::CReactor* apoReactor )
{
    this->mpoReactor = apoReactor;
}

apl_int_t CTrapService::Trap( char const* apcTitle, char const* apcFormat, ... )
{
    acl::CMemoryBlock loBuffer(1024);
    apl_size_t   luTotalLength = 0;
    //async header
    apl_int32_t  liMsgID = apl_hton32(1000);
    apl_int32_t  liResult = 0;
    apl_int32_t  liLength = 0;
    //statserver header
    //apl_int32_t  liLength = 0;
    apl_uint32_t luCommandID = apl_hton32(this->muCommandID);
    apl_uint32_t luInvokeID = apl_hton32(this->muInvokeID);
    
    loBuffer.SetWritePtr(24);

    loBuffer.Write("<", 1 );
    loBuffer.Write(this->moTableName.c_str(), this->moTableName.length() );
    loBuffer.Write(">", 1 );
    
    loBuffer.Write("<sProgramName>", apl_strlen("<sProgramName>") );
    loBuffer.Write(this->moProgramName.c_str(), this->moProgramName.length() );
    loBuffer.Write("</sProgramName>", apl_strlen("</sProgramName>") );
    
    loBuffer.Write("<nType>", apl_strlen("<nType>") );
    loBuffer.Write(this->moType.c_str(), this->moType.length() );
    loBuffer.Write("</nType>", apl_strlen("</nType>") );
    
    loBuffer.Write("<cLevel>", apl_strlen("<cLevel>") );
    loBuffer.Write(this->moLevel.c_str(), this->moLevel.length() );
    loBuffer.Write("</cLevel>", apl_strlen("</cLevel>") );
    
    loBuffer.Write("<sTitle>", apl_strlen("<sTitle>") );
    loBuffer.Write(apcTitle, apl_strlen(apcTitle) );
    loBuffer.Write("</sTitle>", apl_strlen("</sTitle>") );

    loBuffer.Write("<sContent>", apl_strlen("<sContent>") );

    va_list loVaList;
    va_start(loVaList, apcFormat);
    apl_vsnprintf(loBuffer.GetWritePtr(), loBuffer.GetSpace(), apcFormat, loVaList);
    va_end(loVaList);
    
    loBuffer.SetWritePtr( apl_strlen(loBuffer.GetWritePtr() ) );
    loBuffer.Write("</sContent>", apl_strlen("</sContent>") );
    
    if (this->moCounter.length() > 0)
    {
        loBuffer.Write("<nCounter>", apl_strlen("<nCounter>") );
        loBuffer.Write(this->moCounter.c_str(), this->moCounter.length() );
        loBuffer.Write("</nCounter>", apl_strlen("</nCounter>") );
    }
    if (this->moDelResult.length() > 0)
    {
        loBuffer.Write("<delResult>", apl_strlen("<delResult>") );
        loBuffer.Write(this->moDelResult.c_str(), this->moDelResult.length() );
        loBuffer.Write("</delResult>", apl_strlen("</delResult>") );
    }
    if (this->moWarningStat.length() > 0)
    {
        loBuffer.Write("<warningStat>", apl_strlen("<warningStat>") );
        loBuffer.Write(this->moWarningStat.c_str(), this->moWarningStat.length() );
        loBuffer.Write("</warningStat>", apl_strlen("</warningStat>") );
    }
    
    loBuffer.Write("</", 2 );
    loBuffer.Write(this->moTableName.c_str(), this->moTableName.length() );
    loBuffer.Write(">", 1 );
    
    luTotalLength = loBuffer.GetLength();
    liLength = apl_hton32(loBuffer.GetLength() - 12);
    
    loBuffer.Reset();

    loBuffer.Write(&liMsgID, 4);
    loBuffer.Write(&liResult, 4);
    loBuffer.Write(&liLength, 4);
    loBuffer.Write(&liLength, 4);
    loBuffer.Write(&luCommandID, 4);
    loBuffer.Write(&luInvokeID, 4);
    
    acl::TSmartLock<acl::CLock> loLock(this->moLock);
    acl::CTimeValue loTimeout(10);
    
    if (this->moPeer.GetHandle() == ACL_INVALID_HANDLE)
    {
        return -1;
    }
    
    if (this->moPeer.Send(loBuffer.GetBase(), luTotalLength, loTimeout) != (apl_ssize_t)luTotalLength)
    {
        this->moPeer.Close();
        
        return -1;
    }
    
    if (this->moPeer.Recv(loBuffer.GetBase(), 12, loTimeout) != 12
        && apl_get_errno() == APL_ECONNRESET)
    {
        this->moPeer.Close();
        
        return -1;
    }

    return 0;
}

apl_int_t CTrapService::TrapNonBlock( char const* apcTitle, char const* apcFormat, ... )
{
    CTrap   loTrap;
    va_list loVaList;
    
    apl_memset(&loTrap, 0, sizeof(loTrap) );
    apl_strncpy(loTrap.macTitle, apcTitle, sizeof(loTrap.macTitle) );

    va_start(loVaList, apcFormat);
    
    apl_vsnprintf(loTrap.macContent, sizeof(loTrap.macContent), apcFormat, loVaList);
    
    va_end(loVaList);
    
    if ( this->moBuffer.GetSpace() < sizeof(loTrap) )
    {
        return -1;
    }
    else
    {
        apl_memcpy( this->moBuffer.GetWritePtr(), &loTrap, sizeof(loTrap) );
        this->moBuffer.SetWritePtr(sizeof(loTrap) );
        
        if (this->moBuffer.GetLength() > 0 && ACL_BIT_DISABLED(this->miEvents, acl::IEventHandler::WRITE_MASK) )
        {
            if ( this->mpoReactor->RegisterHandler(
                this->moHandlePair.GetSecond().GetHandle(),
                this,
                acl::IEventHandler::WRITE_MASK ) != 0 )
            {
                //Trap HERE: Can only use blocking mode
                apl_errprintf(
                    "TrapService register handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]\n",
                    apl_strerror(apl_get_errno()),
                    apl_get_errno(),
                    (apl_int_t)acl::IEventHandler::WRITE_MASK );
            }
            else
            {
                ACL_SET_BITS(this->miEvents, acl::IEventHandler::WRITE_MASK);
            }
        }
    }
    
    return 0;
}

apl_int_t CTrapService::HandleOutput( apl_handle_t aiHandle )
{
    apl_ssize_t      liResult = 0;
    acl::CSockStream loPeer;
    
    loPeer.SetHandle(aiHandle);

    if (this->moBuffer.GetLength() == 0)
    {
        this->moBuffer.Reset();

        if ( this->mpoReactor->RemoveHandler(aiHandle, acl::IEventHandler::WRITE_MASK) != 0 )
        {
            apl_errprintf(
                "TrapService remove handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]\n",
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
        //Trap HERE: Can only use blocking mode
        apl_errprintf(
            "TrapService Sock pair exception, write fail, [result=%"APL_PRIdINT"][errno=%"APL_PRIdINT"]/[size=%"APL_PRIuINT"]\n",
            liResult,
            apl_get_errno(),
            this->moBuffer.GetLength() );
    }
    
    return 0;
}

void* CTrapService::Svc( void* apvService )
{
    CTrapService*     lpoService = static_cast<CTrapService*>(apvService);
    apl_ssize_t      liResult = 0;
    acl::CSockStream loPeer;
    acl::CTimeValue  loTimeout(2);
    CTrap            loTrap;
    char             lacBuffer[sizeof(CTrap)] = {0};
    char*            lpcCurrBuffer = lacBuffer;
    apl_size_t       luBufferSize = sizeof(CTrap);
    
    loPeer.SetHandle( lpoService->moHandlePair.GetFirst().GetHandle() );

    while(!lpoService->mbIsShutdown)
    {
        liResult = loPeer.Recv(lpcCurrBuffer, luBufferSize, loTimeout);
        if (liResult == sizeof(CTrap) )
        {
            lpoService->CheckConnection();
            
            apl_memcpy(&loTrap, lacBuffer, sizeof(loTrap) );
            if ( lpoService->Trap(loTrap.macTitle, "%s", loTrap.macContent) != 0 )
            {
                //Ignore
            }
            
            lpcCurrBuffer = lacBuffer;
            luBufferSize  = sizeof(CTrap);
        }
        else if (liResult > 0)
        {
            lpcCurrBuffer += liResult;
            luBufferSize  -= liResult;
        }
        else
        {
            if (apl_get_errno() == APL_ETIMEDOUT)
            {
                lpoService->CheckConnection();
            }
            else
            {
                //Trap HERE: Can only use blocking mode
                apl_errprintf(
                    "TrapService Sock pair exception, recv fail, [result=%"APL_PRIdINT"][errno=%"APL_PRIdINT"]\n",
                    liResult,
                    apl_get_errno() );
            }
        }
    }
    
    return NULL;
}

void CTrapService::CheckConnection(void)
{
    if (this->moPeer.GetHandle() == ACL_INVALID_HANDLE)
    {
        this->ConnectToService();
    }
}

apl_int_t CTrapService::ConnectToService(void)
{
    acl::CSockAddr loAddr(this->mu16ServicePort, this->moServiceIp.c_str() );
    acl::CSockConnector loConnector;
    acl::CTimeValue loTimeValue(10);

    if (loConnector.Connect(this->moPeer, loAddr, loTimeValue) != 0)
    {
        apl_errprintf(
            "TrapService connect to server [%s:%"APL_PRIdINT"] fail, %s [errno=%"APL_PRIdINT"]\n",
            this->moServiceIp.c_str(),
            (apl_int_t)this->mu16ServicePort,
            apl_strerror(apl_get_errno()),
            apl_get_errno() );
            
        return -1;
    }
    
    return 0;
}

AAF_NAMESPACE_END
