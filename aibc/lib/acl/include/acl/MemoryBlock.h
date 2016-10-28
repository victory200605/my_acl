/**
 * @file MemoryBlock.h
 */

#ifndef ACL_MEMORYBLOCK_H
#define ACL_MEMORYBLOCK_H

#include "acl/Utility.h"

ACL_NAMESPACE_START

////////////////////////////////////////////// DataBlock /////////////////////////////////////
/**
 * class CDataBlock
 */
class CDataBlock
{
    friend class CMemoryBlock;
    
public:
    /**
     * @brief A constructor.Allocate data block.
     *
     * @param [in] auSize - the size of data block.
     */
    CDataBlock( apl_size_t auSize );
    
    /**
     * @brief A constructor.Allocate data block.
     *
     * @param [in] apcData - the specified data block
     * @param [in] auSize - the size of data block
     * @param [in] aiFlag - the enumeration
     */
    CDataBlock( char const* apcData, apl_size_t auSize, apl_int_t aiFlag );
    
    /**
     * @brief A destructor.
     */
    virtual ~CDataBlock(void);
    
    /**
     * @brief Set outside data block.
     *
     * @param [in] apcData - the specified outside data block
     * @param [in] auSize - the size of data block
     * @param [in] aiFlag - the enumeration
     */
    void SetBase( char const* apcData, apl_size_t auSize, apl_int_t aiFlag );
    
    /**
     * @brief Get base pointer of data block.
     *
     * @return  The base pointer of data block.
     */
    char* GetBase(void);
    
    /**
     * @brief Get end pointer of data block.
     *
     * @return The end pointer of data block. 
     */
    char* GetEnd(void);
    
    /**
     * @brief Get mark.
     *
     * @return The mark
     */
    char* GetMark(void);
    
    /**
     * @brief Change the size of data block.
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t Resize( apl_size_t auSize );
    
    /**
     * @brief Get the size of data block.
     *
     * @return The data block size.
     */
    apl_size_t GetSize(void) const;
    
    /**
     * @brief Get the capacity of data block.
     *
     * @return The data block capacity.
     */
    apl_size_t GetCapacity(void) const;
    
    /**
     * @brief Clone data block.
     *
     * @return The new clone data block object.
     */
    virtual CDataBlock* Clone(void);
    
    /**
     * @brief Clone data block but no copy.
     *
     * @return The new clone but no copy data block object.
     */
    virtual CDataBlock* CloneNoCopy(void);
    
    /**
     * @brief Duplicate data block and increase reference count.
     *
     * @return The new duplicating one.
     */
    CDataBlock* Duplicate(void);
    
    /**
     * @brief Release data block.
     *
     * @return The released data block object.
     */
    CDataBlock* Release(void);
    
    /**
     * @brief Get the reference count of the data block.
     *
     * @return The reference count.
     */
    apl_int_t GetReferCount(void);
    
    /**
     * @brief Get the enum flag.
     *
     * @return  The enum flag.
     */
    apl_int_t GetFlag(void);

protected:
    CDataBlock( CDataBlock const& );
    CDataBlock& operator = ( CDataBlock const& );

private:
    char* mpcBase;
    
    char* mpcEnd;
    
    apl_size_t muCapacity;
    
    apl_size_t muSize;
    
    apl_int_t miReferCount;
    
    apl_int_t miFlag;
};

////////////////////////////////////////////// MemoryBlock /////////////////////////////////////
/**
 * class CMemoryBlock
 */
class CMemoryBlock
{
public:
    /**
     * An enumeration.
     */
    enum EOption
    {
        DONT_DELETE = 0, ///<Don't delete the data on exit since we don't own it.
        
        DO_DELETE   = 1,     ///<Delete the data on exit since we don't own it.
    };
    
public:
    /** 
     * @brief Default constructor
     */
    CMemoryBlock(void);

    /**
     * @brief A constructor.Construct MemoryBlock.
     *
     * @param [in] apoDataBlock - the specified data block
     * @param [in] apoCont - the memoryblock to attach
     */
    CMemoryBlock( CDataBlock* apoDataBlock, CMemoryBlock* apoCont = NULL );
    
