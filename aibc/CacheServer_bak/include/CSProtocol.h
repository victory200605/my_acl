
#ifndef __AI_CACHESERVER_POTOCOL_H__
#define __AI_CACHESERVER_POTOCOL_H__

#include "AIChunkEx.h"
#include "AILogSys.h"
#include "../include/Config.h"
#include "../include/Errno.h"

AI_CACHE_NAMESPACE_START

//request
#define AI_CACHESERVER_REQUEST_PUT             0x0001 //put request
#define AI_CACHESERVER_REQUEST_GET             0x0002 //get request
#define AI_CACHESERVER_REQUEST_PUT_OW          0x0007 //put request
#define AI_CACHESERVER_REQUEST_GET_ND          0x0003 //get request
#define AI_CACHESERVER_REQUEST_DELTIMEOUT      0x0004 //delete over time request
#define AI_CACHESERVER_REQUEST_STAT            0x0005 //get stat request
#define AI_CACHESERVER_REQUEST_UPDATE          0x0006 //get request

//response
#define AI_CACHESERVER_RESPONSE_PUT            0x8001 //put response
#define AI_CACHESERVER_RESPONSE_GET            0x8002 //get response
#define AI_CACHESERVER_RESPONSE_PUT_OW         0x8007 //put response
#define AI_CACHESERVER_RESPONSE_GET_ND         0x8003 //get response
#define AI_CACHESERVER_RESPONSE_DELTIMEOUT     0x8004 //delete over time response
#define AI_CACHESERVER_RESPONSE_STAT           0x8005 //get stat response
#define AI_CACHESERVER_RESPONSE_UPDATE         0x8006 //get request

#define AI_CACHESERVER_RESPONSE_ERROR          0x1000

const size_t AI_DEFAULT_KEY_LEN     = 128;
const size_t AI_DEFAULT_VALUE_LEN   = 1024;
const size_t AI_MAX_RECV_BUFFER_LEN = AI_DEFAULT_KEY_LEN + AI_DEFAULT_VALUE_LEN + 128;

#define INVALID_RET( op, ret )  if ( op != 0 ) return ret;
    
class clsBody;

///////////////////////////////////////////// header ////////////////////////////////////////////
class clsHeader
{
public:
	clsHeader( int aiCmd = 0, clsBody* apoBody = NULL );
	virtual ~clsHeader();

	int Decode(AIBC::AIChunkEx& aoBuffer);
	int Encode(AIBC::AIChunkEx& aoBuffer);
	int GetCmd();
	int GetSize();

protected:
    int ciTotalLen;
	int ciCmd;
	
	clsBody* cpoBody;
};

////////////////////////////////////////////// clsBody /////////////////////////////////////////
class clsBody
{
public:
	virtual ~clsBody() {};

	virtual int Decode(AIBC::AIChunkEx& aoBuffer) = 0;
	virtual int Encode(AIBC::AIChunkEx& aoBuffer) = 0;
	virtual int GetSize() = 0;
};

////////////////////////////////////////////// Request ////////////////////////////////////////
class clsRequestPut : public clsBody
{
public:
	clsRequestPut();
	virtual ~clsRequestPut();

	int Decode(AIBC::AIChunkEx& aoBuffer);
	int Encode(AIBC::AIChunkEx& aoBuffer);
	
	int GetSize();
	AIBC::AIChunkEx& GetKey();
	AIBC::AIChunkEx& GetValue();

protected:
    int       ciMsgType; //compatible last version
    AIBC::AIChunkEx coKey;
    AIBC::AIChunkEx coValue;
};

class clsRequestPutOW : public clsRequestPut {};

class clsRequestGet : public clsBody
{
public:
	clsRequestGet();
	virtual ~clsRequestGet();

	int Decode(AIBC::AIChunkEx& aoBuffer);
	int Encode(AIBC::AIChunkEx& aoBuffer);
	
	int GetSize();
	AIBC::AIChunkEx& GetKey();

protected:
    int       ciKeySize;
    AIBC::AIChunkEx coKey;
};

class clsRequestGetND : public clsRequestGet {};

class clsRequestDelTimeOut : public clsBody
{
public:
	clsRequestDelTimeOut();
	virtual ~clsRequestDelTimeOut();

	int Decode(AIBC::AIChunkEx& aoBuffer);
	int Encode(AIBC::AIChunkEx& aoBuffer);
	
	int GetSize();

protected:
    int ciMsgType; //compatible last version
};

