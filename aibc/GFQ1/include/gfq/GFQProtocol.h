
#ifndef AIBC_GFQ_PROTOCOL_H
#define AIBC_GFQ_PROTOCOL_H

#include "acl/MemoryBlock.h"
#include "gfq/Utility.h"

//start namespace
AIBC_GFQ_NAMESPACE_START

//start command define
//client -> master
#define GFQ_REQUEST_CREATE               0x01
#define GFQ_REQUEST_DELETE               0x02
#define GFQ_REQUEST_WRITESEG             0x03
#define GFQ_REQUEST_WRITESEG_SECOND      0x04
#define GFQ_REQUEST_READSEG              0x05
#define GFQ_REQUEST_READSEG_SECOND       0x06
#define GFQ_REQUEST_QUEUE_STAT           0x07
#define GFQ_REQUEST_MODULE_COUNT         0x08

//client <- master
#define GFQ_RESPONSE_CREATE              0x11
#define GFQ_RESPONSE_DELETE              0x12
#define GFQ_RESPONSE_WRITESEG            0x13
#define GFQ_RESPONSE_READSEG             0x14
#define GFQ_RESPONSE_QUEUE_STAT          0x15
#define GFQ_RESPONSE_MODULE_COUNT        0x16

//module -> master 
#define GFQ_REQUEST_ADD_MODULE           0x21
#define GFQ_REQUEST_ADD_SEG              0x22
#define GFQ_REQUEST_ADD_SEG_TRY          0x23
#define GFQ_REQUEST_DEL_MODULE           0x24
#define GFQ_RESPONSE_ALLOC               0x25
#define GFQ_RESPONSE_FREE                0x26
#define GFQ_RESPONSE_SEGMENTSIZE         0x27

//module <- master
#define GFQ_RESPONSE_ADD_MODULE          0x31
#define GFQ_RESPONSE_ADD_SEG             0x32
#define GFQ_RESPONSE_DEL_MODULE          0x33
#define GFQ_REQUEST_ALLOC                0x34
#define GFQ_REQUEST_FREE                 0x35
#define GFQ_REQUEST_SEGMENTSIZE          0x36

//client -> module
#define GFQ_REQUEST_PUT                  0x41
#define GFQ_REQUEST_GET                  0x42
//client <- module
#define GFQ_RESPONSE_PUT                 0x51
#define GFQ_RESPONSE_GET                 0x52

//Adm(commad) -> master
#define GFQ_REQUEST_RELOADQUEUEINFO      0x60
#define GFQ_REQUEST_PRINT_QUEUES         0x61
#define GFQ_REQUEST_PRINT_MODULES        0x62
//Adm(commad) <- master
#define GFQ_RESPONSE_RELOADQUEUEINFO     0x90
#define GFQ_RESPONSE_PRINT_QUEUES        0x91
#define GFQ_RESPONSE_PRINT_MODULES       0x92

#define GFQ_REQUEST_HEARTBEAT            0x70
#define GFQ_RESPONSE_HEARTBEAT           0x71

#define GFQ_RESPONSE_ERROR               0x81
//end command define

class CMessageBody;
//------------------------CHead------------------------//
class CMessageHeader
{
public:
    CMessageHeader(void);

    CMessageHeader( apl_int_t aiCmd, CMessageBody* apoBody /* = NULL */ );
    
    virtual ~CMessageHeader(void);

    apl_int_t Decode( acl::CMemoryBlock& aoBuffer );

    apl_int_t Encode( acl::CMemoryBlock& aoBuffer );
    
    apl_size_t GetSize(void);
    
    apl_int_t GetCmd(void);
    
protected:
    apl_int32_t miCmd;
    
    CMessageBody* mpoBody;
};

//------------------------CMessageBody------------------------//
class CMessageBody
{

public:
    virtual ~CMessageBody(void){};

    virtual apl_int_t Decode(acl::CMemoryBlock& aoBuffer) = 0;
    virtual apl_int_t Encode(acl::CMemoryBlock& aoBuffer) = 0;
    virtual apl_size_t GetSize() =0;
};

//------------------------CUserRequest------------------------//
class CUserRequest : public CMessageBody
{
public:
    CUserRequest(void);
    virtual ~CUserRequest(void);
    
    virtual apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    virtual apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    virtual apl_size_t GetSize();
    
    void SetUserName( const char* apcUserName );
    const char* GetUserName(void);
    
protected:
    char macUserName[MAX_NAME_LEN];
};

//------------------------------------------------------------------//
class CResponse : public CMessageBody
{
public:
    CResponse(void);
    virtual ~CResponse(void);

