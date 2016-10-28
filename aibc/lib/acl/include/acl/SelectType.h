/**
 * @file SelectType.h
 */
#ifndef ACL_SElECTTYPE_H
#define ACL_SElECTTYPE_H

#include "acl/Utility.h"

ACL_NAMESPACE_START

/**
 * class template TSelect<bool, T, F> 
 */
template < bool Flag, typename T, typename F > struct TSelect
{
    typedef T ResultType;
};

/**
 * instantied class template TSelect<false, T, F>
 */
template <typename T, typename F> struct TSelect<false, T, F>
{
    typedef F ResultType;
};

ACL_NAMESPACE_END

#endif //ACL_OPERATORS_H
