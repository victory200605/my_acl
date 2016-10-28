
#ifndef __AILIB_SMARTPTR_H__
#define __AILIB_SMARTPTR_H__

#include "AISynch.h"
#include "AITypeObject.h"
#include "AIOperators.h"

////////////////////////////////////////////////////////////////////////////////////
/// Select type
template < bool Flag, typename T, typename U > struct Select
{
    typedef T Result;
};

template <typename T, typename U> struct Select<false, T, U>
{
    typedef U Result;
};

////////////////////////////////////////////// DefaultStorage //////////////////////////////////////////
//// DefaultStorage : For storage policy
template< class T >
class AIDefaultStorage
{
public:
    typedef T* StoredType;
    typedef T* PointerType;
    typedef T& ReferenceType;
    
public:
    AIDefaultStorage() : cpPointee(0)
    {}
    
    AIDefaultStorage( const AIDefaultStorage& )
    {}

    template <class U>
    AIDefaultStorage( const AIDefaultStorage<U>& ) 
    {}
        
    AIDefaultStorage( const StoredType& apPointee ) : cpPointee(apPointee)
    {}
    
    PointerType operator -> ()
    {
        return this->cpPointee;
    }
    
    ReferenceType operator * ()
    {
        return *this->cpPointee;
    }
    
    void Swap( AIDefaultStorage& aoRhs )
    {
        AISwapValue( this->cpPointee, aoRhs.cpPointee );
    }
    
    // Accessors
    friend inline PointerType GetImpl( const AIDefaultStorage& aoSp )
    {
        return aoSp.cpPointee;
    }
    
    friend inline const StoredType& GetImplRef( const AIDefaultStorage& aoSp )
    {
        return aoSp.cpPointee;
    }

    friend inline StoredType& GetImplRef( AIDefaultStorage& aoSp )
    {
        return aoSp.cpPointee;
    }

protected:
    void Destroy()
    {
        AI_DELETE( this->cpPointee );
    }
    
    static StoredType Default()
    {
        return 0;
    }
        
protected:
    StoredType cpPointee;
};

//// AIArrayStorage : For array storage policy
template< class T >
class AIArrayStorage
{
public:
	typedef T* StoredType;
	typedef T* PointerType;
	typedef T& ReferenceType;

public:
	AIArrayStorage() : cpPointee(0)
	{}

	AIArrayStorage( const AIArrayStorage& )
	{}

	template <class U>
	AIArrayStorage( const AIArrayStorage<U>& ) 
	{}

	AIArrayStorage( const StoredType& apPointee ) : cpPointee(apPointee)
	{}

	PointerType operator -> ()
	{
		return this->cpPointee;
	}

	ReferenceType operator * ()
	{
		return *this->cpPointee;
	}

	void Swap( AIArrayStorage& aoRhs )
	{
		AISwapValue( this->cpPointee, aoRhs.cpPointee );
	}

	// Accessors
	friend inline PointerType GetImpl( const AIArrayStorage& aoSp )
	{
		return aoSp.cpPointee;
	}

	friend inline const StoredType& GetImplRef( const AIArrayStorage& aoSp )
	{
		return aoSp.cpPointee;
	}

	friend inline StoredType& GetImplRef( AIArrayStorage& aoSp )
	{
		return aoSp.cpPointee;
	}

protected:
	void Destroy()
	{
		AI_DELETE_N( this->cpPointee );
	}

	static StoredType Default()
	{
		return 0;
	}

protected:
	StoredType cpPointee;
};

////////////////////////////////////////////// RefCounted //////////////////////////////////////////
//// RefCounted : For Ownership policy
template < class PointerType >
class AIRefCounted
{
public:
    typedef int CounterType;
    
public:
    AIRefCounted() 
    {
        AI_NEW_ASSERT( this->cpiCount, CounterType );
        *this->cpiCount = 1;
    }
    
    AIRefCounted( const AIRefCounted& aoRhs ) 
		: cpiCount(aoRhs.cpiCount)
    {}

    template < class P >
    AIRefCounted( const AIRefCounted<P>& aoRhs ) 
		: cpiCount(reinterpret_cast<const AIRefCounted&>(aoRhs).cpiCount)
    {}
    
    PointerType Clone( const PointerType& apVal )
    {
        ++*this->cpiCount;
        return apVal;
    }
    
    bool Release( const PointerType& )
    {
        if (!--*this->cpiCount)
        {
            AI_DELETE( this->cpiCount );
            return true;
        }
        return false;
    }
    
    void Swap( AIRefCounted& aoRhs )
    {
        AISwapValue( this->cpiCount, aoRhs.cpiCount );
    }
    
