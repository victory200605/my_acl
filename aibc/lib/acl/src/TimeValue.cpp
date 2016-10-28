
#include "acl/TimeValue.h"

ACL_NAMESPACE_START

CTimeValue const CTimeValue::ZERO(0, APL_TIME_NSEC);
CTimeValue const CTimeValue::MAXTIME(APL_INT64_MAX, APL_TIME_NSEC);

CTimeValue::CTimeValue()
    : mi64Value(0)
{
}

CTimeValue::CTimeValue( apl_time_t ai64TimeValue, apl_time_t ai64Unit )
    : mi64Value(ai64TimeValue * ai64Unit)
{
}

CTimeValue::CTimeValue( double adTimeValue )
    : mi64Value(apl_time_t(adTimeValue * APL_TIME_SEC) )
{
}

void CTimeValue::Set(apl_time_t ai64TimeValue, apl_time_t ai64Unit)
{
    this->mi64Value = ai64TimeValue * ai64Unit;
}

void CTimeValue::Nsec( apl_time_t ai64Nsec )
{
    this->Set( ai64Nsec, APL_TIME_NSEC );
}

void CTimeValue::Usec( apl_time_t ai64Usec )
{
    this->Set( ai64Usec, APL_TIME_USEC );
}

void CTimeValue::Msec( apl_time_t ai64Msec )
{
    this->Set( ai64Msec, APL_TIME_MSEC );
}

void CTimeValue::Sec( apl_time_t ai64Sec )
{
    this->Set( ai64Sec, APL_TIME_SEC );
}

void CTimeValue::Min( apl_time_t ai64Min )
{
    this->Set( ai64Min, APL_TIME_MIN );
}

void CTimeValue::Hour( apl_time_t ai64Hour )
{
    this->Set( ai64Hour, APL_TIME_HOUR );
}

void CTimeValue::Day( apl_time_t ai64Day )
{
    this->Set( ai64Day, APL_TIME_DAY );
}

void CTimeValue::Week( apl_time_t ai64Week )
{
    this->Set( ai64Week, APL_TIME_WEEK );
}

apl_time_t CTimeValue::Nsec(void) const
{
    return this->mi64Value;
}

apl_time_t CTimeValue::Usec(void) const
{
    return this->mi64Value / APL_TIME_USEC;
}

apl_time_t CTimeValue::Msec(void) const
{
    return this->mi64Value / APL_TIME_MSEC;
}

apl_time_t CTimeValue::Sec(void) const
{
    return this->mi64Value / APL_TIME_SEC;
}

apl_time_t CTimeValue::Min(void) const
{
    return this->mi64Value / APL_TIME_MIN;
}

apl_time_t CTimeValue::Hour(void) const
{
    return this->mi64Value / APL_TIME_HOUR;
}

apl_time_t CTimeValue::Day(void) const
{
    return this->mi64Value / APL_TIME_DAY;
}

apl_time_t CTimeValue::Week(void) const
{
    return this->mi64Value / APL_TIME_WEEK;
}

CTimeValue& CTimeValue::operator +=( CTimeValue const& aoRhs )
{
    this->mi64Value += aoRhs.mi64Value;
    return *this;
}

CTimeValue& CTimeValue::operator +=(apl_time_t ai64Rhs )
{
    this->mi64Value += ai64Rhs * APL_TIME_SEC;
    return *this;
}

CTimeValue& CTimeValue::operator =( CTimeValue const& aoRhs )
{
    this->mi64Value = aoRhs.mi64Value;
    return *this;
}

CTimeValue& CTimeValue::operator =(apl_time_t ai64Rhs )
{
    this->mi64Value = ai64Rhs * APL_TIME_SEC;
    return *this;
}

CTimeValue& CTimeValue::operator -=( CTimeValue const& aoRhs )
{
    this->mi64Value -= aoRhs.mi64Value;
    return *this;
}

CTimeValue& CTimeValue::operator -=( apl_time_t ai64Rhs )
{
    this->mi64Value -= ai64Rhs * APL_TIME_SEC;
    return *this;
}

CTimeValue CTimeValue::operator++(int)
{
    CTimeValue loTmp(*this);
    
    (*this) += 1;
    
    return loTmp;
}

CTimeValue& CTimeValue::operator++(void)
{
    (*this) += 1;
    return *this;
}

CTimeValue CTimeValue::operator--(int)
{
    CTimeValue loTmp(*this);
    
    (*this) -= 1;
    
    return loTmp;
}

CTimeValue& CTimeValue::operator--(void)
{
    (*this) -= 1;
    return *this;
}

ACL_NAMESPACE_END
