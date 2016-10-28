//=============================================================================
/**
 * \file    string.h
 * \brief reference STL string standard interface
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: string.h,v 1.7 2010/12/21 02:03:16 daizh Exp $
 *
 * History
 * 2008.02.22 first release by DZH
 */
//=============================================================================
#ifndef __AILIB__STRING_H__
#define __AILIB__STRING_H__

#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "config.h"
#include "algobase.h"
#include "allocate.h"
#include "contain_base.h"

START_STLNAMESPACE

//string base allocate and deallocate memory
template< class TChar >
class string_base : public contain_base<TChar, allocator<TChar> >
{
public:
    typedef contain_base<TChar, allocator<TChar> > base;
public:
    string_base( ):
        cpsStart(NULL),
        cpsFinish(NULL)
    { }
    
    string_base( size_t aiN )
    {
        cpsStart  = base::allocate_block( aiN );
        cpsFinish = cpsStart;
        cpsLast   = cpsStart + aiN;
    }

    ~string_base( )
    {
        deallocate_block();
    }

    void deallocate_block( TChar* cptPtr, size_t aiN )
    {
        base::deallocate_block( cptPtr, aiN );
    }

    void deallocate_block()
    {
        base::deallocate_block( cpsStart, cpsLast - cpsStart );
        cpsStart = NULL;
        cpsFinish = NULL;
        cpsLast = NULL;
    }

protected:
    TChar* cpsStart;
    TChar* cpsFinish;
    TChar* cpsLast;
};

template< class TChar >
class basic_string : public string_base<TChar>
{
public:
    typedef TChar value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef string_base<TChar> base;
    typedef basic_string<TChar> self;

public:
       //iterator 
    template< class TRAIT >
    class clsIterator 
    {
        friend class basic_string;
    public:
        clsIterator() : cpElem(NULL) {}
        clsIterator( typename TRAIT::pointer apElem ) : cpElem(apElem) {}

        typename TRAIT::reference operator *()
        {
            return (*cpElem);
        }
        typename TRAIT::pointer operator ->( )
        {
            return (&**this);
        }
        clsIterator& operator++()
        {    // preincrement
            ++cpElem;
            return (*this);
        }
        clsIterator operator++(int)
        {    // postincrement
            clsIterator liTmp = *this;
            ++*this;
            return (liTmp);
        }

        clsIterator& operator--()
        {    // predecrement
            --cpElem;
            return (*this);
        }

        clsIterator operator--(int)
        {    // postdecrement
            clsIterator liTmp = *this;
            --*this;
            return (liTmp);
        }

        clsIterator& operator+=( size_t aiOff )
        {    // increment by integer
            cpElem += aiOff;
            return (*this);
        }

        clsIterator operator+( size_t aiOff ) const
        {    // return this + integer
            clsIterator liTmp = *this;
            return (liTmp += aiOff);
        }

        clsIterator& operator-=( size_t aiOff )
        {    // decrement by integer
            return (*this += -aiOff);
        }

        clsIterator operator-( size_t aiOff ) const
        {    // return this - integer
            clsIterator liTmp = *this;
            return (liTmp -= aiOff);
        }

        size_t operator-( const clsIterator& aoRight ) const
        {    // return difference of iterators


            return (cpElem - aoRight.cpElem);
        }

        typename TRAIT::reference operator[]( size_t aiOff ) const
        {    // subscript
            return (*(*this + aiOff));
        }

#define DEFINE_OPERATOR_ITERATOR( Opt ) \
        bool operator Opt ( const clsIterator& aoRight ) const \
        {\
            return ( cpElem Opt aoRight.cpElem );\
        }

        DEFINE_OPERATOR_ITERATOR( == );
        DEFINE_OPERATOR_ITERATOR( != );
        DEFINE_OPERATOR_ITERATOR( >= );
        DEFINE_OPERATOR_ITERATOR( <= );
        DEFINE_OPERATOR_ITERATOR( > );
        DEFINE_OPERATOR_ITERATOR( < );

    protected:
        bool is_null()   { return cpElem == NULL; }

        typename TRAIT::pointer cpElem;
    };

    //reverse iterator
    template< class TRAIT >
    class clsReverseIterator
    {
        friend class basic_string;
    public:
        clsReverseIterator() : cpElem(NULL) {}
        clsReverseIterator( typename TRAIT::pointer apElem ) : cpElem(apElem) {}

