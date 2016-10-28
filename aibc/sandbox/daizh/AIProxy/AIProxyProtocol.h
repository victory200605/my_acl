
#ifndef __AIPROXY__PROTOCOL_H__
#define __AIPROXY__PROTOCOL_H__

#include "AISyncTcp.h"
#include "AIProxyUtility.h"

START_AIPROXY_NAMESPACE

class AIProtocol
{
public:
    enum DIRECTION { REQUEST = 0, RESPONSE = 1 };
    
public:
    struct stHeader
    {
        stHeader();
        
        ////Operator
        void SetSequenceNum( uint32_t aiSequenceNum )            { this->ciSequenceNum = aiSequenceNum; }
        void SetType( int aiType )                               { this->ciType = aiType; }
        void SetLength( int aiLength )                           { this->ciLength = aiLength; }
        void SetDataPtr( const char* apcDataPtr )                { this->cpcDataPtr = apcDataPtr; }
        uint32_t GetSequenceNum( void ) const                    { return this->ciSequenceNum; }
        int GetType( void ) const                                { return this->ciType; }
        int GetLength( void ) const                              { return this->ciLength; }
        const char* GetDataPtr( void ) const                     { return this->cpcDataPtr; }
        
        ////Attribute
        uint32_t ciSequenceNum;
        int ciType;
        int ciLength;
        const char* cpcDataPtr;
    };
    typedef stHeader THeader;
    
public:
    virtual int Encode( const THeader& aoHeader, AIChunkEx& aoBuffer ) = 0;
    
    virtual int Decode( const char* apcData, size_t aiSize, THeader& aoHeader ) = 0;
    
    virtual int Send( AISocketHandle aoHandle, const char* apcData, size_t aiSize );
    
    virtual int Recv( AISocketHandle aoHandle, AIChunkEx& aoChunk, int aiTimeOut = AI_SYNC_TIMEOUT ) = 0;
};

class AIInnerProtocol : public virtual AIProtocol
{
public:
    virtual int Authenticate( AISocketHandle aoHandle, char* apcName, size_t aiSize );
    
    virtual int Encode( const THeader& aoHeader, AIChunkEx& aoBuffer );
    
    virtual int Decode( const char* apcData, size_t aiSize, THeader& aoHeader );
    
    virtual int Recv( AISocketHandle aoHandle, AIChunkEx& aoChunk, int aiTimeOut = AI_SYNC_TIMEOUT );

protected:
    int DecodeHeader( AIChunkEx& aoBuffer, THeader& aoHeader );
};

class AIOuterProtocol : public virtual AIProtocol
{
public:
    ///Authenticate
    virtual int Authenticate( AISocketHandle aoHandle, const char* apcName );
};

END_AIPROXY_NAMESPACE

#endif //__AI_PROXY_PROTOCOL_H__
