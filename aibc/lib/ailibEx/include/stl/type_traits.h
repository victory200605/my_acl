//=============================================================================
/**
 * \file    type_traits.h
 * \brief reference STL type_traits standard interface
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: type_traits.h,v 1.1 2008/11/04 03:50:19 fzz Exp $
 *
 * History
 * 2008.02.29 first release by DZH
 */
//=============================================================================
#ifndef __AILIB__TYPE_TRAITS_H__
#define __AILIB__TYPE_TRAITS_H__

#include "config.h"
#include "iterator_base.h"

START_STLNAMESPACE

//struct type define
struct stTrueType       { char cV[2]; };
struct stFalseType      { };

template<class T1, class T2> inline stFalseType PtrCat( T1, T2 )
{ 
     return stFalseType();
}

template<class T> inline stTrueType PtrCat( T **,  T ** )
{
     return stTrueType();
}

template<class T> inline stTrueType PtrCat( T **, const T ** )
{	
     return stTrueType();
}

template< class T > inline stTrueType PtrCat( T *const *, T ** )
{
     return stTrueType();
}

template<class T> inline stTrueType PtrCat( T *const *, const T ** )
{
     return stTrueType();
}

inline stTrueType PtrCat( bool *, bool * )
{
     return stTrueType();
}

inline stTrueType PtrCat( const bool *, bool * )
{
     return stTrueType();
}

inline stTrueType PtrCat( char *, char * )
{
     return stTrueType();
}

inline stTrueType PtrCat( const char *, char * )
{
     return stTrueType();
}

inline stTrueType PtrCat( signed char *, signed char * )
{
     return stTrueType();
}

inline stTrueType PtrCat( const signed char *, signed char * )
{
     return stTrueType();
}

inline stTrueType PtrCat( unsigned char *, unsigned char * )
{
     return stTrueType();
}

inline stTrueType PtrCat( const unsigned char *, unsigned char * )
{
     return stTrueType();
}

#ifdef _NATIVE_WCHAR_T_DEFINED
inline stTrueType PtrCat( wchar_t *, wchar_t * )
{
     return stTrueType();
}

inline stTrueType PtrCat( const wchar_t *, wchar_t * )
{
     return stTrueType();
}
#endif /* _NATIVE_WCHAR_T_DEFINED */

inline stTrueType PtrCat( short *, short * )
{
     return stTrueType();
}

inline stTrueType PtrCat( const short *, short * )
{
     return stTrueType();
}

inline stTrueType PtrCat( unsigned short *, unsigned short * )
{
     return stTrueType();
}

inline stTrueType PtrCat( const unsigned short *, unsigned short * )
{
     return stTrueType();
}

inline stTrueType PtrCat( int *, int * )
{
     return stTrueType();
}

inline stTrueType PtrCat( const int *, int * )
{
     return stTrueType();
}

inline stTrueType PtrCat( unsigned int *, unsigned int * )
{
     return stTrueType();
}

inline stTrueType PtrCat( const unsigned int *, unsigned int * )
{
     return stTrueType();
}

inline stTrueType PtrCat( long *, long * )
{
     return stTrueType();
}

inline stTrueType PtrCat( const long *, long * )
{
     return stTrueType();
}

inline stTrueType PtrCat( unsigned long *, unsigned long * )
{
     return stTrueType();
}

inline stTrueType PtrCat( const unsigned long *, unsigned long * )
{
     return stTrueType();
}

inline stTrueType PtrCat( float *, float * )
{
     return stTrueType();
}

inline stTrueType PtrCat( const float *, float * )
{
     return stTrueType();
}

inline stTrueType PtrCat( double *, double * )
{
     return stTrueType();
}

inline stTrueType PtrCat( const double *, double * )
{
     return stTrueType();
}

inline stTrueType PtrCat( long double *, long double * )
{
     return stTrueType();
}

inline stTrueType PtrCat( const long double *, long double * )
{
     return stTrueType();
}

// nonconst_traits for typevalue,reference or pointer
template < typename T > struct nonconst_traits
{
	typedef T  value_type;
	typedef T& reference;
	typedef T* pointer;
};

template <class T> struct nonconst_traits<T*>
{
	typedef T  value_type;
	typedef T& reference;
	typedef T* pointer;
};

template <class T> struct nonconst_traits<T&>
{
	typedef T  value_type;
	typedef T& reference;
	typedef T* pointer;
};

// const_traits for typevalue,reference or pointer
template < typename T > struct const_traits
{
	typedef const T  value_type;
	typedef const T& reference;
	typedef const T* pointer;
};

template <class T> struct const_traits<T*>
{
	typedef const T  value_type;
	typedef const T& reference;
	typedef const T* pointer;
};

template <class T> struct const_traits<T&>
{
	typedef const T  value_type;
	typedef const T& reference;
	typedef const T* pointer;
};

END_STLNAMESPACE

#endif //__AILIB__TYPE_TRAITS_H__
