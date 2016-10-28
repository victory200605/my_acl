
#ifndef AI_GFQPROTOCOL_H_2008
#define AI_GFQPROTOCOL_H_2008

#include <stdio.h>
#include "GFQUtility.h"

//start namespace
AI_GFQ_NAMESPACE_START

//start command define
//client -> master
#define AI_GFQ_REQUEST_CREATE               0x01
#define AI_GFQ_REQUEST_DELETE               0x02
#define AI_GFQ_REQUEST_WRITESEG             0x03
#define AI_GFQ_REQUEST_WRITESEG_SECOND      0x04
#define AI_GFQ_REQUEST_READSEG              0x05
#define AI_GFQ_REQUEST_READSEG_SECOND       0x06
#define AI_GFQ_REQUEST_QUEUE_STAT           0x07
#define AI_GFQ_REQUEST_MODULE_COUNT         0x08

//client <- master
#define AI_GFQ_RESPONSE_CREATE              0x11
#define AI_GFQ_RESPONSE_DELETE              0x12
#define AI_GFQ_RESPONSE_WRITESEG            0x13
#define AI_GFQ_RESPONSE_READSEG             0x14
#define AI_GFQ_RESPONSE_QUEUE_STAT          0x15
#define AI_GFQ_RESPONSE_MODULE_COUNT        0x16

//module -> master 
#define AI_GFQ_REQUEST_ADD_MODULE           0x21
#define AI_GFQ_REQUEST_ADD_SEG              0x22
#define AI_GFQ_REQUEST_ADD_SEG_TRY          0x23
#define AI_GFQ_REQUEST_DEL_MODULE           0x24
#define AI_GFQ_RESPONSE_ALLOC               0x25
#define AI_GFQ_RESPONSE_FREE                0x26
#define AI_GFQ_RESPONSE_SEGMENTSIZE         0x27

//module <- master
#define AI_GFQ_RESPONSE_ADD_MODULE          0x31
#define AI_GFQ_RESPONSE_ADD_SEG             0x32
#define AI_GFQ_RESPONSE_DEL_MODULE          0x33
#define AI_GFQ_REQUEST_ALLOC                0x34
#define AI_GFQ_REQUEST_FREE                 0x35
#define AI_GFQ_REQUEST_SEGMENTSIZE          0x36

//client -> module
#define AI_GFQ_REQUEST_PUT                  0x41
#define AI_GFQ_REQUEST_GET                  0x42
//client <- module
#define AI_GFQ_RESPONSE_PUT                 0x51
#define AI_GFQ_RESPONSE_GET                 0x52

//Adm(commad) -> master
#define AI_GFQ_REQUEST_RELOADQUEUEINFO      0x60
#define AI_GFQ_REQUEST_PRINT_QUEUES         0x61
#define AI_GFQ_REQUEST_PRINT_MODULES        0x62
//Adm(commad) <- master
#define AI_GFQ_RESPONSE_RELOADQUEUEINFO     0x90
#define AI_GFQ_RESPONSE_PRINT_QUEUES        0x91
#define AI_GFQ_RESPONSE_PRINT_MODULES       0x92

#define AI_GFQ_REQUEST_HEARTBEAT            0x70
#define AI_GFQ_RESPONSE_HEARTBEAT           0x71

#define AI_GFQ_RESPONSE_ERROR               0x81
//end command define

class clsBody;

//------------------------clsHead------------------------//
class clsHeader
{
public:
    clsHeader();
    clsHeader( int32_t aiCmd, clsBody* apoBody /* = NULL */ );
    virtual ~clsHeader();

    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    int32_t GetSize();
    int32_t GetCmd();
    
protected:
    int32_t ciCmd;
    
    clsBody* cpoBody;
};

//------------------------clsBody------------------------//
class clsBody
{

public:
    virtual ~clsBody(){};

    virtual int32_t Decode(AIChunkEx& aoChunk) = 0;
    virtual int32_t Encode(AIChunkEx& aoChunk) = 0;
    virtual int32_t GetSize() =0;
};

//------------------------clsUserRequest------------------------//
class clsUserRequest : public clsBody
{
public:
    clsUserRequest();
    virtual ~clsUserRequest();
    
    virtual int32_t Decode(AIChunkEx& aoChunk);
    virtual int32_t Encode(AIChunkEx& aoChunk);
    virtual int32_t GetSize();
    
    void SetUserName( const char* apsUserName );
    const char* GetUserName();
    
protected:
    struct stBody
    {
        char ccUserName[AI_MAX_NAME_LEN];
    };
    
