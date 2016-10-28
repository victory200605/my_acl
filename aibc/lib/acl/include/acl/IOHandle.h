/**
 * @file IOHandle.h
 */
  
#ifndef ACL_HANDLE_H
#define ACL_HANDLE_H

#include "acl/Utility.h"

ACL_NAMESPACE_START

#define ACL_IS_VALID_HANDLE( handle ) (handle>=0)
#define ACL_INVALID_HANDLE (-1)

/**
 * class CIOHandle
 */
class CIOHandle
{
public:
    /**
     * IO handle switch option enumeration.
     */
    enum ESWOption
    {
        OPT_NONBLOCK = 1,  ///<non-blocking I/O APL_O_NONBLOCK
        
        OPT_CLOEXEC        ///<close-on-exec APL_CLOEXEC
    };
    
public:
    /**
     * @brief A constructor.
     */
    CIOHandle(void);
    
    /**
     * @brief Set handle.
     *
     * @param [in] aiHandle -   the specified handle
     */
    void SetHandle( apl_handle_t aiHandle );
    
    /**
     * @brief Get handle.
     *
     * @return The handle.
     */
    apl_handle_t GetHandle(void) const;
    
    /**
     * @brief Disable IO handle switch option.
     *
     * @param [in] aeOpt -   the IO handle switch option.
     *
     * @retval 0 If disable IO handle switch option successfully.
     * @retval -1 On any error.
     */
    apl_int_t Disable( ESWOption aeOpt ) const;
    
    /**
     * @brief Enable IO handle switch option.
     *
     * @param [in] aeOpt -   the IO handle switch option.
     *
     * @retval 0  If enable IO handle switch option successfully.
     * @retval -1 On any error. 
     */
    apl_int_t Enable( ESWOption aeOpt ) const;
    
    /**
     * @brief Close IO handle.
     */
    void Close(void);
    
private:
    apl_handle_t miHandle;
};

ACL_NAMESPACE_END

#endif //ACL_HANDLE_H
