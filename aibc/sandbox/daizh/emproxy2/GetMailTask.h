#ifndef AIMC_EMPROXY_POPMAILTASK_H
#define AIMC_EMPROXY_POPMAILTASK_H

#include "POPClient.h"
#include "UserMailBox.h"
#include "acl/stl/set.h"
#include "anf/IoSession.h"
#include "Utility.h"

/** 
 * @brief User pop download new mail handler
 */
class CGetMailTask
{
public:
    CGetMailTask( anf::SessionPtrType& aoSession, char const* apcUserName, apl_int_t aiPOPID, apl_int_t aiTraceLevel );

    ~CGetMailTask(void);

    void operator () (void);

protected:
    apl_int_t StartPopHandle( CPOPAccountInfo* apoInfo );

    apl_int_t EndPopHandle( std::string const& aoNewUidls );

    bool CheckMailBoxLimit( apl_size_t auMailSize );

    bool CheckAndModifyMailHeader( CPOPAccountInfo const& aoInfo, std::vector<std::string>& aoHeader );

    apl_int_t PostMail( std::vector<std::string> const& aoHeader, std::string const& aoContent );
    
    bool IsDownloaded( std::string const& aoUidl );

    void InsertDownloaded( std::string const& aoUidl );

private:
    anf::SessionPtrType moSession;

    CUserMailBox moUserMailBox;
    apl_int_t   miPOPID;
    apl_int_t   miTraceLevel;
    std::set<std::string> moDownloadeds;
};

#endif//AIMC_EMPROXY_HANDLER_H

