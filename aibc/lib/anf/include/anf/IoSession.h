
#ifndef ANF_IOSESSION_H
#define ANF_IOSESSION_H

#include "acl/SmartPtr.h"
#include "acl/SockStream.h"
#include "acl/SockDgram.h"
#include "acl/Timestamp.h"
#include "anf/Utility.h"
#include "anf/ConcurrentQueue.h"
#include "anf/IoFilterChain.h"
#include "anf/future/WriteFuture.h"
#include "anf/IoSessionAttribute.h"

ANF_NAMESPACE_START

//forward declare
class IoProcessor;
class IoHandler;
class CIoFilterChain;
class CIoService;

enum ESessionIdleStatus
{
    SESSION_READ_IDLE,
    SESSION_WRITE_IDLE
};

class CIoSession : public acl::CSockHandle
{
    friend class CIoSessionAdapter;

public:
    typedef acl::TSharePtr<CIoSession, acl::CLock> PointerType;
    typedef TConcurrentQueue<CWriteFuture::PointerType> WriteRequestQueueType;
        
public:
    /**
     * Default constructor
     */
    CIoSession(void);
    
    /**
     * Default destructor
     */
    ~CIoSession(void);

    /** 
     * @brief Close session and release all related resource
     * @param[in] aoSession close session
     */
    void Close( SessionPtrType& aoSession );

    /**
     * Sets session id.
     */
    void SetSessionID( apl_uint64_t au64SessionID );
    
    /**
     * Returns session id.
     */
    apl_uint64_t GetSessionID(void);
    
    /**
     * Sets session's processor.
     */
    void SetProcessor( IoProcessor* apoProcessor );
    
    /**
     * Returns session's processor.
     */
    IoProcessor* GetProcessor(void);
    
    /**
     * Sets the session socket handle
     */
    void SetHandle( apl_handle_t aiHandle );

    /** 
     * @brief Sets the session local and remote address
     * 
     * @param aoLocalAddress local address
     * @param aoRemoteAddress remote address
     */
    void SetAddress( acl::CSockAddr const& aoLocalAddress, acl::CSockAddr const& aoRemoteAddress );
       
    /** 
     * @brief Gets remote peer address
     * 
     * @returns remote address
     */
    acl::CSockAddr& GetRemoteAddress(void);

    /** 
     * @brief Gets local peer address
     * 
     * @returns local address
     */
    acl::CSockAddr& GetLocalAddress(void);
    
    /**
     * Sets the handler which will handle all connections managed by this service.
     */
    void SetHandler( IoHandler* apoHandler );
    
    /**
     * Returns the handler which will handle all connections managed by this service.
     */
    IoHandler* GetHandler(void);
    
    /**
     * Sets session's service.
     */
    void SetService( CIoService* apoService );
    
    /**
     * Returns session's service.
     */
    CIoService* GetService(void);
    
    /**
     * Sets session read buffer size.
     */
    void SetReadBufferSize( apl_size_t auBufferSize );
    
    /**
     * Returns session read buffer size.
     */
    apl_size_t GetReadBufferSize(void);
    
    /**
     * Returns session read buffer.
     */
    acl::CMemoryBlock* GetReadBuffer(void);
    
    /**
     * Returns the filter chain of the session.
     */
    CIoFilterChain* GetFilterChain(void);
    
    /**
     * Returns the session write request queue.
     */
    WriteRequestQueueType& GetWriteRequestQueue(void);

    /**
     * Returns the session write request queue condition.
     */
    acl::CCondition& GetWriteRequestCond(void);

    /**
     * Returns the session write request queue lock.
     */
    acl::CLock& GetWriteRequestLock(void);
    
    /**
     * Returns the session attribute object.
     */
    CIoSessionAttribute& GetAttribute(void);
    
    /**
     * Increase the total number of messages which were read and decoded from this session.
     */
    void IncreaseReadMessages( apl_size_t auIncrement );
    
