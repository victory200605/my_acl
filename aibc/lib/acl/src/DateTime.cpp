
#include "acl/DateTime.h"

ACL_NAMESPACE_START

char const* CDateTime::DEFAULT_FORMAT = "%y%m%d%H%M%S";

CDateTime::CDateTime(void)
{
    apl_memset(&this->moTm, 0, sizeof(this->moTm) );
}
    
CDateTime::CDateTime( ETimeRegion aeFlag )
{
    apl_int_t liRetCode = this->Update(aeFlag);

    ACL_ASSERT(liRetCode == 0);
}
    
CDateTime::CDateTime( apl_int_t aiTimeRegion )
{
    apl_int_t liRetCode = this->Update(aiTimeRegion);

    ACL_ASSERT(liRetCode == 0);
}
    
CDateTime::CDateTime( CTimestamp const& aoTimestamp )
{
    apl_int_t liRetCode = this->Update(aoTimestamp);

    ACL_ASSERT(liRetCode == 0);
}

CDateTime::CDateTime( CTimestamp const& aoTimestamp, ETimeRegion aeFlag )
{
    apl_int_t liRetCode = this->Update(aoTimestamp, aeFlag);

    ACL_ASSERT(liRetCode == 0);
}

CDateTime::CDateTime( CTimestamp const& aoTimestamp, apl_int_t aiTimeRegion )
{
    apl_int_t liRetCode = this->Update(aoTimestamp, aiTimeRegion);

    ACL_ASSERT(liRetCode == 0);
}

CDateTime::CDateTime(
    apl_time_t ai64Year,
    apl_time_t ai64Month,
    apl_time_t ai64Day,
    apl_time_t ai64Hour,
    apl_time_t ai64Minute,
    apl_time_t ai64Second )
{
    apl_int_t liRetCode = this->Set(
        ai64Year, 
        ai64Month,
        ai64Day, 
        ai64Hour, 
        ai64Minute, 
        ai64Second);

    ACL_ASSERT(liRetCode == 0);
}

CDateTime::CDateTime( char const* apcStr, char const* apcFormat )
{
    apl_int_t liRetCode = this->Update(apcStr, apcFormat);

    ACL_ASSERT(liRetCode == 0);
}

CDateTime::CDateTime( CDateTime const& aoOther )
{
    *this = aoOther;
}

CDateTime& CDateTime::operator = ( CDateTime const& aoRhs )
{
    apl_memcpy(&this->moTm, &aoRhs.moTm, sizeof(this->moTm) );
    
    return *this;
}

apl_int_t CDateTime::Update(void)
{
    CTimestamp loTimestamp;
    
    loTimestamp.Update(CTimestamp::PRC_SEC);
       
    return this->Update(loTimestamp, LOCAL_TIME);
}

apl_int_t CDateTime::Update( ETimeRegion aeFlag )
{
    CTimestamp loTimestamp;
    
    loTimestamp.Update(CTimestamp::PRC_SEC);
       
    return this->Update(loTimestamp, aeFlag);
}

apl_int_t CDateTime::Update( apl_int_t aiTimeRegion )
{
    CTimestamp loTimestamp;
    
    loTimestamp.Update(CTimestamp::PRC_SEC);
    
    return this->Update(loTimestamp, aiTimeRegion);
}

apl_int_t CDateTime::Update( CTimestamp const& aoTimestamp )
{
    return this->Update(aoTimestamp, LOCAL_TIME);
}

apl_int_t CDateTime::Update( CTimestamp const& aoTimestamp, ETimeRegion aeFlag )
{
    apl_tm_t* lpoResult = NULL;
    
    if ( aeFlag == CDateTime::LOCAL_TIME )
    {
        lpoResult = apl_localtime(aoTimestamp.Nsec(), &this->moTm);
    }
    else
    {
        lpoResult = apl_gmtime(aoTimestamp.Nsec(), &this->moTm);
    }
    
    return lpoResult == NULL ? -1 : 0;
}

apl_int_t CDateTime::Update( CTimestamp const& aoTimestamp, apl_int_t aiTimeRegion )
{
    if (aiTimeRegion > 48 || aiTimeRegion < -48)
    {
        return -1;
    }
    
    CTimestamp loTimestamp(aoTimestamp);
    CTimeValue loOffset(aiTimeRegion * 900, APL_TIME_SEC);
    
    loTimestamp += loOffset;
    
    return this->Update(loTimestamp, CDateTime::GM_TIME);
}

apl_int_t CDateTime::Update( char const* apcStr, char const* apcFormat )
{
    char* lpcResult = apl_strptime(apcStr, apcFormat, &this->moTm);
    if ( lpcResult == NULL )
    {
        return -1;
    }
    
    return apl_mktime(&this->moTm) != -1 ? 0 : -1;
}

const char* CDateTime::Format( char const* apcFormat )
{
    apl_size_t liResult = apl_strftime( 
        this->msBuffer, sizeof(this->msBuffer), apcFormat, &this->moTm );
    
    return liResult > 0 ? this->msBuffer : "";
}

CTimestamp CDateTime::GetTimestamp() const
{
    apl_time_t liTimestamp = apl_mktime(&this->moTm);
    
    return CTimestamp(liTimestamp);
}

apl_int_t CDateTime::GetTimestamp( CTimestamp& aoTimestamp ) const
{
    apl_time_t liTimestamp = apl_mktime(&this->moTm);
    if (liTimestamp == -1)
    {
        return -1;
    }
    else
    {
        aoTimestamp.Nsec(liTimestamp);
        return 0;
    }
}

apl_int_t CDateTime::Set(
    apl_time_t ai64Year,
    apl_time_t ai64Month,
    apl_time_t ai64Day,
    apl_time_t ai64Hour,
    apl_time_t ai64Minute,
    apl_time_t ai64Second )
{
    apl_memset(&this->moTm, 0, sizeof(this->moTm) );
    
    this->moTm.tm_year = ai64Year - 1900;
    this->moTm.tm_mon  = ai64Month - 1;
    this->moTm.tm_mday = ai64Day;
    this->moTm.tm_hour = ai64Hour;
    this->moTm.tm_min  = ai64Minute;
    this->moTm.tm_sec  = ai64Second;
    
    return apl_mktime(&this->moTm) != -1 ? 0 : -1;
}

apl_time_t CDateTime::GetDay(void) const
{
    return this->moTm.tm_mday;
}

apl_time_t CDateTime::GetMonth(void) const
{
    return this->moTm.tm_mon + 1;
}

apl_time_t CDateTime::GetYear(void) const
{
    return this->moTm.tm_year + 1900;
}

apl_time_t CDateTime::GetHour(void) const
{
    return this->moTm.tm_hour;
}

apl_time_t CDateTime::GetMin(void) const
{
    return this->moTm.tm_min;
}

apl_time_t CDateTime::GetSec(void) const
{
    return this->moTm.tm_sec;
}

apl_time_t CDateTime::GetWeekday(void) const
{
    return this->moTm.tm_wday;
}

apl_time_t CDateTime::GetYearday(void) const
{
    return this->moTm.tm_yday;
}

ACL_NAMESPACE_END
