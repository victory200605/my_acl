
/**
 * @file  SNMPMessage.cpp
 */

#include "anf/protocol/SNMPMessage.h"

ANF_NAMESPACE_START

/////////////////////////////[ class CVarBind ]///////////////////////////////////

namespace snmp
{
CVarBind::CVarBind()
    : mcType(0)
    , miValLen(0)
    , mbIsObjid(false)  
{
    apl_memset(macOidName, 0, sizeof(macOidName));
}

CVarBind::CVarBind(const CVarBind& aoVb)
{
    *this = aoVb;
}

CVarBind::~CVarBind()
{

}

const char* CVarBind::GetOidName()
{
    return macOidName;
}

void CVarBind::SetOidName(const char* apcOidName)
{
    apl_strncpy(this->macOidName, apcOidName, sizeof(this->macOidName));
}

apl_int_t CVarBind::GetValueType()
{
    return this->mcType;
}

void CVarBind::SetValueType(unsigned char acType)
{
    this->mcType = acType;
}

void CVarBind::GetValue(apl_int32_t& ai32Val)
{
    ai32Val = moVal.mi32Integer;
}

void CVarBind::GetValue(char* apcVal)
{
    if(mbIsObjid)
    {
        apl_strncpy(apcVal, moVal.macObjid, ANF_SNMP_MAX_LEN);
    }
    else
    {
        apl_strncpy(apcVal, moVal.macString, ANF_SNMP_MAX_LEN);
    }
}

apl_int_t CVarBind::SetValue(char* apcVal, unsigned char acType)
{
    if(acType == ANF_BER_INTEGER || acType == ANF_BER_TIMETICKS)
    {
        this->mcType = acType;
        this->moVal.mi32Integer = apl_strtoi32(apcVal, NULL, 10);
        this->miValLen = sizeof(this->moVal.mi32Integer);
    }
    else if(acType == ANF_BER_OCTET_STR || acType == ANF_BER_IPADDRESS || acType == ANF_BER_NULL)
    {
        this->mcType = acType;
        apl_strncpy(this->moVal.macString, apcVal, sizeof(this->moVal.macString));
        this->miValLen = apl_strlen(this->moVal.macString);
    }
    else if(acType == ANF_BER_OBJECT_ID)
    {
        this->mbIsObjid = true;
        this->mcType = acType;                                                  
        apl_strncpy(this->moVal.macObjid, apcVal, sizeof(this->moVal.macObjid));
        this->miValLen = apl_strlen(this->moVal.macObjid);                
    }
    else if(acType == ANF_BER_NULL)
    {
        this->mcType = acType;
        this->miValLen = 1;
    }
    else
    {
        return -1;    
    }

    return 0;
}

/////////////////////////////[ class CSNMPMessage ]///////////////////////////////////

CSNMPMessage::CSNMPMessage() 
    : mi32Version(ANF_SNMP_VERSION_1)
    , miType(0)
    , miVbCount(0)
    , mbIsValidity(true)
{
    apl_memset(this->macReadCommunity, 0, sizeof(this->macReadCommunity));
    apl_strncpy(this->macReadCommunity, "public", sizeof(this->macReadCommunity));
}

CSNMPMessage::CSNMPMessage(CVarBind* apoVbs, apl_int_t aiVbCount)
    : mi32Version(ANF_SNMP_VERSION_1)
    , miType(0)
    , miVbCount(0)
{
    apl_memset(this->macReadCommunity, 0, sizeof(this->macReadCommunity));
    apl_strncpy(this->macReadCommunity, "public", sizeof(this->macReadCommunity));

    if(0 == aiVbCount)
    {
        this->mbIsValidity = true;
        return ;        
    }

    if(aiVbCount > ANF_SNMP_PDU_MAX_VBS)
    {
        this->mbIsValidity = false;
        return ;
    }

    for(apl_int_t liN=0; liN<aiVbCount; ++liN )
    {
        this->mpoVbs[liN] = new CVarBind(apoVbs[liN]);

        if(0 == this->mpoVbs[liN])
        {
            for(apl_int_t liP=0; liP<liN; ++liP)
            {
                ACL_DELETE(this->mpoVbs[liP]);
                this->mbIsValidity = false;
                return ;
            }
        }
    }

    this->miVbCount = aiVbCount;

    this->mbIsValidity = true;

    return ;
}

CSNMPMessage::~CSNMPMessage()
{
    for(apl_int_t liN=0; liN<this->miVbCount; ++liN)
    {
        ACL_DELETE(this->mpoVbs[liN]);
    }
}

apl_int_t CSNMPMessage::GetVbList(CVarBind* apoVbs, apl_int_t aiVbCount)
{
    if((!apoVbs) || (aiVbCount < 0) || (aiVbCount > this->miVbCount))
    {
        return -1;
    }

    for(apl_int_t liN=0; liN<aiVbCount; ++liN)
    {
        apoVbs[liN] = *(this->mpoVbs[liN]);
    }

    return 0;
}

apl_int_t CSNMPMessage::SetVbList(CVarBind* apoVbs, apl_int_t aiVbCount)
{
    if((!apoVbs) || (aiVbCount < 0) || (aiVbCount > ANF_SNMP_PDU_MAX_VBS))
    {
        return -1;
    }

    // free up current vbs
    for(apl_int_t liN=0; liN<this->miVbCount; ++liN)
    {
        ACL_DELETE(this->mpoVbs[liN]);
    }
    this->miVbCount = 0;

    // check for zero case
    if(0 == aiVbCount)
    {
        this->mbIsValidity = true;
        return -1;
    }

    // loop through all vbs and reassign them
    for(apl_int_t liN=0; liN<aiVbCount; ++liN)
    {
        this->mpoVbs[liN] = new CVarBind(apoVbs[liN]);
        if(0 == this->mpoVbs[liN])
        {
            for(apl_int_t liP=0; liP<liN; ++liP)
            {
                ACL_DELETE(this->mpoVbs[liP]);
            }

            this->mbIsValidity = false;

            return -1;
        }
    }

    this->miVbCount = aiVbCount;

    this->mbIsValidity = true;

    return 0;
}

apl_int_t CSNMPMessage::GetVb(CVarBind& aoVb, apl_int_t aiIdx)
{
    if((aiIdx < 0) || (aiIdx > (this->miVbCount - 1)))
    {
        return -1;
    }

    aoVb = *this->mpoVbs[aiIdx];

    return 0;
}

apl_int_t CSNMPMessage::SetVb(CVarBind& aoVb, apl_int_t aiIdx)
{
    if((aiIdx < 0) || (aiIdx > (this->miVbCount - 1)))
    {
        return -1;
    }

    // save in case new fails
    CVarBind* lpoTmpVb = this->mpoVbs[aiIdx];
    this->mpoVbs[aiIdx] = new CVarBind(aoVb);
    if(this->mpoVbs[aiIdx])
    {
        ACL_DELETE(lpoTmpVb);
    }
    else
    {
        this->mpoVbs[aiIdx] = lpoTmpVb;
        return -1;
    }

    return 0;
}

apl_int_t CSNMPMessage::GetVbCount()
{
    return this->miVbCount;
}

apl_int32_t CSNMPMessage::GetVersion()
{
    return this->mi32Version;
}

void CSNMPMessage::SetVersion(apl_int32_t ai32Version)
{
    this->mi32Version = ai32Version;
}

const char* CSNMPMessage::GetReadCommunity()
{
    return this->macReadCommunity;
}

void CSNMPMessage::SetReadCommunity(const char* apcReadCommunity)
{
    apl_memset(this->macReadCommunity, 0, sizeof(this->macReadCommunity));
    apl_strncpy(this->macReadCommunity, apcReadCommunity, sizeof(this->macReadCommunity));
}

apl_int_t CSNMPMessage::GetType()
{
    return this->miType;
}

void CSNMPMessage::SetType(apl_int_t aiType)
{
    this->miType = aiType;
}

CSNMPMessage& CSNMPMessage::operator+=(CVarBind& aoVb)
{
    if((this->miVbCount + 1) > ANF_SNMP_PDU_MAX_VBS)
    {
        // haven't room
        return *this;
    }

    this->mpoVbs[miVbCount] = new CVarBind(aoVb);

    if(this->mpoVbs[miVbCount])
    {
        this->miVbCount++;
        this->mbIsValidity = true;
    }

    return *this;
}

std::string CSNMPMessage::GetMessageType()
{
    if(this->miType == 0xA0)
    {
        this->moStrType.append("snmp_get");
    }
    else if(this->miType == 0xA1)
    {
        this->moStrType.append("snmp_getnext");
    }
    else if(this->miType == 0xA2)
    {
        this->moStrType.append("snmp_set");
    }
    else if(this->miType == 0xA3)
    {
        this->moStrType.append("snmp_response");
    }
    else if(this->miType == 0xA4)
    {
        this->moStrType.append("snmp_trap");
    }

    return this->moStrType;
}

/////////////////////////////[ class CNormalMessage ]///////////////////////////////////

CNormalMessage::CNormalMessage()
    : mi32RequestId(0)
    , mi32ErrStat(0)
    , mi32ErrIndex(0)
{
}

CNormalMessage::CNormalMessage(CVarBind* apoVbs, apl_int_t aiVbCount)
    : CSNMPMessage(apoVbs, aiVbCount)
    , mi32RequestId(0)
    , mi32ErrStat(0)
    , mi32ErrIndex(0)
{
}

CNormalMessage::~CNormalMessage()
{
}

apl_int32_t CNormalMessage::GetRequestId()
{
    return this->mi32RequestId;
}

void CNormalMessage::SetRequestId(apl_int32_t ai32RequestId)
{
    this->mi32RequestId = ai32RequestId;
}

apl_int32_t CNormalMessage::GetErrStat()
{
    return this->mi32ErrStat;
}

void CNormalMessage::SetErrStat(apl_int32_t ai32ErrStat)
{
    this->mi32ErrStat = ai32ErrStat;
}

apl_int_t CNormalMessage::GetErrIndex()
{
    return this->mi32ErrIndex;
}

void CNormalMessage::SetErrIndex(apl_int32_t ai32ErrIndex)
{
    this->mi32ErrIndex = ai32ErrIndex;
}

CNormalMessage& CNormalMessage::operator=(CNormalMessage& aoNormalPdu)
{
    this->mi32Version = aoNormalPdu.GetVersion();
    apl_strncpy(this->macReadCommunity, aoNormalPdu.GetReadCommunity(), sizeof(this->macReadCommunity));
    this->miType = aoNormalPdu.GetType();
    this->mi32RequestId = aoNormalPdu.GetRequestId();
    this->mi32ErrStat = aoNormalPdu.GetErrStat();
    this->mi32ErrIndex = aoNormalPdu.GetErrIndex();

    this->mbIsValidity = true;
    // free up old vbs
    for(apl_int_t liN=0; liN<this->miVbCount; ++liN)
    {
        ACL_DELETE(this->mpoVbs[liN]);
    }
    this->miVbCount = 0;

    // check for zero case
    if(aoNormalPdu.miVbCount == 0)
    {
        return *this;
    }

    // loop through and fill them up
    for(apl_int_t liN=0; liN<aoNormalPdu.miVbCount; ++liN)
    {
        this->mpoVbs[liN] = new CVarBind(*(aoNormalPdu.mpoVbs[liN]));
        if(0 == this->mpoVbs[liN])
        {
            for(apl_int_t liP=0; liP<liN; ++liP)
            {
                ACL_DELETE(this->mpoVbs[liP]);
                this->mbIsValidity = false;
                return *this;
            }
        }
    }

    this->miVbCount = aoNormalPdu.miVbCount;

    return *this;
}

/////////////////////////////[ class CTrapMessage ]///////////////////////////////////

CTrapMessage::CTrapMessage()
    : mi32TrapType(0)
    , mi32SpecificType(0)
    , mu64Time(time(NULL))
{
    apl_memset(this->macEnterprise, 0, sizeof(this->macEnterprise));

    apl_memset(this->macAgentAddr, 0, sizeof(this->macAgentAddr));

    SetType(ANF_SNMP_BER_TRAP);
}

CTrapMessage::CTrapMessage(CVarBind* apoVbs, apl_int_t aiVbCount)
    : CSNMPMessage(apoVbs, aiVbCount)
    , mi32TrapType(0)
    , mi32SpecificType(0)
    , mu64Time(time(NULL))
{
    apl_memset(this->macEnterprise, 0, sizeof(this->macEnterprise));

    apl_memset(this->macAgentAddr, 0, sizeof(this->macAgentAddr));

    SetType(ANF_SNMP_BER_TRAP);
}

CTrapMessage::~CTrapMessage()
{

}

const char* CTrapMessage::GetEnterprise()
{
    return this->macEnterprise;
}

void CTrapMessage::SetEnterprise(const char* apcEnterprise)
{
    apl_strncpy(this->macEnterprise, apcEnterprise, sizeof(this->macEnterprise));
}

const char* CTrapMessage::GetAgentAddr()
{
    return this->macAgentAddr;
}

void CTrapMessage::SetAgentAddr(const char* apcAgentAddr)
{
    apl_strncpy(this->macAgentAddr, apcAgentAddr, sizeof(this->macAgentAddr));
}

apl_int32_t CTrapMessage::GetTrapType()
{
    return this->mi32TrapType;
}

void CTrapMessage::SetTrapType(apl_int32_t ai32TrapType)
{
    this->mi32TrapType = ai32TrapType;
}

apl_int32_t CTrapMessage::GetSpecificType()
{
    return this->mi32SpecificType;
}

void CTrapMessage::SetSpecificType(apl_int32_t ai32SpecificType)
{
    this->mi32SpecificType = ai32SpecificType;
}

apl_uint32_t CTrapMessage::GetTimestamp()
{
    return this->mu64Time;
}

void CTrapMessage::SetTimestamp(apl_uint32_t au64Time)
{
    this->mu64Time = au64Time;
}

/////////////////////////////[ class CSNMPEncoder ]///////////////////////////////////

CSNMPEncoder::CSNMPEncoder(acl::CMemoryBlock* apoBuffer)
    : mpoBuffer(apoBuffer)
{

}

CSNMPEncoder::~CSNMPEncoder(void)
{

}

apl_int_t CSNMPEncoder::EncodeMessage(CSNMPMessage* apoMsg)
{
    apl_int_t liRet = -1;

    acl::CMemoryBlock* lpoMemBlock = this->GetBuffer();

    CNormalMessage* lpoMsg = NULL;

    lpoMsg = dynamic_cast<CNormalMessage*>(apoMsg);
    if(lpoMsg != NULL)
    {
        liRet = this->NormalPduBuild(
            lpoMsg, 
            (unsigned char*)lpoMemBlock->GetWritePtr(),
            lpoMemBlock->GetCapacity()); 
    }
    else
    {
        CTrapMessage* lpoMsg = dynamic_cast<CTrapMessage*>(apoMsg);
        if(lpoMsg == NULL)
        {
            return -1;
        }

        liRet = this->TrapPduBuild(
            lpoMsg, 
            (unsigned char*)lpoMemBlock->GetWritePtr(),
            lpoMemBlock->GetCapacity()); 
    }

    return liRet;
}

/////////////////////////////[ PDU Encoder ]/////////////////////////////

unsigned char* CSNMPEncoder::VarBindBuild(
    unsigned char* apcBuffer,
    apl_size_t*    apuBufSize,
    apl_uint32_t*  apu32OidName,
    apl_int_t      aiOidNameLen,
    unsigned char  acVarValType,
    char*          apcVal,
    apl_int_t      aiValLen)
{
    apl_size_t          luLength = 0;
    unsigned char       *lpcH,*lpcHe = 0;

    lpcH = apcBuffer;
    luLength = *apuBufSize;

    apcBuffer = BerBuildSequence(apcBuffer, apuBufSize,(unsigned char) (ANF_BER_SEQUENCE), 0); 
    if (apcBuffer== NULL)
    {
        return NULL;
    }
    lpcHe = apcBuffer; 

    apcBuffer = BerBuildObjid(
        apcBuffer, 
        apuBufSize,
        (unsigned char)(ANF_BER_OBJECT_ID), 
        apu32OidName,
        aiOidNameLen);

    if (apcBuffer == NULL) 
    {
        //        printf("Can't build OID for variable");
        return NULL;
    }

    switch(acVarValType) 
    {
    case ANF_BER_GAUGE:
    case ANF_BER_COUNTER:
    case ANF_BER_TIMETICKS:
        apcBuffer = BerBuildUnsignedInt(
            apcBuffer, 
            apuBufSize, 
            acVarValType,
            (apl_uint32_t *)apcVal, 
            aiValLen);
        break;

    case ANF_BER_OCTET_STR:
        apcBuffer = BerBuildString(apcBuffer, apuBufSize, acVarValType, apcVal, aiValLen);
        break;

    case ANF_BER_IPADDRESS:
    {
        char lacAddress[4] = {0};
        apl_int_t liAddLen = CutAddr(apcVal,lacAddress,4);

        apcBuffer= BerBuildString(
            apcBuffer, 
            apuBufSize,
            acVarValType,
            (char*)lacAddress, 
            liAddLen);
        break;
    }

    case ANF_BER_OBJECT_ID:
    {
        apl_uint32_t lau32ObjidVar[ANF_SNMP_OID_NUM] = {0};
        aiValLen = ArrToOid(apcVal,lau32ObjidVar,ANF_SNMP_OID_NUM); 
        apcBuffer = BerBuildObjid(
            apcBuffer, 
            apuBufSize, 
            acVarValType,
            (apl_uint32_t *)lau32ObjidVar,
            aiValLen );
        break;
    }

    case ANF_BER_INTEGER:
        apcBuffer = BerBuildInt(
            apcBuffer, 
            apuBufSize, 
            acVarValType,
            (apl_int32_t *)apcVal, 
            aiValLen);
        break;

    case ANF_BER_NULL:
        apcBuffer = BerBuildNull(apcBuffer, apuBufSize, acVarValType);
        break;

    default:
    {
        apcBuffer = NULL;
        break;
    }
    }

    if (apcBuffer == NULL) 
    {
        return NULL;
    }

    BerBuildSequence(lpcH, &luLength,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer - lpcHe);

    return apcBuffer;
}

apl_int_t CSNMPEncoder::CutAddr( char* apcSrc, char* apcDes, apl_int_t aiSize)
{
    apl_int_t   liCounter = 0;
    const char* lpcName = apcSrc;
    for( lpcName=apcSrc; *lpcName; ++lpcName )
    {
        if( (*lpcName < '0') && (*lpcName > '9') && '.' != *lpcName )
        {
            return -1;
        }
    }

    lpcName = apcSrc;
    char* lpcNext;
    while(*lpcName == '.')
    {   
        lpcName++;
    }  

    for (apl_int_t i = 0; i < aiSize; ++i)
    {   
        errno = 0;
        apl_int_t c = apl_strtou32(lpcName, &lpcNext, 10);

        sprintf(apcDes+i,"%c",c);
        if (errno != 0) 
        {
            return -1;
        }

        ++liCounter;
        if (*lpcNext == '\0')
        {
            break;
        }
        if (*lpcNext != '.')
        {
            return -1;
        }

        lpcName = lpcNext + 1;
    }  

    return liCounter;
}

apl_int_t CSNMPEncoder::ArrToOid( 
    char const*   apcName, 
    apl_uint32_t* apu32Oid, 
    apl_size_t    auSize)
{
    apl_int_t liCounter = 0;
    const char* lpcName = apcName;
    for( lpcName=apcName; *lpcName; ++lpcName )
    {
        if( (*lpcName < '0') && (*lpcName > '9') && '.' != *lpcName )
        {
            return ANF_SNMP_ERROIDS;
        }
    }

    lpcName = apcName;
    char* lpcNext = NULL;
    while(*lpcName == '.')
    {
        lpcName++;
    }

    for (apl_size_t i = 0; i < auSize; ++i)
    {
        errno = 0;
        apu32Oid[i] = apl_strtou32(lpcName, &lpcNext, 10);
        if (errno != 0)
        {
            return ANF_SNMP_ERRCOV;
        }

        ++liCounter;
        if (*lpcNext == '\0')
        {
            break;
        }
        if (*lpcNext != '.')
        {
            return ANF_SNMP_ERRCOV;
        }

        lpcName = lpcNext + 1;
    }

    return liCounter;
}

apl_int_t CSNMPEncoder::NormalPduBuild(
    CNormalMessage* apoMsg, 
    unsigned char*  apcBuffer, 
    apl_size_t      auMaxSize)
{
    apl_size_t       luLength1 = 0;
    unsigned char    *lpcH0 = NULL, *lpcH0e = NULL;

    lpcH0 = apcBuffer;
    luLength1 = auMaxSize;

    apcBuffer = BerBuildSequence(apcBuffer, &auMaxSize, (unsigned char) (ANF_BER_SEQUENCE), 0); 
    if(apcBuffer == NULL)
    {
        return ANF_SNMP_ERRSEQ;
    }
    lpcH0e = apcBuffer;

    //build version
    apl_int32_t li32Version = apoMsg->GetVersion();
    apcBuffer = BerBuildInt(
        apcBuffer, 
        &auMaxSize,
        (unsigned char)(ANF_BER_INTEGER), 
        (apl_int32_t *) &li32Version,
        sizeof(apl_int32_t));

    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERRINT;
    }

