
#ifndef __AILIB_OPERATORS_H__
#define __AILIB_OPERATORS_H__

////////////////////////////////////////////////////////////////////////////////////
/// Operator overwrite by == and <, Basic operator classes
template< class T, class U >
class AIOperators
{
public:
    friend inline bool operator == ( const U& lhs, const T& rhs ) 
    { 
        return rhs == lhs;
    }
    
    friend inline bool operator <  ( const U& lhs, const T& rhs ) 
    { 
        return !(rhs < lhs || rhs == lhs );
    }
    
    friend inline bool operator != ( const T& lhs, const U& rhs ) 
    { 
        return !(lhs == rhs); 
    }
    
    friend inline bool operator != ( const U& lhs, const T& rhs ) 
    { 
        return rhs != lhs;
    }
    
    friend inline bool operator >  ( const T& lhs, const U& rhs ) 
    { 
        return rhs < lhs; 
    }
    
    friend inline bool operator >  ( const U& lhs, const T& rhs ) 
    { 
        return rhs < lhs;
    }
    
    friend inline bool operator <= ( const T& lhs, const U& rhs ) 
    { 
        return !(rhs < lhs);
    }
    
    friend inline bool operator <= ( const U& lhs, const T& rhs ) 
    { 
        return !(rhs < lhs);
    }
    
    friend inline bool operator >= ( const T& lhs, const U& rhs ) 
    { 
        return !(lhs < rhs);
    }
    
    friend inline bool operator >= ( const U& lhs, const T& rhs ) 
    { 
        return !(lhs < rhs);
    }
};
// Trait for same type
template< class T >
class AIOperators<T,T>
{
public:
    friend inline bool operator != ( const T& lhs, const T& rhs ) 
    {
        return !(lhs == rhs);
    }
    
    friend inline bool operator >  ( const T& lhs, const T& rhs )
    {
        return !( lhs == rhs || lhs < rhs );
    }
    
    friend inline bool operator <= ( const T& lhs, const T& rhs )
    {
        return !(rhs < lhs);
    }
    
    friend inline bool operator >= ( const T& lhs, const T& rhs )
    {
        return !(lhs < rhs);
    }
};

#endif //__AILIB_OPERATORS_H__
