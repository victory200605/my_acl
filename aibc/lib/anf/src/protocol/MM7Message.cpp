
/**
 * @file  MM7Message.cpp
 */

#include "anf/protocol/MM7Message.h"

ANF_NAMESPACE_START

namespace mm7
{
/***************************************************************
                         CMM7Content
***************************************************************/
/**
 * CMM7ContentType
 */

CMM7ContentType::CMM7ContentType(void)
    : moPrimaryType("")
    , moSubType("")
{
}

CMM7ContentType::CMM7ContentType(std::string aoType)
{
    apl_size_t auPos = 0;

    auPos = aoType.find_first_of("/");
    if( auPos != std::string::npos )
    {
        this->moPrimaryType = aoType.substr(0, auPos);

        std::string loTmpType = aoType.substr(auPos + 1);
        auPos = loTmpType.find_first_of(";");
        if(auPos != std::string::npos)
        {
            this->moSubType = loTmpType.substr(0, auPos);
        }
        else
        {
            this->moSubType = loTmpType;
        }
    }
    else
    {
        //FIXME: invalid content-type
    }
}

CMM7ContentType::~CMM7ContentType(void)
{
}

bool CMM7ContentType::operator==(std::string aoType)
{
    std::string loTmp(this->moPrimaryType + "/" + this->moSubType);
    return loTmp == aoType;
}

/**
 * CMM7Content
 */
CMM7Content::CMM7Content(void)
    : mpoContentType(NULL)
    , moCharset("")
    , moContentTransferEncoding("")
    , moContentID("")
    , moContentLocation("")
    , mpoContentBuffer(NULL)
    , mbIsMultipart(false)
    , mbIsBase64Encode(false)
{
}

CMM7Content::CMM7Content(char* apcBuffer)
    : mpoContentType(NULL)
    , moCharset("UTF-8")
    , moContentTransferEncoding("")
    , moContentID("")
    , moContentLocation("")
    , mbIsMultipart(false)
    , mbIsBase64Encode(false)
{
    apl_size_t luBufferSize = apl_strlen(apcBuffer) + 1;
    ACL_NEW_ASSERT(mpoContentBuffer, acl::CMemoryBlock(luBufferSize));
    this->mpoContentBuffer->Write(apcBuffer);
}

CMM7Content::CMM7Content(char* apcBuffer, apl_size_t auBufferSize)
    : mpoContentType(NULL)
    , moCharset("UTF-8")
    , moContentTransferEncoding("")
    , moContentID("")
    , moContentLocation("")
    , mbIsMultipart(false)
    , mbIsBase64Encode(false)
{
    ACL_NEW_ASSERT(mpoContentBuffer, acl::CMemoryBlock(auBufferSize));
    this->mpoContentBuffer->Write(apcBuffer, auBufferSize);
}

CMM7Content::~CMM7Content(void)
{
    ACL_DELETE(this->mpoContentType);
    ACL_DELETE(this->mpoContentBuffer);
    if(this->mbIsMultipart)
    {
        ContentListType::iterator loIter;
        for(loIter = this->moSubContents.begin(); loIter != this->moSubContents.end(); ++loIter)
        {
            ACL_DELETE(*loIter);
        }
    }
}

void CMM7Content::AddSubContent(CMM7Content* apoContent)
{
    this->moSubContents.push_back(apoContent);
    this->mbIsMultipart = true;
}

CMM7Content* CMM7Content::GetSubContentByID(std::string aoContentID)
{
    ContentListType::iterator loIter;
    for(loIter = this->moSubContents.begin(); loIter != this->moSubContents.end(); ++loIter)
    {
        if(aoContentID == (*loIter)->GetContentID())
        {
            return *loIter;
        }
    }

    return NULL;
}

CMM7Content* CMM7Content::GetSubContentByLocation(std::string aoContentLocation)
{
    ContentListType::iterator loIter;
    for(loIter = this->moSubContents.begin(); loIter != this->moSubContents.end(); ++loIter)
    {
        if(aoContentLocation == (*loIter)->GetContentID())
        {
            return *loIter;
        }
    }

    return NULL;

}

CMM7Content::ContentListType* CMM7Content::GetSubContents(void)
{
    return &this->moSubContents;
}

acl::CMemoryBlock* CMM7Content::GetContentBuffer(void)
{
    return this->mpoContentBuffer;
}

void CMM7Content::SetContentBuffer(acl::CMemoryBlock* apoBuffer)
{
    this->mpoContentBuffer = apoBuffer;
}

void CMM7Content::SetContentType(CMM7ContentType* apoType)
{
    this->mpoContentType = apoType;
}

void CMM7Content::SetContentType(std::string aoType)
{
    CMM7ContentType* lpoType = NULL;
    ACL_NEW_ASSERT(lpoType, CMM7ContentType(aoType));

    this->mpoContentType = lpoType;
}

CMM7ContentType* CMM7Content::GetContentType(void)
{
    return this->mpoContentType;
}

/***************************************************************
                         CSmilGenerator
***************************************************************/
/**
 * CSmilGenerator
 */
CSmilGenerator::CSmilGenerator(void)
    : moWidth("352")
    , moHeight("144")
    , mpcSmilBuffer(NULL)
{
}

CSmilGenerator::~CSmilGenerator(void)
{
    ACL_FREE(this->mpcSmilBuffer);
}

apl_int_t CSmilGenerator::SmilGenerator(const char** appcSmil)
{
    this->SmilBuildHead();
    this->SmilBuildBody();

    acl::CXmlCreator loCreator;
    loCreator.Create(this->moRootElement, acl::CXmlCreator::OPT_UNFORMAT);

    apl_size_t luSmilSize = apl_strlen(loCreator.GetXml()) + 1;
    ACL_MALLOC_ASSERT(this->mpcSmilBuffer, char, luSmilSize);

    apl_strncpy(this->mpcSmilBuffer, loCreator.GetXml(), luSmilSize);
    *appcSmil = this->mpcSmilBuffer;

    return 0;
}

apl_int_t CSmilGenerator::SmilBuildHead(void)
{
    char lacTmpBuf[128] = {0};

    this->moRootElement.SetName("smil");
    this->moRootElement.AddAttribute("xmlns", "http://www.w3.org/2000/SMIL20/CR/Language");

    acl::CXmlElement::IteratorType loElemIterHeader;
    loElemIterHeader = this->moRootElement.AddElement("head");

    acl::CXmlElement::IteratorType loElemIterLayout;
    for(MetaListType::iterator loIter = this->moMetaList.begin(); 
            loIter != this->moMetaList.end(); ++loIter)
    {
        loElemIterLayout = loElemIterHeader->AddElement("meta");
        apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIter->moName.c_str());
        loElemIterLayout->AddAttribute("name", lacTmpBuf);
        apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIter->moContent.c_str());
        loElemIterLayout->AddAttribute("content", lacTmpBuf);
    }

    loElemIterLayout = loElemIterHeader->AddElement("layout");

    acl::CXmlElement::IteratorType loElemIter;
    loElemIter = loElemIterLayout->AddElement("root-layout");
    apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", this->moWidth.c_str());
    loElemIter->AddAttribute("width", lacTmpBuf);
    apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", this->moHeight.c_str());
    loElemIter->AddAttribute("height", lacTmpBuf);

    for(RegionListType::iterator loIter = this->moRegionList.begin(); 
            loIter != this->moRegionList.end(); ++loIter)
    {
        loElemIter = loElemIterLayout->AddElement("region");
        apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIter->moId.c_str());
        loElemIter->AddAttribute("id", lacTmpBuf);
        apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIter->moWidth.c_str());
        loElemIter->AddAttribute("width", lacTmpBuf);
        apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIter->moHeight.c_str());
        loElemIter->AddAttribute("height", lacTmpBuf);
        apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIter->moLeft.c_str());
        loElemIter->AddAttribute("left", lacTmpBuf);
        apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIter->moTop.c_str());
        loElemIter->AddAttribute("top", lacTmpBuf);
        apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIter->moFit.c_str());
        loElemIter->AddAttribute("fit", lacTmpBuf);
        
    }

    return 0;
}

void CSmilGenerator::AddMeta(CMeta& aoMeta)
{
    this->moMetaList.push_back(aoMeta);
}

void CSmilGenerator::AddRegion(CRegion& aoRegion)
{
    this->moRegionList.push_back(aoRegion);
}

void CSmilGenerator::AddFrame(CFrame& aoFrame)
{
    this->moFrameList.push_back(aoFrame);
}

void CSmilGenerator::AddFrameArray(CFrame* apoFrame, apl_size_t auFrameSize)
{
    for(apl_size_t liN =0; liN<auFrameSize; ++liN)
    {
        if(apoFrame + liN == APL_NULL)
        {
            break;
        }

        this->moFrameList.push_back(apoFrame[liN]);
    }
}

acl::CXmlRootElement* CSmilGenerator::GetRootElement(void)
{
    return &this->moRootElement;
}

/**
 * CSmilMusicShare
 */
