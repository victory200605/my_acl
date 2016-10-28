/**
 * @file FixMemoryPool.h
 */

#ifndef ACL_FIXMEMORYPOOL_H
#define ACL_FIXMEMORYPOOL_H

#include "acl/Utility.h"

ACL_NAMESPACE_START

/**
 * class CFixMemoryPool
 */
class CFixMemoryPool
{
public:
    /**
     * @brief A constructor.
     */
    CFixMemoryPool(void);
    
    /**
     * @brief A destructor.
     */
    ~CFixMemoryPool(void);
    
    /**
     * @brief Initialize fix memory pool.
     *
     * @param [in] aiBlockSize - the block size
     * @param [in] aiBlockCount - the block count
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.
     */
    apl_int_t Initialize( apl_size_t aiBlockSize, apl_size_t aiBlockCount );
    
    /**
     * @brief Initialize fix memory pool according to the specified memory.
     *
     * @param [in,out] apvBuffer - the specified memory block for the memory pool
     * @param [in] aiSize - the memory size
     * @param [in] aiBlockSize - the block size
     * @param [in] aiBlockCount - the block count
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed. 
     */
    apl_int_t Initialize( void* apvBuffer, apl_size_t aiSize, apl_size_t aiBlockSize, apl_size_t aiBlockCount );
    
    /**
     * @brief Allocate the size of the memory.
     * \e aiSize should <= \e aiBlockSize
     *
     * @param [in] aiSize - the size to allocate
     */
    void* Allocate( apl_size_t aiSize );

    /**
     * @brief Deallocate the specified memory.
     *
     * @param [in] apvPtr - the pointer to the specified memory
     */
    void Deallocate( void* apvPtr );

    /**
     * @brief Release the fix memory pool.
     */
    void Release();
    
    /**
     * @brief Reset the fix memory pool.
     */
    void Reset();
        
    /**
     * @brief Get the size of fix memory pool.
     *
     * @return Get the size of fix memory pool.
     */
    apl_size_t GetSize(void);
    
    /**
     * @brief Get the space of fix memory pool.
     *
     * @return Get the space of fix memory pool.
     */
    apl_size_t GetSpace(void);

protected:
    apl_int_t Initialize( void* apvBuffer, apl_size_t aiBlockSize, apl_size_t aiBlockCount, bool abIsDoDelete );
    
private:
    apl_size_t muBlocksAvailable;
    
    apl_size_t muFirstAvailableBlock;
    
    unsigned char* mpcData;
    
    bool mbIsDoDelete;
    
    apl_size_t muBlockSize;
    
    apl_size_t muBlockCount;
};

ACL_NAMESPACE_END

#endif//ACL_MEMORYPOOL_H
