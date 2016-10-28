/** @file AISnmpSendV1Trap.h
*/  
 
#ifndef __AILIBEX__AISNMPSENDV1TRAP_H__
#define __AILIBEX__AISNMPSENDV1TRAP_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h> 
#include <unistd.h>
#include <arpa/inet.h>

#define AI_SOCK_ERROR_INVAL         -90101 //< invalid input arguments
#define AI_SOCK_ERROR_SOCKET        -90113 //< failed to socket()
#define AI_SOCK_ERROR_SENDTO        -90120 //< failed to sendto()

//BER type
#define AI_BER_INTEGER              0x02
#define AI_BER_OCTET_STR            0x04
#define AI_BER_NULL                 0x05
#define AI_BER_OBJECT_ID            0x06
#define AI_BER_SEQUENCE             0x30
#define AI_BER_IPADDRESS            0x40
#define AI_BER_TIMETICKS            0x43

#define AI_SNMP_VERSION_1           0            ///<snmp version 1
#define AI_SNMP_TRAP_PORT           162          ///<trap port
#define AI_SNMP_BER_TRAP            0xA4         ///<trap 

#define AI_SNMP_MAXLINE             16384        ///<buffer size
#define AI_SNMP_MAX_LEN             256      
#define AI_SNMP_MAX_VARLIST_NUM     64           ///<the number of variable-bindings
#define AI_SNMP_OID_NUM             20           ///<oid array size

//errno
#define AI_SNMP_ERRSEQ	            -1           ///<sequence build error
#define AI_SNMP_ERRSEQP             -2           ///<sequence parse error
#define AI_SNMP_ERRINT	            -3           ///<int build error
#define AI_SNMP_ERRINTP             -4           ///<int parse error
#define AI_SNMP_ERRSTR	            -5           ///<string build error
#define AI_SNMP_ERRSTRP             -6           ///<string parse error
#define AI_SNMP_ERROID	            -7           ///<OID build error 
#define AI_SNMP_ERROIDP             -8           ///<OID parse error
#define AI_SNMP_ERRUINT             -9           ///<unsigned int build error
#define AI_SNMP_ERRUINTP            -10          ///<unsigned int parse error
#define AI_SNMP_ERRVARB             -11          ///<variable_binding build error
#define AI_SNMP_ERRVARBP            -12          ///<variable_binding parse error
#define AI_SNMP_EPBUILD             -13          ///<PDU build error
#define AI_SNMP_EPPARSE             -14          ///<PDU parse error
#define AI_SNMP_ERRHEADP            -15          ///<header parse error
#define AI_SNMP_ERRADDVAR           -16          ///<add variable binding error

#define AI_SNMP_ERRTYPE             -21          ///<error variable type
#define AI_SNMP_ERRSETV             -22          ///<set variable oid error
#define AI_SNMP_ERROIDS             -23          ///<OID should be numbers or '.'
#define AI_SNMP_ERRCOV              -24          ///<cannot be convert correctly
#define AI_SNMP_EWRONGCOMMAND       -25          ///<wrong requist command
#define AI_SNMP_EPSEND              -26          ///<PDU send error
#define AI_SNMP_EPRECV              -27          ///<PDU cannot be received
#define AI_SNMP_EARRTOOID           -28          ///<convert character string to unsigned long array error

#define AI_SNMP_ESENDTO             -31          ///<sendto error	
#define AI_SNMP_EBIND               -32          ///<bind error
#define AI_SNMP_ESELECT             -33          ///<select error
#define AI_SNMP_ETIMEOUT            -34          ///<time out
#define AI_SNMP_ERECV               -35          ///<recvfrom error on socket
#define AI_SNMP_ESOCKET             -36          ///<socket error
#define AI_SNMP_EINVALSTRING        -37          ///<input is not a valid IPv4 dotted-decimal string  or a valid IPv6 address string
#define AI_SNMP_ERRPTON             -38          ///<inet_pton error 
#define AI_SNMP_ERRNTOP             -39          ///<inet_ntop error
#define AI_SNMP_ERRMAL              -40          ///<error malloc
#define AI_SNMP_EFCNTL              -41

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

