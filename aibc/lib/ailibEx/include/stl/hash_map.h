//=============================================================================
/**
 * \file    hash_map.h
 * \brief reference STL hash_map standard interface
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: hash_map.h,v 1.4 2010/04/08 14:46:14 daizh Exp $
 *
 * History
 * 2008.02.28 first release by DZH
 */
//=============================================================================

#ifndef __AILIB__HASH_MAP_H__
#define __AILIB__HASH_MAP_H__

#include "string.h"
#include "vector.h"
#include "algorithm.h"
#include <assert.h>

START_STLNAMESPACE

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
	const unsigned long* pos = lower_bound( first, last, n );

	return pos == last ? *( last - 1 ) : *pos;
}

/**
* @brief Hesh Node
*/
template< class T >
struct hash_node
{
	hash_node( T t ):ctVal(t),cpoNext(0)  { }
	T ctVal;
	hash_node* cpoNext;
};

template< class TValue, class TExtractKey, class THashFnc > class hash_table;

/**
* @brief Hesh Iterator, reference from STL resouce
*/
template< class TValue, class TExtractKey, class THashFnc >
class hash_table_iterator
{
public:
	friend class hash_table<TValue, TExtractKey, THashFnc>;
	typedef hash_node<TValue> node;
	typedef hash_table<TValue, TExtractKey, THashFnc> hash_table_type;
	typedef hash_table_iterator<TValue, TExtractKey, THashFnc> iterator;
	typedef unsigned long size_type;
	typedef TValue&   reference;
	typedef TValue*   point;

	hash_table_iterator( );
	hash_table_iterator( node* apn, hash_table_type* apt );

	reference operator * ()                        { return cpoCurr->ctVal;}
	point operator -> ()                           { return &( operator * () );}
	iterator& operator ++ ( );
	iterator operator ++ ( int );
	bool operator == ( iterator it )              { return cpoCurr == it.cpoCurr; }                           
	bool operator != ( iterator it )              { return cpoCurr != it.cpoCurr; }   

protected:
	node*  cpoCurr;
	hash_table_type* cpoHT;
};
template< class TValue, class TExtractKey, class THashFnc >
hash_table_iterator<TValue, TExtractKey, THashFnc>::hash_table_iterator( ):
cpoCurr(NULL),
cpoHT(NULL)
{
}
template< class TValue, class TExtractKey, class THashFnc >
hash_table_iterator<TValue, TExtractKey, THashFnc>::hash_table_iterator( node* apn, hash_table_type* apt )
{
	cpoCurr = apn;
	cpoHT   = apt;
}
template< class TValue, class TExtractKey, class THashFnc >
hash_table_iterator<TValue, TExtractKey, THashFnc>& hash_table_iterator<TValue, TExtractKey, THashFnc>::operator ++ ( )
{
	const node* lpOld = cpoCurr;
	cpoCurr = cpoCurr->cpoNext;
	if ( !cpoCurr )
	{
		size_type liN = cpoHT->bkt_num( lpOld->ctVal, cpoHT->bucket_size() );
		while( !cpoCurr && ++liN < cpoHT->bucket_size() )
		{
			cpoCurr = cpoHT->cvBucket[liN];
		}
	}
	return *this;
}
template< class TValue, class TExtractKey, class THashFnc >
hash_table_iterator<TValue, TExtractKey, THashFnc> hash_table_iterator<TValue, TExtractKey, THashFnc>::operator ++ ( int )
{
	iterator liTmp = *this;
	++(*this);
	return liTmp;
}

//-----------------------------------------hash table----------------------------------------------//
/**
* @brief Hesh Table, reference from STL resouce
*/
template< class TValue, class TExtractKey, class THashFnc >
class hash_table
{
public:
	typedef hash_table_iterator<TValue, TExtractKey, THashFnc> iterator;
	friend class hash_table_iterator<TValue, TExtractKey, THashFnc>;
	typedef unsigned long size_type;
	typedef hash_node<TValue> node;
	typedef hash_table<TValue, TExtractKey, THashFnc> self;
	typedef TValue                 value_type;
	typedef typename TExtractKey::key_type                   key_type;