    //build community
    const char* lpcReadCommunity = apoMsg->GetReadCommunity();
    if(NULL == lpcReadCommunity)
    {
        return ANF_SNMP_ERRGETREADCOMMUNITY;
    }

    apcBuffer = BerBuildString(apcBuffer, 
        &auMaxSize,
        (unsigned char)(ANF_BER_OCTET_STR), 
        (char*)lpcReadCommunity,
        apl_strlen(lpcReadCommunity));

    if(apcBuffer == NULL)
    {
        return ANF_SNMP_ERRSTR;
    }

    apl_int_t     liType = apoMsg->GetType();
    apl_size_t    luLength2 = 0;
    unsigned char *lpcH1,*lpcH1e;

    luLength2 = auMaxSize;
    lpcH1 = apcBuffer;
    apcBuffer = BerBuildSequence(apcBuffer, &auMaxSize, (unsigned char)liType, 0);

    //test request command
    if( liType != 0xA0 && liType != 0xA1)
    {
        return ANF_SNMP_EWRONGCOMMAND;
    }
    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERRSEQ;
    }
    lpcH1e = apcBuffer;

    //build request id  
    apl_int32_t li32RequestId = apoMsg->GetRequestId();
    apcBuffer= BerBuildInt(
        apcBuffer, 
        &auMaxSize,
        (unsigned char)(ANF_BER_INTEGER), 
        &li32RequestId,
        sizeof(li32RequestId));

    if (apcBuffer== NULL)
    {
        return ANF_SNMP_ERRINT;
    }

    //build error status
    apl_int32_t li32ErrStat = apoMsg->GetErrStat();
    apcBuffer = BerBuildInt(
        apcBuffer, 
        &auMaxSize,
        (unsigned char)(ANF_BER_INTEGER), 
        &li32ErrStat,
        sizeof(li32ErrStat));

    if (apcBuffer== NULL)
    {
        return ANF_SNMP_ERRINT;
    }

    //build error index
    apl_int32_t  li32ErrIndex = apoMsg->GetErrIndex();
    apcBuffer = BerBuildInt(
        apcBuffer, 
        &auMaxSize,
        (unsigned char)(ANF_BER_INTEGER), 
        &li32ErrIndex,
        sizeof(li32ErrIndex));
    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERRINT;
    }

    apl_size_t      luLength3 = 0;
    unsigned char   *lpcH2 = NULL,*lpcH2e = NULL;

    luLength3 = auMaxSize;
    lpcH2 = apcBuffer;
    apcBuffer = BerBuildSequence(apcBuffer,&auMaxSize,(unsigned char) (ANF_BER_INTEGER), 0); 
    if(apcBuffer == NULL)
    {
        return ANF_SNMP_ERRSEQ;
    }
    lpcH2e = apcBuffer;

    //build variable-bindings
    apl_int_t liVbCount = apoMsg->GetVbCount();
    CVarBind loTmpVbs[liVbCount];
    apoMsg->GetVbList(loTmpVbs, liVbCount);

    for(apl_int_t liN=0; liN<liVbCount; ++liN)
    {
        apl_uint32_t lau32ObjidVar[ANF_SNMP_OID_NUM];
        apl_int_t liNameLen = ArrToOid(loTmpVbs[liN].macOidName, lau32ObjidVar, ANF_SNMP_OID_NUM);

        apcBuffer = VarBindBuild(
            apcBuffer, 
            &auMaxSize,
            lau32ObjidVar,
            liNameLen, 
            loTmpVbs[liN].mcType,
            (char*)loTmpVbs[liN].moVal.macString,
            loTmpVbs[liN].miValLen); 

        if(apcBuffer == NULL)
        {
            return ANF_SNMP_ERRVARB;
        }
    }

    if ((apcBuffer- lpcH2e) < 0x80)
    {
        BerBuildSequence(lpcH2, &luLength3,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer - lpcH2e);
    } 
    else if ((apcBuffer - lpcH2e) <= 0xFF) 
    {
        apl_memmove(lpcH2e + 1 , lpcH2e , apcBuffer - lpcH2e );
        luLength3 -= 1;
        BerBuildSequence(lpcH2, &luLength3,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer - lpcH2e);
        apcBuffer++;
    } 
    else if ((apcBuffer - lpcH2e) <= 0xFFFF) 
    {
        apl_memmove(lpcH2e + 2 , lpcH2e , apcBuffer - lpcH2e );
        luLength3 -= 2;
        BerBuildSequence(lpcH2, &luLength3,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer - lpcH2e);
        apcBuffer+=2;
    } 

    if ((apcBuffer - lpcH1e) < 0x80)
    {
        BerBuildSequence(lpcH1, &luLength2, (unsigned char)liType, apcBuffer - lpcH1e);
    } 
    else if ((apcBuffer - lpcH1e) <= 0xFF) 
    {
        apl_memmove(lpcH1e + 1 , lpcH1e , apcBuffer - lpcH1e );
        luLength2 -= 1;
        BerBuildSequence(lpcH1, &luLength2, (unsigned char)liType, apcBuffer - lpcH1e);
        apcBuffer++;
    } 
    else if ((apcBuffer - lpcH1e) <= 0xFFFF) 
    {
        apl_memmove(lpcH1e + 2 , lpcH1e , apcBuffer - lpcH1e );
        luLength2 -= 2;
        BerBuildSequence(lpcH1, &luLength2, (unsigned char)liType, apcBuffer - lpcH1e);
        apcBuffer+=2;
    } 
    else if ((apcBuffer - lpcH1e) <= 0xFFFFFF) 
    {
        apl_memmove(lpcH1e + 3 , lpcH1e , apcBuffer - lpcH1e );
        luLength2 -= 3;
        BerBuildSequence(lpcH1, &luLength2, (unsigned char)liType, apcBuffer - lpcH1e);
        apcBuffer+=3;
    } 

    if ((apcBuffer - lpcH0e) < 0x80)
    {
        BerBuildSequence(lpcH0, &luLength1,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer - lpcH0e);
        auMaxSize = apcBuffer - lpcH0;
    } 
    else if ((apcBuffer - lpcH0e) <= 0xFF) 
    {
        apl_memmove(lpcH0e + 1 , lpcH0e , apcBuffer - lpcH0e );
        luLength1 -= 1;
        BerBuildSequence(lpcH0, &luLength1,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer - lpcH0e );
        auMaxSize = apcBuffer - lpcH0 + 1 ;
    } 
    else if ((apcBuffer - lpcH0e) <= 0xFFFF) 
    {
        apl_memmove(lpcH0e + 2 , lpcH0e , apcBuffer - lpcH0e );
        luLength1 -= 2;
        BerBuildSequence(lpcH0, &luLength1,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer - lpcH0e );
        auMaxSize = apcBuffer - lpcH0 + 2 ;
    } 
    else if ((apcBuffer - lpcH0e) <= 0xFFFFFF) 
    {
        apl_memmove(lpcH0e + 3 , lpcH0e , apcBuffer - lpcH0e );
        luLength1 -= 3;
        BerBuildSequence(lpcH0, &luLength1,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer - lpcH0e );
        auMaxSize = apcBuffer - lpcH0 + 3 ;
    } 
    else if ((unsigned)(apcBuffer - lpcH0e) <= 0xFFFFFFFFul) 
    {
        apl_memmove(lpcH0e + 4 , lpcH0e , apcBuffer - lpcH0e );
        luLength1 -= 4;
        BerBuildSequence(lpcH0, &luLength1,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer - lpcH0e );
        auMaxSize = apcBuffer - lpcH0 + 4 ;
    } 

    return auMaxSize;
}