        typename TRAIT::reference operator *()
        {
            typename TRAIT::pointer lpTmp = cpElem;
            return (*--lpTmp);
        }
        typename TRAIT::pointer operator ->( )
        {
            return (&**this);
        }
        clsReverseIterator& operator++()
        {    // preincrement
            --cpElem;
            return (*this);
        }
        clsReverseIterator operator++(int)
        {    // postincrement
            clsReverseIterator liTmp = *this;
            ++*this;
            return (liTmp);
        }

        clsReverseIterator& operator--()
        {    // predecrement
            ++cpElem;
            return (*this);
        }

        clsReverseIterator operator--(int)
        {    // postdecrement
            clsReverseIterator liTmp = *this;
            --*this;
            return (liTmp);
        }

        clsReverseIterator& operator+=( size_t aiOff )
        {    // increment by integer
            cpElem -= aiOff;
            return (*this);
        }

        clsReverseIterator operator+( size_t aiOff ) const
        {    // return this + integer
            clsReverseIterator liTmp = *this;
            return (liTmp += aiOff);
        }

        clsReverseIterator& operator-=( size_t aiOff )
        {    // decrement by integer
            return (*this += aiOff);
        }

        clsReverseIterator operator-( size_t aiOff ) const
        {    // return this - integer
            clsReverseIterator liTmp = *this;
            return (liTmp -= aiOff);
        }

        size_t operator-( const clsReverseIterator& aoRight ) const
        {    // return difference of iterators
            return (cpElem + aoRight.cpElem);
        }

        typename TRAIT::reference operator[]( size_t aiOff ) const
        {    // subscript
            return (*(*this + aiOff));
        }

#define DEFINE_OPERATOR_REVERSEITERATOR( Opt ) \
        bool operator Opt ( const clsReverseIterator& aoRight ) const \
        {\
            return ( cpElem Opt aoRight.cpElem );\
        }

        DEFINE_OPERATOR_REVERSEITERATOR( == );
        DEFINE_OPERATOR_REVERSEITERATOR( != );
        DEFINE_OPERATOR_REVERSEITERATOR( >= );
        DEFINE_OPERATOR_REVERSEITERATOR( <= );
        DEFINE_OPERATOR_REVERSEITERATOR( > );
        DEFINE_OPERATOR_REVERSEITERATOR( < );

    protected:
        bool is_null()   { return cpElem == NULL; }

        typename TRAIT::pointer cpElem;
    };

    struct clsConstIterator
    {
        typedef const TChar& reference;
        typedef const TChar* pointer;
    };

    struct clsAccessIterator
    {
        typedef TChar& reference;
        typedef TChar* pointer;
    };

    typedef clsIterator<clsConstIterator> const_iterator; // const iterator
    typedef clsIterator<clsAccessIterator> iterator;
    typedef clsReverseIterator<clsConstIterator> const_reverse_iterator; //const reverse iterator
    typedef clsReverseIterator<clsAccessIterator> reverse_iterator;

public:
    static const size_t npos = ~(size_t)0;

public:
    // constructor
    basic_string( ) : base(8)
    {
        construct_null( this->cpsFinish );
    }
    
    // constructor
    basic_string( size_t aiN, char acChar ) : base( aiN + 1 )
    {
        this->cpsFinish = AI_STD::fill_n( this->cpsStart, aiN, acChar );
        construct_null( this->cpsFinish );
    }

    basic_string( const TChar* apsPtr )
    {
        range_initialize( apsPtr, apsPtr + strlen( apsPtr ) );
    }

    basic_string( const TChar* apsPtr, size_t aiN )
    {
        range_initialize( apsPtr, apsPtr + aiN );
    }

    basic_string( const basic_string& apoS )
    {
        range_initialize( apoS.cpsStart, apoS.cpsFinish );
    }

    void swap( basic_string& apoS )
    {
        AI_STD::swap( this->cpsStart, apoS.cpsStart );
        AI_STD::swap( this->cpsFinish, apoS.cpsFinish );
        AI_STD::swap( this->cpsLast, apoS.cpsLast );
    }
    
    self& assign( const self& aoRight )
    {
        return assign( aoRight.cpsStart, aoRight.cpsFinish );
    }
    
    self& assign( const TChar* apsPtr, int aiPos, int aiLen )
    {
        return assign( apsPtr + aiPos, apsPtr + aiPos + aiLen );
    }
    
    self& assign( const self& aoPtr, int aiPos, int aiLen )
    {
        return assign( aoPtr.myptr() + aiPos, aoPtr.myptr() + aiPos + aiLen );
    }
    
