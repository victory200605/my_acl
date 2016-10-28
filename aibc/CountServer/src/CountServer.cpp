
#include "acl/SockHandle.h"
#include "CountServer.h"
#include "LogServer.h"
#include "Table.h"

CCountServer* CCountServer::cpoInstance = NULL;
    
CCountServer* CCountServer::Instance(void)
{
    if (cpoInstance == NULL)
    {
        ACL_NEW_ASSERT(cpoInstance, CCountServer);
    }
    
    return cpoInstance;
}
    
void CCountServer::Release(void)
{
    ACL_DELETE(cpoInstance);
}
    
CCountServer::CCountServer(void)
    : mpoReactor(NULL)
    , miRequestNumLimit(1)
{
}
    
CCountServer::~CCountServer(void)
{
    this->Close();
}
    
apl_int_t CCountServer::Startup( acl::CReactor& aoReactor, char const* apcIpAddress, apl_uint16_t au16Port )
{
    apl_int_t      liRetCode = 0;
    acl::CSockAddr loAddr(au16Port, apcIpAddress);
        
    this->mpoReactor = &aoReactor;
        
    if ( ( liRetCode = this->moAcceptor.Open(loAddr) ) != 0 )
    {
        CNTSRV_LOG_ERROR(
            "CountServer open server [%s:%"APL_PRIu16"] fail,%s [errno=%"APL_PRIdINT"]",
            apcIpAddress,
            au16Port,
            apl_strerror(apl_get_errno()),
            apl_get_errno() );
            
        return liRetCode;
    }

    liRetCode = 
        this->mpoReactor->RegisterHandler(
            this->moAcceptor.GetHandle(),
            this,
            acl::IEventHandler::ACCEPT_MASK );
    if (liRetCode != 0)
    {
        CNTSRV_LOG_ERROR(
            "CountServer register accept handler fail,%s [errno=%"APL_PRIdINT"]",
            apl_strerror(apl_get_errno()),
            apl_get_errno() );
    }
    
    return liRetCode;
}
    
void CCountServer::Close(void)
{
    this->moAcceptor.Close();
    
    this->RemoveAll();
}

void CCountServer::SetRequestNumLimit( apl_int_t aiRequestNumLimit )
{
    this->miRequestNumLimit = aiRequestNumLimit;
}
    
apl_int_t CCountServer::HandleInput( apl_handle_t aiHandle )
{
    apl_int_t        liRetCode = 0;
    acl::CSockStream loPeer;
    acl::CSockAddr   loRemote;
    char             lacIpAddress[64] = {0};
    
    liRetCode = this->moAcceptor.Accept(loPeer, NULL);
    if (liRetCode != 0)
    {
        //Log Here
        CNTSRV_LOG_ERROR_NB(
            "CountServer accept new connection fail,%s [errno=%"APL_PRIdINT"]",
            apl_strerror(apl_get_errno()),
            apl_get_errno() );
            
        //Fatal: abort    
        ACL_ASSERT(false);
        
        return -1;
    }
    
    loPeer.Enable(acl::CIOHandle::OPT_NONBLOCK);
    loPeer.GetRemoteAddr(loRemote);
    loRemote.GetIpAddr(lacIpAddress, sizeof(lacIpAddress) );
    
    CCountServerHandler* lpoHandler = NULL;
    ACL_NEW_ASSERT( lpoHandler, CCountServerHandler(*this->mpoReactor, 4096, this->miRequestNumLimit) );
    liRetCode = 
        this->mpoReactor->RegisterHandler(
            loPeer.GetHandle(),
            lpoHandler,
            acl::IEventHandler::READ_MASK );
    if (liRetCode != 0)
    {
        //Log Here
        loPeer.Close();
        ACL_DELETE(lpoHandler);
        
        CNTSRV_LOG_ERROR_NB(
            "CountServer register handler fail,%s [errno=%"APL_PRIdINT"]",
            apl_strerror(apl_get_errno()),
            apl_get_errno() );
    }
    else
    {
        this->InsertHandler(lpoHandler);
            
        CNTSRV_LOG_DEBUG_NB(
            "CountServer accept a new connection success, [Handle=%"APL_PRIdINT"]/[IpAddress=%s:%"APL_PRIdINT"]",
            (apl_int_t)loPeer.GetHandle(),
            lacIpAddress,
            (apl_int_t)loRemote.GetPort() );
    }
    
    return 0;
}

