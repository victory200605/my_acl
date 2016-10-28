
#ifndef ISMG_COUNTSERVERROTOCOL_H
#define ISMG_COUNTSERVERROTOCOL_H

#include <stdlib.h>
#include <inttypes.h>

//define command id
#define CNTSVR_REQUEST_CONN           0x01
#define CNTSVR_REQUEST_SEND           0x02
#define CNTSVR_RESPONE_CONN           0x11
#define CNTSVR_RESPONE_SEND           0x12
//end

class CCSPHead
{
public:
    enum { HEAD_SIZE = 4 + 4 + 4 };

public:
    CCSPHead();
    
    ssize_t Encode( char* apcBuffer, size_t auSize );
    
    ssize_t Decode( char const* apcBuffer, size_t auSize );
    
    int32_t GetMsgID(void);
    
    void SetMsgID( int32_t aiMsgID );
    
    int32_t GetLength(void);

    void SetLength( int32_t aiLength );
    
private:
    int32_t miMsgID;
    int32_t miResult;
    int32_t miLength;
};

class CCSPRequest : public CCSPHead
{
public:
    enum { REQUEST_SIZE = HEAD_SIZE + 4 + 64 + 4 + 4 };

public:
    CCSPRequest();
    
    ssize_t Encode( char* apcBuffer, size_t auSize );
    
    ssize_t Decode( char const* apcBuffer, size_t auSize );
    
    int32_t GetCmdID(void);
    
    void SetCmdID( int32_t aiCmdID );
    
    char const* GetKey(void);
    
    void SetKey( char const* apcKey );
    
    int32_t GetNum(void);
    
    void SetNum( int32_t aiNum );
    
    int32_t GetLastNum(void);
    
    void SetLastNum( int32_t aiNum );
    
private:
    int32_t miCmdID;
    char    macKey[64];
    int32_t miNum;
    int32_t miLastNum;
};

class CCSPResponse : public CCSPHead
{
public:
    enum { RESPONSE_SIZE = HEAD_SIZE + 4 + 4 + 8 };

public:
    CCSPResponse();
    
    ssize_t Encode( char* apcBuffer, size_t auSize );
    
    ssize_t Decode( char const* apcBuffer, size_t auSize );
    
    int32_t GetCmdID(void);
    
    void SetCmdID( int32_t aiMsgID );

    int32_t GetState(void);
    
    void SetState( int32_t aiState );
    
    int64_t GetTimestamp(void);
    
    void SetTimestamp( int64_t ai64Timestamp );

private:
    int32_t miCmdID;
    int32_t miState;
    int64_t mi64Timestamp;
};

#endif// ISMG_COUNTSERVERROTOCOL_H
