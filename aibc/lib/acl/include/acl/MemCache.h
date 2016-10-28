/**
 * @file MemCache.h
 */

#ifndef ACL_MEMCACHE_H
#define ACL_MEMCACHE_H

#include "acl/Utility.h"
#include "acl/Ring.h"
#include "acl/Timestamp.h"
#include "acl/stl/map.h"

ACL_NAMESPACE_START

/**
 * struct CDefaultReplacePolicy
 * this functors will be called when putting a new node and need to replace timeout node
 */
struct CDefaultReplacePolicy
{
    template<typename T>
    apl_int_t operator () ( T& atOprs )
    {
        return 0;
    }
};

/**
 * struct CDefaultMissPolicy
 * this functors will be called when getting a node and the node is unexisted
 */
struct CDefaultMissPolicy
{
    template<typename KeyType, typename ValueType>
    apl_int_t operator () ( KeyType const& atKey, ValueType* apoValue )
    {
        return -1;
    }
};

/**
 * struct CDefaultVisitPolicy
 * this functors will be called when getting a node and the node is existed
 */
struct CDefaultVisitPolicy
{
    template<typename T>
    apl_int_t operator () ( T& atOprs )
    {
        return 0;
    }
};

////////////////////////////////////////////// IteratorType /////////////////////////////////////
/**
 * class TMemCacheIterator
 */
template
<
    typename KeyType,
    typename ValueType,
    typename NodeType,
    typename LinkHeadType
>
class TMemCacheIterator
{
    template<typename,typename,typename,typename, typename> friend class TMemCache;

public:
    struct CNodePointer
    {
        friend class TMemCacheIterator;
        
        CNodePointer( NodeType* apoNode )
            : mpoNode(apoNode)
        {
        }
                
        const KeyType& Key(void) const
        {
            return this->mpoNode->moIter->first;
        }
        
        const KeyType& GetKey(void) const
        {
            return this->mpoNode->moIter->first;
        }
        
        ValueType& Value(void) const
        {
            return this->mpoNode->moIter->second.moValue;
        }
        
        ValueType& GetValue(void) const
        {
            return this->mpoNode->moIter->second.moValue;
        }
        
        const CTimestamp& Timestamp(void) const
        {
            return this->mpoNode->moIter->second.moTimestamp;
        }
        
        const CTimestamp& GetTimestamp(void) const
        {
            return this->mpoNode->moIter->second.moTimestamp;
        }
        
    protected:
        NodeType* mpoNode;
    };
    
public:
    /**
     * @brief A constructor.
     */
    TMemCacheIterator(void)
        : mpoHead(NULL)
        , moNodePtr(NULL)
    {
    }
    
    /**
     * @brief A constructor.
     *
     * @param [in] apoHead - link head type
     * @param [in] apoNode - node type
     */
    TMemCacheIterator( LinkHeadType* apoHead, NodeType* apoNode )
        : mpoHead(apoHead)
        , moNodePtr(apoNode)
    {
    }
    
    /** 
     * @brief Overloaded operator = 
     *
     * @param [in] aoRhs - the TMemCacheIterator object
     *
     * @return This TMemCacheIterator object
     */
    TMemCacheIterator& operator = ( TMemCacheIterator const& aoRhs )
    {
        this->mpoHead = aoRhs.mpoHead;
        this->moNodePtr.mpoNode = aoRhs.moNodePtr.mpoNode;
        return *this;
    }
    
    /**
     * @brief Overloaded operator ==
     *
     * @param [in] aoRhs - the TMemCacheIterator object
     *
     * @retval true If ==
     * @retval false If !=
     */
    bool operator == ( TMemCacheIterator const& aoRhs )
    {
        return this->mpoHead == aoRhs.mpoHead && this->moNodePtr.mpoNode == aoRhs.moNodePtr.mpoNode;
    }
    
    /**
     * @brief Overloaded operator !=
     *
     * @param [in] aoRhs - the TMemCacheIterator object
     *
     * @retval true If !=
     * @retval false If ==
     */
    bool operator != ( TMemCacheIterator const& aoRhs )
    {
        return !(*this == aoRhs);
    }
    
    /**
     * @brief Overloaded operator ->
     */
    CNodePointer* operator -> (void)
    {
        return &this->moNodePtr;
    }
    
