//=============================================================================
/**
* \file    algorithm.h
* \brief reference STL algorithm standard interface
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: algorithm.h,v 1.2 2009/03/16 07:39:22 daizh Exp $
*
* History
* 2008.02.22 first release by DZH
*/
//=============================================================================
#ifndef __AILIB__ALGORITHM_H__
#define __AILIB__ALGORITHM_H__

#include "algobase.h"
#include "type_traits.h"

START_STLNAMESPACE

const int g_stl_threshold = 16;

//-------------------------for insert sort-------------------------//
template< class TRandomAccessIter, class T, class TCmp >
void unguarded_linear_insert( TRandomAccessIter atLast, T atValue, TCmp atLess )
{
    TRandomAccessIter ltNext = atLast;
	--ltNext;

	//inner loop for insertion sort
	while( atLess( atValue, *ltNext ) )
	{
		*atLast = *ltNext;        //adjust value
		atLast = ltNext;          //adjust iterator
		--ltNext;                 //move next
	}
	*atLast = atValue;
}

template< class TRandomAccessIter, class T, class TCmp >
void linear_insert( TRandomAccessIter atFirst, TRandomAccessIter atLast, T*, TCmp atLess )
{
	T liValue = *atLast; //copy last element

	if ( atLess( liValue, *atFirst ) ) //if last is less than first, copy backward, that first is least anytime
	{
		copy_backward( atFirst, atLast, atLast + 1 );
		*atFirst = liValue;
	}
	else
	{
		unguarded_linear_insert( atLast, liValue, atLess );
	}
}

template< class TRandomAccessIter, class TCmp >
void insert_sort( TRandomAccessIter atFirst, TRandomAccessIter atLast, TCmp atLess )
{
	if ( atFirst == atLast )
	{
		return;
	}

	for( TRandomAccessIter liIt = atFirst + 1; liIt != atLast; ++liIt ) //outer loop
	{
		linear_insert( atFirst, liIt, ( typename iterator_traits<TRandomAccessIter>::value_type* )0, atLess );
	}
}

template< class TRandomAccessIter, class TCmp >
inline void unguarded_insertion_sort( TRandomAccessIter atFirst, TRandomAccessIter atLast, TCmp atLess )
{
	for ( TRandomAccessIter liIt = atFirst; liIt != atLast; ++liIt )
	{
		unguarded_linear_insert( liIt, typename iterator_traits<TRandomAccessIter>::value_type(*liIt), atLess );
	}
}

//------------------------for quick sort ---------------------//
//return median of three value
template< class T, class TCmp >
inline const T& median( const T& atA, const T& atB, const T& atC, TCmp atLess )
{
	if ( atLess( atA, atB ) )
	{
		if ( atLess( atB, atC )  ) // atA < atB < atC
		{
			return atB;
		}
		else if ( atLess( atA, atC )  ) // atA < atC < atB
		{
			return atC;
		}
		else // atC < atA < atB
		{
			return atA;
		}
	}
	else if ( atLess( atA, atC )  ) // atB < atA < atC
	{
		return atA;
	}
	else if ( atLess( atB, atC )  ) // atB < atC < atA
	{
		return atC;
	}
	else
	{
		return atB;
	}
}

template< class TRandomAccessIter, class T, class TCmp >
TRandomAccessIter unguarded_partition( TRandomAccessIter atFirst, TRandomAccessIter atLast, T atPivot, TCmp atLess )
{
    while( true )
	{
		while( atLess( *atFirst, atPivot ) ) ++atFirst; // unto first >= atPivot
		--atLast;

		while( atLess( atPivot, *atLast ) ) --atLast; //unto atLast <= atPivot

		if ( !( atFirst < atLast ) ) return atFirst; //interleaving

		AI_STD::swap( *atFirst, *atLast );

		++atFirst;
	}
}

template< class TRandomAccessIter, class TCmp >
void quick_sort_loop( TRandomAccessIter atFirst, TRandomAccessIter atLast, TCmp atLess )
{
	quick_sort_loop_aux( atFirst, atLast, ( typename iterator_traits<TRandomAccessIter>::value_type* )0, atLess );
}

template< class TRandomAccessIter, class T, class TCmp >
void quick_sort_loop_aux( TRandomAccessIter atFirst, TRandomAccessIter atLast, T*, TCmp atLess )
{
	while ( atLast - atFirst > g_stl_threshold )
	{
         //median-of-3 partition
		TRandomAccessIter ltCut = unguarded_partition( atFirst, atLast,
			T( median( *atFirst, *( atFirst + ( atLast - atFirst ) / 2 ), *( atLast -1 ), atLess ) ), atLess
		);

		if ( ltCut - atFirst >= atLast - ltCut )
		{
			quick_sort_loop( ltCut, atLast, atLess );
			atLast = ltCut;
		}
		else
		{
			quick_sort_loop( atFirst, ltCut, atLess );
			atFirst = ltCut;
		}
	}
}

template< class TRandomAccessIter, class TCmp >
void final_insertion_sort( TRandomAccessIter atFirst, TRandomAccessIter atLast, TCmp atLess )
{
	if ( atFirst - atLast > g_stl_threshold )
	{
		insert_sort( atFirst, atFirst + g_stl_threshold, atLess );
		unguarded_insertion_sort( atFirst + g_stl_threshold, atLast, atLess );
	}
	else
	{
		insert_sort( atFirst, atLast, atLess );
	}
}

