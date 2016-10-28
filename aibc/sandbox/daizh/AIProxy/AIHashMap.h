
#ifndef __AILIB__HASH_MAP_DAULLINK_H__
#define __AILIB__HASH_MAP_DAULLINK_H__

#include "stl/string.h"
#include "stl/vector.h"
#include "stl/algorithm.h"
#include "AIDualLink.h"
#include <assert.h>

#if !defined(HASH_BUCKET_SIZE)
#define HASH_BUCKET_SIZE 50
#endif

/**
 * @brief Hesh function
 */
template <class TKey> struct hash { };

inline size_t _hash_string(const char* apsPtr )
{
	assert(apsPtr != NULL );
	unsigned long llH = 0; 
	for ( ; *apsPtr; ++apsPtr)
		llH = 5*llH + *apsPtr;

	return size_t(llH);
}

template <> struct hash<const char*>
{
	size_t operator()(const char* __s) const { assert(__s != NULL ); return _hash_string(__s); }
};
template <> struct hash<const AI_STD::string>
{
	size_t operator()(const AI_STD::string& __s) const { return _hash_string(__s.c_str()); }
};
template <> struct hash<AI_STD::string>
{
	size_t operator()(const AI_STD::string& __s) const { return _hash_string(__s.c_str()); }
};
template <> struct hash<int>
{
	size_t operator()(int __s) const { return __s; }
};
template <> struct hash<unsigned int>
{
	size_t operator()(unsigned int __s) const { return __s; }
};
template <> struct hash<short>
{
	size_t operator()(short __s) const { return __s; }
};
template <> struct hash<double>
{
	double operator()(double __s) const { return __s; }
};


static const int _num_primes = 28;
static const unsigned long _prime_list[ _num_primes ] = { \
53ul,         97ul,         193ul,       389ul,       769ul,      \
1543ul,       3079ul,       6151ul,      12289ul,     24593ul,    \
49157ul,      98317ul,      196613ul,    393241ul,    786433ul,   \
1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul, \
50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul,\
1610612741ul, 3221225473ul, 4294967291ul  \
};

inline unsigned long _next_prime ( unsigned long n )
{
	const unsigned long* first = _prime_list;
	const unsigned long* last  = _prime_list + _num_primes;
	const unsigned long* pos = AI_STD::lower_bound( first, last, n );

	return pos == last ? *( last - 1 ) : *pos;
}

/**
* @brief Hesh Node
*/
template< class T >
struct stHashNode
{
	stHashNode( T t ):ctVal(t)
	{
	    ::memset( &this->coChild, 0, sizeof(LISTHEAD) );
	    ::memset( &this->coTimeChild, 0, sizeof(LISTHEAD) );
	}
	T ctVal;
	LISTHEAD coChild;
	LISTHEAD coTimeChild;
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
	clsHashTableIterator( LISTHEAD* apoListNode );
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
	LISTHEAD* cpoListNode;
};

template< class TValue >
clsHashTableIterator<TValue>::clsHashTableIterator() :
    cpoListNode(NULL)
{
}

template< class TValue >
clsHashTableIterator<TValue>::clsHashTableIterator( LISTHEAD* apoListNode ) :
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
			ai_init_list_head(&coHead);
		}
		stHashHead( const stHashHead& aoRhs )
		{
			ai_init_list_head(&coHead);
		}
		
		LISTHEAD coHead;
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
		LISTHEAD* lpoFirst = &cvBucket[liN].coHead;
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
		LISTHEAD* lpoFirst = &cvBucket[liN].coHead;
		size_type liErased = 0;
        
        for ( LISTHEAD* lpoCurr = lpoFirst->cpNext; lpoCurr != lpoFirst; )
        {
            LISTHEAD* lpoTmp = lpoCurr;
			lpoCurr = lpoCurr->cpNext;
			
            node* lpoNode = AI_GET_STRUCT_PTR( lpoTmp, node, coChild );
            if ( coGetKey(lpoNode->ctVal) == atKey )
			{
				::ai_list_del_any( lpoTmp );
				::ai_list_del_any( &lpoNode->coTimeChild );
				    
				AI_DELETE( lpoNode );
				liErased++;
			}
        }
        
		return liErased;
	}

	void erase( iterator aiIt )
	{
	    node* lpoNode = aiIt.node_ptr();
	    ::ai_list_del_any( &lpoNode->coChild );
        ::ai_list_del_any( &lpoNode->coTimeChild );
		AI_DELETE( lpoNode );
	}

	void clear()
	{
		for ( size_type liN = 0; liN < cvBucket.size(); liN++ )
		{
            LISTHEAD* lpoFirst = &cvBucket[liN].coHead;
            while( !::ai_list_is_empty(lpoFirst) )
			{
			    LISTHEAD* lpoTmpNode = NULL;
				::ai_list_del_head( lpoTmpNode, lpoFirst );
				node* lpoCurr = AI_GET_STRUCT_PTR( lpoTmpNode, node, coChild );
                ::ai_list_del_any( &lpoCurr->coTimeChild );
                
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
				LISTHEAD* lpoFirst = &cvBucket[liN].coHead;
				while( !::ai_list_is_empty(lpoFirst) )
				{
				    LISTHEAD* lpoTmpNode = NULL;
				    ::ai_list_del_head( lpoTmpNode, lpoFirst );
				    node* lpoCurr = AI_GET_STRUCT_PTR( lpoTmpNode, node, coChild );
				    
				    size_type liNewBucket = bkt_num( lpoCurr->ctVal, liBktSize );
					::ai_list_add_tail( &lpoCurr->coChild, &lvTmp[liNewBucket].coHead );
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
		::ai_list_add_tail( &lptTmp->coChild, &cvBucket[liN].coHead );
		::ai_list_add_tail( &lptTmp->coTimeChild, &coTimeHead.coHead );
		    
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
class AIHashMap
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
	typedef AIHashMap<TKey, TValue, THashFnc> self;
	typedef Pair<TKey, TValue> value_type;
	typedef TKey key_type;
	typedef unsigned long size_type;

	AIHashMap( ) : coHashTable(HASH_BUCKET_SIZE) {};

	AIHashMap( size_type aiN ) : coHashTable(aiN) {};

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

#endif //__AILIB__HASH_MAP_DAULLINK_H__
