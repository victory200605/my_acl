/**
 * @file Memmap.h
 */
#ifndef ACL_MEMMAP_H
#define ACL_MEMMAP_H

#include "acl/Utility.h"

ACL_NAMESPACE_START

/**
 * class CMemMap
 *
 */
class CMemMap
{
public:

    /**
     * Constructor
     */
    CMemMap(void);
    
    /**
     * Destructor 
     */
    ~CMemMap(void);

    /**
     * Establish a mapping between the address space of the process at an address for aiLen bytes
     * to the memory object represented by the file path apcFileName at offset ai64Off for aiLen bytes
     *
     * @param [in] apcFileName    file path
     * @param [in] aiProt         determines whether read, write, execute, or some combination of accesses are permitted to the data being mapped. detail in APL mman.c
     * @param [in] aiFlag         provides other information about the handling of the mapped data. detail in APL mman.c
     * @param [in] aiLen          indicate the length of file which represented by ai_fildes to map to memory. Note:This argument shall be multiple of the page size
     * @param [in] ai64Off        offset in the file represented by file path 
     * @retval the address at which the mapping was placed. If success
     * @retval -1. If failed
     */
    apl_int_t Map(
        char const* apcFileName,
        apl_int_t aiProt,
        apl_int_t aiFlag,
        apl_size_t aiLen = 0,
        apl_int64_t ai64Off = 0 );
    
    /**
     * Establish a mapping between the address space of the process at an address for aiLen bytes
     * to the memory object represented by the /dev/shm file path apcFileName at offset ai64Off for aiLen bytes
     *
     * @param [in] apcFileName    file path
     * @param [in] aiProt         determines whether read, write, execute, or some combination of accesses are permitted to the data being mapped. detail in APL mman.c
     * @param [in] aiFlag         provides other information about the handling of the mapped data. detail in APL mman.c
     * @param [in] aiLen          indicate the length of file which represented by ai_fildes to map to memory. Note:This argument shall be multiple of the page size
     * @param [in] ai64Off        offset in the file represented by file path 
     * @retval the address at which the mapping was placed. If success
     * @retval -1. If failed
     */
    apl_int_t ShmMap(
        char const* apcFileName,
        apl_int_t aiProt,
        apl_int_t aiFlag,
        apl_size_t aiLen = 0,
        apl_int64_t ai64Off = 0 );
    
    /**
     * Establish a mapping between the address space of the process at an address for aiLen bytes
     * to the memory object represented by the file handle aiFileHandle at offset ai64Off for aiLen bytes
     *
     * @param [in] aiFileHandle    file handle 
     * @param [in] aiProt          determines whether read, write, execute, or some combination of accesses are permitted to the data being mapped. detail reference to APL MemMap
     * @param [in] aiFlag          provides other information about the handling of the mapped data. detail reference to APL MemMap
     * @param [in] aiLen           indicate the length of file which represented by ai_fildes to map to memory. Note:This argument shall be multiple of the page size
     * @param [in] ai64Off         offset in the file represented by file path 
     * @retval the address at which the mapping was placed. If success
     * @retval -1. If failed
     */
    apl_int_t Map(
        apl_handle_t aiFileHandle,
        apl_int_t aiProt,
        apl_int_t aiFlag,
        apl_size_t aiLen,
        apl_int64_t ai64Off = 0 );
    
    /**
     * Write all modified data to permanent storage locations.
     * 
     * @param [in] aiFlag    the flag of sync. detail reference to APL MemMap
     *
     * @retval 0 If success
     * @retval -1 If failed
     */
    apl_int_t Sync( apl_int_t aiFlag = APL_MS_SYNC );

    /**
     * Remove any mappings for those entire pages containing any part of the address space of the process.
     *
     * @retval 0 If sucess
     * @retval -1 If fail
     */
    apl_int_t Unmap( void );
    
    /**
     * Lock the mapping the pages.
     *
     * @retval 0 If success
     * @retval -1 If failed
     */
    apl_int_t MLock(void);
    
    /**
     * Unlock the mapping the pages.
     *
     * @retval 0 If success
     * @retval -1 If failed
     */
    apl_int_t MUnlock(void);
    
    /**
     * If it is mapped.
     *
     * @retval true  it is mapped
     * @retval false it isn't mapped
     */
    bool IsMaped() const;
    
    /**
     * Get the address at which the mapping was placed.
     *
     * @return the address at which the mapping was placed
     */
    void const* GetAddr() const;
    
    /**
     * Get the address at which the mapping was placed.
     *
     * @return the address at which the mapping was placed
     */
    void* GetAddr();
    
    /**
     * Get the len of mapping.
     *
     * @return the len of mapping
     */ 
    apl_size_t GetSize() const;
    
protected:
    void* mpvAddr;
    
    apl_size_t muSize;
};

ACL_NAMESPACE_END

#endif//ACL_MEMMAP_H