    stBody coBody;
};

//------------------------------------------------------------------//
class clsResponse : public clsBody
{
public:
    clsResponse();
    virtual ~clsResponse();

    virtual int32_t Decode(AIChunkEx& aoChunk);
    virtual int32_t Encode(AIChunkEx& aoChunk);
    virtual void SetStat(int32_t aiStat);
    virtual int32_t GetStat();
    int32_t GetSize();
protected:
    int32_t ciStat;
};

//------------------------clsRequestAddModule------------------------//
class clsRequestAddModule : public clsBody
{
public:
    clsRequestAddModule();
    virtual ~clsRequestAddModule();

    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    
    void SetModuleName(const char* apsModuleName);
    const char* GetModuleName();
    
    void SetIpAddr(const char* apsIpAddr);
    const char* GetIpAddr();
    
    void SetPort(int32_t aiPort);
    int32_t GetPort();
    
    int32_t GetSize();
    
protected:
    struct stBody
    {
        char ccModuleName[AI_MAX_NAME_LEN];
        char ccIpAddr[AI_IP_ADDR_LEN];
        int32_t ciPort;
    };

    stBody coBody;
};

//------------------------clsRequestDelModule------------------------//
class clsRequestDelModule : public clsBody
{
public:
    clsRequestDelModule();
    virtual ~clsRequestDelModule();

    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    
    void SetModuleName(const char* apsModuleName);
    const char* GetModuleName();

    int32_t GetSize();
    
protected:
    struct stBody
    {
        char ccModuleName[AI_MAX_NAME_LEN];
    };

    stBody coBody;
};

//------------------------clsRequestAddSeg------------------------//
class clsRequestAddSeg : public clsBody
{
public:
    clsRequestAddSeg();
    virtual ~clsRequestAddSeg();

    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    
    void SetModuleName(const char* apsModuleName);
    const char* GetModuleName();
    
    void SetSegmentID(int32_t aiSegmentID);
    int32_t GetSegmentID();
    
    void SetQueueName(const char* apsQueueName);
    const char* GetQueueName();
    
    void SetQueueGlobalID(int64_t aiQueueGlobalID);
    int64_t GetQueueGlobalID();
    
    void SetSegmentSize( int32_t aiSize );
    int32_t GetSegmentSize();
    
    void SetSegmentCapacity( int32_t aiCapacity );
    int32_t GetSegmentCapacity();
    
    int32_t GetSize();

protected:
    struct stBody
    {
        char ccModuleName[AI_MAX_NAME_LEN];
        int32_t ciSegmentID;
        char ccQueueName[AI_MAX_NAME_LEN];
        char ccQueueGlobalID[AI_MAX_INT_LEN];
        int32_t ciSegmentSize;
        int32_t ciSegmentCapacity;
    };

    stBody coBody;
};

//---------------------------clsRequestWriteSeg--------------------------------//
class clsRequestAddSegTry : public clsRequestAddSeg
{};

//---------------------------clsRequestWriteSeg--------------------------------//
class clsRequestWriteSeg : public clsUserRequest
{
public:
    clsRequestWriteSeg();
    virtual ~clsRequestWriteSeg();

    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);

    void SetQueueName(const char* apsQueueName);
    const char* GetQueueName();
    
    int32_t GetSize();
protected:
    struct stBody
    {
        char ccQueueName[AI_MAX_NAME_LEN];
    };
    stBody coBody;
};

//---------------------------clsRequestWriteSegSecond--------------------------------//
class clsRequestWriteSegSecond : public clsUserRequest
{
public:
    clsRequestWriteSegSecond();
    virtual ~clsRequestWriteSegSecond();

    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    
    void SetQueueName(const char* apsQueueName);
    const char* GetQueueName();
    
    void SetSegmentID(int32_t aiSegmentID);
    int32_t GetSegmentID();

    void SetModuleID(int32_t aiModuleID);
    int32_t GetModuleID();
    
    void SetRetcode(int32_t aiRetcode);
    int32_t GetRetcode();
    
    int32_t GetSize();
    
protected:
    struct stBody
    {
        char ccQueueName[AI_MAX_NAME_LEN];
        int32_t ciSegmentID;
        int32_t ciModuleID;
        int32_t ciRetcode;
    };

    stBody coBody;
};

