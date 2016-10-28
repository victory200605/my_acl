
/**
 * @file  SNMPMessage.h
 */

#ifndef SNMP_MESSAGE_H
#define SNMP_MESSAGE_H

#include "acl/Utility.h"
#include "acl/MemoryBlock.h"
#include "anf/Utility.h"

#include "acl/stl/string.h"

ANF_NAMESPACE_START

namespace snmp
{
// BER type
#define ANF_BER_INTEGER                  0x02
#define ANF_BER_OCTET_STR                0x04
#define ANF_BER_NULL                     0x05
#define ANF_BER_OBJECT_ID                0x06
#define ANF_BER_SEQUENCE                 0x30
#define ANF_BER_IPADDRESS                0x40
#define ANF_BER_COUNTER                  0x41
#define ANF_BER_GAUGE                    0x42
#define ANF_BER_TIMETICKS                0x43

// command type
#define ANF_SNMP_BER_GET                 0xA0             ///<get request
#define ANF_SNMP_BER_GETNEXT             0xA1             ///<getnext request
#define ANF_SNMP_BER_GETRESP             0xA2             ///<get response 
#define ANF_SNMP_BER_SET                 0xA3             ///<set request
#define ANF_SNMP_BER_TRAP                0xA4             ///<trap 

// error-status type
#define ANF_SNMP_ERRSTAT_NOERR           0                ///<noError
#define ANF_SNMP_ERRSTAT_TOOBIG          1                ///<tooBig
#define ANF_SNMP_ERRSTAT_NOSUCHNAME      2                ///<noSuchName
#define ANF_SNMP_ERRSTAT_BADVAL          3                ///<badValue
#define ANF_SNMP_ERRSTAT_READONLY        4                ///<readOnly
#define ANF_SNMP_ERRSTAT_GENERR          5                ///<general error

#define ANF_SNMP_OID_NAME_LEN            256
#define ANF_SNMP_MAX_LEN                 256
#define ANF_SNMP_PDU_MAX_VBS             50
#define ANF_SNMP_OID_NUM                 20               ///<oid array size
#define ANF_SNMP_MAX_VARLIST_NUM         64               ///<the number of variable-bindings
#define ANF_SNMP_VERSION_1               0                ///<snmp version 1

// errno
#define ANF_SNMP_ERRSEQ                  -1               ///<sequence build error
#define ANF_SNMP_ERRSEQP                 -2               ///<sequence parse error
#define ANF_SNMP_ERRINT                  -3               ///<int build error
#define ANF_SNMP_ERRINTP                 -4               ///<int parse error
#define ANF_SNMP_ERRSTR                  -5               ///<string build error
#define ANF_SNMP_ERRSTRP                 -6               ///<string parse error
#define ANF_SNMP_ERROID                  -7               ///<OID build error 
#define ANF_SNMP_ERROIDP                 -8               ///<OID parse error
#define ANF_SNMP_ERRUINT                 -9               ///<unsigned int build error
#define ANF_SNMP_ERRUINTP                -10              ///<unsigned int parse error
#define ANF_SNMP_ERRVARB                 -11              ///<variable_binding build error
#define ANF_SNMP_ERRVARBP                -12              ///<variable_binding parse error
#define ANF_SNMP_EPBUILD                 -13              ///<PDU build error
#define ANF_SNMP_EPPARSE                 -14              ///<PDU parse error
#define ANF_SNMP_ERRHEADP                -15              ///<header parse error
#define ANF_SNMP_ERRADDVAR               -16              ///<add variable binding error

#define ANF_SNMP_ERRTYPE                 -21              ///<error variable type
#define ANF_SNMP_ERRSETV                 -22              ///<set variable oid error
#define ANF_SNMP_ERROIDS                 -23              ///<OID should be numbers or '.'
#define ANF_SNMP_ERRCOV                  -24              ///<cannot be convert correctly
#define ANF_SNMP_EWRONGCOMMAND           -25              ///<wrong requist command
#define ANF_SNMP_EPSEND                  -26              ///<PDU send error

#define ANF_SNMP_ERRGETAGENTADDR         -50              ///<Get AgentAddr error
#define ANF_SNMP_ERRGETENTERPRISE        -51              ///<Get Enterprise error
#define ANF_SNMP_ERRGETREADCOMMUNITY     -52              ///<Get ReadCommunity error

#define ANF_BER_LONG_LEN                 0x80 
#define ANF_BER_EXTENSION_ID             0x1F
#define ANF_IS_EXTENSION_ID(byte)        (((byte) & ANF_BER_EXTENSION_ID) == ANF_BER_EXTENSION_ID)

/***************************************************************
                       CVarBind
***************************************************************/
/**
 * @class CVarBind
 *
 * @brief The variable-binding class
 */
class CVarBind
{
public:
    /**
     * @brief  A constructor.
     */
    CVarBind(void);

