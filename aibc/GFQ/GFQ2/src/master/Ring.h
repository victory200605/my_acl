#ifndef AIBC_GFQ2_RING_H
#define AIBC_GFQ2_RING_H

#include "acl/Ring.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Ring node entry define, element node shall inherit from this class instead of complex C-API
 * N is entry count
 */
template<apl_size_t N> struct TRingEntry
{
    typedef TRingEntry<N> SelfType;
    ACL_RING_HEAD(HeadType, SelfType);

    TRingEntry(void)
    {
        for (apl_size_t luI = 0; luI < N; luI++)
        {
            this->mpoHead[luI] = APL_NULL;
            ACL_RING_ELEM_INIT(this, moEntry[luI]);
        }
    }

    ~TRingEntry(void)
    {
        for (apl_size_t luI = 0; luI < N; luI++)
        {
            this->mpoHead[luI] = APL_NULL;
            ACL_RING_REMOVE(this, moEntry[luI]);
        }
    }

    ACL_RING_ENTRY(SelfType) moEntry[N];
    
    HeadType* mpoHead[N];
};

/** 
 * @brief Ring node head define, N is entry total count and I entry is linked to the other elements in the ring 
 */
template<apl_size_t N, apl_size_t I> class TRing
{
public:
    typedef TRingEntry<N> EntryType;
    typedef typename EntryType::HeadType HeadType;

public:
    TRing(void)
    {
        ACL_RING_INIT(&this->moHead, EntryType, moEntry[I]);
    }

    TRing( TRing const& aoOther )
    {
        ACL_RING_INIT(&this->moHead, EntryType, moEntry[I]);
    }

    void InsertHead( EntryType* apoEntry )
    {
        ACL_RING_INSERT_HEAD(&this->moHead, apoEntry, EntryType, moEntry[I]);
        
        apoEntry->mpoHead[I] = &this->moHead;
    }

    void InsertTail( EntryType* apoEntry )
    {
        ACL_RING_INSERT_TAIL(&this->moHead, apoEntry, EntryType, moEntry[I]);
        
        apoEntry->mpoHead[I] = &this->moHead;
    }

    void InsertBefore( EntryType* apoLeft, EntryType* apoEntry )
    {
        ACL_RING_INSERT_BEFORE(apoLeft, apoEntry, moEntry[I]);
        
        apoEntry->mpoHead[I] = &this->moHead;
    }

    void InsertAfter( EntryType* apoLeft, EntryType* apoEntry )
    {
        ACL_RING_INSERT_AFTER(apoLeft, apoEntry, moEntry[I]);
        
        apoEntry->mpoHead[I] = &this->moHead;
    }

    void Remove( EntryType* apoEntry )
    {
        if (apoEntry->mpoHead[I] == &this->moHead)
        {
            ACL_RING_REMOVE(apoEntry, moEntry[I]);
            apoEntry->mpoHead[I] = APL_NULL;
        }
    }
    
    template<class T> void All( T& aoAction )
    {
        EntryType* lpoCurrNode = APL_NULL;
        EntryType* lpoNextNode = APL_NULL;

        ACL_RING_FOREACH_SAFE(lpoCurrNode, lpoNextNode, &this->moHead, EntryType, moEntry[I])
        {
            if (!aoAction(lpoCurrNode) )
            {
                break;
            }
        }
    }

    bool IsExisted( EntryType* apoEntry )
    {
        return apoEntry->mpoHead[I] == &this->moHead;
    }

    HeadType* GetHead(void)
    {
        return &this->moHead;
    }

private:
    HeadType moHead;
};

/** 
 * @brief Ring iterator
 */
template<class T, apl_size_t N, apl_size_t I> class TRingIterator
{
    typedef TRing<N,I> RingType;
    typedef TRingEntry<N> EntryType;

public:
    TRingIterator( RingType& aoRing )
        : mpoHead(ACL_RING_SENTINEL(aoRing.GetHead(), EntryType, moEntry[I]) )
        , mpoNode(ACL_RING_FIRST(aoRing.GetHead() ) )
    {
    }

    bool IsDone(void)
    {
        return this->mpoNode == this->mpoHead;
    }

    void Next(void)
    {
        this->mpoNode = ACL_RING_NEXT(this->mpoNode, moEntry[I]);
    }

    T* operator -> (void)
    {
        return static_cast<T*>(this->mpoNode);
    }

    T& operator * (void)
    {
        return *(static_cast<T*>(this->mpoNode) );
    }

private:
    EntryType* mpoHead;

    EntryType* mpoNode;
};

/** 
 * @brief Ring r-iterator
 */
template<class T, apl_size_t N, apl_size_t I> class TRingRIterator
{
    typedef TRing<N,I> RingType;
    typedef TRingEntry<N> EntryType;

public:
    TRingRIterator( RingType& aoRing )
        : mpoHead(ACL_RING_SENTINEL(aoRing.GetHead(), EntryType, moEntry[I]) )
        , mpoNode(ACL_RING_LAST(aoRing.GetHead() ) )
    {
    }

    bool IsDone(void)
    {
        return this->mpoNode == this->mpoHead;
    }

    void Next(void)
    {
        this->mpoNode = ACL_RING_PREV(this->mpoNode, moEntry[I]);
    }

    T* operator -> (void)
    {
        return static_cast<T*>(this->mpoNode);
    }

    T& operator * (void)
    {
        return *(static_cast<T*>(this->mpoNode) );
    }

private:
    EntryType* mpoHead;

    EntryType* mpoNode;
};

AIBC_GFQ_NAMESPACE_END

#endif

