
#ifndef ACL_SOCKADDR_H
#define ACL_SOCKADDR_H

#include "Utility.h"

ACL_NAMESPACE_START

class CSockAddr
{
public:
    CSockAddr(void);
    
    CSockAddr( CSockAddr const& aoOther );
    
    CSockAddr( apl_sockaddr_t const* apoAddr, apl_size_t aiLen );
    
    CSockAddr( char const* apcIpAddr, apl_uint16_t aiPort, apl_int_t aiFamily = APL_AF_INET );
    
    bool IsAny(void) const;
    
    apl_int_t SetAny(void);
    
    apl_int_t SetLoopBack(void);
    
    apl_int_t Set( CSockAddr const& aoOther );
    
    apl_int_t Set( apl_sockaddr_t const* apoAddr, apl_size_t aiLen );
    
    apl_int_t Set( char const* apcIpAddr, apl_uint16_t aiPort, apl_int_t aiFamily = APL_AF_INET );
    
    apl_int_t Set( 
        char const* apcHostName,
        char const* apcSvcName = NULL,
        apl_int_t aiFamily = APL_AF_INET,
        apl_int_t aiType = 0,
        apl_int_t aiProto = 0 );
    
    apl_int_t SetIpAddr( char const* apcIpAddr );
    
    apl_int_t SetPort( apl_uint16_t aiPort );
    
    apl_int_t SetFamily( apl_int_t aiFamily );
    
    apl_sockaddr_t* GetAddr(void);
    
    apl_sockaddr_t const* GetAddr(void) const;
    
    apl_size_t GetLength(void) const;
    
    apl_int_t GetFamily(void) const;
    
    apl_int_t GetPort(void) const;
    
    apl_int_t GetIpAddr( char* apcIpAddrBuff, apl_size_t aiSize ) const;

protected:
    void Reset(void);

protected:
    apl_sockaddr_storage_t coInetAddr;
};

ACL_NAMESPACE_END

#endif//ACL_SOCKADDR_H
