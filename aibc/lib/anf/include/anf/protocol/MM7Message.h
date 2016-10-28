
/**
 * @file  MM7Message.h
 */

#ifndef MM7_MESSAGE_H
#define MM7_MESSAGE_H

#include "acl/stl/string.h"
#include "acl/stl/vector.h"
#include "acl/DateTime.h"
#include "acl/Timestamp.h"
#include "acl/SString.h"
#include "acl/StrAlgo.h"
#include "acl/MemoryBlock.h"
#include "acl/Xml.h"

#include "anf/Utility.h"

ANF_NAMESPACE_START

namespace mm7
{
/***************************************************************
                         Utility 
***************************************************************/
/**
 * Error Code
 */
#define MM7_ERR_TRANSIDEMPTY        -10     //TransactionID is empty
#define MM7_ERR_MM7VERSIONEMPTY     -11     //MM7Version is empty
#define MM7_ERR_SPIDORSIDEMPTY      -12     //VASPID or VASID is empty
#define MM7_ERR_RECIPIENTSEMPTY     -13     //To&Cc&Bcc all are empty
#define MM7_ERR_SENDERADDRESS       -14     //Sender address is empty
#define MM7_ERR_SERVCODEEMPTY       -15     //ServiceCode is empty
#define MM7_ERR_CONTENTTYPEEMPTY    -16     //ContentType is empty
#define MM7_ERR_MESSAGEID           -17     //MessageID is empty
#define MM7_ERR_RECIPIENTADDRESS    -18     //Recipient address is empty
#define MM7_ERR_TIMESTAMP           -19     //Date and time is empty
#define MM7_ERR_MMSTATUS            -20     //MM Status is empty
#define MM7_ERR_STATUSCODE          -21     //StatusCode is empty
#define MM7_ERR_STATUSTEXT          -22     //StatusText is empty

#define MM7_ERR_MISSCOMPULSORYELEMS -30     //miss compulsory elements
#define MM7_ERR_INVALIDSOAP         -31     //invaid soap xml package
#define MM7_ERR_INVALIDMESSAGETYPE  -32     //invaid MessageType
#define MM7_ERR_PARSEXMLFAIL        -33     //Parse soap env fail

//Encoder
#define MM7_ERR_INVALIDDATEFORMAT   -40     //invaid date format, the format should be 2009-09-15T19:20:40
#define MM7_ERR_ENCODEVASPREQ       -41     //encode VASP request failed
#define MM7_ERR_ENCODEVASPRES       -42     //encode VASP response failed
#define MM7_ERR_ENCODESRREQ         -43     //encode SR request failed
#define MM7_ERR_BUILDNORMALPART     -44     //build normal part failed

#define MM7_ERR_WRITE               -51     //Write

#define MM7_ERR_REQEMPTY            -105    //Send message is empty
#define MM7_ERR_INVALIDMESSAGE      -106    //Invalid message
#define MM7_ERR_TOOBIG              -113    //message is too big

/**
 * Warning
 */
#define MM7_WARN_NOT_ENOUGH_SPACE   90000

/**
 * Req/Res name macro
 */
static const char* const SUBMITREQ          = "SubmitReq";
static const char* const SUBMITRES          = "SubmitRsp";
static const char* const DELIVERREQ         = "DeliverReq";
static const char* const DELIVERRES         = "DeliverRsp";
static const char* const DELIVERYREPORTREQ  = "DeliveryReportReq";
static const char* const DELIVERYREPORTRES  = "DeliveryReportRsp";
static const char* const CANCELREQ          = "CancelReq";
static const char* const CANCELRES          = "CancelRsp";
static const char* const REPLACEREQ         = "ReplaceReq";
static const char* const REPLACERES         = "ReplaceRsp";
static const char* const READREPLYREQ       = "ReadReplyReq";
static const char* const READREPLYRES       = "ReadReplyRsp";
static const char* const RSERRORRES         = "RSErrorRsp";
static const char* const VASPERRORRES       = "VASPErrorRsp";

/**
 * misc macro
 */
static const char* const MM7API_VERSION    = "V1.0.0";
static const char* const MM7_SCHEMA        = "\"http://www.3gpp.org/ftp/Specs/archive/23_series/23.140/schema/REL-6-MM7-1-0\"";
static const char* const _3GPP_MMS_VERSION = "6.3.0";
static const char* const XML_COMMENT       = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

static const char* const NEXT_PART_ORG     = "NextPart----------Asiainfo--------------NextPart";
static const char* const NEXT_PART         = "NextPart----------Asiainfo--------------NextPart\r\n";
static const char* const SUB_PART          = "SubPart-------------Asiainfo-------------SubPart\r\n";

static const char* const MM7_DATE_FORMAT   = "%Y-%m-%dT%H:%M:%S";

/**
 * MIME
 */
static const char* const CRLFCRLF                    = "\r\n\r\n";
static const char* const CRLF                        = "\r\n";
static const char* const CONTENT_TYPE                = "Content-Type";
static const char* const CHARSET                     = "charset";
static const char* const CONTENT_TRANSFEREN_CODING   = "Content-Transfer-Encoding";
static const char* const CONTENT_ID                  = "Content-ID";
static const char* const CONTENT_LOCATION            = "Content-Location";
static const char* const CONTENT_LENGTH              = "Content-Length";

/**
 * HTTP - errors in return value
 */
#define AI_HTTP_ERROR_INVAL         (-90601)    ///< invalid args, (e.g. NULL pointer)
#define AI_HTTP_ERROR_NOTFOUND      (-90602)    ///< header field not found (clsHTTPMessage)
#define AI_HTTP_ERROR_TOOBIG        (-90603)    ///< buffer is too small
#define AI_HTTP_ERROR_EOL           (-90604)    ///< End-Of-Line not found
#define AI_HTTP_ERROR_STARTLINE     (-90605)    ///< other error in start-line
#define AI_HTTP_ERROR_BODY          (-90606)    ///< don't know how to recv message-body
#define AI_HTTP_ERROR_SIZE          (-90607)    ///< invalid Content-Length or chunk-size
#define AI_HTTP_ERROR_SOCKET        (-90608)    ///< socket error

#define AI_HTTP_ERROR_VERSION       (-90611)    ///< invalid version in start-line
#define AI_HTTP_ERROR_METHOD        (-90612)    ///< invalid method in request-line
#define AI_HTTP_ERROR_URI           (-90613)    ///< invalid uri in request-line
#define AI_HTTP_ERROR_STATUS        (-90614)    ///< invalid statut in response-line
#define AI_HTTP_ERROR_FIELDNAME     (-90615)    ///< invalid field name
#define AI_HTTP_ERROR_FIELDVALUE    (-90616)    ///< invalid field value
#define AI_HTTP_ERROR_FIELDSEP      (-90617)    ///< ':' not found in feild line
#define AI_HTTP_ERROR_LOCATION      (-90618)    ///< Location not found to redirect
#define AI_HTTP_ERROR_REDIRECT      (-90619)    ///< too many redirection

/**
 * MessageType
 */
const apl_int_t MSG_TYPE_MM7_SUBMIT_REQ          = 0;
const apl_int_t MSG_TYPE_MM7_SUBMIT_RES          = 1;
const apl_int_t MSG_TYPE_MM7_DELIVER_REQ         = 2;
const apl_int_t MSG_TYPE_MM7_DELIVER_RES         = 3;
const apl_int_t MSG_TYPE_MM7_CANCLE_REQ          = 4;
const apl_int_t MSG_TYPE_MM7_CANCLE_RES          = 5;
const apl_int_t MSG_TYPE_MM7_REPLACE_REQ         = 6;
const apl_int_t MSG_TYPE_MM7_REPLACE_RES         = 7;
const apl_int_t MSG_TYPE_MM7_DELIVERY_REPORT_REQ = 8;
const apl_int_t MSG_TYPE_MM7_DELIVERY_REPORT_RES = 9;
const apl_int_t MSG_TYPE_MM7_READ_REPLAY_REQ     = 10;
const apl_int_t MSG_TYPE_MM7_READ_REPLAY_RES     = 11;
const apl_int_t MSG_TYPE_MM7_RS_ERROR_RES        = 12;
const apl_int_t MSG_TYPE_MM7_VASP_ERROR_RES      = 13;

/**
 * priorityType
 */
static const char* const LOW    = "LOW";
static const char* const NORMAL = "NORMAL";
static const char* const HIGH   = "HIGH";

//chargedPartyType
static const char* const NEITHER     = "Neither";
static const char* const SENDER      = "Sender";
static const char* const RECIPIENT   = "Recipient";
static const char* const BOTH        = "Both";
static const char* const THIRD_PARTY = "ThirdParty";

//messageClassType
static const char* const PERSONAL      = "Personal";
static const char* const INFORMATIONAL = "Informational";
static const char* const ADVERTISEMENT = "Advertisement";
static const char* const AUTO          = "Auto";

//DeliveryStatusType
static const char* const EXPIRED       = "Expired";
static const char* const RETRIEVED     = "Retrieved";
static const char* const Rejected      = "Rejected";
static const char* const INDETERMINATE = "Indeterminate";
static const char* const FORWARDED     = "Forwarded";
static const char* const UNRECOGNISED  = "Unrecognised";
static const char* const DEFERRED      = "Deferred";

//ReadStatusType
static const char* const READ          = "Read";
static const char* const DELETED       = "Deleted";
//static const char* const INDETERMINATE = "Indeterminate";


/**
 * RequestStatus
 */
static const apl_int_t SUCCESS                  = 1000;
static const apl_int_t PARTIAL_SUCCESS          = 1001;
static const apl_int_t CLIENT_ERROR             = 2000;
static const apl_int_t OPERATION_RESTRICTED     = 2001;
static const apl_int_t ADDRESS_ERROR            = 2002;
static const apl_int_t ADDRESS_NOTFOUND         = 2003;
static const apl_int_t CONTENT_REFUSED          = 2004;
static const apl_int_t MESSAGEID_NOTFOUND       = 2005;
static const apl_int_t LINKEDID_NOTFOUND        = 2006;
static const apl_int_t MESSAGE_FORMAT_CORRUPT   = 2007;
static const apl_int_t SERVER_ERROR             = 3000;
static const apl_int_t NOT_POSSIBLE             = 3001;
static const apl_int_t MESSAGE_REJECTED         = 3002;
static const apl_int_t MULTIADDRESS_NOTSUPPORT  = 3003;
static const apl_int_t GENERAL_SERVICE_ERROR    = 4000;
static const apl_int_t IMPROPER_ID              = 4001;
static const apl_int_t UNSUPPORT_VERSION        = 4002;
static const apl_int_t UNSUPPORT_OPERATION      = 4003;
static const apl_int_t VALIDATION_ERROR         = 4004;
static const apl_int_t SERVICE_ERROR            = 4005;
static const apl_int_t SERVICE_UNAVAILABLE      = 4006;
static const apl_int_t SERVICE_DENIED           = 4007;
static const char* const TEXT_SUCCESS                 = "Success";
static const char* const TEXT_PARTIAL_SUCCESS         = "Partial Success";
static const char* const TEXT_CLIENT_ERROR            = "Client error";
static const char* const TEXT_OPERATION_RESTRICTED    = "Operation restricted";
static const char* const TEXT_ADDRESS_ERROR           = "Address error";
static const char* const TEXT_ADDRESS_NOTFOUND        = "Address not found";
static const char* const TEXT_CONTENT_REFUSED         = "Content refused";
static const char* const TEXT_MESSAGEID_NOTFOUND      = "MessageID not found";
static const char* const TEXT_LINKEDID_NOTFOUND       = "LinkedID not found";
static const char* const TEXT_MESSAGE_FORMAT_CORRUPT  = "Message format corrupt";
static const char* const TEXT_SERVER_ERROR            = "Server error";
static const char* const TEXT_NOT_POSSIBLE            = "Not possible";
static const char* const TEXT_MESSAGE_REJECTED        = "Message rejected";
static const char* const TEXT_MULTIADDRESS_NOTSUPPORT = "Multi-address not support";
static const char* const TEXT_GENERAL_SERVICE_ERROR   = "General service error";
static const char* const TEXT_IMPROPER_ID             = "Improper ID";
static const char* const TEXT_UNSUPPORT_VERSION       = "Unsupport version";
static const char* const TEXT_UNSUPPORT_OPERATION     = "Unsupport operation";
static const char* const TEXT_VALIDATION_ERROR        = "Validation error";
static const char* const TEXT_SERVICE_ERROR           = "Service error";
static const char* const TEXT_SERVICE_UNAVAILABLE     = "Service unavailable";
static const char* const TEXT_SERVICE_DENIED          = "Service denied";

///operator macro
#define MM7_DECLARE_MEMBER_OPERATOR_N( member ) \
    void Set##member( apl_int_t ai##member )              { this->mi##member = ai##member; } \
    apl_int_t  Get##member()const                         { return this->mi##member; }

#define MM7_DECLARE_MEMBER_OPERATOR_U( member ) \
    void Set##member( apl_size_t au##member )             { this->mu##member = au##member; } \
    apl_size_t  Get##member()const                        { return this->mu##member; }

#define MM7_DECLARE_MEMBER_OPERATOR_U16( member ) \
    void Set##member( apl_uint16_t au16##member )         { this->mu16##member = au16##member; } \
    apl_uint16_t  Get##member()const                      { return this->mu16##member; }

#define MM7_DECLARE_MEMBER_OPERATOR_U32( member ) \
    void Set##member( apl_uint32_t au##member )           { this->mu##member = au##member; } \
    apl_uint32_t  Get##member()const                      { return this->mu##member; }

#define MM7_DECLARE_MEMBER_OPERATOR_B( member ) \
    void Set##member( bool abIs##member )                 { this->mbIs##member = abIs##member; } \
    bool  Is##member()const                               { return this->mbIs##member; }

#define MM7_DECLARE_MEMBER_OPERATOR_S( member ) \
    void Set##member( const char* apc##member )           { this->mo##member = apc##member; acl::stralgo::TrimIf(this->mo##member, IsQuote); } \
    const char* Get##member()const                        { return this->mo##member.c_str(); }

#define MM7_DECLARE_MEMBER_OPERATOR_CS( member ) \
    void Set##member( const char* apc##member )           { apl_strncpy( this->mac##member, sizeof(this->mac##member), "%s", apc##member ); acl::stralgo::TrimIf(this->mac##member, IsQuote); } \
    const char* Get##member()const                        { return this->mac##member; }

#define MM7_DECLARE_MEMBER_OPERATOR_VS( member ) \
    void Push##member( const char* apc##member )          { this->mo##member.push_back( apc##member ); acl::stralgo::TrimIf(this->mo##member.back(), IsQuote); } \
    size_t Get##member##Count()const                      { return this->mo##member.size(); } \
    const char* Get##member( apl_size_t aiN )const        { return this->mo##member[aiN].c_str(); }

#define MM7_DECLARE_MEMBER_OPERATOR_MB( member ) \
    acl::CMemoryBlock& Get##member()                      { return this->mo##member; }
///end

//inline function
inline bool IsNumber( char ac ) { return (ac >= '0' && ac <= '9'); }
inline bool IsQuote( char ac )  { return (ac == ' ' || ac == '"' || ac == '\''); }
inline bool IsSpace( char ac )  { return ac == ' ';}
inline bool IsClear( char ac )  { return IsSpace(ac) || IsQuote(ac);  }

#define MM7_RETURN_IF( ret, op ) if(op) { return ret; }

#define BUFFER_WRITE(buf, content) \
do \
{ \
    if(buf->Write(content, apl_strlen(content)) != (apl_ssize_t)apl_strlen(content)) \
    { \
        return MM7_ERR_WRITE; \
    } \
}while(0)

#define BUFFER_WRITE_LEN(buf, content, len) \
do \
{ \
    if(buf->Write(content, len) != (apl_ssize_t)len) \
    { \
        return MM7_ERR_WRITE; \
    } \
}while(0)

/***************************************************************
                         MM7Content
***************************************************************/
/**
 * @class CMM7ContentType
 *
 * @brief The content-type class
 */
class CMM7ContentType
{
public:
    CMM7ContentType(void);

