
#ifndef ANF_WRITE_FUTURE_H
#define ANF_WRITE_FUTURE_H

#include "acl/SmartPtr.h"
#include "acl/Any.h"
#include "acl/SockAddr.h"
#include "acl/Timestamp.h"
#include "anf/IoFuture.h"

ANF_NAMESPACE_START

class CWriteFuture : public CIoFuture
{
public:
    typedef acl::TSharePtr<CWriteFuture, acl::CLock> PointerType;

public:
    template<typename T>
    CWriteFuture( T& aoMessage, acl::CSockAddr* apoRemoteAddress, void* apvAct )
        : moMessage(aoMessage)
        , mbIsHasRemoteAddress(false)
        , mpvAct(apvAct)
        , muFlushCount(0)
    {
        if (apoRemoteAddress != NULL)
        {
            this->moRemoteAddress.Set(*apoRemoteAddress);
            this->mbIsHasRemoteAddress = true;
        }

        this->moTimestamp.Update();
    }

    ~CWriteFuture(void);
    
    template<typename T>
    void SetMessage( T& aoMessage )
    {
        this->moMessage = aoMessage;
    }
    
    acl::CAny const& GetMessage(void) const;
    
    acl::CSockAddr const& GetRemoteAddress(void) const;
    
    bool IsHasRemoteAddress(void) const;

    void* GetAct(void) const;

    acl::CTimestamp const& GetTimestamp(void) const;
    
    apl_size_t GetFlushCount(void) const;

    void IncreaseFlushCount( apl_size_t auCount );

private:
    acl::CAny moMessage;
    
    acl::CSockAddr moRemoteAddress;
    
    bool mbIsHasRemoteAddress;

    void* mpvAct;

    acl::CTimestamp moTimestamp;

    apl_size_t muFlushCount;
};

ANF_NAMESPACE_END

#endif//ANF_WRITE_FUTURE_H