apl_int_t CSNMPEncoder::TrapPduBuild(
    CTrapMessage*  apoMsg, 
    unsigned char* apcBuffer, 
    apl_size_t     auMaxSize)
{
    apl_size_t       luLength = 0;
    unsigned char    *lpcH = 0,*lpcHe = 0;

    lpcH = apcBuffer;
    luLength = auMaxSize;

    apcBuffer = BerBuildSequence(apcBuffer, &auMaxSize,(unsigned char) (ANF_BER_SEQUENCE), 0); 
    if(apcBuffer == NULL)
    {
        return ANF_SNMP_ERRSEQ;
    }
    lpcHe = apcBuffer;

    //build version
    apl_int32_t li32Version = apoMsg->GetVersion();
    apcBuffer = BerBuildInt(
        apcBuffer, 
        &auMaxSize,
        (unsigned char)(ANF_BER_INTEGER), 
        (apl_int32_t *)&li32Version,
        sizeof(li32Version));

    if (apcBuffer== NULL)
    {
        return ANF_SNMP_ERRINT;
    }

    //build community
    const char* lpcReadCommunity = apoMsg->GetReadCommunity();
    if(NULL == lpcReadCommunity)
    {
        return ANF_SNMP_ERRGETREADCOMMUNITY;
    }
    apcBuffer = BerBuildString(
        apcBuffer, 
        &auMaxSize,
        (unsigned char)(ANF_BER_OCTET_STR), 
        (char*)lpcReadCommunity,
        apl_strlen(lpcReadCommunity));

    if(apcBuffer == NULL)
    {
        return ANF_SNMP_ERRSTR;
    }

    apl_size_t      luLength1 = 0;
    unsigned char   *lpcH0 = 0,*lpcH0e = 0;
    luLength1 = auMaxSize;

    lpcH0 = apcBuffer;
    apcBuffer = BerBuildSequence(apcBuffer, &auMaxSize, (unsigned char) (ANF_SNMP_BER_TRAP), 0);
    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERRSEQ;
    }
    lpcH0e = apcBuffer;

    //enterprise 
    const char* lpcEnterprise = apoMsg->GetEnterprise();
    if(NULL == lpcEnterprise)
    {
        return ANF_SNMP_ERRGETENTERPRISE;
    }

    apl_uint32_t lau32ObjidVar[ANF_SNMP_OID_NUM] = {0};

    apl_int_t liEnterpriseLen = ArrToOid(lpcEnterprise, lau32ObjidVar, ANF_SNMP_OID_NUM);
    apcBuffer= BerBuildObjid(
        apcBuffer, 
        &auMaxSize,
        (unsigned char)(ANF_BER_OBJECT_ID),
        (apl_uint32_t *)lau32ObjidVar,
        liEnterpriseLen);

    if (apcBuffer== NULL)
    {
        return ANF_SNMP_ERROID;
    }

    //agent address
    const char* lpcAgentAddr = apoMsg->GetAgentAddr();
    if(NULL == lpcAgentAddr)
    {
        return ANF_SNMP_ERRGETAGENTADDR;
    }

    char lacAddress[4] = {0};
    apl_int_t liAddLen = CutAddr((char*)lpcAgentAddr,lacAddress,4);
    apcBuffer= BerBuildString(
        apcBuffer, 
        &auMaxSize,
        (unsigned char)(ANF_BER_IPADDRESS),
        (char*) lacAddress, 
        liAddLen); 
    if (apcBuffer== NULL)
    {
        return ANF_SNMP_ERRSTR;
    }

    //trap type
    apl_int32_t li32TrapType = apoMsg->GetTrapType();
    apcBuffer = BerBuildInt(
        apcBuffer, 
        &auMaxSize,
        (unsigned char)(ANF_BER_INTEGER),
        (apl_int32_t *)&li32TrapType,
        sizeof(li32TrapType));

    if (apcBuffer== NULL)
    {
        return ANF_SNMP_ERRINT;
    }

    //special
    apl_int32_t li32SpecificType = apoMsg->GetSpecificType();
    apcBuffer = BerBuildInt(
        apcBuffer, 
        &auMaxSize,
        (unsigned char)(ANF_BER_INTEGER),
        (apl_int32_t *)&li32SpecificType,
        sizeof(li32SpecificType));

    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERRINT;
    }

    //timestamp
    apl_uint32_t lu32Time = apoMsg->GetTimestamp();
    apcBuffer = BerBuildUnsignedInt(
        apcBuffer, 
        &auMaxSize,
        (unsigned char)(ANF_BER_TIMETICKS), 
        (apl_uint32_t*)&lu32Time,
        sizeof(apl_uint32_t));

    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERRUINT;
    }

    apl_size_t      luLength2 = 0;
    unsigned char   *lpcH1 = 0,*lpcH1e = 0;

    luLength2 = auMaxSize;
    lpcH1 = apcBuffer;
    apcBuffer= BerBuildSequence(apcBuffer, &auMaxSize,(unsigned char) (ANF_BER_SEQUENCE), 0); 
    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERRSEQ;
    }
    lpcH1e = apcBuffer;

    //store variable-bindings
    apl_int_t liVbCount = apoMsg->GetVbCount();
    CVarBind loTmpVbs[liVbCount];
    apoMsg->GetVbList(loTmpVbs, liVbCount);

    for(apl_int_t liN=0; liN<liVbCount; ++liN)
    {
        apl_uint32_t lau32ObjidVar[ANF_SNMP_OID_NUM] = {0};
        apl_int_t liNameLength=ArrToOid(loTmpVbs[liN].macOidName, lau32ObjidVar,ANF_SNMP_OID_NUM);

        apcBuffer = VarBindBuild(
            apcBuffer, 
            &auMaxSize,
            lau32ObjidVar, 
            liNameLength, 
            loTmpVbs[liN].mcType, 
            (char*) loTmpVbs[liN].moVal.macString, 
            loTmpVbs[liN].miValLen); 

        if (apcBuffer == NULL)
        {
            return ANF_SNMP_ERRVARB;
        }
    }

    if ((apcBuffer - lpcH1e) < 0x80)
    {
        BerBuildSequence(lpcH1, &luLength2,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer- lpcH1e);
    } 
    else if ((apcBuffer - lpcH1e) <= 0xFF) 
    {
        apl_memmove(lpcH1e + 1 , lpcH1e , apcBuffer - lpcH1e );
        luLength2 -= 1;
        BerBuildSequence(lpcH1, &luLength2,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer- lpcH1e);
        apcBuffer++;
    } 
    else if ((apcBuffer - lpcH1e) <= 0xFFFF) 
    {
        apl_memmove(lpcH1e + 2 , lpcH1e , apcBuffer - lpcH1e );
        luLength2 -= 2;
        BerBuildSequence(lpcH1, &luLength2,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer- lpcH1e);
        apcBuffer += 2;
    } 

    //-----------------------

    if ((apcBuffer - lpcH0e) < 0x80)
    {
        BerBuildSequence(lpcH0, &luLength1, (unsigned char) (ANF_SNMP_BER_TRAP), apcBuffer - lpcH0e);
        //auMaxSize = apcBuffer - lpcH0;
    } 
    else if ((apcBuffer - lpcH0e) <= 0xFF) 
    {
        apl_memmove(lpcH0e + 1 , lpcH0e , apcBuffer - lpcH0e );
        luLength1 -= 1;
        BerBuildSequence(lpcH0, &luLength1, (unsigned char) (ANF_SNMP_BER_TRAP), apcBuffer - lpcH0e);
        //auMaxSize = apcBuffer - lpcH0 + 1 ;
        apcBuffer++;
    } 
    else if ((apcBuffer - lpcH0e) <= 0xFFFF) 
    {
        apl_memmove(lpcH0e + 2 , lpcH0e , apcBuffer - lpcH0e );
        luLength1 -= 2;
        BerBuildSequence(lpcH0, &luLength1, (unsigned char) (ANF_SNMP_BER_TRAP), apcBuffer - lpcH0e);
        //auMaxSize = apcBuffer - lpcH0 + 2 ;
        apcBuffer+=2;
    } 
    else if ((apcBuffer - lpcH0e) <= 0xFFFFFF) 
    {
        apl_memmove(lpcH0e + 3 , lpcH0e , apcBuffer - lpcH0e );
        luLength1 -= 3;
        BerBuildSequence(lpcH0, &luLength1, (unsigned char) (ANF_SNMP_BER_TRAP), apcBuffer - lpcH0e);
        //auMaxSize = apcBuffer - lpcH0 + 3 ;
        apcBuffer+=3;
    } 

    //--------------------------------

    if ((apcBuffer - lpcHe) < 0x80)
    {
        BerBuildSequence(lpcH, &luLength,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer - lpcHe);
        auMaxSize = apcBuffer - lpcH;
    } 
    else if ((apcBuffer - lpcHe) <= 0xFF) 
    {
        apl_memmove(lpcHe + 1 , lpcHe , apcBuffer - lpcHe );
        luLength -= 1;
        BerBuildSequence(lpcH, &luLength,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer - lpcHe );
        auMaxSize = apcBuffer - lpcH + 1 ;
    } 
    else if ((apcBuffer - lpcHe) <= 0xFFFF) 
    {
        apl_memmove(lpcHe + 2 , lpcHe , apcBuffer - lpcHe );
        luLength -= 2;
        BerBuildSequence(lpcH, &luLength,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer - lpcHe );
        auMaxSize = apcBuffer - lpcH + 2 ;
    } 
    else if ((apcBuffer - lpcHe) <= 0xFFFFFF) 
    {
        apl_memmove(lpcHe + 3 , lpcHe , apcBuffer - lpcHe );
        luLength -= 3;
        BerBuildSequence(lpcH, &luLength,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer - lpcHe );
        auMaxSize = apcBuffer - lpcH + 3 ;
    } 
    else if ((unsigned)(apcBuffer - lpcHe) <= 0xFFFFFFFFul) 
    {
        apl_memmove(lpcHe + 4 , lpcHe , apcBuffer - lpcHe );
        luLength -= 4;
        BerBuildSequence(lpcH, &luLength,(unsigned char) (ANF_BER_SEQUENCE),apcBuffer - lpcHe );
        auMaxSize = apcBuffer - lpcH + 4 ;
    } 

    return auMaxSize;
}

