
#include "DateTime.h"

ACL_NAMESPACE_START

char const* CDateTime::DEFAULT_FORMAT = "%y%m%d%H%M%S";

CDateTime::CDateTime()
{
    apl_memset(&this->coTm, 0, sizeof(this->coTm) );
}

apl_int_t CDateTime::Update( ETimeRegion aeFlag )
{
    CTimestamp loTimestamp;
    
    loTimestamp.Update(CTimestamp::PRC_SEC);
       
    return this->Update(loTimestamp, aeFlag);
}

apl_int_t CDateTime::Update( apl_int_t aiTimeRegion )
{
    if (aiTimeRegion > 12 || aiTimeRegion < -12)
    {
        return -1;
    }
    
    CTimestamp loTimestamp;
    CTimeValue loOffset(aiTimeRegion * 3600, APL_TIME_SEC);
    
    loTimestamp.Update(CTimestamp::PRC_SEC);
    loTimestamp += loOffset;
    
    return this->Update( loTimestamp, CDateTime::GM_TIME );
}

apl_int_t CDateTime::Update( CTimestamp const& aoTimestamp, ETimeRegion aeFlag )
{
    apl_tm_t* lpoResult = NULL;
    
    if ( aeFlag == CDateTime::LOCAL_TIME )
    {
        lpoResult = apl_localtime(aoTimestamp.Nsec(), &this->coTm);
    }
    else
    {
        lpoResult = apl_gmtime(aoTimestamp.Nsec(), &this->coTm);
    }
    
    return lpoResult == NULL ? -1 : 0;
}

apl_int_t CDateTime::Update( char const* apcStr, char const* apcFormat )
{
    char* lpcResult = apl_strptime(apcStr, apcFormat, &this->coTm);
    if ( lpcResult == NULL )
    {
        return -1;
    }
    
    return apl_mktime(&this->coTm) != -1 * APL_TIME_SEC ? 0 : -1;
}

const char* CDateTime::Format( char const* apcFormat )
{
    apl_size_t liResult = apl_strftime( 
        this->csBuffer, sizeof(this->csBuffer), apcFormat, &this->coTm );
    
    return liResult > 0 ? this->csBuffer : "";
}

CTimestamp CDateTime::GetTimestamp() const
{
    apl_time_t liTimestamp = apl_mktime(&this->coTm);
    
    return CTimestamp(liTimestamp);
}

apl_int_t CDateTime::GetTimestamp( CTimestamp& aoTimestamp ) const
{
    apl_time_t liTimestamp = apl_mktime(&this->coTm);
    if ( liTimestamp == -1 * APL_TIME_SEC )
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
    apl_memset(&this->coTm, 0, sizeof(this->coTm) );
    
    this->coTm.tm_year = ai64Year - 1900;
    this->coTm.tm_mon  = ai64Month - 1;
    this->coTm.tm_mday = ai64Day;
    this->coTm.tm_hour = ai64Hour;
    this->coTm.tm_min  = ai64Minute;
    this->coTm.tm_sec  = ai64Second;
    
    return apl_mktime(&this->coTm) != -1 * APL_TIME_SEC ? 0 : -1;
}

apl_time_t CDateTime::GetDay(void) const
{
    return this->coTm.tm_mday;
}

apl_time_t CDateTime::GetMonth(void) const
{
    return this->coTm.tm_mon + 1;
}

apl_time_t CDateTime::GetYear(void) const
{
    return this->coTm.tm_year + 1900;
}

apl_time_t CDateTime::GetHour(void) const
{
    return this->coTm.tm_hour;
}

apl_time_t CDateTime::GetMin(void) const
{
    return this->coTm.tm_min;
}

apl_time_t CDateTime::GetSec(void) const
{
    return this->coTm.tm_sec;
}

apl_time_t CDateTime::GetWeekday(void) const
{
    return this->coTm.tm_wday;
}

apl_time_t CDateTime::GetYearday(void) const
{
    return this->coTm.tm_yday;
}

ACL_NAMESPACE_END
