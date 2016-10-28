
#include "acl/SockStream.h"
#include "apl/poll.h"

ACL_NAMESPACE_START

apl_ssize_t CSockStream::Recv( 
    void* apvBuffer, 
    apl_size_t aiN,
    EOption aeFlag ) const
{
    apl_ssize_t liResult = 0;
    
    //If liRetCode = EINTR or EAGAIN .. and retry it
    ACL_RETRY_CHK(
        (liResult = apl_recv(this->GetHandle(), apvBuffer, aiN, aeFlag) ) < 0, 5 );
    if (liResult < 0)
    {
        return -1;
    }
    else if (liResult == 0)
    {
        //Connection closed by peer
        apl_set_errno(APL_ECONNRESET);
        return -1;
    }
    
    return liResult;
}

apl_ssize_t CSockStream::Recv(
    void* apvBuffer, 
    apl_size_t aiN ) const
{
    return this->Recv( apvBuffer, aiN, OPT_UNSPEC );
}

apl_ssize_t CSockStream::Read(
    void* apvBuffer, 
    apl_size_t aiN ) const
{
    return this->Recv( apvBuffer, aiN, OPT_UNSPEC );
}

apl_ssize_t CSockStream::RecvV(
    apl_iovec_t apoIoVec[], 
    apl_size_t aiN ) const
{
    apl_ssize_t liResult = 0;
    
    //If liRetCode = EINTR or EAGAIN .. and retry it
    ACL_RETRY_CHK(
        (liResult = apl_readv(this->GetHandle(), apoIoVec, aiN) ) < 0, 5 );
    if (liResult < 0)
    {
        return -1;
    }
    else if (liResult == 0)
    {
        //Connection closed by peer
        apl_set_errno(APL_ECONNRESET);
        return -1;
    }
    
    return liResult;
}

apl_ssize_t CSockStream::Recv( 
    void* apvBuffer,
    apl_size_t aiN,
    EOption aeFlag,
    CTimeValue const& aoTimeout ) const
{
    apl_ssize_t liResult      = 0;
    apl_size_t  liNeedSize    = aiN;
    char*       lpcCurrBuffer = (char*)apvBuffer;
    apl_time_t  li64Timeout   = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );

    while( liNeedSize > 0 )
    {
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liResult = apl_poll_one(this->GetHandle(), APL_POLLIN, NULL, li64Timeout) ) < 0, 5 );
        if (liResult < 0)
        {
            return -1;
        }
        else if (liResult == 0)
        {
            //Wait event timeout
            apl_set_errno(APL_ETIMEDOUT);
            return aiN - liNeedSize > 0 ? (apl_ssize_t)(aiN - liNeedSize) : -1;
        }
        
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liResult = apl_recv(this->GetHandle(), lpcCurrBuffer, liNeedSize, aeFlag) ) < 0, 5 );
        if (liResult < 0)
        {
            return -1;
        }
        else if (liResult == 0)
        {
            //Connection closed by peer
            apl_set_errno(APL_ECONNRESET);
            return -1;
        }
        
        liNeedSize -= liResult;
        lpcCurrBuffer += liResult;
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
    apl_ssize_t liResult    = 0;
    apl_size_t  liCmplSize  = 0;
    apl_time_t  li64Timeout = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );

    for ( apl_size_t liN = 0; liN < aiN; )
    {
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liResult = apl_poll_one(this->GetHandle(), APL_POLLIN, NULL, li64Timeout) ) < 0, 5 );
        if (liResult < 0)
        {
            return -1;
        }
        else if (liResult == 0)
        {
            //Wait event timeout
            apl_set_errno(APL_ETIMEDOUT);
            return liCmplSize > 0 ? (apl_ssize_t)liCmplSize : -1;
        }
        
        ACL_RETRY_CHK(
		    (liResult = apl_readv(this->GetHandle(), apoIoVec + liN, aiN - liN) ) < 0, 5 );
		if (liResult == 0)
		{
			//EOF
			break;
		}
		else if (liResult < 0)
		{
			//Read error
			return -1;
		}

        for ( liCmplSize += liResult; liN < aiN
             && liResult >= static_cast<apl_ssize_t>(apoIoVec[liN].iov_len);
              liN++)
        {
            liResult -= apoIoVec[liN].iov_len;
        }
        
        if (liResult != 0)
        {
            char* lpcBase = reinterpret_cast<char*>(apoIoVec[liN].iov_base);
            apoIoVec[liN].iov_base = lpcBase + liResult;
            apoIoVec[liN].iov_len  = apoIoVec[liN].iov_len - liResult;
        }
    }

    return liCmplSize;
}