/////////////////////////////[ BER Encoder ]/////////////////////////////

unsigned char* CSNMPEncoder::BerBuildLength(
    unsigned char* apcBuffer, 
    apl_size_t*    apuMaxSize,
    apl_size_t     auValueLen)
{
    unsigned char* lpcBuffer = apcBuffer;
    if (auValueLen < 0x80)
    {
        if (*apuMaxSize < 1)
        {
            return (NULL);
        }
        *apcBuffer ++ = (unsigned char) auValueLen;
    } 

    else if (auValueLen <= 0xFF) 
    {
        if (*apuMaxSize < 2) 
        {
            return (NULL);
        }
        *apcBuffer ++ = (unsigned char) (0x01 | 0x80);
        *apcBuffer ++ = (unsigned char) auValueLen;    
    } 

    else if (auValueLen <= 0xFFFF)
    {                   
        if (*apuMaxSize < 3) 
        {
            return (NULL);
        }
        *apcBuffer ++ = (unsigned char) (0x02 | 0x80);
        *apcBuffer ++ = (unsigned char) ((auValueLen >> 8) & 0xFF);
        *apcBuffer ++ = (unsigned char) (auValueLen & 0xFF);
    }
    else if (auValueLen <= 0xFFFFFF)
    {                    
        if (*apuMaxSize < 4) 
        {
            return (NULL);
        }
        *apcBuffer ++ = (unsigned char) (0x03 | 0x80);
        *apcBuffer ++ = (unsigned char) ((auValueLen >> 16) & 0xFF);
        *apcBuffer ++ = (unsigned char) ((auValueLen >> 8) & 0xFF);
        *apcBuffer ++ = (unsigned char) (auValueLen & 0xFF);
    }
    else
    {
        if(*apuMaxSize <5)
        {
            return (NULL);
        }
        *apcBuffer++ = (unsigned char)(0x04 | 0x80);
        *apcBuffer++ = (unsigned char)((auValueLen >> 24) & 0xFF);
        *apcBuffer++ = (unsigned char)((auValueLen >> 16) & 0xFF);
        *apcBuffer++ = (unsigned char)((auValueLen >> 8) & 0xFF);
        *apcBuffer++ = (unsigned char)(auValueLen & 0xFF);
    }

    *apuMaxSize -= (apcBuffer - lpcBuffer); 
    return (apcBuffer);
}

