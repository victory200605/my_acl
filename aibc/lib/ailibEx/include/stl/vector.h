//=============================================================================
/**
 * \file    vector.h
 * \brief reference STL vector standard interface
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: vector.h,v 1.2 2010/04/08 14:46:14 daizh Exp $
 *
 * History
 * 2008.02.28 first release by DZH
 */
//=============================================================================
#ifndef __AILIB__VECTOR_H__
#define __AILIB__VECTOR_H__

#include "contain_base.h"
#include "allocate.h"
#include "algobase.h"

START_STLNAMESPACE

// vector base allocate and deallocate block
template < class T, class Alloc > 
class vector_base : public contain_base<T, Alloc>
{
public:
    typedef contain_base<T, Alloc> base;
public:
    vector_base( )
        : cptStart(0), cptFinish(0), cptLast(0) 
    {
    }

    vector_base( size_t aiN )
        : cptStart(0), cptFinish(0), cptLast(0)
    {
        cptStart = base::allocate_block( aiN );
        cptFinish = cptStart;
        cptLast = cptStart + aiN;
    }
    //deallocate
    void deallocate_block( T* cptPtr, size_t aiN )
    {
        base::deallocate_block( cptPtr, aiN );
    }

    void deallocate_block( )
    {
        base::deallocate_block( cptStart, cptLast - cptStart );
        cptStart = NULL;
        cptFinish = NULL;
        cptLast = NULL;
    }

    ~vector_base() 
    { 
        deallocate_block( );
    }
            
protected:
    T* cptStart;  //iterator point to block header
    T* cptFinish; //iterator point to block water position
    T* cptLast;   //iterator point to end of block
};

template< class T, class Alloc = allocator<T> >
class vector : public vector_base< T, Alloc >
{
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;

public:
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef size_t size_type;
    typedef unsigned long difference_type;

    typedef vector_base< T, Alloc > base;
    typedef vector< T, Alloc > self;

public:
	   //construct
    vector( )   {}

    vector( size_type aiN, const T& atValue )
    {
        fill_initialize( aiN, atValue );
    }

    vector( size_type aiN )
    {
        fill_initialize( aiN, T() );
    }

    vector( const_iterator atFirst, const_iterator atLast ) 
        :  base( atLast - atFirst )
    {
    	    // [atFirst,atLast] ininitialize copy vector
        this->cptFinish = uninitialized_copy( atFirst, atLast, this->cptStart ); 
    }
    
    vector( const self& aoV ) 
		 :  base( aoV.cptFinish - aoV.cptStart )
	 {
		 // [atFirst,atLast] ininitialize copy vector
		 this->cptFinish = uninitialized_copy( aoV.cptStart, aoV.cptFinish, this->cptStart ); 
	 }
	 
    ~vector( )
    {
        destroy( this->cptStart,this->cptFinish );
    }

public:
    reference front( )  { return *begin(); } //return first element

    reference back( )   { return *( end() - 1 ); } //return last element

    void push_back( const T& atX ); //push back element

    void pop_back( )
    {
    	//pop a element
        --this->cptFinish;
        destroy( this->cptFinish );
    }

    void insert( iterator atPosition, size_type aiN, const T& atX )
    {
    	// insert aiN * atX in atPosition
        fill_insert( atPosition, aiN, atX );
    }

    void insert( iterator atPosition, const_iterator atFirst, const_iterator atLast )
    {
    	// insert [atFirst, atLast] region in atPosition
        range_insert( atPosition, atFirst, atLast );
    }

    void assign( size_type aiN, const T& atVal ) 
    {
    	// assign aiN * atVal
        fill_assign( aiN, atVal ); 
    }

    void assign( const_iterator atFirst, const_iterator atLast )
    {
    	// assign [atFirst, atLast] region in atPosition
        range_assign( atFirst, atLast );
    }

    void reserve( size_type aiN ) ;
    