    /**
     * @brief  A copy constructor.
     */
    CVarBind(const CVarBind& aoVb);

    /**
     * @brief  A destructor.
     */
    ~CVarBind(void);

    /**
     * @brief  Get the vb oid name.
     *
     * @return The vb oid name.
     */
    const char* GetOidName();

    /**
     * @brief  Set the vb oid name.
     *
     * @param[IN]  apcOidName - the vb oid name to set
     */
    void SetOidName(const char* apcOidName);

    /**
     * @brief  Get the vb value type.
     *
     * @return The vb value type.
     */
    apl_int_t GetValueType(void);

    /**
     * @brief  Set the vb value type.
     *
     * @param[IN]  acType - the vb value type to set
     */
    void SetValueType(unsigned char acType);

    /**
     * @brief  Get the vb int value.
     *
     * @param[OUT]  ai32Val - store the int value
     */
    void GetValue(apl_int32_t& ai32Val);

    /**
     * @brief  Get the vb string value.
     *
     * @param[IN]  apcVal - store the string value
     */
    void GetValue(char* apcVal);

    /**
     * @brief  Set the vb value.
     *
     * @param[IN]  apcVal - the value to set
     * @param[IN]  acType - the value type
     *
     * @retval 0  Success. 
     * @retval -1 Failed. 
     */
    apl_int_t SetValue(char* apcVal, unsigned char acType);

public:
    char macOidName[ANF_SNMP_OID_NAME_LEN];

    unsigned char mcType;

    apl_int_t miValLen;

    union
    {
        apl_int32_t  mi32Integer;                   
        char         macString[ANF_SNMP_MAX_LEN];   
        char         macObjid[ANF_SNMP_MAX_LEN];    
    }moVal;

    bool mbIsObjid;         //distinguish objid and string
};

/***************************************************************
                       CSNMPMessage 
***************************************************************/
/**
 * @class CSNMPMessage 
 *
 * @brief The pdu message parent class.
 */
class CSNMPMessage 
{
public:
    /**
     * @brief  A Constructor.
     */
    CSNMPMessage(void);

    /**
     * @brief  A Constructor.
     *
     * @param[IN]  apoVbs - array of pointers to Vb objects
     * @param[IN]  aiVbCount - length of the array
     */
    CSNMPMessage(CVarBind* apoVbs, apl_int_t aiVbCount);

    /**
     * @brief  A copy constructor.
     *
     * @param[IN]  aoPdu - other CSNMPMessage object
     */
    CSNMPMessage(const CSNMPMessage& aoPdu);

    /**
     * @brief  A destructor.
     */
    virtual ~CSNMPMessage(void);
    
    /**
     * @brief  Get Pointers to all Vbs from Pdu.
     *
     * @param[OUT]  apoVbs - array of empty pointers of size aiVbCount
     * @param[IN]   aiVbCount - amount of Vb pointers to get
     *
     * @retval 0  Success.
     * @retval -1 Failed 
     */
    apl_int_t GetVbList(CVarBind* apoVbs, apl_int_t aiVbCount);

    /**
     * @brief  Deposit all Vbs to Pdu.
     *
     * @param[IN]   apoVbs - the variable-bindings to deposit 
     * @param[IN]   aiVbCount - how many Vbs to deposit
     *
     * @retval 0  Success.
     * @retval -1 Failed.
     */
    apl_int_t SetVbList(CVarBind* apoVbs, apl_int_t aiVbCount);