unsigned char* CSNMPEncoder::BerBuildSequence(
    unsigned char* apcBuffer,
    apl_size_t*    apuMaxSize, 
    unsigned char  acType, 
    apl_size_t     auValueLen)
{
    if (*apuMaxSize < 4)
    {
        return (NULL);
    }

    *apuMaxSize -= 1;
    *apcBuffer++ = acType;

    return BerBuildLength(apcBuffer,apuMaxSize,auValueLen);
}

unsigned char* CSNMPEncoder::BerBuildHeader(
    unsigned char* apcBuffer,
    apl_size_t*    apuMaxSize, 
    unsigned char  acType, 
    apl_size_t     auValueLen)
{
    if (*apuMaxSize < 1) 
    {
        return (NULL);
    }

    *apcBuffer ++ = acType; 
    (*apuMaxSize )--;

    return BerBuildLength(apcBuffer, apuMaxSize, auValueLen);
}

unsigned char* CSNMPEncoder::BerBuildInt(
    unsigned char* apcBuffer, 
    apl_size_t*    apuMaxSize, 
    unsigned char  acType, 
    apl_int32_t*   api32Value, 
    apl_size_t     auValueLen)
{
    apl_int32_t      li32Val = 0;
    apl_uint32_t     lu32Mask = 0;

    if (auValueLen != sizeof(apl_int32_t)) 
    {
        return (NULL);
    }
    li32Val = *api32Value;

    lu32Mask = 0x1FFul << ((8 * (sizeof(apl_int32_t) - 1)) - 1);

    while ((((li32Val & lu32Mask) == 0) || ((li32Val & lu32Mask) == lu32Mask))&& auValueLen > 1) 
    {
        auValueLen--;
        li32Val <<= 8;
    }

    apcBuffer= BerBuildHeader(apcBuffer, apuMaxSize, acType, auValueLen);
    if(apcBuffer==NULL)
    {
        return (NULL);
    }

    *apuMaxSize -= auValueLen;
    lu32Mask =  0xFFul << (8 * (sizeof(apl_int32_t) - 1));

    while (auValueLen--) 
    {
        *apcBuffer++ = (char) ((li32Val & lu32Mask) >> (8 * (sizeof(apl_int32_t) - 1)));
        li32Val <<= 8;
    }

    return (apcBuffer);
}

unsigned char* CSNMPEncoder::BerBuildString(
    unsigned char* apcBuffer,
    apl_size_t*    apuMaxSize,
    unsigned char  acType, 
    const char*    apcValue, 
    apl_size_t     auValueLen)
{
    apcBuffer = BerBuildHeader( apcBuffer, apuMaxSize, acType, auValueLen);

    if( apcBuffer == NULL )
    {
        return  (NULL);
    }

    if( *apuMaxSize < auValueLen)
    {
        return  (NULL);
    } 

    apl_memcpy( apcBuffer, apcValue, auValueLen);
    *apuMaxSize -= auValueLen;

    return  (apcBuffer + auValueLen);
}

unsigned char* CSNMPEncoder::BerBuildObjid(
    unsigned char* apcBuffer,
    apl_size_t*    apuMaxSize,
    unsigned char  acType, 
    apl_uint32_t   *api32Value, 
    apl_size_t     auValueLen) 
{
    apl_size_t          luAsnLength = 0;
    apl_uint32_t*       lpu32Op = NULL;
    unsigned char       lacObjidSize[128] = {0};
    apl_uint32_t        lu32ObjidVal = 0;
    apl_uint32_t        lu32FirstObjidVal = 0;
    apl_int_t           i = 0;

    lpu32Op = api32Value;

    if (auValueLen == 0) 
    {
        lu32ObjidVal = 0;
        auValueLen = 2;
    } 
    else if (api32Value[0] > 2) 
    {
        //        printf("build objid: bad first subidentifier");
        return (NULL);
    } 
    else if (auValueLen == 1) 
    {
        lu32ObjidVal = (lpu32Op[0] * 40);
        auValueLen = 2;
        lpu32Op++;
    } 
    else 
    {
        if ((lpu32Op[1] > 40) && (lpu32Op[0] < 2))
        {
            //            printf("build objid: bad second subidentifier");
            return (NULL);
        }
        lu32ObjidVal = (lpu32Op[0] * 40) + lpu32Op[1]; 
        lpu32Op += 2;
    }   
    lu32FirstObjidVal = lu32ObjidVal;

    if (auValueLen > 128)
    {
        return (NULL);
    }
    for (i = 1, luAsnLength= 0;;) 
    {
        if (lu32ObjidVal < (unsigned) 0x80) 
        {
            lacObjidSize[i] = 1;
            luAsnLength += 1;
        } 
        else if (lu32ObjidVal < (unsigned) 0x4000) 
        {
            lacObjidSize[i] = 2;
            luAsnLength += 2;
        } 
        else if (lu32ObjidVal < (unsigned) 0x200000) 
        {
            lacObjidSize[i] = 3;
            luAsnLength += 3;
        } 
        else if (lu32ObjidVal < (unsigned) 0x10000000) 
        {
            lacObjidSize[i] = 4;
            luAsnLength += 4;
        } 
        else 
        {
            lacObjidSize[i] = 5;
            luAsnLength += 5;
        }
        i++;

        if (i >= (apl_int_t) auValueLen)
        {
            break;
        }
        lu32ObjidVal = *lpu32Op++;    
    }

    apcBuffer = BerBuildHeader(apcBuffer , apuMaxSize, acType, luAsnLength);
    if( apcBuffer == NULL )
    {
        return  (NULL);
    }

    for (i = 1, lu32ObjidVal = lu32FirstObjidVal, lpu32Op = api32Value + 2;i < (apl_int_t) auValueLen; i++) 
    {
        if (i != 1) 
        {
            lu32ObjidVal = *lpu32Op++;
        }

        switch (lacObjidSize[i]) 
        {
            case 1:
                *apcBuffer++ = (unsigned char) lu32ObjidVal;
                break;

            case 2:
                *apcBuffer++ = (unsigned char) ((lu32ObjidVal >> 7) | 0x80);
                *apcBuffer++ = (unsigned char) (lu32ObjidVal & 0x07f);
                break;

            case 3:
                *apcBuffer++ = (unsigned char) ((lu32ObjidVal >> 14) | 0x80);
                *apcBuffer++ = (unsigned char) ((lu32ObjidVal >> 7 & 0x7f) | 0x80);
                *apcBuffer++ = (unsigned char) (lu32ObjidVal & 0x07f);
                break;

            case 4:
                *apcBuffer++ = (unsigned char) ((lu32ObjidVal >> 21) | 0x80);
                *apcBuffer++ = (unsigned char) ((lu32ObjidVal >> 14 & 0x7f) | 0x80);
                *apcBuffer++ = (unsigned char) ((lu32ObjidVal >> 7 & 0x7f) | 0x80);
                *apcBuffer++ = (unsigned char) (lu32ObjidVal & 0x07f);
                break;

            case 5:
                *apcBuffer++ = (unsigned char) ((lu32ObjidVal >> 28) | 0x80);
                *apcBuffer++ = (unsigned char) ((lu32ObjidVal >> 21 & 0x7f) | 0x80);
                *apcBuffer++ = (unsigned char) ((lu32ObjidVal >> 14 & 0x7f) | 0x80);
                *apcBuffer++ = (unsigned char) ((lu32ObjidVal >> 7 & 0x7f) | 0x80);
                *apcBuffer++ = (unsigned char) (lu32ObjidVal & 0x07f);
                break;
        }
    }

    *apuMaxSize -= luAsnLength;

    return apcBuffer;
}

unsigned char* CSNMPEncoder::BerBuildUnsignedInt(
    unsigned char* apcBuffer, 
    apl_size_t*    apuMaxSize,
    unsigned char  acType, 
    apl_uint32_t*  apu32Value, 
    apl_int_t      auValueLen)
{
    apl_uint32_t       lu32Val = 0;
    apl_uint32_t       lu32Mask = 0;

    apl_int_t          liAddNullByte = 0;

    if (auValueLen != sizeof(apl_uint32_t))
    {
        return (NULL);
    }

    lu32Val = *apu32Value;

    lu32Mask = 0xFFul << (8 * (sizeof(apl_uint32_t) - 1));

    if ((unsigned char ) ((lu32Val & lu32Mask) >> (8 * (sizeof(apl_uint32_t) - 1))) & 0x80) 
    {
        liAddNullByte = 1;
        auValueLen++;
    } 
    else 
    {
        lu32Mask = ((apl_uint32_t ) 0x1FF) << ((8 * (sizeof(apl_uint32_t) - 1)) - 1);
        while ((((lu32Val & lu32Mask) == 0) || ((lu32Val & lu32Mask) == lu32Mask))&& auValueLen> 1) 
        {
            auValueLen--;
            lu32Val <<= 8;
        }
    }

    apcBuffer = BerBuildHeader(apcBuffer,apuMaxSize , acType, auValueLen);
    if (apcBuffer==NULL)
    {
        return (NULL);
    }

    *apuMaxSize -= auValueLen;

    if (liAddNullByte == 1) 
    {
        *apcBuffer++ = '\0';
        auValueLen--;
    }

    lu32Mask = ((apl_uint32_t) 0xFF) << (8 * (sizeof(apl_uint32_t) - 1));

    while (auValueLen--) 
    {
        *apcBuffer++ = (unsigned char) ((lu32Val & lu32Mask) >> (8 * (sizeof(apl_uint32_t) - 1)));
        lu32Val <<= 8;
    }

    return (apcBuffer);
}