apl_ssize_t CSockStream::RecvUntil(
    void* apvBuffer, 
    apl_size_t aiN,
    char const* apcTag,
    EOption aeFlag,
    CTimeValue const& aoTimeout ) const
{
    apl_ssize_t liResult      = 0;
    char*       lpcUntil      = NULL;
    apl_size_t  liNeedSize    = aiN;
    char*       lpcCurrBuffer = (char*)apvBuffer;
    apl_size_t  liTagLen      = apl_strlen(apcTag);
    apl_time_t  li64Timeout   = aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec();

    while( liNeedSize > 0 )
    {
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liResult = apl_poll_one(this->GetHandle(), APL_POLLIN, NULL, li64Timeout) ) < 0, 5 );
        if (liResult < 0)
        {
            return -1;
        }
        else if (liResult == 0)
        {
            //Wait event timeout
            apl_set_errno(APL_ETIMEDOUT);
            return lpcCurrBuffer - (char*)apvBuffer > 0 ? 
                lpcCurrBuffer - (char*)apvBuffer : -1;
        }
        
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liResult = apl_recv(this->GetHandle(), lpcCurrBuffer, liNeedSize, APL_MSG_PEEK ) ) < 0, 5 );
        if (liResult < 0)
        {
            return -1;
        }
        else if (liResult == 0)
        {
            //Connection closed by peer
            apl_set_errno(APL_ECONNRESET);
            return -1;
        }
        
        //Set end character for strstr
        lpcCurrBuffer[ (apl_size_t)liResult < liNeedSize ? liResult : liNeedSize - 1 ] = '\0';
        
        if ( aeFlag == OPT_PEEK )
        {
            //Search tag token in curr buffer
            if ( (lpcUntil = apl_strstr(
                lpcCurrBuffer - liTagLen >= (char*)apvBuffer? lpcCurrBuffer - liTagLen : (char*)apvBuffer, apcTag ) ) != NULL )
            {
                //Hit, complete
                lpcCurrBuffer = lpcUntil + liTagLen;
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
                liNeedSize = lpcUntil - lpcCurrBuffer + liTagLen;
            }
        }
        
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liResult = apl_recv(this->GetHandle(), lpcCurrBuffer, liNeedSize, aeFlag) ) < 0, 5);
        if (liResult < 0)
        {
            return -1;
        }

        liNeedSize -= liResult;
        lpcCurrBuffer += liResult;
    }
    
    return lpcCurrBuffer - (char*)apvBuffer;
}

apl_ssize_t CSockStream::RecvUntil(
    void* apvBuffer,
    apl_size_t aiN,
    char const* apcTag, 
    CTimeValue const& aoTimeout ) const
{
    return this->RecvUntil( apvBuffer, aiN, apcTag, OPT_UNSPEC, aoTimeout);
}

apl_ssize_t CSockStream::Send(
    void const* apvBuffer, 
    apl_size_t aiN,
    EOption aeFlag ) const
{
    apl_int_t liResult = 0;
    
    //If liRetCode = EINTR or EAGAIN .. and retry it
    ACL_RETRY_CHK(
        (liResult = apl_send(this->GetHandle(), apvBuffer, aiN, aeFlag) ) < 0, 5);
    if (liResult < 0)
    {
        return -1;
    }
    
    return liResult;
}

apl_ssize_t CSockStream::Send(
    void const* apvBuffer, 
    apl_size_t aiN ) const
{
    return this->Send( apvBuffer, aiN, OPT_UNSPEC );
}

apl_ssize_t CSockStream::Write(
    void const* apvBuffer, 
    apl_size_t aiN ) const
{
    return this->Send( apvBuffer, aiN, OPT_UNSPEC );
}

apl_ssize_t CSockStream::SendV(
    apl_iovec_t const apoIoVec[], 
    apl_size_t aiN ) const
{
    apl_int_t liResult = 0;
    
    //If liRetCode = EINTR or EAGAIN .. and retry it
    ACL_RETRY_CHK(
        (liResult = apl_writev(this->GetHandle(), apoIoVec, aiN) ) < 0, 5);
    if (liResult < 0)
    {
        return -1;
    }
    
    return liResult;
}

apl_ssize_t CSockStream::Send(
    void const* apvBuffer, 
    apl_size_t aiN,
    EOption aeFlag,
    CTimeValue const& aoTimeout ) const
{
    apl_ssize_t liResult      = 0;
    apl_size_t  liNeedSize    = aiN;
    char*       lpcCurrBuffer = (char*)apvBuffer;
    apl_time_t  li64Timeout   = aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec();
    
    while( liNeedSize > 0 )
    {
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liResult = apl_poll_one(this->GetHandle(), APL_POLLOUT, NULL, li64Timeout) ) < 0, 5);
        if (liResult < 0)
        {
            return -1;
        }
        else if (liResult == 0)
        {
            //Wait event timeout
            apl_set_errno(APL_ETIMEDOUT);
            return aiN - liNeedSize > 0 ? (apl_ssize_t)(aiN - liNeedSize) : -1;
        }
        
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liResult = apl_send(this->GetHandle(), lpcCurrBuffer, liNeedSize, aeFlag) ) < 0, 5);
        if (liResult < 0)
        {
            return -1;
        }
        
        liNeedSize -= liResult;
        lpcCurrBuffer += liResult;
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