    CMM7ContentType(std::string aoType);

    ~CMM7ContentType(void);

    MM7_DECLARE_MEMBER_OPERATOR_S(PrimaryType);
    MM7_DECLARE_MEMBER_OPERATOR_S(SubType);

public:
    friend bool operator ==(CMM7ContentType& aoLhs, CMM7ContentType& aoRhs);

    bool operator ==(std::string aoType);

private:
    std::string moPrimaryType;

    std::string moSubType;
};

inline bool operator ==(CMM7ContentType& aoLhs, CMM7ContentType& aoRhs)
{
    do
    {
        if( apl_strcmp(aoLhs.GetPrimaryType(), aoRhs.GetPrimaryType()) != 0 )
        {
            break;
        }

        if( apl_strcmp(aoLhs.GetSubType(), aoRhs.GetSubType()) != 0 )
        {
            break;
        }

        return true;

    }while(false);

    return false;
}

/**
 * @class CMM7Content
 *
 * @brief The content class.
 */
class CMM7Content
{
public:
    typedef std::vector<CMM7Content*> ContentListType;

public:
    CMM7Content(void);

    CMM7Content(char* apcBuffer);

    CMM7Content(char* apcBuffer, apl_size_t auBufferSize);

    ~CMM7Content(void);

    //add filename 2010-01-11
    MM7_DECLARE_MEMBER_OPERATOR_S(FileName);

