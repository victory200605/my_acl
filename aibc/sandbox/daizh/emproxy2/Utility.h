#ifndef AIMC_EMPROXY_UTILITY_H
#define AIMC_EMPROXY_UTILITY_H

const apl_size_t EMP_LINE_MAX = 256;

//{ start emproxy errno define
enum
{
    //
    EMP_ERROR_INIT_MAILBOX        = 10001,
    EMP_ERROR_OPEN_MAILBOX        = 10002,
    EMP_ERROR_LOCK_ACCOUNT        = 10003,
    EMP_ERROR_READ_ACCOUNT_INFO   = 10004,
    EMP_ERROR_READ_UIDLLIST       = 10005,
    EMP_ERROR_APPEND_UIDLLIST     = 10006,
    EMP_ERROR_UNLOCK_ACCOUNT      = 10007,
    EMP_ERROR_STAT_MAILBOX        = 10008
};

inline char const* StrError( apl_int_t aiErrno )
{
    switch(aiErrno)
    {
        case EMP_ERROR_INIT_MAILBOX: return "Init user mailbox fail";
        case EMP_ERROR_OPEN_MAILBOX: return "Open user mailbox fail";
        case EMP_ERROR_LOCK_ACCOUNT: return "Lock user pop account fail";
        case EMP_ERROR_READ_ACCOUNT_INFO: return "Read user pop account info fail";
        case EMP_ERROR_READ_UIDLLIST:     return "Read user pop uidl list fail";
        case EMP_ERROR_APPEND_UIDLLIST:   return "Append user pop uidl list fail";
        case EMP_ERROR_UNLOCK_ACCOUNT:    return "Unlock user pop account fail";
        case EMP_ERROR_STAT_MAILBOX:      return "Stat mailbox fail";
    };

    return "Unknown error";
}
//} end emproxy errno define

//{ start emproxy protocol argv define
enum EGETMAIL
{
    GETMAIL_USERNAME_INDEX                    = 1,
    GETMAIL_POPID_INDEX                       = 2,
    GETMAIL_POPSERVER_INDEX                   = 3,
    GETMAIL_TRACELEVEL_INDEX                  = 4
};

enum ETEST
{
    TEST_USERNAME_INDEX                      = 1,
    TEST_POPSERVER_INDEX                     = 2,
    TEST_PORT_INDEX                          = 3,
    TEST_POPUSER_INDEX                       = 4,
    TEST_PASSWD_INDEX                        = 5,
    TEST_AUTHTYPE_INDEX                      = 6,
    TEST_TIMEOUT_INDEX                       = 7
};
//} end protocol argv define

//{ start mailbox setting key
#define KEY_LOCK                             "Poping"
#define KEY_AUTO                             "AutoPOP"
#define KEY_PORT                             "Port"
#define KEY_SERVER                           "Server"
#define KEY_DELMAIL                          "DeleteMail"
#define KEY_ENABLE                           "Enable"
#define KEY_POPFID                           "Fid"
#define KEY_PASSWORD                         "Password"
#define KEY_USER                             "User"
#define KEY_LOCAL_UIDL_FILENAME              "externalpop" //for store local uidl
#define KEY_EXTERNAL_POP                     "ExternalPOP"
//} end mailbox setting key

//{ start other
#define HEADER_TAG_AIMC                      "X-AIMC-EMPROXY"
#define HEADER_TAG_POPFID                    "X-AIMC-POP2FID"
#define CRYPT_PREFIX                         "~~CRYPT~~"
//} end other

// Log
#define EMP_LOG_TRACE(...)
#define EMP_LOG_INFO(format, ...) printf(format "\n", __VA_ARGS__);
#define EMP_LOG_ERROR(...)
#define EMP_LOG_DEBUG(...)

#define WRITE_MESSAGE(opt, session, ... ) \
    if (opt) \
    { \
        CCmdLineMessage loMessage; \
        loMessage.CmdLine(__VA_ARGS__); \
        anf::CWriteFuture::PointerType loFuture = session->Write(session, &loMessage); \
        if (loFuture->IsException() ) \
        { \
            EMP_LOG_TRACE("ServerHandler write message exception, (State=%"APL_PRIdINT",Errno=%"APL_PRIdINT")", \
                (apl_int_t)loFuture->GetState(), \
                (apl_int_t)loFuture->GetErrno() ); \
        } \
    }

#endif//AIMC_EMPROXY_UTILITY_H