typedef struct stNetTrap
{
    long            	ciVersion;	                                ///<SNMP version
    char   	        csCommunity[AI_SNMP_MAX_LEN];                   ///<Community
    int                 ciCommand; 				        ///<PDU Type
    char     		csEnterprise[AI_SNMP_MAX_LEN];		        ///<Enterprise	
    char                csAgentAddr[AI_SNMP_MAX_LEN];                   ///<agent address
    long                ciTrapType;				        ///<Generic-trap
    long                ciSpecificType;			                ///<Specific-trap
    unsigned long       ciTime;   				        ///<Time-stamp
    int                 ciVBNum;                                        ///<Number of variable-bindings
    stVariableBinding   coVariables[AI_SNMP_MAX_VARLIST_NUM];	        ///<Variable-binding
}stSnmpTrap;

/**
* @brief SnmpSendV1Trap - send the trap packet via PDU
*
* @param apoPdu 		IN - the original data
* @param apcRemoteIp	        IN - the remote IP address
* @param aiRemotePort           IN - the remote port
*
* @return Return not 0 on any error.
*/
int SnmpSendV1Trap(stSnmpTrap * apoPdu, char const* apcRemoteIp, int aiRemotePort);

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
 * @brief SnmpAddVar - add variable binding
 *
 * @param apoTrapPdu IN/OUT - trap pdu
 * @param apcOidName IN - the oid name
 * @param acType     IN - value type(i:INTEGER,s:OCTET String,o:oid,t:timeticks,n:NULL)
 * @param apcValue   IN - the value of variable binding
 *
 * @return not 0 on any error.
 */ 
int SnmpAddVar(stSnmpTrap *apoTrapPdu, char *apcOidName, unsigned char acType, char *apcValue);

/**
 * @brief SnmpAddVarInt - add INTEGER type variable binding
 *
 * @param apoTrapPdu IN/OUT - trap pdu
 * @param apcOidName IN - the oid name
 * @param apcValue   IN - the value variable binding
 *
 * @return not 0 on any error.
 */ 
int SnmpAddVarInt(stSnmpTrap *apoTrapPdu, char *apcOidName, char *apcValue);

/**
 * @brief SnmpAddVarStr - add OCTET String type variable binding 
 *
 * @param apoTrapPdu IN/OUT - trap pdu
 * @param apcOidName IN - the oid name
 * @param apcValue   IN - the value variable binding
 *
 * @return not 0 on any error.
 */ 
int SnmpAddVarStr(stSnmpTrap *apoTrapPdu, char *apcOidName, char *apcValue);

/**
 * @brief SnmpAddVarOid - add OID type variable binding
 *
 * @param apoTrapPdu IN/OUT - trap pdu
 * @param apcOidName IN - the oid name
 * @param apcValue   IN - the value variable binding
 *
 * @return not 0 on any error.
 */ 
int SnmpAddVarOid(stSnmpTrap *apoTrapPdu, char *apcOidName, char *apcValue);

/**
 * @brief SnmpAddVarTimeStamp - add timeticks type variable binding
 *
 * @param apoTrapPdu IN/OUT - trap pdu
 * @param apcOidName IN - the oid name
 * @param apcValue   IN - the value variable binding
 *
 * @return not 0 on any error.
 */ 
int SnmpAddVarTimeStamp(stSnmpTrap *apoTrapPdu, char *apcOidName, char *apcValue);

/**
 * @brief snmpaddVarNULL - add null type variable binding
 *
 * @param apotrappdu in/out - trap pdu
 * @param apcoidname in - the oid name
 * @param apcvalue   in - the value variable binding
 *
 * @return not 0 on any error.
 */ 
int SnmpAddVarNULL(stSnmpTrap *apoTrapPdu, char *apcOidName, char *apcValue);

/**
 * @brief snmpaddVarIPAddress - add ip address type variable binding
 *
 * @param apotrappdu in/out - trap pdu
 * @param apcoidname in - the oid name
 * @param apcvalue   in - the value variable binding
 *
 * @return not 0 on any error.
 */ 
