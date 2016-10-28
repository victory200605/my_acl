/**
 * @file IniConfig.h
 */

#ifndef ACL_INICONFIG_H
#define ACL_INICONFIG_H

#include "acl/Utility.h"
#include "acl/Config.h"

ACL_NAMESPACE_START

/**
 * class CIniConfig
 */
class CIniConfig : public CBaseConfig
{
public:
    static const apl_int_t MAX_LINE_LEN;   ///<maximum line length

public:
    /**
     * @brief Open ini config file.
     *
     * @param [in] apcFileName - file name
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Open failed.
     */
    apl_int_t Open( char const* apcFileName );
    
    /**
     * @brief Reload ini config file.
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Open failed.
     */
    apl_int_t Reload(void);
    
    /**
     * @brief Synchronize changes to the ini config file.
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Synchronize failed.
     */
    apl_int_t Sync(void);
    
    /**
     * @brief Close ini config file.
     */
    void Close(void);

private:
    std::string moFileName;
};

ACL_NAMESPACE_END

#endif //ACL_INICONFIG_H