    /**
     * @brief Overloaded operator ++i
     *
     * @return The add-self one.
     */
    TMemCacheIterator& operator ++ (void)
    {
        this->moNodePtr.mpoNode = ACL_RING_NEXT(this->moNodePtr.mpoNode, moLink);
        
        return *this;
    }
    
    /**
     * @brief Overloaded operator i++
     *
     * @return The original one.
     */
    TMemCacheIterator operator ++ (int)
    {
        TMemCacheIterator loTmp(*this);
        
        ++*this;
        
        return loTmp;
    }

protected:
    NodeType* GetNode(void)            { return this->moNodePtr.mpoNode; }

private:
    LinkHeadType* mpoHead;
    CNodePointer moNodePtr;
};

////////////////////////////////////////////// MemCache /////////////////////////////////////
/**
 * class TMemCache
 */
template
<
    typename KeyType,
    typename ValueType,
    typename ReplacePolicyType = CDefaultReplacePolicy,
    typename MissPolicyType = CDefaultMissPolicy,
    typename VisitPolicyType = CDefaultVisitPolicy
>
class TMemCache
{
private:
    struct CValueNode
    {
        CValueNode( ValueType const& aoValue )
            : moValue(aoValue)
        {
            this->moTimestamp.Update();
        }
    
        ACL_RING_ENTRY(CValueNode) moLink;

        ValueType moValue;
        
        CTimestamp moTimestamp;
        
        typename std::map<KeyType, CValueNode>::iterator moIter;
        typename std::multimap<apl_time_t, CValueNode*>::iterator moSlaveIter;
    };

public:
    /**
     * Put switch option enumeration.
     */
    enum EPutOption
    {
        DO_OVERWRITE    = 3,   ///<Overwrite method
        DONT_OVERWRITE  = 4,   ///<Dont-overwrite method
        
        //Keep compatibility, will be delete in next version
        OPT_OVERWRITE   = 1,   ///<Overwrite method
        OPT_NOOVERWRITE = 2    ///<Not-overwrite method
    };

    /**
     * Get switch option enumeration.
     */
    enum EGetOption
    {
        DO_DELETE      = 3,   ///<Delete method
        DONT_DELETE    = 4,   ///<Dont-delete method
        
        //Keep compatibility, will be delete in next version
        OPT_DELETE     = 1,   ///<Delete method
        OPT_NODELETE   = 2    ///<Not-delete method
    };
    
    typedef std::map<KeyType, CValueNode> MapType;
    typedef std::multimap<apl_time_t, CValueNode*> MultiMapType;
        
    class MyValueNodeHead;
    typedef TMemCacheIterator<KeyType, ValueType, CValueNode, MyValueNodeHead> IteratorType;

public: 
    /**
     * @brief A constructor.
     *
     * @param [in] aiCapacity - the capacity of memory cache
     * @param [in] aoReplacePolicy - the replace policy
     * @param [in] aoMissPolicy - the Miss policy
     */
    TMemCache(
        apl_size_t auCapacity = APL_UINT_MAX,
        ReplacePolicyType aoReplacePolicy = ReplacePolicyType(),
        MissPolicyType aoMissPolicy = MissPolicyType(),
        VisitPolicyType aoVisitPolicy = VisitPolicyType() );

    /**
     * @brief Put element to the memory cache.
     *
     * @param [in] atKey - the key
     * @param [in] atValue - the value
     * @param [in] aeFlag - the put option
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.
     */
    apl_int_t Put( KeyType const& atKey, ValueType const& atValue, EPutOption aeFlag = DONT_OVERWRITE );

    /**
     * @brief Get value from memory cache.
     *
     * @param [in] atKey - the key
     * @param [out] aptValue - the value
     * @param [out] apoTimestamp - the build timestamp
     * @param [in] aeFlag - the get option
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.
     */
    apl_int_t Get( KeyType const& atKey, ValueType* aptValue, CTimestamp* apoTimestamp, EGetOption aeFlag = DONT_DELETE );
    
    /**
     * @brief Clear memory cache.
     */
    void Clear(void);
    
    /**
     * @brief Get memory cache size.
     *
     * @return The memory cache size.
     */
    apl_size_t GetSize(void)                  { return this->moMasterMap.size(); }
    
    /**
     * @brief Set the memory cache capacity.
     *
     */
    void SetCapacity( apl_size_t auCapacity ) { this->muCapacity = auCapacity; }
    