    self& assign( const self& aoPtr, int aiLen )
    {
        return assign( aoPtr.myptr(), aoPtr.myptr() + aiLen );
    }
    
    self& assign( const TChar* apsFirst, const TChar* apsLast );
    
    self& append( const self& aoRight )
    {
        return append( aoRight.cpsStart, aoRight.cpsFinish );
    }
    
    self& append( const TChar* apsPtr, int aiPos, int aiLen )
    {
        return append( apsPtr + aiPos, apsPtr + aiPos + aiLen );
    }
    
    self& append( const self& aoPtr, int aiPos, int aiLen )
    {
        return append( aoPtr.myptr() + aiPos, aoPtr.myptr() + aiPos + aiLen );
    }
    
    self& append( const self& aoPtr, int aiLen )
    {
        return append( aoPtr.myptr(), aoPtr.myptr() + aiLen );
    }
    
    self& append( const TChar* apsFirst, const TChar* apsLast );
    
    self substr( size_t aiOff = 0, size_t aiCount = npos )
    {
        return self( this->cpsStart + aiOff, ai_min( aiCount, size() ) );
    }

    size_t find( const TChar* apsPtr, size_t aiOff = 0)
    {
        return find( apsPtr, aiOff, strlen(apsPtr) );
    }

    size_t find( const self& aoRight, size_t aiOff = 0)
    {
        return find( aoRight.myptr(), aiOff, aoRight.size() );
    }

    size_t find( TChar apcV, size_t aiOff = 0 )
    {
        const TChar* lpsVptr;
        if ( ( lpsVptr = find( this->cpsStart + aiOff, size() - aiOff, apcV ) ) == NULL )
        {
            return (npos);
        }
        return ( lpsVptr - this->cpsStart );
    }

    size_t rfind( const TChar* apsPtr, size_t aiOff = npos ) 
    {
        return rfind( apsPtr, aiOff, strlen(apsPtr) );
    }

    size_t rfind( TChar apcV, size_t aiOff = npos )
    {
        const TChar* lpsVptr;
        if ( ( lpsVptr = rfind( this->cpsStart + aiOff, size() - aiOff, apcV ) ) == NULL )
        {
            return (npos);
        }
        return ( lpsVptr - this->cpsStart );
    }

    size_t find_first_of(const self& aoRight, size_t aiOff = 0) const
    {    // look for one of aoRight at or after aiOff
        return find_first_of( aoRight.myptr(), aiOff, aoRight.size() );
    }
    
    size_t find_first_of( const TChar* apsPtr, size_t aiOff = 0 ) const
    {    // look for one of [apsPtr, <null>) at or after aiOff
        return find_first_of(apsPtr, aiOff, strlen(apsPtr));
    }

    size_t find_first_of( TChar acCh, size_t aiOff = 0 ) const
    {    // look for _Ch at or after aiOff
        return find((const TChar *)&acCh, aiOff, (size_t)1);
    }

    size_t find_last_of( const self& aoRight, size_t aiOff = npos ) const
    {    // look for one of aoRight before aiOff
        return find_last_of( aoRight.myptr(), aiOff, aoRight.size());
    }

    size_t find_last_of( const TChar* apsPtr, size_t aiOff = npos) const
    {    // look for one of [apsPtr, <null>) before aiOff
        return find_last_of(apsPtr, aiOff, strlen(apsPtr));
    }

    size_t find_last_of( TChar acCh, size_t aiOff = npos ) const
    {    // look for _Ch before aiOff
        return rfind((const TChar *)&acCh, aiOff, (size_t)1);
    }

    size_t find_first_not_of(const self& aoRight, size_t aiOff = 0) const
    {    // look for none of aoRight at or after aiOff
        return find_first_not_of( aoRight.myptr(), aiOff, aoRight.size());
    }

    size_t find_first_not_of( const TChar* apsPtr, size_t aiOff = 0 ) const
    {    // look for one of [apsPtr, <null>) at or after aiOff
        return find_first_not_of( apsPtr, aiOff, strlen(apsPtr) );
    }

    size_t find_first_not_of( TChar acCh, size_t aiOff = 0 ) const
    {    // look for non acCh at or after aiOff
        return find_first_not_of((const TChar *)&acCh, aiOff, (size_t)1);
    }

    size_t find_last_not_of( const self& aiRight, size_t aiOff = npos ) const
    {    // look for none of aiRight before aiOff
        return find_last_not_of( aiRight.myptr(), aiOff, aiRight.size());
    }