    /**
     * @brief  Get a particular vb.
     *
     * @param[OUT]  aoVb - the CVarBind object to store the vb
     * @param[IN]   aiIdx - the number of Vb to get(0 is the first one)
     *
     * @retval 0  Success.
     * @retval -1 Failed 
     */
    apl_int_t GetVb(CVarBind& aoVb, apl_int_t aiIdx);

    /**
     * @brief  Set a particular vb.
     *
     * @param[IN]  aoVb - the CVarBind object to set to Pdu.
     * @param[IN]  aiIdx - the number of Vb to set(0 is the first one)
     *
     * @retval 0  Success.
     * @retval -1 Failed 
     */
    apl_int_t SetVb(CVarBind& aoVb, apl_int_t aiIdx);

    /**
     * @brief  Get the Vb count.
     *
     * @return The number of Vb objects within the pdu.
     */
    apl_int_t GetVbCount(void);

    /**
     * @brief  Get the message version.
     *
     * @return The message version.
     */
    apl_int32_t GetVersion(void);

    /**
     * @brief  Set the message version.
     *
     * @param[IN]  ai32Version - the message version to set
     */
    void SetVersion(apl_int32_t ai32Version);

    /**
     * @brief  Get the read community.
     *
     * @return The read community.
     */
    const char* GetReadCommunity(void);

    /**
     * @brief  Set the read community.
     *
     * @param[IN]  apcReadCommunity - the read community to set
     */
    void SetReadCommunity(const char* apcReadCommunity);

    /**
     * @brief  Get message type.
     *
     * @return The message type.
     */
    apl_int_t GetType(void);

    /**
     * @brief  Set the message type.
     *
     * @param[IN]  aiType - the message type to set.
     */
    void SetType(apl_int_t aiType);

    /**
     * @brief  Append a vb to the pdu.
     *
     * @param[IN]  aoVb - The Vb that should be added (as last vb) to the pdu
     *
     * @return The new pdu message.
     */
    CSNMPMessage& operator+=(CVarBind& aoVb);

    /**
     * @brief  Get the message type in std::string.
     *
     * @return The message type in std::string.
     */
    virtual std::string GetMessageType();

protected:
    apl_int32_t mi32Version;

    char        macReadCommunity[64];

    apl_int_t   miType;

    apl_int_t   miVbCount;

    CVarBind*   mpoVbs[ANF_SNMP_PDU_MAX_VBS];

    bool        mbIsValidity;

    std::string moStrType;
};

/***************************************************************
                       CNormalMessage 
***************************************************************/
/**
 * @class CNormalMessage
 * 
 * @brief Normal pdu message class, such as Get/GetNext/Set/Response.
 */
class CNormalMessage : public CSNMPMessage
{
public:
    /**
     * @brief  A constructor.
     */
    CNormalMessage(void);

    /**
     * @brief  A constructor.
     *
     * @param[IN]  apoVbs - array of pointers to Vb objects
     * @param[IN]  aiVbCount - length of the array
     */
    CNormalMessage(CVarBind* apoVbs, apl_int_t aiVbCount);

    /**
     * @brief  A destructor.
     */
    ~CNormalMessage(void);

    /**
     * @brief  Get the requestId.
     *
     * @return The requestId.
     */
    apl_int32_t GetRequestId(void);

    /**
     * @brief  Set the requestId.
     *
     * @param[IN]  ai32RequestId - The requestId to set.
     */
    void SetRequestId(apl_int32_t ai32RequestId);

    /**
     * @brief  Get the error-status.
     *
     * @return The error-status.
     */
    apl_int32_t GetErrStat(void);

    /**
     * @brief  Set the error-status.
     *
     * @param[IN] ai32ErrStat - The error-status to set. 
     */
    void SetErrStat(apl_int32_t ai32ErrStat);

    /**
     * @brief  Get the error-index.
     *
     * @return The error-index.
     */
    apl_int_t GetErrIndex(void);

    /**
     * @brief  Set the error-index.
     *
     * @param[IN] ai32ErrIndex - The error-index to set. 
     */
    void SetErrIndex(apl_int32_t ai32ErrIndex);

    /**
     * @brief   Overloaded assignment operator.
     *
     * @param[IN]  aoNormalPdu - pdu that should be assigned to this object
     *
     * @return The normal pdu message.
     */
    CNormalMessage& operator=(CNormalMessage& aoNormalPdu);

private:
    apl_int32_t  mi32RequestId;
    
