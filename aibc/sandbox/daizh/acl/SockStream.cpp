
#include "SockStream.h"
#include "apl/poll.h"

ACL_NAMESPACE_START

apl_ssize_t CSockStream::Recv( 
    void* apvBuffer,
    apl_size_t aiN,
    EOption aeFlag,
    CTimeValue const& aoTimeout ) const
{
    apl_int_t  liRetCode     = 0;
    apl_size_t liNeedSize    = aiN;
    char*      lpcCurrBuffer = (char*)apvBuffer;
    apl_time_t li64Timeout   = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );
    
    //Check socket handle
    if ( !ACL_IS_VALID_SOCKET(this->GetHandle() ) )
    {
        apl_set_errno(APL_EINVAL);
        return -1;
    }
    
    while( liNeedSize > 0 )
    {
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liRetCode = apl_poll_one(this->GetHandle(), APL_POLLIN, NULL, li64Timeout) ) < 0, 5 );
        if (liRetCode < 0)
        {
            return -1;
        }
        else if ( liRetCode == 0 )
        {
            //Wait event timeout
            apl_set_errno(APL_EAGAIN);
            break;
        }
        
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liRetCode = apl_recv(this->GetHandle(), lpcCurrBuffer, liNeedSize, aeFlag) ) < 0, 5 );
        if (liRetCode < 0)
        {
            return -1;
        }
        else if (liRetCode == 0)
        {
            //Connection closed by peer
            apl_set_errno(APL_ECONNRESET);
            return -1;
        }
        
        liNeedSize -= liRetCode;
        lpcCurrBuffer += liRetCode;
    }
    
    return aiN - liNeedSize;
}

apl_ssize_t CSockStream::Recv(
    void* apvBuffer, 
    apl_size_t aiN, 
    CTimeValue const& aoTimeout ) const
{
    return this->Recv(apvBuffer, aiN, OPT_UNSPEC, aoTimeout);
}

apl_ssize_t CSockStream::RecvV(
    apl_iovec_t apoIoVec[], 
    apl_size_t aiN, 
    CTimeValue const& aoTimeout ) const
{
    apl_int_t    liRetCode     = 0;
    apl_size_t   liNeedSize    = 0;
    apl_size_t   liNeedCount   = aiN;
    apl_iovec_t* lpoDupIoVec   = NULL;
    apl_iovec_t* lpoCurrIoVec  = NULL;
    apl_time_t   li64Timeout   = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );
    
    //Check socket handle
    if ( !ACL_IS_VALID_SOCKET(this->GetHandle() ) )
    {
        apl_set_errno(APL_EINVAL);
        return -1;
    }
    
    //Calculate apoIoVec total length
    for ( apl_size_t liIdx = 0; liIdx < aiN; liIdx++ )
    {
        liNeedSize += apoIoVec[liIdx].iov_len;
    }
    
    //Dup apoIoVec, may be will modify it
    ACL_NEW_N_ASSERT( lpoDupIoVec, apl_iovec_t, aiN );
    apl_memcpy( lpoDupIoVec, apoIoVec, sizeof(*apoIoVec) * aiN );
    
    //Set first iovec to lpoCurrIoVec
    lpoCurrIoVec = lpoDupIoVec;
    while( liNeedSize > 0 )
    {
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liRetCode = apl_poll_one(this->GetHandle(), APL_POLLIN, NULL, li64Timeout) ) < 0, 5 );
        if (liRetCode < 0)
        {
            ACL_DELETE_N(lpoDupIoVec);
            return -1;
        }
        else if ( liRetCode == 0 )
        {
            //Wait event timeout
            apl_set_errno(APL_EAGAIN);
            break;
        }
        
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liRetCode = apl_readv(this->GetHandle(), lpoCurrIoVec, liNeedCount) ) < 0, 5 );
        if (liRetCode < 0)
        {
            ACL_DELETE_N(lpoDupIoVec);
            return -1;
        }
        else if (liRetCode == 0)
        {
            //Connection closed by peer
            apl_set_errno(APL_ECONNRESET);
            ACL_DELETE_N(lpoDupIoVec);
            return -1;
        }
        
        liNeedSize -= liRetCode;
        
        if ( liNeedSize > 0 )
        {
            //Recalculate next iovec to readv
            apl_size_t liTmpSum = 0;
            for ( apl_size_t liIdx = 0; liIdx < liNeedCount; liIdx++ )
            {
                liTmpSum += lpoCurrIoVec[liIdx].iov_len;
                if (liTmpSum == (apl_size_t)liRetCode)
                {
                    //Skip completed iovec only
                    lpoCurrIoVec = &lpoCurrIoVec[liIdx];
                    liNeedCount -= liIdx;
                }
                else if (liTmpSum > (apl_size_t)liRetCode)
                {
                    //Skip completed iovec and offset iov_base
                    lpoCurrIoVec = &lpoCurrIoVec[liIdx];
                    lpoCurrIoVec->iov_base = 
                        (char*)(lpoCurrIoVec->iov_base) + (lpoCurrIoVec->iov_len - liTmpSum + liRetCode);
                    liNeedCount -= liIdx;
                }
            }
        }
    }
    
    ACL_DELETE_N(lpoDupIoVec);
    
    return aiN - liNeedSize;
}

