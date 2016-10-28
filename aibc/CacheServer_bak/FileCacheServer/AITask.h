
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
    virtual bool IsEmpty() = 0;
    virtual void ReleaseParam() = 0;
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
    
    virtual ~AIDefaultNode()
    {
        if ( cptParam != NULL )
        {
            AI_DELETE( cptParam );
        }
    }
    
    virtual int  Run()
    {
        if ( cpfHandleFunc != NULL )
        {
            (*cpfHandleFunc)( cptParam );
            cptParam = NULL;
        }
        return 0;
    }
    
    virtual bool IsEmpty()
    {
        return this->cpfHandleFunc == NULL;
    }
    
    virtual void ReleaseParam()
    {
        cptParam = NULL;
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
    
    virtual ~AIDefaultNode()
    {
        if ( cpParam != NULL )
        {
            AI_FREE( cpParam );
        }
    }
    
    virtual int  Run()
    {
        if ( cpfHandleFunc != NULL )
        {
            (*cpfHandleFunc)( cpParam );
            cpParam = NULL;
        }
        return 0;
    }
    
    virtual bool IsEmpty()
    {
        return this->cpfHandleFunc == NULL;
    }
    
    virtual void ReleaseParam()
    {
        cpParam = NULL;
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
    enum {
        NO_ERROR        =  0,
        ERROR_THREAD    = -1,
        ERROR_QUEUEFULL = -2,
        ERROR_ACTIVE    = -3,
        ERROR_CLOSED    = -4
    };
    
    enum RUN_STAT {
        ST_ACTIVE      = 0,
        ST_CLOSED      = 1,
        ST_CLOSED_WAIT = 2
    };
    
public:
    AITask();
    ~AITask();
    
    int  Initialize( size_t aiThreadCnt, size_t aiCacheSize );
    void CloseWait();
    void Close();
    
    template< class TPARAM >
    int  Put( TTHREADFUNC apfHandleFunc, TPARAM* apParam )
    {
        assert( apfHandleFunc != NULL );
        AIDefaultNode<TPARAM>* lpoNode   = NULL;
        
        if ( this->ciClosed != ST_ACTIVE )
        {
            return ERROR_CLOSED;
        }
        
        AI_NEW_INIT_ASSERT( lpoNode, AIDefaultNode<TPARAM>, apfHandleFunc, apParam );
        
        if ( this->coQueue.Push( lpoNode ) != 0 )
        {
            lpoNode->ReleaseParam();
            AI_DELETE( lpoNode );
            return ERROR_QUEUEFULL;
        }
        else
        {
            return NO_ERROR;
        }
    }
    int  Put( TTHREADFUNC apfHandleFunc, void* apParam )
    {
        assert( apfHandleFunc != NULL );
        AIDefaultNode<void>* lpoNode   = NULL;
        
        if ( this->ciClosed != ST_ACTIVE )
        {
            return ERROR_CLOSED;
        }
        
        AI_NEW_INIT_ASSERT( lpoNode, AIDefaultNode<void>, apfHandleFunc, apParam );
        
        if ( this->coQueue.Push( lpoNode ) != 0 )
        {
            lpoNode->ReleaseParam();
            AI_DELETE( lpoNode );
            return ERROR_QUEUEFULL;
        }
        else
        {
            return NO_ERROR;
        }
    }
    
    size_t GetMaxThreadCount();
    size_t GetActiveThreadCount();
    size_t GetCacheCapacity();
    size_t GetCacheSize();

protected:
    static void* ThreadHandle( void* apParam );
    
    void Close( RUN_STAT aiStat );
    void PutExitNode();
    
protected:
    // Close flag
    RUN_STAT ciClosed;
    
    // Thread count and mutex lock
    AIBC::AITypeObject<size_t, AIBC::AIMutexLock> ciThreadCnt;
    AIBC::AITypeObject<size_t, AIBC::AIMutexLock> ciActiveThreadCnt;
    
    // Thread pool
    AIBC::AIThrPool_t coThreadPool;
    
    // Task queue
    AIMsgQueue<AITaskNode*> coQueue;
};

#endif // __AILIB__TASK_H__
