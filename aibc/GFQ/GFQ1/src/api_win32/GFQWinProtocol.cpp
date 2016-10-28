
#include "GFQWinProtocol.h"
 
//start namespace
AI_GFQ_NAMESPACE_START

clsWinRequestQueueStat::clsWinRequestQueueStat() : 
    coGFQPtlHeader(AI_GFQ_REQUEST_QUEUE_STAT, NULL)
{
    memset( &coAsyncHeader, 0, sizeof(coAsyncHeader) );
}

int clsWinRequestQueueStat::Encode( AIChunkEx& aoChunk )
{
    aoChunk.Resize( sizeof(coAsyncHeader) + coGFQPtlHeader.GetSize() + clsRequestQueueStat::GetSize() );
    
    coAsyncHeader.ciLength = aoChunk.GetSize();
    
    AIChunkExOut loOut(aoChunk);
    
    loOut.PutNInt32( coAsyncHeader.ciMsgID );
    loOut.PutNInt32( coAsyncHeader.ciResult );
    loOut.PutNInt32( coAsyncHeader.ciLength );
    
    coGFQPtlHeader.Encode( aoChunk );
	clsRequestQueueStat::Encode( aoChunk );
    
    aoChunk.Resize(coAsyncHeader.ciLength);
    
    return 0;
}

clsWinResponseQueueStat::clsWinResponseQueueStat()
{
    memset( &coAsyncHeader, 0, sizeof(coAsyncHeader) );
}

int clsWinResponseQueueStat::Decode( AIChunkEx& aoChunk )
{
    AIChunkExIn loIn(aoChunk);
    
    AI_RETURN_IF( -1, loIn.ToHInt32( coAsyncHeader.ciMsgID ) != 0 );
    AI_RETURN_IF( -1, loIn.ToHInt32( coAsyncHeader.ciResult ) != 0 );
    AI_RETURN_IF( -1, loIn.ToHInt32( coAsyncHeader.ciLength ) != 0 );
    
    AI_RETURN_IF( -1, coAsyncHeader.ciLength <= 0 );
    
    AI_RETURN_IF( -1, coGFQPtlHeader.Decode( aoChunk ) != 0 );
	AI_RETURN_IF( -1, coGFQPtlHeader.GetCmd() != AI_GFQ_RESPONSE_QUEUE_STAT )
	AI_RETURN_IF( -1, clsResponseQueueStat::Decode( aoChunk ) !=0 );
        
    return 0;
}

//end namespace
AI_GFQ_NAMESPACE_END
