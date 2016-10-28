#ifndef __AILIBEX__AILDAPDEF_H__
#define __AILIBEX__AILDAPDEF_H__

///start namespace
AIBC_NAMESPACE_START

/// In Variables: csAiuumIp, ciAiuumPort, cpfTrapCallback
/// Out Variables: lsBaseDn

/////////////////////////////////////////////////////////////////////////
//
typedef  void (*AiuumTrapCallback)( const char *, int );

/////////////////////////////////////////////////////////////////////////
//
#define  AICOM_UUM_LOGNAME   "AIUUM.log"

/////////////////////////////////////////////////////////////////////////
// Fetch Record (Sync Version from IGK)
#define  LDAP_SYNC_SEARCH_DECLARE_BEGIN() \
    int             liColCount = 0; \
    char            *lpcAttribs[48]; \
    char            *lpcTitle = NULL; \
    char            lsBaseDn[256] = {0,}; \
    char            lsFilter[256] = {0,}; \
    LDAP            *lpoLDAP = NULL; \
    LDAPMessage     *lpoMessage = NULL; \
    LDAPMessage     *lpoTmpMsg = NULL; \
    BerElement      *lpoElement = NULL; \
    struct berval   **lpoBerval = NULL; \
    memset(lpcAttribs, 0, sizeof(lpcAttribs)); \
    if((lpoLDAP = ldap_init(csUUMIP, ciUUMPort)) == NULL) \
    { \
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s ldap_init(%s:%d) fail", \
            __FUNCTION__, csUUMIP, ciUUMPort); \
        return (1); \
    } \
    if(ldap_bind_s(lpoLDAP, "", "", LDAP_AUTH_SIMPLE) != LDAP_SUCCESS) \
    { \
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s ldap_bind_s(%s:%d) fail", \
            __FUNCTION__, csUUMIP, ciUUMPort); \
        ldap_unbind(lpoLDAP); \
        return (2); \
    } 
 

#define  LDAP_SYNC_SEARCH_DECLARE_END(filter) \
    strcpy(lsFilter, filter); \
    if(ldap_search_s(lpoLDAP, lsBaseDn, LDAP_SCOPE_BASE, lsFilter, lpcAttribs, 0, &lpoMessage) == -1) \
    { \
        AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "%s ldap_search fail", __FUNCTION__); \
        ldap_unbind(lpoLDAP); \
        return (3); \
    } 
 


/////////////////////////////////////////////////////////////////////////
// Fetch Record
#define  LDAP_SEARCH_DECLARE_BEGIN() \
    int             liRetCode = 0; \
    int             liColCount = 0; \
    char            *lpcAttribs[48]; \
    char            *lpcTitle = NULL; \
    char            lsBaseDn[256] = {0,}; \
    char            lsFilter[256] = {0,}; \
    LDAP            *lpoLDAP = NULL; \
    LISTHEAD        loTempHead; \
    LDAPMessage     *lpoMessage = NULL; \
    LDAPMessage     *lpoTmpMsg = NULL; \
    BerElement      *lpoElement = NULL; \
    struct berval   **lpoBerval = NULL; \
    memset( lpcAttribs, 0, sizeof(lpcAttribs) ); \
    if( (lpoLDAP = ldap_init( csAiuumIp, ciAiuumPort )) == NULL) \
    { \
        if( cpfTrapCallback ) \
        { \
            cpfTrapCallback( csAiuumIp, ciAiuumPort ); \
        } \
        AIWriteLOG( AICOM_UUM_LOGNAME, AILOG_LEVEL_ERROR, "%s ldap_init( %s:%d ) fail", \
            __FUNCTION__, csAiuumIp, ciAiuumPort ); \
        return  (1); \
    } \
    if( ldap_bind_s( lpoLDAP, "", "", LDAP_AUTH_SIMPLE) != LDAP_SUCCESS ) \
    { \
        if( cpfTrapCallback ) \
        { \
            cpfTrapCallback( csAiuumIp, ciAiuumPort ); \
        } \
        AIWriteLOG( AICOM_UUM_LOGNAME, AILOG_LEVEL_ERROR, "%s ldap_bind_s( %s:%d ) fail", \
            __FUNCTION__, csAiuumIp, ciAiuumPort ); \
        ldap_unbind( lpoLDAP ); \
        return  (2); \
    } \

#define  LDAP_SEARCH_DECLARE( name_ ) \
    lpcAttribs[liColCount++] = (char*)name_;

