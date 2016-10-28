/**
 * @file TimeValue.h
 */

#ifndef ACL_TIMEVALUE_H
#define ACL_TIMEVALUE_H

#include "acl/Utility.h"

ACL_NAMESPACE_START

/**
 * class CTimeValue
 */
class CTimeValue
{
public:

    /**
     * define a static const CTimeValue
     *     which name is ZERO
     *     which value is 0
     */
    static const CTimeValue ZERO;

    /**
     * define a static const CTimeValue
     *     which name is MAXTIME
     *     which value is maxvalue
     */
    static const CTimeValue MAXTIME;
    
public:

    /**
     * default CTimeValue Constructor
     */
    CTimeValue();
    
    /**
     * CTimeValue Constructor
     *
     * @param [in] adTimeValue    the specified Seconds that wanted to be set the CTimeValue
     */    
    CTimeValue( double adTimeValue );
    
    /**
     * CTimeValue Constructor
     *
     * @param [in] ai64TimeValue    a apl_time_t that wanted to be set the value of CTimeValue
     * @param [in] ai64Unit         a apl_time_t that wanted to be set the precision of timevalue
     */ 
    CTimeValue( apl_time_t ai64TimeValue, apl_time_t ai64Unit );
    
    /**
     * Set the value of CTimeValue
     *
     * @param [in] ai64TimeValue    the specified apl_time_t that wanted to be set the value of CTimeValue
     * @param [in] ai64Unit         the specified apl_time_t that wanted to be set the precision of CTimeValue
     */
    void Set( apl_time_t ai64TimeValue, apl_time_t ai64Unit );
    
    /**
     * Set the value of CTimeValue by unit of it is Nsec
     *
     * @param [in] ai64Nsec    the specified Nseconds that wanted to be set the value of CTimeValue 
     */
    void Nsec( apl_time_t ai64Nsec );
    
    /**
     * Set the value of CTimeValue by unit of it is Usec
     *
     * @param [in] ai64Usec    the specified Useconds that wanted to be set the value of CTimeValue
     */
    void Usec( apl_time_t ai64Usec );
    
    /**
     * Set the value of CTimeValue by unit of it is Msec
     *
     * @param [in] ai64Msec    the specified Msecs that wanted to be set the value of CTimeValue
     */
    void Msec( apl_time_t ai64Msec );
    
    /**
     * Set the value of CTimeValue by unit of it is Sec
     *
     * @param [in] ai64Sec    the specified Seconds that wanted to be set the value of CTimeValue
     */ 
    void Sec( apl_time_t ai64Sec );
    
    /**
     * Set the value of CTimeValue by unit of it is Min
     *
     * @param [in]  ai64Min    the specified Minutes that wanted to be set the value of CTimeValue
     */
    void Min( apl_time_t ai64Min );
    
    /** 
     * Set the value of CTimeValue by unit of it is Hour
     *
     * @param [in]  ai64Hour    the specified Hours that wanted to be set the value of CTimeValue
     */ 
    void Hour( apl_time_t ai64Hour );
    
    /**
     * Set the value of CTimeValue by unit of it is day
     *
     * @param [in] ai64Day    the specified Days that wanted to be set the value of CTimeValue
     */
    void Day( apl_time_t ai64Day );
    
    /**
     * Set the value of CTimeValue by unit of it is week
     *
     * @param [in] ai64Week    the specified Weeks that wanted to be set the value of CTimeValue 
     */
    void Week( apl_time_t ai64Week );
    
    /**
     * Get the value of CTimeValue by transform to Nsec
     *
     * @return    the value of CTimeValue by transform to Nsec.
     */
    apl_time_t Nsec(void) const;
    
    /**
     * Get the value of CTimeValue by transform to Usec
     *
     * @return    the value of CTimeValue by transform to Usec. 
     */
    apl_time_t Usec(void) const;
    
    /**
     * Get the value of CTimeValue by transform to Msec 
     *
     * @return    the value of CTimeValue by transform to Msec. 
     */ 
    apl_time_t Msec(void) const;
    
    /**
     * Get the value of CTimeValue by transform to Sec
     *
     * @return    the value of CTimeValue by transform to Sec. 
     */
    apl_time_t Sec(void) const;
    
    /**
     * Get the value of CTimeValue by transform to Min
     *
     * @return    the value of CTimeValue by transform to Min.
     */
    apl_time_t Min(void) const;
    
    /**
     * Get the value of CTimeValue by transform to Hour
     *
     * @return    the value of CTimeValue by transform to Hour. 
     */
    apl_time_t Hour(void) const;
    
    /**
     * Get the value of CTimeValue by transform to day
     *
     * @return    the value of CTimeValue by transform to day. 
     */
    apl_time_t Day(void) const;
    
    /**
     * Get the value of CTimeValue by transform to week
     *
     * @return    the value of CTimeValue by transform to week. 
     */
    apl_time_t Week(void) const;
    
    /**
     * Overloaded operator +=
     *
     * @param [in] aoRhs    the specified CTimeValue wanted to be added
     * @return    the CTimeValue type.
     */
    CTimeValue& operator +=( CTimeValue const& aoRhs );
    