	hash_table( int aibkt_num )
	{
		initialize_bucket( aibkt_num );
	}

    ~hash_table()
	{
	    clear();
	}
	
	hash_table( const hash_table& aOther )
	{
	    copy_from( aOther );
	    coHash = aOther.coHash;
	}
	
	iterator insert( const value_type& atVaue )
	{
		resize( cnNumElem + 1 );
		return inser_unique( atVaue );
	}

	iterator find( const key_type& atKey )
	{
		const size_type liN = coHash( atKey ) % cvBucket.size();
		node* lptFirst = cvBucket[liN];
		for ( node* lptCur = lptFirst; lptCur; lptCur = lptCur->cpoNext )
		{
			if ( coGetKey(lptCur->ctVal) == atKey )
			{
				return iterator( lptCur, this );
			}
		}
		return end();
	} 

	size_type erase( const key_type& atKey )
	{
		const size_type liN = coHash( atKey ) % cvBucket.size();
		node* lptFirst = (node*)cvBucket[liN];
		size_type liErased = 0;

		if ( lptFirst ) 
		{
			node* lptCur = lptFirst;
			node* lptNext = lptCur->cpoNext;
			while ( lptNext ) 
			{
				if ( coGetKey( lptNext->ctVal ) == atKey )
				{
					lptCur->cpoNext = lptNext->cpoNext;
					delete lptNext;
					lptNext = lptCur->cpoNext;
					++liErased;
					--cnNumElem;
				}
				else {
					lptCur = lptNext;
					lptNext = lptCur->cpoNext;
				}
			}
			if ( coGetKey( lptFirst->ctVal ) == atKey )
			{
				cvBucket[liN] = lptFirst->cpoNext;
				delete lptFirst;
				++liErased;
				--cnNumElem;
			}
		}
		return liErased;
	}

	void erase( iterator aiIt )
	{
		const node* lpoPtr = aiIt.cpoCurr;
		if ( lpoPtr ) 
		{
			const size_type liN = bkt_num( *aiIt, cvBucket.size() );
			node* lpoCur = (node*)cvBucket[liN];

			if ( lpoCur == lpoPtr ) 
			{
				cvBucket[liN] = lpoCur->cpoNext;
				delete lpoCur;
				--cnNumElem;
			}
			else 
			{
				node* lpoNext = lpoCur->cpoNext;
				while ( lpoNext ) 
				{
					if ( lpoNext == lpoPtr ) 
					{
						lpoCur->cpoNext = lpoNext->cpoNext;
						delete lpoNext;
						--cnNumElem;
						break;
					}
					else 
					{
						lpoCur = lpoNext;
						lpoNext = lpoCur->cpoNext;
					}
				}
			}
		}
	}

	void clear()
	{
		for ( size_type liIt = 0; liIt < cvBucket.size(); liIt++ )
		{
			node* ltCur = cvBucket[liIt];
			while( ltCur != 0 )
			{
				node* ltNext = ltCur->cpoNext;
				delete ltCur;
				ltCur = ltNext;
			}
			cvBucket[liIt] = 0;
		}
		cnNumElem = 0;
	}
	void copy_from( const hash_table& aoHT )
	{
		clear();
		size_type liBktSize = aoHT.cvBucket.size();
        cvBucket.clear();
		cvBucket.reserve( liBktSize );
		cvBucket.insert( cvBucket.end(), liBktSize, (node*)0 );
		for ( size_type liIt = 0; liIt < liBktSize; ++liIt )
		{
			if ( const node* ltCur = aoHT.cvBucket[liIt] )
			{
				node* lptCopy = new node( ltCur->ctVal );
				cvBucket[liIt] = lptCopy;
				for ( node* ltNext = ltCur->cpoNext; ltNext; ltCur = ltNext, ltNext = ltCur->cpoNext )
				{
					lptCopy->cpoNext = new node( ltNext->ctVal );
					lptCopy = lptCopy->cpoNext;
				}
			}
		}
		cnNumElem = aoHT.cnNumElem;
	}

