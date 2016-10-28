
#ifndef ACL_BITARRAY_H
#define ACL_BITARRAY_H

#include "acl/Utility.h"

ACL_NAMESPACE_START

class CBitArray
{
public:
    static const apl_size_t INVALID_POS = ~(apl_size_t)0;
    
    enum EOption
    {
        DO_COPY = 1, //do copy on construct
        DONT_COPY
    };

public:
    /**
     * @brief default constructor
     */
    CBitArray(void);
    
    /**
     * @brief constructor
     * @param [in] auSize array size
     */
    CBitArray( apl_size_t auSize );

    /** 
     * @brief construct by specified bit-string
     * 
     * @param apcBitStr bit-string
     * @param auLength bit-string length
     * @param aeOpt option
     */
    CBitArray( char const* apcBitStr, apl_size_t auLength, EOption aeOpt );
    
    /**
     * @brief destructor
     */
    ~CBitArray(void);
    
    /** 
     * @brief resize bit array size
     * 
     * @param auSize  array size
     */
    void Resize( apl_size_t auSize );

    /** 
     * @brief set all bit to enable or disable in array
     * 
     * @param abX bit value
     */
    void SetAll( bool abX );
    
    /** 
     * @brief set the bit to enable or disable by index in array
     * 
     * @param auIndex array index
     * @param abX bit value
     */
    void Set( apl_size_t auIndex, bool abX );

    /** 
     * @brief get the bit by index in array
     * 
     * @param auIndex array index
     * 
     * @returns the bit value
     */
    bool Get( apl_size_t auIndex );

    /** 
     * @brief find the bit by specify value @abX and from index @auPos in array
     * 
     * @param auPos start index
     * @param abX bit value
     * 
     * @returns bit position
     */
    apl_size_t Find( apl_size_t auPos, bool abX );

    /** 
     * @brief find the bit by specify value @abX and from the first in array
     * 
     * @param abX bit value
     * 
     * @returns bit position
     */
    apl_size_t Find( bool abX );

    /** 
     * @brief return bit array size
     */
    apl_size_t GetSize(void);

    /** 
     * @brief return bit-cstring
     */
    char const* GetCStr(void);

    /** 
     * @brief return bit-cstring length
     */
    apl_size_t GetCLength(void);

protected:
    void Init( apl_size_t auSize, char const* apcBitStr, apl_size_t auLength, EOption aeOpt );

private:
    unsigned char* mpcStart;

    bool mbIsDelete;

    apl_size_t muSize;
    
    apl_size_t muBitSize;

    apl_size_t muCapacity;
};

ACL_NAMESPACE_END

#endif//ACL_BITARRAY_H

