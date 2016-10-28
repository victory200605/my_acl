
#ifndef ACL_TIMESTAMP_H
#define ACL_TIMESTAMP_H

#include "Utility.h"
#include "TimeValue.h"

ACL_NAMESPACE_START

class CTimestamp
{
public:
    enum EPrecision
    {
        PRC_NSEC = 1,
        PRC_MSEC,
        PRC_USEC,
        PRC_SEC
    };
    
public:
    CTimestamp();
    
    CTimestamp( apl_time_t ai64Timestamp );
    
    apl_int_t Update( EPrecision aePrecision = PRC_SEC );

    void Nsec( apl_time_t ai64Timestamp );
    
    apl_time_t Nsec() const;
    
    void Usec( apl_time_t ai64Timestamp );
    
    apl_time_t Usec() const;
    
    void Msec( apl_time_t ai64Timestamp );
    
    apl_time_t Msec() const;
    
    void Sec( apl_time_t ai64Timestamp );
    
    apl_time_t Sec() const;
    
    CTimestamp& operator += ( CTimeValue const& aoRhs );
    
    CTimestamp& operator -= ( CTimeValue const& aoRhs );
    
    friend CTimeValue operator -( CTimestamp const& aoLhs, CTimestamp const& aoRhs );

protected:
    apl_time_t ci64Value;
};

//////////////////////////////////// inline implement /////////////////////////////////////////
inline CTimeValue operator -( CTimestamp const& aoLhs, CTimestamp const& aoRhs )
{
    CTimeValue loTmp( aoLhs.Nsec() - aoRhs.Nsec(), APL_TIME_NSEC );
    
    return loTmp;
}

ACL_NAMESPACE_END

#endif//ACL_TIMESTAMP_H