    size_t find_last_not_of( const TChar *apsPtr, size_t aiOff = npos ) const
    {    // look for none of [apsPtr, <null>) before aiOff
        return find_last_not_of(apsPtr, aiOff, strlen(apsPtr));
    }

    size_t find_last_not_of(TChar acCh, size_t aiOff = npos) const
    {    // look for non _Ch before aiOff
        return find_last_not_of((const TChar *)&acCh, aiOff, (size_t)1);
    }

    self& replace( size_t aiOff, size_t aiN0, const self& aoRight )
    {    // replace [aiOff, aiOff + aiN0) with aoRight
        return replace( aiOff, aiN0, aoRight.myptr(), aoRight.size() );
    }

    self& replace( size_t aiOff, size_t aiNO, const TChar *apsPtr )
    {    // replace [aiOff, aiOff + _N0) with [_Ptr, <null>)
        return replace( aiOff, aiNO, apsPtr, strlen(apsPtr) );
    }

    self& replace( size_t aiOff, size_t aiN0, size_t aiCount, TChar acCh );

    self& replace( iterator aoFirst, iterator aoLast, const self& aoRight )
    {    // replace [aoFirst, aoLast) with aoRight
        return replace(
            distance(aoFirst, begin()), distance(aoLast, aoFirst), aoRight);
    }

    self& replace( iterator aoFirst, iterator aoLast, const TChar *apsPtr )
    {    // replace [aoFirst, aoLast) with [apsPtr, <null>)
        return replace(
            distance(aoFirst, begin()), distance(aoLast, aoFirst), apsPtr );
    }

    self& replace( iterator aoFirst, iterator aoLast, size_t aiCount, TChar acCh )
    {    // replace [aoFirst, aoLast) with aiCount * acCh
        return replace(
            distance(aoFirst, begin()), distance(aoLast, aoFirst), aiCount, acCh );
    }

//over write assign operator 
public:
    const_reference operator [] ( size_t aiOff ) const
    {
        return ( (myptr())[aiOff] );
    }

    reference operator [] ( size_t aiOff )
    {
        return ( (myptr())[aiOff] );
    }

    const_reference at( size_t aiOff ) const
    {
        if ( size() <= aiOff )
        {
            base::Xran();    // aiOff off end
        }
        return ( (myptr())[aiOff] );
    }

    reference at( size_t aiOff )
    {
        if ( size() <= aiOff )
        {
            base::Xran();    // aiOff off end
        }
        return ( (myptr())[aiOff] );
    }

    self& operator = ( const TChar* apsPtr )
    {
        assign( apsPtr, apsPtr + strlen( apsPtr ) );
        return *this;
    }

    self& operator = ( const self& aoRight )
    {
        assign( aoRight.cpsStart, aoRight.cpsFinish );
        return *this;
    }

    self& operator += ( const TChar* apsPtr )
    {
        append( apsPtr, apsPtr + strlen(apsPtr) );
        return *this;
    }

    self& operator += ( const self& aoRight )
    {
        append( aoRight.cpsStart, aoRight.cpsFinish );
        return *this;
    }

//over write compare operator
public:
#define DEFINE_COMPARE_OPERATOR( Opt ) \
    bool operator Opt ( const TChar* apsPtr ) const \
    {\
        if (AI_STD::compare(this->cpsStart, this->size(), apsPtr, strlen(apsPtr) ) Opt 0 ) return true;\
        else return false;\
    }\
    bool operator Opt ( const self& aoS ) const \
    {\
        if (AI_STD::compare( this->cpsStart, this->size(), aoS.cpsStart, aoS.size() ) Opt 0 ) return true;\
        else return false;\
    }

    DEFINE_COMPARE_OPERATOR( == );
    DEFINE_COMPARE_OPERATOR( != );
    DEFINE_COMPARE_OPERATOR( >= );
    DEFINE_COMPARE_OPERATOR( <= );
    DEFINE_COMPARE_OPERATOR( > );
    DEFINE_COMPARE_OPERATOR( < );        

public:
    const TChar* c_str() const { return this->cpsStart; }

public:
    void resize( size_t aiN )
    {
        if ( capacity() > aiN ) return; //enough

        const size_t liNewLen = aiN + 1;
        TChar* lpsNewStart = this->allocate_block( liNewLen ); //reallocate block
        TChar* lpsNewFinish = lpsNewStart;
        if ( size() )
        {
               //copy to new block
            lpsNewFinish = AI_STD::copy( this->cpsStart, this->cpsFinish, lpsNewStart );
        }

        construct_null( lpsNewFinish ); //end of string

        this->deallocate_block(); //recycle block

        this->cpsStart = lpsNewStart;
        this->cpsFinish = lpsNewFinish;
        this->cpsLast = lpsNewStart + liNewLen;
    }