apl_int_t CSmilMusicShare::SmilBuildBody(void)
{
    char lacTmpBuf[128] = {0};

    acl::CXmlRootElement* lpoRootElement = this->GetRootElement();
    acl::CXmlElement::IteratorType loElemIterBody;

    loElemIterBody = lpoRootElement->AddElement("body");

    acl::CXmlElement::IteratorType loElemIterPar;
    loElemIterPar = loElemIterBody->AddElement("par");

    //Add Seq
    acl::CXmlElement::IteratorType loElemIterSeq;
    loElemIterSeq = loElemIterPar->AddElement("seq");

    acl::CXmlElement::IteratorType loElemIterParInner;
    for(FrameListType::iterator loIter = this->moFrameList.begin(); 
            loIter != this->moFrameList.end(); ++loIter)
    {
        acl::CXmlElement::IteratorType loElemIterNode;

        loElemIterParInner = loElemIterSeq->AddElement("par");
        
        apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIter->moDur.c_str());
        loElemIterParInner->AddAttribute("dur", lacTmpBuf);

        for(NodeListType::iterator loIterNode = loIter->moNodeList.begin();
                loIterNode != loIter->moNodeList.end(); ++loIterNode)
        {
            do
            {
                if(loIterNode->moRegion == "Image")
                {
                    loElemIterNode = loElemIterParInner->AddElement("img");
                    apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIterNode->moSrc.c_str());
                    loElemIterNode->AddAttribute("src", lacTmpBuf);
                    loElemIterNode->AddAttribute("region", "\"Image\"");

                    break; 
                }
                
                if(loIterNode->moRegion == "Text")
                {
                    loElemIterNode = loElemIterParInner->AddElement("text");
                    apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIterNode->moSrc.c_str());
                    loElemIterNode->AddAttribute("src", lacTmpBuf);
                    loElemIterNode->AddAttribute("region", "\"Text\"");
                    break; 
                }

            }while(false);
        }
    }

    //Add Audio
    acl::CXmlElement::IteratorType loElemIterMusic;
    loElemIterMusic = loElemIterPar->AddElement("audio");
    apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", this->moShareMusic.moSrc.c_str());
    loElemIterMusic->AddAttribute("src", lacTmpBuf);

    return 0;
}

void CSmilMusicShare::SetShareMusic(CFrame::CNode& aoShareMusic)
{
    this->moShareMusic = aoShareMusic;
}

/**
 * CSmilMusicIndependence
 */
apl_int_t CSmilMusicIndependence::SmilBuildBody(void)
{
    char lacTmpBuf[128] = {0};

    acl::CXmlRootElement* lpoRootElement = this->GetRootElement();
    acl::CXmlElement::IteratorType loElemIterBody;

    loElemIterBody = lpoRootElement->AddElement("body");

    acl::CXmlElement::IteratorType loElemIterPar;
    for(FrameListType::iterator loIter = this->moFrameList.begin(); 
            loIter != this->moFrameList.end(); ++loIter)
    {
        acl::CXmlElement::IteratorType loElemIterNode;

        loElemIterPar = loElemIterBody->AddElement("par");
        
        apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIter->moDur.c_str());
        loElemIterPar->AddAttribute("dur", lacTmpBuf);

        for(NodeListType::iterator loIterNode = loIter->moNodeList.begin();
                loIterNode != loIter->moNodeList.end(); ++loIterNode)
        {
            do
            {
                if(loIterNode->moRegion == "Image")
                {
                    loElemIterNode = loElemIterPar->AddElement("img");
                    apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIterNode->moSrc.c_str());
                    loElemIterNode->AddAttribute("src", lacTmpBuf);
                    loElemIterNode->AddAttribute("region", "\"Image\"");

                    break; 
                }
                
                if(loIterNode->moRegion == "Text")
                {
                    loElemIterNode = loElemIterPar->AddElement("text");
                    apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIterNode->moSrc.c_str());
                    loElemIterNode->AddAttribute("src", lacTmpBuf);
                    loElemIterNode->AddAttribute("region", "\"Text\"");
                    break; 
                }

                if(loIterNode->moRegion == "")
                {
                    loElemIterNode = loElemIterPar->AddElement("audio");
                    apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", loIterNode->moSrc.c_str());
                    loElemIterNode->AddAttribute("src", lacTmpBuf);
                    break; 
                }

            }while(false);
        }
    }
    return 0;
}

/***************************************************************
                         CMM7Message 
***************************************************************/
CMM7Message::CMM7Message(void)
    : moTransactionID("")
    , moMM7Version(_3GPP_MMS_VERSION)
    , miMessageType(0)
{
}

CMM7Message::CMM7Message(apl_int_t aiMessageType)
    : moTransactionID("")
    , moMM7Version(_3GPP_MMS_VERSION)
    , miMessageType(aiMessageType)
{
}

CMM7Message::~CMM7Message(void)
{
}

apl_ssize_t CMM7Message::Encode(acl::CMemoryBlock* apoBuffer)
{
    apl_int_t liRet = -1;

    do 
    {  
        //CMM7VASPReq
        CMM7VASPReq* lpoVASPReq = APL_NULL; 
        if( (lpoVASPReq = dynamic_cast<CMM7VASPReq*>(this)) != APL_NULL)
        {
            liRet = this->EncodeVASPReq(apoBuffer);
            break;
        }

        //CMM7VASPRes
        CMM7VASPRes* lpoVASPRes = APL_NULL;
        if( (lpoVASPRes = dynamic_cast<CMM7VASPRes*>(this)) != APL_NULL)
        {
            liRet = this->EncodeVASPRes(apoBuffer);     
            break;
        }

        //CMM7RSReq
        CMM7RSReq* lpoRSReq = APL_NULL;
        if( (lpoRSReq = dynamic_cast<CMM7RSReq*>(this)) != APL_NULL)
        {
            liRet = this->EncodeRSReq(apoBuffer);
            break;
        }

        return MM7_ERR_INVALIDMESSAGE;

    }while(false);

    return liRet;
}

apl_ssize_t CMM7Message::Decode(const acl::CMemoryBlock* apoBuffer)
{
    return 0;
}

apl_ssize_t CMM7Message::EncodeVASPReq(acl::CMemoryBlock* apoBuffer)
{
    apl_int_t liRet = -1;
    
    MM7_RETURN_IF(liRet, (liRet = this->EncodeReqNormalPart(apoBuffer)) != 0);
    MM7_RETURN_IF(liRet, (liRet = this->EncodeVASPReqSoapXML(apoBuffer)) != 0);
    MM7_RETURN_IF(liRet, (liRet = this->EncodeVASPReqContentIfHas(apoBuffer)) != 0);

    return 0;
}

apl_ssize_t CMM7Message::EncodeVASPRes(acl::CMemoryBlock* apoBuffer)
{

    return 0;
}

apl_ssize_t CMM7Message::EncodeRSReq(acl::CMemoryBlock* apoBuffer)
{
    return 0;
}

apl_ssize_t CMM7Message::EncodeReqNormalPart(acl::CMemoryBlock* apoBuffer)
{
    BUFFER_WRITE(apoBuffer, "this is a multi-part message in MIME format\r\n");
    BUFFER_WRITE(apoBuffer, "\r\n\r\n");
    BUFFER_WRITE(apoBuffer, "--");
    BUFFER_WRITE(apoBuffer, NEXT_PART);
    BUFFER_WRITE(apoBuffer, "Content-Type:text/xml;charset=\"UTF-8\"\r\n");
    BUFFER_WRITE(apoBuffer, "Content-Transfer-Encoding:8bit\r\n");
    BUFFER_WRITE(apoBuffer, "Content-ID:</tnn-200102/mm7-vasp>\r\n");
    BUFFER_WRITE(apoBuffer, "\r\n");

    return 0;
}

apl_ssize_t CMM7Message::EncodeVASPReqSoapXML(acl::CMemoryBlock* apoBuffer)
{
    apl_int_t liRet = -1;
    MM7_RETURN_IF(liRet, (liRet = this->BuildVASPReqSoapHead()) != 0);

    //TODO: build soap body
    switch(this->miMessageType)
    {
    case MSG_TYPE_MM7_SUBMIT_REQ:
    {
        CMM7SubmitReq* lpoSubmitReqMsg = APL_NULL;
        MM7_RETURN_IF(MM7_ERR_ENCODEVASPREQ, (lpoSubmitReqMsg = dynamic_cast<CMM7SubmitReq*>(this)) == APL_NULL);
        MM7_RETURN_IF(liRet, (liRet = this->BuildSubmitReqSoapBody(lpoSubmitReqMsg)) != 0);
        break;
    }
    case MSG_TYPE_MM7_REPLACE_REQ:
    {
        CMM7ReplaceReq* lpoReplaceReqMsg = APL_NULL;
        MM7_RETURN_IF(MM7_ERR_ENCODEVASPREQ, (lpoReplaceReqMsg = dynamic_cast<CMM7ReplaceReq*>(this)) == APL_NULL);
        MM7_RETURN_IF(liRet, (liRet = this->BuildReplaceReqSoapBody(lpoReplaceReqMsg)) != 0);
        break;
    }
    case MSG_TYPE_MM7_CANCLE_REQ:
    {
        CMM7CancelReq* lpoCancelReqMsg = APL_NULL;
        MM7_RETURN_IF(MM7_ERR_ENCODEVASPREQ, (lpoCancelReqMsg = dynamic_cast<CMM7CancelReq*>(this)) == APL_NULL);
        MM7_RETURN_IF(liRet, (liRet = this->BuildCancelReqSoapBody(lpoCancelReqMsg)) != 0);
        break;
    }
    default:
        return MM7_ERR_ENCODEVASPREQ;
    }

    acl::CXmlCreator loCreator;

    //Create xml buffer
    loCreator.Create(this->moSoapXML, acl::CXmlCreator::OPT_UNFORMAT);

    BUFFER_WRITE(apoBuffer, XML_COMMENT);
    BUFFER_WRITE(apoBuffer, loCreator.GetXml());

    BUFFER_WRITE(apoBuffer, "\r\n");
    BUFFER_WRITE(apoBuffer, "--");
    BUFFER_WRITE(apoBuffer, NEXT_PART);

    return 0;
}

