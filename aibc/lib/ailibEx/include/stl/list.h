//=============================================================================
/**
 * \file    list.h
 * \brief reference STL list standard interface
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: list.h,v 1.1 2008/11/04 03:50:19 fzz Exp $
 *
 * History
 * 2008.02.28 first release by DZH
 */
//=============================================================================
#ifndef __AILIB__LIST_H__
#define __AILIB__LIST_H__

#include "contain_base.h"
#include "type_traits.h"
#include "allocate.h"
#include "algobase.h"

START_STLNAMESPACE

template< class T >
struct list_node 
{
    typedef list_node<T>* node_pointer;
    
    node_pointer cpoPrev;
    node_pointer cpoNext;
    
    T ctData;
};

template< class T, class Traits >
struct list_base_iterator
{
    typedef list_base_iterator<T, Traits> self;
    
	typedef T value_type;
	typedef typename Traits::pointer pointer;
	typedef typename Traits::reference reference;
    
    typedef typename list_node<T>::node_pointer link_type;
    
    // constructor
    list_base_iterator( link_type apoX ) : cpoNode(apoX) {}
    list_base_iterator() {}
    
    // over write operator
    bool operator == ( const self& aoX ) const { return aoX.cpoNode == cpoNode; }
    bool operator != ( const self& aoX ) const { return aoX.cpoNode != cpoNode; }
    reference operator * () const { return cpoNode->ctData; }
    pointer operator -> () const { return &(this->operator* ()); }

    link_type cpoNode;
};

template< class T, class Traits >
struct list_iterator : public list_base_iterator<T, Traits>
{
	typedef list_iterator<T, Traits> iterator;

	typedef list_iterator<T, Traits> self;
	typedef list_base_iterator<T, Traits> base;
	
    // constructor
    list_iterator( typename base::link_type apoX ) : base(apoX) {}
    list_iterator() {}
    list_iterator( const iterator& aoX ) : base(aoX.cpoNode) {}
        
    // self increamce by 1
    self& operator ++ ()
    {
        this->cpoNode = this->cpoNode->cpoNext;
        return *this;
    }
    self operator ++ (int)
    {
        self loTmp = *this;
        ++(*this);
        return loTmp;
    }
    // self decreamce by 1
    self& operator -- ()
    {
        this->cpoNode = this->cpoNode->cpoPrev;
        return *this;
    }
    self operator -- (int)
    {
        self loTmp = *this;
        ++(*this);
        return loTmp;
    }
};

template< class T, class Traits >
struct list_reverse_iterator : public list_base_iterator<T, Traits>
{
	typedef list_reverse_iterator<T, Traits> iterator;

	typedef list_reverse_iterator<T, Traits> self;
	typedef list_base_iterator<T, Traits> base;
	
    // constructor
    list_reverse_iterator( typename base::link_type apoX ) : base(apoX) {}
    list_reverse_iterator() {}
    list_reverse_iterator( const iterator& aoX ) : base(aoX.cpoNode) {}
        
    // self increamce by 1
    self& operator ++ ()
    {
        this->cpoNode = this->cpoNode->cpoPrev;
        return *this;
    }
    self operator ++ (int)
    {
        self loTmp = *this;
        ++(*this);
        return loTmp;
    }
    // self decreamce by 1
    self& operator -- ()
    {
        this->cpoNode = this->cpoNode->cpoNext;
        return *this;
    }
    self operator -- (int)
    {
        self loTmp = *this;
        ++(*this);
        return loTmp;
    }
};

// vector base allocate and deallocate block
template < class T, class Alloc > 
class list_base : public contain_base< list_node<T>, Alloc>
{
public:
    typedef list_node<T> node;
    typedef contain_base<node, Alloc> base;
    typedef typename list_node<T>::node_pointer link_type;
        
public:
    list_base( )
    {
        link_type lpoNode = this->get_node();
        lpoNode->cpoPrev = lpoNode;
        lpoNode->cpoNext = lpoNode;
        cpoNode = lpoNode;
    }
    
    link_type get_node()
    {
        link_type lpoNode = base::allocate_block( 1 );
        lpoNode->cpoPrev = NULL;
        lpoNode->cpoNext = NULL;
        
        return lpoNode;
    }
    