    void clear() 
    {
        this->cpsFinish = this->cpsStart;
        construct_null(this->cpsFinish);
    }

    size_t size() const { return this->cpsFinish - this->cpsStart; }
    size_t capacity() const { return (this->cpsLast - this->cpsStart) - 1; }
    size_t length() const { return size(); }
    bool empty() const   { return this->cpsStart == this->cpsFinish; }

public:
    
    iterator begin()
    {    // return iterator for beginning of mutable sequence
        return iterator( myptr() );
    }

    const_iterator begin() const
    {    // return iterator for beginning of nonmutable sequence
        return const_iterator( myptr() );
    }

    iterator end()
    {    // return iterator for end of mutable sequence
        return iterator( myptr() + size() );
    }

    const_iterator end() const
    {    // return iterator for end of nonmutable sequence
        return const_iterator( myptr() + size() );
    }

    reverse_iterator rbegin()
    {    // return iterator for beginning of mutable sequence
        return reverse_iterator( myptr() + size() );
    }

    const_reverse_iterator rbegin() const
    {    // return iterator for beginning of nonmutable sequence
        return const_reverse_iterator( myptr() + size() );
    }

    reverse_iterator rend()
    {    // return iterator for end of mutable sequence
        return reverse_iterator( myptr() );
    }

    const_reverse_iterator rend() const
    {    // return iterator for end of nonmutable sequence
        return const_reverse_iterator( myptr() );
    }

private:
    const TChar* myptr() const        {   return this->cpsStart; }

    TChar* myptr()                  {   return this->cpsStart; }

    void construct_null( TChar* aps )
    {
        *aps = 0;
    }

    void construct_null( size_t aiN )
    {
        this->cpsFinish = this->cpsStart + aiN;
        construct_null( this->cpsFinish );
    }

    void range_initialize( const TChar* apsFirst, const TChar* apsLast );

    

    size_t find( const TChar* apsPtr, size_t aiOff, size_t aiCount ) const;

    const TChar* find( const TChar* apsPtr, size_t aiN, TChar acCh ) const;

    size_t rfind( const TChar *apsPtr, size_t aiOff, size_t aiCount ) const;

    const TChar* rfind( const TChar* apsPtr, size_t aiN, TChar acCh ) const;

    size_t find_first_of(const TChar* apsPtr, size_t aiOff, size_t aiCount ) const;

    size_t find_first_not_of( const TChar* apsPtr, size_t aiOff, size_t aiCount ) const;

    size_t find_last_not_of(const TChar* apsPtr, size_t aiOff, size_t aiCount) const;

    size_t find_last_of(const TChar* apsPtr, size_t aiOff, size_t aiCount) const;

    self& replace(size_t aiOff, size_t aiN0, const self& aoRight, size_t aiRoff, size_t aiCount);

    self& replace( size_t aiOff, size_t aiNO, const TChar* apsPtr, size_t aiCount );

    self& replace( iterator aoFirst, iterator aoLast, const TChar* apsPtr, size_t aiCount )
    {    // replace [aoFirst, aoLast) with [apsPtr, apsPtr + aiCount)
        return replace(
            distance(aoFirst, begin()), distance(aoLast, aoFirst), apsPtr, aiCount );
    }

    size_t distance( const_iterator aoP2, const_iterator aoP1 )
    {    // compute safe iterator difference
        return ( aoP2.is_null() ? 0 : aoP2 - aoP1 );
    }
    
    size_t distance( iterator aoP2, iterator aoP1 )
    {    // compute safe iterator difference
        return ( aoP2.is_null() ? 0 : aoP2 - aoP1 );
    }
    
    bool inside( const TChar* apsPtr )
    {    // test if apsPtr points inside string
        if ( apsPtr < myptr() || myptr() + size() <= apsPtr)
        {
            return false;    // don't ask
        }
        else
        {
            return true;
        }
    }
    
    bool grow( size_t aiN )
    {
        if ( capacity() > aiN ) return ( 0 < aiN );

        resize( NextAllcoSize( aiN ) );

        return true;
    }

    size_t NextAllcoSize( size_t aiN )
    {
        return ( aiN + size_t(aiN * 0.2) + 1 );
    }
};

