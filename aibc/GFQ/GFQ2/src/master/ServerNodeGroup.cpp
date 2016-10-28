
#include "ServerNodeGroup.h"
#include "acl/Singleton.h"

AIBC_GFQ_NAMESPACE_START

CServerNodeGroupRRS::CServerNodeGroupRRS(void)
{
}

CServerNodeGroupRRS::~CServerNodeGroupRRS(void)
{
}

void CServerNodeGroupRRS::Add( CServerNode* apoServerNode )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    bool lbReturn = this->moPriList.Insert(apoServerNode);
    
    ACL_ASSERT(lbReturn == true);
}

void CServerNodeGroupRRS::Activate( CServerNode* apoServerNode )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    if (apoServerNode->GetFreeSegmentCount() > 0)
    {
        bool lbReturn = this->moPriList.Resume(apoServerNode);
    
        ACL_ASSERT(lbReturn == true);
    }
}

void CServerNodeGroupRRS::Remove( CServerNode* apoServerNode )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    this->moPriList.Remove(apoServerNode);
}

CSegmentHandle CServerNodeGroupRRS::Allocate(void)
{
    apl_size_t luSegmentID = INVALID_ID;

    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    //Search writeable allocate by ServerNode priority
    for (CPriorityListIterator loListIter(this->moPriList);
        !loListIter.IsDone(); loListIter.Next() )
    {
        CServerNode* lpoServerNode = APL_NULL;

        //Find the first ServerNode
        for (CServerNodeIterator loServerNodeIter(loListIter);
            !loServerNodeIter.IsDone(); loServerNodeIter.Next() )
        {
            lpoServerNode = loServerNodeIter.operator -> ();

            ACL_ASSERT(lpoServerNode->GetFreeSegmentCount() > 0);

            break;
        }
            
        if (lpoServerNode == APL_NULL)
        {
            break;
        }
            
        if ( (luSegmentID = lpoServerNode->Allocate() ) != INVALID_ID)
        {
            if (lpoServerNode->GetFreeSegmentCount() <= 0)
            {
                //Suspend empty server node
                this->moPriList.Suspend(lpoServerNode);
            }
            else
            {
                //Move server node to the link back and waiting next allocating
                this->moPriList.MoveToTail(lpoServerNode);
            }

            //printf("Allocated server id = %"APL_PRIuINT", tps = %"APL_PRIuINT", weighting=%"APL_PRIuINT", total=%"APL_PRIuINT"\n", 
            //    lpoServerNode->GetServerNodeID(),
            //    loFirst.GetReadTPS() + loFirst.GetWriteTPS(),
            //    loFirst.GetAllocateWeighting(),
            //    loFirst.GetAccessedTPS() );

            return CSegmentHandle(lpoServerNode->GetServerNodeID(), luSegmentID);
        }
    }

    return CSegmentHandle();
}

void CServerNodeGroupRRS::Deallocate( CServerNode* apoServerNode, CSegmentHandle aoHandle )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    apoServerNode->Deallocate(aoHandle.GetSegmentID() );
        
    if (apoServerNode->GetFreeSegmentCount() == 1)
    {
        this->moPriList.Resume(apoServerNode);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
CServerNodeGroupWLRA::CServerNodeGroupWLRA(void)
{
}

CServerNodeGroupWLRA::~CServerNodeGroupWLRA(void)
{
}

void CServerNodeGroupWLRA::Add( CServerNode* apoServerNode )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    bool lbReturn = this->moPriList.Insert(apoServerNode);
    
    ACL_ASSERT(lbReturn == true);
}

void CServerNodeGroupWLRA::Activate( CServerNode* apoServerNode )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    if (apoServerNode->GetFreeSegmentCount() > 0)
    {
        bool lbReturn = this->moPriList.Resume(apoServerNode);
    
        ACL_ASSERT(lbReturn == true);
    }
}

void CServerNodeGroupWLRA::Remove( CServerNode* apoServerNode )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    this->moPriList.Remove(apoServerNode);
}

CSegmentHandle CServerNodeGroupWLRA::Allocate(void)
{
    apl_size_t luSegmentID = INVALID_ID;

    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    //Search writeable allocate by ServerNode priority
    for (CPriorityListIterator loListIter(this->moPriList);
        !loListIter.IsDone(); loListIter.Next() )
    {
        CServerNode* lpoServerNode = APL_NULL;

        while(true)
        {
            //Find least-recently-idle ServerNode
            for (CServerNodeIterator loServerNodeIter(loListIter);
                !loServerNodeIter.IsDone(); loServerNodeIter.Next() )
            {
                CServerNode* lpoTemp = loServerNodeIter.operator -> ();

                if (lpoServerNode == APL_NULL)
                {
                    if (lpoTemp->GetFreeSegmentCount() > 0)
                    {
                        lpoServerNode = lpoTemp;
                    }

                    continue;
                }
                else if (lpoTemp->GetFreeSegmentCount() > 0)
                {
                    //Compare ServerNode TPS
                    if (lpoServerNode->GetAccessedTPS() > lpoTemp->GetAccessedTPS() )
                    {
                        lpoServerNode = lpoTemp;
                    }

                    continue;
                }
            }
            
            if (lpoServerNode == APL_NULL)
            {
                break;
            }
            
            if ( (luSegmentID = lpoServerNode->Allocate() ) != INVALID_ID)
            {
                if (lpoServerNode->GetFreeSegmentCount() == 0)
                {
                    //Suspend empty server node
                    this->moPriList.Suspend(lpoServerNode);
                }

                //printf("Allocated server id = %"APL_PRIuINT", tps = %"APL_PRIuINT", weighting=%"APL_PRIuINT", total=%"APL_PRIuINT"\n", 
                //    lpoServerNode->GetServerNodeID(),
                //    loFirst.GetReadTPS() + loFirst.GetWriteTPS(),
                //    loFirst.GetAllocateWeighting(),
                //    loFirst.GetAccessedTPS() );

                return CSegmentHandle(lpoServerNode->GetServerNodeID(), luSegmentID);
            }

            //Try again
        }
    }

    return CSegmentHandle();
}

void CServerNodeGroupWLRA::Deallocate( CServerNode* apoServerNode, CSegmentHandle aoHandle )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    apoServerNode->Deallocate(aoHandle.GetSegmentID() );
        
    if (apoServerNode->GetFreeSegmentCount() == 1)
    {
        this->moPriList.Resume(apoServerNode);
    }
}

AIBC_GFQ_NAMESPACE_END

