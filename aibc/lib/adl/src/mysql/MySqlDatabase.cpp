///////////////////////////////////////////////////////////
//  MySqlDataBase.cpp
//  Implementation of the Class CMySqlDatabase
//  Created on:      19-10-2009 14:27:59
//  Original author: hezk
///////////////////////////////////////////////////////////

#include "adl/Adl.h"
#include "adl/MySqlDatabase.h"
#include "acl/stl/map.h"
#include "MySqlImpl.h"


ADL_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////
CMySqlDatabaseImpl::CMySqlDatabaseImpl( 
        const char* apcUser, 
        const char* apcPasswd, 
        const char* apcHostStr, 
        apl_int_t aiMaxConn,
        const char* apcDBName,
        apl_int_t aiPort)
    :moUser(apcUser)
    ,moPasswd(apcPasswd)
    ,moHost(apcHostStr)
    ,miMinConnection(1)
    ,miMaxConnection(aiMaxConn)
    ,moDBName(apcDBName)
    ,miPort(aiPort)
    ,miErrCode(0)
{
}

CMySqlDatabaseImpl::~CMySqlDatabaseImpl()
{
    this->Destroy();
}

apl_int_t CMySqlDatabaseImpl::InitConnPool(
        const char* apcUser, 
        const char* apcPasswd, 
        const char* apcHostStr, 
        apl_int_t aiMinConn, 
        apl_int_t aiMaxConn, 
        const char* apcDBName,
        apl_int_t aiPort)
{

    //make sure we have a controlable connection count
    if ( this->miMaxConnection < 1 
            || this->miMinConnection < 0
            || this->miMinConnection > this->miMaxConnection ) 
    {
        this->miMaxConnection = 5;
        this->miMinConnection = 1;
    }

    if ( this->miMaxConnection > DB_MYSQL_MAX_CONNECTION )
    {
        this->miMaxConnection = DB_MYSQL_MAX_CONNECTION;
    }

    if ( this->miMinConnection > DB_MYSQL_MAX_CONNECTION )
    {
        this->miMinConnection = DB_MYSQL_MAX_CONNECTION;
    }

    apl_int_t liItr;

    for (liItr = 0; liItr < this->miMaxConnection; liItr ++ ) 
    {
        AddMysqlNode();
    }

    if (this->moMysqlList.size() < (apl_size_t)this->miMinConnection)
    {
        this->Destroy();
        return -1;
    }

    return 0;
}

apl_int_t CMySqlDatabaseImpl::Init()
{
    apl_int_t liRet = 0;

    this->ClearErrorMsg();

    liRet = this->InitConnPool(
            moUser.c_str(), 
            moPasswd.c_str(), 
            moHost.c_str(), 
            miMaxConnection,
            miMinConnection,
            moDBName.c_str(),
            miPort);

    return liRet;
}

void CMySqlDatabaseImpl::Destroy()
{
    apl_size_t luItr;

    //release the ptr in moMysqlList
    for(luItr = 0; luItr < this->moMysqlList.size(); ++luItr) 
    {
        if (this->moMysqlList[luItr].mpoMysql != APL_NULL)
        {
            mysql_close(this->moMysqlList[luItr].mpoMysql);
            delete this->moMysqlList[luItr].mpoMysql;
            delete this->moMysqlList[luItr].mpoConn;
        }
    }

    //clear element of moMysqlList
    this->moMysqlList.clear();
}