    /**
     * @brief Get the memory cache capacity.
     *
     * @return The memory cache capacity.
     */
    apl_size_t GetCapacity(void)              { return this->muCapacity; }
    
    /**
     * @brief Earse the node which the Iterator pointed.
     *
     * @return The Iterator.
     */
    IteratorType Erase( IteratorType aoIter )
    {
        IteratorType loTmp(aoIter);
        
        if ( aoIter == this->End() )
        {
            return this->End();
        }

        ++aoIter;
        
        ACL_RING_REMOVE( loTmp.GetNode(), moLink);
        this->moSlaveMap.erase( loTmp.GetNode()->moSlaveIter);
        this->moMasterMap.erase( loTmp.GetNode()->moIter );
        
        return aoIter;
    }
    
    /**
     * @brief Get the begining of the memory cache IteratorType.
     *
     * @return The begining of the memory cache IteratorType.
     */
    IteratorType Begin(void)
    {
        return IteratorType(&moHead, ACL_RING_FIRST(&this->moHead) );
    }
    
    /**
     * @brief Get the ending of the memory cache IteratorType.
     *
     * @return The ending of the memory cache IteratorType.
     */
    IteratorType End(void)
    {
        return IteratorType(&this->moHead, ACL_RING_SENTINEL(&this->moHead, CValueNode, moLink) );
    }

protected:
    void Visit( CValueNode& aoNode );

private:
    apl_size_t muCapacity;

    ReplacePolicyType mtReplacePolicy;
    
    MissPolicyType mtMissPolicy;
    
    VisitPolicyType mtVisitPolicy;

    MapType moMasterMap;
    
    MultiMapType moSlaveMap;

    ACL_RING_HEAD(MyValueNodeHead, CValueNode) moHead;
};

//////////////////////////////////////////// TMemCache implement /////////////////////////////////////////
template
<
    typename KeyType,
    typename ValueType,
    typename ReplacePolicyType,
    typename MissPolicyType,
    typename VisitPolicyType
>
TMemCache<KeyType, ValueType, ReplacePolicyType, MissPolicyType, VisitPolicyType>::TMemCache(
    apl_size_t auCapacity,
    ReplacePolicyType atReplacePolicy,
    MissPolicyType atMissPolicy,
    VisitPolicyType atVisitPolicy )
    : muCapacity(auCapacity)
    , mtReplacePolicy(atReplacePolicy)
    , mtMissPolicy(atMissPolicy)
    , mtVisitPolicy(atVisitPolicy)
{
    ACL_RING_INIT(&this->moHead, CValueNode, moLink);
}

template
<
    typename KeyType,
    typename ValueType,
    typename ReplacePolicyType,
    typename MissPolicyType,
    typename VisitPolicyType
>
apl_int_t TMemCache<KeyType, ValueType, ReplacePolicyType, MissPolicyType, VisitPolicyType>::Put(
    KeyType const& atKey, ValueType const& atValue, EPutOption aeFlag )
{
    CValueNode loValue(atValue);
    
    if ( this->moMasterMap.size() >= this->muCapacity )
    {
        CValueNode* lpoFirst = ACL_RING_FIRST(&this->moHead);
        
        this->mtReplacePolicy(lpoFirst->moValue);
        
        ACL_RING_REMOVE(lpoFirst, moLink);
        
        this->moMasterMap.erase( lpoFirst->moIter );
        this->moSlaveMap.erase( lpoFirst->moSlaveIter );
    }

    std::pair<typename MapType::iterator, bool> loPair = 
        this->moMasterMap.insert( typename MapType::value_type(atKey, loValue) );

    if (loPair.second == false)
    {
        if (aeFlag == OPT_OVERWRITE || aeFlag == DO_OVERWRITE)
        {
            (loPair.first)->second.moValue = atValue;
        
            this->Visit( (loPair.first)->second );
        }
        else
        {
            return -1;
        }
    }
    else
    {
        typename MultiMapType::iterator loSlaveIter = this->moSlaveMap.insert(
            typename MultiMapType::value_type((loPair.first)->second.moTimestamp.Nsec(), &( (loPair.first)->second ) ) );
        
        (loPair.first)->second.moIter = loPair.first;
        (loPair.first)->second.moSlaveIter = loSlaveIter;
        
        ++loSlaveIter;
        
        if ( loSlaveIter == this->moSlaveMap.end() )
        {
            ACL_RING_INSERT_TAIL(&this->moHead, &( (loPair.first)->second ), CValueNode, moLink);
        }
        else
        {
            ACL_RING_INSERT_BEFORE(loSlaveIter->second, &( (loPair.first)->second ), moLink);
        }
    }

    return 0;
}