int SnmpAddVarIPAddress(stSnmpTrap *apoTrapPdu, char *apcOidName, char *apcValue);

/**
* @brief Build an BER header for a length with length specified.
*
* @param apcBuffer          IN - pointer to start of object
* @param apiMaxSize         IN/OUT - number of valid bytes left in buffer
* @param aiValueLen	    IN - length of object
*
* @return Return a pointer to the first byte of the contents of this object.
*         Returns NULL on any error.
*/
unsigned char *BerBuildLength(unsigned char * apcBuffer, size_t * apiMaxSize,size_t aiValueLen);

/**
* @brief Build an BER header for a sequence with the ID and length specified.
* 
* @param apcBuffer         IN - pointer to start of object
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param acType            IN - ber type of object
* @param aiValueLen        IN - length of object
*
* @return Returns a pointer to the first byte of the contents of this object.
*         Returns NULL on any error.
*/
unsigned char * BerBuildSequence(unsigned char * apcBuffer,size_t * apiMaxSize, unsigned char acType, size_t aiValueLen);

/**
* @brief builds an BER header for an object with the ID and length specified.
* 
* @param apcBuffer        IN - pointer to start of object
* @param apiMaxSize       IN/OUT - number of valid bytes left in buffer
* @param acType           IN - ber type of object
* @param aiValueLen       IN - length of object
*
* @return Returns a pointer to the first byte of the contents of this object.
          Returns NULL on any error.
*/
unsigned char *BerBuildHeader(unsigned char * apcBuffer,size_t * apiMaxSize, unsigned char acType, size_t aiValueLen);

/**
* @brief  Build an BER object containing an integer.
*
* @param apcBuffer        IN - pointer to start of output buffer
* @param apiMaxSize       IN/OUT - number of valid bytes left in buffer
* @param acType           IN  - ber type of objec
* @param apiValue         IN - pointer to start of long integer
* @param aiValueLen       IN - size of input buffer
* 
* @return  Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
*          Returns NULL on any error.
*/
unsigned char *BerBuildInt(unsigned char * apcBuffer, size_t * apiMaxSize, const unsigned char acType , const long *apiValue, size_t aiValueLen);

/**
* @brief Build an BER octet string object containing the input string.
*
* @param apcBuffer         IN - pointer to start of object
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param acType            IN - ber type of object
* @param apcValue          IN - pointer to start of input buffer
* @param aiValueLen        IN - size of input buffer
*
* @return  Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
           Returns NULL on any error.
*/
unsigned char * BerBuildString(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned char acType, const char * apcValue, size_t aiValueLen);

/**
* @brief Build an BER object identifier object containing the input string.
*
* @param apcBuffer         IN - pointer to start of object
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param acType            IN - ber type of object
* @param apiValue          IN - pointer to start of input buffer
* @param aiValueLen        IN - number of sub-id's in objid
*
* @return   Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
            Returns NULL on any error.
*/
unsigned  char *BerBuildObjid(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned char acType, unsigned long *apiValue, size_t aiValueLen) ;

/**
* @brief Build an BER object containing an integer.
*
* @param apcBuffer         IN - pointer to start of output buffer
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param acType            IN  - ber type of objec
* @param apiValue          IN - pointer to start of long integer
* @param aiValueLen        IN - size of input buffer
* 
* @return  Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
           Returns NULL on any error.
*/
unsigned char *BerBuildUnsignedInt(unsigned char  * apcBuffer , size_t  * apiMaxSize,unsigned char acType, const unsigned long  * apiValue , int aiValueLen);

/**
* @brief Build an BER null object.
*
* @param apcBuffer         IN - pointer to start of object
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param acType            IN - ber type of object
*
* @retun  Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
*         Returns NULL on any error.
*/
unsigned char *BerBuildNull(unsigned char * apcBuffer ,size_t * apiMaxSize, unsigned char acType);

#endif    //__AILIBEX__AISNMPSENDV1TRAP_H__