    void resize( size_type aiNewSize, T atX ) 
    {
        if ( aiNewSize < size() ) 
            erase( begin() + aiNewSize, end() );
        else
            insert( end(), aiNewSize - size(), atX );
    }
    
    void resize( size_type aiNewSize ) 
    {
        resize( aiNewSize, T() );
    }

    iterator erase( iterator atPosition );

    iterator erase( iterator atFirst, iterator atLast );

    void clear()
    { 
        erase( begin(), end() );
    }

    void swap( vector<T>& atX ) 
    {
        AI_STD::swap( this->cptStart, atX.cptStart );
        AI_STD::swap( this->cptFinish, atX.cptFinish );
        AI_STD::swap( this->cptLast, atX.cptLast );
    }

//over write operator
public:
    self& operator = ( const self& aoRight )
    {
        range_assign( aoRight.begin(), aoRight.end() );
        return *this;
    }

public:
    iterator begin( )                  { return this->cptStart; } // begin
    iterator end( )                    { return this->cptFinish; }
    const_iterator begin( ) const      { return this->cptStart; }
    const_iterator end( ) const        { return this->cptFinish; }

    size_type size()                   { return this->cptFinish - this->cptStart; }
    const size_type size() const       { return this->cptFinish - this->cptStart; }
    size_type capacity( )              { return this->cptLast - this->cptStart; }
    bool empty( ) const                { return begin() == end(); }

    reference operator[] ( size_type aiN )                { return *(begin() + aiN); }
    const_reference operator[] ( size_type aiN ) const    { return *(begin() + aiN); }

    reference at( size_type aiN ) 
    { 
        if ( aiN >= size() )
        {
            base::Xran( );  //out of range
        }
        return (*this)[aiN];
    }
    
    const_reference at( size_type aiN ) const 
    { 
        if ( aiN >= size() )
        {
            base::Xran( );  //out of range
        }
        return (*this)[aiN]; 
    }

private:
    void clear_block( ) 
    {
        if (this->cptStart) 
        {
            // clear block
            destroy( this->cptStart, this->cptFinish );
            this->deallocate_block( );
        }
    }
    
    void set( pointer aptStart, pointer aptFinish, pointer aptLast ) 
    {
        this->cptStart = aptStart;
        this->cptFinish = aptFinish;
        this->cptLast = aptLast;
    }
    
    bool inside( const_iterator atPosition )
    {
        //if atPosition is inside iterator
        if ( atPosition < this->cptStart || atPosition > this->cptFinish )
        {
            return false;
        }
        else return true;
    }
    
    void fill_initialize( size_type aiN, const T& atValue )
    {
        //allocate and fill block
        this->cptStart = allocate_and_fill( aiN, atValue );
        this->cptFinish = this->cptStart + aiN;
        this->cptLast = this->cptFinish;
    }

    iterator allocate_and_fill( size_type aiN, const T& atX )
    {
        //allocate
        iterator ltResult = this->allocate_block( aiN );

        //fill
        uninitialized_fill_n( ltResult, aiN, atX );

        return ltResult;
    }

    iterator allocate_and_copy( size_type aiN, const_iterator atFirst, const_iterator atLast )
    {
        //allocate
        iterator ltResult = this->allocate_block( aiN );

        //fill
        uninitialized_copy( atFirst, atLast, ltResult );

        return ltResult;
    }

    void fill_insert( iterator atPosition, size_type aiN, const T& atX );

    void insert_overflow( iterator atPosition, size_type aiN, const T& atX );

    void insert_overflow( iterator atPosition, const_iterator atFirst, const_iterator atLast );

    void range_insert( iterator atPosition, const_iterator atFirst, const_iterator atLast );

    void inside_range_insert( iterator atPosition, const_iterator atFirst, const_iterator atLast );

    void fill_assign( size_type aiN, const T& atVal );

