
#include "AISMPPStru.h"

///start namespace
AIBC_NAMESPACE_START

class AICDRBuffer
{
public:
    AICDRBuffer(void* apPtr, size_t aiLen) :cpcPtr((char*)apPtr), ciSize(aiLen), ciOffset(0) {}

    size_t GetSize() const
    {
        return ciSize;
    }

    size_t GetOffset() const
    {
        return ciOffset;
    }

    size_t GetLength() const
    {
        assert(ciSize >= ciOffset);

        return ciSize - ciOffset;
    }

    void const* GetBasePtr() const
    {
        return cpcPtr;
    }

    void* GetBasePtr()
    {
        return cpcPtr;
    }

    void const* GetCurPtr() const
    {
        return cpcPtr + ciOffset;
    }

    void* GetCurPtr()
    {
        return cpcPtr + ciOffset;
    }

    int MovePtr(ssize_t aiOff)
    {
        ssize_t liOff = aiOff + ciOffset;
        AI_RETURN_IF(-1, liOff < 0);
        AI_RETURN_IF(-2, (size_t)liOff > ciSize);

        ciOffset = (size_t)liOff;
        return 0;
    }

    void ResetPtr()
    {
        ciOffset = 0;
    }

protected:
    char*   cpcPtr;
    size_t  ciSize;
    size_t  ciOffset;
};


class AICDRWriter :public AICDRBuffer
{
public:
    AICDRWriter(void* apPtr, size_t aiLen) :AICDRBuffer(apPtr, aiLen) {}

    int WriteStr(char const* apcStr)
    {
        size_t liLen = strlen(apcStr) + 1;
        AI_RETURN_IF(-1, liLen > GetLength());
         
        memcpy(GetCurPtr(), apcStr, liLen);
        MovePtr(liLen);
        
        return 0;
    }


    int WriteMem(void const* apPtr, size_t aiLen)
    {
        AI_RETURN_IF(-1, aiLen > GetLength());

        memcpy(GetCurPtr(), apPtr, aiLen);
        MovePtr(aiLen);

        return 0;
    }


    int WriteI32(int32_t aiN)
    {
        AI_RETURN_IF(-1, sizeof(aiN) > GetLength());

        aiN = htonl(aiN);
        memcpy(GetCurPtr(), &aiN, sizeof(aiN));
        MovePtr(sizeof(aiN));

        return 0;
    }


    int WriteU32(uint32_t aiN)
    {
        AI_RETURN_IF(-1, sizeof(aiN) > GetLength());

        aiN = htonl(aiN);
        memcpy(GetCurPtr(), &aiN, sizeof(aiN));
        MovePtr(sizeof(aiN));

        return 0;
    }


    int WriteI16(int16_t aiN)
    {
        AI_RETURN_IF(-1, sizeof(aiN) > GetLength());

        aiN = htons(aiN);
        memcpy(GetCurPtr(), &aiN, sizeof(aiN));
        MovePtr(sizeof(aiN));

        return 0;
    }


    int WriteU16(uint16_t aiN)
    {
        AI_RETURN_IF(-1, sizeof(aiN) > GetLength());

        aiN = htons(aiN);
        memcpy(GetCurPtr(), &aiN, sizeof(aiN));
        MovePtr(sizeof(aiN));

        return 0;
    }


    int WriteI8(int8_t aiN)
    {
        AI_RETURN_IF(-1, sizeof(aiN) > GetLength());

        memcpy(GetCurPtr(), &aiN, sizeof(aiN));
        MovePtr(sizeof(aiN));

        return 0;
    }


    int WriteU8(uint8_t aiN)
    {
        AI_RETURN_IF(-1, sizeof(aiN) > GetLength());

        memcpy(GetCurPtr(), &aiN, sizeof(aiN));
        MovePtr(sizeof(aiN));

        return 0;
    }
};


class AICDRReader :public AICDRBuffer
{
public:
    AICDRReader(void const* apPtr, size_t aiLen) :AICDRBuffer((void*)apPtr, aiLen) {}

