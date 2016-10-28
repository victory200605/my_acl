//=============================================================================
/**
 * \file    Singleton.h
 * \brief
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: Singleton.h,v 1.1 2009/02/23 03:48:19 daizh Exp $
 *
 * History
 * 2008.01.27 first release by DZH
 */
//=============================================================================

#ifndef __AI_SINGLETION_H__
#define __AI_SINGLETION_H__

#include <stdlib.h>

/**
* singleton for all object which construct was protected 
*/
template< class T >
class Singleton
{
public:
     static T*   Instance();
     static void Release();

protected:
     static T* cpoInstance;
};

/**
* initialize static pointer
*/
template< class T > T* Singleton< T >::cpoInstance = NULL;

/**
* instance object for static pointer and return reference
*/
template< class T > T* Singleton< T >::Instance()
{ 
     if ( cpoInstance == NULL )
     {
          return cpoInstance = new T;
     }
     return cpoInstance;
}

template< class T > void Singleton< T >::Release()
{
    delete cpoInstance;
	cpoInstance = NULL;
}

#endif
