
#ifndef ANF_SESSIONCONFIG_H
#define ANF_SESSIONCONFIG_H

#include "anf/Utility.h"
#include "acl/Timestamp.h"
#include "acl/TimeValue.h"

ANF_NAMESPACE_START

class CIoConfiguration
{
public:
    CIoConfiguration(void);
    
    //service configuration
    void SetMaxListenerCount( apl_size_t auCount );
    
    apl_size_t GetMaxListenerCount(void) const;
    
    void SetMaxProgressConnectionCount( apl_size_t auCount );
    
    apl_size_t GetMaxProgressConnectionCount(void) const;
    
    //session configuration
	apl_size_t GetReadBufferSize(void) const;
	
	void SetReadBufferSize( apl_size_t auBufferSize );
	
	apl_size_t GetWriteBufferSize(void) const;
	
	void SetWriteBufferSize( apl_size_t auBufferSize );
	
	acl::CTimeValue const& GetReadIdleTime(void) const;
	
	void SetReadIdleTime( acl::CTimeValue const& aoTime );
	
	acl::CTimeValue const& GetWriteIdleTime(void) const;
	
	void SetWriteIdleTime( acl::CTimeValue const& aoTime );
	
	acl::CTimeValue const& GetWriteTimeout(void) const;
	
	void SetWriteTimeout( acl::CTimeValue const& aoTime );

	apl_size_t GetMaxQueue(void) const;

	void SetMaxQueue( apl_size_t auMaxQueue );

	acl::CTimeValue const& GetQueueTimeout(void) const;

	void SetQueueTimeout( acl::CTimeValue const& aoTime );

	//processor configuration
	void SetAutoSuspendWrite( bool abIsAutoSuspend );
	
	bool IsAutoSuspendWrite(void) const;
	
	void SetAutoResumeRead( bool abIsAutoResume );
	
	bool IsAutoResumeRead(void) const;
	
    void SetMaxConnectionCount( apl_size_t auCount );
	
	apl_size_t GetMaxConnectionCount(void) const;
	
	void SetProcessorPoolSize( apl_size_t auPoolSize );
	
	apl_size_t GetProcessorPoolSize(void) const;

private:
    apl_size_t muMaxListenerCount;
    
    apl_size_t muMaxProgressConnectionCount;
    
    apl_size_t muReadBufferSize;
    
    apl_size_t muWriteBufferSize;
    
    acl::CTimeValue moReaderIdleTime;
    
    acl::CTimeValue moWriterIdleTime;
    
    acl::CTimeValue moWriteTimeout;
        
    bool mbIsAutoSuspendWrite;
    
    bool mbIsAutoResumeRead;
    
    apl_size_t muMaxConnectionCount;
    
    apl_size_t muProcessorPoolSize;

    apl_size_t muMaxQueue;

    acl::CTimeValue moQueueTimeout;
};

ANF_NAMESPACE_END

#endif//ANF_SESSIONCONFIG_H