//implement
template< class TChar >
void basic_string<TChar>::range_initialize( const TChar* apsFirst, const TChar* apsLast )
{
     size_t liN = apsLast - apsFirst;
     
     //allocate block
     this->cpsStart = this->allocate_block( liN + 1 );
     this->cpsFinish = AI_STD::copy( apsFirst, apsLast, this->cpsStart );
     this->cpsLast = this->cpsStart + liN + 1;
     construct_null(this->cpsFinish);
}

template< class TChar >
basic_string<TChar>& basic_string<TChar>::append( const TChar* apsFirst, const TChar* apsLast )
{
     if ( apsFirst != apsLast ) 
     {
          size_t liLen = apsLast - apsFirst;
          size_t liOldSize = size();
          if ( liLen + liOldSize > capacity() )
          {
               // preallocate size
               const size_t liNewLen = NextAllcoSize( liOldSize + liLen );
               TChar* lpsNewStart = this->allocate_block( liNewLen );
               TChar* lpsNewFinish = AI_STD::copy( this->cpsStart, this->cpsFinish, lpsNewStart );
               //copy [apsFirst, apsLast] region
               lpsNewFinish = AI_STD::copy( apsFirst, apsLast, lpsNewFinish ); 
               construct_null( lpsNewFinish );

               this->deallocate_block(); //recycle

               this->cpsStart = lpsNewStart;
               this->cpsFinish = lpsNewFinish;
               this->cpsLast = lpsNewStart + liNewLen;
          }
          else
          {
               // have enough standby memory
               this->cpsFinish = AI_STD::copy( apsFirst, apsLast, this->cpsFinish );
               construct_null( this->cpsFinish );
          }
     }
     else
     {
          construct_null( this->cpsFinish );
     }

     return *this;
}

template< class TChar >
basic_string<TChar>& basic_string<TChar>::assign( const TChar* apsFirst, const TChar* apsLast )
{
     //base
     if ( apsFirst == this->cpsStart ) return *this;

     this->cpsFinish = this->cpsStart;
     append( apsFirst, apsLast );
     return *this;
}

template< class TChar >
size_t basic_string<TChar>::find( const TChar* apsPtr, size_t aiOff, size_t aiCount ) const
{
     if ( aiCount == 0 && aiOff <= size() )
     {
          return (aiOff);	// null string always matches (if inside string)
     }

     size_t liNm;
     if ( aiOff < size() && aiCount <= ( liNm = size() - aiOff ) )
     {	// room for match, look for it
          const TChar *lpsUptr, *lpsVptr;
          for ( liNm -= aiCount - 1, lpsVptr = myptr() + aiOff;
               ( lpsUptr = find( lpsVptr, liNm, *apsPtr ) ) != 0;
               liNm -= lpsUptr - lpsVptr + 1, lpsVptr = lpsUptr + 1 )
          {
               if ( AI_STD::compare( lpsUptr, aiCount, apsPtr, aiCount ) == 0)
               {
                    return ( lpsUptr - myptr() );	// found a match
               }
          }
     }

     return (npos);	// no match
}

template< class TChar >
const TChar* basic_string<TChar>::find( const TChar* apsPtr, size_t aiN, TChar acCh ) const
{
     for ( size_t liN = 0; liN < aiN; liN++ )
     {
          if ( apsPtr[liN] == acCh ) return apsPtr + liN;
     }
     return NULL;
}

template< class TChar >
size_t basic_string<TChar>::rfind( const TChar *apsPtr, size_t aiOff, size_t aiCount ) const
{	// look for [apsPtr, apsPtr + aiCount) beginning before aiOff
     if ( aiCount == 0)
     {
          return ( aiOff < size() ? aiOff : size() );	// null always matches
     }
     if ( aiCount <= size() )
     {	// room for match, look for it
          const TChar *lpsUptr = myptr() +
               ( aiOff < size() - aiCount ? aiOff : size() - aiCount );
          for ( ; ; --lpsUptr )
               if ( *lpsUptr == *apsPtr
                    && AI_STD::compare( lpsUptr, aiCount, apsPtr, aiCount ) == 0 )
               {
                    return lpsUptr - myptr();	// found a match
               }
               else if ( lpsUptr == myptr() )
                    break;	// at beginning, no more chance for match
     }

     return npos;	// no match
}

template< class TChar >
const TChar* basic_string<TChar>::rfind( const TChar* apsPtr, size_t aiN, TChar acCh ) const
{
     for ( size_t liN = aiN - 1; liN >= 0; liN-- )
     {
          if ( apsPtr[liN] == acCh ) return apsPtr + liN;
     }
     return NULL;
}

