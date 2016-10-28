/**
 * @file Number.h
 */

#ifndef ACL_NUMBER_H
#define ACL_NUMBER_H

#include "acl/Utility.h"
#include "acl/Synch.h"

ACL_NAMESPACE_START

/**
 * class TNumber
 */
template< class T, class LockType = CNullLock >
class TNumber
{
public:
    /**
     * @brief A constructor.
     */
    TNumber(void)
        : mtVal(0)
        , mbIsValid(false)
    {
    }
    
    /**
     * @brief A constructor.
     *
     * @param [in] n - the number
     */
    TNumber( T atN )
        : mtVal(atN)
        , mbIsValid(true)
    {
    }
    
    /**
     * @brief A copy constructor.
     */
    TNumber( TNumber const& aoOther )
        : mtVal(aoOther.mtVal)
        , mbIsValid(aoOther.mbIsValid)
    {
    }
    
    /**
     * @brief Check the Number object is empty or not.
     *
     * @retval true The Number object is empty.
     * @retval false The Number object isn't empty.
     */
    bool IsEmpty(void) const
    {
        return this->mbIsValid == false;
    }

    // Operation
    /**
     * @brief Overloaded operator =
     *
     * @param [in] atVal - assign value
     *
     * @return Number current value
     */
    T operator = ( const T atN )
    {
        TSmartLock<LockType> loLock(this->moLock);
        this->mbIsValid = true;
        this->mtVal     = atN;
        return this->mtVal;
    }

    /**
     * @brief Overloaded operator =
     *
     * @param [in] aoN - assign value
     *
     * @return Number current value
     */
    T operator = ( const TNumber& aoN )
    {
        T ltTemp = (*this = (T)aoN);
        return ltTemp;
    }
    
    /**
     * @brief Overloaded operator =
     *
     * @param [in] aoN - assign value
     *
     * @return Number current value
     */
    T operator = ( TNumber& aoN )
    {
        T ltTemp = (*this = (T)aoN);
        return ltTemp;
    }
    
    /**
     * @brief Overloaded operator +=
     *
     * @param [in] atN - assign value
     *
     * @return Number current value 
     */
    T operator += ( T atN )
    {
        TSmartLock<LockType> loLock(this->moLock);
        
        this->mtVal += atN;
        
        return this->mtVal;
    }
    
    /**
     * @brief Overloaded operator +=
     *
     * @param [in] atN - assign value
     *
     * @return Number current value 
     */
    T operator += ( const TNumber& aoN )
    {
        T ltTemp = (*this += (T)aoN);
        return ltTemp;
    }
    
    /**
     * @brief Overloaded operator +=
     *
     * @param [in] atN - assign value
     *
     * @return Number current value 
     */
    T operator += ( TNumber& aoN )
    {
        T ltTemp = (*this += (T)aoN);
        return ltTemp;
    }
    
    /**
     * @brief Overloaded operator -=
     *
     * @param [in] atN - assign value
     *
     * @return Number current value 
     */
    T operator -= ( T atN )
    {
        TSmartLock<LockType> loLock(this->moLock);
        
        this->mtVal -= atN;
        
        return this->mtVal;
    }
    
    /**
     * @brief Overloaded operator -=
     *
     * @param [in] atN - assign value
     *
     * @return Number current value 
     */
    T operator -= ( const TNumber& aoN )
    {
        T ltTemp = (*this -= (T)aoN);
        return ltTemp;
    }
    
    /**
     * @brief Overloaded operator -=
     *
     * @param [in] atN - assign value
     *
     * @return Number current value 
     */
    T operator -= ( TNumber& aoN )
    {
        T ltTemp = (*this -= (T)aoN);
        return ltTemp;
    }

    /**
     * @brief Overloaded operator ++i
     *
     * @return Number current value 
     */
    T operator ++ (void)
    {
        TSmartLock<LockType> loLock(this->moLock);

        return ++this->mtVal;
    }
    
    /**
     * @brief Overloaded operator --i
     *
     * @return Number current value 
     */
    T operator -- (void)
    {
        TSmartLock<LockType> loLock(this->moLock);

        return --this->mtVal;
    }
    
    /**
     * @brief Overloaded operator i++
     *
     * @return Number current value 
     */
    T operator ++ (int)
    {
        TSmartLock<LockType> loLock(this->moLock);
        
        T ltTemp(this->mtVal++);
        
        return ltTemp;
    }
    
    /**
     * @brief Overloaded operator i--
     *
     * @return Number current value 
     */
    T operator -- (int)
    {
        TSmartLock<LockType> loLock(this->moLock);
        
        T ltTemp(this->mtVal--);
        
        return ltTemp;
    }
    
    /**
     * @brief Cast.
     */
    operator T (void)
    {
        TSmartLock<LockType> loLock(this->moLock);
        
        return this->mtVal;
    }
       
    /**
     * @brief Cast.
     */
    operator T (void) const
    {
        return this->mtVal;
    }

private:
    T mtVal;
    bool mbIsValid;
    LockType moLock;
};

ACL_NAMESPACE_END

#endif//ACL_NUMBER_H
