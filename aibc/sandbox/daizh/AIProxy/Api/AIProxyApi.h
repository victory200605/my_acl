
#ifndef __AI_PROXY_API_H__
#define __AI_PROXY_API_H__

#include "AITime.h"
#include "AISynch.h"
#include "AITypeObject.h"
#include "../AITask.h"
#include "../AIProxyUtility.h"
#include "../AISyncTcp.h"
#include "../AIProxyProtocol.h"
#include "../AIHashMap.h"
#include "../AIChannelPool.h"
#include "stl/string.h"

START_AIPROXY_NAMESPACE

//////////////////////////////////// Channel //////////////////////////////////////////
class AIProxyApi;
class AIProxyApiChannel : public AIBaseChannel
{
public:
    void SetProxyApi( AIProxyApi* apoProxyApi );
    virtual int Input( const char* apcData, size_t aiSize );
    virtual int Error( int aiErrno );
    virtual int Exit();
    
    virtual int SendTo( AISocketHandle atHandle, const char* apcData, size_t aiSize );
    virtual int RecvFrom( AISocketHandle atHandle, AIChunkEx& aoBuffer, int aiTimeout );
    
protected:
    AIProxyApi*     cpoProxyApi;
};

//////////////////////////////////////// AIProxyApi ///////////////////////////////////////
class AIProxyApi
{
public:
    struct stMsgNode
    {
        stMsgNode( AIChunkEx* apoBuffer );
        int Wait( AITime_t aiTimeOut );
        void Signal();
        
        void Reference();
        void Release();
        
        int         ciReferenceCnt;
        bool        cbIsSignal;
        AIChunkEx*  cpoBuffer;
        int         ciState;
        AICondition coCondition;
        AIMutexLock coLock;
    };
    
    struct stApiParam
    {
        stApiParam( 
            AIProxyApi* apoProxyApi, 
            stMsgNode*  apoMsgNode,
            uint32_t    aiMsgID,
            const char* apcData, 
            size_t      aiSize );
        ~stApiParam();
        
        uint32_t    ciMsgID;
        AIChunkEx   coData;
        AIProxyApi* cpoProxyApi;
        stMsgNode*  cpoMsgNode;
    };
    
    typedef AIHashMap< uint32_t, stMsgNode* > TMsgNodeMap;
    
    friend class AIProxyApiChannel;

public:
    AIProxyApi( AIProtocol* apoProtocol = NULL );
    virtual ~AIProxyApi();
    
    int Initialize();
    
    int Connect( const char* apcIpAddr, int aiPort, int aiConnCnt, int aiTimeOut );
    
    void Close();
    
    size_t GetConnectionCnt();
    
    int Request( const char* apcData, size_t aiSize );
    int Request( const char* apcData, size_t aiSize, AIChunkEx& aoResponse, int aiTimeOut );
    int Response( const char* apcData, size_t aiSize );
    
    AIProtocol* GetProtocol();
    
    virtual int InputRequest( const char* apcData, size_t aiSize );
    
protected:
    int Request( uint32_t aiMsgID, const char* apcData, size_t aiSize );
    
    ///Channel Entry
    int ChannelInput( const char* apcData, size_t aiSize );
    int ChannelError( AIChannelPtr aoChannel, int aiErrno );
    int ChannelExit( AIChannelPtr aoChannel );
    
    static void* RequestHandleThread( void* apvParam );
    
protected:
    AIBaseConnector   coConnector;
    AIChannelPool     coChannelPool;
    
    AITask            coTask;
    
    AITypeObject<uint32_t, AIMutexLock> ciMsgID;
    
    TMsgNodeMap       coMsgNode;
    AIMutexLock       coLock;
    AIProtocol*       cpoProtocol;
    
    bool              cbIsClosed;
    
    int               ciConnecteTimeOut;
};

END_AIPROXY_NAMESPACE

#endif //__AI_PROXY_API_H__
