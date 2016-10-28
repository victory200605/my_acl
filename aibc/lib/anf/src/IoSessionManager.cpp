
#include "anf/IoServiceListener.h"

ANF_NAMESPACE_START

apl_size_t CIoServiceListener::Size(void)
{
    return this->moSessions.Size();
}

void CIoServiceListener::SessionCreated(SessionPtrType& aoSession)
{
    this->moSessions.Insert(aoSession.GetHandle(), aoSession);
}

void CIoServiceListener::SessionDestroyed(SessionPtrType& aoSession)
{
    this->moSessions.Erase(aoSession.GetHandle() );
}

ANF_NAMESPACE_END

#endif//ANF_IOSERVICELISTENER_H
