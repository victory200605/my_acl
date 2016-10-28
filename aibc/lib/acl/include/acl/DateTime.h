/**
 * @file DateTime.h
 */

#ifndef ACL_DATETIME_H
#define ACL_DATETIME_H

#include "acl/Utility.h"
#include "acl/TimeValue.h"
#include "acl/Timestamp.h"

ACL_NAMESPACE_START

/**
 * class CDateTime
 */
class CDateTime
{
public:
    /**
     * Time region enumeration.
     */
    enum ETimeRegion
    {
        LOCAL_TIME = 1, ///< local time
        
        GM_TIME = 2     ///< UTC time
    };
    
    /**
     * Default datetime format,such as "%y%m%d%H%M%S"
     */
    static char const* DEFAULT_FORMAT;

public:
    /**
     * @brief CDateTime constructor.
     */
    CDateTime(void);
    
    /**
     * @brief Construct and Update current local time or UTC time.
     * !NOTE  if update fail that will thow a exception here by ACL_ASSERT
     *
     * @param [in] aeFlag -   time region
     */
    CDateTime( ETimeRegion aeFlag );
    
    /**
     * @brief Construct and Update current data time by specified time region.
     * !NOTE  if update fail that will thow a exception here by ACL_ASSERT
     *
     * @param [in] aiTimeRegion - time region
     */
    CDateTime( apl_int_t aiTimeRegion );
    
    /**
     * @brief Construct and Update local time by specified timestamp.
     * !NOTE  if update fail that will thow a exception here by ACL_ASSERT
     *
     * @param [in] aoTimestamp -   the specified timestamp
     * @param [in] aeFlag -   time region
     */
    CDateTime( CTimestamp const& aoTimestamp );
    
    /**
     * @brief Construct and Update local time or UTC time by specified timestamp.
     * !NOTE  if update fail that will thow a exception here by ACL_ASSERT
     *
     * @param [in] aoTimestamp -   the specified timestamp
     * @param [in] aeFlag -   time region
     */
    CDateTime( CTimestamp const& aoTimestamp, ETimeRegion aeFlag );
    
    /**
     * @brief Construct and Update date time by specified timestamp and time region.
     * !NOTE  if update fail that will thow a exception here by ACL_ASSERT
     *
     * @param [in] aoTimestamp -   the specified timestamp
     * @param [in] aeFlag -   time region
     */
    CDateTime( CTimestamp const& aoTimestamp, apl_int_t aiTimeRegion );
    
    /**
     * @brief Construct and Set datetime.
     * !NOTE  if update fail that will thow a exception here by ACL_ASSERT
     *
     * @param [in] ai64Year     -   the specified year
     * @param [in] ai64Month    -   the specified month
     * @param [in] ai64Day -   the specified day
     * @param [in] ai64Hour -   the specified hour
     * @param [in] ai64Minute -   the specified minute
     * @param [in] ai64Second -   the specified second
     */
    CDateTime(
        apl_time_t ai64Year,
        apl_time_t ai64Month,
        apl_time_t ai64Day,
        apl_time_t ai64Hour = 0,
        apl_time_t ai64Minute = 0,
        apl_time_t ai64Second = 0 );

    /**
     * @brief Construct and Update datetime from time string and its format.
     * !NOTE  if update fail that will thow a exception here by ACL_ASSERT
     *
     * @param [in] apcStr -   time string
     * @param [in] apcFormat -   time format
     */
    CDateTime( char const* apcStr, char const* apcFormat = DEFAULT_FORMAT );
    
    /**
     * @brief CDateTime copy constructor.
     */
    CDateTime( CDateTime const& aoOther );
    
    /**
     * @brief over write copy operator =.
     *
     * @param [in] aoRhs - source datetime object
     *
     * @return this reference
     */
    CDateTime& operator = ( CDateTime const& aoRhs );
    
    /**
     * @brief Update current local date time.
     *
     * @retval 0 If get local time successfully.
     * @retval -1 If an error is detected.
     */
    apl_int_t Update(void);
    
    /**
     * @brief Update current date time by local time or UTC time.
     *
     * @param [in] aeFlag - time region
     *
     * @retval 0 If set time successfully.
     * @retval -1 If an error is detected.
     */
    apl_int_t Update( ETimeRegion aeFlag );

    /**
     * @brief Update current date time by specified time region value (-12 ~ 12, value 0 is GMT/UTC).
     *
     * @param [in] aiTimeRegion - time region
     *
     * @retval 0 If get datetime successfully.
     * @retval -1 If an error is detected.
     */
    apl_int_t Update( apl_int_t aiTimeRegion );
    
