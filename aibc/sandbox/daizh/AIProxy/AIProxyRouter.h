
#ifndef __AIPROXY_ROUTER_H__
#define __AIPROXY_ROUTER_H__

#include "AILib.h"
#include "AIString.h"
#include "AISynch.h"
#include "AITypeObject.h"
#include "AIHashMap.h"
#include "stl/string.h"
#include "AIProxyUtility.h"

START_AIPROXY_NAMESPACE

class AIServer;
///////////////////////////////////// singleton router ///////////////////////////////////////
class clsSequenceNumItem
{
public:
    clsSequenceNumItem( const char* apcSender = NULL, uint32_t aiInnerSeqNum = 0, uint32_t aiOuterSeqNum = 0 );
    
    void SetSender( const char* apcSender )        { ::StringCopy( this->csSender, apcSender, sizeof(this->csSender) ); }
    void SetInnerSeqNum( uint32_t aiSeqNum )       { this->ciInnerSeqNum = aiSeqNum; }
    void SetOuterSeqNum( uint32_t aiSeqNum )       { this->ciOuterSeqNum = aiSeqNum; }
    
    const char* GetSender() const                  { return this->csSender; }
    uint32_t GetInnerSeqNum() const                { return this->ciInnerSeqNum; }
    uint32_t GetOuterSeqNum() const                { return this->ciOuterSeqNum; }
    int      GetTimestamp() const                  { return this->ciTimestamp; }

protected:
    char csSender[AI_MAX_NAME_LEN];
    uint32_t ciInnerSeqNum;
    uint32_t ciOuterSeqNum;
    int  ciTimestamp;
};

class clsSequenceNumTable
{
public:
    typedef clsSequenceNumItem TItem;
    typedef AIHashMap< uint32_t, TItem > TMap;

public:
    bool Insert( uint32_t aiKey, const TItem& aoItem );
    bool Get( uint32_t aiKey, TItem& aoItem );
    void ClearTimeOut( int aiTimeOut );
    
protected:
    TMap coMap;
    AIMutexLock coMapLock;
};

///////////////////////////////////// base routing //////////////////////////////////////////
class AIRouting
{   
public:
    virtual int Error( int aiErrno, const char* apcSender, const char* apcData, size_t aiSize ) = 0;
    virtual int Route( const char* apcData, size_t aiSize, char* apcAddressee, size_t aiBufferSize ) = 0;
};

///////////////////////////////////// default inner routing /////////////////////////////////
class AIDefaultInnerRouting : public AIRouting
{
public:
    virtual int Error( int aiErrno, const char* apcSender, const char* apcData, size_t aiSize );
    virtual int Route( const char* apcData, size_t aiSize, char* apcAddressee, size_t aiBufferSize );
};

class AIRouter
{
public:
    typedef clsSequenceNumTable::TItem TSeqNumItem;
    typedef clsSequenceNumTable TSeqNumTable;
    
public:
    static AIRouter* Instance();
    
    int Initialize();
    
    void InnerRouting( AIRouting* apoInnerRouting )               { this->cpoInnerRouting = apoInnerRouting; }
    
    void InnerServer( AIServer* apoInnerServer )                  { this->cpoInnerServer = apoInnerServer; }
    AIServer* InnerServer()                                       { return this->cpoInnerServer; }
    
    void OuterRouting( AIRouting* apoOuterRouting )               { this->cpoOuterRouting = apoOuterRouting; }
    
    void OuterServer( AIServer* apoOuterServer )                  { this->cpoOuterServer = apoOuterServer; }
    AIServer* OuterServer()                                       { return this->cpoOuterServer; }

    int Input( const char* apcAddressee, const char* apcData, size_t aiSize );
    
    int Output( const char* apcAddresse, const char* apcData, size_t aiSize );

protected:
    AIRouter();
    
    ///For Sequence Number Table
    bool GetFromOutputTable( uint32_t aiKey, TSeqNumItem& aoItem );
    bool GetFromInputTable( uint32_t aiKey, TSeqNumItem& aoItem );
    bool PutToOutputTable( uint32_t aiKey, const TSeqNumItem& aoItem );
    bool PutToInputTable( uint32_t aiKey, const TSeqNumItem& aoItem );
    
    ///Recycle Timeout Sequence Number
    static void* TimeOutHandleThread( void* apvParam );
    
protected:
    AIServer*   cpoInnerServer;
    AIServer*   cpoOuterServer;
    
    AIRouting*  cpoInnerRouting;
    AIRouting*  cpoOuterRouting;
    
    TSeqNumTable coOuterSeqNumTable;
    TSeqNumTable coInnerSeqNumTable;
    
    AITypeObject<uint32_t, AIMutexLock> ciOuterSequenceNum;
    AITypeObject<uint32_t, AIMutexLock> ciInnerSequenceNum;
};

////////////////////////////////////////// Global interface ////////////////////////////////////////
AIRouter* GetRouter();
AIServer* GetOuterServer();
AIServer* GetInnerServer();
    
template< class TServerPtr >
TServerPtr GetOuterServer() { return dynamic_cast<TServerPtr>(AIRouter::Instance()->OuterServer()); }

template< class TServerPtr >
TServerPtr GetInnerServer() { return dynamic_cast<TServerPtr>(AIRouter::Instance()->InnerServer()); }

END_AIPROXY_NAMESPACE

#endif //#define __AIPROXY_ROUTER_H__