    apl_int32_t  mi32ErrStat;

    apl_int32_t  mi32ErrIndex;
};

/***************************************************************
                       CTrapMessage
***************************************************************/
/**
 * @class CTrapMessage
 *
 * @brief The trap pdu class.
 */
class CTrapMessage : public CSNMPMessage
{
public:
    /**
     * @brief  A constructor.
     */
    CTrapMessage(void);

    /**
     * @brief  A constructor.
     *
     * @param[IN]  apoVbs - array of pointers to Vb objects
     * @param[IN]  aiVbCount - length of the array
     */
    CTrapMessage(CVarBind* apoVbs, apl_int_t aiVbCount);

    /**
     * @brief  A destructor.
     */
    ~CTrapMessage(void);

    /**
     * @brief  Get the enterprise.
     *
     * @return The enterprise.
     */
    const char* GetEnterprise(void);

    /**
     * @brief  Set the enterprise.
     *
     * @param[IN]  apcEnterprise - the enterprise to set
     */
    void SetEnterprise(const char* apcEnterprise);

    /**
     * @brief  Get the agent address.
     *
     * @return The agent address.
     */
    const char* GetAgentAddr(void);

    /**
     * @brief  Set the agent address.
     *
     * @param[IN] apcAgentAddr - the agent address to set 
     */
    void SetAgentAddr(const char* apcAgentAddr);

    /**
     * @brief  Get the trap-type
     *
     * @return The trap-type
     */
    apl_int32_t GetTrapType(void);

    /**
     * @brief  Set the trap-type
     *
     * @param[IN]  ai32TrapType - the trap-type to set
     */
    void SetTrapType(apl_int32_t ai32TrapType);

    /**
     * @brief  Get the specific-type.
     *
     * @return The specific-type.
     */
    apl_int32_t GetSpecificType(void);

    /**
     * @brief  Set the specific-type.
     *
     * @param[IN]  ai32SpecificType - the specific-type to set
     */
    void SetSpecificType(apl_int32_t ai32SpecificType);

    /**
     * @brief  Get the timestamp.
     *
     * @return The timestamp.
     */
    apl_uint32_t GetTimestamp(void);

    /**
     * @brief  Set the timestamp.
     *
     * @param[IN]  au32Time - the timestamp to set
     */
    void SetTimestamp(apl_uint32_t au32Time);

private:
    char        macEnterprise[ANF_SNMP_OID_NAME_LEN];

    char        macAgentAddr[20];

    apl_int32_t mi32TrapType;

    apl_int32_t mi32SpecificType;

    apl_uint32_t mu64Time;
};

/***************************************************************
                       CSNMPEncoder
***************************************************************/
/**
 * @class CSNMPEncoder
 *
 * @brief Pdu message encoder class.
 */
class CSNMPEncoder 
{
public:
    /**
     * @brief  A constructor.
     *
     * @param[OUT]  apoBuffer - the memory block to store the encode message
     */
    CSNMPEncoder(acl::CMemoryBlock* apoBuffer);

    /**
     * @brief  A destructor.
     */
    ~CSNMPEncoder(void);

    acl::CMemoryBlock* GetBuffer() { return this->mpoBuffer; }

    /**
     * @brief  Encode message.
     *
     * @param[IN]  apoMsg - the pdu message to encode.
     *
     * @return The encode buffer size. Otherwise, a negative number shall be returned to indicate the error.
     */
    apl_int_t EncodeMessage(CSNMPMessage* apoMsg);

private:
    /**
     * @brief  Normal pdu message encoder.
     *
     * @param[IN]  apoMsg - the normal pdu message object
     * @param[IN]  apcBuffer - the buffer to store the encoder message
     * @param[IN]  auMaxSize - the buffer size
     *
     * @return The encode buffer size. Otherwise, a negative number shall be returned to indicate the error.
     */
    apl_int_t NormalPduBuild(
        CNormalMessage* apoMsg, 
        unsigned char*  apcBuffer, 
        apl_size_t      auMaxSize);