    /**
     * Overloaded operator +=
     *
     * @param [in] ai64Rhs   the specified apl_time_t wanted to be added
     * @return    the CTimeValue type. 
     */
    CTimeValue& operator +=(apl_time_t ai64Rhs );
    
    /**
     * Overloaded operator=
     *
     * @param [in] aoRhs    the specified CTimeValue wanted to assign
     * @return    the CTimeValue type.
     */
    CTimeValue& operator =( CTimeValue const& aoRhs );
    
    /**
     * Overloaded operator=
     *
     * @param [in] ai64Rhs    the specified apl_time_t wanted to assign
     * @return    the CTimeValue type.
     */
    CTimeValue& operator =(apl_time_t ai64Rhs );
    
    /**
     * Overloaded operator -=
     *
     * @param [in] aoRhs    the specified CTimeValue wanted to be decrease
     * @return    the CTimeValue type. 
     */
    CTimeValue& operator -=( CTimeValue const& aoRhs );
    
    /**
     * Overloaded operator -=
     *
     * @param [in] ai64Rhs    the specified apl_time_t wanted to be decrease
     * @return    the CTimeValue type.
     */
    CTimeValue& operator -=( apl_time_t ai64Rhs );
    
    /**
     * Overloaded operator ++(int). Add 1 Sec to CTimeValue
     *
     * @return    the CTimeValue type.
     */
    CTimeValue operator++(int);
    
    /**
     * Overloaded operator ++(void). Add 1 Sec to CTimeValue
     *
     * @return    the CTimeValue type.
     */
    CTimeValue& operator++(void);
    
    /**
     * Overloaded operator --(int). Decrease 1 Sec to CTimeValue
     *
     * @return    the CTimeValue type.
     */
    CTimeValue operator--(int);
    
    /**
     * Overloaded operator --(void). Decrease 1 Sec to CTimeValue
     *
     * @return    the CTimeValue type.
     */
    CTimeValue& operator--(void);
    
    /**
     * Overloaded operator +.Add two CTimeValue
     *
     * @param [in] aoLhs    the specified CTimeValue 
     * @param [in] aoRhs    the specified CTimeValue
     * @return    the CTimeValue type. 
     */
    friend CTimeValue operator +( CTimeValue const& aoLhs, CTimeValue const& aoRhs);

    /**
     * Overloaded operator -. Decrease two CTimeValue
     *
     * @param [in] aoLhs    the specified CTimeValue 
     * @param [in] aoRhs    the specified CTimeValue
     * @return    the CTimeValue type. 
     */
    friend CTimeValue operator -( CTimeValue const& aoLhs, CTimeValue const& aoRhs);

    /**
     * Overloaded operator <.
     *
     * @param [in] aoLhs    the specified CTimeValue 
     * @param [in] aoRhs    the specified CTimeValue
     * @retval true    the value of aoLhs < the value of aoRhs. 
     * @retval false   the value of aoLhs >= the value of aoRhs.
     */
    friend bool operator <( CTimeValue const& aoLhs, CTimeValue const& aoRhs);

    /**
     * Overloaded operator >
     * 
     * @param [in] aoLhs    the CTimeValue
     * @param [in] aoRhs    the CTimeValue
     * @retval ture    the value of aoLhs > the value of aoRhs.
     * @retval false   the value of aoLhs <= the value of aoRhs.
     */ 
    friend bool operator >( CTimeValue const& aoLhs, CTimeValue const& aoRhs);

    /**
     * OVerloaded operator >=
     *
     * @param [in] aoLhs    the CTimeValue
     * @param [in] aoRhs    the CTimeValue
     * @retval true    the value of aoLhs >= the value of aoRhs
     * @retval false   the value of aoLhs <  the value of aoRhs
     */
    friend bool operator >=( CTimeValue const& aoLhs, CTimeValue const& aoRhs);

    /**
     * Overloaded operator <= 
     * 
     * @param [in] aoLhs    the CTimeValue
     * @param [in] aoRhs    the CTimeValue
     * @retval true    the value of aoLhs <= the value of aoRhs
     * @retval false   the value of aoLhs >  the value of aoRhs
     */
    friend bool operator <=( CTimeValue const& aoLhs, CTimeValue const& aoRhs);

    /**
     * Overloaded operator ==
     *
     * @param [in] aoLhs    the CTimeValue
     * @param [in] aoRhs    the CTimeValue
     * @retval true    the value of aoLhs == the value of aoRhs
     * @retval false   the value of aoLhs != the value of aoRhs
     */
    friend bool operator ==( CTimeValue const& aoLhs, CTimeValue const& aoRhs);

    /**
     * Overloaded operator !=
     *
     * @param [in] aoLhs    the CTimeValue
     * @param [in] aoRhs    the CTimeValue
     * @retval true    the value of aoLhs != the value of aoRhs
     * @retval false   the value of aoLhs == the value of aoRhs
     */
    friend bool operator !=( CTimeValue const& aoLhs, CTimeValue const& aoRhs);

protected:
	apl_time_t mi64Value;
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
