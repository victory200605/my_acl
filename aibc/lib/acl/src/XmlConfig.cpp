
#include "acl/XmlConfig.h"
#include "acl/FileStream.h"
#include "acl/Xml.h"

ACL_NAMESPACE_START

const apl_int_t CXmlConfig::MAX_LINE_LEN = 1024;

apl_int_t CXmlConfig::Open( char const* apcFileName )
{
    apl_ssize_t liCurrFieldID = -1;
    char        lacLine[MAX_LINE_LEN] = {0};
    CFileStream loFile;
    std::string loXml;
    CXmlParser  loParser;
    CXmlRootElement loRootElement;
    
    if ( (loFile.Open(apcFileName, APL_O_RDONLY) ) != 0)
    {
        return -1;
    }
    
    this->moFileName = apcFileName;
    
    while(loFile.ReadLine(lacLine, sizeof(lacLine) ) > 0)
    {
        loXml += lacLine;
    }
    
    loFile.Close();
        
    if ( loParser.Parse(loXml.c_str(), loRootElement) != 0 )
    {
        return -1;
    }
    
    for ( CXmlElement::IteratorType loFieldIter = loRootElement.BeginElement();
          loFieldIter != loRootElement.End(); loFieldIter++ )
    {
        //Get field
        liCurrFieldID = this->AddField( loFieldIter->GetName() );
        for ( CXmlElement::IteratorType loKeyIter = loFieldIter->BeginElement();
              loKeyIter != loFieldIter->End(); loKeyIter++ )
        {
            CXmlText::IteratorType loTextIter = loKeyIter->BeginText();
            if ( loTextIter == loKeyIter->End() )
            {
                continue;
            }
            
            //Get key
            this->AddKey( liCurrFieldID, loKeyIter->GetName(), loTextIter->GetValue() );
        }
    }
    
    return 0;
}

apl_int_t CXmlConfig::Sync(void)
{
    CFile           loFile;
    CXmlCreator     loCreator;
    CXmlRootElement loRootElement;
    
        
    if ( this->moFileName.empty() )
    {
        apl_set_errno(APL_EINVAL);
        return -1;
    }
    
    if ( loFile.Open(this->moFileName.c_str(), APL_O_CREAT | APL_O_RDWR | APL_O_TRUNC, 0600) != 0 )
    {
        return -1;
    }
    
    for( apl_size_t liN = 0; liN < this->GetFieldSize(); liN++ )
    {
        CXmlElement::IteratorType loFieldIter = loRootElement.AddElement( this->GetField(liN).GetName() );

        for( apl_size_t liKeyN = 0; liKeyN < this->GetKeySize(liN); liKeyN++ )
        {
            CXmlElement::IteratorType loKeyIter = loFieldIter->AddElement( this->GetKey(liN, liKeyN).GetKey() );
            if ( loKeyIter != loFieldIter->End() )
            {
                loKeyIter->AddText( this->GetKey(liN, liKeyN).GetValue() );
            }
        }
    }
    
    if ( loCreator.Create(loRootElement, CXmlCreator::OPT_DEFFORMAT) != 0 )
    {
        return -1;
    }
    
    if ( loFile.Write( loCreator.GetXml() , loCreator.GetLength() ) != (apl_ssize_t)loCreator.GetLength() )
    {
        return -1;
    }
    
    return 0;
}
    
apl_int_t CXmlConfig::Reload(void)
{
    this->Clear();
    
    return this->Open( this->moFileName.c_str() );
}

void CXmlConfig::Close(void)
{
    this->Clear();
}

ACL_NAMESPACE_END