    enum { DestructiveCopy = false };

private:
    // Data
    CounterType* cpiCount;
};
//// RefCountedMT : For Ownership policy and multi-thread safe
template < class PointerType >
class AIRefCountedMT
{
public:
    typedef AITypeObject<int, AIMutexLock> CounterType;
    
public:
    AIRefCountedMT() 
    {
        AI_NEW_ASSERT( this->cpiCount, CounterType );
        *this->cpiCount = 1;
    }
    
    AIRefCountedMT( const AIRefCountedMT& aoRhs ) 
        : cpiCount(aoRhs.cpiCount)
    {}
    
    template < class P >
    AIRefCountedMT( const AIRefCountedMT<P>& aoRhs ) 
		: cpiCount(reinterpret_cast<const AIRefCountedMT&>(aoRhs).cpiCount)
    {}
    
    PointerType Clone(const PointerType& apVal )
    {
        ++*this->cpiCount;
        return apVal;
    }
    
    bool Release( const PointerType& )
    {
        if (!--*this->cpiCount)
        {
            AI_DELETE( this->cpiCount );
            return true;
        }
        return false;
    }
    
    void Swap( AIRefCountedMT& aoRhs )
    {
        AISwapValue( this->cpiCount, aoRhs.cpiCount );
    }
    
    enum { DestructiveCopy = false };

private:
    // Data
    CounterType* cpiCount;
};

////////////////////////////////////////////////////////////////////////////////
// class template DestructiveCopy
// Implementation of the OwnershipPolicy used by SmartPtr
// Implements destructive copy semantics (a la std::auto_ptr)
template < class PointerType >
class AIDestructiveCopy
{
public:
    AIDestructiveCopy()
    {}
    
    template < class P1 >
    AIDestructiveCopy( const AIDestructiveCopy<P1>& )
    {}
    
    template < class P1 >
    static PointerType Clone( P1& apoVal )
    {
        PointerType lpoResult(apoVal);
        apoVal = P1();
        return lpoResult;
    }
    
    static bool Release( const PointerType& )
    {
        return true;
    }
    
    static void Swap( AIDestructiveCopy& )
    {}
    
    enum { DestructiveCopy = true };
};

////////////////////////////////////////////////////////////////////////////////
// class template NoCopy
// Implementation of the OwnershipPolicy used by SmartPtr
// Implements a policy that doesn't allow copying objects
template < class PointerType >
class AINoCopy
{
public:
    AINoCopy()
    {}
    
    template < class P1 >
    AINoCopy( const AINoCopy<P1>& )
    {}
    
    static PointerType Clone( const PointerType& )
    {
        // Make it depended on template parameter
        static const bool Disallows_Value_Copying = sizeof(PointerType*) == 0;

        assert( Disallows_Value_Copying );
    }
    
    static bool Release( const PointerType& )
    {
        return true;
    }
    
    static void Swap( AINoCopy& )
    {}
    
    enum { DestructiveCopy = false };
};
    
////////////////////////////////////////////// *SmartPtr* //////////////////////////////////////////
template
< 
    class T,
    template <class> class OwnershipPolicy = AIRefCounted,
    template <class> class StoragePolicy   = AIDefaultStorage 