apl_ssize_t CSockStream::RecvUntil(
    char const* apcTag,
    void* apvBuffer, 
    apl_size_t aiN,
    EOption aeFlag,
    CTimeValue const& aoTimeout ) const
{
    apl_int_t  liRetCode     = 0;
    char*      lpcUntil      = NULL;
    apl_size_t liNeedSize    = aiN;
    char*      lpcCurrBuffer = (char*)apvBuffer;
    apl_size_t liTagLen      = apl_strlen(apcTag);
    apl_time_t li64Timeout   = aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec();
    
    //Check socket handle
    if ( !ACL_IS_VALID_SOCKET(this->GetHandle() ) )
    {
        return -1;
    }
    
    while( liNeedSize > 0 )
    {
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liRetCode = apl_poll_one(this->GetHandle(), APL_POLLIN, NULL, li64Timeout) ) < 0, 5 );
        if (liRetCode < 0)
        {
            return -1;
        }
        else if (liRetCode == 0)
        {
            //Wait event timeout
            apl_set_errno(APL_EAGAIN);
            break;
        }
        
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liRetCode = apl_recv(this->GetHandle(), lpcCurrBuffer, liNeedSize, APL_MSG_PEEK ) ) < 0, 5 );
        if (liRetCode < 0)
        {
            return -1;
        }
        else if (liRetCode == 0)
        {
            //Connection closed by peer
            apl_set_errno(APL_ECONNRESET);
            return -1;
        }
        
        //Set end character for strstr
        lpcCurrBuffer[liNeedSize-1] = '\0';
        
        if ( aeFlag == OPT_PEEK )
        {
            //Search tag token in curr buffer
            if ( (lpcUntil = apl_strstr(
                lpcCurrBuffer - liTagLen >= (char*)apvBuffer? lpcCurrBuffer - liTagLen : (char*)apvBuffer, apcTag ) ) != NULL )
            {
                //Hit, complete
                lpcCurrBuffer = lpcUntil;
                liNeedSize    = 0;
                break;
            }
        }
        else
        {
            //Search tag token in curr buffer
            if ( (lpcUntil = apl_strstr(
                lpcCurrBuffer - liTagLen >= (char*)apvBuffer? lpcCurrBuffer - liTagLen : (char*)apvBuffer, apcTag ) ) != NULL )
            {
                //Hit, offset liNeedSize for recv
                liNeedSize = lpcUntil - lpcCurrBuffer;
            }
        }
        
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liRetCode = apl_recv(this->GetHandle(), lpcCurrBuffer, liNeedSize, aeFlag) ) < 0, 5);
        if (liRetCode < 0)
        {
            return -1;
        }

        liNeedSize -= liRetCode;
        lpcCurrBuffer += liRetCode;
    }
    
    return aiN - liNeedSize;
}

apl_ssize_t CSockStream::RecvUntil(
    char const* apcTag, 
    void* apvBuffer, 
    apl_size_t aiN,
    CTimeValue const& aoTimeout ) const
{
    return this->RecvUntil(apcTag, apvBuffer, aiN, OPT_UNSPEC, aoTimeout);
}