    void range_assign( const_iterator atFirst, const_iterator atLast );

    void destroy( iterator atFirst, iterator atLast ) 
    {
        for ( ; atFirst != atLast; ++atFirst )
        {
            _destroy(&*atFirst);
        }
    }

	 void destroy( iterator atPostion )
	 {
		 _destroy(&*atPostion);
	 }

    void construct( pointer aptPtr, const_reference atValue )
    {
        _construct( aptPtr, atValue);
    }
    
    void uninitialized_fill( iterator atFirst, iterator atLast, 
                        const_reference atX, const stTrueType& ) 
    {
        AI_STD::fill( atFirst, atLast, atX );
    }
    
    void uninitialized_fill( iterator atFirst, iterator atLast, 
                        const_reference atX, const stFalseType& ) 
    {
        iterator loCur = atFirst;
        try {
            for ( ; loCur != atLast; ++loCur )
            {
                construct( &*loCur, atX );
            }
        }
        catch( ... )
        {
            destroy(atFirst, loCur);
        }
    }
    
    void uninitialized_fill( iterator atFirst, iterator atLast,  const_reference atX ) 
    {
        uninitialized_fill( atFirst, atLast, atX, PtrCat( atFirst, atFirst ) );
    }
    
    iterator uninitialized_fill_n( iterator atFirst, size_type atN,
                             const_reference atX, const stTrueType&) 
    {
      return AI_STD::fill_n( atFirst, atN, atX );
    }
    
    iterator uninitialized_fill_n( iterator atFirst, size_type atN,
                                const_reference atX, const stFalseType& )
    {
        iterator ltCur = atFirst;
        try 
        {
            for ( ; atN > 0; --atN, ++ltCur )
            {
                construct( &*ltCur, atX );
            }
            return ltCur;
        }
        catch( ... )
        {
            destroy( atFirst, ltCur );
        }
		  return atFirst;
    }
    
    iterator uninitialized_fill_n( iterator atFirst, size_type atN, const_reference atX ) 
    {
        return uninitialized_fill_n( atFirst, atN, atX, PtrCat( atFirst, atFirst ) );
    }
    
    iterator uninitialized_copy( const_iterator atFirst, const_iterator atLast, iterator atResult,
                            const stTrueType& ) 
    {
        return ( atLast == atFirst ) ? atResult : AI_STD::copy( atFirst, atLast, atResult, stTrueType() );
    }
    
    iterator uninitialized_copy( const_iterator atFirst, const_iterator atLast, iterator atResult,
                            const stFalseType& )
    {
        iterator ltCur = atResult;
        try 
        {
            for ( ; atFirst != atLast; ++atFirst, ++ltCur )
                construct( &*ltCur, *atFirst );
            return ltCur;
        }
        catch( ... )
        {
            destroy( atResult, ltCur );
        }
        return ltCur;
    }
    
    iterator uninitialized_copy( const_iterator atFirst, const_iterator atLast, iterator atResult ) 
    {
        return uninitialized_copy( atFirst, atLast, atResult,  PtrCat(atFirst, atResult) );
    }
};

//implement
template< class T, class Alloc >
void vector<T,Alloc>::push_back( const T& atX ) //push back element
{
    if ( this->cptFinish != this->cptLast )
    {
        //construct element in standby block
        construct( this->cptFinish, atX );
        ++this->cptFinish;
    }
    else
    {
        //not enough standbu block
        fill_insert( end(), 1, atX );
    }
}

template< class T, class Alloc >
void vector<T,Alloc>::reserve( size_type aiN ) 
{
    if ( capacity() < aiN ) 
    {
        const size_type liOldSize = size();
        pointer lptTmp = NULL;
        if ( this->cptStart )
        {
            //reallocate block and initialize copy 
            lptTmp = allocate_and_copy( aiN, this->cptStart, this->cptFinish );
            clear_block( );
        } 
        else 
        {
            //reallocate block
            lptTmp = this->allocate_block( aiN );
        }
        //set pointer
        set( lptTmp, lptTmp + liOldSize, lptTmp + aiN );
    }
}

