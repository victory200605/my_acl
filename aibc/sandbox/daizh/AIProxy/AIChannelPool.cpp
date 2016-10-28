
#include "AIChannelPool.h"

////////////////////////////////////////// channel pool ////////////////////////////////
AIChannelPool::AIChannelPool()
{

}

AIChannelPool::~AIChannelPool()
{
    stList*   lpoList    = NULL;
    LISTHEAD* lpoTmpNode = NULL;
    TNode*    lpoNode    = NULL;
    
    {//recycle all resource
        AISmartLock loLock( this->coLock );
        TChannelListMap::iterator loIter = this->coChannelListMap.begin();
        for ( ; loIter != this->coChannelListMap.end(); loIter++ )
        {
            lpoList = loIter->second;
            
            AISmartLock loLock( lpoList->coLock );
            while( !ai_list_is_empty( &lpoList->coHead ) )
            {
                ai_list_del_head( lpoTmpNode, &lpoList->coHead );
                lpoNode = AI_GET_STRUCT_PTR( lpoTmpNode, TNode, coChild );
                AI_DELETE( lpoNode ); //delete node
            }
            
            AI_DELETE( loIter->second ); //delete list
        }
    }
}

AIChannelPtr AIChannelPool::GetChannel( const char* apcNamePath )
{
    stList*   lpoList    = NULL;
    LISTHEAD* lpoTmpNode = NULL;
    TNode*    lpoNode    = NULL;
    char*     lpcChannelID = NULL; 
    char      lsName[PATH_MAX] = {0};
    
    ::snprintf( lsName, sizeof(lsName), "%s", apcNamePath );
    if ( ( lpcChannelID = ::strchr( lsName, '/' ) ) != NULL )
    {
        *lpcChannelID++ = '\0';
    }
    
    ///lock scope
    {
        AISmartLock loLock( this->coLock );
        TChannelListMap::iterator loIter = this->coChannelListMap.find( lsName );
        if ( loIter == this->coChannelListMap.end() )
        {
            return NULL;
        }
        lpoList = loIter->second;
    }
    
    AISmartLock loLock( lpoList->coLock );
    if ( lpcChannelID == NULL )
    {
        ai_list_del_head( lpoTmpNode, &lpoList->coHead );
        if ( lpoTmpNode == NULL )
        {
            return NULL;
        }
        
        lpoNode = AI_GET_STRUCT_PTR( lpoTmpNode, TNode, coChild );
        ai_list_add_tail( lpoTmpNode, &lpoList->coHead );
        
        return lpoNode->coChannelPtr;
    }
    else
    {
        int liChannelID = atoi(lpcChannelID);
        AI_FOR_EACH_IN_LIST( loIter, &lpoList->coHead, TNode, coChild )
        {
            if ( loIter->coChannelPtr->GetChannelID() == liChannelID )
            {
                return loIter->coChannelPtr;
            }
        }
        
        return NULL;
    }
}

void AIChannelPool::PutChannel( const char* apcGroupName, AIBaseChannel* apoChannel )
{
    stList* lpoList = NULL;
    TNode*  lpoNode = NULL;
    
    {
        AISmartLock loLock( this->coLock );
        TChannelListMap::iterator loIter = this->coChannelListMap.find( apcGroupName );
        if ( loIter == this->coChannelListMap.end() )
        {
            AI_NEW_ASSERT( lpoList, stList );
            ai_init_list_head( &lpoList->coHead );
            lpoList->ciElemNum = 0;
            this->coChannelListMap.insert( TChannelListMap::value_type( apcGroupName, lpoList ) );
        }
        else
        {
            lpoList = loIter->second;
        }
    }
    
    {
        AI_NEW_ASSERT( lpoNode, TNode );
        lpoNode->coChannelPtr = apoChannel;
        
        AISmartLock loLock( lpoList->coLock );
        ai_list_add_tail( &lpoNode->coChild, &lpoList->coHead );
        lpoList->ciElemNum++;
    }
}

void AIChannelPool::EraseChannel( const char* apcGroupName, AIChannelPtr aoChannelPtr )
{
    stList* lpoList = NULL;
    
    {
        AISmartLock loLock( this->coLock );
        TChannelListMap::iterator loIter = this->coChannelListMap.find( apcGroupName );
        if ( loIter == this->coChannelListMap.end() )
        {
            return;
        }
        lpoList = loIter->second;
    }
    
    {
        AISmartLock loLock( lpoList->coLock );
        AI_FOR_EACH_IN_LIST( lpoNode, &lpoList->coHead, TNode, coChild )
        {
            if ( lpoNode->coChannelPtr == aoChannelPtr )
            {
                ai_list_del_any( &lpoNode->coChild );
                AI_DELETE( lpoNode );
                
                lpoList->ciElemNum--;
                
                break;
            }
        }
    }
}

size_t AIChannelPool::GetChannelCnt( const char* apcGroupName )
{
    stList*   lpoList;
    
    {
        AISmartLock loLock( this->coLock );
        TChannelListMap::iterator loIter = this->coChannelListMap.find( apcGroupName );
        if ( loIter == this->coChannelListMap.end() )
        {
            return 0;
        }
        lpoList = loIter->second;
    }
    
    {
        AISmartLock loLock( lpoList->coLock );
        return lpoList->ciElemNum;
    }
}