    int ReadStr(char* apcStr, size_t aiLen) 
    {
        char const* lpcEnd = (char const*)memchr(GetCurPtr(), '\0', GetLength());
        AI_RETURN_IF(-1, NULL == lpcEnd);

        size_t liLen = lpcEnd - (char const*)GetCurPtr() + 1;
        AI_RETURN_IF(-2, aiLen < liLen);

        memcpy(apcStr, GetCurPtr(), liLen);
        MovePtr(liLen);

        return 0;
    }


    int ReadMem(void* apPtr, size_t aiLen)
    {
        AI_RETURN_IF(-1, aiLen > GetLength());

        memcpy(apPtr, GetCurPtr(), aiLen);
        MovePtr(aiLen);

        return 0;
    }


    int ReadI32(int32_t& aiN)
    {
        AI_RETURN_IF(-1, sizeof(aiN) > GetLength());

        memcpy(&aiN, GetCurPtr(), sizeof(aiN));
        aiN = ntohl(aiN);
        MovePtr(sizeof(aiN));

        return 0;
    }


    int ReadU32(uint32_t& aiN)
    {
        AI_RETURN_IF(-1, sizeof(aiN) > GetLength());

        memcpy(&aiN, GetCurPtr(), sizeof(aiN));
        aiN = ntohl(aiN);
        MovePtr(sizeof(aiN));

        return 0;
    }


    int ReadI16(int16_t& aiN)
    {
        AI_RETURN_IF(-1, sizeof(aiN) > GetLength());

        memcpy(&aiN, GetCurPtr(), sizeof(aiN));
        aiN = ntohs(aiN);
        MovePtr(sizeof(aiN));

        return 0;
    }


    int ReadU16(uint16_t& aiN)
    {
        AI_RETURN_IF(-1, sizeof(aiN) > GetLength());

        memcpy(&aiN, GetCurPtr(), sizeof(aiN));
        aiN = ntohs(aiN);
        MovePtr(sizeof(aiN));

        return 0;
    }


    int ReadI8(int8_t& aiN)
    {
        AI_RETURN_IF(-1, sizeof(aiN) > GetLength());

        memcpy(&aiN, GetCurPtr(), sizeof(aiN));
        MovePtr(sizeof(aiN));

        return 0;
    }


    int ReadU8(uint8_t& aiN)
    {
        AI_RETURN_IF(-1, sizeof(aiN) > GetLength());

        memcpy(&aiN, GetCurPtr(), sizeof(aiN));
        MovePtr(sizeof(aiN));

        return 0;
    }
};

#define BW()            AICDRWriter loWriter(apPtr, aiLen)
#define EW()            return loWriter.GetSize() - loWriter.GetLength()

#define BR()            AICDRReader loReader(apPtr, aiLen)
#define ER()            return loReader.GetSize() - loReader.GetLength()

#define W_(type, ...)   AI_RETURN_IF(-loWriter.GetOffset() - 1, loWriter.Write##type(__VA_ARGS__) < 0)
#define WS(type, field) W_(type, field, sizeof(field))


#define R_(type, ...)   AI_RETURN_IF(-loReader.GetOffset() - 1, loReader.Read##type(__VA_ARGS__) < 0)
#define RS(type, field) R_(type, field, sizeof(field))

////////////////////////////////////////////////////////////////////////////
ssize_t SMPP_HEADER::Encode(void* apPtr, size_t aiLen) const
{
    BW();

    W_(U32, command_length);
    W_(U32, command_id);
    W_(U32, command_status);
    W_(U32, sequence_number);

    EW();
}

ssize_t SMPP_HEADER::Decode(void const* apPtr, size_t aiLen)
{
    BR();

    R_(U32, command_length);
    R_(U32, command_id);
    R_(U32, command_status);
    R_(U32, sequence_number);

    ER();
}

////////////////////////////////////////////////////////////////////////////
ssize_t SMPP_BIND::Encode(void* apPtr, size_t aiLen) const
{
    BW();

    W_(Str, system_id);
    W_(Str, password);
    W_(Str, system_type)
    W_(U8, interface_version);
    W_(U8, addr_ton);
    W_(U8, addr_npi);
    W_(Str,address_range);

    EW();
}

