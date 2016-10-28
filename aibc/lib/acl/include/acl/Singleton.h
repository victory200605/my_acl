/**
 * @file Singleton.h
 */
#ifndef ACL_SINGLETON_H
#define ACL_SINGLETON_H

#include "acl/Utility.h"

ACL_NAMESPACE_START

template<typename T>
class TSingleton
{
public:
    static T* Instance(void)
    {
        if (cpoInstance == NULL)
        {
            ACL_NEW_ASSERT(cpoInstance, T);
        }
        return cpoInstance;
    }
    
    static void Release(void)
    {
        ACL_DELETE(cpoInstance);
    }
    
private:
    static T* cpoInstance;
};

template<typename T> T* TSingleton<T>::cpoInstance = NULL;

template<typename T> inline T* Instance(void)
{
    return TSingleton<T>::Instance();
}

template<typename T> inline void Release(void)
{
    TSingleton<T>::Release();
}

ACL_NAMESPACE_END

#endif//ACL_SINGLETON_H

