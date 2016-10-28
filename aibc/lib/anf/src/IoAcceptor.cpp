
#include "anf/IoAcceptor.h"

ANF_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////////////////////
CIoAcceptor::CIoAcceptor(void)
{
}
    
CIoAcceptor::~CIoAcceptor(void)
{
}

void CIoAcceptor::SetDefaultLocalAddress( acl::CSockAddr const& aoLocalAddress )
{
    this->moLocalAddress.Set(aoLocalAddress);
}
    
void CIoAcceptor::GetDefaultLocalAddress( acl::CSockAddr& aoLocalAddress )
{
    aoLocalAddress.Set(this->moLocalAddress);
}
        
apl_int_t CIoAcceptor::Bind(void)
{
    return this->Bind(this->moLocalAddress);
}

apl_int_t CIoAcceptor::Unbind(void)
{
    return this->Unbind(this->moLocalAddress);
}

apl_int_t CIoAcceptor::Bind( acl::CSockAddr const& aoAddress )
{
    return this->Bind0(aoAddress);
}

apl_int_t CIoAcceptor::Unbind( acl::CSockAddr const& aoAddress )
{
    return this->Unbind0(aoAddress);
}

ANF_NAMESPACE_END