template
<
    typename KeyType,
    typename ValueType,
    typename ReplacePolicyType,
    typename MissPolicyType,
    typename VisitPolicyType
>
apl_int_t TMemCache<KeyType, ValueType, ReplacePolicyType, MissPolicyType, VisitPolicyType>::Get( 
    KeyType const& atKey, ValueType* aptValue, CTimestamp* apoTimestamp, EGetOption aeFlag )
{
    apl_int_t  liRetCode = 0;
    
    typename MapType::iterator loIter = this->moMasterMap.find( atKey );
        
    if ( loIter == this->moMasterMap.end() )
    {
        if ( ( liRetCode = this->mtMissPolicy(atKey, aptValue) ) != 0 )
        {
            return liRetCode;
        }
        else
        {
            this->Put( atKey, *aptValue, DONT_OVERWRITE );
            return 0;
        }
    }

    if (aptValue != NULL)
    {
        *aptValue = loIter->second.moValue;
    }
    
    if (apoTimestamp != NULL)
    {
        *apoTimestamp = loIter->second.moTimestamp;
    }
    
    if (aeFlag == OPT_DELETE || aeFlag == DO_DELETE)
    {
        ACL_RING_REMOVE(&loIter->second, moLink);
        this->moSlaveMap.erase(loIter->second.moSlaveIter);
        this->moMasterMap.erase(loIter);
    }
    else
    {
        this->Visit( loIter->second );
    }

    return 0;
}

template
<
    typename KeyType,
    typename ValueType,
    typename ReplacePolicyType,
    typename MissPolicyType,
    typename VisitPolicyType
>
void TMemCache<KeyType, ValueType, ReplacePolicyType, MissPolicyType, VisitPolicyType>::Clear(void)
{
    ACL_RING_INIT(&this->moHead, CValueNode, moLink);
    this->moSlaveMap.clear();
    this->moMasterMap.clear();
}

template
<
    typename KeyType,
    typename ValueType,
    typename ReplacePolicyType,
    typename MissPolicyType,
    typename VisitPolicyType
>
void TMemCache<KeyType, ValueType, ReplacePolicyType, MissPolicyType, VisitPolicyType>::Visit( CValueNode& aoNode )
{
    apl_time_t liTimeKey = 0;
    
    ACL_RING_REMOVE(&aoNode, moLink);
    this->moSlaveMap.erase(aoNode.moSlaveIter);
    
    liTimeKey = aoNode.moTimestamp.Nsec() + mtVisitPolicy(aoNode);
    
    typename MultiMapType::iterator loSlaveIter = this->moSlaveMap.insert(
        typename MultiMapType::value_type( liTimeKey, &aoNode ) );
    
    aoNode.moSlaveIter = loSlaveIter;
    
    ++loSlaveIter;
    
    if ( loSlaveIter == this->moSlaveMap.end() )
    {
        ACL_RING_INSERT_TAIL(&this->moHead, &aoNode, CValueNode, moLink);
    }
    else
    {
        ACL_RING_INSERT_BEFORE(loSlaveIter->second, &aoNode, moLink);
    }
}

//////////////////////////////////////////// MemCache trait DefaultMissPolicy //////////////////////////////////
/**
 * class TMemCache<KeyType, ValueType, ReplacePolicyType, MissPolicyType, CDefaultVisitPolicy>
 */
template
<
    typename KeyType,
    typename ValueType,
    typename ReplacePolicyType,
    typename MissPolicyType
>
class TMemCache<KeyType, ValueType, ReplacePolicyType, MissPolicyType, CDefaultVisitPolicy>
{
private:
    struct CValueNode
    {
        CValueNode( const ValueType& aoValue )
            : moValue(aoValue)
        {
            this->moTimestamp.Update();
        }

        ACL_RING_ENTRY(CValueNode) moLink;
        
        ValueType moValue;
        
        CTimestamp moTimestamp;
        
        typename std::map<KeyType, CValueNode>::iterator moIter;
    };

public:
    /**
     * Put switch option enumeration.
     */
    enum EPutOption
    {
        DO_OVERWRITE    = 3,   ///<Overwrite method
        DONT_OVERWRITE  = 4,   ///<Dont-overwrite method
        
