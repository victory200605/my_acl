#ifndef __AILIBEX__AICMPPSTRU_H__
#define __AILIBEX__AICMPPSTRU_H__

#include <inttypes.h>
#include <time.h>

///start namespace
AIBC_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////
//
#define CMPP_HEADER_LEN                    ((uint32_t)12)
#define MAX_CONTENT_LEN                    ((uint8_t)160)

// CMPP_SUBMIT/FWD.ciFeeUserType
#define FEE_USER_TYPE_DEST_CHARGE          ((uint8_t)0)
#define FEE_USER_TYPE_SRC_CHARGE           ((uint8_t)1)
#define FEE_USER_TYPE_SP_CHARGE            ((uint8_t)2)
#define FEE_USER_TYPE_REF_FEE_TERM         ((uint8_t)3)

// CMPP_SUBMIT/DELIVER/FWD.ciMsgFmt
#define MSG_FMT_ASCII                      ((uint8_t)0)
#define MSG_FMT_WRITE_CARD                 ((uint8_t)3)
#define MSG_FMT_BIN                        ((uint8_t)4)
#define MSG_FMT_UCS2                       ((uint8_t)8)
#define MSG_FMT_GB                         ((uint8_t)15)
#define MSG_FMT_BIG5                       ((uint8_t)16)

// CMPP_FWD.ciFwdType
#define FWD_TYPE_MT                        ((uint8_t)0)
#define FWD_TYPE_MO                        ((uint8_t)1)
#define FWD_TYPE_MT_STATUS                 ((uint8_t)2)
#define FWD_TYPE_MO_STATUS                 ((uint8_t)3)

///////////////////////////////////////////////////////////////////////////
//
#define CMPP_CONNECT                       ((uint32_t)0x00000001) 
#define CMPP_CONNECT_REP                   ((uint32_t)0x80000001)
#define CMPP_TERMINATE                     ((uint32_t)0x00000002)
#define CMPP_TERMINATE_REP                 ((uint32_t)0x80000002)
#define CMPP_SUBMIT                        ((uint32_t)0x00000004)
#define CMPP_SUBMIT_REP                    ((uint32_t)0x80000004)
#define CMPP_DELIVER                       ((uint32_t)0x00000005)
#define CMPP_DELIVER_REP                   ((uint32_t)0x80000005)
#define CMPP_QUERY                         ((uint32_t)0x00000006)
#define CMPP_QUERY_REP                     ((uint32_t)0x80000006)
#define CMPP_CANCEL                        ((uint32_t)0x00000007)
#define CMPP_CANCEL_REP                    ((uint32_t)0x80000007)
#define CMPP_ACTIVE_TEST                   ((uint32_t)0x00000008)
#define CMPP_ACTIVE_TEST_REP               ((uint32_t)0x80000008)
#define CMPP_FWD                           ((uint32_t)0x00000009)
#define CMPP_FWD_REP                       ((uint32_t)0x80000009)
#define CMPP_MT_ROUTE                      ((uint32_t)0x00000010)
#define CMPP_MT_ROUTE_REP                  ((uint32_t)0x80000010)
#define CMPP_MO_ROUTE                      ((uint32_t)0x00000011)
#define CMPP_MO_ROUTE_REP                  ((uint32_t)0x80000011)
#define CMPP_GET_MT_ROUTE                  ((uint32_t)0x00000012)
#define CMPP_GET_MT_ROUTE_REP              ((uint32_t)0x80000012)
#define CMPP_MT_ROUTE_UPDATE               ((uint32_t)0x00000013)
#define CMPP_MT_ROUTE_UPDATE_REP           ((uint32_t)0x80000013)
#define CMPP_MO_ROUTE_UPDATE               ((uint32_t)0x00000014)
#define CMPP_MO_ROUTE_UPDATE_REP           ((uint32_t)0x80000014)
#define CMPP_PUSH_MT_ROUTE_UPDATE          ((uint32_t)0x00000015)
#define CMPP_PUSH_MT_ROUTE_UPDATE_REP      ((uint32_t)0x80000015)
#define CMPP_PUSH_MO_ROUTE_UPDATE          ((uint32_t)0x00000016)
#define CMPP_PUSH_MO_ROUTE_UPDATE_REP      ((uint32_t)0x80000016)
#define CMPP_GET_MO_ROUTE                  ((uint32_t)0x00000017)
#define CMPP_GET_MO_ROUTE_REP              ((uint32_t)0x80000017)