    /**
     * @brief  Trap pdu message encoder.
     *
     * @param[IN]  apoMsg - the normal pdu message object
     * @param[IN]  apcBuffer - the buffer to store the encoder message
     * @param[IN]  auMaxSize - the buffer size
     *
     * @return The encode buffer size. Otherwise, a negative number shall be returned to indicate the error.
     */
    apl_int_t TrapPduBuild(
        CTrapMessage*  apoMsg, 
        unsigned char* apcBuffer, 
        apl_size_t     auMaxSize);

    /**
     * @brief  Variable-binding build.
     *
     * @param[OUT]      apcBuffer - the output buffer to store the encode pdu message
     * @param[IN/OUT]   apuBufSize - the left buffer size
     * @param[IN]       apu32OidName - the oid name
     * @param[IN]       aiOidNameLen - the oid name length
     * @param[IN]       acVarValType - the vb value type 
     * @param[IN]       apcVal - the value 
     * @param[IN]       aiValLen - the value length 
     *
     * @return The encode pdu message buffer.
     */
    unsigned char* VarBindBuild(
        unsigned char* apcBuffer,
        apl_size_t*    apuBufSize,
        apl_uint32_t*  apu32OidName,
        apl_int_t      aiOidNameLen,
        unsigned char  acVarValType,
        char*          apcVal,
        apl_int_t      aiValLen);

    /**
     * @brief  Change string address to array 
     *
     * @param[IN]  apcSrc - the orignal string address
     * @param[OUT] apcDes - the output array 
     * @param[IN]  aiSize - the array size
     *
     * @return The address length. Otherwise,-1 shall be returned.
     */
    apl_int_t CutAddr(char* apcSrc, char* apcDes, apl_int_t aiSize);

    /**
     * @brief  Change oid string to integer array.
     *
     * @param[IN]   apcName - the oid string name
     * @param[OUT]  apu32Oid - the output integer array
     * @param[IN]   auSize - the maximum oid length
     *
     * @return The oid length. Otherwise, an negative shall be returned.
     */
    apl_int_t ArrToOid(char const* apcName, apl_uint32_t* apu32Oid, apl_size_t auSize);

private:
    /**
     * @brief Build an BER header for a length with length specified.
     *
     * @param[IN]     apcBuffer          - pointer to start of object
     * @param[IN/OUT] apuMaxSize         - number of valid bytes left in buffer
     * @param[IN]     auValueLen         - length of object
     *
     * @return Return a pointer to the first byte of the contents of this object.
     *         Returns NULL on any error.
     */
    unsigned char* BerBuildLength(
        unsigned char* apcBuffer, 
        apl_size_t*    apuMaxSize, 
        apl_size_t     auValueLen);

    /**
     * @brief Build an BER header for a sequence with the ID and length specified.
     * 
     * @param[IN]     apcBuffer         - pointer to start of object
     * @param[IN/OUT] apuMaxSize        - number of valid bytes left in buffer
     * @param[IN]     acType            - ber type of object
     * @param[IN]     auValueLen        - length of object
     *
     * @return Returns a pointer to the first byte of the contents of this object.
     *         Returns NULL on any error.
     */
    unsigned char* BerBuildSequence(
        unsigned char* apcBuffer,
        apl_size_t*    apuMaxSize, 
        unsigned char  acType, 
        apl_size_t     auValueLen);

    /**
     * @brief builds an BER header for an object with the ID and length specified.
     * 
     * @param[IN]     apcBuffer        - pointer to start of object
     * @param[IN/OUT] apuMaxSize       - number of valid bytes left in buffer
     * @param[IN]     acType           - ber type of object
     * @param[IN]     auValueLen       - length of object
     *
     * @return Returns a pointer to the first byte of the contents of this object.
     Returns NULL on any error.
     */
    unsigned char* BerBuildHeader(
        unsigned char* apcBuffer,
        apl_size_t*    apuMaxSize, 
        unsigned char  acType, 
        apl_size_t     auValueLen);

    /**
     * @brief  Build an BER object containing an integer.
     *
     * @param[IN]     apcBuffer        - pointer to start of output buffer
     * @param[IN/OUT] apuMaxSize       - number of valid bytes left in buffer
     * @param[IN]     acType           - ber type of objec
     * @param[IN]     api32Value       - pointer to start of apl_int32_t integer
     * @param[IN]     auValueLen       - size of input buffer
     * 
     * @return  Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
     *          Returns NULL on any error.
     */
    unsigned char* BerBuildInt(
        unsigned char* apcBuffer, 
        apl_size_t*    apuMaxSize, 
        unsigned char  acType, 
        apl_int32_t*   api32Value, 
        apl_size_t     auValueLen);

