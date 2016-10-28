#ifndef ANF_CMDLINEMESSAGE_H
#define ANF_CMDLINEMESSAGE_H

#include "acl/MemoryBlock.h"
#include "acl/SString.h"
#include "anf/Utility.h"

ANF_NAMESPACE_START

class CCmdLineMessage
{
public:
    //Default @auLineMax = 1024
    CCmdLineMessage(void);

    CCmdLineMessage( apl_size_t auLineMaxLength );

    ~CCmdLineMessage(void);

    apl_int_t Decode( acl::CMemoryBlock* apoBuffer );

    apl_int_t Encode( acl::CMemoryBlock* apoBuffer );

    void CmdLine( char const* apcPattern, ... );

    apl_size_t GetArgc(void);

    char const* GetArgv( apl_size_t auN );

private:
    acl::CTokenizer moSpliter;
    
    char* mpcBuffer;
    
    apl_size_t muLineMaxLength;
};

ANF_NAMESPACE_END

#endif//ANF_CMDLINEMESSAGE_H