#define CMPP_ERROR_INVALID_PACKET       (1)
#define CMPP_ERROR_INVALID_LEN          (4)
#define CMPP_ERROR_INVALID_FEECODE      (5)
#define CMPP_ERROR_TOO_LONG             (6)
#define CMPP_ERROR_INVALID_SERVICEID    (7)
#define CMPP_ERROR_TOO_FAST             (8)
#define CMPP_ERROR_WRONG_FORWARD        (9)
#define CMPP_ERROR_INVALID_SRCTERMID    (10)
#define CMPP_ERROR_INVALID_ICP          (11)
#define CMPP_ERROR_INVALID_FEETERMID    (12)
#define CMPP_ERROR_INVALID_DESTTERMID   (13)
#define CMPP_ERROR_REJECT_WAPPUSH       (14)
#define CMPP_ERROR_UNMATCH_ICP_TYPE     (15)
#define CMPP_ERROR_IN_BLACK_LIST        (16)
#define CMPP_ERROR_SAVE_GFQ             (17)
#define CMPP_ERROR_INVALID_SRCGWID      (18)
#define CMPP_ERROR_INVALID_DESTGWID     (19)
#define CMPP_ERROR_INVALID_FWDTYPE      (20)
#define CMPP_ERROR_INVALID_REGDEL       (21)
#define CMPP_ERROR_INVALID_MSGFORMAT    (22)
#define CMPP_ERROR_INVALID_SERVICENO    (23)
#define CMPP_ERROR_INVALID_FEETYPE      (24)
#define CMPP_ERROR_INVALID_VALIDTIME    (25)
#define CMPP_ERROR_INVALID_ATTIME       (26)
#define CMPP_ERROR_INVALID_USER_TYPE    (27)
#define CMPP_ERROR_SMMC_REJECTD         (28)
#define CMPP_ERROR_SP_BLACKLIST         (29)
#define CMPP_ERROR_MSG_EXPIRED          (30)
#define CMPP_ERROR_INVALID_TPPID        (31)
#define CMPP_ERROR_SMFILTER_REJECTD     (32)
#define CMPP_ERROR_TESTICP_REJECTD      (33)
#define CMPP_ERROR_UUM_REJECTD_BASE     (34)


#define CMPP_STAT_DELIVRD   "DELIVRD"
#define CMPP_STAT_EXPIRED   "EXPIRED"
#define CMPP_STAT_DELETED   "DELETED"
#define CMPP_STAT_UNDELIV   "UNDELIV"
#define CMPP_STAT_ACCEPTD   "ACCEPTD"
#define CMPP_STAT_UNKNOWN   "UNKNOWN"
#define CMPP_STAT_REJECTD   "REJECTD"
#define CMPP_STAT_NOROUTE   "NOROUTE"


///////////////////////////////////////////////////////////////////////////
typedef struct 
{
    uint32_t    ciTotalLength;
    uint32_t    ciCommandID;
    uint32_t    ciInvokeID;
} CMPPHead;


typedef struct 
{
    char        csSourceAddr[6+1];
    char        csAuthenticatorSource[16];
    uint8_t     ciVersion;
    uint32_t    ciTimestamp;
} CMPPConnect;


typedef struct 
{
    uint32_t    ciStatus;
    char        csAuthenticatorISMG[16];
    uint8_t     ciVersion;
} CMPPConnectRep;


typedef struct 
{
    char        csMsgID[8];
    uint8_t     ciPkTotal;
    uint8_t     ciPkNumber;
    uint8_t     ciRegisteredDelivery;
    uint8_t     ciMsgLevel;
    char        csServiceID[10+1];
    uint8_t     ciFeeUserType;
    char        csFeeTerminalID[32+1];
    uint8_t     ciFeeTerminalType;
    uint8_t     ciTpPID;
    uint8_t     ciTpUDHI;
    uint8_t     ciMsgFmt;
    char        csMsgSrc[6+1];
    char        csFeeType[2+1];
    char        csFeeCode[6+1];
    char        csValidTime[17+1];
    char        csAtTime[17+1];
    char        csSrcID[21+1];
    uint8_t     ciDestUsrTL;
    char        csDestTerminalID[32+1];
    uint8_t     ciDestTerminalType;
    uint8_t     ciMsgLength;
    char        csMsgContent[MAX_CONTENT_LEN+1];
    char        csLinkID[20+1];
    
    // Extra Information //
    char        csExpandMsgID[21];
} CMPPSubmit;