void CCountServer::InsertHandler( acl::IEventHandler* apoHandler )
{
    this->moConnHandlers.insert( std::map<acl::IEventHandler*, apl_int_t>::value_type(apoHandler, 0) );
}

void CCountServer::RemoveHandler( acl::IEventHandler* apoHandler )
{
    std::map<acl::IEventHandler*, apl_int_t>::iterator loIter = this->moConnHandlers.find(apoHandler);
    
    if (loIter != this->moConnHandlers.end() )
    {
        acl::IEventHandler* lpoHandler = loIter->first;
        this->moConnHandlers.erase(loIter);
        ACL_DELETE(lpoHandler);
    }
}

void CCountServer::RemoveAll(void)
{
    std::map<acl::IEventHandler*, apl_int_t>::iterator loIter = this->moConnHandlers.begin();
    
    for ( ; loIter != this->moConnHandlers.end(); ++loIter )
    {        
        acl::IEventHandler* lpoHandler = loIter->first;
        ACL_DELETE(lpoHandler);
    }
    
    this->moConnHandlers.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////
CCountServerHandler::CCountServerHandler(
    acl::CReactor& aoReactor,
    apl_size_t auBuffSize,
    apl_int_t aiRequestNumLimit )
    : moRecvBuff(auBuffSize)
    , moSendBuff(auBuffSize)
    , moReactor(aoReactor)
    , miEvents(acl::IEventHandler::READ_MASK)
    , miRequestNumLimit(aiRequestNumLimit)
{
}
    
CCountServerHandler::~CCountServerHandler(void)
{
}

apl_int_t CCountServerHandler::HandleInput( apl_handle_t aiHandle )
{
    apl_ssize_t      liResult = 0;
    acl::CSockStream loPeer;
    
    loPeer.SetHandle(aiHandle);

    liResult = loPeer.Recv( this->moRecvBuff.GetWritePtr(), this->moRecvBuff.GetSpace() );
    
    CNTSRV_LOG_TRACE_NB(
        "CountServer Recv, [Handle=%"APL_PRIdINT"]/[Size=%"APL_PRIdINT"]",
        (apl_int_t)aiHandle,
        (apl_int_t)liResult );
    
    if (liResult < 0)
    {
        //Log here
        CNTSRV_LOG_ERROR_NB(
            "CountServer Recv fail,%s [Handle=%"APL_PRIdINT"]/[errno=%"APL_PRIdINT"]",
            apl_strerror(apl_get_errno()),
            (apl_int_t)aiHandle,
            apl_get_errno() );
        
        //Close connection
        if (this->moReactor.RemoveHandler(aiHandle, acl::IEventHandler::ALL_EVENTS_MASK) != 0)
        {
            CNTSRV_LOG_ERROR_NB(
                "CountServer remove handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                apl_get_errno(),
                (apl_int_t)acl::IEventHandler::ALL_EVENTS_MASK );
        }

        return 0;
    }
    
    this->moRecvBuff.SetWritePtr(liResult);

    if (this->DispatchRequest(aiHandle) < 0)
    {
        //Close connection
        if (this->moReactor.RemoveHandler(aiHandle, acl::IEventHandler::ALL_EVENTS_MASK) != 0)
        {
            CNTSRV_LOG_ERROR_NB(
                "CountServer remove handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                apl_get_errno(),
                (apl_int_t)acl::IEventHandler::ALL_EVENTS_MASK );
        }
        
        return 0;
    }

    if (this->moRecvBuff.GetSpace() == 0)
    {
        if ( this->moReactor.RemoveHandler(aiHandle, acl::IEventHandler::READ_MASK) != 0 )
        {
            CNTSRV_LOG_ERROR_NB(
                "CountServer remove handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                apl_get_errno(),
                (apl_int_t)acl::IEventHandler::READ_MASK );
        }
        else
        {
            ACL_CLR_BITS(this->miEvents, acl::IEventHandler::READ_MASK);
        }
    }
    
    if (this->moSendBuff.GetLength() > 0 && ACL_BIT_DISABLED(this->miEvents, acl::IEventHandler::WRITE_MASK) )
    {
        if ( this->moReactor.RegisterHandler(aiHandle, this, acl::IEventHandler::WRITE_MASK) != 0 )
        {
            CNTSRV_LOG_ERROR_NB(
                "CountServer register handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                apl_get_errno(),
                (apl_int_t)acl::IEventHandler::WRITE_MASK );
        }
        else
        {
            ACL_SET_BITS(this->miEvents, acl::IEventHandler::WRITE_MASK);
        }
    }
    
    return 0;
}

apl_int_t CCountServerHandler::HandleOutput( apl_handle_t aiHandle )
{
    apl_ssize_t      liResult = 0;
    acl::CSockStream loPeer;
    
    loPeer.SetHandle(aiHandle);

    if (this->moSendBuff.GetLength() == 0)
    {
        this->moSendBuff.Reset();
        
        if ( this->moReactor.RemoveHandler(aiHandle, acl::IEventHandler::WRITE_MASK) != 0 )
        {
            CNTSRV_LOG_ERROR_NB(
                "CountServer remove handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                apl_get_errno(),
                (apl_int_t)acl::IEventHandler::WRITE_MASK );
        }
        else
        {
            ACL_CLR_BITS(this->miEvents, acl::IEventHandler::WRITE_MASK);
        }
        
        return 0;
    }
    
    liResult = loPeer.Send( this->moSendBuff.GetReadPtr(), this->moSendBuff.GetLength() );
    
    CNTSRV_LOG_TRACE_NB(
        "CountServer Write, [Handle=%"APL_PRIdINT"]/[Size=%"APL_PRIdINT"]",
        (apl_int_t)aiHandle,
        (apl_int_t)liResult );
    
    if (liResult <= 0)
    {
        //Log here
        CNTSRV_LOG_ERROR_NB(
            "CountServer Send fail,%s [Handle=%"APL_PRIdINT"]/[errno=%"APL_PRIdINT"]",
            apl_strerror(apl_get_errno()),
            (apl_int_t)aiHandle,
            apl_get_errno() );
        
        //Close connection
        if (this->moReactor.RemoveHandler(aiHandle, acl::IEventHandler::ALL_EVENTS_MASK) != 0)
        {
            CNTSRV_LOG_ERROR_NB(
                "CountServer remove handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                apl_get_errno(),
                (apl_int_t)acl::IEventHandler::ALL_EVENTS_MASK );
        }   

        return 0;
    }
    
    this->moSendBuff.SetReadPtr(liResult);
    
    if (this->DispatchRequest(aiHandle) < 0)
    {
        //Close connection
        if (this->moReactor.RemoveHandler(aiHandle, acl::IEventHandler::ALL_EVENTS_MASK) != 0)
        {
            CNTSRV_LOG_ERROR_NB(
                "CountServer remove handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                apl_get_errno(),
                (apl_int_t)acl::IEventHandler::ALL_EVENTS_MASK );
        }
        
        return 0;
    }
    
    if (this->moRecvBuff.GetSpace() > 0 && ACL_BIT_DISABLED(this->miEvents, acl::IEventHandler::READ_MASK) )
    {
        if ( this->moReactor.RegisterHandler(aiHandle, this, acl::IEventHandler::READ_MASK) != 0 )
        {
            CNTSRV_LOG_ERROR_NB(
                "CountServer register handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                apl_get_errno(),
                (apl_int_t)acl::IEventHandler::READ_MASK );
        }
        else
        {
            ACL_SET_BITS(this->miEvents, acl::IEventHandler::READ_MASK);
        }
    }
    
    return 0;
}

apl_int_t CCountServerHandler::HandleClose( apl_handle_t aiHandle, apl_int_t aiMask )
{
    if (acl::IEventHandler::ALL_EVENTS_MASK == aiMask)
    {
        CNTSRV_LOG_ERROR_NB("CountServer close connection, [Handle=%"APL_PRIdINT"]", (apl_int_t)aiHandle);
        
        //Release all conection
        for (ConnMapType::iterator loIter = this->moConnMap.begin();
             loIter != this->moConnMap.end(); ++loIter )
        {
            CCounter* lpoCounter = CTable::Instance()->GetCounter(loIter->first);
            if (lpoCounter != NULL)
            {
                lpoCounter->Release(loIter->second);
            }
        }
        
        apl_close(aiHandle);
        
        CCountServer::Instance()->RemoveHandler(this);
    }
    
    return 0;
}

apl_ssize_t CCountServerHandler::DispatchRequest( apl_handle_t aiHandle )
{
    apl_int_t   liRetCode = 0;
    apl_ssize_t liResult = 0;
    
    if (this->moSendBuff.GetReadPtr() != this->moSendBuff.GetBase() )
    {
        apl_size_t luLength = this->moSendBuff.GetLength();
        
        if (luLength == 0)
        {
            this->moSendBuff.Reset();
        }
        else
        {
            apl_memmove( this->moSendBuff.GetBase(), this->moSendBuff.GetReadPtr(), luLength );
            this->moSendBuff.Reset();
            this->moSendBuff.SetWritePtr(luLength);
        }
    }
    
    while(true)
    {
        if (this->moRecvBuff.GetLength() >= CCSPRequest::REQUEST_SIZE)
        {
            CCSPRequest  loRequest;
            CCSPResponse loResponse;
            
            //Have enough send buffer
            if (this->moSendBuff.GetSpace() < CCSPResponse::RESPONSE_SIZE )
            {
                //Log Here                
                CNTSRV_LOG_ERROR_NB(
                    "CountServer send buffer too small, [space=%"APL_PRIuINT"]/[response size=%"APL_PRIdINT"]",
                    (apl_size_t)this->moSendBuff.GetSpace(),
                    (apl_int_t)CCSPResponse::RESPONSE_SIZE );
                
                break;
            }
            
            if ( loRequest.Decode(this->moRecvBuff.GetReadPtr(), CCSPRequest::REQUEST_SIZE) > 0 )
            {
                liRetCode = this->RequestEntry( loRequest, loResponse );
            }
            else
            {
                liRetCode = CNTSVR_ERROR_INVALID_PTL;
            }

            if (liRetCode == CNTSVR_ERROR_INVALID_PTL)
            {
                acl::CMemoryBlock loBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE);
                acl::CSockAddr    loRemote;
                acl::CSockStream  loPeer;
                char lacIpAddress[64] = {0};
                
                loPeer.SetHandle(aiHandle);
                
                if ( loPeer.GetRemoteAddr(loRemote) == 0 )
                {
                    loRemote.GetIpAddr( lacIpAddress, sizeof(lacIpAddress) );
                    CNTSRV_LOG_ERROR_NB(
                        "Invalid protocol request, from [Ip=%s:%"APL_PRIdINT"]/[ReadPos=%"APL_PRIdINT"]/[WritePos=%"APL_PRIdINT"]",
                        lacIpAddress,
                        (apl_int_t)loRemote.GetPort(),
                        (apl_int_t)(this->moRecvBuff.GetReadPtr() - this->moRecvBuff.GetBase() ),
                        (apl_int_t)(this->moRecvBuff.GetWritePtr() - this->moRecvBuff.GetBase() ) );
                }
                else
                {
                    CNTSRV_LOG_ERROR_NB(
                        "Invalid protocol request, from [Ip=unknow:unknow]/[ReadPos=%"APL_PRIdINT"]/[WritePos=%"APL_PRIdINT"]",
                        (apl_int_t)(this->moRecvBuff.GetReadPtr() - this->moRecvBuff.GetBase() ),
                        (apl_int_t)(this->moRecvBuff.GetWritePtr() - this->moRecvBuff.GetBase() ) );
                }
                
                CNTSRV_LOG_ERROR_NB(
                    "\n%s",
                    CLogServer::UStringToHex(this->moRecvBuff.GetReadPtr(), this->moRecvBuff.GetLength(), loBuffer) );

                return -1;
            }

            this->moRecvBuff.SetReadPtr(CCSPRequest::REQUEST_SIZE);

            loResponse.Encode( this->moSendBuff.GetWritePtr(), CCSPResponse::RESPONSE_SIZE );
            this->moSendBuff.SetWritePtr(CCSPResponse::RESPONSE_SIZE);
            liResult++;
        }
        else
        {
            break;
        }
    }
    
    if (this->moRecvBuff.GetReadPtr() != this->moRecvBuff.GetBase() )
    {
        apl_size_t luLength = this->moRecvBuff.GetLength();
        
        if (luLength == 0)
        {
            this->moRecvBuff.Reset();
        }
        else
        {
            apl_memmove( this->moRecvBuff.GetBase(), this->moRecvBuff.GetReadPtr(), luLength );
            this->moRecvBuff.Reset();
            this->moRecvBuff.SetWritePtr(luLength);
        }
    }
    
    return liResult;
}