    /**
     * Increase the total number of bytes which were read from this session.
     */
    void IncreaseReadBytes( apl_uint64_t auIncrement, acl::CTimestamp const& aoCurrTime );
    
    /**
     * Increase the total number of messages which were written and encoded by this session.
     */
    void IncreaseWrittenMessages( apl_size_t auIncrement );
    
    /**
     * Increase the total number of messages which were written and encoded by this session.
     */
    void IncreaseWrittenBytes( apl_uint64_t auIncrement, acl::CTimestamp const& aoCurrTime );
    
    /**
     * Returns the total number of notified idle which were fired this session.
     */
    apl_size_t GetIdleCount(void);
    
    /**
     * Returns the total number of messages which were read and decoded from this session.
     */
    apl_size_t GetReadMessages(void);
    
    /**
     * Returns the total number of bytes which were read from this session.
     */
    apl_uint64_t GetReadBytes(void);
    
    /**
     * Returns the total number of messages which were written and encoded by this session.
     */
    apl_size_t GetWrittenMessages(void);
    
    /**
     * Returns the total number of bytes which were written to this session.
     */
    apl_uint64_t GetWrittenBytes(void);
    
    /**
     * Returns the session activate time.
     */
    acl::CTimestamp& GetActivateTime(void);
    
    /**
     * Returns the session last read time.
     */
    acl::CTimestamp& GetLastReadTime(void);
    
    /**
     * Returns the session last read idle time.
     */
    acl::CTimestamp& GetLastReadIdleTime(void);
    
    /**
     * Returns the session last write time.
     */
    acl::CTimestamp& GetLastWriteTime(void);
    
    /**
     * Returns the session last write time.
     */
    acl::CTimestamp& GetLastWriteIdleTime(void);
    
    /** 
     * @brief  Write message on this session, this message will be schedule in the future in processor
     * 
     * @param[in] aoSession specified session
     * @param[in] aoMessage writing message
     * @param[in] apvAct async call back act
     * 
     * @returns write future object, user can wait for completion on it
     */
    template<typename T>
    CWriteFuture::PointerType Write( SessionPtrType& aoSession, T const& aoMessage, void* apvAct = NULL )
    {
        CWriteFuture::PointerType loFuture(new CWriteFuture(aoMessage, NULL, apvAct) );

        if (!this->IsClosing() )
        {
            aoSession->GetFilterChain()->FireFilterWrite(aoSession, loFuture);
        }
        else
        {
            loFuture->Exception(E_SOCK_SEND, APL_EPIPE);
        }

        return loFuture;
    }
    
    /** 
     * @brief  Write message on this session, this message will be schedule in the future in processor
     * 
     * @param[in] aoSession specified session
     * @param[in] aoMessage writing message
     * @param[in] apvAct async call back act
     * @param[in] apoRemoteAddress remote address
     * 
     * @returns write future object, user can wait for completion on it
     */
    template<typename T>
    CWriteFuture::PointerType Write( SessionPtrType& aoSession, T const& aoMessage, acl::CSockAddr* apoRemoteAddress, void* apvAct = NULL )
    {
        CWriteFuture::PointerType loFuture(new CWriteFuture(aoMessage, apoRemoteAddress, NULL) );

        if (!this->IsClosing() )
        {
            aoSession->GetFilterChain()->FireFilterWrite(aoSession, loFuture);
        }
        else
        {
            loFuture->Exception(E_SOCK_SEND, APL_EPIPE);
        }

        return loFuture;
    }
    
    /**
     * Suspends read operations for this session.
     */
    void SuspendRead( SessionPtrType& aoSession );

    /**
     * Suspends write operations for this session.
     */
    void SuspendWrite( SessionPtrType& aoSession );

    /**
     * Resumes read operations for this session.
     */
    void ResumeRead( SessionPtrType& aoSession );