ssize_t SMPP_BIND::Decode(void const* apPtr, size_t aiLen)
{
    BR();

    RS(Str, system_id);
    RS(Str, password);
    RS(Str, system_type)
    R_(U8, interface_version);
    R_(U8, addr_ton);
    R_(U8, addr_npi);
    RS(Str,address_range);

    ER();
}

////////////////////////////////////////////////////////////////////////////
ssize_t SMPP_BIND_RESP::Encode(void* apPtr, size_t aiLen) const
{
    BW();

    W_(Str, system_id);

    EW();
}

ssize_t SMPP_BIND_RESP::Decode(void const* apPtr, size_t aiLen)
{
    BR();

    RS(Str, system_id);

    ER();
}

////////////////////////////////////////////////////////////////////////////
ssize_t SMPP_SM::Encode(void* apPtr, size_t aiLen) const
{
    BW();

    W_(Str, service_type);
    W_(U8, source_addr_ton);
    W_(U8, source_addr_npi);
    W_(Str, source_addr);
    W_(U8, dest_addr_ton);
    W_(U8, dest_addr_ton);
    W_(Str, destination_addr);
    W_(U8, esm_class);
    W_(U8, protocol_id);
    W_(U8, priority_flag);
    W_(Str, schedule_delivery_time);
    W_(Str, validity_period);
    W_(U8, registered_delivery);
    W_(U8, replace_if_present_flag);
    W_(U8, data_coding);
    W_(U8, sm_default_msg_id);
    W_(U8, sm_length);
    W_(Mem, short_message, sm_length);

    EW(); 
}

ssize_t SMPP_SM::Decode(void const* apPtr, size_t aiLen)
{
    BR();

    RS(Str, service_type);
    R_(U8, source_addr_ton);
    R_(U8, source_addr_npi);
    RS(Str, source_addr);
    R_(U8, dest_addr_ton);
    R_(U8, dest_addr_ton);
    RS(Str, destination_addr);
    R_(U8, esm_class);
    R_(U8, protocol_id);
    R_(U8, priority_flag);
    RS(Str, schedule_delivery_time);
    RS(Str, validity_period);
    R_(U8, registered_delivery);
    R_(U8, replace_if_present_flag);
    R_(U8, data_coding);
    R_(U8, sm_default_msg_id);
    R_(U8, sm_length);
    R_(Mem, short_message, sm_length);

    ER(); 
}


////////////////////////////////////////////////////////////////////////////
ssize_t SMPP_SUBMIT_SM_RESP::Encode(void* apPtr, size_t aiLen) const
{
    BW();

    if (0 != coHeader.command_status)
    {
        // Note: The submit_sm_resp PDU Body is not returned 
        //       if the command_status field contains a non-zero value.
    }
    else
    {
        W_(Str, message_id);
    }

    EW();
}

ssize_t SMPP_SUBMIT_SM_RESP::Decode(void const* apPtr, size_t aiLen)
{
    BR();

    if (0 != coHeader.command_status)
    {
        // Note: The submit_sm_resp PDU Body is not returned 
        //       if the command_status field contains a non-zero value.
    }
    else
    {
        RS(Str, message_id);
    }

    ER();
}

////////////////////////////////////////////////////////////////////////////
ssize_t SMPP_DELIVER_SM_RESP::Encode(void* apPtr, size_t aiLen) const
{
    BW();

    W_(Str, message_id);

    EW();
}

ssize_t SMPP_DELIVER_SM_RESP::Decode(void const* apPtr, size_t aiLen)
{
    BR();

    RS(Str, message_id);

    ER();
}


////////////////////////////////////////////////////////////////////////////
AITime_t _ParseAbsSMPPTime(char const* apcSMPPTime, char acSign)
{
    struct tm loTm;
    char const* lpcEnd;
    int liTenths;
    int liQuarters; 
    int liRetCode;

    memset(&loTm, 0, sizeof(loTm));

    lpcEnd = strptime(apcSMPPTime, "%y%m%d%H%M%S", &loTm);

    AI_RETURN_IF(-1,  (NULL == lpcEnd || (lpcEnd - apcSMPPTime) != 12));

    liRetCode = sscanf(lpcEnd, "%01d%02d", &liTenths, &liQuarters);

    AI_RETURN_IF(-1, (liRetCode < 0 || liRetCode != 2));
    AI_RETURN_IF(-1, (liTenths < 0 || liTenths > 9));
    AI_RETURN_IF(-1, (liQuarters < 0 || liQuarters > 48));

    return mktime(&loTm) * AI_TIME_SEC 
           + AIGetTimeZone() 
           + liTenths * AI_TIME_SEC / 10
           - (acSign == '-' ? -1 : 1) * liQuarters * 15 * 60 * AI_TIME_SEC ;

    return -1;
}