#define  LDAP_SEARCH_DECLARE_END( filter ) \
    strcpy( lsFilter, filter ); \
    if( ldap_search( lpoLDAP, lsBaseDn, LDAP_SCOPE_BASE, lsFilter, lpcAttribs, 0 ) == -1) \
    { \
        AIWriteLOG( AICOM_UUM_LOGNAME, AILOG_LEVEL_ERROR, "%s ldap_search fail", __FUNCTION__ ); \
        ldap_unbind( lpoLDAP ); \
        return  (3); \
    } \
    
    
/////////////////////////////////////////////////////////////////////////
//
#define  LDAP_SEARCH_RESULT_LINK_BEGIN( obj ) \
    ai_init_list_head( &loTempHead ); \
    while( 1 ) \
    { \
        liRetCode = ldap_result( lpoLDAP, LDAP_RES_ANY, 0, NULL, &lpoMessage ); \
        if( liRetCode != LDAP_RES_SEARCH_ENTRY ) \
            break; \
        memset( &obj, 0, sizeof(obj) ); \
        lpoTmpMsg = ldap_first_entry( lpoLDAP, lpoMessage ); \
        for(lpcTitle = ldap_first_attribute(lpoLDAP, lpoTmpMsg, &lpoElement ); lpcTitle != NULL; \
            lpcTitle = ldap_next_attribute(lpoLDAP, lpoTmpMsg, lpoElement ) ) \
        { \
            if( (lpoBerval = ldap_get_values_len(lpoLDAP, lpoTmpMsg, lpcTitle )) ) \
            { 

#define  LDAP_SEARCH_RESULT_LINK_END( type, obj ) \
                ber_bvecfree( lpoBerval ); \
            } \
        } \
        type *_NewNode = NULL; \
        AI_NEW_ASSERT( _NewNode, type ); \
        memcpy( _NewNode, &obj, sizeof(type) ); \
        ai_list_add_tail( &(_NewNode->coChild), &loTempHead ); \
        ldap_msgfree( lpoMessage ); \
    } \
    ldap_msgfree( lpoMessage ); \
    ldap_unbind( lpoLDAP );


/////////////////////////////////////////////////////////////////////////
//
#define  LDAP_SEARCH_RESULT_BEGIN( ) \
    while( 1 ) \
    { \
        liRetCode = ldap_result( lpoLDAP, LDAP_RES_ANY, 0, NULL, &lpoMessage ); \
        if( liRetCode != LDAP_RES_SEARCH_ENTRY ) \
            break; \
        lpoTmpMsg = ldap_first_entry( lpoLDAP, lpoMessage ); \
        for(lpcTitle = ldap_first_attribute(lpoLDAP, lpoTmpMsg, &lpoElement ); lpcTitle != NULL; \
            lpcTitle = ldap_next_attribute(lpoLDAP, lpoTmpMsg, lpoElement ) ) \
        { \
            if( (lpoBerval = ldap_get_values_len(lpoLDAP, lpoTmpMsg, lpcTitle )) ) \
            { 

#define  LDAP_SEARCH_RESULT_END( ) \
                ber_bvecfree( lpoBerval ); \
            } \
        } \
        ldap_msgfree( lpoMessage ); \
    } \
    ldap_msgfree( lpoMessage ); \
    ldap_unbind( lpoLDAP );

/////////////////////////////////////////////////////////////////////////
// Append Record
#define  LDAP_APPEND_DECLARE_BEGIN() \
    int             liRetCode = 0; \
    int             liColCount = 0; \
    LDAP            *lpoLDAP = NULL; \
    char            lsBaseDn[256] = {0,}; \
    LDAPMod         *lpoAddData[48]; \
    char            *lpcAttribs[48]; \
    char            *lpcValues[48]; \
    memset( lpcValues, 0, sizeof(lpcValues) ); \
    memset( lpcAttribs, 0, sizeof(lpcAttribs) ); \
    memset( lpoAddData, 0, sizeof(lpoAddData) ); \
    if( (lpoLDAP = ldap_init( csAiuumIp, ciAiuumPort )) == NULL) \
    { \
        if( cpfTrapCallback ) \
        { \
            cpfTrapCallback( csAiuumIp, ciAiuumPort ); \
        } \
        AIWriteLOG( AICOM_UUM_LOGNAME, AILOG_LEVEL_ERROR, "%s ldap_init( %s:%d ) fail", \
            __FUNCTION__, csAiuumIp, ciAiuumPort ); \
        return  (1); \
    } \
    if( ldap_bind_s( lpoLDAP, "", "", LDAP_AUTH_SIMPLE) != LDAP_SUCCESS ) \
    { \
        if( cpfTrapCallback ) \
        { \
            cpfTrapCallback( csAiuumIp, ciAiuumPort ); \
        } \
        AIWriteLOG( AICOM_UUM_LOGNAME, AILOG_LEVEL_ERROR, "%s ldap_bind_s( %s:%d ) fail", \
            __FUNCTION__, csAiuumIp, ciAiuumPort ); \
        ldap_unbind( lpoLDAP ); \
        return  (2); \
    } \

