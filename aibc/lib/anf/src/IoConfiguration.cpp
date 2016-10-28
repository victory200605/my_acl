
#include "anf/IoConfiguration.h"

ANF_NAMESPACE_START

CIoConfiguration::CIoConfiguration(void)
    : muMaxListenerCount(1024)
    , muMaxProgressConnectionCount(1024)
    , muReadBufferSize(1024)
    , muWriteBufferSize(1024)
    , moReaderIdleTime(3*60)
    , moWriterIdleTime(60)
    , moWriteTimeout(acl::CTimeValue::MAXTIME)
    , mbIsAutoSuspendWrite(true)
    , mbIsAutoResumeRead(true)
    , muMaxConnectionCount(1024)
    , muProcessorPoolSize(1)
    , muMaxQueue(10000)
    , moQueueTimeout(3)
{
}

void CIoConfiguration::SetMaxListenerCount( apl_size_t auCount )
{
    this->muMaxListenerCount = auCount;
}
    
apl_size_t CIoConfiguration::GetMaxListenerCount(void) const
{
    return this->muMaxListenerCount;
}

void CIoConfiguration::SetMaxProgressConnectionCount( apl_size_t auCount )
{
    this->muMaxProgressConnectionCount = auCount;
}
    
apl_size_t CIoConfiguration::GetMaxProgressConnectionCount(void) const
{
    return this->muMaxProgressConnectionCount;
}

apl_size_t CIoConfiguration::GetReadBufferSize(void) const
{
    return this->muReadBufferSize;
}

void CIoConfiguration::SetReadBufferSize( apl_size_t auBufferSize )
{
    this->muReadBufferSize = auBufferSize;
}

apl_size_t CIoConfiguration::GetWriteBufferSize(void) const
{
    return this->muWriteBufferSize;
}

void CIoConfiguration::SetWriteBufferSize( apl_size_t auBufferSize )
{
    this->muWriteBufferSize = auBufferSize;
}

acl::CTimeValue const& CIoConfiguration::GetReadIdleTime(void) const
{
    return this->moReaderIdleTime;
}

void CIoConfiguration::SetReadIdleTime( acl::CTimeValue const& aoTime )
{
    this->moReaderIdleTime = aoTime;
}

acl::CTimeValue const& CIoConfiguration::GetWriteIdleTime(void) const
{
    return this->moWriterIdleTime;
}

void CIoConfiguration::SetWriteIdleTime( acl::CTimeValue const& aoTime )
{
    this->moWriterIdleTime = aoTime;
}

acl::CTimeValue const& CIoConfiguration::GetWriteTimeout(void) const
{
    return this->moWriteTimeout;
}

void CIoConfiguration::SetWriteTimeout( acl::CTimeValue const& aoTime )
{
    this->moWriteTimeout = aoTime;
}

apl_size_t CIoConfiguration::GetMaxQueue(void) const
{
    return this->muMaxQueue;
}

void CIoConfiguration::SetMaxQueue( apl_size_t auMaxQueue )
{
    this->muMaxQueue = auMaxQueue;
}

acl::CTimeValue const& CIoConfiguration::GetQueueTimeout(void) const
{
    return this->moQueueTimeout;
}

void CIoConfiguration::SetQueueTimeout( acl::CTimeValue const& aoTime )
{
    this->moQueueTimeout = aoTime;
}

void CIoConfiguration::SetAutoSuspendWrite( bool abIsAutoSuspend )
{
    this->mbIsAutoSuspendWrite = abIsAutoSuspend;
}
	
bool CIoConfiguration::IsAutoSuspendWrite(void) const
{
    return this->mbIsAutoSuspendWrite;
}

void CIoConfiguration::SetAutoResumeRead( bool abIsAutoResume )
{
    this->mbIsAutoResumeRead = abIsAutoResume;
}
	
bool CIoConfiguration::IsAutoResumeRead(void) const
{
    return this->mbIsAutoResumeRead;
}
	
void CIoConfiguration::SetMaxConnectionCount( apl_size_t auCount )
{
    this->muMaxConnectionCount = auCount;
}
	
apl_size_t CIoConfiguration::GetMaxConnectionCount(void) const
{
    return this->muMaxConnectionCount;
}

void CIoConfiguration::SetProcessorPoolSize( apl_size_t auPoolSize )
{
    this->muProcessorPoolSize = auPoolSize;
}
	
apl_size_t CIoConfiguration::GetProcessorPoolSize(void) const
{
    return this->muProcessorPoolSize;
}

ANF_NAMESPACE_END
