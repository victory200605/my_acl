
#ifndef __AI_PROXY_CHANNEL_POOL_H__
#define __AI_PROXY_CHANNEL_POOL_H__

#include "AISyncTcp.h"
#include "AISynch.h"
#include "AITypeObject.h"
#include "AIDualLink.h"
#include "stl/string.h"
#include "stl/hash_map.h"

////////////////////////////////////////// channel pool ////////////////////////////////
class AIChannelPool
{
public:
    struct stNode
    {
        AIChannelPtr coChannelPtr;
        LISTHEAD     coChild;
    };
    typedef stNode TNode;
    
    struct stList
    {
        int         ciElemNum;
        AIMutexLock coLock;
        
        LISTHEAD    coHead;
    };
    
    
    typedef AI_STD::hash_map< AI_STD::string, stList* > TChannelListMap;
    
public:
    AIChannelPool();
    ~AIChannelPool();

    AIChannelPtr GetChannel( const char* apcNamePath );
    
    void PutChannel( const char* apcGroupName, AIBaseChannel* apoChannel );
    
    void EraseChannel( const char* apcGroupName, AIChannelPtr aoChannelPtr );
    
    size_t GetChannelCnt( const char* apcGroupName );
    
protected:
    TChannelListMap coChannelListMap;
    AIMutexLock     coLock;
};

#endif //#define __AI_PROXY_CHANNEL_POOL_H__