#define  LDAP_APPEND_NAMEVALUE( name_, value_ ) \
    lpcAttribs[liColCount] = (char*)name_; \
    lpcValues[liColCount] = (char*)value_; \
    liColCount += 1;

#define LDAP_APPEND_DECLARE_END( table_, basedn_ ) \
    for( int liIt = 0; liIt < liColCount; liIt++ ) \
    { \
        LDAPMod         *lpoLDAPModify; \
        lpoLDAPModify = (LDAPMod *)malloc( sizeof(LDAPMod) ); \
        memset(lpoLDAPModify, 0, sizeof(LDAPMod)); \
        lpoAddData[liIt] = lpoLDAPModify; \
        lpoLDAPModify->mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES; \
        lpoLDAPModify->mod_type = lpcAttribs[liIt]; \
        lpoLDAPModify->mod_bvalues = (struct berval **)malloc(2*sizeof(struct berval*)); \
        lpoLDAPModify->mod_bvalues[1] = NULL; \
        lpoLDAPModify->mod_bvalues[0] = (struct berval *)malloc(sizeof(struct berval)); \
        lpoLDAPModify->mod_bvalues[0]->bv_len = strlen( lpcValues[liIt] ); \
        lpoLDAPModify->mod_bvalues[0]->bv_val = (char *)malloc(strlen(lpcValues[liIt]) + 1); \
        strcpy( lpoLDAPModify->mod_bvalues[0]->bv_val, lpcValues[liIt] ); \
    } \
    sprintf( lsBaseDn, "t=%s,%s", table_, basedn_ ); \
    liRetCode = ldap_add_s( lpoLDAP, lsBaseDn, lpoAddData ); \
    ldap_unbind( lpoLDAP ); \
    for( int liIk = 0; liIk < liColCount; liIk++ ) \
    { \
        if( lpoAddData[liIk]->mod_bvalues ) \
        { \
            ber_bvecfree( lpoAddData[liIk]->mod_bvalues ); \
            lpoAddData[liIk]->mod_bvalues = NULL; \
        } \
        free( lpoAddData[liIk] ); \
        lpoAddData[liIk] = NULL; \
    } \

    
/////////////////////////////////////////////////////////////////////////
// Modify Record
#define  LDAP_MODIFY_DECLARE_BEGIN() \
    int             liRetCode = 0; \
    int             liColCount = 0; \
    LDAP            *lpoLDAP = NULL; \
    char            *lpcPtr = NULL; \
    char            lsBaseDn[1024] = {0,}; \
    LDAPMod         *lpoAddData[48]; \
    char            *lpcAttribs[48]; \
    char            *lpcValues[48]; \
    lpcPtr = lsBaseDn; \
    memset( lpcValues, 0, sizeof(lpcValues) ); \
    memset( lpcAttribs, 0, sizeof(lpcAttribs) ); \
    memset( lpoAddData, 0, sizeof(lpoAddData) ); \
    if( (lpoLDAP = ldap_init( csAiuumIp, ciAiuumPort )) == NULL) \
    { \
        if( cpfTrapCallback ) \
        { \
            cpfTrapCallback( csAiuumIp, ciAiuumPort ); \
        } \
        AIWriteLOG( AICOM_UUM_LOGNAME, AILOG_LEVEL_ERROR, "%s ldap_init( %s:%d ) fail", \
            __FUNCTION__, csAiuumIp, ciAiuumPort ); \
        return  (1); \
    } \
    if( ldap_bind_s( lpoLDAP, "", "", LDAP_AUTH_SIMPLE) != LDAP_SUCCESS ) \
    { \
        if( cpfTrapCallback ) \
        { \
            cpfTrapCallback( csAiuumIp, ciAiuumPort ); \
        } \
        AIWriteLOG( AICOM_UUM_LOGNAME, AILOG_LEVEL_ERROR, "%s ldap_bind_s( %s:%d ) fail", \
            __FUNCTION__, csAiuumIp, ciAiuumPort ); \
        ldap_unbind( lpoLDAP ); \
        return  (2); \
    } \

#define  LDAP_MODIFY_NAMEVALUE( name_, value_ ) \
    lpcAttribs[liColCount] = (char*)name_; \
    lpcValues[liColCount] = (char*)value_; \
    liColCount += 1;