apl_int_t CCountServerHandler::RequestEntry( CCSPRequest& aoRequest, CCSPResponse& aoResponse )
{
    apl_int_t       liRetCode = 0;
    acl::CTimestamp loTimestamp;
    
    loTimestamp.Update(acl::CTimestamp::PRC_USEC);
    
    if (aoRequest.GetCmdID() == CNTSVR_REQUEST_CONN)
    {
        liRetCode = this->CountRequest(aoRequest.GetKey(), aoRequest.GetNum(), aoRequest.GetLastNum());
    }
    else if (aoRequest.GetCmdID() == CNTSVR_REQUEST_SEND)
    {
        liRetCode = this->SpeedRequest(aoRequest.GetKey(), aoRequest.GetNum(), loTimestamp);
    }
    else
    {
        CNTSRV_LOG_ERROR_NB(
            "Invalid protocol request, [CMD=0x%X]/[Key=%s]/[Num=%"APL_PRIdINT"]/[LastNum=%"APL_PRIdINT"]",
            aoRequest.GetCmdID(),
            aoRequest.GetKey(),
            (apl_int_t)aoRequest.GetNum(),
            (apl_int_t)aoRequest.GetLastNum() );
        
        liRetCode = CNTSVR_ERROR_INVALID_PTL;
    }
    
    aoResponse.SetMsgID(aoRequest.GetMsgID() );
    aoResponse.SetCmdID(aoRequest.GetCmdID() | 0x10);
    aoResponse.SetState(liRetCode);
    aoResponse.SetTimestamp( loTimestamp.Nsec() );
    
    return liRetCode;
}
    