template< class T, class Alloc >
typename vector<T,Alloc>::iterator vector<T,Alloc>::erase( iterator atPosition ) 
{
    if ( atPosition + 1 != end() )
    {
        //forward copy [atPosition+1, end()] region element
        AI_STD::copy( atPosition + 1, this->cptFinish, atPosition );
    }
    --this->cptFinish;
    destroy( this->cptFinish ); //recycle
    return atPosition;
}


template< class T, class Alloc >
typename vector<T,Alloc>::iterator vector<T,Alloc>::erase( iterator atFirst, iterator atLast ) 
{
    //forward copy [atLast+1, end()] region element
    pointer lpNewFinish = AI_STD::copy( atLast, this->cptFinish, atFirst );
    destroy( lpNewFinish, this->cptFinish ); //recycle element of end
    this->cptFinish = lpNewFinish;
    return atFirst;
}

template< class T, class Alloc >
void vector<T,Alloc>::fill_insert( iterator atPosition, size_type aiN, const T& atX ) 
{
    //fill the insert region
    if ( aiN > 0 ) 
    {
        if ( size_type(this->cptLast - this->cptFinish) >= aiN ) 
        {
            T ltXCopy = atX; //copy element, ensure uninside element
            const size_type liElemsAfter = this->cptFinish - atPosition;
            iterator lptOldFinish = this->cptFinish;
            if ( liElemsAfter > aiN )
            {
                //ininitialize copy aiN * element to standby region
                //-------------------------------------------
                //                     |-aiN-|
                // |________________|____|__|__|__V__|___|
                //               p        f        l
                //--------------------------------------------
                uninitialized_copy( this->cptFinish - aiN, this->cptFinish, this->cptFinish );
                this->cptFinish += aiN;
                //backward copy element
                //-------------------------------------------
                //                 |----|
                // |________________|__|__|_V__|_____|___|
                //               p        f        l
                //--------------------------------------------
                copy_backward( atPosition, lptOldFinish - aiN, lptOldFinish );
                //use target value fill region
                AI_STD::fill( atPosition, atPosition + aiN, ltXCopy );
            }
            else 
            {
                //fill (aiN - liElemsAfter)*element to cptFinish
                uninitialized_fill_n( this->cptFinish, aiN - liElemsAfter, ltXCopy );
                this->cptFinish += aiN - liElemsAfter;
                //initialize copy region [atPosition, lptOldFinish] to (cptFinish + aiN - liElemsAfter)
                uninitialized_copy( atPosition, lptOldFinish, this->cptFinish );
                this->cptFinish += liElemsAfter;
                //use target value fill region
                AI_STD::fill( atPosition, lptOldFinish, ltXCopy );
            }
        }
        else 
        {
            //not enough block
            insert_overflow( atPosition, aiN, atX );
        }
    }
}

template< class T, class Alloc >
void vector<T,Alloc>::insert_overflow( iterator atPosition, size_type aiN, const T& atX )
{
    const size_type liOldSize = size();
    const size_type liLen = liOldSize + ai_max( liOldSize, aiN );

    iterator ltNewStart = this->allocate_block( liLen );
    iterator ltNewFinish = ltNewStart;

    try
    {
        //copy element to another big blcok
        ltNewFinish = uninitialized_copy( this->cptStart, atPosition, ltNewStart );
        ltNewFinish = uninitialized_fill_n( ltNewFinish, aiN, atX );
        ltNewFinish = uninitialized_copy( atPosition, this->cptFinish, ltNewFinish );
    }
    catch ( ... )
    {
        //exception rallback
        destroy( ltNewStart, ltNewFinish );
        this->deallocate_block( ltNewStart, ltNewFinish - ltNewStart );
        throw;
    }

    clear_block( );
    set( ltNewStart, ltNewFinish, ltNewStart + liLen );
}

