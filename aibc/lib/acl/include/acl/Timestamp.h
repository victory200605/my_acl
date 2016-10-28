/**
 * @file Timestamp.h
 */

#ifndef ACL_TIMESTAMP_H
#define ACL_TIMESTAMP_H

#include "acl/Utility.h"
#include "acl/TimeValue.h"

ACL_NAMESPACE_START

/**
 * class CTimestamp
 */
class CTimestamp
{
public:

    /**
     * timestamp Precison enumeration
     */
    enum EPrecision
    {
        /**
         * PRC_NSEC: option of Precisioni. Nsec
         */
        PRC_NSEC = 1,
        /**
         * PRC_USEC: option of Precision. Usec
         */
        PRC_USEC,
        /**
         * PRC_MSEC: option of Precision. Msec
         */
        PRC_MSEC,
        /**
         * PRC_SEC: option of Precision. Sec
         */
        PRC_SEC
    };
    
public:

    /**
     * A Constructor
     */
    CTimestamp();
    
    /**
     * A Constructor
     *
     * @param [in] ai64Timestamp    the specified Nsec
     */
    CTimestamp( apl_time_t ai64Timestamp );
    
    /**
     * A Construct and Update current time
     *
     * @param [in] aePrecision time Precision
     */
    CTimestamp( EPrecision aePrecision );
    
    /**
     * Update the current the value of timestamp by the option of Precision
     *
     * @param [in] aePrecision    the option of Precision
     * @retval 0   If successful
     * @retval -1  If fail
     */
    apl_int_t Update( EPrecision aePrecision = PRC_SEC );

    /**
     * set the value of timestamp according to apl_time_t.
     *
     * @param [in] ai64Timestamp    the specified Nsec 
     */
    void Nsec( apl_time_t ai64Timestamp );
    
    /**
     * get the value of timestamp by transformed to Nsec.
     *
     * @return    the value of timestamp by transformed to Nsec.
     */
    apl_time_t Nsec() const;
    
    /**
     * set the value of timestamp according to apl_time_t.
     *
     * @param [in] ai64Timestamp    the specified Usec 
     */
    void Usec( apl_time_t ai64Timestamp );
   
    /** get the value of timestamp by transform to Usec.
     *
     * @return    the value of timestamp by transform to Usec
     */
    apl_time_t Usec() const;
  
    /**
     * set the value of timestamp according to apl_time_t.
     *
     * @param [in] ai64Timestamp    the specified Msec 
     */  
    void Msec( apl_time_t ai64Timestamp );
    
    /** get the value of timestamp by transform to Msec.
     *
     * @return    the value of timestamp by transform to Msec.
     */
    apl_time_t Msec() const;
    
    /**
     * set the value of timestamp according to apl_time_t.
     *
     * @param [in] ai64Timestamp    the specified Sec 
     */
    void Sec( apl_time_t ai64Timestamp );
    
    /** get the value of timestamp by transform to Sec.
     *
     * @return    the value of timestamp by transform to Sec. 
     */
    apl_time_t Sec() const;
    
    /**
     * Overloaded operator +=
     * 
     * @param [in] aoRhs    CTimeValue type
     * @return    CTimestamp type
     */
    CTimestamp& operator += ( CTimeValue const& aoRhs );
    
    /**
     * Overloaded operator -=
     * 
     * @param [in] aoRhs    CTimeValue type
     * @return    CTmestamp type
     */
    CTimestamp& operator -= ( CTimeValue const& aoRhs );
    
    /**
     * Overloaded operator -
     *
     * @param [in] aoLhs    CTimestamp type
     * @param [in] aoRhs    CTimestamp type
     * @return    CTimeValue type.
     */
    friend CTimeValue operator -( CTimestamp const& aoLhs, CTimestamp const& aoRhs );
    
    /**
     * Overloaded operator -
     *
     * @param [in] aoLhs    CTimestamp type
     * @param [in] aoRhs    CTimeValue type
     * @return    CTimestamp type.
     */
    friend CTimestamp operator +( CTimestamp const& aoLhs, CTimeValue const& aoRhs );
    