template< class TRandomAccessIter, class TCmp >
void sort( TRandomAccessIter atFirst, TRandomAccessIter atLast, TCmp atLess )
{
	if ( !( atFirst == atLast ) )
	{
		quick_sort_loop( atFirst, atLast, atLess );
		final_insertion_sort( atFirst, atLast, atLess );
	}
}

template< class TRandomAccessIter >
void sort( TRandomAccessIter atFirst, TRandomAccessIter atLast )
{
	if ( !( atFirst == atLast ) )
	{
		quick_sort_loop( atFirst, atLast, __less() );
		final_insertion_sort( atFirst, atLast, __less() );
	}
}

template< class TRandomAccessIter, class T, class TCmp >
TRandomAccessIter lower_bound( TRandomAccessIter atFirst, TRandomAccessIter atLast, const T& atValue, const TCmp& atLess )
{
	int liLen = atLast - atFirst;
	int liHalf;
	TRandomAccessIter ltMiddle;
	while ( liLen > 0 )
	{
		liHalf = liLen >> 1;
		ltMiddle = atFirst + liHalf;
		if ( atLess( *ltMiddle, atValue ) )
		{
			atFirst = ltMiddle + 1;
			liLen = liLen - liHalf - 1;
		}
		else
		{
			liLen = liHalf;
		}
	}
	return atFirst;
}

template< class TRandomAccessIter, class T >
TRandomAccessIter lower_bound( TRandomAccessIter atFirst, TRandomAccessIter atLast, const T& atValue )
{
	return lower_bound( atFirst, atLast, atValue, __less() );
}

template< class TRandomAccessIter, class T, class TCmp >
TRandomAccessIter upper_bound( TRandomAccessIter atFirst, TRandomAccessIter atLast, const T& atValue, const TCmp& atLess )
{
	int liLen = atLast - atFirst;
	int liHalf;
	TRandomAccessIter ltMiddle;
	while ( liLen > 0 )
	{
		liHalf = liLen >> 1;
		ltMiddle = atFirst + liHalf;
		if ( atLess( atValue, *ltMiddle ) )
		{
			liLen = liHalf;
		}
		else
		{
			atFirst = ltMiddle + 1;
			liLen = liLen - liHalf - 1;
		}
	}
	return atFirst;
}

template< class TRandomAccessIter, class T >
TRandomAccessIter upper_bound( TRandomAccessIter atFirst, TRandomAccessIter atLast, const T& atValue )
{
	return upper_bound( atFirst, atLast, atValue, __less() );
}

/////////////////////////////////////////// search ///////////////////////////////////////////////////
template< class TRandomAccessIter, class T >
bool binary_search( TRandomAccessIter atFirst, TRandomAccessIter atLast, const T& atValue )
{
	TRandomAccessIter liIt = lower_bound( atFirst, atLast, atValue, __less() );
	return liIt != atLast && !( atValue < *liIt );
}

template< class TRandomAccessIter, class T, class TCmp >
bool binary_search( TRandomAccessIter atFirst, TRandomAccessIter atLast, const T& atValue, const TCmp& atLess )
{
	TRandomAccessIter liIt = lower_bound( atFirst, atLast, atValue, atLess );
	return liIt != atLast && !( atLess( atValue, *liIt ) );
}

/////////////////////////////////////////// find ///////////////////////////////////////////////////
template <class TInputIter, class T>
inline TInputIter find( TInputIter atFirst, TInputIter atLast, const T& atVal )
{
    while ( atFirst != atLast && !(*atFirst == atVal) )
        ++atFirst;
    return atFirst;
}

template <class TInputIter, class TPredicate>
inline TInputIter find_if( TInputIter atFirst, TInputIter atLast, TPredicate afPred )
{
    while (atFirst != atLast && !afPred(*atFirst))
        ++atFirst;
    return atFirst;
}

template <class TInputIter>
TInputIter adjacent_find( TInputIter atFirst, TInputIter atLast )
{
    return adjacent_find( atFirst, atLast, __Equal() );
}

template <class TInputIter, class TPredicate >
TInputIter adjacent_find( TInputIter atFirst, TInputIter atLast, const TPredicate& afPred )
{
    if ( atFirst == atLast )
    {
        return atLast;
    }
    
    TInputIter ltNext = atFirst;
    while(++ltNext != atLast)
    {
        if (afPred(*atFirst, *ltNext))
        {
            return atFirst;
        }
        
        atFirst = ltNext;
    }
    
    return atLast;
}

template <class TInputIter>
inline TInputIter unique( TInputIter atFirst, TInputIter atLast )
{
    atFirst = adjacent_find(atFirst, atLast);
    return unique_copy( atFirst, atLast, atFirst );
}

template <class TInputIter, class TPredicate>
inline TInputIter unique( TInputIter atFirst, TInputIter atLast, TPredicate afPred )
{
    atFirst = adjacent_find( atFirst, atLast, afPred );
    return unique_copy( atFirst, atLast, atFirst, afPred );
}

END_STLNAMESPACE

#endif //__AILIB__ALGORITHM_H__