////////////////////////////////////////////////////////////////////////////
AITime_t _ParseRelSMPPTime(char const* apcSMPPTime, AITime_t aiCurTime)
{
    struct tm loTM;
    int liYear;
    int liMonth;
    int liDay;
    int liHour;
    int liMinute;
    int liSecond;
    int liRetCode;
    void* lpRetPtr;
    time_t liTime;

    liTime = aiCurTime / AI_TIME_SEC;

    liRetCode = sscanf(
        apcSMPPTime,
        "%02d%02d%02d%02d%02d%02d000R", 
        &liYear,
        &liMonth,
        &liDay,
        &liHour,
        &liMinute,
        &liSecond
    );
  
    AI_RETURN_IF(-1, (6 != liRetCode));
    AI_RETURN_IF(-1, (liYear < 0));
    AI_RETURN_IF(-1, (liMonth < 0));
    AI_RETURN_IF(-1, (liDay < 0));
    AI_RETURN_IF(-1, (liHour < 0));
    AI_RETURN_IF(-1, (liMinute < 0));
    AI_RETURN_IF(-1, (liSecond < 0));

    liTime += liSecond + liMinute * 60 + liHour * 60 * 60 + liDay * 60 * 60 * 24;

    lpRetPtr = gmtime_r(&liTime, &loTM);

    AI_RETURN_IF(-1, (NULL == lpRetPtr));

    loTM.tm_year += liYear + (loTM.tm_mon + liMonth) / 12;
    loTM.tm_mon = (loTM.tm_mon + liMonth) % 12;

    return mktime(&loTM) * AI_TIME_SEC + AIGetTimeZone(); 
}

////////////////////////////////////////////////////////////////////////////
AITime_t AIParseSMPPTime(char const* apcSMPPTime, AITime_t aiCurTime)
{
    char lcType;

    if (strlen(apcSMPPTime) != 16)
    {    
        return -1;
    }

    lcType = apcSMPPTime[15];

    if ('R' == lcType)
    {
        return _ParseRelSMPPTime(apcSMPPTime, aiCurTime);
    }
    else if('-' == lcType || '+' == lcType)
    {
        return _ParseAbsSMPPTime(apcSMPPTime, lcType);
    }

    return -1;
}

////////////////////////////////////////////////////////////////////////////
int AIFormatAbsSMPPTime(char* apcSMPPTime, size_t aiSize, AITime_t aiTime, AITime_t aiTimeZone)
{
    struct tm   loTM;
    time_t      liTime;
    int         liTenths;
    int         liQuarters; 
    char        lcType;
    void*       lpRetPtr;
    int         liRetCode;

    AI_RETURN_IF(-1, (aiSize < 17));
   
    liTime = (aiTime + aiTimeZone) /AI_TIME_SEC; 

    lpRetPtr = gmtime_r(&liTime, &loTM);
    AI_RETURN_IF(-1, (NULL == lpRetPtr));

    liTenths = (aiTime / (AI_TIME_SEC / 10)) % 10;

    if (aiTimeZone < 0)
    {
        lcType = '-';
        liQuarters = (-aiTimeZone) / (AI_TIME_SEC * 60 * 15);
    }
    else
    {
        lcType = '+';
        liQuarters = aiTimeZone / (AI_TIME_SEC * 60 * 15);
    }
    
    liRetCode = strftime(apcSMPPTime, aiSize, "%y%m%d%H%M%S", &loTM);

    AI_RETURN_IF(-1, (12 != liRetCode));

    liRetCode = snprintf(apcSMPPTime + 12, aiSize - 12, "%d%02d%c", liTenths, liQuarters, lcType);

    AI_RETURN_IF(-1, (liRetCode < 0));

    return 0;
}

///end namespace
AIBC_NAMESPACE_END

