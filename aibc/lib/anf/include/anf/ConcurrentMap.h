
#ifndef ANF_CONCURRENT_MAP_H
#define ANF_CONCURRENT_MAP_H

#include "anf/Utility.h"
#include "acl/stl/map.h"
#include "acl/Synch.h"

ANF_NAMESPACE_START

/**
 * Concurrent map for multi-thread accessing stl::map interface
 */
template<
    typename KeyType,
    typename ValueType,
    typename LockType = acl::CLock,
    typename CompType = std::less<KeyType>
>
class TConcurrentMap
{
public:
    typedef typename std::map<KeyType, ValueType, CompType>::iterator iterator;
    typedef typename std::map<KeyType, ValueType, CompType>::value_type value_type;

public:
    /**
     * Insert @aoValue into stl::map in thread-safe
     *
     * @param [in] aoValue insert value pair
     */
    void Insert( value_type const& aoValue )
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        this->moMap.insert(aoValue);
    }
    
    /**
     * Insert key/value pair @aoKey/@aoValue into stl::map in thread-safe
     *
     * @param [in] aoKey insert key
     * @param [in] aoValue insert value
     */
    void Insert( KeyType const& aoKey, ValueType const& aoValue )
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        this->moMap[aoKey] = aoValue;
    }
    
    /**
     * Erase @aoKey from stl::map in thread-safe
     *
     * @param [in] aoKey erase key
     */
    void Erase( KeyType const& aoKey )
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
            
        this->moMap.erase(aoKey);
    }
    
    /**
     * Find @aoKey from stl::map in thread-safe
     *
     * @param [in] aoKey find key
     *
     * @return true should be return if element existed, otherwise return false
     */
    bool Find( KeyType const& aoKey )
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        if  (this->moMap.find(aoKey) != this->moMap.end() )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    /**
     * Find @aoKey from stl::map in thread-safe
     *
     * @param [in] aoKey find key
     * @param [out] aoValue output value
     *
     * @return true should be return if element existed, otherwise return false
     */
    bool Find( KeyType const& aoKey, ValueType& aoValue )
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        iterator loIter = this->moMap.find(aoKey);
        
        if  (loIter != this->moMap.end() )
        {
            aoValue = loIter->second;
            return true;
        }
        else
        {
            return false;
        }
    }
    
    /**
     * Find @aoKey from stl::map in thread-safe
     *
     * @param [out] aoValue output value
     * @param [in] aoPred find predication function
     *
     * @return true should be return if element existed, otherwise return false
     */
    template<class PredicationType>
    bool FindIf(PredicationType const& aoPred, ValueType& aoValue)
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        for (iterator loIter = this->moMap.begin(); loIter != this->moMap.end(); ++loIter)
        {
            if (aoPred(loIter) )
            {
                aoValue = loIter->second;
                return true;
            }
        }
        
        return false;
    }
    
    /**
     * Get stl::map size
     */
    apl_size_t Size(void)
    {
        return this->moMap.size();
    }
    
    /**
     * Clear stl::map in thread-safe
     */
    void Clear(void)
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
            
        return this->moMap.clear();
    }
    
    /**
     * Copy stl::map element to @aoMap in thread-safe
     *
     * @param [out] aoMap result
     */
    void Copy(std::map<KeyType, ValueType, CompType>& aoMap)
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        aoMap = this->moMap;
    }
    
    /**
     * Copy stl::map element to @aoMap in thread-safe
     *
     * @param [out] aoMap result
     * @param [in] aoPred copy predication function
     */
    template<class PredicationType>
    void CopyIf(PredicationType const& aoPred, std::map<KeyType, ValueType, CompType>& aoMap)
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        for (iterator loIter = this->moMap.begin(); loIter != this->moMap.end(); ++loIter)
        {
            if (aoPred(loIter) )
            {
                aoMap[loIter->first] = loIter->second;
            }
        }
    }
    
    /**
     * Call function on all stl::map element in thread-safe
     *
     * @param [in] aoAction Call function
     */
    template<class ActionType>
    void All( ActionType& aoAction )
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        for (iterator loIter = this->moMap.begin(); loIter != this->moMap.end(); ++loIter)
        {
            aoAction(loIter);
        }
    }
    
private:
    LockType moLock;
    std::map<KeyType, ValueType, CompType> moMap;
};

ANF_NAMESPACE_END

#endif//ANF_CONCURRENT_MAP_H