#define  LDAP_MODIFY_INDEXSTR( index, value_ ) \
    lpcPtr += sprintf( lpcPtr, "%s=%s,", index, value_ );
    
#define  LDAP_MODIFY_INDEXINT( index, value_ ) \
    lpcPtr += sprintf( lpcPtr, "%s=%d,", index, value_ );

#define LDAP_MODIFY_DECLARE_END( table_, basedn_ ) \
    for( int liIt = 0; liIt < liColCount; liIt++ ) \
    { \
        LDAPMod         *lpoLDAPModify; \
        lpoLDAPModify = (LDAPMod *)malloc( sizeof(LDAPMod) ); \
        memset(lpoLDAPModify, 0, sizeof(LDAPMod)); \
        lpoAddData[liIt] = lpoLDAPModify; \
        lpoLDAPModify->mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES; \
        lpoLDAPModify->mod_type = lpcAttribs[liIt]; \
        lpoLDAPModify->mod_bvalues = (struct berval **)malloc(2*sizeof(struct berval*)); \
        lpoLDAPModify->mod_bvalues[1] = NULL; \
        lpoLDAPModify->mod_bvalues[0] = (struct berval *)malloc(sizeof(struct berval)); \
        lpoLDAPModify->mod_bvalues[0]->bv_len = strlen( lpcValues[liIt] ); \
        lpoLDAPModify->mod_bvalues[0]->bv_val = (char *)malloc(strlen(lpcValues[liIt]) + 1); \
        strcpy( lpoLDAPModify->mod_bvalues[0]->bv_val, lpcValues[liIt] ); \
    } \
    sprintf( lpcPtr, "t=%s,%s", table_, basedn_ ); \
    liRetCode = ldap_modify_s( lpoLDAP, lsBaseDn, lpoAddData ); \
    ldap_unbind( lpoLDAP ); \
    for( int liIk = 0; liIk < liColCount; liIk++ ) \
    { \
        if( lpoAddData[liIk]->mod_bvalues ) \
        { \
            ber_bvecfree( lpoAddData[liIk]->mod_bvalues ); \
            lpoAddData[liIk]->mod_bvalues = NULL; \
        } \
        free( lpoAddData[liIk] ); \
        lpoAddData[liIk] = NULL; \
    } \
    

/////////////////////////////////////////////////////////////////////////
// Delete Record
#define LDAP_DELETE_DECLARE_BEGIN( ) \
    int             liRetCode = 0; \
    char            *lpcPtr = NULL; \
    LDAP            *lpoLDAP = NULL; \
    char            lsBaseDn[1024] = {0,}; \
    strcpy( lsBaseDn, "VRDN=(" ); \
    lpcPtr = lsBaseDn + strlen(lsBaseDn); \
    if( (lpoLDAP = ldap_init( csAiuumIp, ciAiuumPort )) == NULL) \
    { \
        if( cpfTrapCallback ) \
        { \
            cpfTrapCallback( csAiuumIp, ciAiuumPort ); \
        } \
        AIWriteLOG( AICOM_UUM_LOGNAME, AILOG_LEVEL_ERROR, "%s ldap_init( %s:%d ) fail", \
            __FUNCTION__, csAiuumIp, ciAiuumPort ); \
        return  (1); \
    } \
    if( ldap_bind_s( lpoLDAP, "", "", LDAP_AUTH_SIMPLE) != LDAP_SUCCESS ) \
    { \
        if( cpfTrapCallback ) \
        { \
            cpfTrapCallback( csAiuumIp, ciAiuumPort ); \
        } \
        AIWriteLOG( AICOM_UUM_LOGNAME, AILOG_LEVEL_ERROR, "%s ldap_bind_s( %s:%d ) fail", \
            __FUNCTION__, csAiuumIp, ciAiuumPort ); \
        ldap_unbind( lpoLDAP ); \
        return  (2); \
    } \

#define LDAP_DELETE_INDEXSTR( name_, value_ ) \
    lpcPtr += sprintf( lpcPtr, "%s=%s,", name_, value_ );
    
#define LDAP_DELETE_INDEXINT( name_, value_ ) \
    lpcPtr += sprintf( lpcPtr, "%s=%d,", name_, value_ );
    
#define LDAP_DELETE_DECLARE_END( table_, basedn_ ) \
    sprintf( lpcPtr-1, "),t=%s,%s", table_, basedn_ ); \
    liRetCode = ldap_delete_s( lpoLDAP, lsBaseDn ); \
    ldap_unbind( lpoLDAP ); 

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AILDAPDEF_H__

