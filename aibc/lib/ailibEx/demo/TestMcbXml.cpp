
#include <assert.h>
#include <unistd.h>
#include <stdio.h>

#include "AIMcbXML.h"


int main( int argc, char* argv[] )
{
    McbXMLElement*  lpoRootElement  = NULL;
    McbXMLElement*  lpoTmpElement   = NULL;
    McbXMLNode*     lpoNode         = NULL;
    char*           lpsXmlString    = NULL;
    size_t          liCount         = 0;
    McbXMLElement*  lpoElements[10] = {0};
    
    lpoRootElement = McbCreateRoot();
    
    // Create xml node
    lpoTmpElement = McbAddElement( lpoRootElement, "Field", 1, 1 );
    McbAddText( lpoTmpElement, "Test0", 1 );
    lpoTmpElement = McbAddElement( lpoRootElement, "Field", 1, 1 );
    McbAddText( lpoTmpElement, "Test1", 1 );
    lpoTmpElement = McbAddElement( lpoRootElement, "Field", 1, 1 );
    McbAddText( lpoTmpElement, "Test2", 1 );
    lpoTmpElement = McbAddElement( lpoRootElement, "Field", 1, 1 );
    McbAddText( lpoTmpElement, "Test3", 1 );
    
    // Create xml string
    lpsXmlString = McbCreateXMLString( lpoRootElement, 1, NULL );
    printf( "Create xml :\n%s\n", lpsXmlString );
    delete[] lpsXmlString;
    
    // Find element array
    printf( "Find element array\n" );
    liCount = McbFindElements( lpoRootElement, "Field", lpoElements, sizeof(lpoElements) / sizeof(McbXMLElement*) );
    for ( size_t liIdx = 0; liIdx < liCount; liIdx++ )
    {
        int liIndex = 0;
        lpoNode = McbEnumNodes( lpoElements[liIdx], &liIndex );
        printf( "%s : %s\n", lpoElements[liIdx]->cpcName, lpoNode->coNode.cpoText->cpcValue );
    }
    
    McbDeleteRoot( lpoRootElement );
    
    return 0; 
}