    /**
     * Resumes write operations for this session.
     */
    void ResumeWrite( SessionPtrType& aoSession );
    
    /**
     * Is read operation is suspended for this session. 
     * @return true if suspended
     */
    bool IsReadSuspended(void);
    
    /**
     * Is write operation is suspended for this session.
     * @return true if suspended
     */
    bool IsWriteSuspended(void);
    
    /** 
     * @brief Notify ilde session, SESSION_READ_IDLE & SESSION_WRITE_IDLE evnet will be trigger
     * 
     * @param[in] aoSession notified session
     * @param[in] aoCurrTime current time
     */
    void NotifyIdleSession( SessionPtrType& aoSession, acl::CTimestamp const& aoCurrTime );
    
    /** 
     * @brief Return session current mask event
     * 
     * @returns session event
     */
    apl_int_t GetEvent(void);

    /** 
     * @brief Is the session in closing queue
     * 
     * @returns true if closing
     */
    bool IsClosing(void);

    /** 
     * @brief Is disables further receive operations
     * 
     * @returns true if disable
     */
    bool IsReadClosed(void);

    /** 
     * @brief Is disables further send operations
     * 
     * @returns true if disable
     */
    bool IsWriteClosed(void);

    /** 
     * @brief Is the session closed
     * 
     * @returns true if closed
     */
    bool IsClosed(void);

private:
    apl_int_t GetLocalAddr( acl::CSockAddr& aoAddr ) const;
    apl_int_t GetRemoteAddr( acl::CSockAddr& aoAddr ) const;

protected:
    /** 
     * @brief Reset all stat. variable
     */
    void Reset(void);
    
    apl_int_t SetReadSuspended( bool abIsEnable );
    
    apl_int_t SetWriteSuspended( bool abIsEnable );
    
    void SetEvent(apl_int_t aiEvents );
    
    apl_ssize_t Recv( void* apvBuffer, apl_size_t auLen );

    apl_ssize_t Recv( void* apvBuffer, apl_size_t auLen, acl::CSockAddr* apoRemoteAddress );

    apl_ssize_t Send( void const* apvBuffer, apl_size_t auLen );

    apl_ssize_t Send( void const* apvBuffer, apl_size_t auLen, acl::CSockAddr const& aoRemoteAddress );

    void UpdateReadClosed( bool abIsEnable );

    void UpdateWriteClosed( bool abIsEnable );

private:
    IoProcessor* mpoProcessor;
    
    IoHandler* mpoHandler;
    
    CIoService* mpoService;
    
    apl_uint64_t mu64SessionID;

    apl_int32_t miEvents;
    
    bool mbIsReadSuspended;

    bool mbIsWriteSuspended;
    
    apl_size_t muIdleCount;
    
    apl_size_t muReadMessages;
    
    apl_uint64_t mu64ReadBytes;
    
    apl_size_t muWrittenMessages;
    
    apl_uint64_t mu64WrittenBytes;
    
    acl::CTimestamp moActivateTime;
    
    acl::CTimestamp moLastReadTime;
        
    acl::CTimestamp moLastReadIdleTime;
        
    acl::CTimestamp moLastWriteTime;
    
    acl::CTimestamp moLastWriteIdleTime;
   
    bool mbIsClosing;

    bool mbIsReadClosed;

    bool mbIsWriteClosed;

    acl::CMemoryBlock moReadBuffer;
    
    CIoFilterChain moFilterChain;
    
    WriteRequestQueueType moWriteRequests;

    acl::CCondition moWriteRequestsCond;

    acl::CLock moWriteRequestsLock;

    CIoSessionAttribute moAttribute;

    acl::CSockAddr moLocalAddress;

    acl::CSockAddr moRemoteAddress;

    acl::CSockStream moStream;

    acl::CSockDgram moDgram;
};

ANF_NAMESPACE_END

#endif//ANF_IOSESSION_H