apl_ssize_t CSockStream::SendV( CMemoryBlock* apoMemoryBlock, CTimeValue const& aoTimeout ) const
{
    apl_size_t       liCmplSize   = 0;
    const apl_size_t luMaxVecSize = 16;
    apl_iovec_t      laoIoVec[luMaxVecSize];
    apl_size_t       luIoVecCnt = 0;
    
    while(apoMemoryBlock != NULL)
    {
        //Our current memory block chain.
        const CMemoryBlock* lpoCurrentBlock = apoMemoryBlock;

        while(lpoCurrentBlock != NULL)
        {
            char* lpcThisBlockPtr = lpoCurrentBlock->GetReadPtr();
            apl_size_t luCurrentBlockLength = lpoCurrentBlock->GetLength();

            // Check if this block has any data to be sent.
            while (luCurrentBlockLength > 0)
            {
                apl_int32_t liThisChunkLength = 0;
                if (luCurrentBlockLength > INT32_MAX)
                {
                    liThisChunkLength = INT32_MAX;
                }
                else
                {
                    liThisChunkLength = static_cast<apl_int32_t>(luCurrentBlockLength);
                }
                
                // Collect the data in the iovec.
                laoIoVec[luIoVecCnt].iov_base = lpcThisBlockPtr;
                laoIoVec[luIoVecCnt].iov_len  = liThisChunkLength;
                luCurrentBlockLength  -= liThisChunkLength;
                lpcThisBlockPtr += liThisChunkLength;

                // Increment iovec counter.
                ++luIoVecCnt;

                if (luIoVecCnt == luMaxVecSize)
                {
                    apl_ssize_t liResult = this->SendV(laoIoVec, luIoVecCnt, aoTimeout);
                    if (liResult < 0)
                    {
                        return liResult;
                    }
                    // Add to total bytes transferred.
                    liCmplSize += liResult;

                    // Reset iovec counter.
                    luIoVecCnt = 0;
                }
            }

            // Select the next memory block in the chain.
            lpoCurrentBlock = lpoCurrentBlock->GetCont();
        }

        // Selection of the next memory block chain.
        apoMemoryBlock = apoMemoryBlock->GetNext();
    }

    // Check for remaining buffers to be sent.  This will happen when
    // luMaxVecSize is not a multiple of the number of memory blocks.
    if (luIoVecCnt != 0)
    {
        apl_ssize_t liResult = this->SendV(laoIoVec, luIoVecCnt, aoTimeout);
        if (liResult < 0)
        {
            return liResult;
        }
        
        // Add to total bytes transferred.
        liCmplSize += liResult;
    }

    // Return total bytes transferred.
    return liCmplSize;
}
 
apl_ssize_t CSockStream::SendV(
    apl_iovec_t const apoIoVec[], 
    apl_size_t aiN, 
    CTimeValue const& aoTimeout ) const
{
    apl_ssize_t liResult    = 0;
    apl_size_t  liCmplSize  = 0;
    apl_time_t  li64Timeout = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );
        
    apl_iovec_t* lpoIoVec = const_cast<apl_iovec_t*>(apoIoVec);

    for ( apl_size_t liN = 0; liN < aiN; )
    {
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liResult = apl_poll_one(this->GetHandle(), APL_POLLOUT, NULL, li64Timeout) ) < 0, 5 );
        if (liResult < 0)
        {
            return -1;
        }
        else if (liResult == 0)
        {
            //Wait event timeout
            apl_set_errno(APL_ETIMEDOUT);
            return liCmplSize > 0 ? (apl_ssize_t)liCmplSize : -1;
        }
        
        ACL_RETRY_CHK(
		    (liResult = apl_writev(this->GetHandle(), lpoIoVec + liN, aiN - liN) ) < 0, 5 );
		if (liResult == 0)
		{
			//EOF
			break;
		}
		else if (liResult < 0)
		{
			//Read error
			return -1;
		}

        for ( liCmplSize += liResult; liN < aiN
             && liResult >= static_cast<apl_ssize_t>(lpoIoVec[liN].iov_len);
              liN++)
        {
            liResult -= lpoIoVec[liN].iov_len;
        }
        
        if (liResult != 0)
        {
            char* lpcBase = reinterpret_cast<char*>(lpoIoVec[liN].iov_base);
            lpoIoVec[liN].iov_base = lpcBase + liResult;
            lpoIoVec[liN].iov_len  = lpoIoVec[liN].iov_len - liResult;
        }
    }

    return liCmplSize;
}

ACL_NAMESPACE_END
