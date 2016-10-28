
#ifndef ACL_MSGQUEUE_H
#define ACL_MSGQUEUE_H

#include "acl/Utility.h"
#include "acl/Synch.h"

ACL_NAMESPACE_START

/**
 * struct Msg queue node
 */
template<typename T>
struct CMsgQueueNode
{
    CMsgQueueNode( T const& t ) 
        : mtValue(t)
        , mpoNext(NULL)
    {
    }
        
    T mtValue;
    CMsgQueueNode* mpoNext;
};

/**
 * struct Msg queue iterator
 */
template<typename T>
class CMsgQueueIterator
{
public:
    typedef CMsgQueueNode<T> NodeType;
    
public:
    CMsgQueueIterator()
        : mpoNode(NULL)
    {
    }
    
    CMsgQueueIterator( NodeType* apoNode )
        : mpoNode(apoNode)
    {
    }
    
    T& operator * ()
    {
        return this->mpoNode->mtValue;
    }
    
    T* operator -> ()
    {
        return &(*this);
    }
    
    bool operator == ( const CMsgQueueIterator& aoRh )
    {
        return this->mpoNode == aoRh.mpoNode;
    }
    
    bool operator != ( const CMsgQueueIterator& aoRh )
    {
        return !(*this == aoRh);
    }
    
    CMsgQueueIterator& operator ++ ()
    {
        if ( this->mpoNode != NULL )
        {
            this->mpoNode = this->mpoNode->mpoNext;
        }
        return *this;
    }
    
    CMsgQueueIterator operator ++ (int)
    {
        CMsgQueueIterator loIter( *this );
        ++(*this);
        return loIter;
    }
    
protected:
    NodeType* mpoNode;
};

/////////////////////////////////////////////////// MsgQueue /////////////////////////////////////////
/**
 * @class TMsgQueue.
 * TMsgQueue is producer/consumer queue
 * 
 */
template<typename T>
class TMsgQueue
{
public:
    typedef CMsgQueueNode<T> NodeType;
    typedef CMsgQueueIterator<T> IteratorType;
    
public:
    /**
     * @brief Constructor.
     * @param [in] aiCapacity : Msg Queue capacity, if =0 is unlimited
     */
    TMsgQueue( apl_size_t aiCapacity = 0 );
    
    /**
     * @brief Destructor.
     */
    ~TMsgQueue(void);

    /**
     * @brief Push element.
     * @param [in] atNode   : Push node value
     * @param [in] aoTimeout: Push wait timeout
     *                        =<CTimeValue::MAXTIME> Never timeout
     *                        =<CTimeValue::ZERO> Return immediately
     *                        >0 Wait queue non-full timevalue
     * @retval 1 if successful
     * @retval -1 if not.
     */
    apl_int_t Push( T const& atNode, CTimeValue const& aoTimeout = CTimeValue::ZERO );
    
    /**
     * @brief Pop element.
     * @param [in] atNode   : Storage poped node value
     * @param [in] aoTimeout: Push wait timeout
     *                        =<CTimeValue::MAXTIME> Never timeout
     *                        =<CTimeValue::ZERO> Return immediately
     *                        >0 Wait queue non-empty timevalue
     * @retval 1 if successful
     * @retval -1 if not.     
     */
    apl_int_t Pop( T& atNode, CTimeValue const& aoTimeout = CTimeValue::ZERO );
    
    /**
     * @brief Return queue begin iterator.
     */
    IteratorType Begin() const                           { return IteratorType(this->mpoFirst); }
    
    /**
     * @brief Return queue end iterator.
     */
    IteratorType End() const                             { return IteratorType(NULL); }
    
    /**
     * @brief Return queue capacity.
     */
    apl_size_t GetCapacity(void) const                   { return this->muCapacity; }
    
    /**
     * @brief Return queue size.
     */
    apl_size_t GetSize(void) const                       { return this->muSize; }
    
protected:
    CLock moLock;
    
    CSemaphore moPushSema;
    
    CSemaphore moPopSema;
    
    NodeType* mpoFirst;
    
    NodeType* mpoLast;
    
    apl_size_t muCapacity;
    
    apl_size_t muSize;
};

//////////////////////////////////////////////////////////////////////////////////////
// Msg Queue
template<typename T>
TMsgQueue<T>::TMsgQueue( apl_size_t aiCapacity )
    : moPushSema(aiCapacity)
    , mpoFirst(NULL)
    , mpoLast(NULL)
    , muCapacity(aiCapacity)
    , muSize(0)
{
}

template<typename T>
TMsgQueue<T>::~TMsgQueue()
{
    TSmartLock<CLock> loSmartLock(this->moLock);
    
    while( this->mpoFirst != NULL )
    {
        NodeType* lpoTmp = this->mpoFirst;
        this->mpoFirst = this->mpoFirst->mpoNext;
        
        ACL_DELETE(lpoTmp);
    }
}

template<typename T>
apl_int_t TMsgQueue<T>::Push( T const& atNode, CTimeValue const& aoTimeout )
{
    NodeType* lpoNode = NULL;
    
    if ( this->muCapacity > 0 && this->moPushSema.Wait(aoTimeout) != 0 )
    {
        return -1;
    }
    
    ACL_NEW_ASSERT( lpoNode, NodeType(atNode) );
    
    {
        TSmartLock<CLock> loSmartLock(this->moLock);
        if ( this->mpoFirst == NULL && this->mpoLast == NULL )
        {
            this->mpoFirst = lpoNode;
            this->mpoLast  = lpoNode;
        }
        else
        {
            this->mpoLast->mpoNext = lpoNode;
            this->mpoLast = lpoNode;
        }
        this->muSize++;
    }
    
    this->moPopSema.Post();
    
    return 0;
}

template<typename T>
apl_int_t TMsgQueue<T>::Pop( T& atNode, CTimeValue const& aoTimeout )
{
    NodeType* lpoNode = NULL;

    if (this->moPopSema.Wait(aoTimeout) != 0)
    {
        return -1;
    }

    {
        TSmartLock<CLock> loSmartLock(this->moLock);
        
        ACL_ASSERT(this->mpoFirst != NULL);
        
        if ( this->mpoFirst == this->mpoLast )
        {
            lpoNode = this->mpoFirst;
            this->mpoFirst = NULL;
            this->mpoLast  = NULL;
        }
        else
        {
            lpoNode = this->mpoFirst;
            this->mpoFirst = this->mpoFirst->mpoNext;
        }
        
        this->muSize--;
    }

    atNode = lpoNode->mtValue;
    
    ACL_DELETE( lpoNode );
    
    if ( this->muCapacity > 0 )
    {
        this->moPushSema.Post();
    }
    
    return 0;
}

ACL_NAMESPACE_END

#endif //ACL_MSGQUEUE_H