    /**
     * @brief Update local time by specified timestamp.
     *
     * @param [in] aoTimestamp -   the specified timestamp
     *
     * @retval 0 If set time successfully.
     * @retval -1 If an error is detected.
     */
    apl_int_t Update( CTimestamp const& aoTimestamp );
    
    /**
     * @brief Update local time or UTC time by specified timestamp.
     *
     * @param [in] aoTimestamp -   the specified timestamp
     * @param [in] aeFlag -   time region
     *
     * @retval 0 If set time successfully.
     * @retval -1 If an error is detected.
     */
    apl_int_t Update( CTimestamp const& aoTimestamp, ETimeRegion aeFlag );
    
    /**
     * @brief Update date time by specified time region and timestamp.
     *
     * @param [in] aoTimestamp -   the specified timestamp
     * @param [in] aiTimeRegion - time region
     *
     * @retval 0 If get datetime successfully.
     * @retval -1 If an error is detected.
     */
    apl_int_t Update( CTimestamp const& aoTimestamp, apl_int_t aiTimeRegion );
    
    /**
     * @brief Get datetime from time string and its format.
     *
     * @param [in] apcStr -   time string
     * @param [in] apcFormat -   time format
     *
     * @retval 0 If get datetime from time string and its format successfully.
     * @retval -1 If an error is detected.
     */
    apl_int_t Update( char const* apcStr, char const* apcFormat = DEFAULT_FORMAT );
    
    /**
     * @brief Format the datetime according to the specified format.
     *
     * @param [in] apcFormat -   the specified time format
     *
     * @return The format time. Upon successful completion.Otherwise, -1 If an error is detected.
     */
    const char* Format( char const* apcFormat = DEFAULT_FORMAT );
    
    /**
     * @brief Get timestamp.
     *
     * @return The timestamp object.
     */
    CTimestamp GetTimestamp() const;
    
    /**
     * @brief Get timestamp.
     *
     * @param [out] aoTimestamp -   the timestamp object
     *
     * @retval 0 If Get timestamp succefully.
     * @retval -1 If an error is detected.
     */
    apl_int_t GetTimestamp( CTimestamp& aoTimestamp ) const;
    
    /**
     * @brief Set datetime.
     *
     * @param [in] ai64Year     -   the specified year
     * @param [in] ai64Month    -   the specified month
     * @param [in] ai64Day -   the specified day
     * @param [in] ai64Hour -   the specified hour
     * @param [in] ai64Minute -   the specified minute
     * @param [in] ai64Second -   the specified second
     *
     * @retval 0 If set datetime successfully.
     * @retval -1 If an error is detected.
     */
    apl_int_t Set(
        apl_time_t ai64Year,
        apl_time_t ai64Month,
        apl_time_t ai64Day,
        apl_time_t ai64Hour = 0,
        apl_time_t ai64Minute = 0,
        apl_time_t ai64Second = 0 );

    /**
     * @brief Get day of month[1,31].
     *
     * @return The day of month.
     */
    apl_time_t GetDay(void) const;
    
    /**
     * @brief Get month of year[0,11].
     *
     * @return The month of year.
     */
    apl_time_t GetMonth(void) const;
    
    /**
     * @brief Get years since 1900.
     *
     * @return The years since 1900.
     */
    apl_time_t GetYear(void) const;
    
    /**
     * @brief Get hour[0,23].
     *
     * @return The hour.
     */
    apl_time_t GetHour(void) const;
    
    /**
     * @brief Get minute[0,59].
     *
     * @return The minute.
     */
    apl_time_t GetMin(void) const;
    
    /**
     * @brief Get second[0,60].
     *
     * @return The second.
     */
    apl_time_t GetSec(void) const;
    
    /**
     * @brief Get day of week[0,6].
     *
     * @return The day of week.
     */
    apl_time_t GetWeekday(void) const;
    
    /** 
     * @brief Get day of year[0,365].
     *
     * @return The day of year.
     */
    apl_time_t GetYearday(void) const;

    /**
     * @brief The friend member : operator -.
     *
     * @param [in] aoLhs   -   the datetime object
     * @param [in] aoRhs   -   the datetime object
     *
     * @return The time value.
     */
    friend CTimeValue operator -( CDateTime const& aoLhs, CDateTime const& aoRhs );

protected:
    apl_tm_t moTm;
    
    char msBuffer[128];
};

//////////////////////////////////// inline implement /////////////////////////////////////////
inline CTimeValue operator -( CDateTime const& aoLhs, CDateTime const& aoRhs )
{
    return aoLhs.GetTimestamp() - aoRhs.GetTimestamp();
}

ACL_NAMESPACE_END

#endif//ACL_DATETIME_H