unsigned char* CSNMPEncoder::BerBuildNull(
    unsigned char* apcBuffer,
    apl_size_t*    apuMaxSize, 
    unsigned char  acType)
{
    apcBuffer= BerBuildHeader(apcBuffer, apuMaxSize, acType, 0);

    return (apcBuffer);
}

/////////////////////////////[ class CSNMPDecoder ]///////////////////////////////////

CSNMPDecoder::CSNMPDecoder(acl::CMemoryBlock* apoBuffer)
    : mpoBuffer(apoBuffer)
{
}

CSNMPDecoder::~CSNMPDecoder()
{
}

apl_int_t CSNMPDecoder::DecodeMessage(CSNMPMessage** appoMsg)
{
    apl_int_t liRet = -1;

    acl::CMemoryBlock* lpoMemBlock = this->GetBuffer();

    apl_int_t liType = GetPduType(
        (unsigned char*)lpoMemBlock->GetReadPtr(),
        lpoMemBlock->GetLength());

    if(liType != 0xA4)
    {
        CNormalMessage* lpoMsg = new CNormalMessage;
        liRet = this->NormalPduParse(
            lpoMsg, 
            (unsigned char*)lpoMemBlock->GetReadPtr(),
            lpoMemBlock->GetLength()); 
        if(liRet != 0)
        {
            *appoMsg = NULL;
            return -1;
        }
        *appoMsg = lpoMsg; 
    }
    else
    {
        CTrapMessage* lpoMsg = new CTrapMessage; 
        liRet = this->TrapPduParse(
            lpoMsg, 
            (unsigned char*)lpoMemBlock->GetReadPtr(),
            lpoMemBlock->GetLength()); 

        if(liRet != 0)
        {
            *appoMsg = NULL;
            return -1;
        }

        *appoMsg = lpoMsg;
    }

    return 0;
}

/////////////////////////////[ PDU Decoder ]/////////////////////////////

unsigned char* CSNMPDecoder::VarBindParse(
    unsigned char*  apcBuffer,
    apl_size_t*     apuBufSize,
    apl_uint32_t*   apu32OidName,
    apl_int_t*      apiOidNameLen,
    unsigned char*  apcVarValType,
    unsigned char** appcVal,
    apl_int_t*      apiValLen)
{
    unsigned char    lcVarOpType;
    apl_size_t       luVarOpLen = *apuBufSize;
    unsigned char    *lpcVarOpStart = apcBuffer;

    apcBuffer = BerParseSequence(
        apcBuffer, 
        &luVarOpLen, 
        &lcVarOpType,
        (unsigned char)(ANF_BER_SEQUENCE));
    if (apcBuffer == NULL) 
    {
        return NULL;
    }

    apcBuffer = BerParseObjid(apcBuffer, &luVarOpLen, &lcVarOpType, apu32OidName,apiOidNameLen);
    if (apcBuffer == NULL) 
    {
        //        printf("No OID for variable");
        return NULL;
    }

    if (lcVarOpType != (unsigned char) (ANF_BER_OBJECT_ID))
    {
        return NULL;
    }

    *appcVal = apcBuffer;            
    apcBuffer = BerParseHeader(apcBuffer, &luVarOpLen, apcVarValType);
    if (apcBuffer == NULL) 
    {
        return NULL;
    }

    *apiValLen = luVarOpLen;
    apcBuffer += luVarOpLen;
    *apuBufSize -= (apl_size_t) (apcBuffer - lpcVarOpStart);
    return apcBuffer;
}

apl_int_t CSNMPDecoder::SetVarObjid(
    CVarBind*     apoVb, 
    apl_uint32_t* apu32Objid, 
    apl_int_t     aiOidNameLen)
{
    if (apu32Objid[0] != 0)
    {    
        char* lpcName = (char*)apoVb->GetOidName();
        for(apl_int_t i=0;i<(aiOidNameLen-1);i++)
        {
            apl_int_t liSetVal = sprintf(lpcName,"%"APL_PRIu32".",apu32Objid[i]);
            if (liSetVal < 0)
            {
                return ANF_SNMP_ERRSETV;
            }
            lpcName += liSetVal;
        }
        sprintf(lpcName,"%"APL_PRIu32,apu32Objid[aiOidNameLen-1]);
    }
    return 0;
}

apl_int_t CSNMPDecoder::NormalPduParse(
    CNormalMessage* apoMsg, 
    unsigned char*  apcBuffer, 
    apl_size_t      auMaxSize)
{
    unsigned char      lcType;
    apl_size_t         luLen = 0;
    unsigned char*     lpcVarVal = NULL;
    CVarBind*          lpoVb = NULL;
    apl_uint32_t       lau32Objid[128] = {0};

    //version
    apl_int32_t li32Version = 0;
    apcBuffer = BerParseSequence(apcBuffer, &auMaxSize, &lcType,(char)(ANF_BER_SEQUENCE));
    apcBuffer = BerParseInt(
        apcBuffer, 
        &auMaxSize, 
        &lcType, 
        &li32Version, 
        sizeof(li32Version));

    if (!apcBuffer || lcType != (char)(ANF_BER_INTEGER)) 
    {
        return ANF_SNMP_ERRINTP;
    }
    apoMsg->SetVersion(li32Version);

    //community
    char lacReadCommunity[64] = {0};
    apcBuffer = BerParseString( 
        apcBuffer, 
        &auMaxSize, 
        &lcType, 
        (char*)&lacReadCommunity, 
        sizeof(lacReadCommunity));

    if (!apcBuffer || lcType != (char)(ANF_BER_OCTET_STR))
    {
        return ANF_SNMP_ERRSTRP;
    }
    apoMsg->SetReadCommunity(lacReadCommunity);

    //Get the PDU type 
    unsigned char          lcMsgType;
    apcBuffer = BerParseHeader(apcBuffer, &auMaxSize, &lcMsgType);
    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERRHEADP;
    }

    apoMsg->SetType(lcMsgType);

    // request id 
    apl_int32_t li32RequestId = 0;
    apcBuffer = BerParseInt(
        apcBuffer, 
        &auMaxSize, 
        &lcType, 
        &li32RequestId, 
        sizeof(li32RequestId));

    if (!apcBuffer || lcType != (char)(ANF_BER_INTEGER)) 
    {
        return ANF_SNMP_ERRINTP;
    }
    apoMsg->SetRequestId(li32RequestId);

    //error status (getbulk non-repeaters) 
    apl_int32_t li32ErrStat = 0;
    apcBuffer = BerParseInt(
        apcBuffer, 
        &auMaxSize, 
        &lcType, 
        &li32ErrStat, 
        sizeof(li32ErrStat));
    if (!apcBuffer || lcType != (char)(ANF_BER_INTEGER)) 
    {
        return ANF_SNMP_ERRINTP;
    }
    apoMsg->SetErrStat(li32ErrStat);

    //error index (getbulk max-repetitions) 
    apl_int32_t li32ErrIndex = 0;
    apcBuffer = BerParseInt(
        apcBuffer, 
        &auMaxSize, 
        &lcType, 
        &li32ErrIndex, 
        sizeof(li32ErrIndex));

    if (!apcBuffer || lcType != (char)(ANF_BER_INTEGER)) 
    {
        return ANF_SNMP_ERRINTP;
    }
    apoMsg->SetErrIndex(li32ErrIndex);

    //get header for variable-bindings sequence 
    apcBuffer = BerParseSequence(apcBuffer, &auMaxSize, &lcType,(char)(ANF_BER_SEQUENCE));
    if (!apcBuffer || lcType != (char)(ANF_BER_SEQUENCE))
    {
        return ANF_SNMP_ERRSEQP;
    }

    CVarBind  loTmpVbs[ANF_SNMP_PDU_MAX_VBS];
    apl_int_t liVbCount = 0;
    for ( apl_int_t liN = 0; liN < ANF_SNMP_MAX_VARLIST_NUM; ++liN )
    {
        if (auMaxSize <= 0)
        {
            liVbCount = liN; 
            break;
        }

        lpoVb = &loTmpVbs[liN];

        apl_memset(lpoVb->macOidName,0,sizeof(lpoVb->macOidName));
        apl_memset(lpoVb->moVal.macString,0,sizeof(lpoVb->moVal.macString));
        apl_int_t liNameLen = sizeof(lau32Objid) / sizeof(apl_uint32_t);

        apcBuffer = VarBindParse(
            apcBuffer, 
            &auMaxSize, 
            lau32Objid, 
            &liNameLen, 
            &lpoVb->mcType, 
            &lpcVarVal,
            &lpoVb->miValLen);
        
        if (apcBuffer == NULL)
        {
            return ANF_SNMP_ERRVARBP;
        }

        SetVarObjid(lpoVb, lau32Objid, liNameLen);

        switch ((short) lpoVb->mcType) 
        {
        case ANF_BER_INTEGER:
            lpoVb->miValLen = sizeof(apl_int_t);
            BerParseInt(
                lpcVarVal, 
                &luLen, 
                &lpoVb->mcType,
                (apl_int32_t *)&lpoVb->moVal.mi32Integer,
                sizeof(lpoVb->moVal.mi32Integer));
            break;

        case ANF_BER_COUNTER:
        case ANF_BER_GAUGE:
        case ANF_BER_TIMETICKS:
            lpoVb->miValLen = sizeof(apl_uint32_t);
            BerParseUnsignedInt(
                lpcVarVal, 
                &luLen, 
                &lpoVb->mcType,
                (apl_uint32_t*)&lpoVb->moVal.mi32Integer,
                lpoVb->miValLen);
            break;

        case ANF_BER_OCTET_STR:
            BerParseString(
                lpcVarVal, 
                &luLen, 
                &lpoVb->mcType, 
                (char*)&lpoVb->moVal.macString,
                lpoVb->miValLen);
            break;

        case ANF_BER_IPADDRESS:
            BerParseString(
                lpcVarVal, 
                &luLen, 
                &lpoVb->mcType, 
                (char*)&lpoVb->moVal.macString,
                lpoVb->miValLen);
            break;

        case ANF_BER_OBJECT_ID:
            apl_uint32_t   lau32Objid2[ANF_SNMP_OID_NUM];
            lpoVb->miValLen = ANF_SNMP_OID_NUM;
            BerParseObjid(lpcVarVal, &luLen, &lpoVb->mcType, lau32Objid2, &lpoVb->miValLen);
            //20100516 fix, to do what?
            //if (lau32Objid2)
            {    
                char* lpcOid = lpoVb->moVal.macObjid;
                for(apl_int_t i=0;i<(lpoVb->miValLen-1);i++)
                {
                    apl_int_t liOid = sprintf( lpcOid, "%"APL_PRIu32".", lau32Objid2[i] );
                    if (liOid < 0)
                    {
                        return ANF_SNMP_ERRCOV;
                    }
                    lpcOid += liOid;
                }
                sprintf( lpcOid, "%"APL_PRIu32"", lau32Objid2[lpoVb->miValLen-1] );
            }
            break;

        case ANF_BER_NULL:
            break;

        default:
            return ANF_SNMP_ERRTYPE;
            break;
        }
    }

    apoMsg->SetVbList(loTmpVbs, liVbCount);

    return 0;
}

