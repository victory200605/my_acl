#include "acl/SockHandle.h"
#include "aaf/LogServer.h"
#include "LogServerHandler.h"

    
#if 0
void CLogServerBase::Release(void)
{
}
#endif
    
CLogServerBase::CLogServerBase(void)
    : mpoReactor(NULL)
    , miRequestNumLimit(1)
{
}
    
CLogServerBase::~CLogServerBase(void)
{
    this->Close();
}
    
apl_int_t CLogServerBase::Startup( acl::CReactor& aoReactor, char const* apcIpAddress, apl_uint16_t au16Port )
{
    apl_int_t      liRetCode = 0;
    acl::CSockAddr loAddr(au16Port, apcIpAddress);
        
    this->mpoReactor = &aoReactor;
        
    if ( ( liRetCode = this->moAcceptor.Open(loAddr) ) != 0 )
    {
        AAF_LOG_INFO(
            "LogServer open server [%s:%"APL_PRIu16"] fail,%s [errno=%"APL_PRIdINT"]",
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
        AAF_LOG_INFO(
            "LogServer register accept handler fail,%s [errno=%"APL_PRIdINT"]",
            apl_strerror(apl_get_errno()),
            apl_get_errno() );
    }
    
    return liRetCode;
}
    
void CLogServerBase::Close(void)
{
    this->moAcceptor.Close();
    
    this->RemoveAll();
}

void CLogServerBase::SetRequestNumLimit( apl_int_t aiRequestNumLimit )
{
    this->miRequestNumLimit = aiRequestNumLimit;
}
    
apl_int_t CLogServerBase::HandleInput( apl_handle_t aiHandle )
{
    apl_int_t        liRetCode = 0;
    acl::CSockStream loPeer;
    acl::CSockAddr   loRemote;
    char             lacIpAddress[64] = {0};
    
    liRetCode = this->moAcceptor.Accept(loPeer, NULL);
    if (liRetCode != 0)
    {
        //Log Here
        AAF_LOG_ERROR_NB(
            "LogServer accept new connection fail,%s [errno=%"APL_PRIdINT"]",
            apl_strerror(apl_get_errno()),
            apl_get_errno() );
            
        //Fatal: abort    
        ACL_ASSERT(false);
        
        return -1;
    }
    
    loPeer.Enable(acl::CIOHandle::OPT_NONBLOCK);
    loPeer.GetRemoteAddr(loRemote);
    loRemote.GetIpAddr(lacIpAddress, sizeof(lacIpAddress) );
    
    CLogServerHandler* lpoHandler = NULL;
    ACL_NEW_ASSERT( lpoHandler, CLogServerHandler(*this->mpoReactor, 4096, this->miRequestNumLimit) );
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
        
        AAF_LOG_ERROR_NB(
            "LogServer register handler fail,%s [errno=%"APL_PRIdINT"]",
            apl_strerror(apl_get_errno()),
            apl_get_errno() );
    }
    else
    {
        this->InsertHandler(lpoHandler);
            
        AAF_LOG_DEBUG_NB(
            "LogServer accept a new connection success, [Handle=%"APL_PRIdINT"]/[IpAddress=%s:%"APL_PRIdINT"]",
            (apl_int_t)loPeer.GetHandle(),
            lacIpAddress,
            (apl_int_t)loRemote.GetPort() );
    }
    
    return 0;
}


void CLogServerBase::InsertHandler( acl::IEventHandler* apoHandler )
{
    this->moConnHandlers.insert( std::map<acl::IEventHandler*, apl_int_t>::value_type(apoHandler, 0) );
}


void CLogServerBase::RemoveHandler( acl::IEventHandler* apoHandler )
{
    std::map<acl::IEventHandler*, apl_int_t>::iterator loIter = this->moConnHandlers.find(apoHandler);
    
    if (loIter != this->moConnHandlers.end() )
    {
        acl::IEventHandler* lpoHandler = loIter->first;
        this->moConnHandlers.erase(loIter);
        ACL_DELETE(lpoHandler);
    }
}

void CLogServerBase::RemoveAll(void)
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