apl_ssize_t CMM7Message::EncodeVASPResSoapXML(acl::CMemoryBlock* apoBuffer)
{
    apl_int_t liRet = -1;
    MM7_RETURN_IF(liRet, (liRet = this->BuildVASPResSoapHead()) != 0);

    BUFFER_WRITE(apoBuffer, "<?xml version=\"1.0\"?>");

    switch(this->miMessageType)
    {
    case MSG_TYPE_MM7_DELIVER_RES:
    {
        CMM7DeliverRes* lpoDeliverRes = APL_NULL;
        MM7_RETURN_IF(MM7_ERR_ENCODEVASPRES, (lpoDeliverRes = dynamic_cast<CMM7DeliverRes*>(this)) == APL_NULL);
        MM7_RETURN_IF(liRet, (liRet = this->BuildDeliverResSoapBody(lpoDeliverRes)) != 0);
        break; 
    }
    case MSG_TYPE_MM7_DELIVERY_REPORT_RES:
    {
        CMM7DeliveryReportRes* lpoDeliveryReportRes = APL_NULL;
        MM7_RETURN_IF(MM7_ERR_ENCODEVASPRES, (lpoDeliveryReportRes = dynamic_cast<CMM7DeliveryReportRes*>(this)) == APL_NULL);
        MM7_RETURN_IF(liRet, (liRet = this->BuildDeliveryReportResSoapBody(lpoDeliveryReportRes)) != 0);
        break;
    }
    case MSG_TYPE_MM7_READ_REPLAY_RES:
    {
        CMM7ReadReplyRes* lpoReadReplyRes = APL_NULL;
        MM7_RETURN_IF(MM7_ERR_ENCODEVASPRES, (lpoReadReplyRes = dynamic_cast<CMM7ReadReplyRes*>(this)) == APL_NULL);
        MM7_RETURN_IF(liRet, (liRet = this->BuildReadReplyResSoapBody(lpoReadReplyRes)) != 0);
        break;
    }
    case MSG_TYPE_MM7_VASP_ERROR_RES:
    {
        CMM7VASPErrorRes* lpoVASPErrorRes = APL_NULL;
        MM7_RETURN_IF(MM7_ERR_ENCODEVASPRES, (lpoVASPErrorRes = dynamic_cast<CMM7VASPErrorRes*>(this)) == APL_NULL);
        MM7_RETURN_IF(liRet, (liRet = this->BuildVASPErrorResSoapBody(lpoVASPErrorRes)) != 0);
    }
    default:
        return MM7_ERR_ENCODEVASPRES;
    }

    acl::CXmlCreator loCreator;
    loCreator.Create(this->moSoapXML, acl::CXmlCreator::OPT_UNFORMAT);

    BUFFER_WRITE(apoBuffer, loCreator.GetXml());
    BUFFER_WRITE(apoBuffer, "\r\n");

    return 0;
}

apl_ssize_t CMM7Message::EncodeSRReqSoapXML(acl::CMemoryBlock* apoBuffer)
{
    apl_int_t liRet = -1;
    MM7_RETURN_IF(liRet, (liRet = this->BuildSRReqSoapHead()) != 0);

    //TODO: build soap body
    switch(this->miMessageType)
    {
    case MSG_TYPE_MM7_DELIVER_REQ:
    {
        CMM7DeliverReq* lpoDeliverReqMsg = APL_NULL;
        MM7_RETURN_IF(MM7_ERR_ENCODESRREQ, (lpoDeliverReqMsg = dynamic_cast<CMM7DeliverReq*>(this)) == APL_NULL);
        MM7_RETURN_IF(liRet, (liRet = this->BuildDeliverReqSoapBody(lpoDeliverReqMsg)) != 0);
    }
    default:
        return MM7_ERR_ENCODESRREQ;
    }

    acl::CXmlCreator loCreator;

    //Create xml buffer
    loCreator.Create(this->moSoapXML, acl::CXmlCreator::OPT_UNFORMAT);

    BUFFER_WRITE(apoBuffer, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    BUFFER_WRITE(apoBuffer, loCreator.GetXml());

    BUFFER_WRITE(apoBuffer, "\r\n");
    BUFFER_WRITE(apoBuffer, "--");
    BUFFER_WRITE(apoBuffer, NEXT_PART);

    return 0;
}

apl_ssize_t CMM7Message::EncodeVASPReqContentIfHas(acl::CMemoryBlock* apoBuffer)
{
    const char* lpcPrimaryType = APL_NULL;
    const char* lpcSubType = APL_NULL;
    const char* lpcCharset = APL_NULL;
    const char* lpcContentID = APL_NULL;
    const char* lpcContentLocation = APL_NULL;
    char lacTmpBuf[256] = {0};
    const char* lpcFileName= APL_NULL;
    acl::CMemoryBlock* lpoMBContentBody = APL_NULL;

    CMM7Content* lpoContent = APL_NULL;

    if(this->miMessageType == MSG_TYPE_MM7_SUBMIT_REQ)
    {
        CMM7SubmitReq* lpoMsg = dynamic_cast<CMM7SubmitReq*>(this);
        if(!lpoMsg->IsContentExist())
        {
            return 0;
        }
        lpoContent = lpoMsg->GetContent();
    }
    else if(this->miMessageType == MSG_TYPE_MM7_REPLACE_REQ)
    {
        CMM7ReplaceReq* lpoMsg = dynamic_cast<CMM7ReplaceReq*>(this);
        if(!lpoMsg->IsContentExist())
        {
            return 0;
        }
        lpoContent = lpoMsg->GetContent();
    }
    else if(this->miMessageType == MSG_TYPE_MM7_DELIVER_REQ)
    {
        CMM7DeliverReq* lpoMsg = dynamic_cast<CMM7DeliverReq*>(this);
        if(!lpoMsg->IsContentExist())
        {
            return 0;
        }
        lpoContent = lpoMsg->GetContent();
    }
    else
    {
        return MM7_ERR_INVALIDMESSAGETYPE;
    }

    if( lpoContent->GetContentType() != APL_NULL)
    {
        const char* lpcTmpID = "<START>";
        lpcSubType = lpoContent->GetContentType()->GetSubType();
        if( apl_strcasecmp( lpcSubType, "related") == 0)
        {
            if( lpoContent->IsMultipart() )
            {
                CMM7Content::ContentListType* lpoContentList = lpoContent->GetSubContents();
                CMM7Content::ContentListType::iterator loIter;
                for(loIter = lpoContentList->begin(); loIter != lpoContentList->end();
                        ++loIter)
                {
                    lpcSubType = (*loIter)->GetContentType()->GetSubType();
                    //printf("smil: [%s]\n", lpcSubType);
                    if( lpcSubType[0] != '\0' && apl_strcasecmp(lpcSubType, "smil") == 0)
                    {
                        if( (*loIter)->GetContentID()[0] != '\0')
                        {
                            lpcTmpID = (*loIter)->GetContentID();
                        }

                        break;
                    }
                }
            }

            BUFFER_WRITE(apoBuffer, "Content-Type:multipart/related;start=\"");
            //用smil文件的contentID作为Start
            BUFFER_WRITE(apoBuffer, lpcTmpID);
            BUFFER_WRITE(apoBuffer, "\";type=\"application/smil\";boundary=\"");
            BUFFER_WRITE_LEN(apoBuffer, SUB_PART, apl_strlen(SUB_PART) - 2);
            BUFFER_WRITE(apoBuffer, "\"\r\n");
        }
        else
        {
            BUFFER_WRITE(apoBuffer, "Content-Type:");
            lpcPrimaryType = lpoContent->GetContentType()->GetPrimaryType();
            BUFFER_WRITE(apoBuffer, lpcPrimaryType);
            BUFFER_WRITE(apoBuffer, "/");

            lpcSubType = lpoContent->GetContentType()->GetSubType();
            BUFFER_WRITE(apoBuffer, lpcSubType);
            BUFFER_WRITE(apoBuffer, ";boundary=\"");
            BUFFER_WRITE_LEN(apoBuffer, SUB_PART, apl_strlen(SUB_PART) - 2);
            BUFFER_WRITE(apoBuffer, "\"\r\n");
        }
    }
    else
    {
        BUFFER_WRITE(apoBuffer, "Content-Type:multipart/mixed;boundary=\"");
        BUFFER_WRITE_LEN(apoBuffer, SUB_PART, apl_strlen(SUB_PART) - 2);
        BUFFER_WRITE(apoBuffer, "\"\r\n");
    }

    //Add parent Content-ID
    if( lpoContent->GetContentID()[0] != '\0' )
    {
        BUFFER_WRITE(apoBuffer, "Content-ID:");
        BUFFER_WRITE(apoBuffer, lpoContent->GetContentID());
        BUFFER_WRITE(apoBuffer, "\r\n");
    }
    else
    {
        BUFFER_WRITE(apoBuffer, "Content-ID:<SaturnPics-01020930>\r\n");
    }

    //Add parent Content-Transfer-Encoding
    BUFFER_WRITE(apoBuffer, "Content-Transfer-Encoding:8bit\r\n");

    //Add parent Content-Location if has
    if( lpoContent->GetContentLocation()[0] != '\0' )
    {
        BUFFER_WRITE(apoBuffer, "Content-Location:");
        BUFFER_WRITE(apoBuffer, lpoContent->GetContentLocation());
        BUFFER_WRITE(apoBuffer, "\r\n");
    }

    BUFFER_WRITE(apoBuffer, "\r\n");

    //Add subContents if has
    if( lpoContent->IsMultipart() )
    {
        CMM7Content::ContentListType* lpoContentList = lpoContent->GetSubContents();
        CMM7Content::ContentListType::iterator loIter;
        for(loIter = lpoContentList->begin(); loIter != lpoContentList->end(); ++loIter)
        {
            BUFFER_WRITE(apoBuffer, "--");
            BUFFER_WRITE(apoBuffer, SUB_PART);

            //Add Content-Type
            if( (*loIter)->GetContentType() != APL_NULL)
            {
                BUFFER_WRITE(apoBuffer, "Content-Type:"); 

                lpcPrimaryType = (*loIter)->GetContentType()->GetPrimaryType();
                BUFFER_WRITE(apoBuffer, lpcPrimaryType);
                BUFFER_WRITE(apoBuffer, "/");

                lpcSubType = (*loIter)->GetContentType()->GetSubType();
                BUFFER_WRITE(apoBuffer, lpcSubType);

                if( apl_strcasecmp(lpcPrimaryType, "text") == 0 || apl_strcasecmp(lpcSubType, "smil") == 0)
                {    
                    BUFFER_WRITE(apoBuffer, ";charset=");

                    lpcCharset = (*loIter)->GetCharset();
                    memset(lacTmpBuf, 0, sizeof(lacTmpBuf));
                    apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", lpcCharset);
                    BUFFER_WRITE(apoBuffer, lacTmpBuf);
                }    
        
                //Add FileName, 2010-01-11
                lpcFileName = (*loIter)->GetFileName();
                if(apl_strcasecmp(lpcSubType, "smil") != 0 && lpcFileName[0] != '\0')
                {    
                    BUFFER_WRITE(apoBuffer, ";name=");
                    memset(lacTmpBuf, 0, sizeof(lacTmpBuf));
                    apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"%s\"", lpcFileName);
                    BUFFER_WRITE(apoBuffer, lacTmpBuf);
                } 

                BUFFER_WRITE(apoBuffer, "\r\n");
            }
            else
            {
                return MM7_ERR_CONTENTTYPEEMPTY;
            }

            //Add Content-Transfer-Encoding
            BUFFER_WRITE(apoBuffer, "Content-Transfer-Encoding:");
            if( (*loIter)->IsBase64Encode() )
            {
                BUFFER_WRITE(apoBuffer, "BASE64");
            }
            else
            {
                BUFFER_WRITE(apoBuffer, "8bit");
            }
            BUFFER_WRITE(apoBuffer, "\r\n");

            //Add Content-ID
            lpcContentID = (*loIter)->GetContentID();
            if( lpcContentID[0] != '\0' )
            {
                BUFFER_WRITE(apoBuffer, "Content-ID:");
                BUFFER_WRITE(apoBuffer, lpcContentID);
                BUFFER_WRITE(apoBuffer, "\r\n");
            }

            //Add Content-Location
            lpcContentLocation = (*loIter)->GetContentLocation();
            if( lpcContentLocation[0] != '\0' )
            {
                BUFFER_WRITE(apoBuffer, "Content-Location:");
                BUFFER_WRITE(apoBuffer, lpcContentLocation);
                BUFFER_WRITE(apoBuffer, "\r\n");
            }

            BUFFER_WRITE(apoBuffer, "\r\n");

            //Add Content-Body
            //2009-11-06 modify aimc mmsproxy
            /*
            if( (*loIter)->IsBase64Encode() )
            {
                lpoMBContentBody = (*loIter)->GetContentBuffer();

                acl::CBase64Decoder loDecoder;
                loDecoder.Final(lpoMBContentBody->GetReadPtr(), lpoMBContentBody->GetLength()-1);
                const char* lpcOutput = loDecoder.GetOutput();
                apl_size_t luLen = loDecoder.GetLength();
                apoBuffer->Write(lpcOutput, luLen);
            }
            else 
            */
            {
                lpoMBContentBody = (*loIter)->GetContentBuffer();
                BUFFER_WRITE_LEN(apoBuffer, lpoMBContentBody->GetReadPtr(), lpoMBContentBody->GetLength()-1);
            }

            BUFFER_WRITE(apoBuffer, "\r\n\r\n");
        }
    }

    BUFFER_WRITE(apoBuffer, "--");
    BUFFER_WRITE_LEN(apoBuffer, SUB_PART, apl_strlen(SUB_PART) - 2);
    BUFFER_WRITE(apoBuffer, "--\r\n");
    BUFFER_WRITE(apoBuffer, "--");
    BUFFER_WRITE_LEN(apoBuffer, NEXT_PART, apl_strlen(NEXT_PART) -2);
    BUFFER_WRITE(apoBuffer, "--\r\n");

    return 0;
}