    /**
     * @brief A constructor.Construct MemoryBlock.
     *
     * @param [in] auSize - the size of data block
     * @param [in] apoCont - the memoryblock to attach
     */
    CMemoryBlock( apl_size_t auSize, CMemoryBlock* apoCont = NULL );
    
    /**
     * @brief A constructor.Construct MemoryBlock.
     *
     * @param [in] apcData - the pointer to the outside data block
     * @param [in] auSize - the outside data block size
     * @param [in] aeFlag - the enum flag
     * @param [in] apoCont - the memoryblock to attach
     */
    CMemoryBlock( char const* apcData, apl_size_t auSize, EOption aeFlag = DONT_DELETE, CMemoryBlock* apoCont = NULL );
    
    /**
     * @brief A copy constructor.
     *
     * @param [in] aoMB  - the original MemoryBlock
     */
    CMemoryBlock( CMemoryBlock const& aoMB );
    
    /**
     * @brief A destructor.Release MemoryBlock.
     */
    virtual ~CMemoryBlock(void);
    
    /**
     * @brief Clone MemoryBlock with data block.
     *
     * @return The new MemoryBlock object.
     */
    virtual CMemoryBlock* Clone(void);
    
    /**
     * @brief Duplicate MemoryBlock and increase the reference count of the data block.
     *
     * @return The new MemoryBlock object.
     */
    virtual CMemoryBlock* Duplicate(void);
    
    /**
     * @brief Release MemoryBlock.
     */
    virtual void Release();
    
    /** 
     * @brief Memory block overwrite copy method
     * 
     * @param[in] aoMB right value
     */
    void operator = ( CMemoryBlock const& aoMB );
    
    /**
     * @brief Write buffer to MemoryBlock.
     *
     * @param [in] apcBuffer - the buffer to write
     * @param [in] aiN - write size
     *
     * @return The write size.
     */
    apl_ssize_t Write( void const* apcBuffer, apl_size_t aiN );
    
    /**
     * @brief Write string to MemoryBlock.
     *
     * @param [in] apcBuffer - the string to write
     *
     * @return The write size.
     */
    apl_ssize_t Write( char const* apcBuffer );
    
    /**
     * @brief Read \e aiN bytes from MemoryBlock.
     *
     * @param [out] apcBuffer - the buffer to store the data from MemoryBlock
     * @param [in] aiN - read size
     *
     * @return The number of bytes actually read.
     */
    apl_ssize_t Read( void* apcBuffer, apl_size_t aiN );
    
    /**
     * @brief Reset the MemoryBlock.Put the read-pointer and write-pointer at the base.
     */
    void Reset(void);
    
    /**
     * @brief Get base pointer of data block.
     *
     * @return The base pointer of data block.
     */
    char* GetBase(void) const;
    
    /**
     * @brief Set outside data block.
     *
     * @param [in] apcData - the specified outside data block
     * @param [in] auSize - the size of data block
     * @param [in] aeFlag - the enumeration
     */
    void  SetBase(const char* apcData, apl_size_t auSize, EOption aeFlag);
    
    /**
     * @brief Get end pointer of data block.
     *
     * @return The end pointer of data block. 
     */
    char* GetEnd(void) const;
    
    /**
     * @brief Get the read-pointer.
     *
     * @return The read-pointer.
     */
    char* GetReadPtr(void) const;
    
    /**
     * @brief Set the read-pointer to the parameter-pointer.
     *
     * @param [in] apcPtr - the parameter-pointer
     */
    void  SetReadPtr(char* apcPtr);
    
    /**
     * @brief Set the read-pointer offset \e aiN bytes.
     *
     * @param [in] aiN - the offset size
     */
    void  SetReadPtr(apl_size_t aiN);
    
    /**
     * @brief Get the write-pointer.
     *
     * @return The write-pointer.
     */
    char* GetWritePtr(void) const;
    