    /**
     * Overloaded operator <.
     *
     * @param [in] aoLhs     the specified CTimestamp 
     * @param [in] aoRhs     the specified CTimestamp
     * @retval true    the value of aoLhs < the value of aoRhs 
     * @retval false   the value of aoLhs >= the value of aoRhs
     */
    friend bool operator <( CTimestamp const& aoLhs, CTimestamp const& aoRhs);

    /**
     * Overloaded operator >
     * 
     * @param [in] aoLhs     the CTimestamp
     * @param [in] aoRhs     the CTimestamp
     * @reval true    the value of aoLhs > the value of aoRhs
     * @reval false   the value of aoLhs <= the value of aoRhs
     */ 
    friend bool operator >( CTimestamp const& aoLhs, CTimestamp const& aoRhs);

    /**
     * OVerloaded operator >=
     *
     * @param [in] aoLhs    the CTimestamp
     * @param [in] aoRhs    the CTimestamp
     * @retval true    the value of aoLhs >= the value of aoRhs
     * @retval false   the value of aoLhs <  the value of aoRhs
     */
    friend bool operator >=( CTimestamp const& aoLhs, CTimestamp const& aoRhs);

    /**
     * Overloaded operator <= 
     * 
     * @param [in] aoLhs    the CTimestamp
     * @param [in] aoRhs    the CTimestamp
     * @retval true    the value of aoLhs <= the value of aoRhs
     * @retval false   the value of aoLhs >  the value of aoRhs
     */
    friend bool operator <=( CTimestamp const& aoLhs, CTimestamp const& aoRhs);

    /**
     * Overloaded operator ==
     *
     * @param [in] aoLhs    the CTimestamp
     * @param [in] aoRhs    the CTimestamp
     * @retval true    the value of aoLhs == the value of aoRhs
     * @retval false   the value of aoLhs != the value of aoRhs
     */
    friend bool operator ==( CTimestamp const& aoLhs, CTimestamp const& aoRhs);

    /**
     * Overloaded operator !=
     *
     * @param [in] aoLhs    the CTimestamp
     * @param [in] aoRhs    the CTimestamp
     * @retval true    the value of aoLhs != the value of aoRhs
     * @retval false   the value of aoLhs == the value of aoRhs
     */
    friend bool operator !=( CTimestamp const& aoLhs, CTimestamp const& aoRhs);

protected:
    apl_time_t mi64Value;
};

//////////////////////////////////// inline implement /////////////////////////////////////////
inline CTimeValue operator -( CTimestamp const& aoLhs, CTimestamp const& aoRhs )
{
    CTimeValue loTmp( aoLhs.Nsec() - aoRhs.Nsec(), APL_TIME_NSEC );
    
    return loTmp;
}

inline CTimestamp operator +( CTimestamp const& aoLhs, CTimeValue const& aoRhs )
{
    CTimestamp loTmp(aoLhs);
    
    loTmp += aoRhs;
    
    return loTmp;
}

inline bool operator <( CTimestamp const& aoLhs, CTimestamp const& aoRhs)
{
    return aoLhs.Nsec() < aoRhs.Nsec();
}

inline bool operator >( CTimestamp const& aoLhs, CTimestamp const& aoRhs)
{
    return aoLhs.Nsec() > aoRhs.Nsec();
}

inline bool operator >=( CTimestamp const& aoLhs, CTimestamp const& aoRhs)
{
    return aoLhs.Nsec() >= aoRhs.Nsec();
}

inline bool operator <=( CTimestamp const& aoLhs, CTimestamp const& aoRhs)
{
    return aoLhs.Nsec() <= aoRhs.Nsec();
}

inline bool operator ==( CTimestamp const& aoLhs, CTimestamp const& aoRhs)
{
    return aoLhs.Nsec() == aoRhs.Nsec();
}

inline bool operator !=( CTimestamp const& aoLhs, CTimestamp const& aoRhs)
{
    return aoLhs.Nsec() != aoRhs.Nsec();
}

ACL_NAMESPACE_END

#endif//ACL_TIMESTAMP_H
