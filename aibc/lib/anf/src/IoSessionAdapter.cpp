
#include "anf/IoSessionAdapter.h"
#include "anf/IoSession.h"

ANF_NAMESPACE_START

void CIoSessionAdapter::SetEvent( SessionPtrType& aoSession, apl_int_t aiEvents )
{
    aoSession->SetEvent(aiEvents);
}

apl_int_t CIoSessionAdapter::SetReadSuspended( SessionPtrType& aoSession, bool abIsEnable )
{
    return aoSession->SetReadSuspended(abIsEnable);
}

apl_int_t CIoSessionAdapter::SetWriteSuspended( SessionPtrType& aoSession, bool abIsEnable )
{
    return aoSession->SetWriteSuspended(abIsEnable);
}

apl_ssize_t CIoSessionAdapter::Recv( SessionPtrType& aoSession, void* apvBuffer, apl_size_t auLen )
{
    return aoSession->Recv(apvBuffer, auLen);
}

apl_ssize_t CIoSessionAdapter::Recv(
    SessionPtrType& aoSession,
    void* apvBuffer,
    apl_size_t auLen,
    acl::CSockAddr* apoRemoteAddress )
{
    return aoSession->Recv(apvBuffer, auLen, apoRemoteAddress);
}

apl_ssize_t CIoSessionAdapter::Send( SessionPtrType& aoSession, void const* apvBuffer, apl_size_t auLen )
{
    return aoSession->Send(apvBuffer, auLen);
}

apl_ssize_t CIoSessionAdapter::Send(
    SessionPtrType& aoSession,
    void const* apvBuffer,
    apl_size_t auLen,
    acl::CSockAddr const& aoRemoteAddress )
{
    return aoSession->Send(apvBuffer, auLen, aoRemoteAddress);
}

ANF_NAMESPACE_END