IConnection* CMySqlDatabaseImpl::GetConnection(acl::CTimeValue const& aoTimeout)
{
    apl_size_t luIter;
    bool lbFound = false;
    apl_time_t ltTime;
    acl::CTimestamp loTimestamp;
    IConnection* lpoIConn = APL_NULL;
    MYSQL* lpoMysql = APL_NULL;

    this->ClearErrorMsg();

    //remember when we start
    loTimestamp.Update(acl::CTimestamp::PRC_USEC);
    ltTime = loTimestamp.Usec();

    while (true)
    {
        this->moLock.Lock();

        for (luIter = 0; 
                luIter < this->moMysqlList.size();
                ++luIter)
        {
            if (this->moMysqlList[luIter].mbOccupied == false)
            {
                lbFound = true;
                this->moMysqlList[luIter].mbOccupied = true;
                break;
            }
        }

        //try to add one
        if (!lbFound 
                && this->moMysqlList.size() < (apl_size_t)this->miMaxConnection)
        {
            //add node OK 
            if (AddMysqlNode() == 0)
            {
                lbFound = true;
                this->moMysqlList[luIter].mbOccupied = true;
                //don't need to adjust luIter here
                //it's correct at the end of the list
            }
        }

        //found free connection
        if (lbFound == true)
        {
            //read before unlock
            lpoMysql = this->moMysqlList[luIter].mpoMysql;
            lpoIConn = static_cast<IConnection*>(this->moMysqlList[luIter].mpoConn);
        }

        this->moLock.Unlock();

        if (lbFound == true)
        {
            if (mysql_ping(lpoMysql) != 0)
            {
                this->ReleaseConnection(lpoIConn);
                lbFound = false;
                continue;
            }
            else 
            {
                return lpoIConn;
            }
        }

        loTimestamp.Update(acl::CTimestamp::PRC_USEC);
        //timeout 
        if (loTimestamp.Usec() > ltTime + aoTimeout.Usec())
        {
            break;
        }
    }

    return  APL_NULL;
}

void CMySqlDatabaseImpl::ReleaseConnection(IConnection* apoIConn)
{
    CMySqlConnection* lpoMysqlConn = static_cast<CMySqlConnection*>(apoIConn);

    this->moLock.Lock();

    CMYSQL_LIST::iterator loListIter;

    for (loListIter = this->moMysqlList.begin();
            loListIter != this->moMysqlList.end();
            ++loListIter)
    {
        if (loListIter->mbOccupied == true 
                && loListIter->mpoConn == lpoMysqlConn)
        {
            loListIter->mbOccupied = false;

            //destroy the connection
            //might be not available any more
            if (mysql_ping(loListIter->mpoMysql) != 0)
            {
                mysql_close(loListIter->mpoMysql);
                delete loListIter->mpoMysql;
                delete loListIter->mpoConn;

                //remove the conn from list
                this->moMysqlList.erase(loListIter);
            }

            break;
        }
    }

    this->moLock.Unlock();
}

apl_int_t CMySqlDatabaseImpl::AddMysqlNode()
{
    CMYSQL loCMysql;
    loCMysql.mpoMysql = new MYSQL;
    loCMysql.mpoConn = new CMySqlConnection;
    apl_int_t liRet = 0;

    //should not go on...
    if (loCMysql.mpoMysql == APL_NULL 
            || loCMysql.mpoConn == APL_NULL)
    {
        this->moErrMsg = "Error! Can't malloc MYSQL!";

        if (loCMysql.mpoMysql != APL_NULL)
        {
            delete loCMysql.mpoMysql;
        }

        if (loCMysql.mpoConn != APL_NULL)
        {
            delete loCMysql.mpoConn;
        }

        return -1;
    }

    loCMysql.mbConnected = false;
    loCMysql.mbOccupied = false;
    loCMysql.mpoConn->SetMysqlConn(loCMysql.mpoMysql);

    this->moMysqlList.push_back(loCMysql);

    liRet = MysqlConnect(&loCMysql);

    if (liRet != 0)
    {
        this->moMysqlList.pop_back();
        delete loCMysql.mpoMysql;
        delete loCMysql.mpoConn;
        return -1;
    }

    return 0;
}

apl_int_t CMySqlDatabaseImpl::MysqlConnect(CMYSQL* apoCMysql)
{
    if (apoCMysql == APL_NULL)
    {
        return DB_CONNECTION_FAIL;
    }

    apl_int_t liRet = DB_CONNECTION_OK;

    if(apoCMysql->mbConnected == false) 
    {
        mysql_init(apoCMysql->mpoMysql); 

        if(mysql_real_connect( 
                    apoCMysql->mpoMysql, 
                    this->moHost.c_str(),
                    this->moUser.c_str(),
                    this->moPasswd.c_str(),
                    this->moDBName.c_str(),
                    this->miPort, 
                    NULL, 0 ) == NULL )
        {
            this->SetErrorMsg(mysql_error(apoCMysql->mpoMysql), 
                    mysql_errno(apoCMysql->mpoMysql));
            liRet = DB_CONNECTION_FAIL;

        } 
        else 
        {
            apoCMysql->mbConnected = true;
        }
    }

    return liRet ;
}

void CMySqlDatabaseImpl::ClearErrorMsg()
{
    this->moErrMsg.clear();
    this->miErrCode = 0;
}