apl_int_t CMM7Message::BuildVASPReqSoapHead(void)
{
    acl::CXmlElement::IteratorType loElemIterHeader;

    this->moSoapXML.SetName("env:Envelope");
    this->moSoapXML.AddAttribute("xmlns:env", "\"http://schemas.xmlsoap.org/soap/envelope/\"");

    loElemIterHeader = this->moSoapXML.AddElement("env:Header");
    acl::CXmlElement::IteratorType loElemIterTrans = loElemIterHeader->AddElement("mm7:TransactionID");
    loElemIterTrans->AddAttribute("xmlns:mm7", MM7_SCHEMA);
    loElemIterTrans->AddAttribute("env:mustUnderstand", "\"1\"");

    //Add TransactionID, can't be empty
    if( this->GetTransactionID()[0] != '\0')
    {
        loElemIterTrans->AddText(this->GetTransactionID());
    }
    else
    {
        return MM7_ERR_TRANSIDEMPTY;   
    }

    return 0;
}

apl_int_t CMM7Message::BuildSubmitReqSoapBody(CMM7SubmitReq* apoMsg)
{
    apl_int_t liRet = -1;

    acl::CXmlElement::IteratorType loElemIterBody;
    loElemIterBody = this->moSoapXML.AddElement("env:Body");

    acl::CXmlElement::IteratorType loElemIterReq;
    loElemIterReq = loElemIterBody->AddElement(SUBMITREQ);
    loElemIterReq->AddAttribute("xmlns", MM7_SCHEMA);

    //Add MM7Version, can't be empty
    if(apoMsg->GetMM7Version()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterVer = loElemIterReq->AddElement("MM7Version");
        loElemIterVer->AddText(apoMsg->GetMM7Version());
    }
    else
    {
        return MM7_ERR_MM7VERSIONEMPTY;
    }

    //Add SenderIdentification(VASPID/VASID/SenderAddress), VASPID and VASID must be exist
    if(apoMsg->GetVASPID()[0] != '\0' && apoMsg->GetVASID()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterSenderID = loElemIterReq->AddElement("SenderIdentification");

        acl::CXmlElement::IteratorType loElemIterSPID = loElemIterSenderID->AddElement("VASPID");
        loElemIterSPID->AddText(apoMsg->GetVASPID());

        acl::CXmlElement::IteratorType loElemIterSID = loElemIterSenderID->AddElement("VASID");
        loElemIterSID->AddText(apoMsg->GetVASID());

        if(apoMsg->GetSenderAddress()[0] != '\0')
        {
            acl::CXmlElement::IteratorType loElemIterSendAdd = loElemIterSenderID->AddElement("SenderAddress");
            loElemIterSendAdd->AddText(apoMsg->GetSenderAddress());
        }
    }
    else
    {
        return MM7_ERR_SPIDORSIDEMPTY;
    }

    //Add Recipients(To/Cc/Bcc), must one exist
    if( apoMsg->GetToCount() > 0
        || apoMsg->GetCcCount() > 0
        || apoMsg->GetBccCount() > 0 )
    {
        const char* lpcNumber = APL_NULL;

        acl::CXmlElement::IteratorType loElemIterRecipients = loElemIterReq->AddElement("Recipients");
        if(apoMsg->GetToCount() > 0)
        {
            acl::CXmlElement::IteratorType loElemIterTo = loElemIterRecipients->AddElement("To");

            for(apl_size_t liN=0; liN<apoMsg->GetToCount(); ++liN)
            {
                lpcNumber = apoMsg->GetTo(liN);
                if( apl_strrchr(lpcNumber, '@') != APL_NULL)
                {
                    acl::CXmlElement::IteratorType loElemIterRfc = loElemIterTo->AddElement("RFC2822Address");
                    loElemIterRfc->AddText(lpcNumber);
                }
                else
                {
                    acl::CXmlElement::IteratorType loElemIterNum = loElemIterTo->AddElement("Number");
                    loElemIterNum->AddText(lpcNumber);
                }
            }
        }

        if(apoMsg->GetCcCount() > 0)
        {
            acl::CXmlElement::IteratorType loElemIterCc = loElemIterRecipients->AddElement("Cc");

            for(apl_size_t liN=0; liN<apoMsg->GetCcCount(); ++liN)
            {
                lpcNumber = apoMsg->GetCc(liN);
                if( apl_strrchr(lpcNumber, '@') != APL_NULL)
                {
                    acl::CXmlElement::IteratorType loElemIterRfc = loElemIterCc->AddElement("RFC2822Address");
                    loElemIterRfc->AddText(lpcNumber);
                }
                else
                {
                    acl::CXmlElement::IteratorType loElemIterNum = loElemIterCc->AddElement("Number");
                    loElemIterNum->AddText(lpcNumber);
                }
            }
        }

        if(apoMsg->GetBccCount() > 0)
        {
            acl::CXmlElement::IteratorType loElemIterBcc = loElemIterRecipients->AddElement("Bcc");

            for(apl_size_t liN=0; liN<apoMsg->GetBccCount(); ++liN)
            {
                lpcNumber = apoMsg->GetBcc(liN);
                if( apl_strrchr(lpcNumber, '@') != APL_NULL)
                {
                    acl::CXmlElement::IteratorType loElemIterRfc = loElemIterBcc->AddElement("RFC2822Address");
                    loElemIterRfc->AddText(lpcNumber);
                }
                else
                {
                    acl::CXmlElement::IteratorType loElemIterNum = loElemIterBcc->AddElement("Number");
                    loElemIterNum->AddText(lpcNumber);
                }
            }
        }
    }
    else
    {
        return MM7_ERR_RECIPIENTSEMPTY;
    }

    //Add ServiceCode, can't be empty
    if(apoMsg->GetServiceCode()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterServCode = loElemIterReq->AddElement("ServiceCode");
        loElemIterServCode->AddText(apoMsg->GetServiceCode());
    }
    else
    {
        return MM7_ERR_SERVCODEEMPTY;
    }

    acl::CXmlElement::IteratorType loElemIterTmp;
    //Add LinkedID if has
    if(apoMsg->GetLinkedID()[0] != '\0')
    {
        loElemIterTmp = loElemIterReq->AddElement("LinkedID");
        loElemIterTmp->AddText(apoMsg->GetLinkedID());
    }

    //Add MessageClass if has
    if(apoMsg->GetMessageClass()[0] != '\0')
    {
        loElemIterTmp = loElemIterReq->AddElement("MessageClass");
        loElemIterTmp->AddText(apoMsg->GetMessageClass());
    }

    if(apoMsg->IsTimeStamp())
    {
        acl::CDateTime loDateTime;
        loDateTime.Update();
        const char* lpcTimestamp = loDateTime.Format(MM7_DATE_FORMAT);

        loElemIterTmp = loElemIterReq->AddElement("TimeStamp");
        loElemIterTmp->AddText(lpcTimestamp);
    }

    if(apoMsg->GetEarliestDeliveryTimeAbsolute()[0] != '\0')
    {
        //no need to check format
        /*
        acl::CDateTime loDateTime;
        liRet = loDateTime.Update(apoMsg->GetEarliestDeliveryTimeAbsolute(), MM7_DATE_FORMAT);
        if( liRet != 0 )
        {
            return MM7_ERR_INVALIDDATEFORMAT;
        }
        */

        loElemIterTmp = loElemIterReq->AddElement("EarliestDeliveryTime");
        loElemIterTmp->AddText(apoMsg->GetEarliestDeliveryTimeAbsolute());
    }
    else if(apoMsg->GetEarliestDeliveryTimeDuration() != 0)
    {
        acl::CDateTime loDateTime;
        acl::CTimestamp loTimestamp;
        acl::CTimeValue loTimeValue(apoMsg->GetEarliestDeliveryTimeDuration(), APL_TIME_HOUR);

        loDateTime.Update();
        loDateTime.GetTimestamp(loTimestamp);
        loTimestamp += loTimeValue;

        loDateTime.Update(loTimestamp);
        const char* lpcDateTime = loDateTime.Format(MM7_DATE_FORMAT);
        loElemIterTmp = loElemIterReq->AddElement("EarliestDeliveryTime");
        loElemIterTmp->AddText(lpcDateTime);
    }

    if(apoMsg->GetExpiryDateAbsolute()[0] != '\0')
    {
        /*
        acl::CDateTime loDateTime;
        liRet = loDateTime.Update(apoMsg->GetExpiryDateAbsolute(), MM7_DATE_FORMAT);
        if( liRet != 0 )
        {
            return MM7_ERR_INVALIDDATEFORMAT;
        }
        */

        loElemIterTmp = loElemIterReq->AddElement("ExpiryDate");
        loElemIterTmp->AddText(apoMsg->GetExpiryDateAbsolute());
    }
    else if( apoMsg->GetExpiryDateDuration() != 0 )
    {
        acl::CDateTime loDateTime;
        acl::CTimestamp loTimestamp;
        acl::CTimeValue loTimeValue(apoMsg->GetExpiryDateDuration(), APL_TIME_HOUR);

        loDateTime.Update();
        loDateTime.GetTimestamp(loTimestamp);
        loTimestamp += loTimeValue;

        loDateTime.Update(loTimestamp);
        const char* lpcDateTime = loDateTime.Format(MM7_DATE_FORMAT);
        loElemIterTmp = loElemIterReq->AddElement("ExpiryDate");
        loElemIterTmp->AddText(lpcDateTime);
    }

    //Add DeliveryReport
    loElemIterTmp = loElemIterReq->AddElement("DeliveryReport");
    if(apoMsg->IsDeliveryReport())
    {
        loElemIterTmp->AddText("true");
    }
    else
    {
        loElemIterTmp->AddText("false");
    }

    //Add ReadReply
    loElemIterTmp = loElemIterReq->AddElement("ReadReply");
    if(apoMsg->IsReadReply())
    {
        loElemIterTmp->AddText("true");
    }
    else
    {
        loElemIterTmp->AddText("false");
    }

    //Add ReplyCharging
    if(apoMsg->IsReplyCharging())
    {
        loElemIterTmp = loElemIterReq->AddElement("ReplyCharging");
        if(apoMsg->GetReplyChargingSize() > 0)
        {
            char lacReplyChargingSize[20] = {0};
            apl_snprintf(lacReplyChargingSize,
                sizeof(lacReplyChargingSize),
                "\"%"APL_PRIuINT"\"",
                apoMsg->GetReplyChargingSize());

            loElemIterTmp->AddAttribute("replyChargingSize", lacReplyChargingSize);
        }

        if(apoMsg->GetReplyDeadlineAbsolute()[0] != '\0' )
        {
            acl::CDateTime loDateTime;
            liRet = loDateTime.Update(apoMsg->GetReplyDeadlineAbsolute(), MM7_DATE_FORMAT);
            if( liRet != 0 )
            {
                return MM7_ERR_INVALIDDATEFORMAT;
            }

            loElemIterTmp->AddAttribute("replyDeadline", apoMsg->GetReplyDeadlineAbsolute());
        }
        else if( apoMsg->GetReplyDeadlineDuration() != 0 )
        {

            acl::CDateTime loDateTime;
            acl::CTimestamp loTimestamp;
            acl::CTimeValue loTimeValue(apoMsg->GetReplyDeadlineDuration(), APL_TIME_HOUR);

            loDateTime.Update();
            loDateTime.GetTimestamp(loTimestamp);
            loTimestamp += loTimeValue;

            loDateTime.Update(loTimestamp);
            const char* lpcDateTime = loDateTime.Format(MM7_DATE_FORMAT);
            loElemIterTmp->AddAttribute("replyDeadline", lpcDateTime);
        }
    }

    //Add Priority
    if(apoMsg->GetPriority()[0] != '\0')
    {
        loElemIterTmp = loElemIterReq->AddElement("Priority");
        loElemIterTmp->AddText(apoMsg->GetPriority());
    }

    //Add Subject
    loElemIterTmp = loElemIterReq->AddElement("Subject");
    if(apoMsg->GetSubject()[0] != '\0')
    {
        loElemIterTmp->AddText(apoMsg->GetSubject());
    }

    //Add ChargedParty
    if( apoMsg->GetChargedParty()[0] != '\0')
    {
        loElemIterTmp = loElemIterReq->AddElement("ChargedParty");
        loElemIterTmp->AddText(apoMsg->GetChargedParty());
    }

    //Add ChargedPartyID
    if( apoMsg->GetChargedPartyID()[0] != '\0')
    {
        loElemIterTmp = loElemIterReq->AddElement("ChargedPartyID");
        loElemIterTmp->AddText(apoMsg->GetChargedPartyID());
    }

    //Add DistributionIndicator
    loElemIterTmp = loElemIterReq->AddElement("DistributionIndicator");
    if( apoMsg->IsDistributionIndicator() == false)
    {
        loElemIterTmp->AddText("false");
    }
    else
    {
        loElemIterTmp->AddText("true");
    }

    //Add Content
    if( apoMsg->IsContentExist() )
    {
        loElemIterTmp = loElemIterReq->AddElement("Content");
        char lacTmpBuf[64] = {0};
        apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"cid:%s\"", apoMsg->GetContent()->GetContentID());
        loElemIterTmp->AddAttribute("href", lacTmpBuf);

        if(apoMsg->IsAllowAdaptations())
        {
            loElemIterTmp->AddAttribute("allowAdaptions", "\"true\"");
        }
        else
        {
            loElemIterTmp->AddAttribute("allowAdaptions", "\"false\"");
        }
    }

    return 0;
}

