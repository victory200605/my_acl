/**
 * @file XmlConfig.h
 */

#ifndef ACL_XMLCONFIG_H
#define ACL_XMLCONFIG_H

#include "acl/Utility.h"
#include "acl/Config.h"

ACL_NAMESPACE_START

/**
 * class CXmlConfig
 */
class CXmlConfig : public CBaseConfig
{
public:
    static const apl_int_t MAX_LINE_LEN;   ///<maximum line length

public:
    /**                                     
     * @brief Open xml config file.         
     *                                       
     * @param [in] apcFileName    file name  
     *                                       
     * @retval 0 Upon successful completion.
     * @retval -1 Open failed.              
     */                                     
    apl_int_t Open( char const* apcFileName );
    
    /**
     * @brief Reload xml config file.
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Open failed.              
     */
    apl_int_t Reload(void);
    
    /**
     * @brief Synchronize changes to the xml config file.
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Synchronize failed.       
     */
    apl_int_t Sync(void);
    
    /**
     * @brief Close xml config file.
     */
    void Close(void);

private:
    std::string moFileName;
};

ACL_NAMESPACE_END

#endif //ACL_XMLCONFIG_H
