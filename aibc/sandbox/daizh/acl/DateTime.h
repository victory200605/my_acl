
#ifndef ACL_DATETIME_H
#define ACL_DATETIME_H

#include "Utility.h"
#include "TimeValue.h"
#include "Timestamp.h"

ACL_NAMESPACE_START

class CDateTime
{
public:
    enum ETimeRegion
    {
        /**
         * local time
         */
        LOCAL_TIME = 1,
        
        /**
         * UTC time
         */
        GM_TIME = 2
    };
    
    static char const* DEFAULT_FORMAT;

public:
    CDateTime();
    
    apl_int_t Update( ETimeRegion aeFlag = LOCAL_TIME );
    
    apl_int_t Update( CTimestamp const& aoTimestamp, ETimeRegion aeFlag = LOCAL_TIME );
    
    apl_int_t Update( char const* apcStr, char const* apcFormat = DEFAULT_FORMAT );
    
    apl_int_t Update( apl_int_t aiTimeRegion );
    
    const char* Format( char const* apcFormat = DEFAULT_FORMAT );
    
    CTimestamp GetTimestamp() const;
    
    apl_int_t GetTimestamp( CTimestamp& aoTimestamp ) const;
    
    apl_int_t Set(
        apl_time_t ai64Year,
        apl_time_t ai64Month,
        apl_time_t ai64Day,
        apl_time_t ai64Hour = 0,
        apl_time_t ai64Minute = 0,
        apl_time_t ai64Second = 0 );

    apl_time_t GetDay(void) const;
    
    apl_time_t GetMonth(void) const;
    
    apl_time_t GetYear(void) const;
    
    apl_time_t GetHour(void) const;
    
    apl_time_t GetMin(void) const;
    
    apl_time_t GetSec(void) const;
    
    apl_time_t GetWeekday(void) const;
    
    apl_time_t GetYearday(void) const;
    
    friend CTimeValue operator -( CDateTime const& aoLhs, CDateTime const& aoRhs );

protected:
    apl_tm_t coTm;
    
    char csBuffer[128];
};

//////////////////////////////////// inline implement /////////////////////////////////////////
inline CTimeValue operator -( CDateTime const& aoLhs, CDateTime const& aoRhs )
{
    return aoLhs.GetTimestamp() - aoRhs.GetTimestamp();
}

ACL_NAMESPACE_END

#endif//ACL_DATETIME_H
