
#include "anf/IoSession.h"
#include "anf/IoService.h"
#include "anf/IoFilterChain.h"
#include "anf/IoProcessor.h"

ANF_NAMESPACE_START

//////////////////////////////////////////////////////////////////////////////////////
CIoSession::CIoSession(void)
    : mpoProcessor(NULL)
    , mpoHandler(NULL)
    , mu64SessionID(0)
    , miEvents(APL_POLLIN)
    , mbIsReadSuspended(false)
    , mbIsWriteSuspended(true)
    , moReadBuffer(NULL, acl::CMemoryBlock::DONT_DELETE)
{
    this->Reset();
}
    
CIoSession::~CIoSession(void)
{
    if (this->GetHandle() != ACL_INVALID_HANDLE)
    {
        acl::CSockHandle::Close();
    }

    //Debug trace point
    //ACL_ASSERT(this->GetWriteRequestQueue().Size() == 0);
}

void CIoSession::Close( SessionPtrType& aoSession )
{
    if (!aoSession->mbIsClosing)
    {
        aoSession->mbIsClosing = true;
        aoSession->UpdateReadClosed(true);
        aoSession->UpdateWriteClosed(true);
        if (aoSession->GetService()->GetConfiguration()->GetMaxQueue() > 0)
        {
            aoSession->GetWriteRequestCond().Broadcast();
        }

        aoSession->moFilterChain.FireFilterClose(aoSession);
    }
}

void CIoSession::Reset(void)
{
    this->muIdleCount = 0;
    this->muReadMessages = 0;
    this->mu64ReadBytes = 0;
    this->muWrittenMessages = 0;
    this->mu64WrittenBytes = 0;
    this->mbIsClosing = false;
    this->mbIsReadClosed = false;
    this->mbIsWriteClosed = false;
    this->moActivateTime.Update(acl::CTimestamp::PRC_USEC);
    this->moLastReadTime = this->moActivateTime;
    this->moLastReadIdleTime = this->moActivateTime;
    this->moLastWriteTime = this->moActivateTime;
    this->moLastWriteIdleTime = this->moActivateTime;
}


void CIoSession::SetSessionID( apl_uint64_t au64SessionID )
{
    this->mu64SessionID = au64SessionID;
}
    
apl_uint64_t CIoSession::GetSessionID(void)
{
    return this->mu64SessionID;
}

void CIoSession::SetProcessor( IoProcessor* apoProcessor )
{
    this->mpoProcessor = apoProcessor;
}
    
IoProcessor* CIoSession::GetProcessor(void)
{
    return this->mpoProcessor;
}

void CIoSession::SetHandle( apl_handle_t aiHandle )
{
    this->moStream.SetHandle(aiHandle);
    this->moDgram.SetHandle(aiHandle);
    acl::CSockHandle::SetHandle(aiHandle);
}

void CIoSession::SetAddress( acl::CSockAddr const& aoLocalAddress, acl::CSockAddr const& aoRemoteAddress )
{
    this->moLocalAddress.Set(aoLocalAddress);
    this->moRemoteAddress.Set(aoRemoteAddress);
}

acl::CSockAddr& CIoSession::GetLocalAddress(void)
{
    return this->moLocalAddress;
}

acl::CSockAddr& CIoSession::GetRemoteAddress(void)
{
    return this->moRemoteAddress;
}

void CIoSession::SetHandler( IoHandler* apoHandler )
{
    this->mpoHandler = apoHandler;
}
    
IoHandler* CIoSession::GetHandler(void)
{
    return this->mpoHandler;
}

void CIoSession::SetService( CIoService* apoService )
{
    this->mpoService = apoService;
}
    
CIoService* CIoSession::GetService(void)
{
    return this->mpoService;
}

void CIoSession::SetReadBufferSize( apl_size_t auBufferSize )
{
    this->moReadBuffer.Resize(auBufferSize);
}

apl_size_t CIoSession::GetReadBufferSize(void)
{
    return this->moReadBuffer.GetSize();
}

acl::CMemoryBlock* CIoSession::GetReadBuffer(void)
{
    return &this->moReadBuffer;
}
    
CIoFilterChain* CIoSession::GetFilterChain(void)
{
    return &this->moFilterChain;
}

CIoSession::WriteRequestQueueType& CIoSession::GetWriteRequestQueue(void)
{
    return this->moWriteRequests;
}

acl::CCondition& CIoSession::GetWriteRequestCond(void)
{
    return this->moWriteRequestsCond;
}

acl::CLock& CIoSession::GetWriteRequestLock(void)
{
    return this->moWriteRequestsLock;
}

CIoSessionAttribute& CIoSession::GetAttribute(void)
{
    return this->moAttribute;
}

void CIoSession::IncreaseReadMessages( apl_size_t auIncrement )
{
    this->muReadMessages += auIncrement;
}

void CIoSession::IncreaseReadBytes( apl_uint64_t auIncrement, acl::CTimestamp const& aoCurrTime )
{
    this->mu64ReadBytes += auIncrement;
    this->moLastReadTime = aoCurrTime;
}

void CIoSession::IncreaseWrittenMessages( apl_size_t auIncrement )
{
    this->muWrittenMessages += auIncrement;
}

void CIoSession::IncreaseWrittenBytes( apl_uint64_t auIncrement, acl::CTimestamp const& aoCurrTime )
{
    this->mu64WrittenBytes += auIncrement;
    this->moLastWriteTime = aoCurrTime;
}