//---------------------------clsRequestQueueStat--------------------------------//
class clsRequestQueueStat : public clsUserRequest
{
public:
    clsRequestQueueStat();
    virtual ~clsRequestQueueStat();

    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);

    void SetQueueName(const char* apsQueueName);
    const char* GetQueueName();
    
    int32_t GetSize();
protected:
    struct stBody
    {
        char ccQueueName[AI_MAX_NAME_LEN];
    };
    stBody coBody;
};

//---------------------------clsRequestCommad--------------------------------//
class clsRequestCommad : public clsUserRequest
{
public:
    clsRequestCommad();
    virtual ~clsRequestCommad();

    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);

    int32_t GetSize();
};

//------------------------clsRequestPrintQueue------------------------//
class clsRequestPrintQueue : public clsRequestCommad {};
    
//------------------------clsRequestPrintModule------------------------//
class clsRequestPrintModule : public clsRequestCommad {};
    
//------------------------clsRequestPrintModule------------------------//
class clsRequestReloadQueueInfo : public clsRequestCommad {};
    
//------------------------clsRequestPrintModule------------------------//
class clsRequestModuleCount : public clsRequestCommad {};
    
//------------------------clsRequestReadSeg------------------------//
class clsRequestReadSeg : public clsRequestWriteSeg {};

//------------------------clsRequestReadSegSecond------------------------//
class clsRequestReadSegSecond : public clsRequestWriteSegSecond {};

//------------------------clsRequestAlloc------------------------//
class clsRequestAlloc : public clsBody
{
public:
    clsRequestAlloc();
    virtual ~clsRequestAlloc();

    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    
    void SetSegmentID(int32_t aiSegmentID);
    int32_t GetSegmentID();
    
    void SetQueueName(const char* apsQueueName);
    const char* GetQueueName();
    
    void SetQueueGlobalID(int64_t aiQueueGlobalID);
    int64_t GetQueueGlobalID();
    
    int32_t GetSize();

protected:
    struct stBody
    {
        int32_t ciSegmentID;
        char ccQueueName[AI_MAX_NAME_LEN];
        char ccQueueGlobalID[AI_MAX_INT_LEN];
    };

    stBody coBody;
};

//------------------------clsRequestFree------------------------//
class clsRequestFree : public clsBody
{
public:
    clsRequestFree();
    virtual ~clsRequestFree();

    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    
    void SetSegmentID(int32_t aiSegmentID);
    int32_t GetSegmentID();
    
    int32_t GetSize();

protected:
    struct stBody
    {
        int32_t ciSegmentID;
    };

    stBody coBody;
};

//------------------------clsRequestSegmentSize------------------------//
class clsRequestSegmentSize : public clsBody
{
public:
    clsRequestSegmentSize();
    virtual ~clsRequestSegmentSize();

    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    
    void SetSegmentID(int32_t aiSegmentID);
    int32_t GetSegmentID();
    
    int32_t GetSize();

protected:
    struct stBody
    {
        int32_t ciSegmentID;
    };

    stBody coBody;
};

//------------------------clsRequestPut------------------------//
class clsRequestPut : public clsUserRequest
{
public:
    clsRequestPut();
    virtual ~clsRequestPut();

    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    
    void SetSegmentID(int32_t aiSegmentID);
    int32_t GetSegmentID();
    
    void SetQueueName(const char* apsQueueName);
    const char* GetQueueName();
    
    void SetQueueGlobalID(int64_t aiQueueGlobalID);
    int64_t GetQueueGlobalID();
    
    void SetData(const AIChunkEx& aoData);
    void GetData(AIChunkEx& aoData);
    
    int32_t GetSize();

protected:
    struct stBody
    {
        int32_t ciSegmentID;
        char ccQueueName[AI_MAX_NAME_LEN];
        char ccQueueGlobalID[AI_MAX_INT_LEN];
        char ccData[AI_MAX_DATA_LEN];
        int32_t  ciDataSize;
    };

    stBody coBody;
};

//------------------------clsRequestGet------------------------//
class clsRequestGet : public clsUserRequest
{
public:
    clsRequestGet();
    virtual ~clsRequestGet();

    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    
    void SetSegmentID(int32_t aiSegmentID);
    int32_t GetSegmentID();
    
    void SetQueueName(const char* apsQueueName);
    const char* GetQueueName();
    
    void SetQueueGlobalID(int64_t aiQueueGlobalID);
    int64_t GetQueueGlobalID();
    
    void SetWhenSec(int32_t aiWhenSec);
    int32_t GetWhenSec();
    
