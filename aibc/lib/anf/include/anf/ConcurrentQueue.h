
#ifndef ANF_CONCURRENT_QUEUE_H
#define ANF_CONCURRENT_QUEUE_H

#include "anf/Utility.h"
#include "acl/stl/deque.h"
#include "acl/Synch.h"

ANF_NAMESPACE_START

/**
 * Concurrent queue for multi-thread accessing stl::deque interface
 */
template<typename ValueType, typename LockType = acl::CLock>
class TConcurrentQueue
{
public:
    typedef typename std::deque<ValueType>::iterator iterator;
    typedef typename std::deque<ValueType>::value_type value_type;

public:
    TConcurrentQueue(void)
        : muSize(0)
    {
    }
    
    /**
     * Push @aoValue into stl::deque in thread-safe
     *
     * @param [in] aoValue push value
     *
     * @return current queue size
     */
    apl_size_t Push( value_type const& aoValue )
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        this->moQueue.push_back(aoValue);
        
        this->muSize++;

        return this->muSize;
    }

    /**
     * Push @aoValue into stl::deque front in thread-safe
     *
     * @param [in] aoValue push value
     *
     * @return current queue size
     */
    apl_size_t PushFront( value_type const& aoValue )
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        this->moQueue.push_front(aoValue);
        
        this->muSize++;

        return this->muSize;
    }
    
    /**
     * Pop @aoValue from stl::deque in thread-safe
     *
     * @param [in] aoValue pop value
     *
     * @return true should be return if pop successful, otherwise return false
     */
    bool Pop(value_type& aoValue)
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        if (this->muSize > 0)
        {
            aoValue.Swap(this->moQueue.front() );
        
            this->moQueue.pop_front();
            
            this->muSize--;
            
            return true;
        }
        else
        {
            return false;
        }
    }
    
    /**
     * Pop @aoValue from stl::deque in thread-safe
     *
     * @param [in] aoValue pop value
     * @param [in] aoCond pop condition 
     *
     * @return true should be return if pop successful, otherwise return false
     */
    template<typename ConditionType>
    bool PopIf(value_type& aoValue, ConditionType const& aoCond)
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        if (this->muSize > 0)
        {
            value_type& loFront = this->moQueue.front();
            if (!aoCond(loFront) )
            {
                return false;
            }

            aoValue.Swap(loFront);

            this->moQueue.pop_front();
            
            this->muSize--;
            
            return true;
        }
        else
        {
            return false;
        }
    }
     
    /**
     * Pop @aoValue from stl::deque back in thread-safe
     *
     * @param [in] aoValue pop value
     *
     * @return true should be return if pop successful, otherwise return false
     */
    bool PopBack(value_type& aoValue)
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        if (this->muSize > 0)
        {
            aoValue.Swap(this->moQueue.back() );
        
            this->moQueue.pop_back();
            
            this->muSize--;
            
            return true;
        }
        else
        {
            return false;
        }
    }
 
    /**
     * Pop @aoValue from stl::deque back in thread-safe
     *
     * @param [in] aoValue pop value
     * @param [in] aoCond pop condition 
     *
     * @return true should be return if pop successful, otherwise return false
     */
    template<typename ConditionType>
    bool PopBackIf(value_type& aoValue, ConditionType const& aoCond)
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        if (this->muSize > 0)
        {
            value_type& loBack = this->moQueue.back();
            if (!aoCond(loBack) )
            {
                return false;
            }

            aoValue.Swap(loBack);
            
            this->moQueue.pop_back();
            
            this->muSize--;
            
            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * empty queue sync action in thread-safe
     *
     * @param [in] aoAction action
     */
    template<typename ActionType>
    void IfEmptyDo( ActionType& aoAction )
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
        
        if (this->muSize == 0)
        {
            aoAction();
        }
    }
    
    /**
     * Get stl::deque size
     */
    apl_size_t Size(void)
    {
        return this->muSize;
    }
    
    /**
     * Clear stl::deque in thread-safe
     */
    void Clear(void)
    {
        acl::TSmartLock<LockType> loGuard(this->moLock);
            
        return this->moQueue.clear();
    }
    
private:
    LockType moLock;
    std::deque<ValueType> moQueue;
    apl_size_t muSize;
};

ANF_NAMESPACE_END

#endif//ANF_CONCURRENT_SET_H
