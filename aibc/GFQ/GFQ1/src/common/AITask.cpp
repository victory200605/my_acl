
#include "AITask.h"
#include "gfq/GFQUtility.h"

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
    coThreadPool = AIThrPoolCreate( aiThreadCnt );
    
    for ( size_t liN = 0; liN < aiThreadCnt; liN++ )
    {
        if ( AIThrPoolCall( coThreadPool, ThreadHandle, this ) != 0 )
        {
            AIThrPoolDestroy( coThreadPool );
            coThreadPool = NULL;
            
            return -1;
        }
    }
    
    coQueue.Capacity( aiCacheSize );
    
    ciThreadCnt      = aiThreadCnt;
    this->cbIsClosed = false;
    
    return 0;
}

size_t AITask::GetMaxThreadCount()
{
    return (size_t)ciThreadCnt;
}

size_t AITask::GetActiveThreadCount()
{
    return (size_t)ciActiveThreadCnt;
}

size_t AITask::GetCacheCapacity()
{
    return coQueue.Capacity();
}

size_t AITask::GetCacheSize()
{
    return coQueue.Size();
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
    if ( coThreadPool != NULL )
    {
        AIThrPoolDestroy( coThreadPool );
        coThreadPool = NULL;
    }
    
    ciThreadCnt = 0;
}

void* AITask::ThreadHandle( void* apParam )
{
    AITask*     lpoTask     = (AITask*)apParam;
    AITaskNode* lpoTaskNode = NULL;
    
    while( !lpoTask->cbIsClosed )
    {
        lpoTaskNode = lpoTask->coQueue.Pop( AI_TIME_SEC );
        if ( lpoTaskNode == NULL )
        {
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
