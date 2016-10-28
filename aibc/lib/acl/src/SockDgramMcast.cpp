
#include "acl/SockDgramMcast.h"

ACL_NAMESPACE_START

CSockDgramMcast::CSockDgramMcast( EBindOption aeOpt )
    : meOpt(aeOpt)
{
}

CSockDgramMcast::~CSockDgramMcast(void)
{
}

apl_int_t CSockDgramMcast::Open( CSockAddr const& aoMcastAddr, char const* apcIfname, apl_uint_t  auIfindex )
{
    CSockAddr loBindAddr(aoMcastAddr);

    if (this->GetHandle() != ACL_INVALID_HANDLE)
    {
        return 0;
    }
    
    //If opt OPT_BINDADDR_NO and set any to bindaddr
    if ( this->meOpt == OPT_BINDADDR_NO )
    {
        if ( loBindAddr.SetAny() != 0 )
        {
            return -1;
        }
    }
    
    //Open sock dgram handle
    if ( CSockDgram::Open(loBindAddr) != 0 )
    {
        return -1;
    }
    
    this->moSendAddr = aoMcastAddr;
    
    //Default ifname/ifindex
    if (apcIfname != NULL || auIfindex != 0 )
    {
        if ( this->SetIf(apcIfname, auIfindex) != 0 )
        {
            return -1;
        }
    }
    
    return 0;
}

apl_int_t CSockDgramMcast::OpenI( CSockAddr const& aoMcastAddr, char const* apcIfname, apl_uint_t  auIfindex )
{
    acl::CSockAddr loSubscribeAddr(aoMcastAddr);

    // If port# is 0, insert bound port# if it is set.
    apl_uint64_t lu16DefPort = this->moSendAddr.GetPort();
    if (loSubscribeAddr.GetPort() == 0
        && lu16DefPort != 0)
    {
        loSubscribeAddr.SetPort(lu16DefPort);
    }

    // Check for port# different than bound port#.
    apl_uint16_t lu16SubPort = aoMcastAddr.GetPort();
    if (lu16SubPort != 0
        && lu16DefPort != 0
        && lu16SubPort != lu16DefPort)
    {
        apl_set_errno(APL_ENXIO);
        return -1;
    }

    // If OPT_BINDADDR_YES is enabled, check for address different than bound address.
    if (ACL_BIT_ENABLED(this->meOpt, OPT_BINDADDR_YES)
        && !this->moSendAddr.IsAny()
        && apl_strcmp(this->moSendAddr.GetIpAddr(), loSubscribeAddr.GetIpAddr() ) != 0 )
    {
        apl_set_errno(APL_ENXIO);
        return -1;
    }

    return this->Open(loSubscribeAddr, apcIfname, auIfindex);
}
                
apl_int_t CSockDgramMcast::Join( CSockAddr const& aoGrpAddr, char const* apcIfname, apl_uint_t  auIfindex )
{
    // Open the socket first
    if (this->OpenI(aoGrpAddr, apcIfname, auIfindex) != 0)
    {
        return -1;
    }

    return apl_mcast_join(this->GetHandle(), aoGrpAddr.GetAddr(), aoGrpAddr.GetLength(), apcIfname, auIfindex);
}

apl_int_t CSockDgramMcast::Leave( const CSockAddr& aoGrpAddr )
{
    return apl_mcast_leave(this->GetHandle(), aoGrpAddr.GetAddr(), aoGrpAddr.GetLength() );
}

apl_int_t CSockDgramMcast::BlockSource( 
    CSockAddr const& aoSrcAddr,
    CSockAddr const& aoGrpAddr,
    char const* apcIfname,
    apl_uint_t  auIfindex )
{
    return apl_mcast_block_source(
        this->GetHandle(),
        aoSrcAddr.GetAddr(),
        aoSrcAddr.GetLength(),
        aoGrpAddr.GetAddr(),
        aoGrpAddr.GetLength(),
        apcIfname,
        auIfindex );
}

apl_int_t CSockDgramMcast::UnblockSource( const CSockAddr& aoSrcAddr, const CSockAddr& aoGrpAddr )
{
    return apl_mcast_unblock_source(
        this->GetHandle(), aoSrcAddr.GetAddr(), aoSrcAddr.GetLength(), aoGrpAddr.GetAddr(), aoGrpAddr.GetLength() );
}

apl_int_t CSockDgramMcast::JoinSourceGroup( 
    CSockAddr const& aoSrcAddr,
    CSockAddr const& aoGrpAddr,
    char const* apcIfname,
    apl_uint_t  auIfindex )
{
    // Open the socket first
    if (this->OpenI(aoGrpAddr, apcIfname, auIfindex) != 0)
    {
        return -1;
    }

    return apl_mcast_join_source_group(
        this->GetHandle(),
        aoSrcAddr.GetAddr(),
        aoSrcAddr.GetLength(),
        aoGrpAddr.GetAddr(),
        aoGrpAddr.GetLength(),
        apcIfname,
        auIfindex );
}

apl_int_t CSockDgramMcast::LeaveSourceGroup( 
    CSockAddr const& aoSrcAddr,
    CSockAddr const& aoGrpAddr )
{
    return apl_mcast_leave_source_group(
        this->GetHandle(),
        aoSrcAddr.GetAddr(),
        aoSrcAddr.GetLength(),
        aoGrpAddr.GetAddr(),
        aoGrpAddr.GetLength() );
}

apl_int_t CSockDgramMcast::SetIf( char const* apcIfname, apl_uint_t  auIfindex )
{   
    return apl_mcast_set_if(this->GetHandle(), apcIfname, auIfindex);
}

apl_int_t CSockDgramMcast::GetIf( apl_uint_t* apuIfindex )
{
    return apl_mcast_get_if(this->GetHandle(), apuIfindex);
}

apl_int_t CSockDgramMcast::SetLoop( apl_uint_t auLoop )
{
    return apl_mcast_set_loop(this->GetHandle(), auLoop);
}

apl_int_t CSockDgramMcast::GetLoop( apl_uint_t* apuLoop )
{
    return apl_mcast_get_loop(this->GetHandle(), apuLoop);
}

apl_int_t CSockDgramMcast::SetTTL( apl_uint_t auTTL )
{
    return apl_mcast_set_ttl(this->GetHandle(), auTTL);
}

apl_int_t CSockDgramMcast::GetTTL( apl_uint_t* apuTTL )
{
    return apl_mcast_get_ttl(this->GetHandle(), apuTTL);
}

apl_ssize_t CSockDgramMcast::SendM(
    void const* apvBuffer,
    apl_size_t aiN,
    CSockDgram::EOption aeFlag,
    CTimeValue const& aoTimeout ) const
{
    return CSockDgram::Send(apvBuffer, aiN, this->moSendAddr, aeFlag, aoTimeout);
}
                    
apl_ssize_t CSockDgramMcast::SendM(
    void const* apvBuffer,
    apl_size_t aiN,
    CTimeValue const& aoTimeout ) const
{
    return CSockDgram::Send(apvBuffer, aiN, this->moSendAddr, OPT_UNSPEC, aoTimeout);
}

ACL_NAMESPACE_END