    void put_node( link_type apoX )
    {
        base::deallocate_block( apoX, 1 );
    }
    
    void destroy_node( link_type apoX )
    {
        _destroy( &apoX->ctData );
        this->put_node(apoX);
    }
    
    link_type create_node( const T& atX )
    {
        link_type lpoNode = this->get_node();
        _construct( &lpoNode->ctData, atX );
        
        return lpoNode;
    }
    
    ~list_base() 
    {
        this->put_node(cpoNode);
    }
            
protected:
    link_type cpoNode;  //
};

template< class T, class Alloc = allocator< list_node<T> > >
class list : public list_base< T, Alloc >
{
public:
    typedef list_base< T, Alloc > base;
    typedef list<T, Alloc> self;
    
    typedef typename base::link_type link_type;
    typedef list_iterator< T, nonconst_traits<T> > iterator;
	typedef list_iterator< T, const_traits<T> > const_iterator;
	typedef list_reverse_iterator< T, nonconst_traits<T> > reverse_iterator;
	typedef list_reverse_iterator< T, const_traits<T> > const_reverse_iterator;
	typedef typename iterator::reference reference;
	typedef typename iterator::pointer pointer;

public:
    list()
    {
    }
    ~list()
    {
        this->clear();
    }
    
    iterator insert( iterator aoPosition, const T& atX )
    {
        link_type lpoTmp = this->create_node(atX);

        lpoTmp->cpoNext = aoPosition.cpoNode;
        lpoTmp->cpoPrev = aoPosition.cpoNode->cpoPrev;
        aoPosition.cpoNode->cpoPrev->cpoNext = lpoTmp;
        aoPosition.cpoNode->cpoPrev = lpoTmp;
        
        return lpoTmp;
    }
    
    iterator erase( iterator aoPosition )
    {
        link_type lpoNext = aoPosition.cpoNode->cpoNext;
        link_type lpoPrev = aoPosition.cpoNode->cpoPrev;
        
        lpoPrev->cpoNext = lpoNext;
        lpoNext->cpoPrev = lpoPrev;
        
        this->destroy_node( aoPosition.cpoNode );

        return lpoNext;
    }
    
    void clear()
    {
        link_type lpoCur = this->cpoNode->cpoNext;
        while( lpoCur != this->cpoNode )
        {
            link_type lpoTmp = lpoCur;
            lpoCur = lpoCur->cpoNext;
            this->destroy_node( lpoTmp );
        }
        //reset initialize state
        this->cpoNode->cpoNext = this->cpoNode;
        this->cpoNode->cpoPrev = this->cpoNode;
    }
    
    void remove(  const T& aoValue )
    {
        iterator loFirst = this->begin();
        iterator loLast  = this->end();
        
        while( loFirst != loLast )
        {
            iterator loNext = loFirst;
            ++loNext;
            if ( *loFirst == aoValue )
            {
                this->erase( loFirst );
            }
            loFirst = loNext;
        }
    }
    
    void unique()
    {
        iterator loFirst = this->begin();
        iterator loLast  = this->end();
        
        if ( loFirst == loLast ) return;
        
        iterator loNext = loFirst;
        while( ++loNext != loLast )
        {
            if ( *loFirst == *loNext )
            {
                this->erase( loNext );
            }
            else
            {
                loFirst = loNext;
            }
            
            loNext = loFirst;
        }
    }
    
    void splice( iterator aoPosition, self& aoX )
    {
        if ( !aoX.empty() )
        {
            transfer( aoPosition, aoX.begin(), aoX.end() );
        }
    }
    
    void splice( iterator aoPosition, self&, iterator aoI )
    {
        iterator loLast = aoI;
        ++loLast;
        if ( aoPosition == aoI || aoPosition == loLast ) return;
        
        transfer( aoPosition, aoI, loLast );
    }
    
    void splice( iterator aoPosition, self&, iterator aoFirst, iterator aoLast )
    {
        if ( aoFirst != aoLast )
        {
            transfer( aoPosition, aoFirst, aoLast );
        }
    }
    
    void merge( self& aoX )
    {
		this->merge( aoX, __less() );
    }
    
