
#ifndef ACL_LOG_H
#define ACL_LOG_H

#include "acl/Utility.h"
#include "acl/MemoryBlock.h"

ACL_NAMESPACE_START

enum ELLType
{
    LOG_LVL_SYSTEM  = 1,
    LOG_LVL_ERROR   = 2,
    LOG_LVL_WARN    = 3,
    LOG_LVL_INFO    = 4,
    LOG_LVL_DEBUG   = 5,
    LOG_LVL_TRACE   = 6,
    LOG_LVL_ANY     = 7,
    LOG_LVL_USER    = 8
};

//////////////////////////////////////// Log Format ///////////////////////////////////
struct __DefaultLogFormat
{
    apl_size_t operator () (
        apl_int_t aiLevel,
        char const* apcFormat,
        va_list& aoVl,
        char* apcBuffer,
        apl_size_t aiBufferSize );
};

class CAnyLogFormat
{
public:
    class CPlaceHolder
    {
    public:
        virtual ~CPlaceHolder(void) {}

        virtual apl_size_t Format(
            apl_int_t aiLevel,
            char const* apcFormat,
            va_list& aoVl,
            char* apcBuffer,
            apl_size_t aiBufferSize ) = 0;
        
        virtual CPlaceHolder* Clone(void) = 0;
    };
    
    template<typename FormatType>
    class CHolder : public CPlaceHolder
    {
    public:
        CHolder( FormatType const& aoFormat )
            : moFormat(aoFormat)
        {
        }

        virtual ~CHolder(void)
        {
        }
        
        apl_size_t Format(
            apl_int_t aiLevel,
            char const* apcFormat,
            va_list& aoVl,
            char* apcBuffer,
            apl_size_t aiBufferSize )
        {
            return this->moFormat(aiLevel, apcFormat, aoVl, apcBuffer, aiBufferSize);
        }
        
        virtual CPlaceHolder* Clone(void)
        {
            CHolder* lpoTmp = NULL;
            ACL_NEW_ASSERT(lpoTmp, CHolder<FormatType>(this->moFormat) );
            
            return lpoTmp;
        }
        
    private:
        FormatType moFormat;
    };
    
    typedef __DefaultLogFormat DefaultFormatType;

public:
    template<typename FormatType>
    CAnyLogFormat( FormatType const& atFormatType = DefaultFormatType() )
    {
        ACL_NEW_ASSERT( this->mpoHeld, CHolder<FormatType>(atFormatType) );
    }
    
    ~CAnyLogFormat(void)
    {
        ACL_DELETE(this->mpoHeld);
    }
    
    CAnyLogFormat( CAnyLogFormat const& aoOther )
      : mpoHeld(aoOther.mpoHeld ? aoOther.mpoHeld->Clone() : NULL)
    {
    }
    
    apl_size_t operator () (
            apl_int_t aiLevel,
            char const* apcFormat,
            va_list& aoVl,
            char* apcBuffer,
            apl_size_t aiBufferSize )
    {
        return this->mpoHeld->Format(aiLevel, apcFormat, aoVl, apcBuffer, aiBufferSize);
    }
    
private:
    CPlaceHolder* mpoHeld;
};

////////////////////////////////////////////////////////////////////////////////
template<int BufferSize>
class CDefaultFormatter
{
public:
    CDefaultFormatter(void)
        : muLength(0)
    {
        this->macBuffer[0] = '\0';
    }

    CDefaultFormatter( ELLType aeLevel, char const* apcFormat, ... )
    {
        va_list loVaList;
        __DefaultLogFormat loFormat;
        
        va_start(loVaList, apcFormat);

        this->muLength = loFormat(aeLevel, apcFormat, loVaList, this->macBuffer, sizeof(this->macBuffer) );

        va_end(loVaList);
        
        this->meLevel = aeLevel;
    }

    char const* Format( ELLType aeLevel, char const* apcFormat, ... )
    {
        va_list loVaList;
        __DefaultLogFormat loFormat;

        va_start(loVaList, apcFormat);

        this->muLength = loFormat(aeLevel, apcFormat, loVaList, this->macBuffer, sizeof(this->macBuffer) );

        va_end(loVaList);

        this->meLevel = aeLevel;

        return this->macBuffer;
    }

    char const* GetOutput(void) const
    {
        return this->macBuffer;
    }

    apl_size_t GetLength(void) const
    {
        return this->muLength;
    }

    ELLType GetLevel(void) const
    {
        return this->meLevel;
    }

private:
    char macBuffer[BufferSize];

    apl_size_t muLength;

    ELLType meLevel;
};

/**
 * @class CHexFormatter, Hex formatter, format ustring to hex.
 */
class CHexFormatter
{
public:
    /**
     * Constructor
     *
     * @param [in]    auColumn    printing the number of columns 
     * @param [in]    auGroup     printing the number of characters for a group of 
     */
    CHexFormatter(apl_size_t auColumn = 4, apl_size_t auGroup = 4);
    
    /**
     * Constructor
     *
     * @param [in]    apcInput    format input string.
     * @param [in]    auSize      string length.
     * @param [in]    auColumn    printing the number of columns 
     * @param [in]    auGroup     printing the number of characters for a group of 
     */
    CHexFormatter( char const* apcInput, apl_size_t auInputLen, apl_size_t auColumn = 4, apl_size_t auGroup = 4);
    
    /**
     * Format ustring
     *
     * @param [in]    apcInput    format input string.
     * @param [in]    auSize      string length.
     * @return  a pointer to the formated string
     */
    char const* Format(char const* apcInput, apl_size_t auInputLen, apl_size_t* apuOutputLen = NULL);
    
    char const* GetStr(void) const;
    
    char const* GetOutput(void) const;
    
    apl_size_t GetLength(void) const;
    
    void Clear(void);
    
protected:
    apl_size_t muColumn;
    apl_size_t muGroup;
    CMemoryBlock moBuffer;
};


ACL_NAMESPACE_END

#endif//ACL_LOG_H