    /**
     * @brief  Get/Set content charset
     */
    MM7_DECLARE_MEMBER_OPERATOR_S(Charset);

    /**
     * @brief  Get/Set content Content-Transfer-Encoding
     */
    MM7_DECLARE_MEMBER_OPERATOR_S(ContentTransferEncoding);

    /**
     * @brief  Get/Set content Content-ID
     */
    MM7_DECLARE_MEMBER_OPERATOR_S(ContentID);

    /**
     * @brief  Get/Set content charset
     */
    MM7_DECLARE_MEMBER_OPERATOR_S(ContentLocation);

    /**
     * @brief  Get/Set the content is multipart or not
     */
    MM7_DECLARE_MEMBER_OPERATOR_B(Multipart);

    /**
     * @brief  Get/Set the content is need encode or not
     */
    MM7_DECLARE_MEMBER_OPERATOR_B(Base64Encode);

public:
    
    /**
     * @brief  Add sub-content to the MM content
     */
    void AddSubContent(CMM7Content* apoContent);

    /**
     * @brief  Get sub-content by the Content-ID
     *
     * @param[IN]  aoContentID - The specified Content-ID
     *
     * @return The sub-content
     */
    CMM7Content* GetSubContentByID(std::string aoContentID);

    /**
     * @brief  Get sub-content by the Content-Location
     *
     * @param[IN]  aoContentLocation - the specified Content-Location
     *
     * @return The sub-content.
     */
    CMM7Content* GetSubContentByLocation(std::string aoContentLocation);

