#ifndef __AILIBEX__AISNMP_H__ 
#define __AILIBEX__AISNMP_H__
 
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <math.h>
#include <stddef.h>
#include <pthread.h>
#include "AIString.h"
#include "AISocket.h"
#include "AISynch.h"
#include "AIChunkEx.h"
#include "AILogSys.h"
#include "AIBer.h"

#define AI_SNMP_MAXLINE             16384        //buffer size
#define AI_SNMP_MAX_LEN             256          
#define AI_SNMP_MAX_VARLIST_NUM     64           //the number of variable-bindings
#define AI_SNMP_OID_NUM             20           //oid array size
#define _StringCopy(d, s)  StringCopy(d, s, sizeof(d))

//pdu type
#define AI_SNMP_BER_GET             0xA0
#define AI_SNMP_BER_GETNEXT         0xA1
#define AI_SNMP_BER_GETRESP         0xA2
#define AI_SNMP_BER_SET             0xA3
#define AI_SNMP_BER_TRAP            0xA4

//errer status
#define AI_SNMP_ERRSTAT_NOERR       0          //noError
#define AI_SNMP_ERRSTAT_TOOBIG      1          //tooBig
#define AI_SNMP_ERRSTAT_NOSUCHNAME  2          //noSuchName
#define AI_SNMP_ERRSTAT_BADVAL      3          //badValue
#define AI_SNMP_ERRSTAT_READONLY    4          //readOnly
#define AI_SNMP_ERRSTAT_GENERR      5          //general error

//errno
#define AI_SNMP_ERRSEQ	            -1      //sequence build error
#define AI_SNMP_ERRSEQP             -2      //sequence parse error
#define AI_SNMP_ERRINT	            -3      //int build error
#define AI_SNMP_ERRINTP             -4      //int parse error
#define AI_SNMP_ERRSTR	            -5      //string build error
#define AI_SNMP_ERRSTRP             -6      //string parse error
#define AI_SNMP_ERROID	            -7      //OID build error 
#define AI_SNMP_ERROIDP             -8      //OID parse error
#define AI_SNMP_ERRUINT             -9      //unsigned int build error
#define AI_SNMP_ERRUINTP            -10     //unsigned int parse error
#define AI_SNMP_ERRVARB             -11     //variable_binding build error
#define AI_SNMP_ERRVARBP            -12     //variable_binding parse error
#define AI_SNMP_EPBUILD             -13     //PDU build error
#define AI_SNMP_EPPARSE             -14     //PDU parse error
#define AI_SNMP_ERRHEADP            -15     //header parse error

#define AI_SNMP_ERRTYPE             -21     //error variable type
#define AI_SNMP_ERRSETV             -22     //set variable oid error
#define AI_SNMP_ERROIDS             -23     //OID should be numbers or '.'
#define AI_SNMP_ERRCOV              -24     //cannot be convert correctly
#define AI_SNMP_EWRONGCOMMAND       -25     //wrong requist command
#define AI_SNMP_EPSEND              -26     //PDU send error
#define AI_SNMP_EPRECV              -27     //PDU cannot be received
#define AI_SNMP_EARRTOOID           -28     //convert character string to unsigned long array error

#define AI_SNMP_ESENDTO             -31     //sendto error	
#define AI_SNMP_EBIND               -32     //bind error
#define AI_SNMP_ESELECT             -33     //select error
#define AI_SNMP_ETIMEOUT            -34     //time out
#define AI_SNMP_ERECV               -35     //recvfrom error on socket
#define AI_SNMP_ESOCKET             -36     //socket error
#define AI_SNMP_EINVALSTRING        -37     //input is not a valid IPv4 dotted-decimal string  or a valid IPv6 address string
#define AI_SNMP_ERRPTON             -38     //inet_pton error 
#define AI_SNMP_ERRNTOP             -39     //inet_ntop error
#define AI_SNMP_ERRMAL              -40     //error malloc
#define AI_SNMP_EFCNTL              -41

#define AI_SNMP_AGENT_ECANTREG      -51     //If the oid Prefix  you specified is already exist in CallbackTable.Cannot register!
#define AI_SNMP_AGENT_ECANTUREG     -52     //Can't find right item to unregister.
#define AI_SNMP_AGENT_ENODATA       -53     //No data in the table.
#define AI_SNMP_AGENT_EGETDATA      -54     //Get Data error
#define AI_SNMP_AGENT_EFINDNEXTDATA -55     //FindNextData error
#define AI_SNMP_AGENT_EARRTOLONG    -56     //convert character string to long int array error
#define AI_SNMP_AGENT_ESAMEOIDPREFIX-57     //same oid prefix(such as 1.1.2/1.1/1.1.2.1 can't exist at the same time)

typedef union 
{
    long	      ciInteger;
    char     	      csString[AI_SNMP_MAX_LEN];
    char      	      csObjid[AI_SNMP_MAX_LEN];
    char     	      csBitstring[AI_SNMP_MAX_LEN];
} unNetsnmpVardata;

typedef struct stNetVariableBinding
{   
    char   		csName[AI_SNMP_MAX_LEN];                        ///<Object identifier of variable
    unsigned char	ccType;	                                        ///<BER type of variable
    int 		ciValLen;                                       ///<the length of the value to be copied into buf 
    unNetsnmpVardata    coVal; 	                                        ///<value of variable
}stVariableBinding;