    /**
     * @brief Build an BER octet string object containing the input string.
     *
     * @param[IN]     apcBuffer         - pointer to start of object
     * @param[IN/OUT] apuMaxSize        - number of valid bytes left in buffer
     * @param[IN]     acType            - ber type of object
     * @param[IN]     apcValue          - pointer to start of input buffer
     * @param[IN]     auValueLen        - size of input buffer
     *
     * @return  Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
     Returns NULL on any error.
     */
    unsigned char* BerBuildString(
        unsigned char* apcBuffer,
        apl_size_t*    apuMaxSize,
        unsigned char  acType, 
        const char*    apcValue, 
        apl_size_t     auValueLen);

    /**
     * @brief Build an BER object identifier object containing the input string.
     *
     * @param[IN]     apcBuffer         - pointer to start of object
     * @param[IN/OUT] apuMaxSize        - number of valid bytes left in buffer
     * @param[IN]     acType            - ber type of object
     * @param[IN]     api32Value        - pointer to start of input buffer
     * @param[IN]     auValueLen        - number of sub-id's in objid
     *
     * @return   Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
     Returns NULL on any error.
     */
    unsigned char* BerBuildObjid(
        unsigned char* apcBuffer,
        apl_size_t*    apuMaxSize,
        unsigned char  acType, 
        apl_uint32_t*  apu32Value, 
        apl_size_t     auValueLen) ;

    /**
     * @brief Build an BER object containing an integer.
     *
     * @param[IN]     apcBuffer         - pointer to start of output buffer
     * @param[IN/OUT] apiMaxSize        - number of valid bytes left in buffer
     * @param[IN]     acType            - ber type of objec
     * @param[IN]     apu32Value        - pointer to start of apl_int32_t integer
     * @param[IN]     aiValueLen        - size of input buffer
     * 
     * @return  Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
     Returns NULL on any error.
     */
    unsigned char* BerBuildUnsignedInt(
        unsigned char* apcBuffer, 
        apl_size_t*    apuMaxSize,
        unsigned char  acType, 
        apl_uint32_t*  apu32Value, 
        apl_int_t      aiValueLen);

    /**
     * @brief Build an BER null object.
     *
     * @param[IN]     apcBuffer         - pointer to start of object
     * @param[IN/OUT] apiMaxSize        - number of valid bytes left in buffer
     * @param[IN]     acType            - ber type of object
     *
     * @retun  Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
     *         Returns NULL on any error.
     */
    unsigned char* BerBuildNull(
        unsigned char* apcBuffer,
        apl_size_t*    apuMaxSize, 
        unsigned char  acType);

private:
    acl::CMemoryBlock*  mpoBuffer;
};

/***************************************************************
                       CSNMPDecoder 
***************************************************************/
/**
 * @class CSNMPDecoder
 * 
 * @brief Pdu message decoder class.
 */
class CSNMPDecoder 
{
public:
    /**
     * @brief  A constructor.
     *
     * @param[IN]  apoBuffer - the memory block to put message
     */
    CSNMPDecoder(acl::CMemoryBlock* apoBuffer);

    /**
     * @brief  A destructor.
     */
    ~CSNMPDecoder(void);

    acl::CMemoryBlock* GetBuffer() { return this->mpoBuffer; }

    /**
     * @brief  Decode message.
     *
     * @param[IN]  appoMsg - the pointer to the object to store the decode message.
     *
     * @retval 0  Success.
     * @retval -1 Failed.
     */
    apl_int_t DecodeMessage(CSNMPMessage** appoMsg);

private:
    /**
     * @brief  Normal pdu message decoder.
     *
     * @param[OUT]  apoMsg - the object to store the decode pdu message
     * @param[IN]   apcBffer - the input encode pdu message buffer
     * @param[IN]   auMaxSize - the buffer length
     *
     * @retval 0  Success.
     * @retval <0 Failed.
     */
    apl_int_t NormalPduParse(
        CNormalMessage* apoMsg, 
        unsigned char*  apcBuffer, 
        apl_size_t      auMaxSize);

