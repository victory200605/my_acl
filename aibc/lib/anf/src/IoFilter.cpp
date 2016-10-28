
#include "anf/IoFilter.h"

ANF_NAMESPACE_START

void CIoFilter::Init(void)
{
}

void CIoFilter::Destroy(void)
{
}

void CIoFilter::SessionOpened(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession )
{
    apoNextFilter->SessionOpened(aoSession);
}

void CIoFilter::SessionClosed(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession )
{
    apoNextFilter->SessionClosed(aoSession);
}

void CIoFilter::SessionIdle(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    apl_int_t aiStatus )
{
    apoNextFilter->SessionIdle(aoSession, aiStatus);
}

//void CIoFilter::Exception(
//    IoNextFilter* apoNextFilter,
//    CIoService* apoService,
//    CThrowable const& aoCause )
//{
//    apoNextFilter->Exception(apoService,aoCause);
//}
    
void CIoFilter::SessionException(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    CThrowable const& aoCause )
{
    apoNextFilter->SessionException(aoSession, aoCause);
}

void CIoFilter::MessageReceived(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    acl::CAny const& aoMessage )
{
    apoNextFilter->MessageReceived(aoSession, aoMessage);
}

void CIoFilter::MessageFinished(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    CWriteFuture::PointerType& aoWriteFuture )
{
    apoNextFilter->MessageFinished(aoSession, aoWriteFuture);
}

void CIoFilter::FilterClose(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession )
{
    apoNextFilter->FilterClose(aoSession);
}

void CIoFilter::FilterWrite(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    CWriteFuture::PointerType& aoWriteFuture )
{
    apoNextFilter->FilterWrite(aoSession, aoWriteFuture);
}

ANF_NAMESPACE_END