    /**
     * @brief  Get sub-content list
     *
     * @return The sub-content list
     */
    ContentListType* GetSubContents(void);

    /**
     * @brief  Get content buffer
     *
     * @return The content buffer
     */
    acl::CMemoryBlock* GetContentBuffer(void);

    
    /**
     * @brief  Set content buffer
     *
     * @param[IN]  apoBuffer - the buffer to be set
     */
    void SetContentBuffer(acl::CMemoryBlock* apoBuffer);

    /**
     * @brief  Set Content-Type, Content-Type should be NEWed
     *
     * @param[IN]  apoType - pointer to the Content-Type object
     */
    void SetContentType(CMM7ContentType* apoType);

    /**
     * @brief  Set Content-Type
     *
     * @param[IN]  aoType - The Content-Type
     */
    void SetContentType(std::string aoType);

    /**
     * @brief  Get Content-TYpe
     *
     * @return The pointer to the Content-Type object
     */
    CMM7ContentType* GetContentType(void);

private:
    CMM7ContentType* mpoContentType;

    std::string moFileName;

    std::string moCharset;

    std::string moContentTransferEncoding;

    std::string moContentID;

    std::string moContentLocation;

    acl::CMemoryBlock* mpoContentBuffer;

    bool mbIsMultipart;

    bool mbIsBase64Encode;

    ContentListType moSubContents;
};

/***************************************************************
                         SmilGenerator
***************************************************************/
/**
 * @class CSmilGenerator
 *
 * @brief Create smil file
 */
class CSmilGenerator
{
public:
    struct CMeta
    {
        CMeta()
            : moName("title")
            , moContent("mm7")
        {}

        CMeta(std::string aoName, std::string aoContent)
            : moName(aoName)
            , moContent(aoContent)
        {}

        std::string moName;
        std::string moContent;
    };
    typedef std::vector<CMeta> MetaListType;

    struct CRegion
    {
        CRegion()
            : moId("")
            , moWidth("100%")
            , moHeight("100%")
            , moLeft("0")
            , moTop("0")
            , moFit("meet")
        {}