>
class AISmartPtr
    : public StoragePolicy<T>
    , public OwnershipPolicy< typename StoragePolicy<T>::PointerType >
    , public AIOperators< AISmartPtr<T, OwnershipPolicy, StoragePolicy>, typename StoragePolicy<T>::PointerType >
{
    typedef StoragePolicy<T> SP;
    typedef OwnershipPolicy<typename StoragePolicy<T>::PointerType> OP;

public:
    typedef typename SP::PointerType   PointerType;
    typedef typename SP::StoredType    StoredType;
    typedef typename SP::ReferenceType ReferenceType;
    
    typedef typename Select< OP::DestructiveCopy, AISmartPtr, const AISmartPtr >::Result CopySelfType;
        
public:
    AISmartPtr()
    {
    }
    
    AISmartPtr( PointerType apoPtr ) : SP(apoPtr)
    {
    }

    AISmartPtr( CopySelfType& aoRhs ) 
        : SP(aoRhs), OP(aoRhs)
    {
        GetImplRef(*this) = OP::Clone(GetImplRef(aoRhs));
    }
    
    template
    <
        class T1,
        template <class> class OP1,
        template <class> class SP1
    >
    AISmartPtr( AISmartPtr<T1, OP1, SP1>& aoRhs ) 
        : SP(aoRhs), OP(aoRhs)
    {
        GetImplRef(*this) = OP::Clone(GetImplRef(aoRhs));
    }
    
    template
    <
        class T1,
        template <class> class OP1,
        template <class> class SP1
    >
    AISmartPtr( const AISmartPtr<T1, OP1, SP1>& aoRhs ) 
        : SP(aoRhs), OP(aoRhs)
    {
        GetImplRef(*this) = OP::Clone(GetImplRef(aoRhs));
    }
    
    ~AISmartPtr( )
    {
        if ( OP::Release( GetImpl(*this) ) )
        {
            SP::Destroy();
        }
    }
    
    AISmartPtr& operator = ( PointerType apoPtr )
    {
        AISmartPtr loTmp(apoPtr);
        loTmp.Swap(*this);
        return *this;
    }
    
    AISmartPtr& operator = ( CopySelfType& aoRhs ) 
    {
        AISmartPtr loTmp(aoRhs);
        loTmp.Swap(*this);
        return *this;
    }
    
    template
    <
        class T1,
        template <class> class OP1,
        template <class> class SP1
    >
    AISmartPtr& operator = ( AISmartPtr<T1, OP1, SP1>& aoRhs ) 
    {
        AISmartPtr loTmp(aoRhs);
        loTmp.Swap(*this);
        return *this;
    }
    
    template
    <
        class T1,
        template <class> class OP1,
        template <class> class SP1
    >
    AISmartPtr& operator = ( const AISmartPtr<T1, OP1, SP1>& aoRhs ) 
    {
        AISmartPtr loTmp(aoRhs);
        loTmp.Swap(*this);
        return *this;
    }
    
    void Swap( AISmartPtr& aoRhs)
    {
        OP::Swap(aoRhs);
        SP::Swap(aoRhs);
    }
    
    friend inline void Release( AISmartPtr& aoSp, StoredType& apPointee )
    {
        apPointee = SP::GetImplRef(aoSp);
        SP::GetImplRef(aoSp) = SP::Default();
    }
    
    friend inline void Reset( AISmartPtr& aoSp, StoredType apPointee )
    {
        AISmartPtr(apPointee).Swap(aoSp); 
    }

    PointerType operator -> ()
    {
        return SP::operator->();
    }

    PointerType operator -> () const
    {
        return SP::operator->();
    }

    ReferenceType operator * ()
    {
        return SP::operator*();
    }
    
    ReferenceType operator * () const
    {
        return SP::operator*();
    }
    
    ReferenceType operator [] ( int aiN )
	{
		return SP::operator->()[aiN];
	}

	ReferenceType operator [] ( int aiN ) const
	{
		return SP::operator->()[aiN];
	}
        
    bool operator ! () const // Enables "if (!sp) ..."
    {
        return GetImpl(*this) == 0;
    }

    // Ambiguity buster
    template
    < 
        class T1,
        template <class> class OP1,
        template <class> class SP1
    >
    bool operator == ( const AISmartPtr<T1, OP1, SP1>& aoRhs ) const
    {
        return GetImpl(*this) == GetImpl(aoRhs);
    }
    
    // Ambiguity buster
    template
    < 
        class T1,
        template <class> class OP1,
        template <class> class SP1
    >
    bool operator != ( const AISmartPtr<T1, OP1, SP1>& aoRhs ) const
    {
        return !(*this == aoRhs);
    }

    // Ambiguity buster
    template
    < 
        class T1,
        template <class> class OP1,
        template <class> class SP1
    >
    bool operator < ( const AISmartPtr<T1, OP1, SP1>& aoRhs ) const
    {
        return GetImpl(*this) < GetImpl(aoRhs);
    }
    
    bool operator == ( const PointerType apPointee ) const
    {
        return GetImpl(*this) == apPointee;
    }
    
    bool operator < ( const PointerType apPointee ) const
    {
        return GetImpl(*this) < apPointee;
    }
};

///Class template SmartPtrDef
template< class T >
struct AISmartPtrBox
{
    ///scope_ptr use for stack ptr
    typedef AISmartPtr< T, AINoCopy, AIDefaultStorage >          scope_ptr;
    ///share_ptr use by reference count but non-multi-thread-safe
    typedef AISmartPtr< T, AIRefCounted, AIDefaultStorage >      share_ptr;
    ///share_ptr use by reference count and multi-thread-safe
    typedef AISmartPtr< T, AIRefCountedMT, AIDefaultStorage >    share_ptr_r;
    ///auto_ptr
    typedef AISmartPtr< T, AIDestructiveCopy, AIDefaultStorage > auto_ptr;
    ///scope_array
    typedef AISmartPtr< T, AINoCopy, AIArrayStorage >            scope_array;
    ///share_array use by reference count but non-multi-thread-safe
    typedef AISmartPtr< T, AIRefCounted, AIArrayStorage >        share_array;
    ///share_array use by reference count and multi-thread-safe
    typedef AISmartPtr< T, AIRefCountedMT, AIArrayStorage >      share_array_r;
    ///auto_array
    typedef AISmartPtr< T, AIDestructiveCopy, AIArrayStorage >   auto_array;
};

#endif //__AILIB_SMARTPTR_H__
