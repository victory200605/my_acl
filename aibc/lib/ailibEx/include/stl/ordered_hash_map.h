
#ifndef __AILIB__HASH_MAP_DAULLINK_H__
#define __AILIB__HASH_MAP_DAULLINK_H__

#include "string.h"
#include "vector.h"
#include "algorithm.h"
#include "hash_map.h"
#include "../AIDualLink.h"
#include <assert.h>

START_STLNAMESPACE

/**
* @brief Hesh Node
*/
template< class T >
struct stHashNode
{
	stHashNode( T t ):ctVal(t)
	{
	    ::memset( &this->coChild, 0, sizeof(AIBC::LISTHEAD) );
	    ::memset( &this->coTimeChild, 0, sizeof(AIBC::LISTHEAD) );
	}
	T ctVal;
	AIBC::LISTHEAD coChild;
	AIBC::LISTHEAD coTimeChild;
};

template< class TValue, class TExtractKey, class THashFnc > class clsHashTable;

/**
* @brief Hesh Iterator, reference from STL resouce
*/
template< class TValue >
class clsHashTableIterator
{
public:
	typedef stHashNode<TValue> node;
	typedef clsHashTableIterator<TValue> iterator;
	typedef unsigned long size_type;
	typedef TValue&   reference;
	typedef TValue*   point;
    
    clsHashTableIterator();
	clsHashTableIterator( AIBC::LISTHEAD* apoListNode );
	clsHashTableIterator( const clsHashTableIterator& aoRhs );
	clsHashTableIterator& operator = ( const clsHashTableIterator& aoRhs );

	reference operator * ();
	point operator -> ()                           { return &( operator * () );}
	iterator& operator ++ ( );
	iterator operator ++ ( int );
	bool operator == ( iterator it )               { return cpoListNode == it.cpoListNode; }                           
	bool operator != ( iterator it )               { return cpoListNode != it.cpoListNode; }
	
	node* node_ptr();

protected:
	AIBC::LISTHEAD* cpoListNode;
};

template< class TValue >
clsHashTableIterator<TValue>::clsHashTableIterator() :
    cpoListNode(NULL)
{
}

template< class TValue >
clsHashTableIterator<TValue>::clsHashTableIterator( AIBC::LISTHEAD* apoListNode ) :
    cpoListNode(apoListNode)
{
}

template< class TValue >
clsHashTableIterator<TValue>::clsHashTableIterator( const clsHashTableIterator& aoRhs )
{
    *this = aoRhs;
}

template< class TValue >
clsHashTableIterator<TValue>& clsHashTableIterator<TValue>::operator = ( const clsHashTableIterator& aoRhs )
{
    this->cpoListNode = aoRhs.cpoListNode;
    return *this;
}

template< class TValue >
typename clsHashTableIterator<TValue>::reference clsHashTableIterator<TValue>::operator * () 
{
    return this->node_ptr()->ctVal;
}

template< class TValue >
clsHashTableIterator<TValue>& clsHashTableIterator<TValue>::operator ++ ( )
{
    this->cpoListNode = this->cpoListNode->cpNext;
	return *this;
}

template< class TValue >
clsHashTableIterator<TValue> clsHashTableIterator<TValue>::operator ++ ( int )
{
	iterator liTmp = *this;
	++(*this);
	return liTmp;
}

template< class TValue >
typename clsHashTableIterator<TValue>::node* clsHashTableIterator<TValue>::node_ptr()
{
    return AI_GET_STRUCT_PTR( this->cpoListNode, node, coTimeChild );
}

//-----------------------------------------hash table----------------------------------------------//
/**
 * @brief Hesh Table, reference from STL resouce
 */
template< class TValue, class TExtractKey, class THashFnc >
class clsHashTable
{
public:
    // Hash table hash head struct
	struct stHashHead
	{
		stHashHead()
		{
			AIBC::ai_init_list_head(&coHead);
		}
		stHashHead( const stHashHead& aoRhs )
		{
			AIBC::ai_init_list_head(&coHead);
		}
		