        CRegion(
            std::string aoId, 
            std::string aoWidth, 
            std::string aoHeight, 
            std::string aoLeft, 
            std::string aoTop, 
            std::string aoFit)
            : moId(aoId)
            , moWidth(aoWidth)
            , moHeight(aoHeight)
            , moLeft(aoLeft)
            , moTop(aoTop)
            , moFit(aoFit)
        {}

        std::string moId;
        std::string moWidth;
        std::string moHeight;
        std::string moLeft;
        std::string moTop; 
        std::string moFit;
    };
    typedef std::vector<CRegion> RegionListType;

    struct CFrame
    {
        CFrame()
            : moDur("3000ms")
        {}

        struct CNode
        {
            CNode()
                : moSrc("")
                , moRegion("")
            {}

            CNode(std::string aoSrc, std::string aoRegion)
                : moSrc(aoSrc)
                , moRegion(aoRegion)
            {}

            CNode(std::string aoSrc)
                : moSrc(aoSrc)
                , moRegion("")
            {}
            
            std::string moSrc;
            std::string moRegion;
        };

        void AddNode(CNode& aoNode)
        {
            moNodeList.push_back(aoNode);
        }

        void AddNodeArray(CNode* apoNode, apl_size_t auNodeSize)
        {
            for(apl_size_t liN=0; liN<auNodeSize; ++liN)
            {
                if(apoNode + liN == APL_NULL)
                {
                    break;
                }
                moNodeList.push_back(apoNode[liN]);
            }
        }

        std::string moDur;
        std::vector<CNode> moNodeList;
    };
    typedef std::vector<CFrame>   FrameListType;
    typedef std::vector<CFrame::CNode>  NodeListType;

public:
    
    /**
     * @brief  A constructor.
     */
    CSmilGenerator(void);

    /**
     * @brief  A destructor.
     */
    virtual ~CSmilGenerator(void);

    /**
     * @brief  Get the smil buffer.
     *
     * @param[IN/OUT] appcSmil - the pointer to the smil buffer
     */
    apl_int_t SmilGenerator(const char** appcSmil);

protected:
    apl_int_t SmilBuildHead(void);

    virtual apl_int_t SmilBuildBody(void) = 0;

public:
    MM7_DECLARE_MEMBER_OPERATOR_S(Width);
    MM7_DECLARE_MEMBER_OPERATOR_S(Height);

    void AddMeta(CMeta& aoMeta);

    void AddRegion(CRegion& aoRegion);

    void AddFrame(CFrame& aoFrame);

    void AddFrameArray(CFrame* apoFrame, apl_size_t auFrameSize);

    acl::CXmlRootElement* GetRootElement(void);

protected:
    std::string  moWidth;
    std::string  moHeight;
    acl::CXmlRootElement moRootElement;
    MetaListType   moMetaList;
    RegionListType moRegionList;
    FrameListType   moFrameList;

private:
    char* mpcSmilBuffer;
};

/**
 * @class CSmilMusicShare
 *
 * @brief MM with one background music
 */
class CSmilMusicShare : public CSmilGenerator
{
public:
    void SetShareMusic(CFrame::CNode& aoShareMusic);

protected:
    virtual apl_int_t SmilBuildBody(void);

private:
    CFrame::CNode  moShareMusic;
};

/**
 * @class CSmilMusicIndependence
 *
 * @brief Each frame has a background music
 */
class CSmilMusicIndependence : public CSmilGenerator
{
protected:
    virtual apl_int_t SmilBuildBody(void);
};

/***************************************************************
                         MM7Message
***************************************************************/
/**
 * @class CMM7Message
 */
class CMM7SubmitReq;
class CMM7SubmitRes;
class CMM7DeliverReq;
class CMM7DeliverRes;
class CMM7CancelReq;
class CMM7CancelRes;
class CMM7ReplaceReq;
class CMM7ReplaceRes;
class CMM7DeliveryReportReq;
class CMM7DeliveryReportRes;
class CMM7ReadReplyReq;
class CMM7ReadReplyRes;
class CMM7RSErrorRes;
class CMM7VASPErrorRes;
class CMM7Message
{
public:
    CMM7Message(void);

    CMM7Message(apl_int_t aiMessageType);

    virtual ~CMM7Message(void);

    MM7_DECLARE_MEMBER_OPERATOR_S(TransactionID);
    MM7_DECLARE_MEMBER_OPERATOR_S(MM7Version);

    MM7_DECLARE_MEMBER_OPERATOR_N(MessageType);

    MM7_DECLARE_MEMBER_OPERATOR_S(HostIp);
    MM7_DECLARE_MEMBER_OPERATOR_U16(HostPort);

public:
    apl_ssize_t Encode(acl::CMemoryBlock* apoBuffer);

    apl_ssize_t Decode(const acl::CMemoryBlock* apoBuffer);

private:
    apl_ssize_t EncodeVASPReq(acl::CMemoryBlock* apoBuffer);

    apl_ssize_t EncodeVASPRes(acl::CMemoryBlock* apoBuffer);

    apl_ssize_t EncodeRSReq(acl::CMemoryBlock* apoBuffer);

    apl_ssize_t EncodeReqNormalPart(acl::CMemoryBlock* apoBuffer);

