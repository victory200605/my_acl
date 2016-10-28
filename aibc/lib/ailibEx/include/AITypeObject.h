#ifndef __AILIBEX__AITYPEOBJECT_H__
#define __AILIBEX__AITYPEOBJECT_H__

#include "AISynch.h"

///start namespace
AIBC_NAMESPACE_START

// Types define
template< class T, class TLock = AINullMutexLock >
class AITypeObject
{
public:
	struct clsNil { public: clsNil(){} };

	static const clsNil nil;

public:
	AITypeObject() : 
	    ctVal(0), cbIsValid(false)
	{
	}
	
	AITypeObject( T n ) : 
	    ctVal(n), cbIsValid(true)
	{
	}
	
	AITypeObject( const AITypeObject& aoRhs ) : 
	    ctVal(0), cbIsValid(false)
	{
	    if ( aoRhs != nil ) *this = aoRhs;
	}

	// Operation
	AITypeObject& operator = ( const T aiN )
	{
	    this->coLock.Lock();
        	cbIsValid = true;
        	ctVal     = aiN;
    	this->coLock.Unlock();
		return *this;
	}
	AITypeObject& operator = ( const AITypeObject& aiN )
	{
		*this = (T)aiN;
		return *this;
	}
    
    bool operator == ( const clsNil& aiN )
	{
	    this->coLock.Lock();
	        bool liFlag = (cbIsValid == false);
	    this->coLock.Unlock();
		return liFlag;
	}
	
    bool operator == ( const clsNil& aiN ) const                      
	{
		this->coLock.Lock();
	        bool liFlag = (cbIsValid == false);
	    this->coLock.Unlock();
		return liFlag;
	}
	
	bool operator != ( const clsNil& aiN )
	{
		return !(*this == aiN);
	}
	
    bool operator != ( const clsNil& aiN ) const                      
	{
		return !(*this == aiN);
	}

	T operator ++ ()
	{
	    this->coLock.Lock();
	        T ltTmp = ++ctVal;
	    this->coLock.Unlock();
	    return ltTmp;
	}
	
	T operator -- ()
	{
	    this->coLock.Lock();
	        T ltTmp = --ctVal;
	    this->coLock.Unlock();
	    return ltTmp;
	}
	
    T operator ++ ( int )
	{
	    this->coLock.Lock();
	        T ltTmp = ctVal++;
	    this->coLock.Unlock();
	    return ltTmp;
	}
	
	T operator -- ( int )
	{
	    this->coLock.Lock();
	        T ltTmp = ctVal--;
	    this->coLock.Unlock();
	    return ltTmp;
	}
		
	operator T ()
	{
	    this->coLock.Lock();
	        T ltTmp = ctVal;
	    this->coLock.Unlock();
		return ltTmp;
	}
	
	operator T () const
	{
	    this->coLock.Lock();
	        T ltTmp = ctVal;
	    this->coLock.Unlock();
		return ltTmp;
	}

protected:
	T     ctVal;
	bool  cbIsValid;
	TLock coLock;
};

template< class T, class TLock > const typename AITypeObject<T, TLock>::clsNil AITypeObject<T, TLock>::nil;

typedef AITypeObject<int>      AIInt;
typedef AITypeObject<short>    AIShort;
typedef AITypeObject<long>     AILong;
typedef AITypeObject<double>   AIDouble;

///end namespace
AIBC_NAMESPACE_END

#endif //__AILIBEX__AITYPEOBJECT_H__