		AIBC::LISTHEAD coHead;
	};
	
public:
	typedef clsHashTableIterator<TValue> iterator;
	typedef unsigned long size_type;
	typedef stHashNode<TValue> node;
	typedef clsHashTable<TValue, TExtractKey, THashFnc> self;
	typedef TValue                 value_type;
	typedef typename TExtractKey::key_type  key_type;

	clsHashTable( int aibkt_num )
	{
		initialize_bucket( aibkt_num );
	}

    ~clsHashTable()
	{
	    clear();
	}

	iterator insert( const value_type& atVaue )
	{
		resize( cnNumElem + 1 );
		return inser_unique( atVaue );
	}

	iterator find( const key_type& atKey )
	{
		const size_type liN = coHash( atKey ) % cvBucket.size();
		AIBC::LISTHEAD* lpoFirst = &cvBucket[liN].coHead;
		AI_FOR_EACH_IN_LIST( lpoCurr, lpoFirst, node, coChild )
		{
		    if ( coGetKey(lpoCurr->ctVal) == atKey )
			{
				return iterator( &lpoCurr->coTimeChild );
			}
		}
		return end();
	} 

	size_type erase( const key_type& atKey )
	{
		const size_type liN = coHash( atKey ) % cvBucket.size();
		AIBC::LISTHEAD* lpoFirst = &cvBucket[liN].coHead;
		size_type liErased = 0;
        
        for ( AIBC::LISTHEAD* lpoCurr = lpoFirst->cpNext; lpoCurr != lpoFirst; )
        {
            AIBC::LISTHEAD* lpoTmp = lpoCurr;
			lpoCurr = lpoCurr->cpNext;
			
            node* lpoNode = AI_GET_STRUCT_PTR( lpoTmp, node, coChild );
            if ( coGetKey(lpoNode->ctVal) == atKey )
			{
				AIBC::ai_list_del_any( lpoTmp );
				AIBC::ai_list_del_any( &lpoNode->coTimeChild );
				    
				AI_DELETE( lpoNode );
				liErased++;
				
				--cnNumElem;
			}
        }
        
		return liErased;
	}

	void erase( iterator aiIt )
	{
	    node* lpoNode = aiIt.node_ptr();
	    AIBC::ai_list_del_any( &lpoNode->coChild );
        AIBC::ai_list_del_any( &lpoNode->coTimeChild );
		AI_DELETE( lpoNode );
		
		--cnNumElem;
	}

	void clear()
	{
		for ( size_type liN = 0; liN < cvBucket.size(); liN++ )
		{
            AIBC::LISTHEAD* lpoFirst = &cvBucket[liN].coHead;
            while( !AIBC::ai_list_is_empty(lpoFirst) )
			{
			    AIBC::LISTHEAD* lpoTmpNode = NULL;
				AIBC::ai_list_del_head( lpoTmpNode, lpoFirst );
				node* lpoCurr = AI_GET_STRUCT_PTR( lpoTmpNode, node, coChild );
                AIBC::ai_list_del_any( &lpoCurr->coTimeChild );
                
				AI_DELETE( lpoCurr );
			}
		}
		cnNumElem = 0;
	}

	size_type size()                                 { return cnNumElem; }
	size_type bucket_size()                          { return cvBucket.size(); }

	iterator begin()
	{ 
	    return iterator( coTimeHead.coHead.cpNext );
	}
	
	iterator end() { return iterator( &coTimeHead.coHead ); }

protected:
    clsHashTable( const clsHashTable& aOther )
	{
	}
	
	clsHashTable& operator = ( const clsHashTable& aOther )
	{
	}
	
	size_type next_size( size_type aiN )               { return _next_prime(aiN); }
	void initialize_bucket( size_type aiN )            
	{
		size_type liBktSize = next_size(aiN);
		cvBucket.resize( liBktSize );
		cnNumElem = 0;
	}
	
