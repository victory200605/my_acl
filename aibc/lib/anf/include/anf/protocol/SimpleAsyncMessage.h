
#ifndef ANF_SIMPLESYNCMESSAGE_H
#define ANF_SIMPLESYNCMESSAGE_H

#include "acl/MemoryBlock.h"
#include "anf/Utility.h"

ANF_NAMESPACE_START

class CSimpleAsyncMessage
{
public:
    CSimpleAsyncMessage(void);
    
    ~CSimpleAsyncMessage(void);

    apl_ssize_t Encode( acl::CMemoryBlock& aoBuffer );
    
    apl_ssize_t Decode( acl::CMemoryBlock& aoBuffer );
        
    void SetMessageID( apl_size_t auMessageID );
    
    apl_size_t GetMessageID(void) const;
    
    acl::CMemoryBlock& GetBody(void);
    
private:
    apl_int32_t miMessageID;
    
    apl_int32_t miResult;
  
    acl::CMemoryBlock moBody;
};

ANF_NAMESPACE_END

#endif//ANF_SIMPLESYNCMESSAGE_H
