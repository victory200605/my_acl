
#include "anf/Exception.h"

ANF_NAMESPACE_START

CThrowable::CThrowable(apl_int_t aiState, apl_int_t aiErrno, char const* apcMessage)
    : miState(aiState)
    , miErrno(aiErrno)
    , moMessage(apcMessage)
{
}

apl_int_t CThrowable::GetState(void) const
{
    return this->miState;
}
    
apl_int_t CThrowable::GetErrno(void) const
{
    return this->miErrno;
}
    
char const* CThrowable::GetMessage(void) const
{
    return this->moMessage.c_str();
}

ANF_NAMESPACE_END
