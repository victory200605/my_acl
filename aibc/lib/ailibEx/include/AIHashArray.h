
#ifndef __AILIB_HASHARRAY_H__
#define __AILIB_HASHARRAY_H__

///start namespace
AIBC_NAMESPACE_START

/// Hesh function
template <class TKey> struct __hash { };

inline size_t _hash_string(const char* apcString )
{
	assert( apcString != NULL );
	//unsigned long llH = 0; 
	unsigned int liRetValue = 1;
	unsigned int liTempValue = 4;
	size_t       liLength    = strlen(apcString);

	while( liLength-- ) /* from MySQL */
	{ 
		liRetValue ^= (((liRetValue & 63)+liTempValue)*((unsigned int)(unsigned char) *apcString++))+ (liRetValue << 8);
		liTempValue += 3; 
	} 
	
	return liRetValue;
}

template <> struct __hash<const char*>
{
	size_t operator()(const char* __s) const { assert(__s != NULL ); return _hash_string(__s); }
};
template <> struct __hash<int>
{
	size_t operator()(int __s) const { return __s; }
};
template <> struct __hash<unsigned int>
{
	size_t operator()(unsigned int __s) const { return __s; }
};
template <> struct __hash<short>
{
	size_t operator()(short __s) const { return __s; }
};
template <> struct __hash<double>
{
	double operator()(double __s) const { return __s; }
};

///Extract key
template <class T> struct __extract_key
{
    typedef T key_type;
    T& operator () ( T& v ) { return v; }
};

///Equeue
template <class T> struct __equeue
{    
    bool operator () ( const T& __l, const T& __r ) const { return __l == __r; }
};
template <> struct __equeue<const char*>
{    
    bool operator () ( const char* __l, const char* __r ) const { return strcmp(__l, __r) == 0 ? true : false; }
};

//////////////////////////////////////////////////////////////////////////////////////////
template< class T, 
          int ASize,
          class TExtractKey = __extract_key<T>,
          class TEqueue = __equeue< typename TExtractKey::key_type>,
          class THashFnc = __hash< typename TExtractKey::key_type> >
class AIHashArray
{
public:
    typedef T value_type;
    typedef typename TExtractKey::key_type key_type;
        
public:
    struct stHashNode
    {
	    stHashNode( value_type* t ):cptValue(t),cpoNext(0)  { }
	    value_type* cptValue;
	    stHashNode* cpoNext;
    };

public:
    AIHashArray() : 
        ciNumElem(0)
    {
        ::memset( this->cpoBucket, 0, sizeof(this->cpoBucket) );
    }
    ~AIHashArray()
	{
	    this->Clear();
	}
    
    void Insert( size_t aiN )
    {
        size_t liBkt = this->BktNum( this->coValue[aiN], ASize );
        stHashNode* lpoFirst = cpoBucket[liBkt];
        for ( stHashNode* lpoCur = lpoFirst; lpoCur; lpoCur = lpoCur->cpoNext )
		{
			if ( lpoCur->cptValue == &this->coValue[aiN] )
			{
				return;
			}
		}
        stHashNode* lpoNode = new stHashNode(&this->coValue[aiN]);
        lpoNode->cpoNext = lpoFirst;
        cpoBucket[liBkt] = lpoNode;
        ++ciNumElem;
    }
    
    void Erase( size_t aiN )
    {
        size_t liBkt = this->BktNum( this->coValue[aiN], ASize );
        stHashNode* lpoFirst = cpoBucket[liBkt];
        stHashNode* lpoPrev  = lpoFirst;
        for ( stHashNode* lpoCur = lpoFirst; lpoCur; lpoPrev = lpoCur, lpoCur = lpoCur->cpoNext )
		{
			if ( lpoCur->cptValue == &this->coValue[aiN] )
			{
			    if ( lpoPrev == lpoCur )
			    {
			        cpoBucket[liBkt] = lpoCur.cpoNext;
			        delete lpoCur;
			    }
			    else
			    {
			        lpoPrev->cpoNext = lpoCur.cpoNext;
			        delete lpoCur;
			    }
			    --ciNumElem;
			}
		}
    }
    
    void Clear()
    {
        for ( size_t liIt = 0; liIt < ASize; liIt++ )
		{
			stHashNode* lpoCur = cpoBucket[liIt];
			while( lpoCur != 0 )
			{
				stHashNode* lpoNext = lpoCur->cpoNext;
				delete lpoCur;
				lpoCur = lpoNext;
			}
			cpoBucket[liIt] = 0;
		}
		ciNumElem = 0;
    }
    
    value_type* Find( key_type atKey ) const
    {
        size_t liBkt = this->coHash( atKey ) % ASize;
        stHashNode* lpoFirst = this->cpoBucket[liBkt];
        for ( stHashNode* lpoCur = lpoFirst; lpoCur; lpoCur = lpoCur->cpoNext )
		{
			if ( coCompare( coGetKey(*lpoCur->cptValue), atKey ) )
			{
				return lpoCur->cptValue;
			}
		}
		return NULL;
    }
    
    value_type& operator [] ( size_t aiN )
    {
        return this->coValue[aiN];
    }
    
    size_t Size() const
    {
        return this->ciNumElem;
    }

protected:
    size_t BktNum( value_type atValue, size_t aiN )
	{
		return this->coHash( coGetKey(atValue) ) % aiN;
	}
	
protected:
    size_t      ciNumElem;
    value_type  coValue[ASize];
    stHashNode* cpoBucket[ASize];
    TExtractKey coGetKey;
    THashFnc    coHash;
    TEqueue     coCompare;
};

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIB_HASHARRAY_H__
