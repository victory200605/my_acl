#ifndef AIMC_EMPROXY_POPCLIENT_H
#define AIMC_EMPROXY_POPCLIENT_H

#include "acl/TimeValue.h"
#include "acl/SockStream.h"
#include "acl/SString.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"

class IPOPResponse
{
public:
    virtual ~IPOPResponse(void) {};

    virtual apl_int_t Decode( char const* apcBuffer ) = 0;
};

/** 
 * @brief POP client LIST command resultset's node info type define
 */
class CPOPListInfo : public IPOPResponse
{
public:
    CPOPListInfo( apl_size_t auNum = 0, apl_size_t auSize = 0 );

    virtual ~CPOPListInfo(void) {};
    
    virtual apl_int_t Decode( char const* apcBuffer );

    apl_size_t GetNum(void) const;

    apl_size_t GetSize(void) const;

private:
    apl_size_t muNum;
    apl_size_t muSize;
};

/** 
 * @brief POP client UIDL command resultset's node info type define
 */
class CPOPUidlInfo : public IPOPResponse
{
public:
    CPOPUidlInfo( apl_size_t auNum = 0, char const* apcID = "");
    
    virtual ~CPOPUidlInfo(void) {};
    
    virtual apl_int_t Decode( char const* apcBuffer );

    apl_size_t GetNum(void) const;

    std::string const& GetID(void) const;

private:
    apl_size_t  muNum;
    std::string moID;
};

/** 
 * @brief POP client STAT command result info type define
 */
class CPOPStatInfo : public IPOPResponse
{
public:
    CPOPStatInfo( apl_size_t auCount = 0, apl_size_t auSize = 0 );
    
    virtual ~CPOPStatInfo(void) {};
    
    virtual apl_int_t Decode( char const* apcBuffer );

    apl_size_t GetCount(void) const;

    apl_size_t GetSize(void) const;

private:
    apl_size_t muCount;
    apl_size_t muSize;
};

/** 
 * @brief POP protocol client operation interface define
 */
class CPOPClient
{
public:
    typedef std::vector<CPOPListInfo> ListResultType;
    typedef std::vector<CPOPUidlInfo> UidlResultType;

public:
    CPOPClient(void);

    ~CPOPClient(void);

    apl_int_t Connect( 
        char const* apcHostName, 
        apl_uint16_t au16Port, 
        char const* apcUser, 
        char const* apcPass,
        const acl::CTimeValue& aoTimedout );

    void Close(void);

    apl_int_t Stat( CPOPStatInfo* apoStatInfo, const acl::CTimeValue& aoTimedout );

    apl_int_t List( ListResultType* apoResult, const acl::CTimeValue& aoTimedout );
    
    apl_int_t List( apl_size_t auNum, CPOPListInfo* apoListInfo, const acl::CTimeValue& aoTimedout );

    apl_int_t Uidl( UidlResultType* apoResult, const acl::CTimeValue& aoTimedout );
    
    apl_int_t Uidl( apl_size_t auNum, CPOPUidlInfo* apoUidlInfo, const acl::CTimeValue& aoTimedout );

    apl_int_t Uidl( char const* apcUidl, CPOPUidlInfo* apoUidlInfo, const acl::CTimeValue& aoTimedout );

    apl_int_t Top( apl_size_t auNum, apl_size_t auN, std::string* apoContent, const acl::CTimeValue& aoTimedout );
    
    apl_int_t Top(
        apl_size_t auNum,
        apl_size_t auN,
        std::vector<std::string>* apoLines,
        const acl::CTimeValue& aoTimedout );
        
    apl_int_t Retr( apl_size_t auNum, std::string* apoContent, const acl::CTimeValue& aoTimedout );
    
    apl_int_t Retr( apl_size_t auNum, std::string* apoHeader, std::string* apoBody, const acl::CTimeValue& aoTimedout );
    
    apl_int_t Retr(
        apl_size_t auNum,
        std::vector<std::string>* apoHeader,
        std::string* apoBody,
        const acl::CTimeValue& aoTimedout );
        
    apl_int_t Rset( const acl::CTimeValue& aoTimedout );

    apl_int_t Dele( apl_size_t auNum, const acl::CTimeValue& aoTimedout );
    
    char const* GetLastMessage(void);

protected:
    apl_int_t SendCommand( char const* apcCmd, IPOPResponse* apoResponse, const acl::CTimeValue& aoTimedout );
    
    apl_int_t RecvMail( bool abIsHeaderOnly, std::string* apoContent, const acl::CTimeValue& aoTimedout );

    apl_int_t RecvMail( bool abIsHeaderOnly, std::vector<std::string>* apoContent, const acl::CTimeValue& aoTimedout );

private:
    acl::CSockStream moPeer;

    char macMessage[128];
};

#endif//AIMC_EMPROXY_POPCLIENT_H