template< class T, class Alloc >
void vector<T,Alloc>::insert_overflow( iterator atPosition, const_iterator atFirst, const_iterator atLast )
{
    const size_type liN = atLast - atFirst;
    const size_type liOldSize = size();
    const size_type liLen = liOldSize + ai_max( liOldSize, liN );

    iterator ltNewStart = this->allocate_block( liLen );
    iterator ltNewFinish = ltNewStart;

    try
    {
        ltNewFinish = uninitialized_copy( this->cptStart, atPosition, ltNewStart );
        ltNewFinish = uninitialized_copy( atFirst, atLast, ltNewFinish );
        ltNewFinish = uninitialized_copy( atPosition, this->cptFinish, ltNewFinish );
    }
    catch ( ... )
    {
        destroy( ltNewStart, ltNewFinish );
        this->deallocate_block( ltNewStart, ltNewFinish - ltNewStart );
        throw;
    }

    clear_block( );
    set( ltNewStart, ltNewFinish, ltNewStart + liLen );
}

template< class T, class Alloc >
void vector<T,Alloc>::range_insert( iterator atPosition, const_iterator atFirst, const_iterator atLast )
{
    //insert region [atFirst, atLast] element to atPosition
    if ( inside( atFirst ) )
    {
        //[atFirst, atLast] is inside region
        inside_range_insert( atPosition, atFirst, atLast );
        return;
    }
    const size_type liN = atLast - atFirst;
    if ( liN > 0 ) 
    {
        if ( size_type(this->cptLast - this->cptFinish) >= liN ) 
        {
            const size_type liElemsAfter = this->cptFinish - atPosition;
            iterator lptOldFinish = this->cptFinish;
            if ( liElemsAfter > liN )
            {
                //ininitialize copy aiN * element to standby region
                //-------------------------------------------
                //                     |-aiN-|
                // |________________|____|__|__|__V__|___|
                //               p        f        l
                //--------------------------------------------
                uninitialized_copy( this->cptFinish - liN, this->cptFinish, this->cptFinish );
                this->cptFinish += liN;
                //backward copy element
                //-------------------------------------------
                //                 |----|
                // |________________|__|__|_V__|_____|___|
                //               p        f        l
                //--------------------------------------------
                copy_backward( atPosition, lptOldFinish - liN, lptOldFinish );
                //use target value fill region
                AI_STD::copy( atFirst, atLast, atPosition ); 
            }
            else 
            {
                //fill [ltMid, atLast] region element to cptFinish
                const_iterator ltMid = atFirst + liElemsAfter;
                uninitialized_copy( ltMid, atLast, this->cptFinish );
                this->cptFinish += liN - liElemsAfter;
                //copy [atPosition, lptOldFinish] region element to cptFinish + liN - liElemsAfter
                uninitialized_copy( atPosition, lptOldFinish, this->cptFinish );
                this->cptFinish += liElemsAfter;
                //copy
                AI_STD::copy( atFirst, ltMid, atPosition );
            }
        }
        else 
        {
            //not enough standby block
            insert_overflow( atPosition, atFirst, atLast );
        }
    }
}