typedef struct 
{
    char        csMsgID[8];
    uint32_t    ciResult;
} CMPPSubmitRep;

typedef struct
{
    char        csMsgID[8];
    char        csDestID[21+1];
    char        csServiceID[10+1];
    uint8_t     ciTpPID;
    uint8_t     ciTpUDHI;
    uint8_t     ciMsgFmt;
    char        csSrcTerminalID[32+1];
    uint8_t     ciSrcTerminalType;
    uint8_t     ciRegisteredDelivery;
    uint8_t     ciMsgLength;
    uint8_t     csMsgContent[MAX_CONTENT_LEN+1];
    char        csLinkID[20+1];
    
    // For Stat Report //
    char        csSRMsgID[8];
    char        csSRStatus[7+1];
    char        csSRSubmitTime[10+1];
    char        csSRDoneTime[10+1];
    char        csSRDestTerminalID[32+1];
    uint32_t    ciSRSmscSequence;

    // Extra Information //
    char        csExpandMsgID[21];
    char        csExpandSRMsgID[21];
} CMPPDeliver;

typedef struct stCMPP_DELIVER_REP
{
    char        csMsgID[8];
    uint32_t    ciResult;
} CMPPDeliverRep;

typedef struct stCMPP_FWD
{
    char        csSourceID[6+1];
    char        csDestinationID[6+1];
    uint8_t     ciNodeCount;
    uint8_t     ciFwdType; 
    char        csMsgID[8];
    uint8_t     ciPkTotal;
    uint8_t     ciPkNumber;
    uint8_t     ciRegisteredDelivery;
    uint8_t     ciMsgLevel;
    char        csServiceID[10+1];
    uint8_t     ciFeeUserType;
    char        csFeeTerminalID[21+1];
    char        csFeeTerminalPseudo[32+1];
    uint8_t     ciFeeTerminalUserType;
    uint8_t     ciTpPID;
    uint8_t     ciTpUDHI;
    uint8_t     ciMsgFmt;
    char        csMsgSrc[6+1];
    char        csFeeType[2+1];
    char        csFeeCode[6+1];
    char        csValidTime[17+1];
    char        csAtTime[17+1];
    char        csSrcTerminalID[21+1];
    char        csSrcTerminalPseudo[32+1];
    uint8_t     ciSrcTerminalUserType;
    uint8_t     ciSrcTerminalType;
    uint8_t     ciDestUsrTL;
    char        csDestTerminalID[21+1];
    char        csDestTerminalPseudo[32+1];
    uint8_t     ciDestTerminalUserType;
    uint8_t     ciMsgLength;
    char        csMsgContent[MAX_CONTENT_LEN+1];
    char        csLinkID[20+1];

    // For Stat Report //
    char        csSRMsgID[8];
    char        csSRStatus[7+1];
    char        csSRSubmitTime[10+11];
    char        csSRDoneTime[10+11];
    char        csSRDestTerminalID[32+1];
    uint32_t    ciSRSmscSequence;

    // Extra Information //
    char        csExpandMsgID[20+1];
    char        csExpandSRMsgID[20+1];
} CMPPFwd;

typedef struct stCMPP_FWD_REP
{
    char        csMsgID[8];
    uint8_t     ciPkTotal;
    uint8_t     ciPkNumber;
    uint32_t    ciResult;
} CMPPFwdRep;

extern  void EncodeCMPPMsgID(char* apcMsgID, int aiMsgID, time_t aiTime, char const*apcISMGNo);
extern  void EncodeCMPPMsgID(char* apcMsgID, int aiMsgID, char const* apcTime, char const* apcISMGNo);

extern  void SGIPToCMPPMsgID(char *apcMsgID, char const* apcSGIPMsgID);

extern  void DecodeMsgID(int* apiMsgID, char* apcISMGNo, char* apcTime, char const* apcMsgID);
extern  void GetRateMsgID(char* apcRateMsgID, char const* apcMsgID);

extern  int Remove86(char* apcPhoneNo);

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AICMPPSTRU_H__

