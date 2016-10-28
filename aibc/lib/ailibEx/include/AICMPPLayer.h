#ifndef  __AILIBEX__AICMPPLAYER_H__
#define  __AILIBEX__AICMPPLAYER_H__
#include "AICMPPStru.h"
#include "AIMemAlloc.h"
#include "AIIndexLink.h"

///start namespace
AIBC_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////
//
#define  ERROR_CL_SENDDATA              (-90200)
#define  ERROR_CL_RECVDATA              (-90201)
#define  ERROR_CL_INV_LENGTH            (-90202)
#define  ERROR_CL_INV_COMMAND           (-90203)
#define  ERROR_CL_SOCKET_TIMEOUT        (-90204)

///////////////////////////////////////////////////////////////////////////
//
#define  ERROR_CODE_INVALID_PACKET       (1)
#define  ERROR_CODE_INVALID_LEN          (4)
#define  ERROR_CODE_DEST_USR_TL_FAIL     (115)

///////////////////////////////////////////////////////////////////////////
//
typedef struct CMPP_SUBMIT_NODE
{
    LISTHEAD        coChild;
    CMPPSubmit      coSubmit;
} CMPP_SUBMIT_NODE;

typedef clsIndexLink<CMPP_SUBMIT_NODE>  clsSubmitList;

///////////////////////////////////////////////////////////////////////////
//
#define  CL_MAX_BODY_LENGTH             (5120)
#define  CMPP_MAX_DEST_USR_COUNT        (100)

#define  CMPP20_CONNECT_REQ_BODY_LEN    (6+16+1+4)
#define  CMPP20_CONNECT_REP_BODY_LEN    (1+16+1)
#define  CMPP20_SUBMIT_MIN_BODY_LEN     (117+21+1+8)
#define  CMPP30_CONNECT_REQ_BODY_LEN    (6+16+1+4)
#define  CMPP30_CONNECT_REP_BODY_LEN    (4+16+1)
#define  CMPP30_SUBMIT_MIN_BODY_LEN     (129+32+1+1+20)

///////////////////////////////////////////////////////////////////////////
//
extern int RecvCMPPPacket(int aiSocket, CMPPHead *apoCMPPHead, AIChunk *apoAIChunk, int aiTimeout);
extern int SendCMPPPacket(int aiSocket, CMPPHead const *apoCMPPHead, void const*apvBody, int aiTimeout);
extern int LoginCmpp20Gateway(int aiSocket, char const*apcID, char const*apcPasswd, int aiVer, int aiTimeout);
extern int LoginCmpp30Gateway(int aiSocket, char const*apcID, char const*apcPasswd, int aiVer, int aiTimeout);

extern int CheckCMPP20_SUBMIT(CMPPHead *apoCMPPHead, char *apcBody, CMPPSubmit *apoSubmit);
extern int LoadCMPP20_SUBMIT(CMPPHead *apoCMPPHead, char *apcBody, clsSubmitList &aoList);

extern int CheckCMPP30_SUBMIT(CMPPHead *apoCMPPHead, char *apcBody, CMPPSubmit *apoSubmit);
extern int LoadCMPP30_SUBMIT(CMPPHead *apoCMPPHead, char *apcBody, clsSubmitList &aoList);

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AICMPPLAYER_H__ 

