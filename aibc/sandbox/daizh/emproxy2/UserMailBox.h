#ifndef AIMC_EMPROXY_USERMAILBOX_H
#define AIMC_EMPROXY_USERMAILBOX_H

#include "acl/MemoryBlock.h"
#include "acl/stl/string.h"

struct CPOPAccountInfo
{
    std::string moUser;
    std::string moPass;
    std::string moHostName;
    apl_int_t   miPort;
    bool        mbIsDownloadAndDelete;
    apl_int_t   miFoldID;
};

class CUserMailBox
{
public:
    CUserMailBox( char const* apcUserName );

    apl_int_t Init(void);

    apl_int_t OpenMailBox(void);

    apl_int_t GetPOPAccountInfo( apl_int_t aiPOPID, CPOPAccountInfo* apoInfo );

    apl_int_t ReadUidlFile( apl_int_t aiPOPID, acl::CMemoryBlock* apoBuffer );

    apl_int_t AppendUidlFile( apl_int_t aiPOPID, std::string const& aoStr );

    apl_int_t LockPOPAccount( apl_int_t aiPOPID );

    apl_int_t UnlockPOPAccount( apl_int_t aiPOPID );

    apl_int_t GetStat( apl_size_t* apuCount, apl_size_t* apuSpace );

    char const* DecodePassword( char* apcPassword, apl_size_t auBufferSize );

    apl_int_t PostMail( std::string const& aoContent );

    void CloseMailBox(void);

    char const* GetUserName(void);

    char const* GetUID(void);

    char const* GetMBID(void);

    apl_int_t GetMailRoot(void);

    apl_int_t GetMaxMailCount(void);

private:
    //Mailbox user info 
    std::string moUserName;
    std::string moUID;
    std::string moMBID;
    apl_int_t   miMailRoot;
    apl_int_t   miMaxMailCount;
};

#endif//AIMC_EMPROXY_USERMAILBOX_H