template< class T, class Alloc >
void vector<T,Alloc>::inside_range_insert( iterator atPosition, const_iterator atFirst, const_iterator atLast )
{
    //inside range insert
    if ( atLast > this->cptFinish )
    {
        atLast = this->cptFinish; //trim atLast to cptFinish
    }
    const size_type liN = atLast - atFirst;
    if ( liN > 0 ) 
    {
        if ( size_type(this->cptLast - this->cptFinish) >= liN ) 
        {
            const size_type liElemsAfter = this->cptFinish - atPosition;
            iterator lptOldFinish = this->cptFinish;
            if ( liElemsAfter > liN )
            {
                //move [atPosition,cptFinish] region element first
                uninitialized_copy( this->cptFinish - liN, this->cptFinish, this->cptFinish );
                this->cptFinish += liN;
                copy_backward( atPosition, lptOldFinish - liN, lptOldFinish );

                if ( atFirst >= atPosition )
                {
                    //[atFirst, atLast] in [atPosition, aptFinish], need to offset
                    AI_STD::copy( atFirst + liN, atLast + liN, atPosition );
                }
                else if ( atFirst < atPosition && atLast >= atPosition )
                {
                    const size_type liNum = atPosition - atFirst;
                    //[atFirst, atFirst + ( atPosition - atFirst )] region, do not offset
                    AI_STD::copy( atFirst, atFirst + ( atPosition - atFirst ), atPosition );
                    //[atFirst + ( atPosition - atFirst ), atLast] region, need to offset
                    AI_STD::copy( atFirst + liNum+ liN, atLast + liN, atPosition + liNum );
                }
                else
                {
                    //[atFirst, atLast] out of [atPosition, aptFinish], do not offset
                    AI_STD::copy( atFirst, atLast, atPosition );
                }
            }
            else 
            {
                //atFirst out of [atPosition, aptFinish]
                const_iterator ltMid = atFirst + liElemsAfter;
                uninitialized_copy( ltMid, atLast, this->cptFinish );
                this->cptFinish += liN - liElemsAfter;
                uninitialized_copy( atPosition, lptOldFinish, this->cptFinish );
                this->cptFinish += liElemsAfter;

                if ( ltMid >= atPosition )
                {
                    const size_type liNum = atPosition - atFirst;
                    //[atFirst, atFirst + liNum] region, do not offset
                    AI_STD::copy( atFirst, atFirst + liNum, atPosition );
                    //[atFirst + liNum, ltMid] region, need to offset
                    AI_STD::copy( atFirst + liNum+ liN, ltMid + liN, atPosition + liNum );
                }
                else
                {
                    //[atFirst, ltMid] in [atPosition,aptFinish]
                    AI_STD::copy( atFirst, ltMid, atPosition );
                }
            }
        }
        else 
        {
            //not enough block
            insert_overflow( atPosition, atFirst, atLast );
        }
    }
}

template< class T, class Alloc >
void vector<T,Alloc>::fill_assign( size_type aiN, const T& atVal )
{
    if ( aiN > capacity() ) 
    {
        //construct and swap object
        vector<T> lvTmp( aiN, atVal );
        lvTmp.swap(*this);
    }
    else if ( aiN > size() ) 
    {
        AI_STD::fill( begin(), end(), atVal );
        this->cptFinish = uninitialized_fill_n( this->cptFinish, aiN - size(), atVal );
    }
    else
    {
        erase( fill_n( begin(), aiN, atVal ), end() );
    }
}

template< class T, class Alloc >
void vector<T,Alloc>::range_assign( const_iterator atFirst, const_iterator atLast )
{
    size_type liLen = atLast - atFirst;

    if ( liLen > capacity() ) 
    {
        //reallcoate and copy
        iterator ltTmp = allocate_and_copy( liLen, atFirst, atLast );
        clear_block();
        set( ltTmp, ltTmp + liLen, ltTmp + liLen );
    }
    else if ( size() >= liLen ) 
    {
        //copy element and destroy redundant
        iterator ltNewFinish = AI_STD::copy( atFirst, atLast, this->cptStart );
        destroy( ltNewFinish, this->cptFinish );
        this->cptFinish = ltNewFinish;
    }
    else 
    {
        //enough block
        const_iterator ltMid = atFirst + size() ;
        AI_STD::copy( atFirst, ltMid, this->cptStart );
        this->cptFinish = uninitialized_copy( ltMid, atLast, this->cptFinish );
    }
}

END_STLNAMESPACE

#endif // __AILIB__VECTOR_H__