template< class TChar >
size_t basic_string<TChar>::find_first_of(const TChar *apsPtr, size_t aiOff, size_t aiCount ) const
{	// look for one of [apsPtr, apsPtr + aiOff) at or after aiOff
     if ( 0 < aiCount && aiOff < size() )
     {	// room for match, look for it
          const TChar *const lpsVptr = myptr() + size();
          for ( const TChar *lpsUptr = myptr() + aiOff; lpsUptr < lpsVptr; ++lpsUptr )
               if ( find(apsPtr, aiCount, *lpsUptr) != 0)
                    return ( lpsUptr - myptr() );	// found a match
     }
     return npos;	// no match
}

template< class TChar >
size_t basic_string<TChar>::find_first_not_of( const TChar *apsPtr, size_t aiOff, size_t aiCount ) const
{	// look for none of [apsPtr, apsPtr + aiCount) at or after aiOff
     if ( aiOff < size() )
     {	// room for match, look for it
          const TChar *const lpsVptr = myptr() + size();
          for ( const TChar *lpsUptr = myptr() + aiOff; lpsUptr < lpsVptr; ++lpsUptr )
          {
               if ( find( apsPtr, aiCount, *lpsUptr ) == 0 )
               {
                    return (lpsUptr - myptr());
               } 
          }
     }
     return npos;
}

template< class TChar >
size_t basic_string<TChar>::find_last_not_of(const TChar *apsPtr, size_t aiOff, size_t aiCount) const
{	// look for none of [apsPtr, apsPtr + aiCount) before aiOff
     if ( 0 < size() )
     {
          for ( const TChar *lpsUptr = myptr()
               + (aiOff < size() ? aiOff : size() - 1); ; --lpsUptr )
          {
               if ( find(apsPtr, aiCount, *lpsUptr) == 0 )
               {
                    return (lpsUptr - myptr());
               }
               else if ( lpsUptr == myptr() )
               {
                    break;
               }     
          }
     }
     return npos;
}

template< class TChar >
size_t basic_string<TChar>::find_last_of(const TChar *apsPtr, size_t aiOff, size_t aiCount) const
{	// look for one of [apsPtr, apsPtr + aiCount) before aiOff
     if ( 0 < aiCount && 0 < size() )
     {
          for ( const TChar *lpsUptr = myptr()
               + (aiOff < size() ? aiOff : size() - 1); ; --lpsUptr )
          {
               if ( find( apsPtr, aiCount, *lpsUptr ) != 0)
               {
                    return ( lpsUptr - myptr());	// found a match
               }
               else if ( lpsUptr == myptr())
               {
                    break;	// at beginning, no more chance for match
               }
          }
     }

     return npos;	// no match
}

template< class TChar >
basic_string<TChar>& basic_string<TChar>::replace( size_t aiOff, size_t aiN0, size_t aiCount, TChar acCh )
{	// replace [aiOff, aiOff + _N0) with aiCount * acCh
     if ( size() < aiOff )
     {
          base::Xran();	// aiOff off end
     }     
     if ( size() - aiOff < aiN0 )
     {
          aiN0 =  size() - aiOff;	// trim aiN0 to size 
     }
     if ( npos - aiCount <=  size() - aiN0 )
     {
          base::Xlen();	// result too long 
     }
          
     size_t liNm =  size() - aiN0 - aiOff;

     if ( aiCount < aiN0 )
     {
          AI_STD::move( myptr() + aiOff + aiCount,
                        myptr() + aiOff + aiN0, liNm );	// smaller hole, move tail up 
     }
     size_t liNum;
     if ( (0 < aiCount || 0 < aiN0) && grow(liNum =  size() + aiCount - aiN0) )
     {	// make room and rearrange
          if ( aiN0 < aiCount )
          {
               AI_STD::move( myptr() + aiOff + aiCount,
                             myptr() + aiOff + aiN0, liNm ); // move tail down 
          }
          AI_STD::assign( myptr() + aiOff, aiCount, acCh );	// fill hole 
          construct_null( liNum );
     }
     return (*this);
}