class clsRequestCommon : public clsBody
{
public:
	clsRequestCommon();
	virtual ~clsRequestCommon();

	int Decode(AIBC::AIChunkEx& aoBuffer);
	int Encode(AIBC::AIChunkEx& aoBuffer);
	int GetSize();
};

class clsRequestUpdate : public clsRequestPut {};

class clsRequestStat : public clsRequestCommon {};

///////////////////////////////////////////// response ////////////////////////////////////
class clsResponse : public clsBody
{
public:
	clsResponse();
	virtual ~clsResponse();

	int Encode(AIBC::AIChunkEx& aoBuffer);
	int Decode(AIBC::AIChunkEx& aoBuffer);
	
	int GetStat();
	void SetStat(int aiStat);
	
	int GetSize();

protected:
    int ciStat;
};

class clsResponsePut : public clsResponse {};

class clsResponsePutOW : public clsResponse {};
        
class clsResponseUpdate : public clsResponse {};

class clsResponseGet : public clsResponse
{
public:
	clsResponseGet();
	virtual ~clsResponseGet();

	int Decode(AIBC::AIChunkEx& aoBuffer);
	int Encode(AIBC::AIChunkEx& aoBuffer);

	int GetSize();
	
	AIBC::AIChunkEx& GetValue();

protected:
    AIBC::AIChunkEx coValue;
};

class clsResponseGetND : public clsResponseGet {};

class clsResponseDelTimeOut : public clsResponse
{
public:
	clsResponseDelTimeOut();
	virtual ~clsResponseDelTimeOut();

	int Decode(AIBC::AIChunkEx& aoBuffer);
	int Encode(AIBC::AIChunkEx& aoBuffer);
	
	int GetSize();
	AIBC::AIChunkEx& GetKey();
	AIBC::AIChunkEx& GetValue();

protected:
	AIBC::AIChunkEx coKey;
    AIBC::AIChunkEx coValue;
};

class clsResponseStat : public clsResponse
{
public:
	clsResponseStat();
	virtual ~clsResponseStat();

	int Decode(AIBC::AIChunkEx& aoBuffer);
	int Encode(AIBC::AIChunkEx& aoBuffer);
	
	int GetSize();
	
	void SetFileCount(int aiFileCnt);
	int  GetFileCount();
	void SetCacheCapacity(int aiCapacity);
	int  GetCacheCapacity();
	void SetCacheSize(int aiSize);
	int  GetCacheSize();

protected:
	int ciFileCount;
    int ciCapacity;
    int ciSize;
};

// process request error
#define AI_DECODE_RESPONSE( response, header, body, ret )           \
	header.Decode(response);                                        \
	if ( header.GetCmd() == AI_CACHESERVER_RESPONSE_ERROR )         \
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
int DoRequestTo( TAsyncClient& aoAsyncClient, int aiRequestCmd, TRequest& aoRequest, TResponse& aoResponse  )
{
	clsHeader loHeader( aiRequestCmd, &aoRequest );
	clsHeader loRespHeader;
	AIBC::AIChunkEx loRequestChunk;
	AIBC::AIChunkEx loResponseChunk( AI_MAX_RECV_BUFFER_LEN );
	int liRepSize = 0;
	int liRet = AI_NO_ERROR;
	
	loHeader.Encode(loRequestChunk);
	
	if ( ( liRet = aoAsyncClient.SendRequest( loRequestChunk.BasePtr(), loRequestChunk.GetSize(), NULL ) ) != 0 )
	{
	    AIBC::AIWriteLOG( NULL, AILOG_LEVEL_ERROR, "[Do Request]Send Request fail - ASYNCCODE:%d, MSG:%s", liRet, strerror(errno) );
		return AI_ERROR_SOCKET_SEND;
	}
    
	if ( ( liRet = aoAsyncClient.RecvResponse( loResponseChunk.BasePtr(), loResponseChunk.GetSize(), liRepSize ) ) != 0 )
	{
	    AIBC::AIWriteLOG( NULL, AILOG_LEVEL_ERROR, "[Do Request]Recv Response fail - ASYNCCODE:%d, MSG:%s", liRet, strerror(errno) );
		return AI_ERROR_SOCKET_RECV;
	}
	
	loResponseChunk.Reset();
	loResponseChunk.WritePtr(liRepSize);

	AI_DECODE_RESPONSE( loResponseChunk, loRespHeader, aoResponse, liRet );
	
	return liRet;
}

AI_CACHE_NAMESPACE_END

#endif // __AI_CACHESERVER_POTOCOL_H__