    template< class TStrictWeakOrdering >
    void merge( self& aoX, TStrictWeakOrdering afComp )
    {
		iterator aoFirst1 = this->begin();
		iterator aoLast1  = this->end();
		iterator aoFirst2 = aoX.begin();
		iterator aoLast2  = aoX.end();
		
		while ( aoFirst1 != aoLast1 && aoFirst2 != aoLast2 )
		{
			if ( afComp( *aoFirst2, *aoFirst1 ) )
			{
				iterator loNext = aoFirst2;
				transfer( aoFirst1.cpoNode, aoFirst2.cpoNode, (++loNext).cpoNode );
				aoFirst2 = loNext;
			}
			else
				++aoFirst1;
		}
		
		if ( aoFirst2 != aoLast2 )
		{
		    transfer( aoLast1.cpoNode, aoFirst2.cpoNode, aoLast2.cpoNode );
		}
    }
    
	void sort() 
	{
	    this->sort( __less() );
	}
	
    template< class TStrictWeakOrdering >
	void sort( TStrictWeakOrdering afComp ) 
	{
		// Do nothing if the list has length 0 or 1.
		if ( this->cpoNode->cpoNext == this->cpoNode || this->cpoNode->cpoNext->cpoNext == this->cpoNode ) return;
		
		self loCarry;
		self loCounter[64];

		int liFill = 0;
		
		while ( !this->empty() ) 
		{
			loCarry.splice( loCarry.begin(), *this, (*this).begin() );
			
			int liI = 0;
			while( liI < liFill && !loCounter[liI].empty() ) 
			{
				loCounter[liI].merge( loCarry, afComp );
				loCarry.swap(loCounter[liI++]);
			}
			
			loCarry.swap(loCounter[liI]);         
			if ( liI == liFill ) ++liFill;
		}

		for ( int liI = 1; liI < liFill; ++liI ) 
		{
		    loCounter[liI].merge( loCounter[liI-1], afComp );
		}
		this->swap( loCounter[liFill-1] );
	}
    
    void swap( self& aoX )
    {
        AI_STD::swap( this->cpoNode, aoX.cpoNode ); 
    }
  
    iterator begin()                      { return this->cpoNode->cpoNext; }
	const_iterator begin() const          { return this->cpoNode->cpoNext; }

    iterator end()                        { return this->cpoNode; }
	const_iterator end() const            { return this->cpoNode; }
	
	reverse_iterator rbegin()             { return reverse_iterator(this->cpoNode->cpoPrev); }
	const_reverse_iterator rbegin() const { return const_reverse_iterator(this->cpoNode->cpoPrev); }

	reverse_iterator rend()               { return reverse_iterator(this->cpoNode); }
	const_reverse_iterator rend() const   { return const_reverse_iterator(this->cpoNode); }

    bool empty() const                    { return this->cpoNode->cpoNext == this->cpoNode; }
	size_t size() const                   { return this->distance(begin(), end()); }
                                         
	reference front()                     { return *begin(); }
	reference back()                      { return *(--end()); }
    void push_back( const T& aoX )        { this->insert( end(), aoX ); }
    void push_front( const T& aoX )       { this->insert( begin(), aoX ); }
    void pop_front()                      { this->erase( begin() ); }
    void pop_back()                       { this->erase( (--end()) ); }

protected:
    void transfer( iterator aoPosition, iterator aoFirst, iterator aoLast )
    {
        if ( aoPosition != aoLast )
        {
            aoLast.cpoNode->cpoPrev->cpoNext  = aoPosition.cpoNode;
            aoFirst.cpoNode->cpoPrev->cpoNext = aoLast.cpoNode;
            aoPosition.cpoNode->cpoPrev->cpoNext = aoFirst.cpoNode;
            link_type lpoTmp = aoPosition.cpoNode->cpoPrev;
            aoPosition.cpoNode->cpoPrev = aoLast.cpoNode->cpoPrev;
            aoLast.cpoNode->cpoPrev = aoFirst.cpoNode->cpoPrev;
            aoFirst.cpoNode->cpoPrev = lpoTmp;
        }
    }

    size_t distance( const_iterator aoFirst, const_iterator aoLast ) const
    {
        size_t liSize = 0;
        while( aoFirst != aoLast )
        {
            ++liSize;
            ++aoFirst;
        }
        return liSize;
    }
};

END_STLNAMESPACE

#endif // __AILIB__LIST_H__