    apl_ssize_t EncodeVASPReqSoapXML(acl::CMemoryBlock* apoBuffer);

    apl_ssize_t EncodeVASPResSoapXML(acl::CMemoryBlock* apoBuffer);

    apl_ssize_t EncodeSRReqSoapXML(acl::CMemoryBlock* apoBuffer);

    apl_ssize_t EncodeVASPReqContentIfHas(acl::CMemoryBlock* apoBuffer);

private:
    apl_int_t BuildVASPReqSoapHead(void);

    apl_int_t BuildSubmitReqSoapBody(CMM7SubmitReq* apoMsg);

    apl_int_t BuildReplaceReqSoapBody(CMM7ReplaceReq* apoMsg);

    apl_int_t BuildCancelReqSoapBody(CMM7CancelReq* apoMsg);

private:
    apl_int_t BuildVASPResSoapHead(void);

    apl_int_t BuildDeliverResSoapBody(CMM7DeliverRes* apoMsg);

    apl_int_t BuildDeliveryReportResSoapBody(CMM7DeliveryReportRes* apoMsg);

    apl_int_t BuildReadReplyResSoapBody(CMM7ReadReplyRes* apoMsg);

    apl_int_t BuildVASPErrorResSoapBody(CMM7VASPErrorRes* apoMsg);

private:
    apl_int_t BuildSRReqSoapHead(void);

private:
    apl_int_t BuildRSReqSoapHead(void);

    apl_int_t BuildDeliverReqSoapBody(CMM7DeliverReq* apoMsg);

private:
    std::string     moTransactionID;

    std::string     moMM7Version;

    apl_int_t       miMessageType;

    //just for VASPReq
    std::string     moHostIp;
    apl_uint16_t    mu16HostPort;

    acl::CXmlRootElement moSoapXML;
};

/**
 * @class CMM7VASPReq
 *
 * @brief
 */
class CMM7VASPReq : public CMM7Message
{
public:
    CMM7VASPReq(apl_int_t aiMessageType);

    virtual ~CMM7VASPReq(void);

    MM7_DECLARE_MEMBER_OPERATOR_S(VASPID);
    MM7_DECLARE_MEMBER_OPERATOR_S(VASID);

private:
    std::string moVASPID;

    std::string moVASID;
};

/**
 * @class MM7RSReq
 *
 * @brief
 */
class CMM7RSReq : public CMM7Message
{
public:
    CMM7RSReq(void);

    CMM7RSReq(apl_int_t aiMessageType);

    virtual ~CMM7RSReq(void);

    MM7_DECLARE_MEMBER_OPERATOR_S(MMSRelayServerID);

private:
    std::string moMMSRelayServerID;
};

/**
 * @class CMM7VASPRes
 *
 * @brief
 */
class CMM7VASPRes : public CMM7Message
{
public:
    CMM7VASPRes(apl_int_t aiMessageType);

    virtual ~CMM7VASPRes(void);

    MM7_DECLARE_MEMBER_OPERATOR_N(StatusCode);

    MM7_DECLARE_MEMBER_OPERATOR_S(StatusText);

private:
    apl_int_t miStatusCode;

    std::string moStatusText;
};

/**
 * @class CMM7RSRes
 *
 * @brief
 */
class CMM7RSRes : public CMM7Message
{
public:
    CMM7RSRes(void)
    {}

    CMM7RSRes(apl_int_t aiMessageType);

    virtual ~CMM7RSRes(void);

    MM7_DECLARE_MEMBER_OPERATOR_N(StatusCode);

    MM7_DECLARE_MEMBER_OPERATOR_S(StatusText);

private:
    apl_int_t miStatusCode;

    std::string moStatusText;
};

/**
 * @class CMM7SubmitReq
 *
 * @brief 
 */
class CMM7SubmitReq : public CMM7VASPReq
{
public:
    typedef std::vector<std::string> RecipientsListType;

public:
    CMM7SubmitReq(void);

    ~CMM7SubmitReq(void);

public:
    MM7_DECLARE_MEMBER_OPERATOR_S(SenderAddress);

    MM7_DECLARE_MEMBER_OPERATOR_VS(To);
    MM7_DECLARE_MEMBER_OPERATOR_VS(Cc);
    MM7_DECLARE_MEMBER_OPERATOR_VS(Bcc);

    MM7_DECLARE_MEMBER_OPERATOR_S(ServiceCode);
    MM7_DECLARE_MEMBER_OPERATOR_S(LinkedID);
    MM7_DECLARE_MEMBER_OPERATOR_S(MessageClass);

    MM7_DECLARE_MEMBER_OPERATOR_B(TimeStamp);

    MM7_DECLARE_MEMBER_OPERATOR_S(ExpiryDateAbsolute);
    MM7_DECLARE_MEMBER_OPERATOR_U(ExpiryDateDuration);

    MM7_DECLARE_MEMBER_OPERATOR_S(EarliestDeliveryTimeAbsolute);
    MM7_DECLARE_MEMBER_OPERATOR_U(EarliestDeliveryTimeDuration);

    MM7_DECLARE_MEMBER_OPERATOR_B(DeliveryReport);
    MM7_DECLARE_MEMBER_OPERATOR_B(ReadReply);

