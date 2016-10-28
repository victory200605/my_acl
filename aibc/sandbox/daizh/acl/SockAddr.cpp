
#include "SockAddr.h"

ACL_NAMESPACE_START

CSockAddr::CSockAddr(void)
{
    this->Reset();
}
    
CSockAddr::CSockAddr( CSockAddr const& aoOther )
{
    this->Reset();
    this->Set(aoOther);
}
    
CSockAddr::CSockAddr( apl_sockaddr_t const* apoAddr, apl_size_t aiLen )
{
    this->Reset();
    this->Set(apoAddr, aiLen);
}
    
CSockAddr::CSockAddr( char const* apcIpAddr, apl_uint16_t aiPort, apl_int_t aiFamily )
{
    this->Reset();
    this->Set(apcIpAddr, aiPort, aiFamily);
}
    
bool CSockAddr::IsAny(void) const
{
    return false;
}
    
apl_int_t CSockAddr::SetAny(void)
{
    return apl_sockaddr_setwild(this->GetAddr(), this->GetLength() );
}

apl_int_t CSockAddr::SetLoopBack(void)
{
    return apl_sockaddr_setloopback(this->GetAddr(), this->GetLength() );
}
    
apl_int_t CSockAddr::Set( CSockAddr const& aoOther )
{
    apl_memcpy(this->GetAddr(), aoOther.GetAddr(), aoOther.GetLength() );
    
    return 0;
}
    
apl_int_t CSockAddr::Set( apl_sockaddr_t const* apoAddr, apl_size_t aiLen )
{
    apl_size_t liLen = aiLen > this->GetLength() ? this->GetLength() : aiLen;
    
    apl_memcpy(this->GetAddr(), apoAddr, liLen );
    
    return 0;
}

apl_int_t CSockAddr::Set( char const* apcIpAddr, apl_uint16_t aiPort, apl_int_t aiFamily )
{
    if ( apl_sockaddr_setfamily(this->GetAddr(), aiFamily) != 0 )
    {
        return -1;
    }
    
    if ( apl_sockaddr_setaddr(this->GetAddr(), this->GetLength(), apcIpAddr) != 0 )
    {
        return -1;
    }
    
    if ( apl_sockaddr_setport(this->GetAddr(), this->GetLength(), aiPort) != 0 )
    {
        return -1;
    }
    
    return 0;
}
    
apl_int_t CSockAddr::Set( 
    char const* apcHostName, char const* apcSvcName, apl_int_t aiFamily, apl_int_t aiType, apl_int_t aiProto )
{
    apl_int_t       liRetCode   = 0;
    apl_sockaddr_t* lpoInetAddr = this->GetAddr();

    liRetCode = apl_sockaddr_resolve(aiFamily, aiType, aiProto, apcHostName, apcSvcName, &lpoInetAddr, 1);
    
    return liRetCode > 0 ? 0 : -1;
}

apl_int_t CSockAddr::SetIpAddr( char const* apcIpAddr )
{
    return apl_sockaddr_setaddr(this->GetAddr(), this->GetLength(), apcIpAddr);
}
    
apl_int_t CSockAddr::SetPort( apl_uint16_t aiPort )
{
    return apl_sockaddr_setport(this->GetAddr(), this->GetLength(), aiPort);
}
    
apl_int_t CSockAddr::SetFamily( apl_int_t aiFamily )
{
    return apl_sockaddr_setfamily(this->GetAddr(), aiFamily);
}

apl_sockaddr_t* CSockAddr::GetAddr(void)
{
    return reinterpret_cast<apl_sockaddr_t*>(&this->coInetAddr);
}
  
apl_sockaddr_t const* CSockAddr::GetAddr(void) const
{
    return reinterpret_cast<const apl_sockaddr_t*>(&this->coInetAddr);
}

apl_size_t CSockAddr::GetLength(void) const
{
    return sizeof(this->coInetAddr);
}

apl_int_t CSockAddr::GetFamily(void) const
{
    apl_int_t liFamily = 0;
    
    if ( apl_sockaddr_getfamily( const_cast<apl_sockaddr_t*>(this->GetAddr() ), &liFamily) != 0 )
    {
        return -1;
    }
    else
    {
        return liFamily;
    }
}
    
apl_int_t CSockAddr::GetPort(void) const
{
    apl_uint16_t liPort = 0;
    
    if ( apl_sockaddr_getport(const_cast<apl_sockaddr_t*>(this->GetAddr() ), &liPort) != 0 )
    {
        return -1;
    }
    else
    {
        return liPort;
    }
}
    
apl_int_t CSockAddr::GetIpAddr( char* apcIpAddrBuff, apl_size_t aiSize ) const
{    
    return apl_sockaddr_getaddr(const_cast<apl_sockaddr_t*>(this->GetAddr() ), apcIpAddrBuff, aiSize);
}

void CSockAddr::Reset(void)
{
    apl_memset(this->GetAddr(), 0, this->GetLength() );
}

ACL_NAMESPACE_END
