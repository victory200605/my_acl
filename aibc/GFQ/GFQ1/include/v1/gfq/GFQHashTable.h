
#ifndef AI_HASH_TABLE_H_2008
#define AI_HASH_TABLE_H_2008

#include "GFQUtility.h"

// Start Hash function define
template <class TKey> struct stHash {}; // hash class

// Hash for string
template <> struct stHash<const char*>
{
	size_t operator()( const char* apcString ) const 
	{ 
		assert( apcString != NULL );
		//unsigned long llH = 0; 
		size_t aiLength = strlen(apcString);
		unsigned int	liRetValue = 1;
		unsigned int	liTempValue = 4;
		
		while( aiLength-- ) /* from MySQL */
		{ 
			liRetValue ^= (((liRetValue & 63)+liTempValue)*((unsigned int)(unsigned char) *apcString++))+ (liRetValue << 8);
			liTempValue += 3; 
		} 
		
		return liRetValue; 
	}
};
// Hash for int
template <> struct stHash<int>
{
	size_t operator()(int aN ) const { return aN; }
};
// Hash for unsigned int
template <> struct stHash<unsigned int>
{
	size_t operator()(unsigned int aN) const { return aN; }
};
// Hash for short
template <> struct stHash<short>
{
	size_t operator()(short aN) const { return aN; }
};
// Hash for double
template <> struct stHash<double>
{
	double operator()(double aN) const { return aN; }
};
// End hash function define

// Primes array, defined for bucket number
static const int _NumPrimes = 28;
static const unsigned long _PrimeList[ _NumPrimes ] = {               \
	53ul,         97ul,         193ul,       389ul,       769ul,      \
	1543ul,       3079ul,       6151ul,      12289ul,     24593ul,    \
	49157ul,      98317ul,      196613ul,    393241ul,    786433ul,   \
	1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul, \
	50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul,\
	1610612741ul, 3221225473ul, 4294967291ul  \
};

// Get next prime, To find leftmost prime
inline unsigned long _NextPrime ( unsigned long n )
{
	const unsigned long* lpiFirst = _PrimeList;
	const unsigned long* lpiLast  = _PrimeList + _NumPrimes;
	
	size_t liLen = lpiLast - lpiFirst;
	size_t liHalf;
	const unsigned long* lpiMiddle;
	while ( liLen > 0 )
	{
		liHalf = liLen >> 1;
		lpiMiddle = lpiFirst + liHalf;
		if ( *lpiMiddle < n )
		{
			lpiFirst = lpiMiddle + 1;
			liLen = liLen - liHalf - 1;
		}
		else
		{
			liLen = liHalf;
		}
	}

	return lpiFirst == lpiLast ? *( lpiLast - 1 ) : *lpiFirst;
}

// Forward declare
template< class TKey, class TValue, class THashFnc > class clsHashTable;

// Hash table single iterator
template< class TKey, class TValue, class THashFnc >
class clsIterator
{
public:
	friend class clsHashTable<TKey, TValue, THashFnc>;
	
public:
	clsIterator( TKey, TValue* aptValue ) : cptValue(aptValue) {}
	
	bool operator == ( const clsIterator& aoIter )
	{
		return cptValue == aoIter.cptValue;
	}
	
	TValue* operator -> ()
	{
		return &(**this);
	}
	
	TValue& operator * ()
	{
		return *cptValue;
	}
	
	operator TValue* ()
	{
		return cptValue;
	}

protected:
	TValue* cptValue;
};

// Template hash table, Dependency by DualLink
template< class TKey, class TValue, class THashFnc = stHash<TKey> >
class clsHashTable
{
public:
	// iterator typedef
	typedef clsIterator<TKey, TValue, THashFnc> iterator;
	
	// Hash table hash head struct
	struct stHashHead
	{
		stHashHead()
		{
			ai_init_list_head(&coNodeHead);
		}
		
		LISTHEAD coNodeHead;
	};
	
	// Hash table hash node struct
	struct stHashNode : public TValue
	{
		LISTHEAD _coNodeChild;
	};
	
public:
	clsHashTable() : 
		cpoBucket(NULL),
		ciBktSize(0),
		ciNumElem(0)
	{
		InitializeBucket( ciNumElem );
	}
	
	~clsHashTable( )
	{
		Clear();
		AI_DELETE_ARRAY( cpoBucket );
	}
	
	void Swap( clsHashTable& aoR )
	{
		Swap( cpoBucket, aoR.cpoBucket );
		Swap( ciBktSize, aoR.ciBktSize );
		Swap( ciNumElem, aoR.ciNumElem );
	}
	
	// Insert hash node by pointer
	iterator Insert( TValue* aptValue )
	{
		// Resize hash table, reset size search from prime list by _NextPrime function
		Resize( ciNumElem + 1 );
		
		ciNumElem++;
		
		// Get node bucket number
		size_t liBucket = BktNum( aptValue->GetKey() ); 
		
		// Add to hash link
		ai_list_add_tail( &(((stHashNode*)aptValue)->_coNodeChild), &(cpoBucket[liBucket].coNodeHead) );
		
		// Return current iterator
		return iterator( TKey(), ((stHashNode*)aptValue) );
	}
	