        //Keep compatibility, will be delete in next version
        OPT_OVERWRITE   = 1,   ///<Overwrite method
        OPT_NOOVERWRITE = 2    ///<Not-overwrite method
    };

    /**
     * Get switch option enumeration.
     */
    enum EGetOption
    {
        DO_DELETE      = 3,   ///<Delete method
        DONT_DELETE    = 4,   ///<Dont-delete method
        
        //Keep compatibility, will be delete in next version
        OPT_DELETE     = 1,   ///<Delete method
        OPT_NODELETE   = 2    ///<Not-delete method
    };
    
    typedef CDefaultVisitPolicy VisitPolicyType;
    typedef std::map<KeyType, CValueNode> MapType;
        
    class MyValueNodeHead;
    typedef TMemCacheIterator<KeyType, ValueType, CValueNode, MyValueNodeHead> IteratorType;

public:
    /**
     * @brief A constructor.
     *
     * @param [in] aiCapacity - the memory cache capacity
     * @param [in] aoReplacePolicy - the replace policy 
     * @param [in] aoMissPolicy - the Miss policy  
     */
    TMemCache(
        apl_size_t auCapacity = APL_UINT_MAX,
        ReplacePolicyType atReplacePolicy = ReplacePolicyType(),
        MissPolicyType atMissPolicy = MissPolicyType(),
        VisitPolicyType atVisitPolicy = VisitPolicyType() );

    /**
     * @brief Put element to the memory cache. 
     *
     * @param [in] atKey - the key
     * @param [in] atValue - the value
     * @param [in] aeFlag  - the put option 
     * 
     * @retval 0 Upon successful completion.  
     * @retval -1 Failed.  
     */
    apl_int_t Put( KeyType const& atKey, ValueType const& atValue, EPutOption aeFlag = DONT_OVERWRITE );

    /**
     * @brief Get value from memory cache.
     *
     * @param [in] atKey - the key
     * @param [out] aptValue - the value
     * @param [out] apoTimestamp - the build timestamp
     * @param [in] aeFlag - the get option
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.
     */
    apl_int_t Get( KeyType const& atKey, ValueType* aptValue, CTimestamp* apoTimestamp, EGetOption aeFlag = DONT_DELETE );
    
    /**
     * @brief Clear memory cache.
     */
    void Clear(void);
    
    /**
     * @brief Get memory cache size.
     *
     * @return The memory cache size.
     */
    apl_size_t GetSize(void)                  { return this->moMap.size(); }
    
    /**
     * @brief Set the memory cache capacity.
     *
     */
    void SetCapacity( apl_size_t auCapacity ) { this->muCapacity = auCapacity; }
    
    /**
     * @brief Get the memory cache capacity.
     *
     * @return The memory cache capacity.
     */
    apl_size_t GetCapacity(void)              { return this->muCapacity; }
    
    /**
     * @brief Earse the node which the Iterator pointed.
     *
     * @return The Iterator.
     */
    IteratorType Erase( IteratorType aoIter )
    {
        IteratorType loTmp(aoIter);
        
        if ( aoIter == this->End() )
        {
            return this->End();
        }

        ++aoIter;
        
        ACL_RING_REMOVE( loTmp.GetNode(), moLink);
        this->moMap.erase( loTmp.GetNode()->moIter );
        
        return aoIter;
    }
    
    /**
     * @brief Get the begining of the memory cache IteratorType.
     *
     * @return The begining of the memory cache IteratorType.
     */
    IteratorType Begin(void)
    {
        return IteratorType(&moHead, ACL_RING_FIRST(&this->moHead) );
    }
    
    /**
     * @brief Get the ending of the memory cache IteratorType.
     *
     * @return The ending of the memory cache IteratorType.
     */
    IteratorType End(void)
    {
        return IteratorType(&this->moHead, ACL_RING_SENTINEL(&this->moHead, CValueNode, moLink) );
    }

private:
    apl_size_t muCapacity;

    ReplacePolicyType mtReplacePolicy;
    
    MissPolicyType mtMissPolicy;
    
    VisitPolicyType mtVisitPolicy;

    MapType moMap;

    ACL_RING_HEAD(MyValueNodeHead, CValueNode) moHead;
};

