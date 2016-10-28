
#ifndef __AIMCACHE_CMDLINETCP_H__
#define __AIMCACHE_CMDLINETCP_H__

#include "AICacheTcp.h"
#include "../include/Utility.h"

AI_CACHE_NAMESPACE_START

///
class AICmdLineParamter
{
public:
    struct stParamter
    {
        stParamter()
            : cpcFirst(NULL)
            , ciLen(0)
        {
        }
        
        const char* cpcFirst;
        int ciLen;
    };
public:
    int Decode( const char* apcData, size_t aiSize );
    static int Encode( AIBC::AIChunkEx& aoBuffer, const char* apcFormat, ... );
    
    const char* GetCmdPtr()                                  { return this->coCmd.cpcFirst; }
    int GetCmdLen()                                          { return this->coCmd.ciLen; }
    
    const char* GetPtr( int aiOpt )                          { return this->coParamter[aiOpt - 'a'].cpcFirst; }
    int GetLen( int aiOpt )                                  { return this->coParamter[aiOpt - 'a' ].ciLen; }
    
protected:
    const char* cpcBasePtr;
    int ciSize;
    
    stParamter coCmd;
    
    stParamter coParamter['z' - 'a' + 1];
};

/////////////////////////////////////////////////////////////////////////////////////
//AICmdLineChannel
class AICmdLineChannel : public AIBaseChannel
{
public:
    AICmdLineChannel( CHANNEL_MODE aiMode = REACTIVE );
    virtual ~AICmdLineChannel();
    
protected:
    /// Reserve transmit interface for protocol
    virtual int  SendTo( AISocketHandle atHandle, const char* apcData, size_t aiSize ) ;
    virtual int  RecvFrom( AISocketHandle atHandle, AIBC::AIChunkEx& aoBuffer, int aiTimeout = AI_SYNC_TIMEOUT );
};

AI_CACHE_NAMESPACE_END

#endif //__AIMCACHE_CMDLINETCP_H__