	// Find hash node from hash table
	iterator Find( TKey atKey )
	{
		// Get node bucket number
		size_t liBucket = BktNum( atKey );
		
		// Get node hash link
		LISTHEAD* lpoHead = &(cpoBucket[liBucket].coNodeHead);
		LISTHEAD* lpoNode = cpoBucket[liBucket].coNodeHead.cpNext;
		for ( ; lpoNode != NULL && lpoNode != lpoHead; lpoNode = lpoNode->cpNext )
		{//poll hash link to search
			stHashNode* lpoHashNode = AI_GET_STRUCT_PTR( lpoNode , stHashNode, _coNodeChild );
			
			// Node must define Compare function
			if ( lpoHashNode->Compare(atKey) == 0 )
			{
				//Yes, i found it
				return iterator( atKey, lpoHashNode );
			}
		}
		
		// Return node iterator
		return iterator( atKey, NULL );
	}
	
	// Erase node
	void Erase( iterator atIter )
	{
		Erase( atIter.cptValue );
	}
	
	void Erase( TValue* aptValue )
	{
		if ( aptValue == NULL ) return;
		
		// Convert node type
		stHashNode* lpoNode = (stHashNode*)(aptValue);
		LISTHEAD* lpoLinkNode = &lpoNode->_coNodeChild;
		// Unlink node self
		if ( lpoLinkNode->cpPrev != NULL && lpoLinkNode->cpNext != NULL )
		{
		    ciNumElem++;
			lpoLinkNode->cpPrev->cpNext = lpoLinkNode->cpNext;
	    	lpoLinkNode->cpNext->cpPrev = lpoLinkNode->cpPrev;
	    	lpoLinkNode->cpPrev = NULL;
	    	lpoLinkNode->cpNext = NULL;
		}
		// Recycle node
		AI_DELETE( lpoNode );
	}
	
	iterator End()
	{
		// End iterator
		return iterator( TKey(), NULL );
	}
	
	void Clear()
	{
		for ( size_t liN = 0; liN < ciBktSize; liN++ )
		{// Clear all
			LISTHEAD* lpoHead = &cpoBucket[liN].coNodeHead;
			while ( !ai_list_is_empty(lpoHead) )
			{
				LISTHEAD* lpoTmp = NULL;
				ai_list_del_head( lpoTmp, lpoHead );
				stHashNode* lpoNode = AI_GET_STRUCT_PTR( lpoTmp , stHashNode, _coNodeChild );
				AI_DELETE( lpoNode );
			}
		}
		
		ciNumElem = 0;
	}
	
	size_t Size()
	{
	    return ciNumElem;
	}
	
	// Hash table node must use this function to create
	static TValue* CreateNode()
	{
		stHashNode* lpoHashNode = NULL;
		AI_NEW_ASSERT( lpoHashNode, stHashNode );
		return lpoHashNode;
	}
	// Hash table node must use this function to destory
	static void DestoryNode( TValue* aptValue )
	{
		stHashNode* lpoHashNode = (stHashNode*)aptValue;
		AI_DELETE( lpoHashNode );
	}
	
protected:
	template< class T >
    void Swap( T& l, T& r )
    {
        T ltTemp(l);
        l = r;
        r = ltTemp;
    }
    
	// Get next prime for size
	size_t NextSize( size_t aiN )               { return _NextPrime(aiN); }
	
	void InitializeBucket( size_t aiN )
	{
		ciBktSize = NextSize(aiN);
		
		AI_NEW_ARRAY_ASSERT( cpoBucket, stHashHead, ciBktSize );

		ciNumElem = 0;
	}
	
	// Resize bucket
	void Resize( size_t liNum )
	{
		size_t liBktSize = NextSize( liNum );
		size_t liOld = ciBktSize;

		if ( liBktSize > liOld )
		{
			// Set new size
			ciBktSize = liBktSize;
		
			// New tmp bucket for new size
			stHashHead* lpoTmpBkt = NULL;
			AI_NEW_ARRAY_ASSERT( lpoTmpBkt, stHashHead, liBktSize );

			for ( size_t liN = 0; liN < liOld; liN++ )
			{// move node from old bucket
				LISTHEAD* lpoHead = &cpoBucket[liN].coNodeHead;
				while ( !ai_list_is_empty(lpoHead) )
				{
					LISTHEAD* lpoTmp = NULL;
					ai_list_del_head( lpoTmp, lpoHead );
					stHashNode* lpoNode = AI_GET_STRUCT_PTR( lpoTmp , stHashNode, _coNodeChild );
					size_t liNewBucket = BktNum( lpoNode->GetKey() );
					ai_list_add_tail( &(lpoNode->_coNodeChild), &(lpoTmpBkt[liNewBucket].coNodeHead) );
				}
			}
			
			AI_DELETE_ARRAY( cpoBucket );
			cpoBucket = lpoTmpBkt;
		}
	}
	
	size_t BktNum( TKey atKey )
	{
		return coHash( atKey ) % ciBktSize;
	}
	
protected:
	stHashHead* cpoBucket;
	size_t      ciBktSize;
	
	size_t ciNumElem;
	
	THashFnc coHash;
};

#endif //AI_HASH_TABLE_H_2008
