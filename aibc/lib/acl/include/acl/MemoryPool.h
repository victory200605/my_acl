/**
 * @file MemoryPool.h
 */

#ifndef ACL_MEMORYPOOL_H
#define ACL_MEMORYPOOL_H

#include "acl/Utility.h"
#include "acl/Ring.h"

ACL_NAMESPACE_START

/**
 * class CMemoryPool
 */
class CMemoryPool
{
private:
    struct CBlockNode 
    {
        enum { HEAD_SIZE = 2 * sizeof(apl_int32_t) };
        
        apl_int32_t miPrevLength;
        
        apl_int32_t miLength;
        
        ACL_RING_ENTRY(CBlockNode) moLink;
    };
    
public:
    /**
     * @brief A constructor.
     */
    CMemoryPool(void);
    
    /**
     * @brief A destructor.
     */
    ~CMemoryPool(void);
    
    /**
     * @brief Initialize memory pool.
     * 
     * @param [in] aiPoolSize - the size of memory pool
     *
     * @retval 0 Initialize memory pool successfully.
     * @retval -1 Failed.
     */
    apl_int_t Initialize( apl_size_t aiPoolSize );
    
    /**
     * @brief Allocate the size of the memory.
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
     * @brief Release the memory pool.
     */
    void Release();
    
    /**
     * @brief Reset the memory pool.
     */
    void Reset();
        
    /**
     * @brief Get the size of memory pool.
     *
     * @return The size of memory pool.
     */
    apl_size_t GetSize(void);
    
    /**
     * @brief Get the space of memory pool.
     *
     * @return The space of memory pool.
     */
    apl_size_t GetSpace(void);

private:
    apl_size_t muPoolSize;
    
    apl_size_t muUsedSize;
    
    unsigned char* mpcData;
    
    unsigned char* mpcEnd;
    
    ACL_RING_HEAD(MyNodeHeader, CBlockNode) moHead;
};

ACL_NAMESPACE_END

#endif//ACL_MEMORYPOOL_H
