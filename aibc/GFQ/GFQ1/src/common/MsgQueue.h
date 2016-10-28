
#ifndef __AI_MSGQUEUE_H__
#define __AI_MSGQUEUE_H__

#include "AILib.h"
#include "AIDualLink.h"
#include "AISynch.h"
#include "AIThrPool.h"

/////////////////////////////////////////////////////////
// Queue
template< class T >
class clsMsgQueue
{
public:
    struct stNode
    {
        stNode();
        
        T* cptNode;
        LISTHEAD coListChild;
    };
    
public:
    clsMsgQueue( size_t aiCapacity = 10000 );
    ~clsMsgQueue();

    int Push( T* aptNode );
    T*  Pop( AITime_t aiTimeout );
    
    void Capacity( size_t aiCapacity )             { ciCapacity = aiCapacity; }
    size_t Capacity()                              { return ciCapacity; }
    size_t Size()                                  { return ciSize; }
    
protected:
    AIMutexLock coLock;
    AISemaphore coSem;
    
    LISTHEAD    coListHead;
    size_t      ciCapacity;
    size_t      ciSize;
};

//////////////////////////////////////////////////////////////////////////////////////
template< class T >
clsMsgQueue<T>::stNode::stNode() : cptNode(NULL)
{
    memset( &coListChild, 0, sizeof(LISTHEAD) );
}

//////////////////////////////////////////////////////////////////////////////////////
// Msg Queue
template< class T >
clsMsgQueue<T>::clsMsgQueue( size_t aiCapacity /* = 10000 */ ) : 
    ciCapacity(aiCapacity),
    ciSize(0)
{
    ai_init_list_head( &coListHead );
}

template< class T >
clsMsgQueue<T>::~clsMsgQueue()
{
    stNode*     lpoNode      = NULL;
    LISTHEAD*   lpoListChild = NULL;
    
    AISmartLock loSmartLock(coLock);
    
    while( !ai_list_is_empty( &coListHead ) )
    {
        ai_list_del_head( lpoListChild, &coListHead );
        lpoNode = AI_GET_STRUCT_PTR( lpoListChild, stNode, coListChild );
        AI_DELETE( lpoNode->cptNode );
        AI_DELETE( lpoNode );
    }
}

template< class T >
int clsMsgQueue<T>::Push( T* aptNode )
{
    int     liRetCode = 0;
    stNode* lpoNode   = NULL;
    
    if ( ciSize >= ciCapacity ) return ( liRetCode = -1 );
    
    AI_NEW_ASSERT( lpoNode, stNode );
    lpoNode->cptNode = aptNode;
        
    {
        AISmartLock loSmartLock(coLock);
        ai_list_add_tail( &lpoNode->coListChild, &coListHead );
        ciSize++;
    }
    
    coSem.Post();
    
    return liRetCode;
}

template< class T >
T* clsMsgQueue<T>::Pop( AITime_t aiTimeout )
{
    stNode*     lpoNode      = NULL;
    LISTHEAD*   lpoListChild = NULL;
    T*          lptNode      = NULL;

    if ( coSem.WaitFor( aiTimeout ) != 0 )
    {
        return NULL;
    }

    {
        AISmartLock loSmartLock(coLock);
        
        if ( ai_list_is_empty( &coListHead ) ) return NULL;
        
        ai_list_del_head( lpoListChild, &coListHead );
        ciSize--;
    }
    
    lpoNode = AI_GET_STRUCT_PTR( lpoListChild, stNode, coListChild );
    
    lptNode = lpoNode->cptNode;
    AI_DELETE( lpoNode );
    
    return lptNode;
}

#endif //__AI_MSGQUEUE_H__