	void resize( size_type liNum )
	{
		size_type liBktSize = next_size( liNum );
		size_type liOld = cvBucket.size();
		if ( liBktSize > liOld )
		{
			AI_STD::vector<stHashHead> lvTmp;
			lvTmp.resize(liBktSize);
			for ( size_type liN = 0; liN < liOld; liN++ )
			{
				AIBC::LISTHEAD* lpoFirst = &cvBucket[liN].coHead;
				while( !AIBC::ai_list_is_empty(lpoFirst) )
				{
				    AIBC::LISTHEAD* lpoTmpNode = NULL;
				    AIBC::ai_list_del_head( lpoTmpNode, lpoFirst );
				    node* lpoCurr = AI_GET_STRUCT_PTR( lpoTmpNode, node, coChild );
				    
				    size_type liNewBucket = bkt_num( lpoCurr->ctVal, liBktSize );
					AIBC::ai_list_add_tail( &lpoCurr->coChild, &lvTmp[liNewBucket].coHead );
				}
			}
			cvBucket.swap( lvTmp );
		}
	}

	iterator inser_unique( const value_type& atValue )
	{
		const size_type liN = bkt_num( atValue, cvBucket.size() );
		AI_FOR_EACH_IN_LIST( lpoCurr, &cvBucket[liN].coHead, node, coChild )
		{
		    if ( lpoCurr->ctVal == atValue )
			{
				return iterator( &lpoCurr->coTimeChild );
			}
		}
		
		node* lptTmp = NULL;
		AI_NEW_INIT_ASSERT( lptTmp, node, atValue );
		AIBC::ai_list_add_tail( &lptTmp->coChild, &cvBucket[liN].coHead );
		AIBC::ai_list_add_tail( &lptTmp->coTimeChild, &coTimeHead.coHead );
		    
		++cnNumElem;
		
		return iterator( &lptTmp->coTimeChild );
	}

	size_type bkt_num( value_type atValue, size_type aiN )
	{
		return coHash( coGetKey(atValue) ) % aiN;
	}

protected:
	AI_STD::vector<stHashHead>  cvBucket;
	stHashHead                  coTimeHead;
	
	size_type           cnNumElem;
	THashFnc            coHash;
	TExtractKey         coGetKey;
};

/**
* @brief Hesh Map, reference from STL resouce
*/
template< class TKey, class TValue, class THashFnc = hash<TKey> >
class ordered_hash_map
{
public:
	template< class TK, class TV >
	struct Pair
	{
		typedef TK key_type;

		Pair():first(TK()), second(TV()) {};
		Pair( const TK& k, const TV& v ):first(k), second(v) {}
		TK   first;
		TV   second;
		TK& operator () ( Pair& p )
		{
			return p.first;
		}
		bool operator == ( const Pair& p )
		{
			return first == p.first;
		}
		bool operator != ( const Pair& p )
		{
			return first != p.first;
		}
	};

	typedef clsHashTableIterator< Pair<TKey, TValue> > iterator;
	typedef clsHashTable< Pair<TKey, TValue>, Pair<TKey, TValue>, THashFnc > hash_table;
	typedef ordered_hash_map<TKey, TValue, THashFnc> self;
	typedef Pair<TKey, TValue> value_type;
	typedef TKey key_type;
	typedef unsigned long size_type;

	ordered_hash_map( ) : coHashTable(HASH_BUCKET_SIZE) {};

	ordered_hash_map( size_type aiN ) : coHashTable(aiN) {};

	iterator insert( const value_type& atEl )
	{
		return coHashTable.insert( atEl );
	}
	TValue& operator [] ( const key_type& ak )
	{
		return coHashTable.insert( value_type( ak, TValue() ) )->second;
	}
	iterator find( const key_type& atKey )
	{
		return coHashTable.find( atKey );
	}
	void erase( const iterator& aiIt )
	{
		return coHashTable.erase( aiIt );
	}
	size_type erase( const key_type& atKey )
	{
		return coHashTable.erase( atKey );
	}

	void clear()                     { coHashTable.clear(); }
	size_type size()                 { return coHashTable.size(); }

	iterator begin()                 { return coHashTable.begin(); }
	iterator end()                   { return coHashTable.end(); }

protected:
	hash_table coHashTable;
};

END_STLNAMESPACE

#endif //__AILIB__HASH_MAP_DAULLINK_H__
