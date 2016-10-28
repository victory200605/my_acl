
#ifndef ACL_HASHARRAY_H
#define ACL_HASHARRAY_H

#include "acl/Utility.h"
#include "acl/stl/string.h"

ACL_NAMESPACE_START

namespace {
    
template <class KeyType> struct Hash { };

inline apl_size_t _hash_string( char const* apcPtr )
{
    apl_size_t luH = 0;
    
	for ( ; *apcPtr; ++apcPtr)
	{
	    luH = 5 * luH + *apcPtr;
    }

	return luH;
}

template <> struct Hash<char const*>
{
	apl_size_t operator()(char const* __s) const { return _hash_string(__s); }
};

template <> struct Hash<std::string const>
{
	apl_size_t operator()(std::string const& __s) const { return _hash_string(__s.c_str()); }
};

template <> struct Hash<std::string>
{
	apl_size_t operator()(const std::string& __s) const { return _hash_string(__s.c_str()); }
};

template <> struct Hash<apl_int16_t>
{
	apl_size_t operator()(apl_int16_t __s) const { return __s; }
};

template <> struct Hash<apl_int32_t>
{
	apl_size_t operator()(apl_int32_t __s) const { return __s; }
};

template <> struct Hash<apl_int64_t>
{
	apl_size_t operator()(apl_int64_t __s) const { return __s; }
};

template <> struct Hash<apl_uint16_t>
{
	apl_size_t operator()(apl_uint16_t __s) const { return __s; }
};

template <> struct Hash<apl_uint32_t>
{
	apl_size_t operator()(apl_uint32_t __s) const { return __s; }
};

template <> struct Hash<apl_uint64_t>
{
	apl_size_t operator()(apl_uint64_t __s) const { return __s; }
};

template <> struct Hash<double>
{
	double operator()(double __s) const { return __s; }
};

}

////////////////////////////////////////////////////////////////////////////////////
// iterator
template<typename ValueType, typename NodeType, typename HashArrayType>
class THashArrayIterator
{
public:
	typedef ValueType&   reference;
	typedef ValueType*   pointer;
	typedef THashArrayIterator<ValueType, NodeType, HashArrayType> iterator;

public:
    THashArrayIterator(void)
        : mpoNode(NULL)
        , mpoHashArray(NULL)
    {
    }
    
    THashArrayIterator(NodeType* apoNode, const HashArrayType* apoHashArray)
        : mpoNode(apoNode)
        , mpoHashArray(apoHashArray)
    {
    }
    
    reference operator * (void)
    {
        return this->mpoNode->moValue;
    }
    
	pointer operator -> (void)
	{
	    return &this->mpoNode->moValue;
	}
	
	const reference operator * (void) const
    {
        return this->mpoNode->moValue;
    }
    
	const pointer operator -> (void) const
	{
	    return &this->mpoNode->moValue;
	}
	
	iterator& operator ++ (void)
	{
	    NodeType const* lpoOld = this->mpoNode;
    	this->mpoNode = this->mpoNode->cpoNext;
    	if (this->mpoNode == NULL)
    	{
    		apl_size_t luN = this->mpoHashArray->moHash(lpoOld->moValue) % this->mpoHashArray->muBktNum;
    		while( this->mpoNode == NULL && ++luN < this->mpoHashArray->muBktNum )
    		{
    			this->mpoNode = this->mpoHashArray->mppoBucket[luN];
    		}
    	}
    	
    	return *this;    
	}
	
	iterator operator ++ (int)
	{
	    iterator loTmp = *this;
    	++(*this);
    	return loTmp;
	}
	
	bool operator == ( iterator it ) const
	{
	    return this->mpoNode == it.mpoNode;
	}
	                           
	bool operator != ( iterator it ) const
	{
	    return this->mpoNode != it.mpoNode;
	} 
	
private:
    NodeType* mpoNode;
    const HashArrayType* mpoHashArray;
};

template<typename KeyType, typename ValueType>
struct CHashArrayPair
{
	CHashArrayPair(void)
	    : first(KeyType() )
	    , second(ValueType() )
	{
	}
	
	CHashArrayPair( const KeyType& k, const ValueType& v )
	    : first(k)
	    , second(v)
	{
	}
	
	KeyType& operator () ( CHashArrayPair& p )
	{
		return p.first;
	}
	
