#ifndef __AILIBEX__AIAGENT_H__
#define __AILIBEX__AIAGENT_H__

#include "AISnmp.h"
#include "stl/vector.h"
#include <iostream>

#define MIB_BEGIN(caller) { \
    AIAgent* lpoAgent = (AIAgent*)caller; \
    stVariableBindingAgent loVB; \
    NULL

#define MIB_END() } NULL

#define MIB_TABLE_BEGIN(table_oid) { \
    char const* const lsTableOid = table_oid; \
    NULL

#define MIB_TABLE_END() } NULL

#define MIB_ROW_BEGIN(i) { \
    int row = i; \
    NULL

#define MIB_ROW_END() } NULL

#define MIB_ROW_SET_INT(col, value) \
    memset(&loVB,0,sizeof(stVariableBindingAgent));\
    snprintf(loVB.csName, sizeof(loVB.csName), "%s.%d.%d", lsTableOid, row, col); \
    loVB.ccType = AI_BER_INTEGER; \
    loVB.coVal.ciInteger = value; \
    loVB.ciValLen = sizeof(int); \
    lpoAgent->SetData(loVB);


#define MIB_ROW_SET_STR(col, value) \
    memset(&loVB,0,sizeof(stVariableBindingAgent));\
    snprintf(loVB.csName, sizeof(loVB.csName), "%s.%d.%d", lsTableOid, row, col); \
    loVB.ccType = AI_BER_OCTET_STR; \
    StringCopy(loVB.coVal.csString, value,sizeof(loVB.coVal.csString)); \
    loVB.ciValLen = strlen(value); \
    lpoAgent->SetData(loVB);


#define MIB_ROW_SET_OID(col, value) \
    memset(&loVB,0,sizeof(stVariableBindingAgent));\
    snprintf(loVB.csName, sizeof(loVB.csName), "%s.%d.%d", lsTableOid, row, col); \
    loVB.ccType = AI_BER_OBJECT_ID; \
    StringCopy(loVB.coVal.csObjid, value,sizeof(loVB.coVal.csObjid)); \
    loVB.ciValLen = strlen(value); \
    lpoAgent->SetData(loVB);


#define MIB_ROW_SET_TIMETICKS(col, value) \
    memset(&loVB,0,sizeof(stVariableBindingAgent));\
    snprintf(loVB.csName, sizeof(loVB.csName), "%s.%d.%d", lsTableOid, row, col); \
    loVB.ccType = AI_BER_TIMETICKS; \
    loVB.coVal.ciInteger = value; \
    loVB.ciValLen = sizeof(unsigned long); \
    lpoAgent->SetData(loVB);

///////////////////////////////////////////////////////////////////////////////////

#define MIB_SET_INT(oid, value) \
    memset(&loVB,0,sizeof(stVariableBindingAgent));\
    StringCopy(loVB.csName, oid, sizeof(loVB.csName)); \
    loVB.ccType = AI_BER_INTEGER; \
    loVB.coVal.ciInteger = value; \
    loVB.ciValLen = sizeof(int); \
    lpoAgent->SetData(loVB); 

#define MIB_SET_STR(oid, value) \
    memset(&loVB,0,sizeof(stVariableBindingAgent));\
    StringCopy(loVB.csName, oid, sizeof(loVB.csName)); \
    loVB.ccType = AI_BER_OCTET_STR; \
    StringCopy(loVB.coVal.csString, value,sizeof(loVB.coVal.csString)); \
    loVB.ciValLen = strlen(value); \
    lpoAgent->SetData(loVB); 

#define MIB_SET_OID(oid, value) \
    memset(&loVB,0,sizeof(stVariableBindingAgent));\
    StringCopy(loVB.csName, oid, sizeof(loVB.csName)); \
    loVB.ccType = AI_BER_OBJECT_ID; \
    StringCopy(loVB.coVal.csObjid, value,sizeof(loVB.coVal.csObjid)); \
    loVB.ciValLen = strlen(value); \
    lpoAgent->SetData(loVB);

#define MIB_SET_TIMETICKS(oid, value) \
    memset(&loVB,0,sizeof(stVariableBindingAgent));\
    StringCopy(loVB.csName, oid, sizeof(loVB.csName)); \
    loVB.ccType = AI_BER_TIMETICKS; \
    loVB.coVal.ciInteger = value; \
    loVB.ciValLen = sizeof(unsigned long); \
    lpoAgent->SetData(loVB);