template< class TChar >
basic_string<TChar>& basic_string<TChar>::replace(size_t aiOff, size_t aiN0, const basic_string<TChar>& aoRight, size_t aiRoff, size_t aiCount)
{	// replace [aiOff, aiOff + aiN0) with aoRight [aiRoff, aiRoff + aiCount)
     if ( size() < aiOff || aoRight.size() < aiRoff )
     {
          base::Xran();	// aiOff or aiRoff off end
     }     
     if ( size() - aiOff < aiN0 )
     {
          aiN0 = size() - aiOff;	// trim aiN0 to size
     }
     size_t liNum = aoRight.size() - aiRoff;
     if ( liNum < aiCount )
     {
          aiCount = liNum;	// trim aiCount to size
     }
     if (npos - aiCount <= size() - aiN0)
     {
          base::Xlen();	// result too long
     }

     size_t liNm = size() - aiN0 - aiOff;	// length of preserved tail
     size_t liNewsize = size() + aiCount - aiN0;
     if ( size() < liNewsize )
     {
          grow( liNewsize );
     }     

     if ( this != &aoRight )
     {	// no overlap, just move down and copy in new stuff
          AI_STD::move( myptr() + aiOff + aiCount, myptr() + aiOff + aiN0, liNm );	// empty hole
          AI_STD::copy( myptr() + aiOff, aoRight.myptr() + aiRoff, aiCount);	// fill hole
     }
     else if ( aiCount <= aiN0 )
     {	// hole doesn't get larger, just copy in substring
          AI_STD::move( myptr() + aiOff,  myptr() + aiRoff, aiCount);	// fill hole
          AI_STD::move( myptr() + aiOff + aiCount, myptr() + aiOff + aiN0, liNm);	// move tail down
     }
     else if ( aiRoff <= aiOff )
     {	// hole gets larger, substring begins before hole
          AI_STD::move( myptr() + aiOff + aiCount, myptr() + aiOff + aiN0, liNm);	// move tail down
          AI_STD::move( myptr()+ aiOff, myptr() + aiRoff, aiCount);	// fill hole
     }
     else if ( aiOff + aiN0 <= aiRoff )
     {	// hole gets larger, substring begins after hole
          AI_STD::move( myptr() + aiOff + aiCount, myptr() + aiOff + aiN0, liNm);	// move tail down
          AI_STD::move( myptr() + aiOff, myptr() + (aiRoff + aiCount - aiN0), aiCount);	// fill hole
     }
     else
     {	// hole gets larger, substring begins in hole
          AI_STD::move( myptr() + aiOff,  myptr() + aiRoff, aiN0);	// fill old hole
          AI_STD::move( myptr() + aiOff + aiCount, myptr() + aiOff + aiN0, liNm);	// move tail down
          AI_STD::move( myptr() + aiOff + aiN0, myptr() + aiRoff + aiCount,
               aiCount - aiN0); // fill rest of new hole
     }
     construct_null( liNewsize );
     return *this;
}

template< class TChar >
basic_string<TChar>& basic_string<TChar>::replace( size_t aiOff, size_t aiNO, const TChar *apsPtr, size_t aiCount )
{	// replace [aiOff, aiOff + aiNO) with [apsPtr, apsPtr + aiCount)
     if (inside(apsPtr))
     {
          return replace(aiOff, aiNO, *this, apsPtr - myptr(), aiCount);	// substring, replace carefully
     }

     if ( size() < aiOff )
     {
          base::Xran();	// aiOff off end
     }     

     if ( size() - aiOff < aiNO )
     {
          aiNO = size() - aiOff;	// trim aiNO to size
     }

     if ( npos - aiCount <= size() - aiNO )
     {
          base::Xlen();	// result too long
     }

     size_t liNm = size() - aiNO - aiOff;

     if (aiCount < aiNO)
     {
          AI_STD::move( myptr() + aiOff + aiCount, myptr() + aiOff + aiNO, liNm );	// smaller hole, move tail up
     }
     size_t liNum;
     if ( (0 < aiCount || 0 < aiNO) && grow( liNum = size() + aiCount - aiNO ) )
     {	// make room and rearrange
          if ( aiNO < aiCount )
          {
               AI_STD::move( myptr() + aiOff + aiCount, myptr() + aiOff + aiNO, liNm );	// move tail down
          }
          AI_STD::copy( myptr() + aiOff, apsPtr, aiCount );	// fill hole
          construct_null( liNum );
     }
     return *this;
}

typedef basic_string<char> string;

END_STLNAMESPACE

inline AI_STD::string operator + ( AI_STD::string& aoLeft, AI_STD::string& aoRight )
{
    return ( AI_STD::string(aoLeft) += aoRight );
}

inline AI_STD::string operator + ( AI_STD::string& aoLeft, const char* apsRight )
{
    return ( AI_STD::string(aoLeft) += apsRight );
}

#endif // __AILIB__STRING_H__