apl_int_t CMM7Message::BuildReplaceReqSoapBody(CMM7ReplaceReq* apoMsg)
{
    acl::CXmlElement::IteratorType loElemIterBody;
    loElemIterBody = this->moSoapXML.AddElement("env:Body");

    acl::CXmlElement::IteratorType loElemIterReq;
    loElemIterReq = loElemIterBody->AddElement(REPLACEREQ);
    loElemIterReq->AddAttribute("xmlns", MM7_SCHEMA);

    acl::CXmlElement::IteratorType loElemIterTmp;

    //Add MM7Version, can't be empty
    if(apoMsg->GetMM7Version()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterVer = loElemIterReq->AddElement("MM7Version");
        loElemIterVer->AddText(apoMsg->GetMM7Version());
    }
    else
    {
        return MM7_ERR_MM7VERSIONEMPTY;
    }

    if(apoMsg->GetVASPID()[0] != '\0' || apoMsg->GetVASID()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterSenderID = loElemIterReq->AddElement("SenderIdentification");

        if(apoMsg->GetVASPID()[0] != '\0')
        {
            acl::CXmlElement::IteratorType loElemIterSPID = loElemIterSenderID->AddElement("VASPID");
            loElemIterSPID->AddText(apoMsg->GetVASPID());
        }

        if(apoMsg->GetVASID()[0] != '\0')
        {
            acl::CXmlElement::IteratorType loElemIterSID = loElemIterSenderID->AddElement("VASID");
            loElemIterSID->AddText(apoMsg->GetVASID());
        }
    }

    if(apoMsg->GetServiceCode()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterServCode = loElemIterReq->AddElement("ServiceCode");
        loElemIterServCode->AddText(apoMsg->GetServiceCode());
    }

    if(apoMsg->GetMessageID()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterMsgID = loElemIterReq->AddElement("MessageID");
        loElemIterMsgID->AddText(apoMsg->GetMessageID());
    }
    else
    {
        return MM7_ERR_MESSAGEID;
    }


    if(apoMsg->IsTimeStamp())
    {
        acl::CDateTime loDateTime;
        loDateTime.Update();
        const char* lpcTimestamp = loDateTime.Format(MM7_DATE_FORMAT);

        loElemIterTmp = loElemIterReq->AddElement("TimeStamp");
        loElemIterTmp->AddText(lpcTimestamp);
    }

    if(apoMsg->GetEarliestDeliveryTimeAbsolute()[0] != '\0')
    {
        /*
        acl::CDateTime loDateTime;
        liRet = loDateTime.Update(apoMsg->GetEarliestDeliveryTimeAbsolute(), MM7_DATE_FORMAT);
        if( liRet != 0 )
        {
            return MM7_ERR_INVALIDDATEFORMAT;
        }
        */

        loElemIterTmp = loElemIterReq->AddElement("EarliestDeliveryTime");
        loElemIterTmp->AddText(apoMsg->GetEarliestDeliveryTimeAbsolute());
    }
    else if(apoMsg->GetEarliestDeliveryTimeDuration() != 0)
    {
        acl::CDateTime loDateTime;
        acl::CTimestamp loTimestamp;
        acl::CTimeValue loTimeValue(apoMsg->GetEarliestDeliveryTimeDuration(), APL_TIME_HOUR);

        loDateTime.Update();
        loDateTime.GetTimestamp(loTimestamp);
        loTimestamp += loTimeValue;

        loDateTime.Update(loTimestamp);
        const char* lpcDateTime = loDateTime.Format(MM7_DATE_FORMAT);
        loElemIterTmp = loElemIterReq->AddElement("EarliestDeliveryTime");
        loElemIterTmp->AddText(lpcDateTime);
    }

    //Add ReadReply
    loElemIterTmp = loElemIterReq->AddElement("ReadReply");
    if(apoMsg->IsReadReply())
    {
        loElemIterTmp->AddText("true");
    }
    else
    {
        loElemIterTmp->AddText("false");
    }

    //Add DistributionIndicator
    loElemIterTmp = loElemIterReq->AddElement("DistributionIndicator");
    if( apoMsg->IsDistributionIndicator() == false)
    {
        loElemIterTmp->AddText("false");
    }
    else
    {
        loElemIterTmp->AddText("true");
    }

    //Add Content
    if( apoMsg->IsContentExist() )
    {
        loElemIterTmp = loElemIterReq->AddElement("Content");
        char lacTmpBuf[64] = {0};
        apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"cid:%s\"", apoMsg->GetContent()->GetContentID());
        loElemIterTmp->AddAttribute("href", lacTmpBuf);

        if(apoMsg->IsAllowAdaptations())
        {
            loElemIterTmp->AddAttribute("allowAdaptions", "\"true\"");
        }
        else
        {
            loElemIterTmp->AddAttribute("allowAdaptions", "\"false\"");
        }
    }

    return 0;
}