    virtual apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    virtual apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    virtual void SetStat(apl_int_t aiStat);
    virtual apl_int_t GetStat();
    apl_size_t GetSize();

protected:
    apl_int32_t miStat;
};

//------------------------CRequestAddModule------------------------//
class CRequestAddModule : public CMessageBody
{
public:
    CRequestAddModule(void);
    virtual ~CRequestAddModule(void);

    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    
    void SetModuleName(const char* apcModuleName);
    const char* GetModuleName();
    
    void SetIpAddr(const char* apsIpAddr);
    const char* GetIpAddr();
    
    void SetPort(apl_int_t aiPort);
    apl_int_t GetPort();
    
    apl_size_t GetSize();
    
protected:
    char macModuleName[MAX_NAME_LEN];
    char macIpAddr[IP_ADDR_LEN];
    apl_int32_t miPort;
};

//------------------------CRequestDelModule------------------------//
class CRequestDelModule : public CMessageBody
{
public:
    CRequestDelModule(void);
    virtual ~CRequestDelModule(void);

    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    
    void SetModuleName(const char* apcModuleName);
    const char* GetModuleName(void);

    apl_size_t GetSize(void);
    
protected:
    char macModuleName[MAX_NAME_LEN];
};

//------------------------CRequestAddSeg------------------------//
class CRequestAddSeg : public CMessageBody
{
public:
    CRequestAddSeg(void);
    virtual ~CRequestAddSeg(void);

    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    
    void SetModuleName(const char* apcModuleName);
    const char* GetModuleName(void);
    
    void SetSegmentID(apl_int_t aiSegmentID);
    apl_int_t GetSegmentID(void);
    
    void SetQueueName(const char* apcQueueName);
    const char* GetQueueName(void);
    
    void SetQueueGlobalID(apl_int64_t aiQueueGlobalID);
    apl_int64_t GetQueueGlobalID(void);
    
    void SetSegmentSize( apl_int_t aiSize );
    apl_int_t GetSegmentSize(void);
    
    void SetSegmentCapacity( apl_int_t aiCapacity );
    apl_int_t GetSegmentCapacity(void);
    
    apl_size_t GetSize(void);

protected:
    char macModuleName[MAX_NAME_LEN];
    apl_int32_t miSegmentID;
    char macQueueName[MAX_NAME_LEN];
    char macQueueGlobalID[MAX_INT_LEN];
    apl_int32_t miSegmentSize;
    apl_int32_t miSegmentCapacity;
};

//---------------------------CRequestWriteSeg--------------------------------//
class CRequestAddSegTry : public CRequestAddSeg
{};

//---------------------------CRequestWriteSeg--------------------------------//
class CRequestWriteSeg : public CUserRequest
{
public:
    CRequestWriteSeg(void);
    virtual ~CRequestWriteSeg(void);

    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);

    void SetQueueName(const char* apcQueueName);
    const char* GetQueueName(void);
    
    apl_size_t GetSize(void);

protected:
    char macQueueName[MAX_NAME_LEN];
};

//---------------------------CRequestWriteSegSecond--------------------------------//
class CRequestWriteSegSecond : public CUserRequest
{
public:
    CRequestWriteSegSecond(void);
    virtual ~CRequestWriteSegSecond(void);

    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    
    void SetQueueName(const char* apcQueueName);
    const char* GetQueueName(void);
    
    void SetSegmentID(apl_int_t aiSegmentID);
    apl_int_t GetSegmentID(void);

    void SetModuleID(apl_int_t aiModuleID);
    apl_int_t GetModuleID(void);
    
    void SetRetcode(apl_int_t aiRetcode);
    apl_int_t GetRetcode(void);
    
    apl_size_t GetSize(void);
    
protected:
    char macQueueName[MAX_NAME_LEN];
    apl_int32_t miSegmentID;
    apl_int32_t miModuleID;
    apl_int32_t miRetcode;
};

//---------------------------CRequestQueueStat--------------------------------//
class CRequestQueueStat : public CUserRequest
{
public:
    CRequestQueueStat(void);
    virtual ~CRequestQueueStat(void);

    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);

    void SetQueueName(const char* apcQueueName);
    const char* GetQueueName(void);
    
    apl_size_t GetSize();

protected:
    char macQueueName[MAX_NAME_LEN];
};

//---------------------------CRequestCommad--------------------------------//
class CRequestCommad : public CUserRequest
{
public:
    CRequestCommad(void);
    virtual ~CRequestCommad(void);

    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);

    apl_size_t GetSize(void);
};