    MM7_DECLARE_MEMBER_OPERATOR_B(ReplyCharging);
    MM7_DECLARE_MEMBER_OPERATOR_S(ReplyDeadlineAbsolute);
    MM7_DECLARE_MEMBER_OPERATOR_U(ReplyDeadlineDuration);
    MM7_DECLARE_MEMBER_OPERATOR_U(ReplyChargingSize);

    MM7_DECLARE_MEMBER_OPERATOR_S(Priority);
    MM7_DECLARE_MEMBER_OPERATOR_S(Subject);

    MM7_DECLARE_MEMBER_OPERATOR_B(AllowAdaptations);

    MM7_DECLARE_MEMBER_OPERATOR_S(ChargedParty);
    MM7_DECLARE_MEMBER_OPERATOR_S(ChargedPartyID);

    MM7_DECLARE_MEMBER_OPERATOR_B(DistributionIndicator);

public:
    void SetContent(CMM7Content* apoContent);

    CMM7Content* GetContent(void);

    bool IsContentExist(void);

    void SetTo(RecipientsListType& aoTo);

    apl_int_t GetTo(RecipientsListType& aoTo);

    void PushToStr(const char* apcToStr, const char* apcDelimiter = ",");

    void SetCc(RecipientsListType& aoCc);

    void GetCc(RecipientsListType& aoCc);

    void PushCcStr(const char* apcCcStr, const char* apcDelimiter = ",");

    void SetBcc(RecipientsListType& aoBcc);

    apl_int_t GetBcc(RecipientsListType& aoBcc);

    void PushBccStr(const char* apcBccStr, const char* apcDelimiter = ",");

private:
    std::string moSenderAddress;

    RecipientsListType moTo;

    RecipientsListType moCc;

    RecipientsListType moBcc;

    std::string moServiceCode;

    std::string moLinkedID;

    std::string moMessageClass;

    bool mbIsTimeStamp;

    std::string moExpiryDateAbsolute;
    apl_size_t  muExpiryDateDuration;

    std::string moEarliestDeliveryTimeAbsolute;
    apl_size_t  muEarliestDeliveryTimeDuration;

    bool mbIsDeliveryReport;

    bool mbIsReadReply;

    bool mbIsReplyCharging;

    std::string moReplyDeadlineAbsolute;
    apl_size_t  muReplyDeadlineDuration;

    apl_size_t muReplyChargingSize;

    std::string moPriority;

    std::string moSubject;

    bool mbIsAllowAdaptations;

    std::string moChargedParty;

    std::string moChargedPartyID;

    CMM7Content* mpoContent;

    bool mbIsDistributionIndicator;

private:
    bool mbIsContentExist;
};

/**
 * @class CMM7SubmitRes
 *
 * @brief
 */
class CMM7SubmitRes : public CMM7RSRes
{
public:
    CMM7SubmitRes(void);

    ~CMM7SubmitRes(void);

    MM7_DECLARE_MEMBER_OPERATOR_S(MessageID);

private:
    std::string moMessageID;
};

/**
 * @class CMM7DeliverReq
 *
 * @brief
 */
class CMM7DeliverReq : public CMM7RSReq
{
public:
    typedef std::vector<std::string> RecipientsListType;

public:
    CMM7DeliverReq(void);

    ~CMM7DeliverReq(void);

    MM7_DECLARE_MEMBER_OPERATOR_S(LinkedID);
    MM7_DECLARE_MEMBER_OPERATOR_S(SenderAddress);

    MM7_DECLARE_MEMBER_OPERATOR_VS(To);
    MM7_DECLARE_MEMBER_OPERATOR_VS(Cc);
    MM7_DECLARE_MEMBER_OPERATOR_VS(Bcc);

    //send
    MM7_DECLARE_MEMBER_OPERATOR_B(TimeStamp);
    //recv
    MM7_DECLARE_MEMBER_OPERATOR_S(TimeStamp);

    MM7_DECLARE_MEMBER_OPERATOR_S(ReplyChargingID);
    MM7_DECLARE_MEMBER_OPERATOR_S(Priority);
    MM7_DECLARE_MEMBER_OPERATOR_S(Subject);

public:
    void SetContent(CMM7Content* apoContent);

    bool IsContentExist(void);

    CMM7Content* GetContent(void);

    void SetTo(RecipientsListType& aoTo);

    void PushToStr(const char* apcToStr, const char* apcDelimiter);

    apl_int_t GetTo(RecipientsListType& aoTo);

    void SetCc(RecipientsListType& aoCc);

    void PushCcStr(const char* apcCcStr, const char* apcDelimiter);

    void GetCc(RecipientsListType& aoCc);

    void SetBcc(RecipientsListType& aoBcc);

    void PushBccStr(const char* apcBccStr, const char* apcDelimiter);

    apl_int_t GetBcc(RecipientsListType& aoBcc);

private:
    std::string moLinkedID;

    std::string moSenderAddress;

    RecipientsListType moTo;

    RecipientsListType moCc;

    RecipientsListType moBcc;

    //send
    bool        mbIsTimeStamp;
    //recv
    std::string moTimeStamp;

    std::string moReplyChargingID;

    std::string moPriority;

    std::string moSubject;

    CMM7Content* mpoContent;

    bool mbIsContentExist;
};

/**
 * @class CMM7DeliverRes
 *
 * @brief
 */
class CMM7DeliverRes : public CMM7VASPRes
{
public:
    CMM7DeliverRes(void);

    ~CMM7DeliverRes(void);

