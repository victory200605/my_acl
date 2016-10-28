
#ifndef ANF_IOSIMPLEPROCESSOR_POOL_H
#define ANF_IOSIMPLEPROCESSOR_POOL_H

#include "anf/IoProcessor.h"
#include "anf/IoSession.h"
#include "acl/stl/vector.h"

ANF_NAMESPACE_START

template<typename ProcessorImplType> 
class TIoSimpleProcessorPool : public IoProcessor
{
public:
    TIoSimpleProcessorPool( apl_size_t auSize )
        : muCurrProcessor(0)
    {
        ProcessorImplType* lpoProcessor = APL_NULL;

        for (apl_size_t luN = 0; luN < auSize; luN++)
        {
            ACL_NEW_ASSERT(lpoProcessor, ProcessorImplType);

            this->moProcessors.push_back(lpoProcessor);
        }
        
        this->muPoolSize = this->moProcessors.size();
    }
    
    virtual ~TIoSimpleProcessorPool(void)
    {
        for (apl_size_t luN = 0; luN < this->muPoolSize; luN++)
        {
            ACL_DELETE(this->moProcessors[luN]);
        }
    }

    
    /**
     * Set the Configuration which provides I/O service to this processor
     */
    virtual void SetConfiguration( CIoConfiguration* apoConfiguration )
    {
        for (apl_size_t luN = 0; luN < this->muPoolSize; luN++)
        {
            this->moProcessors[luN]->SetConfiguration(apoConfiguration);
        }
    }
    
    /**
     * Set the Handler which provides I/O service to this processor
     */
    virtual void SetHandler( IoHandler* apoHandler )
    {
        for (apl_size_t luN = 0; luN < this->muPoolSize; luN++)
        {
            this->moProcessors[luN]->SetHandler(apoHandler);
        }
    }
    
    /** 
     * @brief Dispose I/O processor and release all related resource
     * 
     * @returns 0 if successful, otherwise -1 shall be return 
     */
    virtual apl_int_t Dispose( bool abIsWait = true )
    {
        if (abIsWait)
        {
            // Signal all first
            for (apl_size_t luN = 0; luN < this->muPoolSize; luN++)
            {
                this->moProcessors[luN]->Dispose(false);
            }
            
            // Wait all
            for (apl_size_t luN = 0; luN < this->muPoolSize; luN++)
            {
                this->moProcessors[luN]->Dispose(abIsWait);
            }
        }
        else
        {
            for (apl_size_t luN = 0; luN < this->muPoolSize; luN++)
            {
                this->moProcessors[luN]->Dispose(abIsWait);
            }
        }
        
        return 0;
    }

    /**
     * Adds the specified session to the I/O processor so that
     * the I/O processor starts to perform any I/O operations related
     * with the session.
     */
    virtual apl_int_t Add( SessionPtrType& aoSession )
    {
        return this->GetProcessor(aoSession)->Add(aoSession);
    }

    /**
     * Flushes the internal write request queue of the specified session.
     */
    virtual apl_int_t Flush( SessionPtrType& aoSession )
    {
        return this->GetProcessor(aoSession)->Flush(aoSession);
    }

    /**
     * Controls the traffic of the specified session
     */
    virtual apl_int_t UpdateTrafficControl( SessionPtrType& aoSession )
    {
        return this->GetProcessor(aoSession)->UpdateTrafficControl(aoSession);
    }

    /**
     * Removes and closes the specified session from the I/O
     * processor so that the I/O processor closes the connection
     * associated with the session and releases any other related
     * resources.
     */
    virtual apl_int_t Remove( SessionPtrType& aoSession )
    {
        return this->GetProcessor(aoSession)->Remove(aoSession);
    }

    /** 
     * Return session size of all processor
     */
    virtual apl_size_t GetSize(void)
    {
        apl_size_t luSize = 0;

        for (apl_size_t luN = 0; luN < this->muPoolSize; luN++)
        {
            luSize += this->moProcessors[luN]->GetSize();
        }

        return luSize;
    }

protected:
    IoProcessor* GetProcessor( SessionPtrType& aoSession )
    {
        IoProcessor* lpoProcessor = aoSession->GetProcessor();
        IoProcessor* lpoLeastProcessor = NULL;
        
        if (lpoProcessor == NULL)
        {
            acl::TSmartLock<acl::CLock> loGuard(this->moLock);

            //Find the least session processor
            for (apl_size_t luN = 0; luN < this->muPoolSize; luN++)
            {
                if(lpoLeastProcessor == NULL)
                {
                    lpoLeastProcessor = this->moProcessors[luN];
                }
                else if (lpoLeastProcessor->GetSize() > this->moProcessors[luN]->GetSize() )
                {
                    lpoLeastProcessor = this->moProcessors[luN];
                }
            }
        
            //Curr allocate processor
            lpoProcessor = this->moProcessors[this->muCurrProcessor];
            this->muCurrProcessor = ++this->muCurrProcessor % this->muPoolSize;

            //The best processor
            if (lpoProcessor->GetSize() > lpoLeastProcessor->GetSize() )
            {
                lpoProcessor = lpoLeastProcessor;
            }
        }
        
        return lpoProcessor;
    }

private:
    std::vector<IoProcessor*> moProcessors;
    
    apl_size_t muPoolSize;
    
    apl_size_t muCurrProcessor;

    acl::CLock moLock;
};

ANF_NAMESPACE_END

#endif//ANF_IOSIMPLEPROCESSOR_POOL_H
