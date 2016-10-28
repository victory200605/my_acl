//=============================================================================
/**
 * \file    contain_base.h
 * \brief reference STL contain_base standard interface
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: contain_base.h,v 1.1 2008/11/04 03:50:19 fzz Exp $
 *
 * History
 * 2008.02.22 first release by DZH
 */
//=============================================================================
#ifndef __AILIB__CONTAIN_BASE_H__
#define __AILIB__CONTAIN_BASE_H__

#include <assert.h>
#include <stdio.h>

#include "config.h"

START_STLNAMESPACE

template < class T, class Alloc > 
class contain_base
{
public:
     T* allocate_block( size_t aiN )
     {
          return ((Alloc*)0)->allocate( aiN );
     }

     virtual ~contain_base() {}
     
	 void deallocate_block( T* cptPtr, size_t aiN )
     {
          if ( cptPtr != NULL ) 
          {
               ((Alloc*)0)->deallocate( cptPtr, aiN ); 
          }
          cptPtr = NULL;
     }
	
     virtual void Xlen() const 
     { 
          bool length_error = false;
          assert( length_error == true );
     };	// report a length_error

     virtual void Xran() const 
     {
          bool out_of_range = false;
          assert( out_of_range == true );
     };	// report an out_of_range error
};

END_STLNAMESPACE

#endif //__AILIB__CONTAIN_BASE_H__