apl_int_t CCountServerHandler::CountRequest( char const* apcKey, apl_int32_t ai32Num, apl_int32_t ai32LastNum )
{
    apl_int_t liRetCode = 0;
    CCounter* lpoCounter = NULL;
    apl_int_t liIdx = 0;
    
    if( ai32LastNum < 0
        || (ai32Num + ai32LastNum) < 0
        || ai32Num < -this->miRequestNumLimit
        || ai32Num > this->miRequestNumLimit )
    {
        return CNTSVR_ERROR_LIMITED;
    }

    if ( ( lpoCounter = CTable::Instance()->GetCounter(apcKey, &liIdx) ) != NULL )
    {
        apl_int_t& liCurrNum = this->moConnMap[liIdx];
        
        //Adjust counter
        if (liCurrNum > ai32LastNum)
        {
            lpoCounter->Release(liCurrNum - ai32LastNum);
        }
        else if (liCurrNum < ai32LastNum)
        {
            lpoCounter->Acquire(ai32LastNum - liCurrNum, CCounter::OPT_FORCE);
        }
        
        liCurrNum = ai32LastNum;
        
        if (ai32Num > 0)
        {
            liRetCode = lpoCounter->Acquire(ai32Num);
        }      
        else if (ai32Num < 0)
        {
            liRetCode = lpoCounter->Release(-ai32Num);
        }
        
        if (liRetCode != 0)
        {
            CNTSRV_LOG_WARN_NB(
                "Counter acquire fail, [Key=%s]/[Num=%"APL_PRId32"]/[LastNum=%"APL_PRId32"]/[CurrNum=%"APL_PRIuINT"]/[MaxNum=%"APL_PRIuINT"]",
                apcKey,
                ai32Num,
                ai32LastNum,
                lpoCounter->GetCurTokens(),
                lpoCounter->GetBucketSize() );
        
            return CNTSVR_ERROR_LIMITED;
        }
        else
        {
            CNTSRV_LOG_DEBUG_NB(
                "Counter acquire success, [Key=%s]/[Num=%"APL_PRId32"]/[LastNum=%"APL_PRId32"]/[CurrNum=%"APL_PRIuINT"]/[MaxNum=%"APL_PRIuINT"]",
                apcKey,
                ai32Num,
                ai32LastNum,
                lpoCounter->GetCurTokens(),
                lpoCounter->GetBucketSize() );
                
            liCurrNum += ai32Num;
            
            return liRetCode;
        }
    }
    else
    {
        CNTSRV_LOG_ERROR_NB(
            "Counter no found, [Key=%s]/[Num=%"APL_PRId32"]/[LastNum=%"APL_PRId32"]",
            apcKey,
            ai32Num,
            ai32LastNum );
        
        return CNTSVR_ERROR_NO_FOUND;
    }
}
    