void CMySqlDatabaseImpl::SetErrorMsg(const char* apcMsg, apl_int_t aiErrCode)
{
    this->moErrMsg = apcMsg;
    this->miErrCode = aiErrCode;
}

const char* CMySqlDatabaseImpl::GetErrorMsg()
{
    return this->moErrMsg.c_str();
}

apl_int_t CMySqlDatabaseImpl::GetErrorCode()
{
    return this->miErrCode;
}

////////////////////////////////////////////////////////////////////////////////

CMySqlDatabase::CMySqlDatabase(
            const char* apcUser, 
            const char* apcPasswd, 
            const char* apcHostStr, 
            apl_int_t aiMaxConn, 
            const char* apcDBName,
            apl_int_t aiPort)
{
    this->mpoImpl = new CMySqlDatabaseImpl(
            apcUser,
            apcPasswd, 
            apcHostStr, 
            aiMaxConn, 
            apcDBName,
            aiPort);

    assert(this->mpoImpl != APL_NULL);
}

CMySqlDatabase::~CMySqlDatabase()
{
    delete this->mpoImpl;
}

apl_int_t CMySqlDatabase::Init()
{
    return this->mpoImpl->Init();
}

void CMySqlDatabase::Destroy()
{
    this->mpoImpl->Destroy();
}

IConnection* CMySqlDatabase::GetConnection(acl::CTimeValue const& aoTimeout)
{
    return this->mpoImpl->GetConnection(aoTimeout);
}

void CMySqlDatabase::ReleaseConnection(IConnection* apoIConn)
{
    this->mpoImpl->ReleaseConnection(apoIConn);
}

const char* CMySqlDatabase::GetErrorMsg()
{
    return this->mpoImpl->GetErrorMsg();
}

apl_int_t CMySqlDatabase::GetErrorCode()
{
    return this->mpoImpl->GetErrorCode();
}

apl_int_t CMySqlDatabase::GetThreadSafety()
{
    return THREAD_SAFETY_MODULE;
}

////////////////////////////////////////////////////////////////////////////////////////
//Define for dynamic load database type
extern "C"
{
    IDatabase* CreateDatabase0( std::map<std::string, std::string>& aoParameteres );
}

IDatabase* CreateDatabase0( std::map<std::string, std::string>& aoParameteres )
{
    CMySqlDatabase* lpoDatabase = APL_NULL;
    std::string loName;
    std::string loUser;
    std::string loPass;
    std::string loHost;
    apl_int_t   liPort;
    apl_int_t   liConnPool = 0;

    //Get database name
    std::map<std::string, std::string>::iterator loIter = aoParameteres.find("name");
    if (loIter == aoParameteres.end() )
    {
        apl_errprintf("Create database fail,unknown db name, example:name=<dbname>\n");

        return APL_NULL;
    }
    loName = loIter->second;

    if ( (loIter = aoParameteres.find("user") ) == aoParameteres.end() )
    {
        apl_errprintf("Create database fail,unknown user, example:user=<username>\n");

        return APL_NULL;
    }
    loUser = loIter->second;

    if ( (loIter = aoParameteres.find("pass") ) == aoParameteres.end() )
    {
        apl_errprintf("Create database fail,unknown pass, example:pass=<...>\n");

        return APL_NULL;
    }
    loPass = loIter->second;

    if ( (loIter = aoParameteres.find("host") ) == aoParameteres.end() )
    {
        apl_errprintf("Create database fail,unknown host, example:host=<ip>\n");

        return APL_NULL;
    }
    loHost = loIter->second;

    if ( (loIter = aoParameteres.find("port") ) == aoParameteres.end() )
    {
        apl_errprintf("Create database fail,unknown host, example:host=<num>\n");

        return APL_NULL;
    }
    liPort = apl_strtoi32(loIter->second.c_str(), APL_NULL, 10);

    if ( (loIter = aoParameteres.find("connpool") ) == aoParameteres.end() )
    {
        apl_errprintf("Create database fail,unknown connpool, example:connpool=<num>\n");

        return APL_NULL;
    }

    liConnPool = apl_strtoi32(loIter->second.c_str(), APL_NULL, 10);

    ACL_NEW_ASSERT(lpoDatabase, 
        CMySqlDatabase(
            loUser.c_str(),
            loPass.c_str(),
            loHost.c_str(),
            liConnPool,
            loName.c_str(),
            liPort ) );

    return lpoDatabase;
}

ADL_NAMESPACE_END
