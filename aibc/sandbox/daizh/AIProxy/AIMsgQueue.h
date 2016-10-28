
#ifndef __AILIB__MSGQUEUE_H__
#define __AILIB__MSGQUEUE_H__

#include "AILib.h"
#include "AIDualLink.h"
#include "AISynch.h"

template< class T >
struct __stNode
{
    __stNode( const T& t ) : ctValue(t) {}
    
    T ctValue;
    LISTHEAD coListChild;
};

template< class T > class AIMsgQueue;

template< class T >
class clsMsgQueueIterator
{
public:
    typedef __stNode<T>  TNode;
    
public:
    clsMsgQueueIterator() :
        cpoListNode(NULL)
    {
    }
    
    clsMsgQueueIterator( LISTHEAD* apoListNode ) :
        cpoListNode(apoListNode)
    {
    }
    
    clsMsgQueueIterator( const clsMsgQueueIterator& aoRhs )
    {
        *this = aoRhs;
    }
    
    clsMsgQueueIterator& operator = ( const clsMsgQueueIterator& aoRhs )
    {
        this->cpoListNode = aoRhs.cpoListNode;
        return *this;
    }
    
    T& operator * ()
    {
        TNode* lpoNode = AI_GET_STRUCT_PTR( this->cpoListNode, TNode, coListChild );
        return lpoNode->ctValue;
    }
    
    T* operator -> ()
    {
        return &(*this);
    }
    
    bool operator == ( const clsMsgQueueIterator& aoRh )
    {
        return this->cpoListNode == aoRh.cpoListNode;
    }
    
    bool operator != ( const clsMsgQueueIterator& aoRh )
    {
        return !(*this == aoRh);
    }
    
    clsMsgQueueIterator& operator ++ ()
    {
        this->cpoListNode = this->cpoListNode->cpNext;
        return *this;
    }
    
    clsMsgQueueIterator operator ++ ( int )
    {
        clsMsgQueueIterator loIter( *this );
        ++(*this);
        return loIter;
    }
    
    clsMsgQueueIterator& operator -- ()
    {
        this->cpoListNode = this->cpoListNode->cpPrev;
        return *this;
    }
    
    clsMsgQueueIterator operator -- ( int )
    {
        clsMsgQueueIterator loIter( *this );
        --(*this);
        return loIter;
    }
    
protected:
    LISTHEAD* cpoListNode;
};

/////////////////////////////////////////////////////////
// MsgQueue
template< class T >
class AIMsgQueue
{
public:
    friend class clsMsgQueueIterator<T>;
    
    typedef __stNode<T>           TNode;
    typedef clsMsgQueueIterator<T> TIterator;
    typedef clsMsgQueueIterator<T> TIteratorConst;
    
public:
    AIMsgQueue( size_t aiCapacity = 10000 );
    ~AIMsgQueue();

    ///push element
    int Push( const T& atNode );
    
    int Pop( T& atNode );
    
    TIterator Begin()                                    { return TIterator(this->coListHead.cpNext); }
    
    TIterator End()                                      { return TIterator(&this->coListHead); }
    
    TIteratorConst Begin() const                         { return TIteratorConst(this->coListHead->cpNext); }
    
    TIteratorConst End() const                           { return TIteratorConst(&this->coListHead); }
    
    void Capacity( size_t aiCapacity )                   { ciCapacity = aiCapacity; }
    
    size_t Capacity() const                              { return ciCapacity; }
    
    size_t Size() const                                  { return ciSize; }
    
protected:
    AIMutexLock coLock;
    AISemaphore coSem;
    
    LISTHEAD    coListHead;
    size_t      ciCapacity;
    size_t      ciSize;
};

//////////////////////////////////////////////////////////////////////////////////////
// Msg Queue
template< class T >
AIMsgQueue<T>::AIMsgQueue( size_t aiCapacity /* = 10000 */ ) : 
    ciCapacity(aiCapacity),
    ciSize(0)
{
    ai_init_list_head( &coListHead );
}

template< class T >
AIMsgQueue<T>::~AIMsgQueue()
{
    TNode*    lpoNode      = NULL;
    LISTHEAD* lpoListChild = NULL;
    
    AISmartLock loSmartLock(coLock);
    
    while( !ai_list_is_empty( &coListHead ) )
    {
        ai_list_del_head( lpoListChild, &coListHead );
        lpoNode = AI_GET_STRUCT_PTR( lpoListChild, TNode, coListChild );
        AI_DELETE( lpoNode );
    }
}

template< class T >
int AIMsgQueue<T>::Push( const T& atNode )
{
    TNode* lpoNode = NULL;
    
    if ( ciSize >= ciCapacity ) return  -1;
    
    AI_NEW_INIT_ASSERT( lpoNode, TNode, atNode );
        
    {
        AISmartLock loSmartLock(coLock);
        ai_list_add_tail( &lpoNode->coListChild, &coListHead );
        ciSize++;
    }
    
    coSem.Post();
    
    return 0;
}

template< class T >
int AIMsgQueue<T>::Pop( T& atNode )
{
    TNode*    lpoNode      = NULL;
    LISTHEAD* lpoListChild = NULL;

    //if ( coSem.WaitFor( aiTimeout ) != 0 )
    if ( coSem.Wait() != 0 )
    {
        return -1;
    }

    {
        AISmartLock loSmartLock(coLock);
        
        if ( ai_list_is_empty( &coListHead ) ) return -1;
        
        ai_list_del_head( lpoListChild, &coListHead );
        ciSize--;
    }
    
    lpoNode = AI_GET_STRUCT_PTR( lpoListChild, TNode, coListChild );
    
    atNode  = lpoNode->ctValue;
    AI_DELETE( lpoNode );
    
    return 0;
}

#endif //__AI_MSGQUEUE_H__