apl_int_t CMM7Message::BuildCancelReqSoapBody(CMM7CancelReq* apoMsg)
{
    acl::CXmlElement::IteratorType loElemIterBody;
    loElemIterBody = this->moSoapXML.AddElement("env:Body");

    acl::CXmlElement::IteratorType loElemIterReq;
    loElemIterReq = loElemIterBody->AddElement(CANCELREQ);
    loElemIterReq->AddAttribute("xmlns", MM7_SCHEMA);

    //Add MM7Version, can't be empty
    if(apoMsg->GetMM7Version()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterVer = loElemIterReq->AddElement("MM7Version");
        loElemIterVer->AddText(apoMsg->GetMM7Version());
    }
    else
    {
        return MM7_ERR_MM7VERSIONEMPTY;
    }

    if(apoMsg->GetVASPID()[0] != '\0' || apoMsg->GetVASID()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterSenderID = loElemIterReq->AddElement("SenderIdentification");

        if(apoMsg->GetVASPID()[0] != '\0')
        {
            acl::CXmlElement::IteratorType loElemIterSPID = loElemIterSenderID->AddElement("VASPID");
            loElemIterSPID->AddText(apoMsg->GetVASPID());
        }

        if(apoMsg->GetVASID()[0] != '\0')
        {
            acl::CXmlElement::IteratorType loElemIterSID = loElemIterSenderID->AddElement("VASID");
            loElemIterSID->AddText(apoMsg->GetVASID());
        }
    }

    if(apoMsg->GetSenderAddress()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterSendAdd = loElemIterReq->AddElement("SenderAddress");
        loElemIterSendAdd->AddText(apoMsg->GetSenderAddress());
    }

    if(apoMsg->GetMessageID()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterMsgID = loElemIterReq->AddElement("MessageID");
        loElemIterMsgID->AddText(apoMsg->GetMessageID());
    }
    else
    {
        return MM7_ERR_MESSAGEID;
    }

    return 0;
}

apl_int_t CMM7Message::BuildVASPResSoapHead(void)
{
    acl::CXmlElement::IteratorType loElemIterHeader;

    this->moSoapXML.SetName("env:Envelope");
    this->moSoapXML.AddAttribute("xmlns:env", "\"http://schemas.xmlsoap.org/soap/envelope/\"");

    loElemIterHeader = this->moSoapXML.AddElement("env:Header");
    acl::CXmlElement::IteratorType loElemIterTrans = loElemIterHeader->AddElement("mm7:TransactionID");
    loElemIterTrans->AddAttribute("xmlns:mm7", MM7_SCHEMA);

    if(this->GetTransactionID()[0] != '\0')
    {
        loElemIterTrans->AddText(this->GetTransactionID());
    }
    else
    {
        return MM7_ERR_TRANSIDEMPTY;
    }

    return 0;
}

apl_int_t CMM7Message::BuildDeliverResSoapBody(CMM7DeliverRes* apoMsg)
{
    acl::CXmlElement::IteratorType loElemIterBody;
    loElemIterBody = this->moSoapXML.AddElement("env:Body");

    acl::CXmlElement::IteratorType loElemIter = loElemIterBody->AddElement(DELIVERRES);
    loElemIter->AddAttribute("xmlns", MM7_SCHEMA);

    acl::CXmlElement::IteratorType loElemIterSub;
    const char* lpcTmpValue = APL_NULL;

    lpcTmpValue = apoMsg->GetMM7Version();
    if(lpcTmpValue[0] != '\0')
    {
        loElemIterSub = loElemIter->AddElement("MM7Version");
        loElemIterSub->AddText(lpcTmpValue);
    }
    else
    {
        return MM7_ERR_MM7VERSIONEMPTY;
    }

    lpcTmpValue = apoMsg->GetServiceCode();
    if(lpcTmpValue[0] != '\0')
    {
        loElemIterSub = loElemIter->AddElement("ServiceCode");
        loElemIterSub->AddText(lpcTmpValue);
    }

    acl::CXmlElement::IteratorType loElemIterStatus;
    loElemIterStatus = loElemIter->AddElement("Status");

    acl::CXmlElement::IteratorType loElemIterStatusSub;

    apl_int_t liStatusCode = -1;
    liStatusCode = apoMsg->GetStatusCode();
    char lacTmpBuf[16] = {0};
    apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "%"APL_PRId32, liStatusCode);

    if(liStatusCode != -1)
    {
        loElemIterStatusSub = loElemIterStatus->AddElement("StatusCode");
        loElemIterStatusSub->AddText(lacTmpBuf);
    }
    else
    {
        return MM7_ERR_STATUSCODE;
    }

    lpcTmpValue = apoMsg->GetStatusText();
    if(lpcTmpValue[0] != '\0')
    {
        loElemIterStatusSub = loElemIterStatus->AddElement("StatusText");
        loElemIterStatusSub->AddText(lpcTmpValue);
    }

    return 0;
}

apl_int_t CMM7Message::BuildDeliveryReportResSoapBody(CMM7DeliveryReportRes* apoMsg)
{
    acl::CXmlElement::IteratorType loElemIterBody;
    loElemIterBody = this->moSoapXML.AddElement("env:Body");

    acl::CXmlElement::IteratorType loElemIter = loElemIterBody->AddElement(DELIVERYREPORTRES);
    loElemIter->AddAttribute("xmlns", MM7_SCHEMA);

    acl::CXmlElement::IteratorType loElemIterSub;
    const char* lpcTmpValue = APL_NULL;

    lpcTmpValue = apoMsg->GetMM7Version();
    if(lpcTmpValue[0] != '\0')
    {
        loElemIterSub = loElemIter->AddElement("MM7Version");
        loElemIterSub->AddText(lpcTmpValue);
    }
    else
    {
        return MM7_ERR_MM7VERSIONEMPTY;
    }

    acl::CXmlElement::IteratorType loElemIterStatus;
    loElemIterStatus = loElemIter->AddElement("Status");

    acl::CXmlElement::IteratorType loElemIterStatusSub;

    apl_int_t liStatusCode = apoMsg->GetStatusCode();
    char lacTmpBuf[16] = {0};
    apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "%"APL_PRId32, liStatusCode);

    if(lpcTmpValue[0] != '\0')
    {
        loElemIterStatusSub = loElemIterStatus->AddElement("StatusCode");
        loElemIterStatusSub->AddText(lacTmpBuf);
    }
    else
    {
        return MM7_ERR_STATUSCODE;
    }

    lpcTmpValue = apoMsg->GetStatusText();
    if(lpcTmpValue[0] != '\0')
    {
        loElemIterStatusSub = loElemIterStatus->AddElement("StatusText");
        loElemIterStatusSub->AddText(lpcTmpValue);
    }

    return 0;
}

apl_int_t CMM7Message::BuildReadReplyResSoapBody(CMM7ReadReplyRes* apoMsg)
{
    acl::CXmlElement::IteratorType loElemIterBody;
    loElemIterBody = this->moSoapXML.AddElement("env:Body");

    acl::CXmlElement::IteratorType loElemIter = loElemIterBody->AddElement(READREPLYRES);
    loElemIter->AddAttribute("xmlns", MM7_SCHEMA);

    acl::CXmlElement::IteratorType loElemIterSub;
    const char* lpcTmpValue = APL_NULL;

    lpcTmpValue = apoMsg->GetMM7Version();
    if(lpcTmpValue[0] != '\0')
    {
        loElemIterSub = loElemIter->AddElement("MM7Version");
        loElemIterSub->AddText(lpcTmpValue);
    }
    else
    {
        return MM7_ERR_MM7VERSIONEMPTY;
    }

    acl::CXmlElement::IteratorType loElemIterStatus;
    loElemIterStatus = loElemIter->AddElement("Status");

    acl::CXmlElement::IteratorType loElemIterStatusSub;

    apl_int_t liStatusCode = apoMsg->GetStatusCode();
    char lacTmpBuf[16] = {0};
    apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "%"APL_PRId32, liStatusCode);

    if(lpcTmpValue[0] != '\0')
    {
        loElemIterStatusSub = loElemIterStatus->AddElement("StatusCode");
        loElemIterStatusSub->AddText(lacTmpBuf);
    }
    else
    {
        return MM7_ERR_STATUSCODE;
    }

    lpcTmpValue = apoMsg->GetStatusText();
    if(lpcTmpValue[0] != '\0')
    {
        loElemIterStatusSub = loElemIterStatus->AddElement("StatusText");
        loElemIterStatusSub->AddText(lpcTmpValue);
    }
    else
    {
        return MM7_ERR_STATUSTEXT;
    }

    return 0;
}