apl_int_t CCountServerHandler::SpeedRequest( char const* apcKey, apl_int32_t ai32Num, acl::CTimestamp& aoTimestamp )
{
    CSpeeder* lpoSpeeder = NULL;
	acl::CTimestamp loLastTime;
    
    if (ai32Num <= 0)
    {
        CNTSRV_LOG_ERROR_NB(
            "Invalid protocol request, [CMD=0x%X]/[Key=%s]/[Num=%"APL_PRId32"]",
            CNTSVR_REQUEST_SEND,
            apcKey,
            ai32Num );
        
        return CNTSVR_ERROR_INVALID_PTL;
    }
     
    if ( ( lpoSpeeder = CTable::Instance()->GetSpeeder(apcKey, NULL) ) != NULL )
    {
		loLastTime = lpoSpeeder->GetLastTime();
        if ( lpoSpeeder->Acquire(ai32Num, aoTimestamp) == 0 )
        {
            CNTSRV_LOG_DEBUG_NB(
                "Speeder acquire success, [Key=%s]/[Num=%"APL_PRId32"]/[Speed=%"APL_PRIuINT"]/[CurrToken=%lf]/[LastTime=%"APL_PRId64"]/[CurrTime=%"APL_PRId64"]",
                apcKey,
                ai32Num,
                lpoSpeeder->GetSpeed(),
                lpoSpeeder->GetCurTokens(),
				(apl_int64_t)loLastTime.Nsec(),
                (apl_int64_t)aoTimestamp.Nsec() );
                
            return 0;
        }
        else
        {
            CNTSRV_LOG_WARN_NB(
                "Speeder acquire fail, [Key=%s]/[Num=%"APL_PRId32"]/[Speed=%"APL_PRIuINT"]/[CurrToken=%lf]/[LastTime=%"APL_PRId64"]/[CurrTime=%"APL_PRId64"]",
                apcKey,
                ai32Num,
                lpoSpeeder->GetSpeed(),
                lpoSpeeder->GetCurTokens(),
				(apl_int64_t)loLastTime.Nsec(),
                (apl_int64_t)aoTimestamp.Nsec() );
        
            return CNTSVR_ERROR_LIMITED;
        }
    }
    else
    {
        CNTSRV_LOG_ERROR_NB(
            "Speeder no found, [Key=%s]/[Num=%"APL_PRId32"]",
            apcKey,
            ai32Num );
        
        return CNTSVR_ERROR_NO_FOUND;
    }
}