//------------------------CRequestPrintQueue------------------------//
class CRequestPrintQueue : public CRequestCommad {};
    
//------------------------CRequestPrintModule------------------------//
class CRequestPrintModule : public CRequestCommad {};
    
//------------------------CRequestPrintModule------------------------//
class CRequestReloadQueueInfo : public CRequestCommad {};
    
//------------------------CRequestPrintModule------------------------//
class CRequestModuleCount : public CRequestCommad {};
    
//------------------------CRequestReadSeg------------------------//
class CRequestReadSeg : public CRequestWriteSeg {};

//------------------------CRequestReadSegSecond------------------------//
class CRequestReadSegSecond : public CRequestWriteSegSecond {};

//------------------------CRequestAlloc------------------------//
class CRequestAlloc : public CMessageBody
{
public:
    CRequestAlloc(void);
    virtual ~CRequestAlloc(void);

    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    
    void SetSegmentID(apl_int_t aiSegmentID);
    apl_int_t GetSegmentID(void);
    
    void SetQueueName(const char* apsQueueName);
    const char* GetQueueName(void);
    
    void SetQueueGlobalID(int64_t aiQueueGlobalID);
    int64_t GetQueueGlobalID(void);
    
    apl_size_t GetSize(void);

protected:
    apl_int32_t miSegmentID;
    char macQueueName[MAX_NAME_LEN];
    char macQueueGlobalID[MAX_INT_LEN];
};

//------------------------CRequestFree------------------------//
class CRequestFree : public CMessageBody
{
public:
    CRequestFree(void);
    virtual ~CRequestFree(void);

    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    
    void SetSegmentID(apl_int_t aiSegmentID);
    apl_int_t GetSegmentID(void);
    
    apl_size_t GetSize(void);

protected:
    apl_int32_t miSegmentID;
};

//------------------------CRequestSegmentSize------------------------//
class CRequestSegmentSize : public CMessageBody
{
public:
    CRequestSegmentSize(void);
    virtual ~CRequestSegmentSize(void);

    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    
    void SetSegmentID(apl_int_t aiSegmentID);
    apl_int_t GetSegmentID(void);
    
    apl_size_t GetSize(void);

protected:
    apl_int32_t miSegmentID;
};

//------------------------CRequestPut------------------------//
class CRequestPut : public CUserRequest
{
public:
    CRequestPut(void);
    virtual ~CRequestPut(void);

    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    
    void SetSegmentID(apl_int_t aiSegmentID);
    apl_int_t GetSegmentID(void);
    
    void SetQueueName(const char* apsQueueName);
    const char* GetQueueName(void);
    
    void SetQueueGlobalID(apl_int64_t aiQueueGlobalID);
    apl_int64_t GetQueueGlobalID(void);
    
    void SetData(const acl::CMemoryBlock& aoData);
    void GetData(acl::CMemoryBlock& aoData);
    
    apl_size_t GetSize(void);

protected:
    apl_int32_t miSegmentID;
    char macQueueName[MAX_NAME_LEN];
    char macQueueGlobalID[MAX_INT_LEN];
    char macData[MAX_DATA_LEN];
    apl_int32_t miDataSize;
};

//------------------------CRequestGet------------------------//
class CRequestGet : public CUserRequest
{
public:
    CRequestGet(void);
    virtual ~CRequestGet(void);

    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    
    void SetSegmentID(apl_int_t aiSegmentID);
    apl_int_t GetSegmentID(void);
    
    void SetQueueName(const char* apsQueueName);
    const char* GetQueueName(void);
    
    void SetQueueGlobalID(int64_t aiQueueGlobalID);
    int64_t GetQueueGlobalID(void);
    
    void SetWhenSec(apl_int_t aiWhenSec);
    apl_int_t GetWhenSec(void);
    
    apl_size_t GetSize(void);

protected:
    apl_int32_t miSegmentID;
    char macQueueName[MAX_NAME_LEN];
    char macQueueGlobalID[MAX_INT_LEN];
    apl_int32_t miWhenSec;
};

//------------------------CRequestHeartBeat------------------------//
class CRequestHeartBeat : public CMessageBody
{
public:
    CRequestHeartBeat(void);
    virtual ~CRequestHeartBeat(void);
    
    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);

    void SetModuleName(const char* apcModuleName);
    const char* GetModuleName(void);

    apl_size_t GetSize(void);
    
protected:
    char macModuleName[MAX_NAME_LEN];
};

