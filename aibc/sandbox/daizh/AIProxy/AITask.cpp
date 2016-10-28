
#include "AITask.h"

//////////////////////////////////////////////////////////////////////////////////////
// AITask
AITask::AITask() :
    cbIsClosed(false),
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
    this->coThreadPool = AIThrPoolCreate( aiThreadCnt );
    
    for ( size_t liN = 0; liN < aiThreadCnt; liN++ )
    {
        if ( AIThrPoolCall( this->coThreadPool, ThreadHandle, this ) != 0 )
        {
            AIThrPoolDestroy( this->coThreadPool );
            this->coThreadPool = NULL;
            
            return -1;
        }
    }
    
    this->coQueue.Capacity( aiCacheSize );
    
    this->ciThreadCnt = aiThreadCnt;
    this->cbIsClosed  = false;
    
    return 0;
}

void AITask::Close()
{
    if ( this->cbIsClosed ) return;
    
    this->cbIsClosed = true;
    
    // End of Queue
    for ( size_t liN = 0; liN < (size_t)this->ciThreadCnt; liN++ )
    {
        this->Put( NULL, NULL );
    }
    
    // Exit all handle thread
    while( true )
    {
        if ( (size_t)this->ciThreadCnt > 0 )
        {
            // End of Queue
            this->Put( NULL, NULL );
            AISleepFor( AI_TIME_MSEC * 100 );
            
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
    
    while( !lpoTask->cbIsClosed )
    {
        if ( lpoTask->coQueue.Pop( lpoTaskNode ) != 0 )
        {
            AISleepFor( AI_TIME_MSEC * 100 );
            continue;
        }
        lpoTask->ciActiveThreadCnt++;
        lpoTaskNode->Run();
        lpoTask->ciActiveThreadCnt--;
        
        AI_DELETE( lpoTaskNode );
    }
    
    lpoTask->ciThreadCnt--;
    
    return NULL;
}
