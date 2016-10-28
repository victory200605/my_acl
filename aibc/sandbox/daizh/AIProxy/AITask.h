
#ifndef __AILIB__TASK_H__
#define __AILIB__TASK_H__

#include "AILib.h"
#include "AIDualLink.h"
#include "AISynch.h"
#include "AIThrPool.h"
#include "AIMsgQueue.h"
#include "AITypeObject.h"

/////////////////////////////////////////////////////////
// AITaskNode
class AITaskNode
{
public:
    virtual ~AITaskNode() {}
    virtual int Run() = 0;
};

/////////////////////////////////////////////////////////
// Thread function
typedef void* (*TTHREADFUNC)( void* apoParam );

/////////////////////////////////////////////////////////
// Default AITaskNode
template< class TPARAM >
class AIDefaultNode : public AITaskNode
{
public:
    AIDefaultNode( TTHREADFUNC apfHandleFunc, TPARAM* aptParam )
        : cpfHandleFunc(apfHandleFunc), cptParam(aptParam)
    {
    }
    
    ~AIDefaultNode()
    {
        if ( cptParam != NULL )
        {
            AI_DELETE( cptParam );
        }
    }
    
    int  Run()
    {
        if ( cpfHandleFunc != NULL )
        {
            (*cpfHandleFunc)( cptParam );
            cptParam = NULL;
        }
        return 0;
    }
    
protected:
    TTHREADFUNC cpfHandleFunc;
    TPARAM*     cptParam;
};

template< >
class AIDefaultNode<void> : public AITaskNode
{
public:
    AIDefaultNode( TTHREADFUNC apfHandleFunc, void* apParam )
        : cpfHandleFunc(apfHandleFunc), cpParam(apParam)
    {
    }
    
    ~AIDefaultNode()
    {
        if ( cpParam != NULL )
        {
            AI_FREE( cpParam );
        }
    }
    
    int  Run()
    {
        if ( cpfHandleFunc != NULL )
        {
            (*cpfHandleFunc)( cpParam );
            cpParam = NULL;
        }
        return 0;
    }
    
protected:
    TTHREADFUNC cpfHandleFunc;
    void*       cpParam;
};

/////////////////////////////////////////////////////////
// AITask
class AITask
{
public:
    AITask();
    ~AITask();
    
    int  Initialize( size_t aiThreadCnt, size_t aiCacheSize );
    void Close();
    
    template< class TPARAM >
    int  Put( TTHREADFUNC apfHandleFunc, TPARAM* apParam )
    {
        int                    liRetCode = 0;
        AIDefaultNode<TPARAM>* lpoNode   = NULL;
        
        AI_NEW_INIT_ASSERT( lpoNode, AIDefaultNode<TPARAM>, apfHandleFunc, apParam );
        
        if ( this->coQueue.Push( lpoNode ) != 0 )
        {
            AI_DELETE( lpoNode );
            liRetCode = -1;
        }
        
        return liRetCode;
    }
    int  Put( TTHREADFUNC apfHandleFunc, void* apParam )
    {
        int                  liRetCode = 0;
        AIDefaultNode<void>* lpoNode   = NULL;
        
        AI_NEW_INIT_ASSERT( lpoNode, AIDefaultNode<void>, apfHandleFunc, apParam );
        
        if ( this->coQueue.Push( lpoNode ) != 0 )
        {
            AI_DELETE( lpoNode );
            liRetCode = -1;
        }
        
        return liRetCode;
    }
    
    size_t GetMaxThreadCount();
    size_t GetActiveThreadCount();
    size_t GetCacheCapacity();
    size_t GetCacheSize();

protected:
    static void* ThreadHandle( void* apParam );
    
protected:
    // Close flag
    bool cbIsClosed;
    
    // Thread count and mutex lock
    AITypeObject<size_t, AIMutexLock> ciThreadCnt;
    AITypeObject<size_t, AIMutexLock> ciActiveThreadCnt;
    
    // Thread pool
    AIThrPool_t coThreadPool;
    
    // Task queue
    AIMsgQueue<AITaskNode*> coQueue;
};

#endif // __AILIB__TASK_H__
