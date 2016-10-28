//=============================================================================
/**
 * \file    iterator_base.h
 * \brief reference STL iterator_base standard interface
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: iterator_base.h,v 1.1 2008/11/04 03:50:19 fzz Exp $
 *
 * History
 * 2008.02.28 first release by DZH
 */
//=============================================================================
#ifndef __AILIB__ITERATOR_BASE_H__
#define __AILIB__ITERATOR_BASE_H__

#include "config.h"

START_STLNAMESPACE

template <class TIterator>
struct iterator_traits {
     typedef typename TIterator::value_type        value_type;
     typedef typename TIterator::difference_type   difference_type;
     typedef typename TIterator::pointer           pointer;
     typedef typename TIterator::reference         reference;
};

template <class Tp>
struct iterator_traits<const Tp*> {
     typedef Tp                          value_type;
     typedef unsigned long               difference_type;
     typedef const Tp*                   pointer;
     typedef const Tp&                   reference;
};

template <class Tp>
struct iterator_traits<Tp*> {
     typedef Tp                          value_type;
     typedef unsigned long               difference_type;
     typedef Tp*                         pointer;
     typedef Tp&                         reference;
};

END_STLNAMESPACE

#endif //__AILIB__ITERATOR_BASE_H__
