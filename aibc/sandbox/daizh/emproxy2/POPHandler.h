#ifndef AIMC_EMPROXY_POPHANDLER_H
#define AIMC_EMPROXY_POPHANDLER_H

#include "POPClient.h"
#include "acl/stl/set.h"

/** 
 * @brief Pop config info define
 */
class CPOPConfigInfo
{
public:
    CPOPConfigInfo(void);

    bool IsDownloaded( std::string const& aoID );

    void InsertDownloaded( std::string const& aoID );
    
    void SetUser( char const* apcUser );

    char const* GetUser(void);
    
    void SetPass( char const* apcPass );

    char const* GetPass(void);

    void SetHostName( char const* apcHostName );

    char const* GetHostName(void);

    void SetPort( apl_int_t aiPort );

    apl_int_t GetPort(void);

    void SetDownloadAndDelete( bool abIsEnable );

    bool IsDownloadAndDelete(void);

private: 
    std::string moUser;
    std::string moPass;
    std::string moHostName;
    apl_int_t   miPort;
    bool        mbIsDownloadAndDelete;
    std::set<std::string> moDownloadeds;
};

/** 
 * @brief User pop download new mail handler
 */
class CPOPHandler
{
public:
    CPOPHandler( char const* apcUID, char const* apcMBID, apl_int_t aiMailRoot );

    ~CPOPHandler(void);

    void operator () (void);

protected:
    apl_int_t StartPopHandle( CPOPConfigInfo& aoConfigInfo );

    apl_int_t EndPopHandle( std::vector<CPOPUidlInfo> const& aoNewUidls );

    bool IsDownloaded( std::string const& aoID );

    apl_int_t PostMail( std::string& aoContent );
    
    char const* DecodePassword( char* apcPassword, apl_size_t auBufferSize );

private:
    //For open clsMailBox
    std::string moUID;
    std::string moMBID;
    apl_int_t   miMailRoot;
    apl_int_t   miPopItemNo;
    apl_int_t   miMaxSpaceSize;
    apl_int_t   miMaxMailCount;
};

#endif//AIMC_EMPROXY_HANDLER_H

