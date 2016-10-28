
#ifndef ACL_OPERATORS_H
/**
 * @file Operators.h
 */
#define ACL_OPERATORS_H

#include "acl/Utility.h"

ACL_NAMESPACE_START

/**
 * class template TLessThan2
 * Operator overwrite by T < U and T > U, Basic operator classes
 */
template<typename T, typename U>
class TLessThan2
{
public:
    friend inline bool operator <= (const T& x, const U& y) { return !(x > y); }
    friend inline bool operator >= (const T& x, const U& y) { return !(x < y); }
    friend inline bool operator > (const U& x, const T& y)  { return y < x; }
    friend inline bool operator < (const U& x, const T& y)  { return y > x; }
    friend inline bool operator <= (const U& x, const T& y) { return !(y < x); }
    friend inline bool operator >= (const U& x, const T& y) { return !(y > x); }
};

/**
 * class template TLessThan1
 * Operator overwrite by T < U, Basic operator classes
 */
template<typename T>
class TLessThan1
{
public:
    friend inline bool operator > (const T& x, const T& y)  { return y < x; }
    friend inline bool operator <= (const T& x, const T& y) { return !(y < x); }
    friend inline bool operator >= (const T& x, const T& y) { return !(x < y); }
};

/**
 * class template TLessThanAll2
 * Operator overwrite by T < U and T > U, Basic operator classes
 */
template<typename T, typename U>
class TLessThanAll2
{
public:
    friend inline bool operator <= (const T& x, const U& y) { return !(x > y); }
    friend inline bool operator >= (const T& x, const U& y) { return !(x < y); }
    friend inline bool operator == (const T& x, const U& y) { return !(x < y) && !(y < x); }
    friend inline bool operator != (const T& x, const U& y) { return !(x == y); }
    friend inline bool operator > (const U& x, const T& y)  { return y < x; }
    friend inline bool operator < (const U& x, const T& y)  { return y > x; }
    friend inline bool operator <= (const U& x, const T& y) { return !(y < x); }
    friend inline bool operator >= (const U& x, const T& y) { return !(y > x); }
    friend inline bool operator == (const U& x, const T& y) { return y == x; }
    friend inline bool operator != (const U& x, const T& y) { return !(y == x); }
};

/**
 * class template TLessThanAll1
 * Operator overwrite by T < U, Basic operator classes
 */
template<typename T>
class TLessThanAll1
{
public:
    friend inline bool operator >  (const T& x, const T& y) { return y < x; }
    friend inline bool operator == (const T& x, const T& y) { return !(y < x) && !(y > x); }
	friend inline bool operator != (const T& x, const T& y) { return !(y == x); }
    friend inline bool operator <= (const T& x, const T& y) { return !(y < x); }
    friend inline bool operator >= (const T& x, const T& y) { return !(x < y); }
};

/**
 * class template TEquality2 
 * Operator overwrite by == , Basic operator classes
 */
template <typename T, typename U>
struct TEquality2
{
     friend inline bool operator == (const U& y, const T& x) { return x == y; }
     friend inline bool operator != (const U& y, const T& x) { return !(x == y); }
     friend inline bool operator != (const T& y, const U& x) { return !(y == x); }
};

/**
 * class template TEquality1
 * Operator overwrite by == , Basic operator classes
 */
template <typename T>
struct TEquality1
{
     friend inline bool operator != (const T& x, const T& y) { return !(x == y); }
};

ACL_NAMESPACE_END

#endif //ACL_OPERATORS_H