apl_ssize_t CSockStream::Send(
    void const* apvBuffer, 
    apl_size_t aiN,
    EOption aeFlag,
    CTimeValue const& aoTimeout ) const
{
    apl_int_t  liRetCode     = 0;
    apl_size_t liNeedSize    = aiN;
    char*      lpcCurrBuffer = (char*)apvBuffer;
    apl_time_t li64Timeout   = aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec();
    
    //Check socket handle
    if ( !ACL_IS_VALID_SOCKET(this->GetHandle() ) )
    {
        return -1;
    }
    
    while( liNeedSize > 0 )
    {
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liRetCode = apl_poll_one(this->GetHandle(), APL_POLLOUT, NULL, li64Timeout) ) < 0, 5);
        if (liRetCode < 0)
        {
            return -1;
        }
        else if ( liRetCode == 0 )
        {
            //Wait event timeout
            apl_set_errno(APL_EAGAIN);
            break;
        }
        
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liRetCode = apl_send(this->GetHandle(), lpcCurrBuffer, liNeedSize, aeFlag) ) < 0, 5);
        if (liRetCode < 0)
        {
            return -1;
        }
        
        liNeedSize -= liRetCode;
        lpcCurrBuffer += liRetCode;
    }
    
    return aiN - liNeedSize;
}

apl_ssize_t CSockStream::Send(
    void const* apvBuffer, 
    apl_size_t aiN, 
    CTimeValue const& aoTimeout ) const
{
    return this->Send(apvBuffer, aiN, OPT_UNSPEC, aoTimeout);
}
 
apl_ssize_t CSockStream::SendV(
    apl_iovec_t const apoIoVec[], 
    apl_size_t aiN, 
    CTimeValue const& aoTimeout ) const
{
    apl_int_t    liRetCode     = 0;
    apl_size_t   liNeedSize    = 0;
    apl_size_t   liNeedCount   = aiN;
    apl_iovec_t* lpoDupIoVec   = NULL;
    apl_iovec_t* lpoCurrIoVec  = NULL;
    apl_time_t   li64Timeout   = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );
    
    //Check socket handle
    if ( !ACL_IS_VALID_SOCKET(this->GetHandle() ) )
    {
        return -1;
    }
    
    //Calculate apoIoVec total length
    for ( apl_size_t liIdx = 0; liIdx < aiN; liIdx++ )
    {
        liNeedSize += apoIoVec[liIdx].iov_len;
    }
    
    //Dup apoIoVec, may be will modify it
    ACL_NEW_N_ASSERT( lpoDupIoVec, apl_iovec_t, aiN );
    apl_memcpy( lpoDupIoVec, apoIoVec, sizeof(*apoIoVec) * aiN );
    
    lpoCurrIoVec = lpoDupIoVec;
    
    while( liNeedSize > 0 )
    {
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liRetCode = apl_poll_one(this->GetHandle(), APL_POLLIN, NULL, li64Timeout) ) < 0, 5 );
        if (liRetCode < 0)
        {
            ACL_DELETE_N(lpoDupIoVec);
            return -1;
        }
        else if ( liRetCode == 0 )
        {
            //Wait event timeout
            apl_set_errno(APL_EAGAIN);
            break;
        }
        
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liRetCode = apl_writev(this->GetHandle(), lpoCurrIoVec, liNeedCount) ) < 0, 5 );
        if (liRetCode < 0)
        {
            ACL_DELETE_N(lpoDupIoVec);
            return -1;
        }
        
        liNeedSize -= liRetCode;
        
        if ( liNeedSize > 0 )
        {
            //Recalculate next iovec to writev
            apl_size_t liTmpSum = 0;
            for ( apl_size_t liIdx = 0; liIdx < liNeedCount; liIdx++ )
            {
                liTmpSum += lpoCurrIoVec[liIdx].iov_len;
                if (liTmpSum == (apl_size_t)liRetCode)
                {
                    //Skip completed iovec only
                    lpoCurrIoVec = &lpoCurrIoVec[liIdx];
                    liNeedCount -= liIdx;
                }
                else if (liTmpSum > (apl_size_t)liRetCode)
                {
                    //Skip completed iovec, and offset voc_base
                    lpoCurrIoVec = &lpoCurrIoVec[liIdx];
                    lpoCurrIoVec->iov_base = 
                        (char*)(lpoCurrIoVec->iov_base) + (lpoCurrIoVec->iov_len - liTmpSum + liRetCode);
                    liNeedCount -= liIdx;
                }
            }
        }
    }
    
    ACL_DELETE_N(lpoDupIoVec);
    
    return aiN - liNeedSize;
}

ACL_NAMESPACE_END
