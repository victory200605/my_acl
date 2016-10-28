
#include "acl/IniConfig.h"
#include "acl/FileStream.h"

ACL_NAMESPACE_START

const apl_int_t CIniConfig::MAX_LINE_LEN = 1024;

apl_int_t CIniConfig::Open( char const* apcFileName )
{
    apl_size_t  liLeftPos = 0;
    apl_size_t  liRightPos = 0;
    apl_ssize_t liCurrFieldID = -1;
    apl_ssize_t liCurrKeyID = -1;
    char        lacLine[MAX_LINE_LEN] = {0};
    CFileStream loFile;
    std::string loLine;
    
    if ( (loFile.Open(apcFileName, APL_O_RDONLY) ) != 0)
    {
        return -1;
    }
    
    this->moFileName = apcFileName;
    
    while(loFile.ReadLine(lacLine, sizeof(lacLine) ) >= 0)
    {
        apl_size_t liLen = apl_strlen(lacLine);
        
        //Erase "\r\n"
        if (liLen > 1 && lacLine[liLen - 1] == '\n')
        {
            lacLine[--liLen] = '\0';
        }
        
        if (liLen > 1 && lacLine[liLen - 1] == '\r')
        {
            lacLine[--liLen] = '\0';
        }
        
        //Skip blank line
        if ( liLen == 0 )
        {
            //Process Header comment
            if (liCurrFieldID == -1)
            {
                this->FixComment( -2, -1 );
            }
            
            continue;
        }
        
        loLine = lacLine;

        if ( (liLeftPos = loLine.find_first_of(";#[=") ) != std::string::npos )
        {
            switch(loLine[liLeftPos])
            {
                case '[':
                {
                    if ( ( liRightPos = loLine.find(']', liLeftPos) ) != std::string::npos 
                        && liRightPos > liLeftPos)
                    {
                        liCurrFieldID = this->AddField( loLine.substr(liLeftPos + 1, liRightPos - liLeftPos - 1).c_str() );

                        if ( (liLeftPos = loLine.find_first_of(";#", liRightPos) ) != std::string::npos )
                        {
                            this->AddComment( -1, -1, loLine.substr(liLeftPos + 1).c_str() );
                        }
                        
                        this->FixComment( liCurrFieldID, -1 );
                    }
                    break;
                }
                case '=':
                {
                    if ( ( liRightPos = loLine.find_first_of(";#", liLeftPos) ) != std::string::npos 
                        && liRightPos > liLeftPos)
                    {
                        liCurrKeyID = this->AddKey( 
                            liCurrFieldID,
                            loLine.substr(0, liLeftPos).c_str(),
                            loLine.substr(liLeftPos + 1, liRightPos - liLeftPos - 1).c_str() );
                        
                        this->AddComment( -1, -1, loLine.substr(liRightPos + 1).c_str() );
                    }
                    else
                    {
                        liCurrKeyID = this->AddKey( 
                            liCurrFieldID,
                            loLine.substr(0, liLeftPos).c_str(),
                            loLine.substr(liLeftPos + 1).c_str() );
                    }
                    
                    this->FixComment( liCurrFieldID, liCurrKeyID );
                    
                    liCurrKeyID = -1;
                    
                    break;
                }
                case ';':
                case '#':
                {
                    this->AddComment( -1, -1, loLine.substr(liLeftPos + 1).c_str() );
                    break;
                }
            };
        }
    }
    
    loFile.Close();
        
    return 0;
}

apl_int_t CIniConfig::Sync(void)
{
    CFile loFile;
    std::string loContext;
    char  lacBuffer[MAX_LINE_LEN];
        
    if ( this->moFileName.empty() )
    {
        apl_set_errno(APL_EINVAL);
        return -1;
    }
    
    //Write header comments
    for( apl_size_t liCommentID = 0; liCommentID < this->GetCommentSize(); liCommentID++ )
    {
        if ( this->GetComment(liCommentID).GetFieldID() == -2 )
        {
            apl_snprintf(lacBuffer, sizeof(lacBuffer), ";%s\n", this->GetComment(liCommentID).GetComment() );
            loContext += lacBuffer;
        }
    }
    
    if (loContext.length() > 0)
    {
        loContext += "\n";
    }
    
    //Make all field context
    for( apl_size_t liN = 0; liN < this->GetFieldSize(); liN++ )
    {
        for( apl_size_t liCommentID = 0; liCommentID < this->GetCommentSize(); liCommentID++ )
        {
            if ( this->GetComment(liCommentID).GetFieldID() == (apl_ssize_t)liN 
                && this->GetComment(liCommentID).GetKeyID() == -1 )
            {
                apl_snprintf(lacBuffer, sizeof(lacBuffer), ";%s\n", this->GetComment(liCommentID).GetComment() );
                loContext += lacBuffer;
            }
        }
        
        apl_snprintf(lacBuffer, sizeof(lacBuffer), "[%s]\n", this->GetField(liN).GetName() );
        loContext += lacBuffer;
        
        //Make all key context
        for( apl_size_t liKeyN = 0; liKeyN < this->GetKeySize(liN); liKeyN++ )
        {
            std::string loComment;
            for( apl_size_t liCommentID = 0; liCommentID < this->GetCommentSize(); liCommentID++ )
            {
                if ( this->GetComment(liCommentID).GetFieldID() == (apl_ssize_t)liN 
                    && this->GetComment(liCommentID).GetKeyID() == (apl_ssize_t)liKeyN )
                {
                    apl_snprintf(lacBuffer, sizeof(lacBuffer), ";%s\n", this->GetComment(liCommentID).GetComment() );
                    loComment += lacBuffer;
                }
            }

            if (liKeyN > 0 && loComment.length() > 0)
            {
                loContext += "\n";
            }
        
            apl_snprintf(lacBuffer, sizeof(lacBuffer), 
                    "%s\t=\t%s\n",
                    this->GetKey(liN, liKeyN).GetKey(),
                    this->GetKey(liN, liKeyN).GetValue() );
             
            loContext += loComment;
            loContext += lacBuffer;
        }

        loContext += "\n";
    }
    
    if ( (loFile.Open(this->moFileName.c_str(), APL_O_CREAT|APL_O_RDWR|APL_O_TRUNC, 0600) ) != 0)
    {
        return -1;
    }

    if (loFile.Write(loContext.c_str(), loContext.length() ) != (apl_ssize_t)loContext.length() )
    {
        return -1;
    }
    
    loFile.Close();
    
    return 0;
}
    
apl_int_t CIniConfig::Reload(void)
{
    this->Clear();
    
    return this->Open( this->moFileName.c_str() );
}

void CIniConfig::Close(void)
{
    this->Clear();
}

ACL_NAMESPACE_END
