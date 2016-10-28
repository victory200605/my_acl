
#ifndef ANF_IOSESSIONATTRIBUTE_H
#define ANF_IOSESSIONATTRIBUTE_H

#include "anf/Utility.h"
#include "acl/stl/map.h"
#include "acl/stl/vector.h"
#include "acl/Synch.h"
#include "acl/Any.h"

ANF_NAMESPACE_START

/** 
 * @brief Session attributes, dynamic extending session attribute by any type
 */
class CIoSessionAttribute
{
    typedef std::map<std::string, acl::CAny> MapType;
    typedef std::vector<acl::CAny*> ListType;

public:
    CIoSessionAttribute(void)
    {
    }

    ~CIoSessionAttribute(void)
    {
        for (apl_size_t luN = 0; luN < this->moAttributeList.size(); luN++)
        {
            ACL_DELETE(this->moAttributeList[luN]);
        }
    }
    
    template<typename T>
    void Insert( char const* apcName, T& atValue )
    {
        acl::TSmartWLock<acl::CRWLock> loGuard(this->moLock);

        this->moAttributeMap.insert( MapType::value_type(apcName, acl::CAny(atValue) ) );
    }

    template<typename T>
    void Insert( std::string const& aoName, T const& atValue )
    {
        acl::TSmartWLock<acl::CRWLock> loGuard(this->moLock);

        this->moAttributeMap.insert( MapType::value_type(aoName, acl::CAny(atValue) ) );
    }
    
    void Erase( std::string const& aoName )
    {
        acl::TSmartWLock<acl::CRWLock> loGuard(this->moLock);

        this->moAttributeMap.erase(aoName);
    }

    template<typename T>
    void PushBack( T const& atValue )
    {
        acl::TSmartWLock<acl::CRWLock> loGuard(this->moLock);

        acl::CAny* lpoAny = APL_NULL;
        ACL_NEW_ASSERT(lpoAny, acl::CAny(atValue) );

        this->moAttributeList.push_back(lpoAny);
    }

    apl_size_t GetListSize(void) const
    {
        return this->moAttributeList.size();
    }

    //friends
    template<typename T>
    friend T* Find( CIoSessionAttribute& aoAttribute, char const* apcName )
    {
        return aoAttribute.Find( (T*)NULL, apcName);
    }

    template<typename T>
    friend T* Find( CIoSessionAttribute& aoAttribute, std::string const& aoName )
    {
        return aoAttribute.Find( (T*)NULL, aoName);
    }

    template<typename T>
    friend T* Find( CIoSessionAttribute* apoAttribute, char const* apcName )
    {
        return apoAttribute->Find( (T*)NULL, apcName);
    }

    template<typename T>
    friend T* Find( CIoSessionAttribute* apoAttribute, std::string const& aoName )
    {
        return apoAttribute->Find( (T*)NULL, aoName);
    }

    template<typename T>
    friend T* Find( CIoSessionAttribute* apoAttribute, apl_size_t auIndex )
    {
        return apoAttribute->Find( (T*)NULL, auIndex);
    }

    template<typename T>
    friend T* Find( CIoSessionAttribute& aoAttribute, apl_size_t auIndex )
    {
        return aoAttribute.Find( (T*)NULL, auIndex);
    }

protected:
    template<typename T>
    T* Find( T*, std::string const& aoName )
    {
        acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

        MapType::iterator loIter = this->moAttributeMap.find(aoName);

        if (loIter == this->moAttributeMap.end() )
        {
            return APL_NULL;
        }
        else
        {
            T* lptValue = acl::AnyCast<T>(loIter->second);
            if (lptValue == APL_NULL)
            {
                return APL_NULL;
            }
            else
            {
                return lptValue;
            }
        }
    }

    template<typename T>
    T* Find( T*, apl_size_t auIndex )
    {
        acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);
    
        if (auIndex < this->moAttributeList.size() )
        {
            T* lptValue = acl::AnyCast<T>(*this->moAttributeList[auIndex]);
            if (lptValue == APL_NULL)
            {
                return APL_NULL;
            }
            else
            {
                return lptValue;
            }
        }
        else
        {
            return APL_NULL; 
        }
    }

protected:
    acl::CRWLock moLock;

    MapType moAttributeMap;
    
    ListType moAttributeList;
};

ANF_NAMESPACE_END

#endif//ANF_IOSESSIONATTRIBUTE_H