    int32_t GetSize();

protected:
    struct stBody
    {
        int32_t ciSegmentID;
        char ccQueueName[AI_MAX_NAME_LEN];
        char ccQueueGlobalID[AI_MAX_INT_LEN];
        int32_t ciWhenSec;
    };

    stBody coBody;
};

//------------------------clsRequestHeartBeat------------------------//
class clsRequestHeartBeat : public clsBody
{
public:
    clsRequestHeartBeat();
    virtual ~clsRequestHeartBeat();
    
    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);

    void SetModuleName(const char* apsModuleName);
    const char* GetModuleName();

    int32_t GetSize();
    
protected:
    struct stBody
    {
        char ccModuleName[AI_MAX_NAME_LEN];
    };

    stBody coBody;
};

/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////reponse////////////////////////////////////////
class clsResponseSegment : public clsResponse
{
public:
    clsResponseSegment();
    virtual ~clsResponseSegment();

    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    
    void SetSegmentID(int32_t aiSegmentID);
    int32_t GetSegmentID();
    
    void SetModuleID(int32_t aiModuleID);
    int32_t GetModuleID();
    
    void SetQueueGlobalID(int64_t aiQueueGlobalID);
    int64_t GetQueueGlobalID();
    
    void SetIpAddr(const char* apsIpAddr);
    const char* GetIpAddr();
    
    void SetPort(int32_t aiPort);
    int32_t GetPort();
    
    int32_t GetSize();

protected:
    struct stBody
    {
        int32_t ciSegmentID;
        int32_t ciModuleID;
        char ccQueueGlobalID[AI_MAX_INT_LEN];
        char ccIpAddr[AI_IP_ADDR_LEN];
        int32_t ciPort;
    };

    stBody coBody;
};

//----------------------------------------clsResponseQueueStat-----------------------//
class clsResponseQueueStat : public clsResponse
{
public:
    clsResponseQueueStat();
    virtual ~clsResponseQueueStat();
    
    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    
    void SetQueueSize( int32_t aiSize );
    int32_t GetQueueSize( );
    
    void SetQueueCapacity( int32_t aiCapacity );
    int32_t GetQueueCapacity( );
    
    int32_t GetSize();

protected:
    struct stBody
    {
        int32_t ciSize;
        int32_t ciCapacity;
    };

    stBody coBody;
};

//----------------------------------------clsResponseModuleCount-----------------------//
class clsResponseModuleCount : public clsResponse
{
public:
    clsResponseModuleCount();
    virtual ~clsResponseModuleCount();
    
    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    
    void SetModuleCount( int32_t aiCount );
    int32_t GetModuleCount( );
    
    int32_t GetSize();

protected:
    struct stBody
    {
        int32_t ciCount;
    };

    stBody coBody;
};

//----------------------------------------clsResponseGet-----------------------//
class clsResponseGet : public clsResponse
{
public:
    clsResponseGet();
    virtual ~clsResponseGet();
    
    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    
    void SetData(const AIChunkEx& aoData);
    void GetData(AIChunkEx& aoData);
    
    int32_t GetSize();

protected:
    struct stBody
    {
        char ccData[AI_MAX_DATA_LEN];
        int32_t  ciDataSize;
    };

    stBody coBody;
};

//----------------------------------------clsResponseSegmentSize-----------------------//
class clsResponseSegmentSize : public clsResponse
{
public:
    clsResponseSegmentSize();
    virtual ~clsResponseSegmentSize();
    
    int32_t Decode(AIChunkEx& aoChunk);
    int32_t Encode(AIChunkEx& aoChunk);
    
    void SetSegmentSize( int32_t aiSize );
    int32_t GetSegmentSize( );
    
    int32_t GetSize();

protected:
    struct stBody
    {
        int32_t ciSize;
    };

    stBody coBody;
};

//----------------------------------------clsResponseWriteSeg-----------------------//
typedef clsResponseSegment clsResponseWriteSeg;

//----------------------------------------clsResponseWriteSeg-----------------------//
typedef clsResponseSegment clsResponseReadSeg;

//----------------------------------------clsResponseWriteSeg-----------------------//
typedef clsResponseSegment clsResponseWriteSegSecond;

//----------------------------------------clsResponseWriteSeg-----------------------//
typedef clsResponseSegment clsResponseReadSegSecond;

//----------------------------------------clsResponseAddModule-----------------------//
typedef clsResponse clsResponseAddModule;

//----------------------------------------clsResponseDelModule-----------------------//
typedef clsResponse clsResponseDelModule;

