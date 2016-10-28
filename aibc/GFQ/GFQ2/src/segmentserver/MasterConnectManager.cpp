
#include "MasterConnectManager.h"
#include "acl/SString.h"
#include "acl/SockDgram.h"

AIBC_GFQ_NAMESPACE_START

CMasterConnectManager::CMasterConnectManager(void)
    : mbIsMcastAddr(false)
{
}

apl_int_t CMasterConnectManager::Connect( acl::CSockAddr const& aoRemoteAddress )
{
    acl::CSockAddr loMasterAddr;

    this->moRemoteAddress.Set(aoRemoteAddress);
    
    apl_int_t liHigh = apl_strtoi32(this->moRemoteAddress.GetIpAddr(), APL_NULL, 10);

    if (liHigh >= 224 && liHigh <= 239)
    {
        this->mbIsMcastAddr = true;
    }
    else
    {
        this->mbIsMcastAddr = false;
    }
    
    apl_int_t liRetCode = this->moDgram.Open();
    ACL_ASSERT(liRetCode == 0);

    if (this->GetRemoteAddress(&loMasterAddr, 2) != 0)
    {
        apl_errprintf("Get remote master address fail, wait to retry it\n");
    }

    return anf::CConnectManager::Connect("master", loMasterAddr, 1, true);
}

apl_int_t CMasterConnectManager::GetRemoteAddress( acl::CSockAddr* apoRemoteAddress, apl_int_t aiTryTimes )
{
    if (this->mbIsMcastAddr)
    {
        acl::CTimeValue loTimedout(2);
        char lacBuffer[64] = {0,};

        for (apl_int_t i = 0; i < aiTryTimes; i++)
        {
            if (this->moDgram.Send("who is master", 14, this->moRemoteAddress, loTimedout) != 14)
            {
                continue;
            }

            if (this->moDgram.Recv(lacBuffer, sizeof(lacBuffer) - 1, APL_NULL, loTimedout) > 0)
            {
                acl::CTokenizer loTokenizer(":");

                loTokenizer.Parse(lacBuffer);

                if (loTokenizer.GetSize() != 2)
                {
                    continue;
                }

                apoRemoteAddress->Set(
                    apl_strtoi32(loTokenizer.GetField(1), APL_NULL, 10), loTokenizer.GetField(0) );

                return 0;
            }
        }

        return -1;
    }
    else
    {
        apoRemoteAddress->Set(this->moRemoteAddress);

        return 0;
    }
}

void CMasterConnectManager::HandleConnectException(
    char const* apcName,
    acl::CSockAddr const& aoRemoteAddress,
    apl_int_t aiState,
    apl_int_t aiErrno )
{
    acl::CSockAddr loMasterAddr;
    
    if (this->GetRemoteAddress(&loMasterAddr, 10) != 0)
    {
        apl_errprintf("Get remote master address fail, wait to retry it\n");
    }

    anf::CConnectManager::Connect("master", loMasterAddr, 1, false);
}

AIBC_GFQ_NAMESPACE_END