apl_int_t CSNMPDecoder::TrapPduParse(
    CTrapMessage*  apoMsg, 
    unsigned char* apcBuffer, 
    apl_size_t     auMaxSize)
{
    unsigned char           lcType;
    unsigned char           *lpcVarVal = NULL;
    CVarBind                *lpoVb     = NULL;
    apl_size_t              liLen      = 0;
    apl_uint32_t            lau32Objid[ANF_SNMP_OID_NUM] = {0};

    apcBuffer = BerParseSequence(apcBuffer, &auMaxSize, &lcType,(char)(ANF_BER_SEQUENCE));

    //version
    apl_int32_t li32Version = 0;
    apcBuffer = BerParseInt(
        apcBuffer, 
        &auMaxSize, 
        &lcType, 
        &li32Version, 
        sizeof(li32Version));

    if (!apcBuffer || lcType != (char)(ANF_BER_INTEGER)) 
    {
        return ANF_SNMP_ERRINTP;
    }

    apoMsg->SetVersion(li32Version);

    //community
    char lacReadCommunity[64] = {0};
    apcBuffer = BerParseString(
        apcBuffer, 
        &auMaxSize, 
        &lcType, 
        (char*)&lacReadCommunity, 
        sizeof(lacReadCommunity));

    if (!apcBuffer || lcType != (char)(ANF_BER_OCTET_STR))
    {
        return ANF_SNMP_ERRSTRP;
    }
    apoMsg->SetReadCommunity(lacReadCommunity);

    //type
    apl_int_t liType = 0;
    apcBuffer = BerParseHeader(apcBuffer, &auMaxSize, (unsigned char*)&liType);
    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERRHEADP;
    }
    apoMsg->SetType(liType);

    apl_int_t liEnterpriseLen = ANF_SNMP_OID_NUM;
    apcBuffer = BerParseObjid(apcBuffer, &auMaxSize, &lcType, lau32Objid, &liEnterpriseLen);
    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERROIDP;
    }

    //enterprise
    char lacEnterprise[ANF_SNMP_OID_NAME_LEN] = {0};
    if (lau32Objid[0] != 0)
    {    
        char* lpcOid = lacEnterprise;

        for(apl_int_t i=0;i<(liEnterpriseLen-1);i++)
        {
            apl_int_t liOid = sprintf(lpcOid,"%"APL_PRIu32".",lau32Objid[i]);
            if (liOid < 0)
            {
                return ANF_SNMP_ERRCOV;
            }
            lpcOid += liOid;
        }
        sprintf(lpcOid,"%"APL_PRIu32"",lau32Objid[liEnterpriseLen-1]);
    }
    apoMsg->SetEnterprise(lacEnterprise);

    //agent addr
    char lacAgentAddr[20] = {0};
    apl_int_t liAgeLen = 4;
    apcBuffer = BerParseAgentAddr(
        apcBuffer, 
        &auMaxSize, 
        &lcType,
        (char*)lacAgentAddr, 
        liAgeLen);

    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERRSTRP;
    }

    apoMsg->SetAgentAddr(lacAgentAddr);

    //trap type
    apl_int32_t li32TrapType = 0;
    apcBuffer = BerParseInt(
        apcBuffer, 
        &auMaxSize, 
        &lcType, 
        (apl_int32_t *)&li32TrapType, 
        sizeof(li32TrapType));
    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERRINTP;
    }
    apoMsg->SetTrapType(li32TrapType);

    //specific type
    apl_int32_t li32SpecificType = 0;
    apcBuffer =BerParseInt(
        apcBuffer, 
        &auMaxSize, 
        &lcType,
        (apl_int32_t *)&li32SpecificType,
        sizeof(li32SpecificType));
    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERRINTP;
    }
    apoMsg->SetSpecificType(li32SpecificType);

    //timestamp
    apl_uint32_t lu32Time = 0;
    apcBuffer = BerParseUnsignedInt(
        apcBuffer, 
        &auMaxSize, 
        &lcType, 
        (apl_uint32_t*)&lu32Time,
        sizeof(lu32Time));
    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERRUINTP;
    }
    apoMsg->SetTimestamp(lu32Time);

    //get header for variable-bindings sequence 
    apcBuffer = BerParseSequence(apcBuffer, &auMaxSize, &lcType,(unsigned char)(ANF_BER_SEQUENCE));
    if ( !apcBuffer || lcType != (unsigned char)(ANF_BER_SEQUENCE))
    {
        return ANF_SNMP_ERRSEQP;
    }

    apl_int_t liVbCount = 0;
    CVarBind loTmpVbs[ANF_SNMP_MAX_VARLIST_NUM];

    for(apl_int_t liN=0; liN < ANF_SNMP_MAX_VARLIST_NUM; ++liN)
    {
        if (auMaxSize <= 0)
        {
            liVbCount = liN;
            break;
        }

        lpoVb = &loTmpVbs[liN];         

        apl_memset(lpoVb->macOidName,0,sizeof(lpoVb->macOidName));
        apl_memset(lpoVb->moVal.macString,0,sizeof(lpoVb->moVal.macString));
        apl_int_t liNameLength=sizeof(lau32Objid)/sizeof(apl_uint32_t);
        apcBuffer = VarBindParse(
            apcBuffer, 
            &auMaxSize, 
            lau32Objid, 
            &liNameLength, 
            &lpoVb->mcType, 
            &lpcVarVal,
            &lpoVb->miValLen);

        if (apcBuffer == NULL)
        {
            return ANF_SNMP_ERRVARBP;
        }

        SetVarObjid(lpoVb, lau32Objid, liNameLength);

        switch ((short) lpoVb->mcType) 
        {
        case ANF_BER_INTEGER:
            lpoVb->miValLen = sizeof(apl_int_t);
            BerParseInt(
                lpcVarVal, 
                &liLen, 
                &lpoVb->mcType,
                (apl_int32_t *) &lpoVb->moVal.mi32Integer,
                sizeof(lpoVb->moVal.mi32Integer));
            break;

        case ANF_BER_COUNTER:
        case ANF_BER_GAUGE:
        case ANF_BER_TIMETICKS:
            lpoVb->miValLen = sizeof(apl_uint32_t);
            BerParseUnsignedInt(
                lpcVarVal, 
                &liLen, 
                &lpoVb->mcType,
                (apl_uint32_t *)&lpoVb->moVal.mi32Integer,
                lpoVb->miValLen);
            break;

        case ANF_BER_OCTET_STR:
            BerParseString(
                lpcVarVal, 
                &liLen, 
                &lpoVb->mcType, 
                (char*)&lpoVb->moVal.macString,
                lpoVb->miValLen);
            break;

        case ANF_BER_IPADDRESS:
            BerParseAgentAddr(
                lpcVarVal, 
                &liLen, 
                &lpoVb->mcType,
                (char*)lpoVb->moVal.macString, 
                liAgeLen);
            break;

        case ANF_BER_OBJECT_ID:
            {
                apl_uint32_t   lau32Objid2[ANF_SNMP_OID_NUM];
                apl_memset(lau32Objid2, 0, sizeof(lau32Objid2));
                lpoVb->miValLen = ANF_SNMP_OID_NUM;
                BerParseObjid(lpcVarVal, &liLen, &lpoVb->mcType, lau32Objid2, &lpoVb->miValLen);
                if (lau32Objid2[0] != 0)
                {    
                    char* lpcName = lpoVb->moVal.macObjid;

                    for(apl_int_t i=0;i<(lpoVb->miValLen-1);i++)
                    {
                        apl_int_t liOid = sprintf(lpcName,"%"APL_PRIu32".",lau32Objid2[i]);
                        if (liOid < 0)
                        {
                            return ANF_SNMP_ERRCOV;
                        }
                        lpcName += liOid;
                    }
                    sprintf(lpcName,"%"APL_PRIu32,lau32Objid2[lpoVb->miValLen-1]);
                }
                break;
            }

        case ANF_BER_NULL:
            break;

        default:
            return ANF_SNMP_ERRTYPE;
            break;
        }
    }

    apoMsg->SetVbList(loTmpVbs, liVbCount);

    return 0;
}

apl_int_t CSNMPDecoder::GetPduType(unsigned char* apcBuffer, apl_size_t auMaxSize)
{
    unsigned char           lcType;

    apcBuffer = BerParseSequence(apcBuffer, &auMaxSize, &lcType,(char)(ANF_BER_SEQUENCE));

    //version
    apl_int32_t li32Version = 0;
    apcBuffer = BerParseInt(
        apcBuffer, 
        &auMaxSize, 
        &lcType, 
        &li32Version, 
        sizeof(li32Version));

    if (!apcBuffer || lcType != (char)(ANF_BER_INTEGER)) 
    {
        return ANF_SNMP_ERRINTP;
    }

    //community
    char lacReadCommunity[64] = {0};
    apcBuffer = BerParseString(
        apcBuffer, 
        &auMaxSize, 
        &lcType, 
        (char*)&lacReadCommunity, 
        sizeof(lacReadCommunity));

    if (!apcBuffer || lcType != (char)(ANF_BER_OCTET_STR))
    {
        return ANF_SNMP_ERRSTRP;
    }

    //Get the PDU type 
    unsigned char          lcMsgType;
    apcBuffer = BerParseHeader(apcBuffer, &auMaxSize, &lcMsgType);
    if (apcBuffer == NULL)
    {
        return ANF_SNMP_ERRHEADP;
    }

    return lcMsgType;
}