apl_int_t CMM7Message::BuildVASPErrorResSoapBody(CMM7VASPErrorRes* apoMsg)
{
    acl::CXmlElement::IteratorType loElemIterBody;
    loElemIterBody = this->moSoapXML.AddElement("env:Body");

    acl::CXmlElement::IteratorType loElemIter = loElemIterBody->AddElement(VASPERRORRES);
    loElemIter->AddAttribute("xmlns", MM7_SCHEMA);

    acl::CXmlElement::IteratorType loElemIterSub;
    const char* lpcTmpValue = APL_NULL;

    lpcTmpValue = apoMsg->GetMM7Version();
    if(lpcTmpValue[0] != '\0')
    {
        loElemIterSub = loElemIter->AddElement("MM7Version");
        loElemIterSub->AddText(lpcTmpValue);
    }
    else
    {
        return MM7_ERR_MM7VERSIONEMPTY;
    }

    acl::CXmlElement::IteratorType loElemIterStatus;
    loElemIterStatus = loElemIter->AddElement("Status");

    acl::CXmlElement::IteratorType loElemIterStatusSub;

    apl_int_t liStatusCode = apoMsg->GetStatusCode();
    char lacTmpBuf[16] = {0};
    apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "%"APL_PRId32, liStatusCode);

    if(lpcTmpValue[0] != '\0')
    {
        loElemIterStatusSub = loElemIterStatus->AddElement("StatusCode");
        loElemIterStatusSub->AddText(lacTmpBuf);
    }
    else
    {
        return MM7_ERR_STATUSCODE;
    }

    lpcTmpValue = apoMsg->GetStatusText();
    if(lpcTmpValue[0] != '\0')
    {
        loElemIterStatusSub = loElemIterStatus->AddElement("StatusText");
        loElemIterStatusSub->AddText(lpcTmpValue);
    }

    return 0;
}

apl_int_t CMM7Message::BuildSRReqSoapHead(void)
{
    return this->BuildVASPReqSoapHead();
}

apl_int_t CMM7Message::BuildDeliverReqSoapBody(CMM7DeliverReq* apoMsg)
{
    acl::CXmlElement::IteratorType loElemIterBody;
    loElemIterBody = this->moSoapXML.AddElement("env:Body");

    acl::CXmlElement::IteratorType loElemIterReq;
    loElemIterReq = loElemIterBody->AddElement(DELIVERREQ);
    loElemIterReq->AddAttribute("xmlns", MM7_SCHEMA);

    //Add MM7Version, can't be empty
    if(apoMsg->GetMM7Version()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterVer = loElemIterReq->AddElement("MM7Version");
        loElemIterVer->AddText(apoMsg->GetMM7Version());
    }
    else
    {
        return MM7_ERR_MM7VERSIONEMPTY;
    }

    if(apoMsg->GetMMSRelayServerID()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterVer = loElemIterReq->AddElement("MMSRelayServerID");
        loElemIterVer->AddText(apoMsg->GetMMSRelayServerID());
    }

    if(apoMsg->GetSenderAddress()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterSendAdd = loElemIterReq->AddElement("Sender");
        loElemIterSendAdd->AddText(apoMsg->GetSenderAddress());
    }
    else
    {
        return MM7_ERR_SENDERADDRESS;
    }

    //Add Recipients(To/Cc/Bcc), must one exist
    if( apoMsg->GetToCount() > 0
        || apoMsg->GetCcCount() > 0
        || apoMsg->GetBccCount() > 0 )
    {
        const char* lpcNumber = APL_NULL;

        acl::CXmlElement::IteratorType loElemIterRecipients = loElemIterReq->AddElement("Recipients");
        if(apoMsg->GetToCount() > 0)
        {
            acl::CXmlElement::IteratorType loElemIterTo = loElemIterRecipients->AddElement("To");

            for(apl_size_t liN=0; liN<apoMsg->GetToCount(); ++liN)
            {
                lpcNumber = apoMsg->GetTo(liN);
                if( apl_strrchr(lpcNumber, '@') != APL_NULL)
                {
                    acl::CXmlElement::IteratorType loElemIterRfc = loElemIterTo->AddElement("RFC2822Address");
                    loElemIterRfc->AddText(lpcNumber);
                }
                else
                {
                    acl::CXmlElement::IteratorType loElemIterNum = loElemIterTo->AddElement("Number");
                    loElemIterNum->AddText(lpcNumber);
                }
            }
        }

        if(apoMsg->GetCcCount() > 0)
        {
            acl::CXmlElement::IteratorType loElemIterCc = loElemIterRecipients->AddElement("Cc");

            for(apl_size_t liN=0; liN<apoMsg->GetCcCount(); ++liN)
            {
                lpcNumber = apoMsg->GetCc(liN);
                if( apl_strrchr(lpcNumber, '@') != APL_NULL)
                {
                    acl::CXmlElement::IteratorType loElemIterRfc = loElemIterCc->AddElement("RFC2822Address");
                    loElemIterRfc->AddText(lpcNumber);
                }
                else
                {
                    acl::CXmlElement::IteratorType loElemIterNum = loElemIterCc->AddElement("Number");
                    loElemIterNum->AddText(lpcNumber);
                }
            }
        }

        if(apoMsg->GetBccCount() > 0)
        {
            acl::CXmlElement::IteratorType loElemIterBcc = loElemIterRecipients->AddElement("Bcc");

            for(apl_size_t liN=0; liN<apoMsg->GetBccCount(); ++liN)
            {
                lpcNumber = apoMsg->GetBcc(liN);
                if( apl_strrchr(lpcNumber, '@') != APL_NULL)
                {
                    acl::CXmlElement::IteratorType loElemIterRfc = loElemIterBcc->AddElement("RFC2822Address");
                    loElemIterRfc->AddText(lpcNumber);
                }
                else
                {
                    acl::CXmlElement::IteratorType loElemIterNum = loElemIterBcc->AddElement("Number");
                    loElemIterNum->AddText(lpcNumber);
                }
            }
        }
    }
    /*
    else
    {
        return MM7_ERR_RECIPIENTSEMPTY;
    }
    */

    acl::CXmlElement::IteratorType loElemIterTmp;
    if(apoMsg->IsTimeStamp())
    {
        acl::CDateTime loDateTime;
        loDateTime.Update();
        const char* lpcTimestamp = loDateTime.Format(MM7_DATE_FORMAT);

        loElemIterTmp = loElemIterReq->AddElement("TimeStamp");
        loElemIterTmp->AddText(lpcTimestamp);
    }

    if(apoMsg->GetReplyChargingID()[0] != '\0')
    {
        acl::CXmlElement::IteratorType loElemIterServCode = loElemIterReq->AddElement("ReplyChargingID");
        loElemIterServCode->AddText(apoMsg->GetReplyChargingID());
    }

    //Add LinkedID if has
    if(apoMsg->GetLinkedID()[0] != '\0')
    {
        loElemIterTmp = loElemIterReq->AddElement("LinkedID");
        loElemIterTmp->AddText(apoMsg->GetLinkedID());
    }

    //Add Priority
    if(apoMsg->GetPriority()[0] != '\0')
    {
        loElemIterTmp = loElemIterReq->AddElement("Priority");
        loElemIterTmp->AddText(apoMsg->GetPriority());
    }

    //Add Subject
    loElemIterTmp = loElemIterReq->AddElement("Subject");
    if(apoMsg->GetSubject()[0] != '\0')
    {
        loElemIterTmp->AddText(apoMsg->GetSubject());
    }

    //Add Content
    if( apoMsg->IsContentExist() )
    {
        loElemIterTmp = loElemIterReq->AddElement("Content");
        char lacTmpBuf[64] = {0};
        apl_snprintf(lacTmpBuf, sizeof(lacTmpBuf), "\"cid:%s\"", apoMsg->GetContent()->GetContentID());
        loElemIterTmp->AddAttribute("href", lacTmpBuf);

    }

    return 0;
}


/**
 * CMM7VASPReq
 */
CMM7VASPReq::CMM7VASPReq(apl_int_t aiMessageType)
    : CMM7Message(aiMessageType)
    , moVASPID("")
    , moVASID("")
{
}

CMM7VASPReq::~CMM7VASPReq(void)
{
}

/**
 * CMM7RSReq
 */
CMM7RSReq::CMM7RSReq(void)
    : moMMSRelayServerID("")
{
}

CMM7RSReq::CMM7RSReq(apl_int_t aiMessageType)
    : CMM7Message(aiMessageType)
    , moMMSRelayServerID("")
{
}

CMM7RSReq::~CMM7RSReq(void)
{
}

/**
 * CMM7VASPRes
 */
CMM7VASPRes::CMM7VASPRes(apl_int_t aiMessageType)
    : CMM7Message(aiMessageType)
    , miStatusCode(0)
    , moStatusText("")
{
}

CMM7VASPRes::~CMM7VASPRes(void)
{
}

/**
 * CMM7RSRes
 */
CMM7RSRes::CMM7RSRes(apl_int_t aiMessageType)
    : CMM7Message(aiMessageType)
    , miStatusCode(0)
    , moStatusText("")
{
}

CMM7RSRes::~CMM7RSRes(void)
{
}

/**
 * CMM7SubmitReq
 */
