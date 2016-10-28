
#include "acl/SockAddr.h"

ACL_NAMESPACE_START

CSockAddr::CSockAddr(void)
{
    this->Reset();
}
    
CSockAddr::CSockAddr( CSockAddr const& aoOther )
{
    this->Reset();
    apl_int_t liRetCode = this->Set(aoOther);
    ACL_ASSERT(liRetCode == 0);
}
    
CSockAddr::CSockAddr( apl_sockaddr_t const* apoAddr, apl_size_t aiLen )
{
    this->Reset();
    apl_int_t liRetCode = this->Set(apoAddr, aiLen);
    ACL_ASSERT(liRetCode == 0);
}
    
CSockAddr::CSockAddr( apl_uint16_t aiPort, char const* apcIpAddr, apl_int_t aiFamily )
{
    this->Reset();
    apl_int_t liRetCode = this->Set(aiPort, apcIpAddr, aiFamily);
    ACL_ASSERT(liRetCode == 0);
}
    
CSockAddr& CSockAddr::operator = ( CSockAddr const& aoRhs )
{
    this->Reset();
    this->Set(aoRhs);

    return *this;
}
    
bool CSockAddr::IsAny(void) const
{
    return apl_sockaddr_checkwild(this->GetAddr(), this->GetLength() ) == 1 ? true : false;
}

bool CSockAddr::IsLoopBack(void) const
{
    return apl_sockaddr_checkloopback(this->GetAddr(), this->GetLength() ) == 1 ? true : false;
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
    if ( aiLen > this->GetCapacity() )
    {
        apl_set_errno(APL_EINVAL);
        return -1;
    }
    
    apl_memcpy(this->GetAddr(), apoAddr, aiLen);
    
    return 0;
}

apl_int_t CSockAddr::Set( apl_uint16_t aiPort, char const* apcIpAddr, apl_int_t aiFamily )
{
    if ( this->SetFamily(aiFamily) != 0 )
    {
        return -1;
    }
    
    if (apcIpAddr != NULL)
    {
        if ( this->SetIpAddr(apcIpAddr) != 0 )
        {
            return -1;
        }
    }
    else
    {
        if ( this->SetAny() != 0 )
        {
            return -1;
        }
    }
    
    if ( this->SetPort(aiPort) != 0 )
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
    return apl_sockaddr_setaddr(this->GetAddr(), this->GetCapacity(), apcIpAddr);
}
    
apl_int_t CSockAddr::SetPort( apl_uint16_t aiPort )
{
    return apl_sockaddr_setport(this->GetAddr(), this->GetCapacity(), aiPort);
}
    
apl_int_t CSockAddr::SetFamily( apl_int_t aiFamily )
{
    return apl_sockaddr_setfamily(this->GetAddr(), aiFamily);
}

apl_sockaddr_t* CSockAddr::GetAddr(void)
{
    return reinterpret_cast<apl_sockaddr_t*>(&this->moInetAddr);
}
  
apl_sockaddr_t const* CSockAddr::GetAddr(void) const
{
    return reinterpret_cast<const apl_sockaddr_t*>(&this->moInetAddr);
}

apl_size_t CSockAddr::GetLength(void) const
{
    apl_int_t liLen = apl_sockaddr_getlen( this->GetAddr() );
    
    return liLen < 0 ? 0 : liLen;
}

apl_size_t CSockAddr::GetCapacity(void) const
{
    return sizeof(this->moInetAddr);
}

apl_int_t CSockAddr::GetFamily(void) const
{
    apl_int_t liFamily = 0;
    
    if ( apl_sockaddr_getfamily( this->GetAddr(), &liFamily) != 0 )
    {
        return -1;
    }
    else
    {
        return liFamily;
    }
}

apl_uint16_t CSockAddr::GetPort(void) const
{
    apl_uint16_t liPort = 0;
    
    if ( apl_sockaddr_getport( this->GetAddr(), &liPort) != 0 )
    {
        return 0;
    }
    else
    {
        return liPort;
    }
}

apl_int_t CSockAddr::GetPort( apl_uint16_t* apu16Port ) const
{
    apl_uint16_t liPort = 0;
    
    if ( apl_sockaddr_getport( this->GetAddr(), &liPort) != 0 )
    {
        return -1;
    }
    
    if (apu16Port != NULL)
    {
        *apu16Port = liPort;
    }
    
    return 0;
}
    
apl_int_t CSockAddr::GetIpAddr( char* apcIpAddrBuff, apl_size_t aiSize ) const
{
    return apl_sockaddr_getaddr(const_cast<apl_sockaddr_t*>(this->GetAddr() ), apcIpAddrBuff, aiSize);
}

char const* CSockAddr::GetIpAddr(void)
{
    if (apl_sockaddr_getaddr(
        const_cast<apl_sockaddr_t*>(this->GetAddr() ),
        this->macIpBuffer,
        sizeof(macIpBuffer)) != 0)
    {
        this->macIpBuffer[0] = '\0';
    }
    
    return this->macIpBuffer;
}

bool CSockAddr::operator == ( CSockAddr const& aoRhs )
{
    if (this->GetLength() != aoRhs.GetLength() )
    {
        return false;
    }

    return apl_memcmp(this->GetAddr(), aoRhs.GetAddr(), this->GetLength() ) == 0;
}

bool CSockAddr::operator != ( CSockAddr const& aoRhs )
{
    return !(*this == aoRhs);
}

void CSockAddr::Reset(void)
{
    apl_memset(this->GetAddr(), 0, sizeof(this->moInetAddr) );
}

ACL_NAMESPACE_END