/////////////////////////////[ BER Decoder ]/////////////////////////////

unsigned char* CSNMPDecoder::BerParseLength(
    unsigned char* apcBuffer, 
    apl_size_t*    apuValueLen)
{
    unsigned char lcLengthByte;

    if (!apcBuffer || !apuValueLen) 
    {
        //        printf("parse length: NULL pointer");
        return (NULL);
    }
    lcLengthByte = *apcBuffer;

    if (lcLengthByte & ANF_BER_LONG_LEN) 
    {
        lcLengthByte &= ~ANF_BER_LONG_LEN;    
        if (lcLengthByte == 0) 
        {
            return (NULL);
        }
        if ((apl_int_t)lcLengthByte > sizeof(apl_int32_t)) 
        {
            return (NULL);
        }
        apcBuffer++;            
        *apuValueLen = 0;          
        while (lcLengthByte--) 
        {
            *apuValueLen <<= 8;
            *apuValueLen |= *apcBuffer++;
        }
        return apcBuffer;
    } 
    else 
    { 
        *apuValueLen = (apl_size_t)lcLengthByte;
        return apcBuffer + 1;
    }
}

unsigned char* CSNMPDecoder::BerParseHeader(
    unsigned char* apcBuffer, 
    apl_size_t*    apuMaxSize, 
    unsigned char* apcType)
{
    unsigned char* lpcBuf = NULL;
    apl_size_t     luAsnLen = 0;

    if (!apcBuffer || !apuMaxSize || !apcType) 
    {
        //        printf("parse header: NULL pointer");
        return (NULL);
    }
    lpcBuf = apcBuffer; 
    if (ANF_IS_EXTENSION_ID(*lpcBuf)) 
    {
        //        printf("can't process ID >= 30");
        return (NULL);
    }
    *apcType = *lpcBuf++; 

    lpcBuf = BerParseLength(lpcBuf, &luAsnLen); 

    *apuMaxSize = (apl_size_t) luAsnLen;

    return lpcBuf;
}

unsigned char* CSNMPDecoder::BerParseSequence(
    unsigned char* apcBuffer, 
    apl_size_t*    apuMaxSize, 
    unsigned char* apcType, 
    unsigned char  acExpectedType)
{                                                       
    apcBuffer = BerParseHeader(apcBuffer, apuMaxSize, apcType);
    if (apcBuffer && (*apcType!= acExpectedType)) 
    {
        return (NULL);
    }
    return apcBuffer;
}

unsigned char* CSNMPDecoder::BerParseObjid(
    unsigned char* apcBuffer,
    apl_size_t*    apuMaxSize,
    unsigned char* apcType, 
    apl_uint32_t*  apu32Objid, 
    apl_int_t*     apiObjidLen)
{
    unsigned char      *lpcBuf = apcBuffer;
    apl_uint32_t       *lpu32Oid = apu32Objid + 1;
    apl_uint32_t       lu32SubIdentifier = 0;
    apl_int32_t        li32Length = 0;
    apl_size_t         luAsnLen = 0;

    *apcType = *lpcBuf++;
    lpcBuf = BerParseLength(lpcBuf, &luAsnLen);

    *apuMaxSize -= (apl_int_t) luAsnLen + (lpcBuf - apcBuffer);

    if (luAsnLen == 0)
    {
        apu32Objid[0] = apu32Objid[1] = 0;
    }
    li32Length = luAsnLen;
    (*apiObjidLen)--;          

    while (li32Length > 0 && (*apiObjidLen)-- > 0)
    {
        lu32SubIdentifier = 0;
        do 
        {                   
            lu32SubIdentifier = (lu32SubIdentifier << 7) + (*(unsigned char *) lpcBuf & ~(0x80));
            li32Length--;
        } 
        while (*(unsigned char *) lpcBuf++ & (0x80));        

        *lpu32Oid++ = lu32SubIdentifier;
    }

    if (0 != li32Length) 
    {
        //        printf("OID length exceeds buffer size");
        return NULL;
    }

    lu32SubIdentifier = (apl_uint32_t) apu32Objid[1];
    if (lu32SubIdentifier == 0x2B) 
    {
        apu32Objid[0] = 1;
        apu32Objid[1] = 3;
    } 
    else 
    {
        if (lu32SubIdentifier < 40) 
        {
            apu32Objid[0] = 0;
            apu32Objid[1] = lu32SubIdentifier;
        } 
        else if (lu32SubIdentifier < 80) 
        {
            apu32Objid[0] = 1;
            apu32Objid[1] = lu32SubIdentifier - 40;
        } 
        else 
        {
            apu32Objid[0] = 2;
            apu32Objid[1] = lu32SubIdentifier - 80;
        }
    }

    *apiObjidLen = (apl_int_t) (lpu32Oid - apu32Objid);

    return lpcBuf;
}

unsigned char* CSNMPDecoder::BerParseInt(
    unsigned char* apcBuffer,
    apl_size_t*    apuMaxSize,
    unsigned char* apcType, 
    apl_int32_t*   api32Value, 
    apl_size_t     auValueLen)
{
    unsigned char* lpcBuf = apcBuffer;
    apl_size_t     luAsnLen = 0;
    apl_int32_t    liValue = 0;

    if (auValueLen != sizeof(apl_int32_t)) 
    {
        return (NULL);
    }
    *apcType = *lpcBuf++;

    lpcBuf = BerParseLength(lpcBuf, &luAsnLen);

    if ( luAsnLen > auValueLen)
    {
        return (NULL);
    }

    *apuMaxSize -= luAsnLen + (lpcBuf - apcBuffer);

    if (*lpcBuf & 0x80)        
    {
        liValue = -1;  
    }    

    while (luAsnLen--)
    {
        liValue = (liValue << 8) | *lpcBuf++;
    }

    *api32Value = liValue;

    return lpcBuf;
}

unsigned char* CSNMPDecoder::BerParseString(
    unsigned char* apcBuffer, 
    apl_size_t*    apuMaxSize, 
    unsigned char* apcType, 
    char*          apcValue, 
    apl_size_t     auValueLen)
{
    unsigned char* lpcBuf = apcBuffer;
    apl_size_t     luAsnLen = 0;

    *apcType = *lpcBuf++;
    lpcBuf = BerParseLength(lpcBuf, &luAsnLen);

    if ( luAsnLen > auValueLen) 
    {
        return (NULL);
    }

    apl_memmove(apcValue, lpcBuf, luAsnLen);

    if ( luAsnLen > auValueLen)
    {
        apcValue[luAsnLen] = 0;   
    }    

    auValueLen = luAsnLen;

    *apuMaxSize -= luAsnLen + (apl_size_t)(lpcBuf - apcBuffer);

    return (lpcBuf + luAsnLen);
}

unsigned char* CSNMPDecoder::BerParseAgentAddr(
    unsigned char* apcBuffer, 
    apl_size_t*    apuMaxSize, 
    unsigned char* apcType, 
    char*          apcValue, 
    apl_size_t     auValueLen)
{
    unsigned char* lpcBuf = apcBuffer;
    apl_size_t     luAsnLen = 0;

    *apcType = *lpcBuf++;
    lpcBuf = BerParseLength(lpcBuf, &luAsnLen);

    if ( luAsnLen > auValueLen) 
    {
        return (NULL);
    }

    apl_int_t liCount = 0;

    liCount = sprintf(apcValue,"%d.",lpcBuf[0]);
    liCount += sprintf(apcValue + liCount,"%d.",lpcBuf[1]);
    liCount += sprintf(apcValue + liCount,"%d.",lpcBuf[2]);
    liCount += sprintf(apcValue + liCount,"%d",lpcBuf[3]);

    if ( luAsnLen > auValueLen)
    {
        apcValue[luAsnLen] = 0;   
    }    

    auValueLen = luAsnLen;

    *apuMaxSize -= luAsnLen + (apl_size_t)(lpcBuf - apcBuffer);

    return (lpcBuf + luAsnLen);
}

unsigned char* CSNMPDecoder::BerParseNull(
    unsigned char* apcBuffer, 
    apl_size_t*    apuMaxSize, 
    unsigned char* apcType)
{
    unsigned char* lpcBuf = apcBuffer;
    apl_size_t     luAsnLen = 0;

    *apcType = *lpcBuf++;

    lpcBuf = BerParseLength(lpcBuf, &luAsnLen);
    if (lpcBuf == NULL)
    {
        //        printf("parse null: bad length");
        return (NULL);
    }

    if (luAsnLen != 0) 
    {
        //        printf("parse null: malformed BER.1 null");
        return (NULL);
    }

    *apuMaxSize -= (apl_size_t)(lpcBuf - apcBuffer);

    return lpcBuf + luAsnLen;
}

unsigned char* CSNMPDecoder::BerParseUnsignedInt(
    unsigned char* apcBuffer,
    apl_size_t*    apuMaxSize,
    unsigned char* apcType, 
    apl_uint32_t*  apu32Value, 
    apl_size_t     auValueLen)
{
    unsigned char* lpcBuf = apcBuffer;
    apl_size_t     luAsnLen = 0;
    apl_uint32_t   liValue = 0;

    if (auValueLen!= sizeof(apl_uint32_t)) 
    {
        return (NULL);
    }

    *apcType= *lpcBuf++;

    lpcBuf = BerParseLength(lpcBuf, &luAsnLen);
    if ((luAsnLen > (auValueLen + 1)) || 
        ((luAsnLen == auValueLen + 1) && 
        *lpcBuf != 0x00)) 
    {
        return NULL;
    }

    *apuMaxSize -= luAsnLen + (apl_size_t)(lpcBuf - apcBuffer);

    if (*lpcBuf & 0x80)
    {
        liValue = ~liValue;        
    }

    while (luAsnLen--)
    {
        liValue = (liValue << 8) | *lpcBuf++;
    }    

    *apu32Value = liValue;

    return lpcBuf;
}
} //namespace snmp

ANF_NAMESPACE_END


