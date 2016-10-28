
#ifndef ANF_UTILITY_H
#define ANF_UTILITY_H

#include "acl/Synch.h"
#include "acl/SmartPtr.h"

#define ANF_NAMESPACE_START namespace anf {
#define ANF_NAMESPACE_END }

ANF_NAMESPACE_START

class CIoSession;
typedef acl::TSharePtr<CIoSession, acl::CLock> SessionPtrType;

ANF_NAMESPACE_END

#endif //ANF_UTILITY_H