//////////////////////////////////////////// TMemCache implement /////////////////////////////////////////
template
<
    typename KeyType,
    typename ValueType,
    typename ReplacePolicyType,
    typename MissPolicyType
>
TMemCache<KeyType, ValueType, ReplacePolicyType, MissPolicyType, CDefaultVisitPolicy>::TMemCache(
    apl_size_t auCapacity,
    ReplacePolicyType atReplacePolicy,
    MissPolicyType atMissPolicy,
    VisitPolicyType atVisitPolicy )
    : muCapacity(auCapacity)
    , mtReplacePolicy(atReplacePolicy)
    , mtMissPolicy(atMissPolicy)
    , mtVisitPolicy(atVisitPolicy)
{
    ACL_RING_INIT(&this->moHead, CValueNode, moLink);
}

template
<
    typename KeyType,
    typename ValueType,
    typename ReplacePolicyType,
    typename MissPolicyType
>
apl_int_t TMemCache<KeyType, ValueType, ReplacePolicyType, MissPolicyType, CDefaultVisitPolicy>::Put(
    KeyType const& atKey, ValueType const& atValue, EPutOption aeFlag )
{
    CValueNode loValue(atValue);
    
    if ( this->moMap.size() >= this->muCapacity )
    {
        CValueNode* lpoFirst = ACL_RING_FIRST(&this->moHead);
        
        this->mtReplacePolicy(lpoFirst->moValue);
        
        ACL_RING_REMOVE(lpoFirst, moLink);
        
        this->moMap.erase( lpoFirst->moIter );
    }

    std::pair<typename MapType::iterator, bool> loPair = 
        this->moMap.insert( typename MapType::value_type(atKey, loValue) );

    if (loPair.second == false)
    {
        if (aeFlag == OPT_OVERWRITE || aeFlag == DO_OVERWRITE)
        {
            CValueNode& loValueNode = (loPair.first)->second;
            loValueNode.moValue = atValue;
            
            ACL_RING_REMOVE(&loValueNode, moLink);
            ACL_RING_INSERT_TAIL(&this->moHead, &loValueNode, CValueNode, moLink);
            
            loValueNode.moTimestamp.Update();
        }
        else
        {
            return -1;
        }
    }
    else
    {
        (loPair.first)->second.moIter = loPair.first;
        ACL_RING_INSERT_TAIL(&this->moHead, &( (loPair.first)->second ), CValueNode, moLink);
    }

    return 0;
}

template
<
    typename KeyType,
    typename ValueType,
    typename ReplacePolicyType,
    typename MissPolicyType
>
apl_int_t TMemCache<KeyType, ValueType, ReplacePolicyType, MissPolicyType, CDefaultVisitPolicy>::Get(
    KeyType const& atKey, ValueType* aptValue, CTimestamp* apoTimestamp, EGetOption aeFlag )
{
    apl_int_t liRetCode = 0;
    typename MapType::iterator loIter = this->moMap.find( atKey );
        
    if ( loIter == this->moMap.end() )
    {
        if ( ( liRetCode = this->mtMissPolicy(atKey, aptValue) ) != 0 )
        {
            return liRetCode;
        }
        else
        {
            this->Put( atKey, *aptValue, DONT_OVERWRITE );
            return 0;
        }
    }

    if (aptValue != NULL)
    {
        *aptValue = loIter->second.moValue;
    }
    
    if (apoTimestamp != NULL)
    {
        *apoTimestamp = loIter->second.moTimestamp;
    }
    
    if (aeFlag == OPT_DELETE || aeFlag == DO_DELETE)
    {
        ACL_RING_REMOVE(&loIter->second, moLink);
        this->moMap.erase(loIter);
    }
    else
    {
        ACL_RING_REMOVE(&loIter->second, moLink);
        ACL_RING_INSERT_TAIL(&this->moHead, &loIter->second, CValueNode, moLink);
    }

    return 0;
}

template
<
    typename KeyType,
    typename ValueType,
    typename ReplacePolicyType,
    typename MissPolicyType
>
void TMemCache<KeyType, ValueType, ReplacePolicyType, MissPolicyType, CDefaultVisitPolicy>::Clear(void)
{
    ACL_RING_INIT(&this->moHead, CValueNode, moLink);
    this->moMap.clear();
}

ACL_NAMESPACE_END

#endif//ACL_MEMCACHE_H
