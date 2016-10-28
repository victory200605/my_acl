
#ifndef ANF_ROUTINGTABLE_H
#define ANF_ROUTINGTABLE_H

#include "anf/Utility.h"
#include "acl/stl/map.h"
#include "acl/Synch.h"

ANF_NAMESPACE_START

template<typename KeyType>
struct TRoutingTableImplLess
{
    bool operator () ( KeyType const& aoLeft, KeyType const& aoRight )
    {
        return aoLeft < aoRight;
    }
};

template <> struct TRoutingTableImplLess<std::string>
{
    bool operator () ( std::string const& aoLeft, std::string const& aoRight )
    {
        apl_size_t liCmpLen = 
            aoLeft.length() < aoRight.length() ? aoLeft.length() : aoRight.length();
            
        return apl_strncmp(aoLeft.c_str(), aoRight.c_str(), liCmpLen ) < 0;
    }
};

template <typename KeyType, typename ValueType>
class TRoutingTableImpl
{
    struct CValue
    {
        CValue(void)
        {
        }
        
        CValue( KeyType const& aoKey, ValueType const& aoValue )
            : mtKeyBegin(aoKey)
            , mtValue(aoValue)
        {
        }
        
        KeyType   mtKeyBegin;
        ValueType mtValue;
    };
    
public:
    typedef std::map<KeyType, CValue, TRoutingTableImplLess<KeyType> > RTMapType;

public:
    bool Insert( KeyType const& aoKeyBegin, KeyType const& aoKeyEnd, ValueType const& aoValue );

    bool Find( KeyType const& aoKey, ValueType& aoValue );

    void Clear(void);

private:
    RTMapType moTable;
};

//Routing-Table
template <typename KeyType, typename ValueType>
class TRoutingTable
{
public:
    typedef TRoutingTableImpl<KeyType, ValueType> TableType;
        
public:
    TRoutingTable(void)
        : mpoMaster(NULL)
        , mpoSlaver(NULL)
    {
        ACL_NEW_ASSERT(this->mpoMaster, TableType);
        ACL_NEW_ASSERT(this->mpoSlaver, TableType);
    }
    
    ~TRoutingTable(void)
    {
        ACL_DELETE(this->mpoMaster);
        ACL_DELETE(this->mpoSlaver);
    }
    
    bool Update(std::string const& aoKeyBegin, std::string const& aoKeyEnd, std::string const& aoValue)
    {
        return this->mpoSlaver->Insert(aoKeyBegin, aoKeyEnd, aoValue);
    }

    bool Find(std::string const& aoKey, std::string& aoValue)
    {
        acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

        return this->mpoMaster->Find(aoKey, aoValue);
    }

    void Commit(void)
    {
        acl::TSmartWLock<acl::CRWLock> loGuard(this->moLock);
            
        std::swap(this->mpoMaster, this->mpoSlaver);
        
        this->mpoSlaver->Clear();
    }
    
    void Rollback(void)
    {
        this->mpoSlaver->Clear();
    }
    
private:
    TableType* mpoMaster;
    TableType* mpoSlaver;
    
    acl::CRWLock moLock;
};

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename KeyType, typename ValueType>
bool TRoutingTableImpl<KeyType, ValueType>::Insert(
    KeyType const& aoKeyBegin, 
    KeyType const& aoKeyEnd, 
    ValueType const& aoValue )
{
    CValue loValue(aoKeyBegin, aoValue);

    moTable[aoKeyEnd] = loValue;

    return true;
}

template <typename KeyType, typename ValueType>
bool TRoutingTableImpl<KeyType, ValueType>::Find( KeyType const& aoKey, ValueType& aoValue )
{
    typename RTMapType::iterator loIter = this->moTable.lower_bound(aoKey);
        
    if( loIter != this->moTable.end() && loIter->second.mtKeyBegin <= aoKey )
    {
        aoValue = loIter->second.mtValue;
        return true;
    }
    else
    {
        return false;
    }
}

template <typename KeyType, typename ValueType>
void TRoutingTableImpl<KeyType, ValueType>::Clear()
{
    this->moTable.clear();
}

ANF_NAMESPACE_END

#endif //ROUTINGTABLE_H
