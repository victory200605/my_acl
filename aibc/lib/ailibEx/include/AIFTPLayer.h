/** 
 * @file AIFTPLayer.h
 */

#ifndef __AILIBEX__AIFTPLAYER_H__
#define __AILIBEX__AIFTPLAYER_H__

#include "AITcpSocket.h"
#include "AITime.h"

///start namespace
AIBC_NAMESPACE_START

#define AI_FTP_CMD_USER "USER"
#define AI_FTP_CMD_PASS "PASS"
#define AI_FTP_CMD_ACCT "ACCT"
#define AI_FTP_CMD_CWD  "CWD" 
#define AI_FTP_CMD_CDUP "CDUP"
#define AI_FTP_CMD_SMNT "SMNT"
#define AI_FTP_CMD_QUIT "QUIT"
#define AI_FTP_CMD_REIN "REIN"
#define AI_FTP_CMD_PORT "PORT"
#define AI_FTP_CMD_PASV "PASV"
#define AI_FTP_CMD_TYPE "TYPE"
#define AI_FTP_CMD_STRU "STRU"
#define AI_FTP_CMD_MODE "MODE"
#define AI_FTP_CMD_RETR "RETR"
#define AI_FTP_CMD_STOR "STOR"
#define AI_FTP_CMD_STOU "STOU"
#define AI_FTP_CMD_APPE "APPE"
#define AI_FTP_CMD_ALLO "ALLO"
#define AI_FTP_CMD_REST "REST"
#define AI_FTP_CMD_RNFR "REST"
#define AI_FTP_CMD_RNTO "RNTO"
#define AI_FTP_CMD_ABOR "ABOR"
#define AI_FTP_CMD_DELE "DELE"
#define AI_FTP_CMD_RMD  "RMD"
#define AI_FTP_CMD_MKD  "MKD"
#define AI_FTP_CMD_PWD  "PWD"
#define AI_FTP_CMD_LIST "LIST"
#define AI_FTP_CMD_NLST "NLST"
#define AI_FTP_CMD_SITE "SITE"
#define AI_FTP_CMD_SYST "SYST"
#define AI_FTP_CMD_STAT "STAT"
#define AI_FTP_CMD_HELP "HELP"
#define AI_FTP_CMD_NOOP "NOOP"

#define AI_FTP_STATUS_OF_PPR(e)     ((e)/100%10 == 1)
#define AI_FTP_STATUS_OF_PCR(e)     ((e)/100%10 == 2)
#define AI_FTP_STATUS_OF_PIR(e)     ((e)/100%10 == 3)
#define AI_FTP_STATUS_OF_TNCR(e)    ((e)/100%10 == 4)
#define AI_FTP_STATUS_OF_PNCR(e)    ((e)/100%10 == 5)

#define AI_FTP_STATUS_ABOUT_SYNX(e) ((e)/10%10 == 0)
#define AI_FTP_STATUS_ABOUT_INFO(e) ((e)/10%10 == 1)
#define AI_FTP_STATUS_ABOUT_CONN(e) ((e)/10%10 == 2)
#define AI_FTP_STATUS_ABOUT_AUTH(e) ((e)/10%10 == 3)
#define AI_FTP_STATUS_ABOUT_UNSP(e) ((e)/10%10 == 4)
#define AI_FTP_STATUS_ABOUT_FSYS(e) ((e)/10%10 == 5)

#define AI_FTP_ERROR_SOCKET     (-90701)
#define AI_FTP_ERROR_TOOBIG     (-90702)
#define AI_FTP_ERROR_RESPLEN    (-90703)
#define AI_FTP_ERROR_REPLYCODE  (-90704)
#define AI_FTP_ERROR_REPLYMSG   (-90705)
#define AI_FTP_ERROR_ADDRESS    (-90706)
#define AI_FTP_ERROR_DATASOCK   (-90707)
#define AI_FTP_ERROR_OPENFILE   (-90708)
#define AI_FTP_ERROR_SENDFILE   (-90709)
#define AI_FTP_ERROR_USER       (-90710)
#define AI_FTP_ERROR_PASS       (-90711)
#define AI_FTP_ERROR_CWD        (-90712)
#define AI_FTP_ERROR_PASV       (-90713)
#define AI_FTP_ERROR_STOR       (-90714)
#define AI_FTP_ERROR_TYPE       (-90715)

/**
 * @brief class clsFTPConnection
 */ 
class clsFTPConnection
    :public clsTcpSocket
{
public:
    clsFTPConnection(int aiSocket = -1);
    virtual ~clsFTPConnection();

    /**
     * @brief Get reply code.
     *
     * @return Return reply code.
     */ 
    int GetReplyCode() const;

    /**
     * @brief Get reply message.
     *
     * @return Return reply message.
     */ 
    char const* GetReplyMsg() const;

    /**
     * @brief Connect to FTP server.
     *
     * @param apcIP      IN - FTP server ip address
     * @param aiPort     IN - FTP server port
     * @param aiTimeout  IN - timeout,default 5s
     *
     * @return Return 0 if successfully connect.
     */ 
    int ConnectTo(char const* apcIP, int aiPort, AITime_t aiTimeout = 5 * AI_TIME_SEC);

    /**
     * @brief Send command.
     *
     * @param apcCommand      IN - the command
     * @param apcArgs         IN - command arguments
     * @param aiTimeout       IN - timeout,default 5s
     *
     * @return Return 0 if success.Otherwise,<0 shall be returned to indicate the error.
     */ 
    int SendCommand(char const* apcCommand, char const* apcArgs, AITime_t aiTimeout = 5 * AI_TIME_SEC);

    /**
     * @brief Receive response.
     *
     * @param aiTimeout       IN - timeout,default 0
     *
     * @return Return 0 if success.Otherwise,<0 shall be returned to indicate the error.
     */ 
    int RecvResponse(AITime_t aiTimeout = 0);

    /**
     * @brief Handle command.
     *
     * @param apcCommand      IN - the command
     * @param apcArgs         IN - command arguments
     * @param aiTimeout       IN - timeout,default 5s
     *
     * return Return 0 if success.Otherwise,error.
     */ 
    int HandleCommand(char const* apcCommand, char const* apcArgs, AITime_t aiTimeout = 5 * AI_TIME_SEC);
    
    /**
     * @brief Login FTP server.
     *
     * @param apcUser     IN - user name
     * @param apcPass     IN - password
     * @param aiTimeout   IN - timeout,default 5s
     *
     * @return Return 0 if successfully login.Otherwise,<0 shall be returned to indicate the error.
     */ 
    int Login(char const* apcUser, char const* apcPass, AITime_t aiTimeout = 5 * AI_TIME_SEC);

    /**
     * @brief Put local file to remote directory.
     * 
     * @param apcRemoteDir       IN - the remote directory
     * @param apcLocalFile       IN - the local file
     * @param aiTimeout          IN - timeout,default 5s
     *
     * @return Return 0 if success.Otherwise,<0 shall be returned to indicate the error.
     */ 
    int PutFile(char const* apcRemoteDir, char const* apcLocalFile, AITime_t aiTimeout = 5 * AI_TIME_SEC);

protected:
    int ciReplyCode;
    char csReplyMsg[1024];
};

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AIFTP_H__

