#ifndef AIMC_EMPROXY_CMDLINEMESSAGE_H
#define AIMC_EMPROXY_CMDLINEMESSAGE_H

#include "acl/MemoryBlock.h"
#include "acl/SString.h"

class CCmdLineMessage
{
public:
    CCmdLineMessage(void);

    ~CCmdLineMessage(void);

    apl_int_t Decode( acl::CMemoryBlock* apoBuffer );

    apl_int_t Encode( acl::CMemoryBlock* apoBuffer );

    void CmdLine( char const* apcPattern, ... );

    apl_size_t GetArgc(void);

    char const* GetArgv( apl_size_t auN );

private:
    acl::CTokenizer moSpliter;
    char macBuffer[256];
};

#endif//AIMC_EMPROXY_CMDLINEMESSAGE_H