/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////reponse////////////////////////////////////////
class CResponseSegment : public CResponse
{
public:
    CResponseSegment(void);
    virtual ~CResponseSegment(void);

    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    
    void SetSegmentID(apl_int_t aiSegmentID);
    apl_int_t GetSegmentID(void);
    
    void SetModuleID(apl_int_t aiModuleID);
    apl_int_t GetModuleID(void);
    
    void SetQueueGlobalID(apl_int64_t aiQueueGlobalID);
    apl_int64_t GetQueueGlobalID(void);
    
    void SetIpAddr(const char* apsIpAddr);
    const char* GetIpAddr(void);
    
    void SetPort(apl_int_t aiPort);
    apl_int_t GetPort(void);
    
    apl_size_t GetSize();

protected:
    apl_int32_t miSegmentID;
    apl_int32_t miModuleID;
    char macQueueGlobalID[MAX_INT_LEN];
    char macIpAddr[IP_ADDR_LEN];
    apl_int32_t miPort;
};

//----------------------------------------CResponseQueueStat-----------------------//
class CResponseQueueStat : public CResponse
{
public:
    CResponseQueueStat(void);
    virtual ~CResponseQueueStat(void);
    
    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    
    void SetQueueSize( apl_int_t aiSize );
    apl_int_t GetQueueSize(void);
    
    void SetQueueCapacity( apl_int_t aiCapacity );
    apl_int_t GetQueueCapacity(void);
    
    apl_size_t GetSize(void);

protected:
    apl_int32_t miSize;
    apl_int32_t miCapacity;
};

//----------------------------------------CResponseModuleCount-----------------------//
class CResponseModuleCount : public CResponse
{
public:
    CResponseModuleCount(void);
    virtual ~CResponseModuleCount(void);
    
    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    
    void SetModuleCount( apl_int_t aiCount );
    apl_int_t GetModuleCount(void);
    
    apl_size_t GetSize(void);

protected:
    apl_int32_t miCount;
};

//----------------------------------------CResponseGet-----------------------//
class CResponseGet : public CResponse
{
public:
    CResponseGet(void);
    virtual ~CResponseGet(void);
    
    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    
    void SetData(const acl::CMemoryBlock& aoData);
    void GetData(acl::CMemoryBlock& aoData);
    
    apl_size_t GetSize(void);

protected:
    char macData[MAX_DATA_LEN];
    apl_int32_t  miDataSize;
};

//----------------------------------------CResponseSegmentSize-----------------------//
class CResponseSegmentSize : public CResponse
{
public:
    CResponseSegmentSize(void);
    virtual ~CResponseSegmentSize(void);
    
    apl_int_t Decode(acl::CMemoryBlock& aoBuffer);
    apl_int_t Encode(acl::CMemoryBlock& aoBuffer);
    
    void SetSegmentSize( apl_int_t aiSize );
    apl_int_t GetSegmentSize(void);
    
    apl_size_t GetSize(void);

protected:
    apl_int32_t miSize;
};

//----------------------------------------CResponseWriteSeg-----------------------//
typedef CResponseSegment CResponseWriteSeg;

//----------------------------------------CResponseWriteSeg-----------------------//
typedef CResponseSegment CResponseReadSeg;

//----------------------------------------CResponseWriteSeg-----------------------//
typedef CResponseSegment CResponseWriteSegSecond;

//----------------------------------------CResponseWriteSeg-----------------------//
typedef CResponseSegment CResponseReadSegSecond;

//----------------------------------------CResponseAddModule-----------------------//
typedef CResponse CResponseAddModule;

//----------------------------------------CResponseDelModule-----------------------//
typedef CResponse CResponseDelModule;

//----------------------------------------CResponseAddSeg-----------------------//
typedef CResponse CResponseAddSeg;

//----------------------------------------CResponseAlloc-----------------------//
typedef CResponse CResponseAlloc;

//----------------------------------------CResponseAlloc-----------------------//
typedef CResponse CResponseFree;

//----------------------------------------CResponsePut-----------------------//
typedef CResponse CResponsePut;

//----------------------------------------CResponseHearBeat-----------------------//
typedef CResponse CResponseHeartBeat;

//----------------------------------------CResponsePrintQueue-----------------------//
typedef CResponse CResponsePrintQueue;

//----------------------------------------CResponsePrintQueue-----------------------//
typedef CResponse CResponsePrintModule;

//----------------------------------------CResponsePrintQueue-----------------------//
typedef CResponse CResponseReloadQueueInfo;

//----------------------------------------CResponseError-----------------------//
typedef CResponse CResponseError;

//end namespace
AIBC_GFQ_NAMESPACE_END

#endif //AIBC_GFQPROTOCOL_H