CMM7SubmitReq::CMM7SubmitReq(void)
    : CMM7VASPReq(MSG_TYPE_MM7_SUBMIT_REQ)
    , moSenderAddress("")
    , moServiceCode("")
    , moLinkedID("")
    , moMessageClass("Personal")
    , mbIsTimeStamp(false)
    , moExpiryDateAbsolute("")
    , muExpiryDateDuration(0)
    , moEarliestDeliveryTimeAbsolute("")
    , muEarliestDeliveryTimeDuration(0)
    , mbIsDeliveryReport(false)
    , mbIsReadReply(false)
    , mbIsReplyCharging(false)
    , moReplyDeadlineAbsolute("")
    , muReplyDeadlineDuration(0)
    , muReplyChargingSize(0)
    , moPriority("Normal")
    , moSubject("")
    , mbIsAllowAdaptations(true)
    , moChargedParty("")
    , moChargedPartyID("")
    , mpoContent(APL_NULL)
    , mbIsDistributionIndicator(false)
    , mbIsContentExist(false)
{
}

CMM7SubmitReq::~CMM7SubmitReq(void)
{
    if(this->mbIsContentExist)
    {
        ACL_DELETE(this->mpoContent);
    }
}

void CMM7SubmitReq::SetContent(CMM7Content* apoContent)
{
    this->mpoContent = apoContent;
    this->mbIsContentExist = true;
}

CMM7Content* CMM7SubmitReq::GetContent(void)
{
    return this->mpoContent;
}

bool CMM7SubmitReq::IsContentExist(void)
{
    return this->mbIsContentExist;
}

void CMM7SubmitReq::PushToStr(const char* apcToStr, const char* apcDelimiter)
{
    acl::CSpliter loSpliter;
    loSpliter.Delimiter(apcDelimiter);
    loSpliter.Parse(apcToStr);
    for(apl_size_t luN=0; luN<loSpliter.GetSize(); ++luN)
    {
        this->PushTo(loSpliter.GetField(luN));
    }
}

void CMM7SubmitReq::SetTo(RecipientsListType& aoTo)
{
    this->moTo = aoTo;
}

apl_int_t CMM7SubmitReq::GetTo(RecipientsListType& aoTo)
{
    aoTo = this->moTo;
    return 0;
}

void CMM7SubmitReq::PushCcStr(const char* apcCcStr, const char* apcDelimiter)
{
    acl::CSpliter loSpliter;
    loSpliter.Delimiter(apcDelimiter);
    loSpliter.Parse(apcCcStr);
    for(apl_size_t luN=0; luN<loSpliter.GetSize(); ++luN)
    {
        this->PushCc(loSpliter.GetField(luN));
    }
}

void CMM7SubmitReq::SetCc(RecipientsListType& aoCc)
{
    this->moCc = aoCc;
}

void CMM7SubmitReq::GetCc(RecipientsListType& aoCc)
{
    aoCc = this->moCc;
}

void CMM7SubmitReq::PushBccStr(const char* apcBccStr, const char* apcDelimiter)
{
    acl::CSpliter loSpliter;
    loSpliter.Delimiter(apcDelimiter);
    loSpliter.Parse(apcBccStr);
    for(apl_size_t luN=0; luN<loSpliter.GetSize(); ++luN)
    {
        this->PushBcc(loSpliter.GetField(luN));
    }
}

void CMM7SubmitReq::SetBcc(RecipientsListType& aoBcc)
{
    this->moBcc = aoBcc;
}

apl_int_t CMM7SubmitReq::GetBcc(RecipientsListType& aoBcc)
{
    aoBcc = this->moBcc;
    return 0;
}

/**
 * CMM7SubmitRes
 */
CMM7SubmitRes::CMM7SubmitRes(void)
    : CMM7RSRes(MSG_TYPE_MM7_SUBMIT_RES)
{
}

CMM7SubmitRes::~CMM7SubmitRes(void)
{
}

/**
 * CMM7DeliverReq
 */
CMM7DeliverReq::CMM7DeliverReq(void)
    : CMM7RSReq(MSG_TYPE_MM7_DELIVER_REQ)
    , mbIsTimeStamp(true)
    , moPriority("Normal")
    , mpoContent(APL_NULL)
    , mbIsContentExist(false)
{
}

CMM7DeliverReq::~CMM7DeliverReq(void)
{
    if(this->mbIsContentExist)
    {
        ACL_DELETE(this->mpoContent);
    }
}

void CMM7DeliverReq::SetContent(CMM7Content* apoContent)
{
    this->mpoContent = apoContent;
    this->mbIsContentExist = true;
}

bool CMM7DeliverReq::IsContentExist(void)
{
    return this->mbIsContentExist;
}

CMM7Content* CMM7DeliverReq::GetContent(void)
{
    return this->mpoContent;
}

void CMM7DeliverReq::SetTo(RecipientsListType& aoTo)
{
    this->moTo = aoTo;
}

void CMM7DeliverReq::PushToStr(const char* apcToStr, const char* apcDelimiter)
{
    acl::CSpliter loSpliter;
    loSpliter.Delimiter(apcDelimiter);
    loSpliter.Parse(apcToStr);
    for(apl_size_t luN=0; luN<loSpliter.GetSize(); ++luN)
    {
        this->PushTo(loSpliter.GetField(luN));
    }
}

apl_int_t CMM7DeliverReq::GetTo(RecipientsListType& aoTo)
{
    aoTo = this->moTo;
    return 0;
}

void CMM7DeliverReq::SetCc(RecipientsListType& aoCc)
{
    this->moCc = aoCc;
}

void CMM7DeliverReq::PushCcStr(const char* apcCcStr, const char* apcDelimiter)
{
    acl::CSpliter loSpliter;
    loSpliter.Delimiter(apcDelimiter);
    loSpliter.Parse(apcCcStr);
    for(apl_size_t luN=0; luN<loSpliter.GetSize(); ++luN)
    {
        this->PushCc(loSpliter.GetField(luN));
    }
}

void CMM7DeliverReq::GetCc(RecipientsListType& aoCc)
{
    aoCc = this->moCc;
}

void CMM7DeliverReq::SetBcc(RecipientsListType& aoBcc)
{
    this->moBcc = aoBcc;
}

void CMM7DeliverReq::PushBccStr(const char* apcBccStr, const char* apcDelimiter)
{
    acl::CSpliter loSpliter;
    loSpliter.Delimiter(apcDelimiter);
    loSpliter.Parse(apcBccStr);
    for(apl_size_t luN=0; luN<loSpliter.GetSize(); ++luN)
    {
        this->PushBcc(loSpliter.GetField(luN));
    }
}

apl_int_t CMM7DeliverReq::GetBcc(RecipientsListType& aoBcc)
{
    aoBcc = this->moBcc;
    return 0;
}

/**
 * CMM7DeliverRes
 */
CMM7DeliverRes::CMM7DeliverRes(void)
    : CMM7VASPRes(MSG_TYPE_MM7_DELIVER_RES)
{
}

CMM7DeliverRes::~CMM7DeliverRes(void)
{
}

/**
 * CMM7CancelReq
 */

CMM7CancelReq::CMM7CancelReq(void)
    : CMM7VASPReq(MSG_TYPE_MM7_CANCLE_REQ)
{
}

CMM7CancelReq::~CMM7CancelReq(void)
{
}

/**
 * CMM7CancelRes
 */
CMM7CancelRes::CMM7CancelRes(void)
    : CMM7RSRes(MSG_TYPE_MM7_CANCLE_RES)
{
}

/**
 * CMM7ReplaceReq
 */
CMM7ReplaceReq::CMM7ReplaceReq(void)
    : CMM7VASPReq(MSG_TYPE_MM7_REPLACE_REQ)
    , mbIsReadReply(false)
    , mbIsAllowAdaptations(true)
    , mbIsDistributionIndicator(false)
    , mpoContent(APL_NULL)
    , mbIsContentExist(false)
{
}

CMM7ReplaceReq::~CMM7ReplaceReq(void)
{
    if(this->mbIsContentExist)
    {
        ACL_DELETE(this->mpoContent);
    }
}

void CMM7ReplaceReq::SetContent(CMM7Content* apoContent)
{
    this->mpoContent = apoContent;
    this->mbIsContentExist = true;
}

bool CMM7ReplaceReq::IsContentExist(void)
{
    return this->mbIsContentExist;
}

CMM7Content* CMM7ReplaceReq::GetContent(void)
{
    return this->mpoContent;
}

/**
 * CMM7ReplaceRes
 */
CMM7ReplaceRes::CMM7ReplaceRes(void)
    : CMM7RSRes(MSG_TYPE_MM7_REPLACE_RES)
{
}

/**
 * CMM7DeliveryReportReq
 */
CMM7DeliveryReportReq::CMM7DeliveryReportReq(void)
    : CMM7RSReq(MSG_TYPE_MM7_DELIVERY_REPORT_REQ)
{
}

CMM7DeliveryReportReq::~CMM7DeliveryReportReq(void)
{
}

/**
 * CMM7DeliveryReportRes
 */
CMM7DeliveryReportRes::CMM7DeliveryReportRes(void)
    : CMM7VASPRes(MSG_TYPE_MM7_DELIVERY_REPORT_RES)
{
}

/**
 * CMM7ReadReplyReq
 */
CMM7ReadReplyReq::CMM7ReadReplyReq(void)
    : CMM7RSReq(MSG_TYPE_MM7_READ_REPLAY_REQ)
{
}

/**
 * CMM7ReadReplyRes
 */
CMM7ReadReplyRes::CMM7ReadReplyRes(void)
    : CMM7VASPRes(MSG_TYPE_MM7_READ_REPLAY_RES)
{
}

/**
 * CMM7RSErrorRes
 */
CMM7RSErrorRes::CMM7RSErrorRes(void)
    : CMM7RSRes(MSG_TYPE_MM7_RS_ERROR_RES)
{
}

/**
 * CMM7VASPErrorRes
 */
CMM7VASPErrorRes::CMM7VASPErrorRes(void)
    : CMM7VASPRes(MSG_TYPE_MM7_VASP_ERROR_RES)
{
}

} //namespace mm7 

ANF_NAMESPACE_END
