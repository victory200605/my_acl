
#include "AITask.h"

//////////////////////////////////////////////////////////////////////////////////////
// AITask
AITask::AITask() :
    ciClosed(ST_CLOSED),
    ciThreadCnt(0),
    coThreadPool(NULL),
    coQueue(0)
{
}

AITask::~AITask()
{
    this->Close();
}
    
int AITask::Initialize( size_t aiThreadCnt, size_t aiCacheSize )
{
    if ( this->ciClosed == ST_ACTIVE )
    {
        return ERROR_ACTIVE;
    }
    
    this->coThreadPool = AIBC::AIThrPoolCreate( aiThreadCnt );
    this->ciClosed     = ST_ACTIVE;
    this->coQueue.Capacity( aiCacheSize );
    
    for ( size_t liN = 0; liN < aiThreadCnt; liN++ )
    {
        if ( AIThrPoolCall( this->coThreadPool, ThreadHandle, this ) != 0 )
        {
            AIThrPoolDestroy( this->coThreadPool );
            this->coThreadPool = NULL;
            
            this->ciClosed = ST_CLOSED;
            
            return ERROR_THREAD;
        }
    }

    return 0;
}

void AITask::CloseWait()
{
    this->Close( ST_CLOSED_WAIT );
}

void AITask::Close()
{
    this->Close( ST_CLOSED );
}

void AITask::Close( RUN_STAT aiStat )
{
    if ( this->ciClosed != ST_ACTIVE ) return;
        
    this->ciClosed = aiStat;

    // End of Queue
    for ( size_t liN = 0; liN < (size_t)this->ciThreadCnt; liN++ )
    {
        this->PutExitNode();
    }
    
    // Exit all handle thread
    while( true )
    {
        if ( (size_t)this->ciThreadCnt > 0 )
        {
            // End of Queue
            this->PutExitNode();
            AIBC::AISleepFor( AI_TIME_MSEC * 100 );
            
            continue;
        }
        
        break;
    }
    
    // Destroy thread pool
    if ( this->coThreadPool != NULL )
    {
        AIThrPoolDestroy( this->coThreadPool );
        this->coThreadPool = NULL;
    }
    
    // Recycle node resoure
    for ( AIMsgQueue<AITaskNode*>::TIterator loIter = this->coQueue.Begin(); 
          loIter != this->coQueue.End(); loIter++ )
    {
        AI_DELETE( *loIter );
    }
    
    this->ciThreadCnt = 0;
}

void AITask::PutExitNode()
{
    AIDefaultNode<void>* lpoNode   = NULL;
    
    AI_NEW_INIT_ASSERT( lpoNode, AIDefaultNode<void>, NULL, NULL );
    
    if ( this->coQueue.Push( lpoNode ) != 0 )
    {
        AI_DELETE( lpoNode );
    }
}

size_t AITask::GetMaxThreadCount()
{
    return (size_t)this->ciThreadCnt;
}

size_t AITask::GetActiveThreadCount()
{
    return (size_t)this->ciActiveThreadCnt;
}

size_t AITask::GetCacheCapacity()
{
    return this->coQueue.Capacity();
}

size_t AITask::GetCacheSize()
{
    return this->coQueue.Size();
}

void* AITask::ThreadHandle( void* apParam )
{
    AITask*     lpoTask     = (AITask*)apParam;
    AITaskNode* lpoTaskNode = NULL;
    
    lpoTask->ciThreadCnt++;
    
    while( lpoTask->ciClosed != ST_CLOSED )
    {
        if ( lpoTask->coQueue.Pop( lpoTaskNode ) != 0 )
        {
            AIBC::AISleepFor( AI_TIME_MSEC * 100 );
            continue;
        }
        
        if ( lpoTaskNode->IsEmpty() )
        {
            //Exit
            break;
        }
        
        lpoTask->ciActiveThreadCnt++;
        lpoTaskNode->Run();
        lpoTask->ciActiveThreadCnt--;
        
        AI_DELETE( lpoTaskNode );
    }
    
    lpoTask->ciThreadCnt--;
    
    return NULL;
}
