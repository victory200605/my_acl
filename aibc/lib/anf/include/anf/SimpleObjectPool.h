
#ifndef ANF_SIMPLEOBJECTPOOL_H
#define ANF_SIMPLEOBJECTPOOL_H

#include "anf/Utility.h"
#include "acl/Singleton.h"
#include "acl/Ring.h"

ANF_NAMESPACE_START

template<typename T> class TSimpleObjectPool
{
    struct CNode : public T
    {
        ACL_RING_ENTRY(CNode) moEntry;
    };

public:
    TSimpleObjectPool(void)
        : muSize(0)
        , muCapacity(64)
    {
        ACL_RING_INIT(&this->moHead, CNode, moEntry);
    }

    ~TSimpleObjectPool(void)
    {
        CNode* lpoCurr = NULL;
        CNode* lpoNext = NULL;
        
        ACL_RING_FOREACH_SAFE(lpoCurr, lpoNext, &this->moHead, CNode, moEntry)
        {
            ACL_DELETE(lpoCurr);
        }
    }

    void SetCapacity( apl_size_t auCapacity )
    {
        this->muCapacity = auCapacity;
    }

    T* Allocate(void)
    {
        this->moLock.Lock();

        if (ACL_RING_IS_EMPTY(&this->moHead, CNode, moEntry) )
        {
            this->moLock.Unlock();

            CNode* lpoNew = NULL;
            ACL_NEW(lpoNew, CNode);
            ACL_RING_ELEM_INIT(lpoNew, moEntry);

            return lpoNew;
        }

        CNode* lpoFirst = ACL_RING_FIRST(&this->moHead);

        ACL_RING_REMOVE(lpoFirst, moEntry);

        this->muSize--;

        this->moLock.Unlock();

        return lpoFirst;
    }

    void Destroy( T* apoPtr )
    {
        CNode* lpoTemp = dynamic_cast<CNode*>(apoPtr);
        if (lpoTemp != NULL)
        {
            if (this->muSize < this->muCapacity)
            {
                //recycle
                this->moLock.Lock();
                
                ACL_RING_INSERT_TAIL(&this->moHead, lpoTemp, CNode, moEntry);
                
                this->muSize++;

                this->moLock.Unlock();
            }
            else
            {
                //discard
                ACL_DELETE(lpoTemp);
            }
        }
        else
        {
            ACL_DELETE(apoPtr);
        }
    }

private:
    apl_size_t muSize;

    apl_size_t muCapacity;

    acl::CLock moLock;

    ACL_RING_HEAD(MyHead, CNode) moHead;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T> TSimpleObjectPool<T>* SimpleObjectPoolInstance(void)
{
    return acl::TSingleton<TSimpleObjectPool<T> >::Instance();
}

template<typename T> TSimpleObjectPool<T>* SimpleObjectPoolRelease(void)
{
    return acl::TSingleton<TSimpleObjectPool<T> >::Release();
}

ANF_NAMESPACE_END

#endif //ANF_SIMPLEOBJECTPOOL_H

