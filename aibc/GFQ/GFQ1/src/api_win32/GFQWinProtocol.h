
#ifndef __AI_GFQWIN_PROTOCOL__
#define __AI_GFQWIN_PROTOCOL__
 
#include "Utility.h"

//start namespace
AI_GFQ_NAMESPACE_START

////////////////////////////////////////////////////////////////////
// AIAsyncTcp Header
typedef struct stASYNCHEAD
{
    int     ciMsgID;
    int     ciResult;
    int     ciLength;
} ASYNCHEAD;

class clsWinRequestQueueStat : public clsRequestQueueStat
{
public:
    clsWinRequestQueueStat();
    
    int Encode( AIChunkEx& aoChunk );
    
protected:
    stASYNCHEAD coAsyncHeader;
    
    clsHeader   coGFQPtlHeader;
};

class clsWinResponseQueueStat : public clsResponseQueueStat
{
public:
    clsWinResponseQueueStat();

    int Decode( AIChunkEx& aoChunk );
    
protected:
    stASYNCHEAD coAsyncHeader;
    
    clsHeader   coGFQPtlHeader;
};

//end namespace
AI_GFQ_NAMESPACE_END

#endif
