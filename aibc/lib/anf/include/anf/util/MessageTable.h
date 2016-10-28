
#ifndef ANF_MESSAGETABLE_H
#define ANF_MESSAGETABLE_H

#include "acl/stl/string.h"
#include "acl/Timestamp.h"
#include "acl/Synch.h"
#include "acl/MemCache.h"
#include "anf/Utility.h"

ANF_NAMESPACE_START

//////////////////////////////////////////////////////////////////////////////////////
template<typename KeyType, typename ValueType>
class TMessageTable
{
public:
    typedef acl::TMemCache<KeyType, ValueType> TableType;
        
public:
    TMessageTable( apl_size_t auCapacity = APL_UINT_MAX );
    
    void Put( KeyType const& atKey, ValueType const& atValue );
    
    bool Get( KeyType const& atKey, ValueType& atValue );
    
    bool Find( KeyType const& atKey, ValueType& atValue );
    
    template<typename RecycleFuncType>
    void ClearTimeout( acl::CTimeValue const& aoTimeout, RecycleFuncType const& aoRecycle );

private:
    acl::CLock moLock;
        
    TableType moTable;
};

/////////////////////////////////////////////////////////////////////////////////////////
//implement
template<typename KeyType, typename ValueType>
TMessageTable<KeyType, ValueType>::TMessageTable( apl_size_t auCapacity )
    : moTable(auCapacity)
{
}

template<typename KeyType, typename ValueType>
void TMessageTable<KeyType, ValueType>::Put( KeyType const& atKey, ValueType const& atValue )
{
    acl::TSmartLock<acl::CLock> loGuarb(this->moLock);
        
    this->moTable.Put(atKey, atValue);
}

template<typename KeyType, typename ValueType>
bool TMessageTable<KeyType, ValueType>::Get( KeyType const& atKey, ValueType& atValue )
{
    acl::TSmartLock<acl::CLock> loGuarb(this->moLock);
        
    return this->moTable.Get(atKey, &atValue, NULL, TableType::OPT_DELETE) == 0;
}

template<typename KeyType, typename ValueType>
bool TMessageTable<KeyType, ValueType>::Find( KeyType const& atKey, ValueType& atValue )
{
    acl::TSmartLock<acl::CLock> loGuarb(this->moLock);
        
    return this->moTable.Get(atKey, &atValue, NULL, TableType::OPT_NODELETE) == 0;
}
 
template<typename KeyType, typename ValueType>
template<typename RecycleFuncType>
void TMessageTable<KeyType, ValueType>::ClearTimeout( acl::CTimeValue const& aoTimeout, RecycleFuncType const& aoRecycle )
{
    acl::CTimestamp loCurrTime;
    acl::TSmartLock<acl::CLock> loGuarb(this->moLock);
        
    loCurrTime.Update(acl::CTimestamp::PRC_USEC);
    
    for (typename TableType::IteratorType loIter = this->moTable.Begin();
        loIter != this->moTable.End(); )
    {
        if (loCurrTime.Sec() - loIter->Timestamp().Sec() > aoTimeout.Sec() )
        {
            aoRecycle(loIter->Value() );
            
            loIter = this->moTable.Erase(loIter);
            
            continue;
        }
        
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//Message - node
class CProxyMessageNode
{
public:
    CProxyMessageNode(void);
    
    CProxyMessageNode(
        apl_uint64_t au64InnerSessionID,
        apl_uint64_t au64OuterSessionID,
        apl_uint64_t au64InnerMessageID,
        apl_uint64_t au64OuterMessageID,
        std::string const& aoRemark );
    
    apl_uint64_t GetInnerSessionID(void);
    
    apl_uint64_t GetOuterSessionID(void);
    
    apl_uint64_t GetInnerMessageID(void);
    
    apl_uint64_t GetOuterMessageID(void);
    
    std::string& GetRemark(void);
    
    acl::CTimestamp& GetTimestamp(void);

private:
    apl_uint64_t mu64InnerSessionID;
    apl_uint64_t mu64OuterSessionID;
    apl_uint64_t mu64InnerMessageID;
    apl_uint64_t mu64OuterMessageID;
    std::string  moRemark;
    acl::CTimestamp moTimestamp;
};

/////////////////////////////////////////////////////////////////////////////////////////
typedef TMessageTable<apl_uint64_t, CProxyMessageNode> ProxyMessageTableType;

ANF_NAMESPACE_END

#endif//ANF_MESSAGETABLE_H