    /**
     * @brief Set the write-pointer to the parameter-pointer.
     *
     * @param [in] apcPtr - the parameter-pointer
     */
    void  SetWritePtr(char* apcPtr);
    
    /**
     * @brief Set the write-pointer offset \e aiN bytes.
     *
     * @param [in] aiN - the offset size
     */
    void  SetWritePtr(apl_size_t aiN);
    
    /**
     * @brief Get the size of data block.
     *
     * @return The data block size.
     */
    apl_size_t GetSize(void) const;
    
    /**
     * @brief Get the length.(WritePtr-ReadPtr)
     *
     * @return The length of the MemoryBlock.
     */
    apl_size_t GetLength(void) const;
    
    /**
     * @brief Set the length.(ReadPtr + aiLength)
     *
     * @param [in] aiLength - the spectified length
     */
    void SetLength(apl_size_t aiLength);
    
    /**
     * @brief Change the size of data block.
     *
     * @retval 0 Upon successful completion.
     */
    void Resize(apl_size_t auSize);

    /**
     * @brief Get the capacity of data block.
     *
     * @return The data block capacity.
     */
    apl_size_t GetCapacity(void) const;
    
    /**
     * @brief Get the space size.
     *
     * @return The space size.
     */
    apl_size_t GetSpace(void) const;
    
    /**
     * @brief Get the total length(include attached MemoryBlock).
     *
     * @return The total length.
     */
    apl_size_t GetTotalLength(void) const;
    
    /**
     * @brief Get the total size(include attached MemoryBlock).
     *
     * @return The total size.
     */
    apl_size_t GetTotalSize(void) const;
    
    /**
     * @brief Get the total capacity(include attached MemoryBlock).
     *
     * @return The total capacity.
     */
    apl_size_t GetTotalCapacity(void) const;
    
    /**
     * @brief Get the data block.
     *
     * @return The data block.
     */
    CDataBlock* GetDataBlock(void) const;
    
    /**
     * @brief Set the data block.
     *
     * @param [in] apoDB - the specified data block
     *
     * @return The original data block.
     */
    CDataBlock* SetDataBlock(CDataBlock* apoDB);
 
    /** 
     * @brief Swap data block between memory blocks
     * 
     * @param aoMB memory block
     */
    void SwapDataBlock( CMemoryBlock& aoMB );

    /**
     * @brief Get the attached MemoryBlock.
     *
     * @return  The attached MemoryBlock pointer.
     */
    CMemoryBlock* GetCont(void) const;
    
    /**
     * @brief Set the attached MemoryBlock.
     *
     * @param [in] apoMB - the attached MemoryBlock to set
     */
    void SetCont(CMemoryBlock* apoMB);
    
    /**
     * @brief Get the next MemoryBlock.
     *
     * @return  The next MemoryBlock pointer.
     */
    CMemoryBlock* GetNext(void) const;
    
    /**
     * @brief Set the next MemoryBlock.
     *
     * @param [in] apoMB - the next MemoryBlock to set
     */
    void SetNext(CMemoryBlock* apoMB);
    
    /**
     * @brief Get the prev MemoryBlock.
     *
     * @return  The prev MemoryBlock pointer.
     */
    CMemoryBlock* GetPrev(void) const;
    
    /**
     * @brief Set the prev MemoryBlock.
     *
     * @param [in] apoMB - the prev MemoryBlock to set
     */
    void SetPrev(CMemoryBlock* apoMB);
    
    /**
     * @brief Get the reference count of data block
     *
     * @return The reference count of data block
     */
    apl_int_t GetReferCount(void) const;

protected:
    apl_int_t Initialize(
        apl_size_t auSize, char const* apcData, apl_int_t aiFlag, CDataBlock* apoDB, CMemoryBlock* apoCont );

private:
    CDataBlock* mpoDataBlock;
    
    apl_size_t muReadPos;
    
    apl_size_t muWritePos;
    
    CMemoryBlock* mpoCont;
    
    CMemoryBlock* mpoNext;
    
    CMemoryBlock* mpoPrev;
};

ACL_NAMESPACE_END

#endif//ACL_MEMORY_BLOCK