CLogServerHandler::CLogServerHandler(
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
    

CLogServerHandler::~CLogServerHandler(void)
{
}


apl_int_t CLogServerHandler::HandleInput( apl_handle_t aiHandle )
{
    apl_ssize_t      liResult = 0;
    acl::CSockStream loPeer;
    
    loPeer.SetHandle(aiHandle);

    liResult = loPeer.Recv( this->moRecvBuff.GetWritePtr(), this->moRecvBuff.GetSpace() );
    
    AAF_LOG_TRACE_NB(
        "LogServer Recv, [Handle=%"APL_PRIdINT"]/[Size=%"APL_PRIdINT"]",
        (apl_int_t)aiHandle,
        (apl_int_t)liResult );
    
    if (liResult < 0)
    {
        //Log here
        AAF_LOG_ERROR_NB(
            "LogServer Recv fail,%s [Handle=%"APL_PRIdINT"]/[errno=%"APL_PRIdINT"]",
            apl_strerror(apl_get_errno()),
            (apl_int_t)aiHandle,
            apl_get_errno() );
        
        //Close connection
        if (this->moReactor.RemoveHandler(aiHandle, acl::IEventHandler::ALL_EVENTS_MASK) != 0)
        {
            AAF_LOG_ERROR_NB(
                "LogServer remove handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                apl_get_errno(),
                (apl_int_t)acl::IEventHandler::ALL_EVENTS_MASK );
        }

        return 0;
    }
    
    this->moRecvBuff.SetWritePtr(liResult);

   
    if (this->DispatchRequest(aiHandle, liResult) < 0)
    {
        //Close connection
        if (this->moReactor.RemoveHandler(aiHandle, acl::IEventHandler::ALL_EVENTS_MASK) != 0)
        {
            AAF_LOG_ERROR_NB(
                "LogServer remove handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
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
            AAF_LOG_ERROR_NB(
                "LogServer remove handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
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
            AAF_LOG_ERROR_NB(
                "LogServer register handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
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

apl_int_t CLogServerHandler::HandleOutput( apl_handle_t aiHandle )
{
    apl_ssize_t      liResult = 0;
    acl::CSockStream loPeer;
    
    loPeer.SetHandle(aiHandle);

    if (this->moSendBuff.GetLength() == 0)
    {
        this->moSendBuff.Reset();
        
        if ( this->moReactor.RemoveHandler(aiHandle, acl::IEventHandler::WRITE_MASK) != 0 )
        {
            AAF_LOG_ERROR_NB(
                "LogServer remove handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
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
    
    AAF_LOG_TRACE_NB(
        "LogServer Write, [Handle=%"APL_PRIdINT"]/[Size=%"APL_PRIdINT"]",
        (apl_int_t)aiHandle,
        (apl_int_t)liResult );
    
    if (liResult <= 0)
    {
        //Log here
        AAF_LOG_ERROR_NB(
            "LogServer Send fail,%s [Handle=%"APL_PRIdINT"]/[errno=%"APL_PRIdINT"]",
            apl_strerror(apl_get_errno()),
            (apl_int_t)aiHandle,
            apl_get_errno() );
        
        //Close connection
        if (this->moReactor.RemoveHandler(aiHandle, acl::IEventHandler::ALL_EVENTS_MASK) != 0)
        {
            AAF_LOG_ERROR_NB(
                "LogServer remove handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                apl_get_errno(),
                (apl_int_t)acl::IEventHandler::ALL_EVENTS_MASK );
        }   

        return 0;
    }
    
    this->moSendBuff.SetReadPtr(liResult);
    
    if (this->DispatchRequest(aiHandle, liResult) < 0)
    {
        //Close connection
        if (this->moReactor.RemoveHandler(aiHandle, acl::IEventHandler::ALL_EVENTS_MASK) != 0)
        {
            AAF_LOG_ERROR_NB(
                "LogServer remove handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
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
            AAF_LOG_ERROR_NB(
                "LogServer register handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
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

apl_int_t CLogServerHandler::HandleClose( apl_handle_t aiHandle, apl_int_t aiMask )
{
    if (acl::IEventHandler::ALL_EVENTS_MASK == aiMask)
    {
        AAF_LOG_ERROR_NB("LogServer close connection, [Handle=%"APL_PRIdINT"]", (apl_int_t)aiHandle);
        
        //Release all conection
        apl_close(aiHandle);
        
        acl::Instance<CLogServerBase>()->RemoveHandler(this);
    }
    
    return 0;
}

apl_ssize_t CLogServerHandler::DispatchRequest( apl_handle_t aiHandle, apl_ssize_t auRecvLen )
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
        if (this->moRecvBuff.GetLength() >= LOG_SERVER_PROTOCOL_HEADER_LEN)
        {
            CLogServerMsg loLogserverMsg;
            CLogServerProtocol loRequest(&loLogserverMsg);
            CLogServerProtocol loResponse(APL_NULL);
            
            //Have enough send buffer
            if (this->moSendBuff.GetSpace() < LOG_SERVER_PROTOCOL_HEADER_LEN)
            {
                //Log Here                
                AAF_LOG_ERROR_NB(
                    "LogServer send buffer too small, [space=%"APL_PRIuINT"]/[response size=%"APL_PRIdINT"]",
                    (apl_size_t)this->moSendBuff.GetSpace(),
                    (apl_int_t)LOG_SERVER_PROTOCOL_HEADER_LEN);
                
                break;
            }
            
            if ( (liRetCode = loRequest.Decode(this->moRecvBuff.GetReadPtr(), this->moRecvBuff.GetLength())) > 0 )
            {
                this->moRecvBuff.SetReadPtr(liRetCode);
                this->RequestEntry( loRequest, loResponse );
            }
            //not enough content, wait for next recv 
            else if (liRetCode == 0)
            {
                break;
            } 
            //decode failed
            else 
            {
                // clear what we recv from this handler
                this->moRecvBuff.SetReadPtr(auRecvLen);

                acl::CMemoryBlock loBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE);
                acl::CSockAddr    loRemote;
                acl::CSockStream  loPeer;
                char lacIpAddress[64] = {0};
                
                loPeer.SetHandle(aiHandle);
                
                if ( loPeer.GetRemoteAddr(loRemote) == 0 )
                {
                    loRemote.GetIpAddr( lacIpAddress, sizeof(lacIpAddress) );
                    AAF_LOG_ERROR_NB(
                        "Invalid protocol request, from [Ip=%s:%"APL_PRIdINT"]/[ReadPos=%"APL_PRIdINT"]/[WritePos=%"APL_PRIdINT"]",
                        lacIpAddress,
                        (apl_int_t)loRemote.GetPort(),
                        (apl_int_t)(this->moRecvBuff.GetReadPtr() - this->moRecvBuff.GetBase() ),
                        (apl_int_t)(this->moRecvBuff.GetWritePtr() - this->moRecvBuff.GetBase() ) );
                }
                else
                {
                    AAF_LOG_ERROR_NB(
                        "Invalid protocol request, from [Ip=unknow:unknow]/[ReadPos=%"APL_PRIdINT"]/[WritePos=%"APL_PRIdINT"]",
                        (apl_int_t)(this->moRecvBuff.GetReadPtr() - this->moRecvBuff.GetBase() ),
                        (apl_int_t)(this->moRecvBuff.GetWritePtr() - this->moRecvBuff.GetBase() ) );
                }
                
                AAF_LOG_ERROR_NB(
                    "\n%s",
                    this->moRecvBuff.GetReadPtr());

                return -1;
            }

            //check whether we need to response 
            if (LOG_SERVER_MODE_TWO_WAY == loRequest.GetMode())
            {
                liRetCode = loResponse.Encode( this->moSendBuff.GetWritePtr(), this->moSendBuff.GetSpace() );
                if (liRetCode > 0) 
                {
                    this->moSendBuff.SetWritePtr(liRetCode);
                }
                else 
                {
                    //there are something terrible wrong, we should 
                    //have enough space encode but we don't now.
                    //we will ignore this response for ever
                    AAF_LOG_ERROR_NB(
                            "LogServer send buffer too small, [space=%"APL_PRIuINT"]/[response size=%"APL_PRIdINT"], Fatal, the response won't send out",
                            (apl_size_t)this->moSendBuff.GetSpace(),
                            (apl_int_t)LOG_SERVER_PROTOCOL_HEADER_LEN);
                }
            }

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


void CLogServerHandler::RequestEntry(CLogServerProtocol& aoRequest, CLogServerProtocol& aoResponse)
{
    CLogServerMsg* lpoLogServerMsg;

    //get the message we need to use.
    lpoLogServerMsg = aoRequest.GetLogServerMsg();

    //dlopen so and got the caller ?
    
    //call the dynamic lib and get response 
    
    //filled the response protocol, ONE_WAY or TWO_WAY
}
