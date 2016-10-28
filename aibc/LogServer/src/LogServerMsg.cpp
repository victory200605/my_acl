#include "apl/inttypes.h"
#include "LogServerMsg.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"
#include "acl/Utility.h"


#define LOG_SERVER_TLVITEMS_RESERVE     1024


class CLogServerMsgImpl
{
public:

    struct CTLVFormat
    {
        apl_uint16_t muTag;
        std::string moValue;
    };

    typedef std::vector<CTLVFormat> CTLVItems;

    CLogServerMsgImpl()
    {
        moTLVItems.reserve(LOG_SERVER_TLVITEMS_RESERVE);
    }

    ~CLogServerMsgImpl() {}

    apl_int_t AddTagValue(apl_uint16_t auTag, char* apcValue, apl_uint16_t auLen)
    {
        if (apcValue == APL_NULL)
            return -1;

        CTLVFormat loTLV;

        loTLV.muTag = auTag;
        loTLV.moValue.append(apcValue, 0, auLen);

        this->moTLVItems.push_back(loTLV);

        //return the index
        return this->moTLVItems.size() - 1 ;
    }

    apl_int_t SetTagValue(apl_int_t aiIdx, apl_uint16_t auTag, char* apcValue, apl_uint16_t auLen)
    {
        //invalid index
        if ((apl_size_t)aiIdx >= this->moTLVItems.size())
            return -1;

        this->moTLVItems[aiIdx].muTag = auTag;
        this->moTLVItems[aiIdx].moValue.clear();
        this->moTLVItems[aiIdx].moValue.append(apcValue, 0, auLen);

        return aiIdx;
    }

    void DelTagValue(apl_int_t aiIdx)
    {
        //invalid index
        if ((apl_size_t)aiIdx >= this->moTLVItems.size())
            return;

        std::vector<CTLVFormat>::iterator pos;
        apl_int_t liIndex = 0;

        //do nothing but get the pos
        for (pos = this->moTLVItems.begin(), liIndex = 0;
                pos != this->moTLVItems.end() && liIndex < aiIdx;
                ++pos, ++liIndex);


        if (pos != this->moTLVItems.end())
        {
            this->moTLVItems.erase(pos);
        }
    }

    apl_int_t GetSize()
    {
        return this->moTLVItems.size();
    }

    apl_int_t GetTLV(apl_int_t aiIdx, apl_uint16_t* apuTag, apl_uint16_t* apuLen, void** appvValue)
    {
        //invalid index
        if ((apl_size_t)aiIdx >= this->moTLVItems.size())
            return -1;

        if (apuTag != APL_NULL)
            *apuTag = this->moTLVItems[aiIdx].muTag;
        if (apuLen != APL_NULL)
            *apuLen = this->moTLVItems[aiIdx].moValue.size();
        if (appvValue != APL_NULL)
            *appvValue = (void *)this->moTLVItems[aiIdx].moValue.c_str();

        return aiIdx;
    }

    apl_int_t Search(apl_uint16_t auTag, apl_int_t aiBegin)
    {
        apl_int_t liIdx = 0;
        apl_int_t liEnd = this->moTLVItems.size();

        for (liIdx = aiBegin; liIdx < liEnd; ++liIdx)
        {
            if (this->moTLVItems[liIdx].muTag == auTag)
                break;
        }

        if (liIdx >= liEnd)
            liIdx = -1;

        return liIdx;
    }

    void Clear()
    {
        this->moTLVItems.clear();
    }


private:

    CTLVItems moTLVItems;

};


CLogServerMsg::CLogServerMsg ()
{
    ACL_NEW_ASSERT(this->mpoLogServerMsgImpl, CLogServerMsgImpl);
}

CLogServerMsg::~CLogServerMsg ()
{
    ACL_DELETE(this->mpoLogServerMsgImpl);
}


int CLogServerMsg::AddTagValue(unsigned short auTag, char* apcValue, unsigned short auLen)
{
    return this->mpoLogServerMsgImpl->AddTagValue(auTag, apcValue, auLen);
}

int CLogServerMsg::SetTagValue(int aiIdx, unsigned short auTag, char* apcValue, unsigned short auLen)
{
    return this->mpoLogServerMsgImpl->SetTagValue(aiIdx, auTag, apcValue, auLen);
}

void CLogServerMsg::DelTagValue(int aiIdx)
{
    return this->mpoLogServerMsgImpl->DelTagValue(aiIdx);
}

int CLogServerMsg::GetSize()
{
    return this->mpoLogServerMsgImpl->GetSize();
}

int CLogServerMsg::GetTLV(int aiIdx, unsigned short* apuTag, unsigned short* apuLen, void** appvValue)
{
    return this->mpoLogServerMsgImpl->GetTLV(aiIdx, apuTag, apuLen, appvValue);
}

int CLogServerMsg::Search(unsigned short auTag, int aiBegin)
{
    return this->mpoLogServerMsgImpl->Search(auTag, aiBegin);
}

void CLogServerMsg::Clear()
{
    return this->mpoLogServerMsgImpl->Clear();
}
