/**
 * @file Any.h
 */

#ifndef ACL_ANY_H
#define ACL_ANY_H

#include "acl/Utility.h"
#include "acl/stl/algorithm.h"
#include <typeinfo>

ACL_NAMESPACE_START

/**
 * class CAny
 */
class CAny
{
public: // structors
    /**
     * @brief A constructor.
     */
    CAny(void)
      : mpoContent(NULL)
    {
    }

    /**
     * @brief A constructor.
     *
     * @tparam ValueType - value type template
     * @param  atValue  - any type value
     */
    template<typename ValueType>
    CAny( ValueType const& atValue )
      : mpoContent(NULL)
    {
        ACL_NEW_ASSERT( this->mpoContent, CHolder<ValueType>(atValue) );
    }

    /**
     * @brief A copy constructor.
     *
     * @param aoOther - the other CAny object
     */
    CAny( CAny const& aoOther )
      : mpoContent(aoOther.mpoContent ? aoOther.mpoContent->Clone() : NULL)
    {
    }

    /**
     * @brief A destructor.
     */
    ~CAny()
    {
        ACL_DELETE(this->mpoContent);
    }

public: // modifiers

    /**
     * @brief Swap Cany object.
     *
     * @param [in] aoRhs - the specified CAny object
     *
     * @return This CAny object.
     */
    CAny& Swap( CAny& aoRhs )
    {
        std::swap( this->mpoContent, aoRhs.mpoContent );
        return *this;
    }

    /**
     * @brief Overloaded operator = 
     *
     * @tparam ValueType - the value type template
     * @param [in] aoRhs - the specified type value
     *
     * @return This CAny object.
     */
    template<typename ValueType>
    CAny& operator = (  ValueType const& aoRhs )
    {
        CAny(aoRhs).Swap(*this);
        return *this;
    }

    /**
     * @brief Overloaded operator =
     *
     * @param [in] aoRhs - the specified CAny object
     *
     * @return This CAny object.
     */
    CAny& operator = ( const CAny& aoRhs )
    {
        CAny(aoRhs).Swap(*this);
        return *this;
    }

public: // queries
    /**
     * @brief Check the CAny object is empty or not.
     *
     * @retval true The CAny object is empty.
     * @retval false The CAny object isn't empty.
     */
    bool IsEmpty(void) const
    {
        return this->mpoContent == NULL;
    }
    
    /**
     * @brief Cast.
     *
     * @return The specified type value.
     */
    template<typename ValueType>
    ValueType* CastTo(void) const
    {
        return this->CastTo0( (ValueType*)0 );
    }

    /**
     * @brief Cast.
     *
     * @return The specified type value.
     */
    template<typename ValueType>
    bool CastTo( ValueType& aoValue ) const
    {
        if (this->mpoContent == NULL || this->mpoContent->Type() != typeid(ValueType) )
        {
            return false;
        }
        else
        {
            aoValue = static_cast< CAny::CHolder<ValueType>* >(this->mpoContent)->mtHeld;
            return true;
        }
    }

protected:
    // Define for AnyCast in some compiling environment
    template<typename ValueType>
    ValueType* CastTo0( ValueType* ) const
    {
        if (this->mpoContent == NULL || this->mpoContent->Type() != typeid(ValueType) )
        {
            return NULL;
        }
        else
        {
            return &static_cast< CAny::CHolder<ValueType>* >(this->mpoContent)->mtHeld;
        }
    }
    
public:
    template<typename ValueType> friend inline ValueType* AnyCast( CAny const& aoOperand );
    template<typename ValueType> friend inline ValueType* AnyCast( CAny const* apoOperand );
    
private:
    class CPlaceHolder
    {
    public:
        virtual ~CPlaceHolder(){};
        
        virtual const std::type_info& Type(void) const = 0;
            
        virtual CPlaceHolder* Clone(void) const = 0;
    };
    
    template<typename ValueType>
    class CHolder : public CPlaceHolder
    {
    public:
        CHolder( const ValueType& aoValue )
            : mtHeld(aoValue)
        {
        }
        
        virtual const std::type_info& Type(void) const 
        {
            return typeid(this->mtHeld);
        }
            
        virtual CPlaceHolder* Clone() const
        {
            CHolder* lpoHolder = NULL;
            
            ACL_NEW_ASSERT( lpoHolder, CHolder(this->mtHeld) );
            
            return lpoHolder;
        }
        
    public:
        ValueType mtHeld;
    };
    
    CPlaceHolder* mpoContent;
};

///////////////////////////////////////////////////////////////////////////////////////
// friends inline safe cast function
template<typename ValueType> inline ValueType* AnyCast( CAny const& aoOperand )
{
    return aoOperand.CastTo0( (ValueType*)0 );
}

template<typename ValueType> inline ValueType* AnyCast( CAny const* apoOperand )
{
    return apoOperand != NULL ? apoOperand->CastTo0( (ValueType*)0 ) : NULL;
}

template<typename ValueType> inline bool AnyCast( CAny const& aoOperand, ValueType& aoValue )
{
    return aoOperand.CastTo(aoValue);
}

ACL_NAMESPACE_END

#endif//ACL_ANY_H