//----------------------------------------clsResponseAddSeg-----------------------//
typedef clsResponse clsResponseAddSeg;

//----------------------------------------clsResponseAlloc-----------------------//
typedef clsResponse clsResponseAlloc;

//----------------------------------------clsResponseAlloc-----------------------//
typedef clsResponse clsResponseFree;

//----------------------------------------clsResponsePut-----------------------//
typedef clsResponse clsResponsePut;

//----------------------------------------clsResponseHearBeat-----------------------//
typedef clsResponse clsResponseHeartBeat;

//----------------------------------------clsResponsePrintQueue-----------------------//
typedef clsResponse clsResponsePrintQueue;

//----------------------------------------clsResponsePrintQueue-----------------------//
typedef clsResponse clsResponsePrintModule;

//----------------------------------------clsResponsePrintQueue-----------------------//
typedef clsResponse clsResponseReloadQueueInfo;

//----------------------------------------clsResponseError-----------------------//
typedef clsResponse clsResponseError;

// protocol packet length
#define AI_PACKET_LEN( body ) \
    ( clsHeader().GetSize() + body.GetSize() )
    
// process request error
#define AI_GFQ_DECODE_RESPONSE( response, header, body, ret )       \
    header.Decode(response);                                        \
    if ( header.GetCmd() == AI_GFQ_RESPONSE_ERROR )                 \
    {                                                               \
        clsResponse loResponseBody;                                 \
        loResponseBody.Decode( response );                          \
                                                                    \
        ret = loResponseBody.GetStat();                             \
    }                                                               \
    else                                                            \
    {                                                               \
        body.Decode( response );                                    \
        ret = body.GetStat();                                       \
    }

// do request to server
template< class TAsyncClient, class TRequest, class TResponse >
int32_t DoRequestTo( TAsyncClient& aoAsyncClient, int32_t aiRequestCmd, TRequest& aoRequest, TResponse& aoResponse  )
{
    clsHeader loHeader( aiRequestCmd, &aoRequest );
    clsHeader loRespHeader;
    AIChunkEx loRequestChunk;
    AIChunkEx loResponseChunk( loRespHeader.GetSize() + aoResponse.GetSize() );
    int32_t liRepSize = 0;
    int32_t liRet = AI_NO_ERROR;
    
    loHeader.Encode(loRequestChunk);
    
    if ( ( liRet = aoAsyncClient.SendRequest( loRequestChunk.BasePtr(), loRequestChunk.GetSize(), NULL ) ) != 0 )
    {
        AI_GFQ_ERROR( "[Do Request]Send Request fail - ASYNCCODE:%d, MSG:%s", liRet, strerror(errno) );
        return AI_ERROR_SOCKET_SEND;
    }
    
    if ( ( liRet = aoAsyncClient.RecvResponse( loResponseChunk.BasePtr(), loResponseChunk.GetSize(), liRepSize ) ) != 0 )
    {
        AI_GFQ_ERROR( "[Do Request]Recv Response fail - ASYNCCODE:%d, MSG:%s", liRet, strerror(errno) );
        return AI_ERROR_SOCKET_RECV;
    }
    
    loResponseChunk.Reset();
    loResponseChunk.WritePtr(liRepSize);

    AI_GFQ_DECODE_RESPONSE( loResponseChunk, loRespHeader, aoResponse, liRet );
    
    return liRet;
}

// do request to server
//template< class TAsyncClient, class TRequest, class TResponse >
//int32_t DoRequestTo( TAsyncClient& aoAsyncClient, int32_t aiRequestCmd, TRequest& aoRequest )
//{
//    clsHeader loHeader( aiRequestCmd, &aoRequest );
//    clsHeader loRespHeader;
//    AIChunkEx loRequestChunk;
//    int32_t liRepSize = 0;
//    int32_t liRet = AI_NO_ERROR;
//    
//    loHeader.Encode(loRequestChunk);
//    
//    if ( ( liRet = aoAsyncClient.SendRequest( loRequestChunk.BasePtr(), loRequestChunk.GetSize(), NULL ) ) != 0 )
//    {
//        AI_GFQ_ERROR( "[Do Request]Send Request fail - ASYNCCODE:%d, MSG:%s", liRet, strerror(errno) );
//        return AI_ERROR_SOCKET_SEND;
//    }
//
//    return liRet;
//}

//end namespace
AI_GFQ_NAMESPACE_END

#endif //AI_GFQPROTOCOL_H_2008
