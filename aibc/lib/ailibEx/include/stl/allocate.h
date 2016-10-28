//=============================================================================
/**
 * \file    Allocate.h
 * \brief reference STL allocate standard interface
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: allocate.h,v 1.1 2008/11/04 03:50:19 fzz Exp $
 *
 * History
 * 2008.02.22 first release by DZH
 */
//=============================================================================
#ifndef __AILIB__ALLOCATE_H__
#define __AILIB__ALLOCATE_H__

#include <assert.h>
#include <new>
#include "config.h"

START_STLNAMESPACE

template< class T >
inline T* _allocate( size_t aiN, T* )
{
     T* lptTmp = (T*)(::operator new((size_t) ( aiN * sizeof(T) ) ));
     assert ( lptTmp != NULL );
     return lptTmp;
}

template< class T >
inline void _deallocate( T* aptPtr, size_t aiN )
{
     ::operator delete( aptPtr );
}

template< class T1, class T2 >
inline void _construct( T1* aptPtr, const T2& atValue )
{
     new (aptPtr) T1( atValue );
}

template< class T >
inline void _destroy( T* aptPtr )
{
     aptPtr->~T();
}

// allocator for AI_STL, just overwrite new/delete operator
template< class T >
class allocator
{
public:
     typedef T          value_type;
     typedef T*         pointer;
     typedef const T*   const_pointer;
     typedef T&         reference;
     typedef const T&   const_reference;
     typedef size_t     size_type;

public:

public:
     pointer allocate( size_type aiN )
     {
          return _allocate( aiN, (pointer)0 );
     }
     
     void deallocate( pointer aptPtr, size_type aiN )
     {
          _deallocate( aptPtr, aiN );
     }
     
     void construct( pointer aptPtr, const_reference atValue )
     {
          _construct( aptPtr, atValue );
     }
     
     void destroy( pointer aptPtr )
     {
          _destroy( aptPtr );
     }

     pointer address( reference atX )    { return &atX; }
};

END_STLNAMESPACE

#endif //__AILIB__ALLOCATE_H__