using namespace AI_STD;

typedef int (*AIAgentCallback)(void* apCaller);

typedef struct stNetCallbackEntry
{
    long            caOid[AI_SNMP_OID_NUM];
    char            csOid[AI_SNMP_MAX_LEN];
    AIAgentCallback cpCallback;
    time_t          ciInterval;
    time_t          ciLastUpdate; 
}stCallbackEntry;

typedef vector<stCallbackEntry> AIAgentCallbackTable;

typedef struct stNetVariableBindingAgent
{   
    long                caOid[AI_SNMP_OID_NUM];
    char                csName[AI_SNMP_MAX_LEN];   
    unsigned char       ccType;               
    int                 ciValLen;             
    unNetsnmpVardata    coVal;                
}stVariableBindingAgent;

typedef vector < stVariableBindingAgent > SnmpDataTable;

/**
 * class AIAgent
 */ 
class AIAgent
{
public:

    /**
     * @brief Bind IP and Port to socket and create a thread to solve the request.
     *
     * @param apcIP         IN - the local IP address
     * @param aiServPort    IN - the local port
     *
     * @return 0 if success.
     */ 
    int StartUp(char *apcIP, int aiServPort);

    /**
     * @brief Close socket and shutdown the thread.
     *
     * @return 0 if success.
     */ 
    int Shutdown();

    /**
     * @brief Set oid to Snmp Data Table.
     * 
     * @param aoBata      IN - the oid need to set
     *
     * @return 0 if success
     */ 
    int SetData(stVariableBindingAgent &aoData);

    /**
     * @brief Get variable-binding from SnmpDataTable to the snmp_pdu structure.
     *
     * @param apoPdu      IN/OUT - the snmp pdu
     *
     * @return 0 if sucess.
     */ 
     int GetData(stSnmpPdu *apoPdu);

    /**
     * @brief Get-next variable-binding from SnmpDataTable to the snmp_pdu structure.
     *
     * @param apoPdu      IN/OUT - the snmp pdu
     *
     * @return 0 if sucess.
     */ 
    int FindNextData(stSnmpPdu *apoPdu);

    /**
     * @brief Delete variable-binding rom SnmpDataTable according to the specified oid.
     *
     * @param apcOid      IN - the specified oid
     *
     * @return 0 if success.-1 if can't delete
     */ 
    int DelData(char *apcOid);

    /**
     * @brief Register CallbackTable.
     * 
     * @param apcOidPrefix       IN - the oid prefix you specified
     * @param apfCallback        IN - the function pointer 
     * @param aiRefreshInterval  IN - the refresh interval
     * 
     * @return 0 if register succefully; AI_SNMP_ECANTREG If the oid Prefix  you specified is already exist in CallbackTable.Cannot register!
     */ 
    int RegisterCallback(char const* apcOidPrefix, AIAgentCallback apfCallback, time_t aiRefreshInterval);
    
    /**
     * @brief Unregister according to specified oid prefix.
     * 
     * @param apcOidPrefix      IN - the oid prefix you specified
     *
     * @return 0 if unregister succefully according to oid prefix;
     */ 
    int UnregisterCallback(char const* apcOidPrefix);
    
    /**
     * @brief Unregister according to specified function pointer
     *
     * @param apfCallback        IN - the function pointer you specified
     */ 
    int UnregisterCallback(AIAgentCallback apfCallback);

    /**
     * @brief Refresh SnmpDataTable according to CallbackTable.
     */ 
    int RefreshSnmpDataTable();

    /**
     * @brief (just for test)Print csName and its value in SnmpDataTable.
     */ 
    int ShowDataINSnmpDataTable();

    /**
     * @brief (just for test)Print csOid and ciInterval in CallbackTable.
     */ 
    int ShowDataINAIAgentCallbackTable();
    

private:

    int                   ProcessFunc();
    static void*          ThreadEntry(void*);
    
    AIMutexLock           coLock;
    SnmpDataTable         coSnmpDataTable;
    AIAgentCallbackTable  coAIAgentCallbackTable;
    int                   ciSockfd;
    bool                  cbIsRunning;
};


#endif  //__AILIBEX__AIAGENT_H__
