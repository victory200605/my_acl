
#include "anf/util/MessageTable.h"

ANF_NAMESPACE_START

/////////////////////////////////////////////////////////////////////////////////////////
//Message - node
CProxyMessageNode::CProxyMessageNode(void)
    : mu64InnerSessionID(0)
    , mu64OuterSessionID(0)
    , mu64InnerMessageID(0)
    , mu64OuterMessageID(0)
{
    this->moTimestamp.Update(acl::CTimestamp::PRC_USEC);
}
    
CProxyMessageNode::CProxyMessageNode(
    apl_uint64_t au64InnerSessionID,
    apl_uint64_t au64OuterSessionID,
    apl_uint64_t au64InnerMessageID,
    apl_uint64_t au64OuterMessageID,
    std::string const& aoRemark )
    : mu64InnerSessionID(au64InnerSessionID)
    , mu64OuterSessionID(au64OuterSessionID)
    , mu64InnerMessageID(au64InnerMessageID)
    , mu64OuterMessageID(au64OuterMessageID)
    , moRemark(aoRemark)
{
    this->moTimestamp.Update(acl::CTimestamp::PRC_USEC);
}
    
apl_uint64_t CProxyMessageNode::GetInnerSessionID(void)
{
    return this->mu64InnerSessionID;
}
    
apl_uint64_t CProxyMessageNode::GetOuterSessionID(void)
{
    return this->mu64OuterSessionID;
}
    
apl_uint64_t CProxyMessageNode::GetInnerMessageID(void)
{
    return this->mu64InnerMessageID;
}

apl_uint64_t CProxyMessageNode::GetOuterMessageID(void)
{
    return this->mu64OuterMessageID;
}

std::string& CProxyMessageNode::GetRemark(void)
{
    return this->moRemark;
}
    
acl::CTimestamp& CProxyMessageNode::GetTimestamp(void)
{
    return this->moTimestamp;
}

ANF_NAMESPACE_END