	bool operator == ( const CHashArrayPair& p ) const
	{
		return first == p.first;
	}
	
	bool operator != ( const CHashArrayPair& p ) const
	{
		return first != p.first;
	}
	
	KeyType   first;
	ValueType second;
};

template<typename KeyType, typename ValueType>
struct CHashArrayNode
{
    CHashArrayNode(void)
        : mpoNext(NULL)
    {
    }
    
    CHashArrayPair<KeyType, ValueType>  moValue;
    CHashArrayNode* mpoNext;
};

template<typename KeyType, typename ValueType, typename HashFuncType = Hash<KeyType> >
class THashArray
{
public:
	typedef THashArray<KeyType, ValueType, HashFuncType> self;
	typedef KeyType key_type;
	typedef CHashArrayNode<KeyType, ValueType> node;
	typedef CHashArrayPair<KeyType, ValueType> value_type;
	typedef THashArrayIterator<value_type, node, self> iterator;
	typedef const THashArrayIterator<value_type, node, self> const_iterator;
	typedef apl_size_t size_type;
    
public:
    THashArray( apl_size_t auCapacity, apl_size_t auBktNum = 0 )
        : muCapacity(auCapacity)
        , muBktNum(auBktNum)
        , muElemNum(0)
    {
        if (this->muBktNum == 0)
        {
            this->muBktNum = this->muCapacity;
        }

        ACL_NEW_N_ASSERT(this->mppoBucket, node*, this->muBktNum);
        ACL_NEW_N_ASSERT(this->mpoElements, node, this->muCapacity);
        
        apl_memset(this->mppoBucket, 0, sizeof(node*) * this->muBktNum);
    }
    
    ~THashArray(void)
    {
        ACL_DELETE_N(this->mppoBucket);
        ACL_DELETE_N(this->mpoElements);
    }
    
    iterator insert( value_type const& aoValue )
    {
        return this->insert(aoValue.first, aoValue.second);
    }
    
    ValueType& operator [] ( KeyType const& aoKey )
	{
	    return this->insert(aoKey, ValueType() )->second;
	}
    
    iterator find( KeyType const& aoKey ) const
    {
        apl_size_t luKey = this->moHash(aoKey) % this->muBktNum;

        node* lpoFirst = this->mppoBucket[luKey];
        for ( ; lpoFirst != NULL; lpoFirst = lpoFirst->mpoNext )
        {
            if (aoKey == lpoFirst->moValue.first)
            {
                return iterator(lpoFirst, this);
            }
        }

        return iterator(NULL, this);
    }
    
    void clear(void)
    {
        if (this->muElemNum > 0)
        {
            apl_memset(this->mppoBucket, 0, sizeof(node*) * this->muBktNum );
        	this->muElemNum = 0;
        }
    }
    
    iterator begin(void) const
    {
        for (size_type luN = 0; luN < this->muBktNum; ++luN)
		{
			if (this->mppoBucket[luN] != NULL)
			{
				return iterator((node*)this->mppoBucket[luN], this);
			}
		}
		
        return iterator(NULL, this);
    }
    
    iterator end(void) const
    {
        return iterator(NULL, this);
    }

protected:
    iterator insert( KeyType const& aoKey, ValueType const& aoValue )
    {
        if (this->muElemNum >= this->muCapacity)
        {
            return iterator(NULL, this);
        }
        
        apl_size_t luKey = this->moHash(aoKey) % this->muBktNum;
        
        node*& lpoFirst = this->mppoBucket[luKey];
        if (lpoFirst == NULL)
        {
            lpoFirst = &this->mpoElements[this->muElemNum];
            lpoFirst->moValue.first = aoKey;
            lpoFirst->moValue.second = aoValue;
        }
        else
        {
            node* lpoNew = &this->mpoElements[this->muElemNum];
            lpoNew->moValue.first = aoKey;
            lpoNew->moValue.second = aoValue;
    
            lpoNew->mpoNext = lpoFirst;
            lpoFirst = lpoNew;
        }
    
    	this->muElemNum++;
    	
    	return iterator(lpoFirst, this);
    }
    
private:
    apl_size_t muCapacity;
    apl_size_t muBktNum;
    
    node**     mppoBucket;
    node*      mpoElements;
    apl_size_t muElemNum;
    HashFuncType moHash;   
};

ACL_NAMESPACE_END

#endif//ACL_HASHARRAY_H
