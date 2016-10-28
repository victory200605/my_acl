
#ifndef __AI_TASK_H__
#define __AI_TASK_H__

#include "AILib.h"
#include "AIDualLink.h"
#include "AISynch.h"
#include "AIThrPool.h"
#include "MsgQueue.h"

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
    
    void ReleaseParam()
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
    
    void ReleaseParam()
    {
        cpParam = NULL;
    }
    
protected:
    TTHREADFUNC cpfHandleFunc;
    void*       cpParam;
};

/////////////////////////////////////////////////////////
// AITask:clsTaskNum
template< class T >
class clsAtomNum
{
public:
    clsAtomNum( T ltN = 0) : coNum(ltN) {}
    
    clsAtomNum& operator = ( T ltN )
    {
        AISmartLock loLock( coLock );
        coNum = ltN;
        
        return *this;
    }
    T operator ++ ()
    {
        AISmartLock loLock( coLock );
        T ltTmp = coNum++;
        
        return ltTmp;
    }
    T operator ++ ( int )
    {
        AISmartLock loLock( coLock );
        T ltTmp = ++coNum;
        
        return ltTmp;
    }
    T operator -- ()
    {
        AISmartLock loLock( coLock );
        T ltTmp = coNum--;
        
        return ltTmp;
    }
    T operator -- ( int )
    {
        AISmartLock loLock( coLock );
        T ltTmp = --coNum;
        
        return ltTmp;
    }
    operator T ()
    {
        AISmartLock loLock( coLock );
     
        return coNum;
    }
    
protected:
    T coNum;
    AIMutexLock coLock;
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
        
        if ( coQueue.Push( lpoNode ) != 0 )
        {
            lpoNode->ReleaseParam();
            
            AI_DELETE( lpoNode );
            liRetCode = -1;
        }
        
        return liRetCode;
    }
    int  Put( TTHREADFUNC apfHandleFunc, void* apParam )
    {
        int                    liRetCode = 0;
        AIDefaultNode<void>* lpoNode   = NULL;
        
        AI_NEW_INIT_ASSERT( lpoNode, AIDefaultNode<void>, apfHandleFunc, apParam );
        
        if ( coQueue.Push( lpoNode ) != 0 )
        {
            lpoNode->ReleaseParam();
            
            AI_DELETE( lpoNode );
            liRetCode = -1;
        }
        
        return liRetCode;
    }
    
    size_t GetMaxThreadCount();
    size_t GetActiveThreadCount();
    size_t GetCacheCapacity();
    size_t GetCacheSize();

    static void* ThreadHandle( void* apParam );
    
protected:
    // Close flag
    bool           cbIsClosed;
    
    // Thread count and mutex lock
    clsAtomNum<size_t> ciThreadCnt;
    clsAtomNum<size_t> ciActiveThreadCnt;
    
    // Thread pool
    AIThrPool_t    coThreadPool;
    
    // Task queue
    clsMsgQueue<AITaskNode> coQueue;
};

#endif