    /**
     * @brief  Trap pdu message decoder.
     *
     * @param[OUT]  apoMsg - the object to store the decode pdu message
     * @param[IN]   apcBffer - the input encode pdu message buffer
     * @param[IN]   auMaxSize - the buffer length
     *
     * @retval 0  Success.
     * @retval <0 Failed.
     */
    apl_int_t TrapPduParse(
        CTrapMessage*  apoMsg, 
        unsigned char* apcBuffer, 
        apl_size_t     auMaxSize);

    /**
     * @brief  Just get the pdu type for decoder.
     *
     * @param[IN]  apcBuffer - the input encode pdu message buffer
     * @param[IN]  auMaxSize - the buffer length
     *
     * @return Upon successfully complete, pdu type shall be returned.Otherwise, <0 shall be returned.
     */
    apl_int_t GetPduType(unsigned char* apcBuffer, apl_size_t auMaxSize);

    /**
     * @brief  Variable-binding parse.
     *
     * @param[IN]       apcBuffer - the input encode pdu message buffer
     * @param[IN/OUT]   apuBufSize - the left buffer size
     * @param[OUT]      apu32OidName - the oid name
     * @param[OUT]      apiOidNameLen - the oid name length
     * @param[OUT]      apcVarValType - the vb value type
     * @param[OUT]      appcVal - the vb value
     * @param[OUT]      apiValLen - the vb value length
     *
     * @return The orignal input encode pdu message buffer.Otherwise, NULL shall be returned.
     */
    unsigned char* VarBindParse(
        unsigned char*  apcBuffer,
        apl_size_t*     apuBufSize,
        apl_uint32_t*   apu32OidName,
        apl_int_t*      apiOidNameLen,
        unsigned char*  apcVarValType,
        unsigned char** appcVal,
        apl_int_t*      apiValLen);

    /**
     * @brief  Set oid name to CVarBind object.
     *
     * @param[IN/OUT]  apoVb - the object to store array oid name
     * @param[IN]      apu32OidName - the array oid name
     * @param[IN]      aiOidNameLen - the oid name length
     *
     * @retval 0  Success. 
     * @retval <0 Failed. 
     */
    apl_int_t SetVarObjid(CVarBind* apoVb, apl_uint32_t* apu32Objid, apl_int_t aiOidNameLen);

private:
    /**
     * @brief Interpret the length of the current object.
     *
     * @param[IN]  apcBuffer         - pointer to start of length field
     * @param[OUT] apuValueLen       - value of length field
     *
     * @return Returns a pointer to the first byte after this length field.
     Returns NULL on any error.
     */
    unsigned char* BerParseLength(
        unsigned char* apcBuffer, 
        apl_size_t*    apuValueLen);

    /**
     * @brief  Interpret the ID and length of the current object.
     *
     * @param[IN]     apcBuffer         - pointer to start of object
     * @param[IN/OUT] apuMaxSize        - number of valid bytes left in buffer
     * @param[OUT]    apcType           - ber type of object
     *
     * @return  Returns a pointer to the first byte of the contents of this object.
     Returns NULL on any error.
     */
    unsigned char* BerParseHeader(
        unsigned char* apcBuffer, 
        apl_size_t*    apuMaxSize, 
        unsigned char* apcType);

    /**
     * @brief Interpret the ID and length of the current object.
     *
     * @param[IN]     apcBuffer          - pointer to start of object
     * @param[IN/OUT] apuMaxSize         - number of valid bytes left in buffer
     * @param[OUT]    apcType            - ber type of object
     * @paramp[IN]    acExpectedType     - expected type
     *
     * @return  Returns a pointer to the first byte of the contents of this object.
     Returns NULL on any error.
     */
    unsigned char* BerParseSequence(
        unsigned char* apcBuffer, 
        apl_size_t*    apuMaxSize, 
        unsigned char* apcType, 
        unsigned char  acExpectedType);

