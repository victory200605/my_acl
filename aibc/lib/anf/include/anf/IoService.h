
#ifndef ANF_IOSERVICE_H
#define ANF_IOSERVICE_H

#include "acl/Timestamp.h"
#include "acl/Number.h"
#include "acl/Synch.h"
#include "acl/stl/list.h"
#include "anf/Utility.h"
#include "anf/IoFilterChain.h"
#include "anf/IoHandler.h"
#include "anf/ConcurrentMap.h"
#include "anf/IoConfiguration.h"

ANF_NAMESPACE_START

/**
 * Base interface for all {@link IoAcceptor}s and {@link IoConnector}s
 * that provide I/O service and manage {@link CIoSession}s.
 */
class CIoService
{
public:
    template<typename T> struct TPointer
    {
        TPointer(void)
            : mpoPtr(APL_NULL)
            , mbIsShared(false)
        {
        }

        ~TPointer(void)
        {
            this->Set(APL_NULL, true);
        }

        void Set( T* apoPtr, bool abIsShared )
        {
            if (!this->mbIsShared && this->mpoPtr != APL_NULL)
            {
                ACL_DELETE(this->mpoPtr);
            }

            this->mpoPtr = apoPtr;
            this->mbIsShared = abIsShared;
        }

        T* GetPtr(void)
        {
            return this->mpoPtr;
        }

        bool IsShared(void)
        {
            return this->mbIsShared;
        }

        bool IsEmpty(void)
        {
            return this->mpoPtr == APL_NULL;
        }

        T* mpoPtr;
        bool mbIsShared;
    };

    typedef TConcurrentMap<apl_uint64_t, SessionPtrType> SessionMapType;
        
public:
    /**
     * Default constructor
     */
    CIoService(void);
    
    /**
     * Default destructor
     */
    virtual ~CIoService(void);

    /**
     * Releases any resources allocated by this service.  Please note that
     * this method might block as long as there are any sessions managed by
     * this service.
     */
    virtual apl_int_t Dispose(void) = 0;

    /** 
     * @brief Returns a value of whether or not this service is disposed
     * 
     * @returns whether of not the service is disposed
     */
    virtual bool IsDisposed(void);
    
    /**
     * Returns a value of whether or not this service is active
     *
     * @return whether of not the service is active.
     */
    virtual bool IsActivated(void);
    
    /**
     * Returns the time when this service was activated.  It returns the last
     * time when this service was activated if the service is not active now.
     */
    virtual acl::CTimestamp GetActivationTime(void);

    /**
     * Returns the handler which will handle all connections managed by this service.
     */
    IoHandler* GetHandler(void);

    /**
     * Sets the handler which will handle all connections managed by this service.
     */
    void SetHandler( IoHandler* apoHandler );
    
    /**
     * Returns the processor which will schedule all connections managed by this service.
     */
    IoProcessor* GetProcessor(void);
    
    /**
     * Sets the processor which will schedule all connections managed by this service.
     * Processor will not be dispose and delete by servive when disposing
     */
    void SetProcessor( IoProcessor* apoProcessor );

    bool IsSharedProcessor(void);
    
    /**
     * Sets the Configuration for this service.
     * Configuration will not be delete by servive when disposing
     */
    void SetConfiguration( CIoConfiguration* apoConfiguration );

    /**
     * Returns the map of the sessions which are currently managed by this
     * service. 
     */
    bool FindManagedSession( apl_uint64_t au64SessionID, SessionPtrType& aoSession );
     
    /**
     * Returns the map of the sessions which are currently managed by this
     * service. @aoPred is Predication function.
     */
    template<class PredicationType>
    bool FindManagedSessionIf( PredicationType& aoPred, SessionPtrType& aoSession )
    {
        return this->moSessions.FindIf(aoPred, aoSession);
    }

    /**
     * Returns the all sessions which are currently managed by this
     * service.
     */
    void GetManagedSessions( std::map<apl_uint64_t, SessionPtrType>& aoSessions );

    /**
     * Returns the number of all sessions which are currently managed by this
     * service.
     */
    apl_size_t GetManagedSessionCount(void);

    /**
     * Returns the default configuration of the new {@link CIoSession}s
     * created by this service.
     */
    CIoConfiguration* GetConfiguration(void);
    
    /**
     * Returns the filterchain which will filter all connections managed by this service.
     */
    CIoFilterChain* GetFilterChain(void);
    
    /**
     * Writes the specified {@aoMessage} to all the {@link CIoSession}s
     * managed by this service.
     */
    template<typename WriterType>
    void Broadcast( WriterType& aoWriter )
    {
        this->moSessions.All(aoWriter);
    }
    
//listen method
    void InitSession( SessionPtrType& aoSession );
    
    void DestroySession( SessionPtrType& aoSession );

protected:
    void SetDisposed( bool abFlag );

    void SetActivated( bool abFlag );

    void UpdateActivationTime(void);

    void DefaultProcessor(void);
    
private:
    TPointer<IoProcessor> moProcessorPtr;
    
    TPointer<CIoConfiguration> moConfigurationPtr;
    
    TPointer<IoHandler> moHandlerPtr;
    
    TPointer<CIoFilterChain> moFilterChainPtr;

    SessionMapType moSessions;
    
    static acl::TNumber<apl_uint64_t, acl::CLock> moSessionID;
    
    bool mbIsDisposed;

    bool mbIsActivated;

    acl::CTimestamp moActivationTime;
};

ANF_NAMESPACE_END

#endif//ANF_IOSERVICE_H
