//=============================================================================
/**
 * \file    algobase.h
 * \brief reference STL algobase standard interface
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: algobase.h,v 1.5 2010/11/18 04:13:34 daizh Exp $
 *
 * History
 * 2008.02.29 first release by DZH
 */
//=============================================================================
#ifndef __AILIB__ALGOBASE_H__
#define __AILIB__ALGOBASE_H__

#include <string.h>

#include "config.h"
#include "type_traits.h"

START_STLNAMESPACE

#define ai_max(a,b)    (((a) > (b)) ? (a) : (b))
#define ai_min(a,b)    (((a) < (b)) ? (a) : (b))

struct __less
{
	template< class T >
	bool operator () ( const T& atLeft, const T& atRight ) const
	{
		return atLeft < atRight;
	}
};

struct __Equal
{
	template< class T >
	bool operator () ( const T& atLeft, const T& atRight ) const
	{
		return atLeft == atRight;
	}
};

//---------------------- fill -----------------------//
template <class TForwardIter, class Tp>
inline void fill( TForwardIter atFirst, TForwardIter atLast, const Tp& atVal ) 
{
    assert( atLast >= atFirst );
    for ( ; atFirst != atLast; ++atFirst )
    {
        *atFirst = atVal;
    }
}

template <class TOutputIter, class TSize, class Tp>
inline TOutputIter fill_n( TOutputIter atFirst, TSize aiN, const Tp& atVal ) 
{
    for ( ; aiN > 0; --aiN, ++atFirst)
    {
         *atFirst = atVal;
    }  
    return atFirst;
}

// specialise for char*
inline void fill( unsigned char* atFirst, unsigned char* atLast,
               const unsigned char& atVal ) 
{
    unsigned char lcTmp = atVal;
    memset( atFirst, lcTmp, atLast - atFirst );
}

inline void fill( signed char* atFirst, signed char* atLast,
               const signed char& atVal ) 
{
    signed char lcTmp = atVal;
    memset(atFirst, (unsigned char)lcTmp, atLast - atFirst);
}

inline void fill( char* atFirst, char* atLast, const char& atVal ) 
{
    char lcTmp = atVal;
    memset(atFirst, (unsigned char)lcTmp, atLast - atFirst);
}

inline unsigned char* fill_n( unsigned char* atFirst, size_t aiN,
                        const unsigned char& atVal ) 
{
    fill(atFirst, atFirst + aiN, atVal);
    return atFirst + aiN;
}

inline signed char* fill_n( signed char* atFirst, size_t aiN,
                       const signed char& atVal ) 
{
    fill(atFirst, atFirst + aiN, atVal);
    return atFirst + aiN;
}

inline char* fill_n( char* atFirst, size_t aiN, const char& atVal ) 
{
    fill(atFirst, atFirst + aiN, atVal);
    return atFirst + aiN;
}

//------------------------------- copy ------------------------------------//
template <class TInputIter, class TOutputIter>
inline TOutputIter copy_backward( TInputIter atFirst, TInputIter atLast, TOutputIter atResult ) 
{
    return copy_backward( atFirst, atLast, atResult, PtrCat( atFirst, atResult ) );
}

template <class TInputIter, class TOutputIter>
inline TOutputIter copy_backward( TInputIter atFirst, TInputIter atLast, TOutputIter atResult,
                             const stFalseType & ) 
{
    while ( atFirst != atLast )
    {
        *--atResult = *--atLast;
    }
    return atResult;
}

template <class TInputIter, class TOutputIter>
inline TOutputIter copy_backward( TInputIter atFirst, TInputIter atLast, TOutputIter atResult,
                             const stTrueType & ) 
{
    const size_t liNum = (const char*)atLast - (const char*)atFirst;
    return ( liNum > 0 ) ? (TOutputIter)((char*)memmove( (char*)atResult - liNum, atFirst, liNum ) - liNum) : atResult;
}

template <class TInputIter, class TOutputIter >
inline TOutputIter unique_copy( TInputIter atFirst, TInputIter atLast, TOutputIter atResult ) 
{
    if (atFirst == atLast) return atLast;

    return unique_copy( atFirst, atLast, atResult, __Equal() );
}

template <class TInputIter, class TOutputIter, class TPredicate >
inline TOutputIter unique_copy( TInputIter atFirst, TInputIter atLast, TOutputIter atResult,
    const TPredicate& afPred ) 
{
    *atResult = *atFirst;
    while ( ++atFirst != atLast )
    {
        if (!afPred(*atResult, *atFirst)) *++atResult = *atFirst;
    }
    return ++atResult;
}

template <class TInputIter, class TOutputIter>
inline TOutputIter copy( TInputIter atFirst, TInputIter atLast, TOutputIter atResult ) 
{
    return copy( atFirst, atLast, atResult, PtrCat( atFirst, atResult ) );
}

template < class TInputIter, class TOutputIter >
inline TOutputIter copy( TInputIter atFirst, TInputIter atLast, TOutputIter atResult,
                    const stFalseType &  )
{
    for ( ; atFirst != atLast; ++atResult, ++atFirst)
    {
        *atResult = *atFirst;
    }
    return atResult;
}

template <class TInputIter, class TOutputIter>
inline TOutputIter copy( TInputIter atFirst, TInputIter atLast, TOutputIter atResult,
                    const stTrueType & ) 
{
    const size_t liNum = (const char*)atLast - (const char*)atFirst;
    return ( liNum > 0 ) ? (TOutputIter)((char*)memmove( (char*)atResult, atFirst, liNum ) + liNum) : atResult ;
}

template <class TInputIter, class TOutputIter>
inline TOutputIter copy( TOutputIter atDest, TInputIter atSrc, size_t aiN )
{
    return copy( atDest, atSrc, aiN, PtrCat( atSrc, atDest ) );
}

template <class TInputIter, class TOutputIter>
inline TOutputIter copy( TOutputIter atDest, TInputIter atSrc, size_t aiN, const stFalseType & )
{
    for ( ; aiN > 0; --aiN, ++atSrc, ++atDest )
    {
        *atDest = *atSrc;
    }  
    return atDest;
}
template <class TInputIter, class TOutputIter>
inline TOutputIter copy( TOutputIter atDest, TInputIter atSrc, size_t aiN, const stTrueType & )
{
    return ( aiN > 0 ) ? ( (char*)memmove( atDest, atSrc, aiN ) + aiN ) : atDest;
}

//--------------------------------- swap --------------------------------//
template<class T> inline void swap( T& apsL, T& apsR )
{
    T lptTmp = apsR;
    apsR = apsL;
    apsL = lptTmp;
}

//--------------------------------- compare ------------------------------//
inline int compare( const char* apsF1, size_t aiN1, const char* apsF2, size_t aiN2 )
{
    size_t liN = ai_min(aiN1, aiN2);
    int liCmp = strncmp(apsF1, apsF2, liN);
    return liCmp != 0 ? liCmp : (aiN1 < aiN2 ? -1 : aiN1 > aiN2 ? 1 : 0);
}

//------------------------------ move --------------------------------//
inline char* move( char * apsDest, const char * apsSrc, size_t aiN )
{ 
    return ( (char *)::memmove(apsDest, apsSrc, aiN) + aiN );
}

//------------------------------ move --------------------------------//
inline char * assign( char * apsFirst, size_t aiN, char acCh )
{ 
    return ( (char *)::memset(apsFirst, acCh, aiN) + aiN );
}

END_STLNAMESPACE

#endif //__AILIB__ALGOBASE_H__