    /**
     * @brief Pull an object indentifier out of an BER object identifier type.
     *
     * @param[IN]     apcBuffer         - pointer to start of object
     * @param[IN/OUT] apuMaxSize        - number of valid bytes left in buffer
     * @param[OUT]    apcType           - ber type of object
     * @param[IN/OUT] apu32Objid        - pointer to start of output buffer
     * @param[IN/OUT] apiObjidLen       - number of sub-id's in objid
     *
     *  @return Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
     Returns NULL on any error.
     */
    unsigned char* BerParseObjid(
        unsigned char* apcBuffer,
        apl_size_t*    apuMaxSize,
        unsigned char* apcType, 
        apl_uint32_t*  apu32Objid, 
        apl_int_t*     apiObjidLen);

    /**
     * @brief Pull a apl_int32_t out of an int type.
     *
     * @param[IN]     apcBuffer       - pointer to start of object
     * @param[IN/OUT] apuMaxSize      - number of valid bytes left in buffer
     * @param[OUT]    apcType         - ber type of object
     * @param[IN/OUT] api32Value      - pointer to start of output buffer
     * @param[IN]     auValueLen      - size of output buffer
     * 
     * @return Returns pointer to the first byte past the end of this object (i.e. the start of the next object).
     Returns NULL on any error.
     */
    unsigned char* BerParseInt(
        unsigned char* apcBuffer,
        apl_size_t*    apuMaxSize,
        unsigned char* apcType, 
        apl_int32_t*   api32Value, 
        apl_size_t     auValueLen);

    /**
     * @brief Pull an octet string out of an BER octet string type.
     *
     * @param[IN]     apcBUffer        - pointer to start of object
     * @param[IN/OUT] apuMaxSize       - number of valid bytes left in buffer
     * @param[OUT]    acType           - ber type of object 
     * @param[IN/OUT] apcValue         - pointer to start of output buffer
     * @param[IN]     auValueLen       - size of output buffer
     * 
     * @return  Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
     Returns NULL on any error.
     */
    unsigned char* BerParseString(
        unsigned char* apcBuffer, 
        apl_size_t*    apuMaxSize, 
        unsigned char* apcType, 
        char*          apcValue, 
        apl_size_t     auValueLen);

    /**
     * @brief Interpret an BER null type.
     *
     * @param[IN]     apcBuffer         - pointer to start of object
     * @param[IN/OUT] apuMaxSize        - number of valid bytes left in buffer
     * @param[OUT]    acType            - ber type of object
     *
     *  @return Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
     Returns NULL on any error.
     */
    unsigned char* BerParseNull(
        unsigned char* apcBuffer, 
        apl_size_t*    apuMaxSize, 
        unsigned char* apcType);

    /**
     * @brief Pull an unsigned int out of an BER int type.
     *
     * @param[IN]     apcBuffer       - pointer to start of object
     * @param[IN/OUT] apuMaxSize      - number of valid bytes left in buffer
     * @param[OUT]    acType          - ber type of object
     * @param[IN/OUT] apu32Value      - pointer to start of output buffer
     * @param[IN]     auValueLen      - size of output buffer
     * 
     * @return Returns pointer to the first byte past the end of this object (i.e. the start of the next object).
     Returns NULL on any error
     */
    unsigned char* BerParseUnsignedInt(
        unsigned char* apcBuffer,
        apl_size_t*    apuMaxSize,
        unsigned char* apcType, 
        apl_uint32_t*  apu32Value, 
        apl_size_t     auValueLen);

    /**
     * @brief Pull agent address out of encoder buffer.
     *
     * @param[IN]     apcBuffer       - pointer to start of object
     * @param[IN/OUT] apuMaxSize      - number of valid bytes left in buffer
     * @param[OUT]    acType          - ber type of object
     * @param[OUT]    apcValue        - pointer to start of output buffer
     * @param[IN]     auValueLen      - size of output buffer
     * 
     * @return Returns pointer to the first byte past the end of this object (i.e. the start of the next object).
     Returns NULL on any error
     */
    unsigned char* BerParseAgentAddr(
        unsigned char* apcBuffer, 
        apl_size_t*    apuMaxSize, 
        unsigned char* apcType, 
        char*          apcValue, 
        apl_size_t     auValueLen);

private:
    acl::CMemoryBlock*  mpoBuffer;
};
} //namespace snmp

ANF_NAMESPACE_END

#endif //SNMP_MESSAGE_H