	size_type size()                                 { return cnNumElem; }
	size_type bucket_size()                          { return cvBucket.size(); }

	iterator begin()
	{ 
		for ( size_type liN = 0; liN < cvBucket.size(); ++liN )
		{
			if (cvBucket[liN])
			{
				return iterator((node*)cvBucket[liN], this);
			}
		}
		return end();
	}
	iterator end() { return iterator((node*)0, this); }

protected:
	size_type next_size( size_type aiN )               { return _next_prime(aiN); }
	void initialize_bucket( size_type aiN )            
	{
		size_type liBktSize = next_size(aiN);
		cvBucket.reserve( liBktSize );
		cvBucket.insert( cvBucket.end(), liBktSize, (node*)0 );
		cnNumElem = 0;
	}
	void resize( size_type liNum )
	{
		size_type liBktSize = next_size( liNum );
		size_type liOld = cvBucket.size();
		if ( liBktSize > liOld )
		{
			vector<node*> lvTmp( liBktSize, (node*)0 );
			for ( size_type liN = 0; liN < liOld; liN++ )
			{
				node* lptFirst = cvBucket[liN];
				while( lptFirst )
				{
					size_type liNewBucket = bkt_num( lptFirst->ctVal, liBktSize );
					cvBucket[liN] = lptFirst->cpoNext;
					lptFirst->cpoNext = lvTmp[liNewBucket];
					lvTmp[liNewBucket] = lptFirst;
					lptFirst = cvBucket[liN];
				}
			}
			cvBucket.swap( lvTmp );
		}
	}

	iterator inser_unique( const value_type& atValue )
	{
		const size_type liN = bkt_num( atValue, cvBucket.size() );
		node* lptFirst = cvBucket[liN];
		for ( node* lptCur = lptFirst; lptCur; lptCur = lptCur->cpoNext )
		{
			if ( lptCur->ctVal == atValue )
			{
				return iterator( lptCur, this );
			}
		}
		node* lptTmp = new node(atValue);
		lptTmp->cpoNext = lptFirst;
		cvBucket[liN] = lptTmp;
		++cnNumElem;
		return iterator( lptTmp, this );
	}

	size_type bkt_num( value_type atValue, size_type aiN )
	{
		return coHash( coGetKey(atValue) ) % aiN;
	}

protected:
	AI_STD::vector<node*>  cvBucket;
	size_type           cnNumElem;
	THashFnc            coHash;
	TExtractKey         coGetKey;
};

/**
* @brief Hesh Map, reference from STL resouce
*/
template< class TKey, class TValue, class THashFnc = hash<TKey> >
class hash_map
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

	typedef hash_table_iterator< Pair<TKey, TValue>, Pair<TKey, TValue>, THashFnc > iterator;
	typedef hash_table< Pair<TKey, TValue>, Pair<TKey, TValue>, THashFnc > hash_table_type;
	typedef hash_map<TKey, TValue, THashFnc> self;
	typedef Pair<TKey, TValue> value_type;
	typedef TKey key_type;
	typedef unsigned long size_type;

	hash_map( ) : coHashTable(HASH_BUCKET_SIZE) {};

	hash_map( size_type aiN ) : coHashTable(aiN) {};

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
    self& operator = ( const hash_map& aoHM )
    {
        coHashTable.copy_from( aoHM.coHashTable );
        return *this;
    }

	void clear()                     { coHashTable.clear(); }
	size_type size()                 { return coHashTable.size(); }

	iterator begin()                 { return coHashTable.begin(); }
	iterator end()                   { return coHashTable.end(); }

protected:
	hash_table_type coHashTable;
};

END_STLNAMESPACE

#endif //__AILIB__HASH_MAP_H__
