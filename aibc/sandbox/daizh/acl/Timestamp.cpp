
#include "Timestamp.h"

ACL_NAMESPACE_START

CTimestamp::CTimestamp()
    : ci64Value(0)
{
}

CTimestamp::CTimestamp( apl_time_t ai64Timestamp )
    : ci64Value(ai64Timestamp)
{
}

apl_int_t CTimestamp::Update( EPrecision aePrecision )
{
    switch( aePrecision )
    {
        case CTimestamp::PRC_SEC:
            this->ci64Value = apl_time();
            break;
        case CTimestamp::PRC_MSEC:
            this->ci64Value = apl_gettimeofday();
            break;
        case CTimestamp::PRC_USEC:
            this->ci64Value = apl_gettimeofday();
            break;
        case CTimestamp::PRC_NSEC:
            this->ci64Value = apl_clock_gettime();
            break;
        default:
            ACL_ASSERT_MSG( false, invalid_time_precision );
    };
    
    return 0;
}

void CTimestamp::Nsec( apl_time_t ai64Timestamp )
{
    this->ci64Value = ai64Timestamp * APL_TIME_NSEC;
}

apl_time_t CTimestamp::Nsec() const
{
    return this->ci64Value / APL_TIME_NSEC;
}

void CTimestamp::Usec( apl_time_t ai64Timestamp )
{
    this->ci64Value = ai64Timestamp * APL_TIME_USEC;
}

apl_time_t CTimestamp::Usec() const
{
    return this->ci64Value / APL_TIME_USEC;
}

void CTimestamp::Msec( apl_time_t ai64Timestamp )
{
    this->ci64Value = ai64Timestamp * APL_TIME_MSEC;
}

apl_time_t CTimestamp::Msec() const
{
    return this->ci64Value / APL_TIME_MSEC;
}

void CTimestamp::Sec( apl_time_t ai64Timestamp )
{
    this->ci64Value = ai64Timestamp * APL_TIME_SEC;
}

apl_time_t CTimestamp::Sec() const
{
    return this->ci64Value / APL_TIME_SEC;
}

CTimestamp& CTimestamp::operator += ( CTimeValue const& aoRhs )
{
    this->ci64Value += aoRhs.Nsec();
    
    return *this;
}
    
CTimestamp& CTimestamp::operator -= ( CTimeValue const& aoRhs )
{
    this->ci64Value -= aoRhs.Nsec();
    
    return *this;
}

ACL_NAMESPACE_END