    MM7_DECLARE_MEMBER_OPERATOR_S(ServiceCode);

private:
    std::string moServiceCode;
};

/**
 * @class CMM7CancelReq
 *
 * @brief
 */
class CMM7CancelReq : public CMM7VASPReq
{
public:
    CMM7CancelReq(void);

    ~CMM7CancelReq(void);

    MM7_DECLARE_MEMBER_OPERATOR_S(SenderAddress);
    MM7_DECLARE_MEMBER_OPERATOR_S(MessageID);

private:
    std::string moSenderAddress;

    std::string moMessageID;
};

/**
 * @class CMM7CancelRes
 *
 * @brief
 */
class CMM7CancelRes : public CMM7RSRes
{
public:
    CMM7CancelRes(void);
};

/**
 * @class CMM7ReplaceReq
 *
 * @brief
 */
class CMM7ReplaceReq : public CMM7VASPReq
{
public:
    CMM7ReplaceReq(void);

    ~CMM7ReplaceReq(void);

    MM7_DECLARE_MEMBER_OPERATOR_S(MessageID);
    MM7_DECLARE_MEMBER_OPERATOR_S(ServiceCode);

    MM7_DECLARE_MEMBER_OPERATOR_B(TimeStamp);

    MM7_DECLARE_MEMBER_OPERATOR_S(EarliestDeliveryTimeAbsolute);
    MM7_DECLARE_MEMBER_OPERATOR_U(EarliestDeliveryTimeDuration);

    MM7_DECLARE_MEMBER_OPERATOR_B(ReadReply);
    MM7_DECLARE_MEMBER_OPERATOR_B(AllowAdaptations);
    MM7_DECLARE_MEMBER_OPERATOR_B(DistributionIndicator);

public:
    void SetContent(CMM7Content* apoContent);

    CMM7Content* GetContent(void);

    bool IsContentExist(void);

private:
    std::string moMessageID;

    std::string moServiceCode;

    bool mbIsTimeStamp;

    std::string moEarliestDeliveryTimeAbsolute;
    apl_size_t  muEarliestDeliveryTimeDuration;

    bool mbIsReadReply;

    bool mbIsAllowAdaptations;

    bool mbIsDistributionIndicator;

    CMM7Content* mpoContent;

private:
    bool mbIsContentExist;
};

/**
 * @class CMM7ReplaceRes
 *
 * @brief
 */
class CMM7ReplaceRes : public CMM7RSRes
{
public:
    CMM7ReplaceRes(void);
};

/**
 * @class CMM7DeliveryReportReq
 *
 * @brief
 */
class CMM7DeliveryReportReq : public CMM7RSReq
{
public:
    CMM7DeliveryReportReq(void);

    ~CMM7DeliveryReportReq(void);

    MM7_DECLARE_MEMBER_OPERATOR_S(MessageID);
    MM7_DECLARE_MEMBER_OPERATOR_S(RecipientAddress);
    MM7_DECLARE_MEMBER_OPERATOR_S(SenderAddress);
    MM7_DECLARE_MEMBER_OPERATOR_S(TimeStamp);
    MM7_DECLARE_MEMBER_OPERATOR_S(MMStatus);
    MM7_DECLARE_MEMBER_OPERATOR_S(StatusText);

    MM7_DECLARE_MEMBER_OPERATOR_N(StatusErrorCode);

private:
    std::string moMessageID;

    std::string moRecipientAddress;

    std::string moSenderAddress;

    std::string moTimeStamp;

    apl_int_t   miStatusErrorCode;

    std::string moStatusText;

    std::string moMMStatus;
};

/**
 * @class CMM7DeliveryReportRes
 *
 * @brief
 */
class CMM7DeliveryReportRes : public CMM7VASPRes
{
public:
    CMM7DeliveryReportRes(void);
};

/**
 * @class CMM7ReadReplyReq
 *
 * @brief
 */
class CMM7ReadReplyReq : public CMM7RSReq
{
public:
    CMM7ReadReplyReq(void);

    MM7_DECLARE_MEMBER_OPERATOR_S(MessageID);
    MM7_DECLARE_MEMBER_OPERATOR_S(RecipientAddress);
    MM7_DECLARE_MEMBER_OPERATOR_S(SenderAddress);
    MM7_DECLARE_MEMBER_OPERATOR_S(TimeStamp);
    MM7_DECLARE_MEMBER_OPERATOR_S(MMStatus);
    MM7_DECLARE_MEMBER_OPERATOR_S(StatusText);

private:
    std::string moMessageID;

    std::string moRecipientAddress;

    std::string moSenderAddress;

    std::string moTimeStamp;

    std::string moMMStatus;

    std::string moStatusText;
};

/**
 * @class CMM7ReadReplyRes
 *
 * @brief
 */
class CMM7ReadReplyRes : public CMM7VASPRes
{
public:
    CMM7ReadReplyRes(void);
};

/**
 * @class CMM7RSErrorRes
 *
 * @brief
 */
class CMM7RSErrorRes : public CMM7RSRes
{
public:
    CMM7RSErrorRes(void);
};

/**
 * @class CMM7VASPErrorRes
 *
 * @brief
 */
class CMM7VASPErrorRes : public CMM7VASPRes
{
public:
    CMM7VASPErrorRes(void);
};

} //end namespace mm7

ANF_NAMESPACE_END

#endif //MM7_MESSAGE_H
