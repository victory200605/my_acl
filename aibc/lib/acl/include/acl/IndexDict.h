#ifndef ACL__INDEXDICT_H
#define ACL__INDEXDICT_H

#include "acl/Utility.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"
#include "acl/stl/deque.h"
#include "acl/MemoryBlock.h"

ACL_NAMESPACE_START

//////////////////////////////////////////////////////////////////////////////////////////
template<class T> class TSimpleArray
{
public:
    TSimpleArray( apl_size_t auSize = 0 )
        : moFirst(T() )
        , mpoStart(APL_NULL)
        , muSize(0)
        , muCapacity(1)
    {
        this->reserve(auSize);
    }

    ~TSimpleArray(void)
    {
        ACL_DELETE_N(this->mpoStart);
    }

    void push_back( T const& aoVal )
    {
        if (this->muSize >= this->muCapacity)
        {
            this->reserve(this->muSize * 2);
        }

        if (this->muSize == 0)
        {
            this->moFirst = aoVal;
        }
        else
        {
            this->mpoStart[this->muSize - 1] = aoVal;
        }
        
        this->muSize++;
    }

    T& fill_front(void)
    {
        this->muSize++;

        return this->moFirst;
    }

    void erase( apl_size_t auIndex )
    {
        if (auIndex == 0)
        {
            if (this->muSize <= 1)
            {
                this->muSize = 0;
            }
            else
            {
                this->moFirst = this->mpoStart[0];
                apl_memcpy(this->mpoStart, this->mpoStart + 1, sizeof(T*) * (this->muSize - 2) );
                this->muSize--;
            }
        }
        else if (auIndex < this->muSize)
        {
            apl_size_t luNewIndex = auIndex - 1;
            apl_size_t luNewSize = this->muSize - 1;

            apl_memcpy(this->mpoStart + luNewIndex, this->mpoStart + luNewIndex + 1, sizeof(T*) * (luNewSize - luNewIndex - 1) );

            this->muSize--;
        }
    }

    void reserve( apl_size_t auSize )
    {
        if (auSize > this->muCapacity)
        {
            apl_size_t luNewSize = auSize - 1;
            apl_size_t luOldSize = this->muSize - 1;
            T* lpoTemp = APL_NULL;
            ACL_NEW_N_ASSERT(lpoTemp, T, luNewSize);

            if (this->muSize > 1)
            {
                apl_memcpy(lpoTemp, this->mpoStart, sizeof(T) * luOldSize);
                apl_memset(lpoTemp + luOldSize, 0, sizeof(T) * (luNewSize - luOldSize) );
            }
            else
            {
                apl_memset(lpoTemp, 0, sizeof(T) * luNewSize);
            }

            if (this->mpoStart != APL_NULL)
            {
                ACL_DELETE_N(this->mpoStart);
            }

            this->mpoStart = lpoTemp;
            this->muCapacity = auSize;
        }
    }

    void resize( apl_size_t auSize )
    {
        if (auSize > this->muCapacity)
        {
            this->reserve(auSize * 2);
        }
        
        this->muSize = auSize;
    }

    void clear(void)
    {
        this->muSize = 0;
    }

    const T& operator [] ( apl_size_t auN ) const
    {
        if (auN == 0)
        {
            return this->moFirst;
        }
        else
        {
            return this->mpoStart[auN - 1];
        }
    }
    
    T& operator [] ( apl_size_t auN )
    {
        if (auN == 0)
        {
            return this->moFirst;
        }
        else
        {
            return this->mpoStart[auN - 1];
        }
    }

    T& back(void)
    {
        if (this->muSize <= 1)
        {
            return this->moFirst;
        }
        else
        {
            return this->mpoStart[this->muSize - 2];
        }
    }

    apl_size_t size(void) const
    {
        return this->muSize;
    }

    apl_size_t capacity(void) const
    {
        return this->muCapacity;
    }

    bool empty(void) const
    {
        return this->muSize == 0;
    }

    void swap( TSimpleArray& aoRhs )
    {
        std::swap(this->moFirst, aoRhs.moFirst);
        std::swap(this->mpoStart, aoRhs.mpoStart);
        std::swap(this->muSize, aoRhs.muSize);
        std::swap(this->muCapacity, aoRhs.muCapacity);
    }

private:
    TSimpleArray( TSimpleArray const& );
    TSimpleArray& operator = ( TSimpleArray const& );

private:
    T  moFirst;
    T* mpoStart;
    apl_size_t muSize;
    apl_size_t muCapacity;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> class TObjectAllocatee
{
public:
    TObjectAllocatee( apl_size_t auBlockSize )
        : muBlockSize(auBlockSize)
        , moBlocks(64)
        , miAvailableBlock(-1)
        , muSize(auBlockSize)
    {
    }

    ~TObjectAllocatee(void)
    {
        this->Release();
    }

    T* Allocate(void)
    {
        if (this->muSize >= this->muBlockSize)
        {
            this->AllocateBlock();
        }

        return &this->moBlocks[this->miAvailableBlock][this->muSize++];
    }

    void Reset(void)
    {
        this->Release();
    }

    apl_size_t GetBlockSize(void) const
    {
        return this->muBlockSize;
    }

    void Swap( TObjectAllocatee& aoRhs )
    {
        std::swap(this->muBlockSize, aoRhs.muBlockSize);
        this->moBlocks.swap(aoRhs.moBlocks);
        std::swap(this->miAvailableBlock, aoRhs.miAvailableBlock);
        std::swap(this->muSize, aoRhs.muSize);
    }

protected:
    void AllocateBlock(void)
    {
        T* lpoTemp = APL_NULL;
        ACL_NEW_N_ASSERT(lpoTemp, T, this->muBlockSize);

        this->moBlocks.push_back(lpoTemp);
        this->miAvailableBlock++;
        this->muSize = 0;
    }

    void Release(void)
    {
        apl_size_t luSize = this->moBlocks.size();
        for (apl_size_t luN = 0; luN < luSize; luN++)
        {
            ACL_DELETE_N(this->moBlocks[luN]);
        }

        this->moBlocks.clear();
        this->muSize = this->muBlockSize;
        this->miAvailableBlock = -1;
    }

private:
    apl_size_t muBlockSize;
    TSimpleArray<T*> moBlocks;
    apl_int_t miAvailableBlock;
    apl_size_t muSize;
};

//////////////////////////////////////////////////////////////////////////////////////////////
class CIndexDictAllocatee
{
public:
    CIndexDictAllocatee( apl_size_t auSmallSize, apl_size_t auBlockSize );

    ~CIndexDictAllocatee(void);

    void* Allocate( apl_size_t auSize );

    void Reset(void);
    
    void Swap( CIndexDictAllocatee& aoRhs );

    apl_size_t GetSmallSize(void) const
    {
        return this->muSmallSize;
    }

    apl_size_t GetBlockSize(void) const
    {
        return this->muBlockSize;
    }

protected:
    void Init(void);

    void Release(void);

private:
    apl_size_t muSmallSize;
    apl_size_t muBlockSize;
    TSimpleArray<char*> moBlocks;
    char* mpoCurrAllocPtr;
    apl_size_t muSize;
};

///////////////////////////////////////////////////////////////////////////////////////////////
class CIndexDictValue
{
public:
    CIndexDictValue(void);
    
    CIndexDictValue( char const* apcInput, CIndexDictAllocatee& aoAllocatee );

    CIndexDictValue( void const* apvInput, apl_size_t auLen, CIndexDictAllocatee& aoAllocatee );
    
    CIndexDictValue( void const* apvInput, apl_size_t auLen );

    ~CIndexDictValue(void);

    void Assign( char const* apcInput, CIndexDictAllocatee& aoAllocatee );

    void Assign( void const* apvInput, apl_size_t auLen, CIndexDictAllocatee& aoAllocatee );

    bool operator == ( CIndexDictValue const& aoRhs ) const;

    bool operator != ( CIndexDictValue const& aoRhs ) const;

    bool operator == ( char const* apcRhs ) const;
    
    bool operator != ( char const* apcRhs ) const;

    bool operator == ( std::string const& aoRhs ) const;

    bool operator != ( std::string const& aoRhs ) const;

    char const* GetStr(void) const;

    apl_size_t GetLength(void) const;

    bool IsEmpty(void) const;

    void Clear(void);

    //string operator
    char const* c_str(void) const;

    apl_size_t size(void) const;

    apl_size_t length(void) const;

    bool empty(void) const;

    operator std::string (void)
    {
        return std::string(this->mpcStart, this->muLength);
    }

private:
    char* mpcStart;
    apl_size_t muLength;
};

////////////////////////////////////////////////////////////////////////////////////////////////
class CIndexDict
{
public:
    typedef CIndexDictValue ValueType;
    typedef TSimpleArray<CIndexDictValue> ItemType;
    typedef CIndexDictAllocatee AllocateeType;
    typedef TObjectAllocatee<ItemType> ItemAllocateeType;

public:
    CIndexDict( 
        apl_size_t auSize = 16, 
        apl_size_t auItemBlockSize = 0, //default = auSize
        apl_size_t auSmallValueSize = 64, 
        apl_size_t auValueBlockSize = 1024 );

    CIndexDict( CIndexDict const& aoRhs );

    CIndexDict& operator = ( CIndexDict const& aoRhs );

    ~CIndexDict(void);

    bool operator == ( CIndexDict const& aoRhs ) const;

    bool operator != ( CIndexDict const& aoRhs ) const;

    /**
     * Read and parse bencoding data from a memory block.
     * All old values would be deleted when load success.
     * This method would modify the read pointer of the memory block,
     *
     * @param[in,out] apoMB Pointer to a CMemoryBlock
     * @retval 0 Success, would set the read pointer of the memory block to skip read data.
     * @retval -1 Failure, read pointer of the memory block would be kept unmodified. 
     */
    apl_int_t Decode( CMemoryBlock* apoBuffer );


    /**
     * Format and write bencoding data to a memory block.
     * This method would modify the write pointer of the memory block,
     *
     * @param[out] apoMB Pointer to a CMemoryBlock
     * @retval 0 Success.
     * @retval -1 Failure.
     */
    apl_int_t Encode( CMemoryBlock* apoBuffer );


    /**
     * Get a value as c-string by tag. 
     *   
     * @param[in] auTag The tag number
     * @param[in] apcDefault The default value to be return 
     * @retval The first value if the tag is found or default if the tag is not found
     */
    char const*         Get(apl_size_t auTag, char const* apcDefault=APL_NULL) const;


    /**
     * Get a value as string by tag. 
     *   
     * @param[in] auTag The tag number
     * @param[in] apcDefault The default value to be return 
     * @retval The first value if the tag is found or default if the tag is not found
     */
    ValueType           GetStr(apl_size_t auTag, std::string const& aoDefault=NULL_VAL ) const;


    /**
     * Get a value as integer by tag. 
     *   
     * @param[in] auTag The tag number
     * @param[in] apcDefault The default value to be return 
     * @retval The first value if the tag is found or default if the tag is not found
     */
    apl_intmax_t        GetInt(apl_size_t auTag, apl_intmax_t aimDefault=APL_INTMAX_C(0)) const;


    /**
     * Get a value as c-string by tag and index. 
     *   
     * @param[in] auTag The tag number
     * @param[in] auIndex The value index 
     * @param[in] apcDefault The default value to be return 
     * @retval The first value if the tag is found or default if the tag is not found
     */
    char const*         Get2(apl_size_t auTag, apl_size_t auIindex, char const* apcDefault=APL_NULL) const;


    /**
     * Get a value as string by tag and index. 
     *   
     * @param[in] auTag The tag number
     * @param[in] auIndex The value index 
     * @param[in] apcDefault The default value to be return 
     * @retval The first value if the tag is found or default if the tag is not found
     */
    ValueType           GetStr2(apl_size_t auTag, apl_size_t auIindex, std::string const& aoDefault=NULL_VAL ) const;


    /**
     * Get a value as integer by tag and index. 
     *   
     * @param[in] auTag The tag number
     * @param[in] auIndex The value index 
     * @param[in] apcDefault The default value to be return 
     * @retval The first value if the tag is found or default if the tag is not found
     */
    apl_intmax_t        GetInt2(apl_size_t auTag, apl_size_t auIindex, apl_intmax_t aimDefault=APL_INTMAX_C(0)) const;


    /**
     * Set a c-string to a tag. 
     *   
     * @param[in] auTag The tag number
     * @param[in] apcValue The value to be added
     * @retval 0 Success
     * @retval -1 Failure
     */
    apl_int_t Set(apl_size_t auTag, char const* apcValue);

    /**
     * Set a buffer to a tag.
     *   
     * @param[in] auTag The tag number
     * @param[in] apvValue The buffer to be added
     * @param[in] auLen The length of buffer
     * @retval 0 Success
     * @retval -1 Failure
     */
    apl_int_t Set(apl_size_t auTag, void const* apvValue, apl_size_t auLen);

    /**
     * Set a string to a tag.
     *   
     * @param[in] auTag The tag number
     * @param[in] aoValue The string to be added
     * @retval 0 Success
     * @retval -1 Failure
     */ 
    apl_int_t Set(apl_size_t auTag, std::string const& aoValue);


    /**
     * Set a integer to a tag.
     *   
     * @param[in] auTag The tag number
     * @param[in] aimValue The integer to be added
     * @retval 0 Success
     * @retval -1 Failure
     */ 
    apl_int_t Set(apl_size_t auTag, apl_intmax_t aimValue);


    /**
     * Set a c-string to a tag by index.
     *   
     * @param[in] auTag The tag number
     * @param[in] auIndex The index of values to set
     * @param[in] apcValue The value to be added
     * @retval 0 Success
     * @retval -1 Failure
     */
    apl_int_t Set2(apl_size_t auTag, apl_size_t auIndex, char const* apcValue);


    /**
     * Set a buffer to a tag by index.
     *   
     * @param[in] auTag The tag number
     * @param[in] apvValue The buffer to be added
     * @param[in] auLen The length of buffer
     * @retval 0 Success
     * @retval -1 Failure
     */
    apl_int_t Set2(apl_size_t auTag, apl_size_t auIndex, void const* apvValue, apl_size_t auLen);


    /**
     * Set a string to a tag by index.
     *   
     * @param[in] auTag The tag number
     * @param[in] aoValue The string to be added
     * @retval 0 Success
     * @retval -1 Failure
     */ 
    apl_int_t Set2(apl_size_t auTag, apl_size_t auIndex, std::string const& aoValue);


    /**
     * Set a integer to a tag by index.
     *   
     * @param[in] auTag The tag number
     * @param[in] aimValue The integer to be added
     * @retval 0 Success
     * @retval -1 Failure
     */ 
    apl_int_t Set2(apl_size_t auTag, apl_size_t auIndex, apl_intmax_t aimValue);


    /**
     * Add a c-string to a tag.
     *   
     * @param[in] auTag the tag number
     * @param[in] apcValue the string to be added
     * @retval 0 Success
     * @retval -1 Failure
     */
    apl_int_t Add(apl_size_t auTag, char const* apcValue);


    /**
     * Add a buffer to a tag.
     *   
     * @param[in] auTag The tag number
     * @param[in] apvValue The buffer to be added
     * @param[in] auLen The length of buffer 
     * @retval 0 Success
     * @retval -1 Failure
     */
    apl_int_t Add(apl_size_t auTag, void const* apvValue, apl_size_t auLen);

    /**
     * Add a string to a tag.
     *   
     * @param[in] auTag The tag number
     * @param[in] aoValue The string to be added
     * @retval 0 Success
     * @retval -1 Failure
     */
    apl_int_t Add(apl_size_t auTag, std::string const& aoValue);


    /**
     * Add a integer to a tag.
     *   
     * @param[in] auTag The tag number
     * @param[in] aimValue The integer to be added
     * @retval 0 Success
     * @retval -1 Failure
     */
    apl_int_t Add(apl_size_t auTag, apl_intmax_t aimValue);


    /**
     * Delete a tag and its associated values.
     * 
     * @param[in] auTag the tag to delete
     * @retval 0 Success
     * @retval -1 Failure
     */
    apl_int_t Del(apl_size_t auTag);


    /**
     * Delete one associated value to a tag
     * 
     * @param[in] auTag the tag
     * @param[in] auIndex the index of values to be deleted
     * @retval 0 Success
     * @retval -1 Failure
     */
    apl_int_t Del(apl_size_t auTag, apl_size_t auIindex);


    /**
     * Clear all the values and tag in the dict.
     */
    void Clear();


    /**
     * Swap the value of two CIndexDict objects.
     *
     * @param[in,out] aoRhs the CIndexDict object to swap with
     */
    void Swap(CIndexDict& aoRhs);


    /**
     * Count how many values are associated with the tag.
     *
     * @param[in] auTag The tag to count
     * @return The number of values, if tag is not in the dict, 0 would be return
     */
    apl_size_t Count(apl_size_t auTag) const;


    /**
     * Check if the tag is in the dict.
     *
     * @param[in] auTag The tag to check
     * @return Check result
     */
    bool Has(apl_size_t auTag) const;

    apl_size_t TotalTags(void) const;

    apl_size_t MinTag(void) const;

    apl_size_t NextTag(apl_size_t auIndex) const;

    void Dump(apl_size_t auLevel = 1) const;

protected:
    void CopyFrom( CIndexDict const& aoRhs );

    ValueType& Insert( apl_size_t auTag, apl_size_t auIndex );
    
    ValueType& PushBack( apl_size_t auTag );

    ValueType* GetValue( apl_size_t auTag, apl_size_t auIndex ) const;

private:
    static const std::string NULL_VAL;

    apl_size_t muBlockSize;

    TSimpleArray<ItemType*> moDict;
    
    ItemAllocateeType       moItemAllocatee; 

    AllocateeType           moAllocatee;
};

ACL_NAMESPACE_END

#endif // __ACL__INDEXDICT_H__