struct stNetPdu
{
    long            	ciVersion;	                                ///<SNMP version
    char   	        csCommunity[AI_SNMP_MAX_LEN];                   ///<Community
    int             	ciCommand;                                      ///<PDU Type
    long            	ciReqid;                                        ///<Request-id
    long            	ciErrstat;                                      ///<Error-status
    long            	ciErrindex;                                     ///<Error-index
    int                 ciVBNum;                                        ///<Number of variable-bindings
    stVariableBinding   coVariables[AI_SNMP_MAX_VARLIST_NUM];           ///<Variable-binding
};
typedef struct stNetPdu stSnmpPdu;

typedef struct stNetTrap
{
    int                  	 ciCommand; 				///<PDU Type
    char     		     	 csEnterprise[AI_SNMP_MAX_LEN];		///<Enterprise	
    long                 	 ciTrapType;				///<Generic-trap
    long                 	 ciSpecificType;			///<Specific-trap
    char                  	 csAgentAddr[AI_SNMP_MAX_LEN];          ///<agent address
    unsigned long         	 ciTime;   				///<Time-stamp
    stVariableBinding            coVariables[AI_SNMP_MAX_VARLIST_NUM];	///<Variable-binding
}stSnmpTrap;


/**
* @brief SnmpPduBuild - build PDU packet(include: get/getnext/getresponse/set)
*
* @param apoPdu			IN - the original data
* @param apcBuffer 		IN/OUT - pointer to start of output buffer 
* @param aiMaxSize		IN - the buffer size
*
* @return Return the length of the buffer.Return a negative value on any error.
*/
int SnmpPduBuild(stSnmpPdu *apoPdu, unsigned char * apcBuffer, size_t aiMaxSize);

/**
* @brief SnmpTrapBuild - build trap packet
*
* @param apoPdu			IN - the original data
* @param apcBuffer 		IN/OUT - pointer to start of output buffer 
* @param aiMaxSize		IN - the buffer size
*
* @return Return the length of the buffer.Return a negative value on any error.
*/
int SnmpTrapBuild(stSnmpTrap *apoPdu, unsigned char * apcBuffer, size_t  aiMaxSize);

/**
* @brief SnmpPduParse - parse pdu packet(include: get/getnext/getresponse/set)
*
* @param apoPdu			OUT - the struct to store the result
* @param apcBuffer 		IN - pointer to start of input buffer 
* @param aiMaxSize		IN - the buffer size
*
* @return Return not 0 on any error.
*/
int SnmpPduParse(stSnmpPdu *apoPdu, unsigned char * apcBuffer, size_t aiMaxSize);

/**
* @brief SnmpTrapParse - parse trap packet
*
* @param apoPdu			OUT - the struct of snmp_pdu to store the result
* @param apcBuffer 		IN - pointer to start of input buffer 
* @param aiMaxSize		IN - the buffer size
*
* @return Return not 0 on any error.
*/
int SnmpTrapParse(stSnmpTrap *apoPdu, unsigned char * apcBuffer, size_t aiMaxSize);

/**
* @brief SnmpSendPdu - send the packet via PDU
*
* @param aiSocket		IN - the input socket identifier
* @param apoPdu 		IN - the original data
* @param apcRemoteIp	        IN - the remote IP address
* @param aiRemotePort           IN - the remote port
*
* @return Return not 0 on any error.
*/
int SnmpSendPdu(int aiSocket, stSnmpPdu* apoPdu,char const * apcRemoteIp, int aiRemotePort);

/**
* @brief SnmpRecvPdu - recvfrom the pdu packet via PDU --- working for Client
*
* @param aiSocket		IN - the input socket identifier
* @param apoPdu 		OUT - the struct of snmp_pdu to store the result
* @param apcRemoteIp	        IN - the remote IP address
* @param aiRemotePort           IN - the remote port
*
* @return Return not 0 on any error.
*/
int SnmpRecvPdu(int aiSocket, stSnmpPdu* apoPdu,char const* apcLocalIp, int aiLocalPort);

/**
* @brief SnmpRecvPdu - recvfrom the pdu packet via PDU --- working for Agent
*
* @param aiSocket               IN - the input socket identifier
* @param apoPdu                 OUT - the struct of snmp_pdu to store the result
* @param apoCliAddr             IN - the structure to store client address
*
* @return Return not 0 on any error.
*/
int SnmpRecvPdu(int aiSocket, stSnmpPdu *apoPdu, struct sockaddr_in *apoCliAddr);

/**
* @brief SnmpSendTrap - send the trap packet via PDU
* @param aiSocket		IN - the input socket identifier
* @param apoPdu 		IN - the original data
* @param apcRemoteIp	        IN - the remote IP address
* @param aiRemotePort           IN - the remote port
*
* @return Return not 0 on any error.
*/
int SnmpSendTrap(int aiSocket, stSnmpTrap * apoPdu, char const* apcRemoteIp, int aiRemotePort);

/**
* @brief SnmpRecvTrap -recvfrom the trap packet via PDU
* @param aiSocket		IN - the input socket identifier
* @param apoPdu 		OUT - the struct of snmp_pdu to store the result
* @param apcRemoteIp	        IN - the remote IP address
* @param aiRemotePort           IN - the remote port
*
* @return Return not 0 on any error.
*/
int SnmpRecvTrap(int aiSocket,stSnmpTrap* apoPdu, char const* apcLocalIp, int aiLocalPort);

/**
* @brief Show Received Data.
* 
* @param apoPdu                 IN - the snmp_pdu structure you want to show
*
* @return 0 if success.
*/
int ShowReceivedData(stSnmpPdu * apoPdu);

#endif   //__AILIBEX__AISNMP_H__


