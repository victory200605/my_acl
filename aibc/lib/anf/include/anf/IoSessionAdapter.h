
#ifndef ANF_IOSESSIONADAPTER_H
#define ANF_IOSESSIONADAPTER_H

#include "anf/Utility.h"
#include "acl/SockAddr.h"

ANF_NAMESPACE_START

class CIoSessionAdapter
{
protected:
    void SetEvent( SessionPtrType& aoSession, apl_int_t aiEvents );
    
    apl_int_t SetReadSuspended( SessionPtrType& aoSession, bool abIsEnable );
    
    apl_int_t SetWriteSuspended( SessionPtrType& aoSession, bool abIsEnable );
    
    apl_ssize_t Recv( SessionPtrType& aoSession, void* apvBuffer, apl_size_t auLen );

    apl_ssize_t Recv(
        SessionPtrType& aoSession,
        void* apvBuffer,
        apl_size_t auLen,
        acl::CSockAddr* apoRemoteAddress );

    apl_ssize_t Send( SessionPtrType& aoSession, void const* apvBuffer, apl_size_t auLen );

    apl_ssize_t Send(
        SessionPtrType& aoSession,
        void const* apvBuffer,
        apl_size_t auLen,
        acl::CSockAddr const& aoRemoteAddress );
};

ANF_NAMESPACE_END

#endif//ANF_IOSESSIONADAPTER_H

