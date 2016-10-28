
#ifndef ACL_TIMEVALUE_H
#define ACL_TIMEVALUE_H

#include "Utility.h"

ACL_NAMESPACE_START

class CTimeValue
{
public:
    static const CTimeValue ZERO;
    static const CTimeValue MAXTIME;
    
public:
    CTimeValue();
    
    CTimeValue( double adTimeValue );
    
    CTimeValue( apl_time_t ai64TimeValue, apl_time_t ai64Unit );
    
    void Set( apl_time_t ai64TimeValue, apl_time_t ai64Unit );
    
    void Nsec( apl_time_t ai64Nsec );
    
    void Usec( apl_time_t ai64Usec );
    
    void Msec( apl_time_t ai64Msec );
    
    void Sec( apl_time_t ai64Sec );
    
    void Min( apl_time_t ai64Min );
    
    void Hour( apl_time_t ai64Hour );
    
    void Day( apl_time_t ai64Day );
    
    void Week( apl_time_t ai64Week );
    
    apl_time_t Nsec(void) const;
    
    apl_time_t Usec(void) const;
    
    apl_time_t Msec(void) const;
    
    apl_time_t Sec(void) const;
    
    apl_time_t Min(void) const;
    
    apl_time_t Hour(void) const;
    
    apl_time_t Day(void) const;
    
    apl_time_t Week(void) const;
    
    CTimeValue& operator +=( CTimeValue const& aoRhs );
    
    CTimeValue& operator +=(apl_time_t ai64Rhs );
    
    CTimeValue& operator =( CTimeValue const& aoRhs );
    
    CTimeValue& operator =(apl_time_t ai64Rhs );
    
    CTimeValue& operator -=( CTimeValue const& aoRhs );
    
    CTimeValue& operator -=( apl_time_t ai64Rhs );
    
    CTimeValue operator++(int);
    
    CTimeValue& operator++(void);
    
    CTimeValue operator--(int);
    
    CTimeValue& operator--(void);
    
    friend CTimeValue operator +( CTimeValue const& aoLhs, CTimeValue const& aoRhs);
    friend CTimeValue operator -( CTimeValue const& aoLhs, CTimeValue const& aoRhs);
    friend bool operator <( CTimeValue const& aoLhs, CTimeValue const& aoRhs);
    friend bool operator >( CTimeValue const& aoLhs, CTimeValue const& aoRhs);
    friend bool operator >=( CTimeValue const& aoLhs, CTimeValue const& aoRhs);
    friend bool operator <=( CTimeValue const& aoLhs, CTimeValue const& aoRhs);
    friend bool operator ==( CTimeValue const& aoLhs, CTimeValue const& aoRhs);
    friend bool operator !=( CTimeValue const& aoLhs, CTimeValue const& aoRhs);

protected:
	apl_time_t ci64Value;
};

//////////////////////////////////// inline implement /////////////////////////////////////////
inline CTimeValue operator +( CTimeValue const& aoLhs, CTimeValue const& aoRhs )
{
    CTimeValue loTmp(aoLhs);
    loTmp += aoRhs;
    return loTmp;
}

inline CTimeValue operator -( CTimeValue const& aoLhs, CTimeValue const& aoRhs)
{
    CTimeValue loTmp(aoLhs);
    loTmp -= aoRhs;
    return loTmp;
}

inline bool operator <( CTimeValue const& aoLhs, CTimeValue const& aoRhs)
{
    return aoLhs.Nsec() < aoRhs.Nsec();
}

inline bool operator >( CTimeValue const& aoLhs, CTimeValue const& aoRhs)
{
    return aoLhs.Nsec() > aoRhs.Nsec();
}

inline bool operator >=( CTimeValue const& aoLhs, CTimeValue const& aoRhs)
{
    return aoLhs.Nsec() >= aoRhs.Nsec();
}

inline bool operator <=( CTimeValue const& aoLhs, CTimeValue const& aoRhs)
{
    return aoLhs.Nsec() <= aoRhs.Nsec();
}

inline bool operator ==( CTimeValue const& aoLhs, CTimeValue const& aoRhs)
{
    return aoLhs.Nsec() == aoRhs.Nsec();
}

inline bool operator !=( CTimeValue const& aoLhs, CTimeValue const& aoRhs)
{
    return aoLhs.Nsec() != aoRhs.Nsec();
}

ACL_NAMESPACE_END

#endif//ACL_TIMEVALUE_H