apl_size_t CIoSession::GetIdleCount(void)
{
    return this->muIdleCount;
}

apl_size_t CIoSession::GetReadMessages(void)
{
    return this->muReadMessages;
}

apl_uint64_t CIoSession::GetReadBytes(void)
{
    return this->mu64ReadBytes;
}

apl_size_t CIoSession::GetWrittenMessages(void)
{
    return this->muWrittenMessages;
}

apl_uint64_t CIoSession::GetWrittenBytes(void)
{
    return this->mu64WrittenBytes;
}
    
acl::CTimestamp& CIoSession::GetActivateTime(void)
{
    return this->moActivateTime;
}

acl::CTimestamp& CIoSession::GetLastReadTime(void)
{
    return this->moLastReadTime;
}

acl::CTimestamp& CIoSession::GetLastReadIdleTime(void)
{
    return this->moLastReadIdleTime;
}

acl::CTimestamp& CIoSession::GetLastWriteTime(void)
{
    return this->moLastWriteTime;
}

acl::CTimestamp& CIoSession::GetLastWriteIdleTime(void)
{
    return this->moLastWriteIdleTime;
}

void CIoSession::SuspendRead( SessionPtrType& aoSession )
{
    this->mbIsReadSuspended = true;
    this->GetProcessor()->UpdateTrafficControl(aoSession);
}

void CIoSession::SuspendWrite( SessionPtrType& aoSession )
{
    this->mbIsWriteSuspended = true;
    this->GetProcessor()->UpdateTrafficControl(aoSession);
}

void CIoSession::ResumeRead( SessionPtrType& aoSession )
{
    this->mbIsReadSuspended = false;
    this->GetProcessor()->UpdateTrafficControl(aoSession);
}

void CIoSession::ResumeWrite( SessionPtrType& aoSession )
{
    this->mbIsWriteSuspended = false;
    this->GetProcessor()->UpdateTrafficControl(aoSession);
}

bool CIoSession::IsReadSuspended(void)
{
    return this->mbIsReadSuspended;
}

bool CIoSession::IsWriteSuspended(void)
{
    return this->mbIsWriteSuspended;
}

bool CIoSession::IsClosing(void)
{
    return this->mbIsClosing;
}

bool CIoSession::IsReadClosed(void)
{
    return this->mbIsReadClosed;
}

bool CIoSession::IsWriteClosed(void)
{
    return this->mbIsWriteClosed;
}

bool CIoSession::IsClosed(void)
{
    return this->GetHandle() == ACL_INVALID_HANDLE;
}

void CIoSession::NotifyIdleSession( SessionPtrType& aoSession, acl::CTimestamp const& aoCurrTime )
{
    {
        acl::CTimeValue const& loIdleTime = this->GetService()->GetConfiguration()->GetReadIdleTime();
        acl::CTimestamp  loLastTime = std::max(this->GetLastReadIdleTime(), this->GetLastReadTime() );
        
        if (aoCurrTime.Sec() - loLastTime.Sec() > loIdleTime.Sec() )
        {
            this->GetFilterChain()->FireSessionIdle(aoSession, SESSION_READ_IDLE);
            this->moLastReadIdleTime = aoCurrTime;
        }
    }
    
    {
        acl::CTimeValue const& loIdleTime = this->GetService()->GetConfiguration()->GetWriteIdleTime();
        acl::CTimestamp  loLastTime = std::max(this->GetLastWriteIdleTime(), this->GetLastWriteTime() );
        
        if (aoCurrTime.Sec() - loLastTime.Sec() > loIdleTime.Sec() )
        {
            this->GetFilterChain()->FireSessionIdle(aoSession, SESSION_WRITE_IDLE);
            this->moLastWriteIdleTime = aoCurrTime;
        }
    }
    
    this->muIdleCount += 1;
}

apl_int_t CIoSession::SetReadSuspended( bool abIsEnable )
{
    this->mbIsReadSuspended = abIsEnable;
    return 0;
}

apl_int_t CIoSession::SetWriteSuspended( bool abIsEnable )
{
    this->mbIsWriteSuspended = abIsEnable;
    return 0;
}

void CIoSession::SetEvent( apl_int_t aiEvents )
{
    this->miEvents = aiEvents;
}

apl_int_t CIoSession::GetEvent(void)
{
    return this->miEvents;
}

apl_ssize_t CIoSession::Recv( void* apvBuffer, apl_size_t auLen )
{
    return this->moStream.Recv(apvBuffer, auLen);
}

apl_ssize_t CIoSession::Recv( void* apvBuffer, apl_size_t auLen, acl::CSockAddr* apoRemoteAddress )
{
    return this->moDgram.Recv(apvBuffer, auLen, apoRemoteAddress);
}

apl_ssize_t CIoSession::Send( void const* apvBuffer, apl_size_t auLen )
{
    return this->moStream.Send(apvBuffer, auLen);
}

apl_ssize_t CIoSession::Send( void const* apvBuffer, apl_size_t auLen, acl::CSockAddr const& aoRemoteAddress )
{
    return this->moDgram.Send(apvBuffer, auLen, aoRemoteAddress);
}

void CIoSession::UpdateReadClosed( bool abIsEnable )
{
    this->mbIsReadClosed = abIsEnable;
}

void CIoSession::UpdateWriteClosed( bool abIsEnable )
{
    this->mbIsWriteClosed = abIsEnable;
}

ANF_NAMESPACE_END
