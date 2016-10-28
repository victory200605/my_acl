--------------------------------------------------------------------
-- set output file name, defualt is wsdl file name
--------------------------------------------------------------------
OUTPUT_FILENAME        = FILENAME_BASE

--------------------------------------------------------------------
-- namespace map, you can add or modified it
-- soapenv/xsd/xsi/targetNamespace is defualt namespace, 
-- if unconfig will be load from wsdl file
--------------------------------------------------------------------
XML_NAMESPACE = { 
    soapenv         = { alias="soapenv", value="http://schemas.xmlsoap.org/soap/envelope/"  },
    xsd             = { alias="xsd", value="http://www.w3.org/2001/XMLSchema" },
    xsi             = { alias="xsi", value="http://www.w3.org/2001/XMLSchema-instance" },
--  targetNamespace = { alias="dsmp", value="http://www.monternet.com/dsmp/schemas" },
}

---------------------------------------------------------------------
-- *.h or *.cpp file header region, the content will paste to file immediacy,
-- and you can modified it if you want
---------------------------------------------------------------------
function __H__Header()
    return 
[[

#ifndef __AIDSMP_H__
#define __AIDSMP_H__

#include "AIMcbXML.h"
#include "AIChunkEx.h"
#include "stl/string.h"
#include "stl/vector.h"
using namespace AI_STD;

#include "WSDLBase.h"

]]

end

function __CPP__Header()
    return "" ..
           "\n" ..
           "#include \"" .. OUTPUT_FILENAME .. ".h\"\n\n" ..
[[

#define ELEMENT_ASSIGN_TO_OBJ( base, element, obj )\
   {\
        int liInd = 0;\
        AIBC::McbXMLNode* lpoSubNode = NULL;\
        while( ( lpoSubNode = AIBC::McbEnumNodes( element, &liInd ) ) != NULL )\
        {\
            if ( lpoSubNode->ciType == AIBC::eNodeText ) break;\
        }\
        base->Assign( obj, (lpoSubNode == NULL? NULL : lpoSubNode->coNode.cpoText->cpcValue ) );\
   }
]]

end

---------------------------------------------------------------------
-- *.h or *.cpp file tail region, the content will paste to file immediacy,
-- and you can modified it if you want
---------------------------------------------------------------------
function __H__Tail()
    return 
[[
#endif //__AIDSMP_H__

]]

end

function __CPP__Tail()
    return ""
end

--------------------------------------------------------------------
-- type map, wsdl base type map to cxx type
--------------------------------------------------------------------
TYPE_MAP               = {
-- xsd type                 cxx type
   {"string",               "string"           },
   {"integer",              "AIBC::AIInt"      },
   {"int",                  "AIBC::AIInt",     },
   {"long",                 "AIBC::AILong",    },
   {"boolean",              "bool",            },
   {"dateTime",             "string",          },
   {"date",                 "string",          },
   {"base64Binary",         "AIBC::AIChunkEx", }
}

---------------------------------------------------------------------
-- complex-type h file coder generate function
-- and you can modified it if you want
---------------------------------------------------------------------
function __H__ComplexType( t )
    
    name = FormatName( "st", t["name"] )
    body = t["body"]
    row  = ""
    
    for n, r in pairs(body) do
        if r["maxOccurs"] < 0 and r["minOccurs"] < 0 then
            row = row .. "    " .. FormatType( TypeFerfix(r["ptypet"]), r["ptype"] ) .. " " .. FormatName( "c" .. NameFerfix(r["type"]), r["name"] ) .. ";\n"
        else
            row = row .. "    vector<" .. FormatType( TypeFerfix(r["ptypet"]), r["ptype"] ) .. "> " .. FormatName( "c" .. NameFerfix(r["type"]), r["name"] ) .. ";\n"
        end
    end

    return "" ..
           "/**\n" ..
           " * complex-type : " .. name .. "\n" ..
           " */\n" ..
           "struct " .. name .. "\n" ..
           "{\n" ..
           "    ///operator\n" ..
           "    int Encode( clsBaseWSDL* apoBase, AIBC::McbXMLElement* apoRoot, string& aoCurrPoint );\n" ..
           "    int Decode( clsBaseWSDL* apoBase, AIBC::McbXMLElement* apoRoot, string& aoCurrPoint  );\n" ..
           "\n" ..
           "    ///attribute\n" ..
           row ..
           "};\n\n"
end

---------------------------------------------------------------------
-- port-type h file coder generate function
-- and you can modified it if you want
---------------------------------------------------------------------
function __H__PortType( t )
    
    name = FormatName( "cls", t["name"] )
    input_header_t = t["input"]["header"]
    input_body_t = t["input"]["body"]
    output_header_t = t["output"]["header"]
    output_body_t = t["output"]["body"]
    
    input_header_c = ""
    input_body_c = ""
    output_header_c = ""
    output_body_c = ""
    
    make_coder = function(l)
        result = ""
        
        for n, r in pairs(l) do
            type_name = FormatType( TypeFerfix(r["ptypet"]), r["ptype"] )
            var_name  = FormatName( "c" .. NameFerfix(r["ptype"]), r["name"] )
            result = result .. "        " .. type_name .. " " .. var_name .. ";\n"
        end
        
        return result
    end
    
    input_header_c  = make_coder(input_header_t)
    input_body_c    = make_coder(input_body_t)
    output_header_c = make_coder(output_header_t)
    output_body_c   = make_coder(output_body_t)

    return string.format( 
               "" ..
               "/**\n" ..
               " * port-type :%s\n" ..
               " */\n" ..
               "class %s\n" ..
               "{\n" ..
               "public:\n" ..
               "    struct Input : public clsBaseWSDL\n" ..
               "    {\n" ..
               "        ///operator\n" ..
               "        Input( clsSoapEnvelope* apoEnv = NULL, int aiOpt = DONT_ESCAPE ) : clsBaseWSDL(apoEnv, aiOpt) {}\n" ..
               "\n" ..
               "        const char* EncodeXMLString();\n" ..
               "        int DecodeXMLString( const char* apsXMLString );\n" ..
               "        int DecodeXMLString( clsWSDLRequest& apoRequest );\n" ..
               "\n" ..
               "        ///header\n" ..
               "%s" ..
               "\n" ..
               "        ///body\n" ..
               "%s" ..
               "    };\n" ..
               "\n" ..
               "    struct Output : public clsBaseWSDL\n" ..
               "    {\n" ..
               "        ///operator\n" ..
               "        Output( clsSoapEnvelope* apoEnv = NULL, int aiOpt = DONT_ESCAPE ) : clsBaseWSDL(apoEnv, aiOpt) {}\n" ..
               "\n" ..
               "        const char* EncodeXMLString();\n" ..
               "        int DecodeXMLString( const char* apsXMLString );\n" ..
               "        int DecodeXMLString( clsWSDLRequest& apoRequest );\n" ..
               "\n" ..
               "        ///header\n" ..
               "%s" ..
               "\n" ..
               "        ///body\n" ..
               "%s" ..
               "    };\n" ..
               "\n" ..
               "    static void CopyHeader( Output& aoOutput, Input& aoInupt );\n"..
               "};\n\n",
               name,
               name,
               input_header_c,
               input_body_c,
               output_header_c,
               output_body_c
    )
end

---------------------------------------------------------------------
-- complex-type cpp file coder generate function
-- and you can modified it if you want
---------------------------------------------------------------------
function __CPP__ComplexType( t )
    
    name = FormatName( "st", t["name"] )
    body = t["body"]
    encode = ""
    decode = ""
    entry_attr = ""
    element_array = ""
    
    ---- entry attr fro soap use encoded
    if SOAP_USE == "encoded" then
        entry_attr = entry_attr .. string.format( 
            "    AIBC::McbAddAttribute( lpoCurrElem, strdup( \"%s\:type\" ), strdup( \"\\\"%s\:%s\\\"\" ), 1 );\n",
            XML_NAMESPACE["xsi"]["alias"],
            XML_NAMESPACE["targetNamespace"]["alias"],
            t["name"]
        )
    end
        
    for n, r in pairs(body) do
        var_name  = FormatName( "c" .. NameFerfix(r["type"]), r["name"] )
        
        if r["ptypet"] == "complex" then
            if r["maxOccurs"] < 0 and r["minOccurs"] < 0 then
                -------------------------------- encode ------------------------------------
                encode = encode .. string.format( 
                   "        // for member %s \n" ..
                   "        lpoSubElem = AIBC::McbAddElement( lpoCurrElem, apoBase->ToString( \"\", \"%s\" ), 1, 1 );\n" ..
                   "        liRetCode = this->%s.Encode( apoBase, lpoSubElem, loStrCurrElem );\n"..
                   "        AI_GOTO_IF( ERROR, (liRetCode != AISOAP::AI_NO_ERROR) );\n\n",
                   var_name,
                   r["name"],
                   var_name                 
                )
                
                -------------------------------- decode ------------------------------------
                decode = decode .. string.format( 
                   "        // for membr %s \n"..
                   "        lpoSubElem = AISOAP::McbFindElementNns( lpoCurrElem, \"%s\" );\n"..
                   "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_ELEMENT_UNEXIST, lpoSubElem == NULL), (loStrCurrElem = \"%s\") );\n"..
                   "        liRetCode = this->%s.Decode( apoBase, lpoSubElem, loStrCurrElem );\n"..
                   "        AI_GOTO_IF( ERROR, (liRetCode != AISOAP::AI_NO_ERROR) );\n",
                   var_name,
                   r["name"],
                   r["name"],
                   var_name
                )
            else
                ------------------------------------ encode --------------------------------
                encode = encode .. string.format(
                   "        // for array member %s \n" ..
                   "        liSize = this->%s.size();\n",
                   var_name,
                   var_name
                )
                
                if r["minOccurs"] >= 0 then
                    encode = encode .. string.format(
                       "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_OUT_OF_RANGE, liSize < %d), (loStrCurrElem = \"%s\")  );\n",
                        r["minOccurs"],
                        r["name"]
                    )
                end
                
                if r["maxOccurs"] > 0 then
                    encode = encode .. string.format(
                        "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_OUT_OF_RANGE, liSize > %d), (loStrCurrElem = \"%s\") );\n",
                        r["maxOccurs"],
                        r["name"]
                    )
                end
                
                encode = encode .. string.format( 
                   "        for( size_t liIdx = 0; liIdx < liSize; liIdx++ )\n" ..
                   "        {\n" ..
                   "            lpoSubElem = AIBC::McbAddElement( lpoCurrElem, apoBase->ToString( \"\", \"%s\" ), 1, 1 );\n" ..
                   "            liRetCode = this->%s[liIdx].Encode( apoBase, lpoSubElem, loStrCurrElem );\n" ..
                   "            AI_GOTO_IF( ERROR, (liRetCode != AISOAP::AI_NO_ERROR) );\n"..
                   "        }\n",
                   r["name"],
                   var_name
                )
                
                ----------------------------------------- decode ------------------------------
                element_array = "    vector<AIBC::McbXMLElement*> loXmlElements;\n"
                
                decode = decode .. string.format(
                   "        // for array member %s \n" ..
                   "        liSize = AISOAP::McbFindElementsNns( lpoCurrElem, \"%s\", loXmlElements);\n",
                   var_name,
                   r["name"]
                )
                
                if r["minOccurs"] >= 0 then
                    decode = decode .. string.format(
                       "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_OUT_OF_RANGE, liSize < %d), (loStrCurrElem = \"%s\")  );\n",
                        r["minOccurs"],
                        r["name"]
                    )
                end
                
                if r["maxOccurs"] > 0 then
                    decode = decode .. string.format(
                        "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_OUT_OF_RANGE, liSize > %d), (loStrCurrElem = \"%s\") );\n",
                        r["maxOccurs"],
                        r["name"]
                    )
                end

                decode = decode .. string.format( 
                   "        this->%s.resize( liSize );\n" ..
                   "        for( size_t liIdx = 0; liIdx < liSize; liIdx++ )\n" ..
                   "        {\n" ..
                   "            liRetCode = this->%s[liIdx].Decode( apoBase, loXmlElements[liIdx], loStrCurrElem );\n" ..
                   "            AI_GOTO_IF( ERROR, (liRetCode != AISOAP::AI_NO_ERROR) );\n"..
                   "        }\n",
                   var_name,
                   var_name
                )
             end
        else
            if r["maxOccurs"] < 0 and r["minOccurs"] < 0 then
                encode = encode .. string.format( 
                   "        // for member %s , use=%s\n" ..
                   "        lpoSubElem = AIBC::McbAddElement( lpoCurrElem, apoBase->ToString( \"\", \"%s\" ), 1, 1 );\n" ..
                   "        AIBC::McbAddText( lpoSubElem, apoBase->ToString( this->%s ), 1 );\n\n",
                   var_name,
                   SOAP_USE,
                   r["name"],
                   var_name
                )
                
                if SOAP_USE == "encoded" then
                    encode = encode .. string.format( 
                       "        AIBC::McbAddAttribute( lpoSubElem, strdup( \"%s\:type\" ), strdup( \"\\\"%s\:%s\\\"\" ), 1 );\n\n",
                       XML_NAMESPACE["xsi"]["alias"],
                       XML_NAMESPACE["xsd"]["alias"],
                       ToXsdType( r["ptype"] )
                    )
                end
                
                decode = decode .. string.format( 
                   "        // for membr %s \n"..
                   "        lpoSubElem = AISOAP::McbFindElementNns( lpoCurrElem, \"%s\" );\n"..
                   "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_ELEMENT_UNEXIST, lpoSubElem == NULL), (loStrCurrElem = \"%s\") );\n"..
                   "        ELEMENT_ASSIGN_TO_OBJ( apoBase, lpoSubElem, this->%s );\n",
                   var_name,
                   r["name"],
                   r["name"],
                   var_name
                )
            else
                ------------------------------------ encode --------------------------------
                encode = encode .. string.format(
                   "        // for array member %s \n" ..
                   "        liSize = this->%s.size();\n",
                   var_name,
                   var_name
                )
                
                if r["minOccurs"] >= 0 then
                    encode = encode .. string.format(
                       "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_OUT_OF_RANGE, liSize < %d), (loStrCurrElem = \"%s\")  );\n",
                        r["minOccurs"],
                        r["name"]
                    )
                end
                
                if r["maxOccurs"] > 0 then
                    encode = encode .. string.format(
                        "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_OUT_OF_RANGE, liSize > %d), (loStrCurrElem = \"%s\") );\n",
                        r["maxOccurs"],
                        r["name"]
                    )
                end
                
                if SOAP_USE == "encoded" then
                    encode = encode .. string.format(
                       "        for( size_t liIdx = 0; liIdx < liSize; liIdx++ )\n" ..
                       "        {\n" ..
                       "            // for member %s \n" ..
                       "            lpoSubElem = AIBC::McbAddElement( lpoCurrElem, apoBase->ToString( \"\", \"%s\" ), 1, 1 );\n" ..
                       "            AIBC::McbAddText( lpoSubElem, apoBase->ToString( this->%s[liIdx] ), 1 );\n" ..
                       "            AIBC::McbAddAttribute( lpoSubElem, strdup( \"%s\:type\" ), strdup( \"\\\"%s\:%s\\\"\" ), 1 );\n\n"..
                       "        }\n",
                       var_name,
                       r["name"],
                       var_name,
                       XML_NAMESPACE["xsi"]["alias"],
                       XML_NAMESPACE["xsd"]["alias"],
                       ToXsdType( r["ptype"] )
                    )
                else
                    encode = encode .. string.format( 
                       "        for( size_t liIdx = 0; liIdx < liSize; liIdx++ )\n" ..
                       "        {\n" ..
                       "            // for member %s \n" ..
                       "            lpoSubElem = AIBC::McbAddElement( lpoCurrElem, apoBase->ToString( \"\", \"%s\" ), 1, 1 );\n" ..
                       "            AIBC::McbAddText( lpoSubElem, apoBase->ToString( this->%s[liIdx] ), 1 );\n" ..
                       "        }\n",
                       var_name,
                       r["name"],
                       var_name
                    )
                end
                
                ------------------------------------ decode --------------------------------
                element_array = "    vector<AIBC::McbXMLElement*> loXmlElements;\n"
                
                decode = decode .. string.format(
                   "        // for array member %s \n" ..
                   "        liSize = AISOAP::McbFindElementsNns( lpoCurrElem, \"%s\", loXmlElements);\n",
                   var_name,
                   r["name"]
                )
                
                if r["minOccurs"] >= 0 then
                    decode = decode .. string.format(
                       "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_OUT_OF_RANGE, liSize < %d), (loStrCurrElem = \"%s\")  );\n",
                        r["minOccurs"],
                        r["name"]
                    )
                end
                
                if r["maxOccurs"] > 0 then
                    decode = decode .. string.format(
                        "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_OUT_OF_RANGE, liSize > %d), (loStrCurrElem = \"%s\") );\n",
                        r["maxOccurs"],
                        r["name"]
                    )
                end

                decode = decode .. string.format(
                   "        this->%s.resize( liSize );\n" ..
                   "        for( size_t liIdx = 0; liIdx < liSize; liIdx++ )\n" ..
                   "        {\n" ..
                   "            ELEMENT_ASSIGN_TO_OBJ( apoBase, loXmlElements[liIdx], this->%s[liIdx] );\n"..
                   "        }\n",
                   var_name,
                   var_name
                )
            end
        end
    end

    return string.format( 
           "/**\n" ..
           " * complex-type : %s\n" ..
           " */\n" ..
           "int %s::Encode( clsBaseWSDL* apoBase, AIBC::McbXMLElement* apoRoot, string& aoStrCurrElement )\n" ..
           "{\n" ..
           "    /* start to create XML string for struct sms_cdr_schema */ \n" ..
           "    size_t         liSize;\n" ..
           "    int            liRetCode;\n" ..
           "    string&        loStrCurrElem = aoStrCurrElement;\n" ..
           "    AIBC::McbXMLElement* lpoSubElem;\n" ..
           "    AIBC::McbXMLElement* lpoCurrElem;\n" ..
           "\n" ..
           "    liSize         = 0;\n" ..
           "    liRetCode      = AISOAP::AI_NO_ERROR;\n" ..
           "    loStrCurrElem  = \"\";\n" ..
           "    lpoSubElem     = NULL;\n" ..
           "    lpoCurrElem    = apoRoot;\n" ..
           "%s\n"..
           "    do\n" ..
           "    {\n" ..
           "%s" ..
           "        return AISOAP::AI_NO_ERROR;\n" ..
           "    }\n" ..
           "    while( false );\n" ..
           "    goto ERROR;\n" ..
           "\n" ..
           "ERROR:\n" ..
           "    return liRetCode;\n" ..
           "}\n\n" ..
           "int %s::Decode( clsBaseWSDL* apoBase, AIBC::McbXMLElement* apoRoot, string& aoStrCurrElement )\n"..
           "{\n"..
           "    // Declare varible \n"..
           "    size_t         liSize;\n"..
           "    int            liInd;\n"..
           "    int            liRetCode;\n"..
           "    string&        loStrCurrElem = aoStrCurrElement;\n"..
           "    AIBC::McbXMLElement* lpoSubElem;\n".. element_array ..
           "    AIBC::McbXMLElement* lpoCurrElem;\n"..
           "\n"..
           "    // Initailize varible \n"..
           "    liSize         = 0;\n"..
           "    liInd          = 0;\n"..
           "    liRetCode      = AISOAP::AI_NO_ERROR;\n"..
           "    loStrCurrElem  = \"\";\n"..
           "    lpoSubElem     = NULL;\n"..
           "    lpoCurrElem    = apoRoot;\n"..
           "\n"..
           "    // Start encode \n"..
           "    do\n"..
           "    {\n"..
           "\n"..
           "%s"..
           "\n"..
           "        return AISOAP::AI_NO_ERROR;\n"..
           "    }\n"..
           "    while( false );\n"..
           "    goto ERROR;\n"..
           "\n"..
           "ERROR:\n"..
           "    return liRetCode;\n"..
           "}\n",
           name,
           name,
           entry_attr,
           encode,
           name,
           decode
    )
end

---------------------------------------------------------------------
-- port-type cpp file coder generate function
-- and you can modified it if you want
---------------------------------------------------------------------
function __CPP__PortType( t )
    
    name = FormatName( "cls", t["name"] )
    input_header_t = t["input"]["header"]
    input_body_t = t["input"]["body"]
    output_header_t = t["output"]["header"]
    output_body_t = t["output"]["body"]
    
    input_header_e = ""
    input_body_e = ""
    output_header_e = ""
    output_body_e = ""
    input_header_d = ""
    input_body_d = ""
    output_header_d = ""
    output_body_d = ""
    soapattr = ""
    input_entry_opt_e = ""
    input_entry_opt_d = ""
    output_entry_opt_e = ""
    output_entry_opt_d = ""
    header_copy = ""
    name_space  = XML_NAMESPACE["targetNamespace"]["alias"]
    
    ---- process soapevn namespace
    for n, r in pairs(XML_NAMESPACE) do
        soapattr = soapattr .. string.format(
            "        this->GetSoapEnvelope()->PutEnvelopeAttr(\"xmlns:%s\", \"\\\"%s\\\"\");\n",
            r["alias"],
            r["value"]
        )
    end
    
    ---- entry operation for soap rpc
    if SOAP_STYLE == "rpc" then
        response_name = t["name"] .. "Response"
        
        input_entry_opt_e = input_entry_opt_e .. string.format( 
            "        lpoCurrElem = AIBC::McbAddElement( lpoCurrElem, ToString(\"%s\",\"%s\"), 1, 1 );\n",
            XML_NAMESPACE["targetNamespace"]["alias"],
            t["name"]
        )
        input_entry_opt_d = input_entry_opt_d .. string.format( 
            "        lpoCurrElem = AISOAP::McbFindElementNns( lpoCurrElem, \"%s\" );\n"..
            "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_ELEMENT_UNEXIST, lpoCurrElem == NULL), (loStrCurrElem = \"%s\") );\n",
            t["name"],
            t["name"]
        )
        
        output_entry_opt_e = output_entry_opt_e .. string.format( 
            "        lpoCurrElem = AIBC::McbAddElement( lpoCurrElem, ToString(\"%s\",\"%s\"), 1, 1 );\n",
            XML_NAMESPACE["targetNamespace"]["alias"],
            response_name
        )
        output_entry_opt_d = output_entry_opt_d .. string.format( 
            "        lpoCurrElem = AISOAP::McbFindElementNns( lpoCurrElem, \"%s\" );\n"..
            "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_ELEMENT_UNEXIST, lpoCurrElem == NULL), (loStrCurrElem = \"%s\") );\n",
            response_name,
            response_name
        )
        
        name_space = ""
    end

    make_coder = function( l )
        encode = ""
        decode = ""
        
        for n, r in pairs(l) do
            element_name = ""
            
            if r["typet"] == "element" then
                element_name = r["type"]
            else
                element_name = r["name"]
            end
            
            var_name  = FormatName( "c" .. NameFerfix(r["ptype"]), r["name"] )
            
            if r["ptypet"] == "complex" then
                encode = encode .. string.format( 
                   "        // for member %s \n" ..
                   "        lpoSubElem = AIBC::McbAddElement( lpoCurrElem, ToString( \"%s\", \"%s\" ), 1, 1 );\n" ..
                   "        liRetCode = this->%s.Encode( this, lpoSubElem, loStrCurrElem );\n"..
                   "        AI_GOTO_IF( ERROR, (liRetCode != AISOAP::AI_NO_ERROR) );\n\n",
                   var_name,
                   name_space,
                   element_name,
                   var_name                   
                )
                decode = decode .. string.format( 
                   "        // for membr %s \n"..
                   "        lpoSubElem = AISOAP::McbFindElementNns( lpoCurrElem, \"%s\" );\n"..
                   "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_ELEMENT_UNEXIST, lpoSubElem == NULL), (loStrCurrElem = \"%s\") );\n"..
                   "        liRetCode = this->%s.Decode( this, lpoSubElem, loStrCurrElem );\n"..
                   "        AI_GOTO_IF( ERROR, (liRetCode != AISOAP::AI_NO_ERROR) );\n",
                   var_name,
                   element_name,
                   element_name,
                   var_name
                   
                )
            else
                encode = encode .. string.format(
                   "        // for member %s \n" ..
                   "        lpoSubElem = AIBC::McbAddElement( lpoCurrElem, ToString( \"%s\", \"%s\" ), 1, 1 );\n" ..
                   "        AIBC::McbAddText( lpoSubElem, ToString( this->%s ), 1 );\n\n",
                   var_name,
                   name_space,
                   element_name,
                   var_name
                )
                
                if SOAP_USE == "encoded" then
                    encode = encode .. string.format( 
                       "        AIBC::McbAddAttribute( lpoSubElem, strdup( \"%s\:type\" ), strdup( \"\\\"%s\:%s\\\"\" ), 1 );\n\n",
                       XML_NAMESPACE["xsi"]["alias"],
                       XML_NAMESPACE["xsd"]["alias"],
                       ToXsdType( r["ptype"] )
                    )
                end
                
                decode = decode .. string.format( 
                   "        // for membr %s \n"..
                   "        lpoSubElem = AISOAP::McbFindElementNns( lpoCurrElem, \"%s\" );\n"..
                   "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_ELEMENT_UNEXIST, lpoSubElem == NULL), (loStrCurrElem = \"%s\") );\n"..
                   "        ELEMENT_ASSIGN_TO_OBJ( this, lpoSubElem, this->%s );\n",
                   var_name,
                   element_name,
                   element_name,
                   var_name
                )
            end
        end
        
        return encode, decode
    end

    input_header_e, input_header_d   = make_coder(input_header_t)
    input_body_e, input_body_d       = make_coder(input_body_t)
    output_header_e, output_header_d = make_coder(output_header_t)
    output_body_e, output_body_d     = make_coder(output_body_t)

    ---- header copy
    for n, r in pairs(input_header_t) do
        var_name  = FormatName( "c" .. NameFerfix(r["ptype"]), r["name"] )
        header_copy = header_copy .. "    aoOutput." .. var_name .. " = aoInput." .. var_name .. ";\n"
    end
    
    header_copy = string.format(
              "void %s::CopyHeader( %s::Output& aoOutput, %s::Input& aoInput )\n"..
              "{\n"..
              "%s"..
              "}\n\n",
              name,
              name,
              name,
              header_copy
    )
    
    intput = string.format( 
               "const char* %s::Input::EncodeXMLString()\n"..
               "{\n"..
               "    // Declare varible \n"..
               "    size_t         liSize;\n"..
               "    int            liRetCode;\n"..
               "    AIBC::McbXMLElement* lpoSubElem;\n"..
               "    AIBC::McbXMLElement* lpoCurrElem; \n"..
               "    string         loStrCurrElem;\n"..
               "    AIBC::McbXMLElement* lpoEnvelopeElem; \n"..
               "    AIBC::McbXMLElement* lpoRootElem; \n"..
               "\n"..
               "    // Initailize varible \n"..
               "    liSize          = 0;\n"..
               "    liRetCode       = 0;\n"..
               "    lpoSubElem      = NULL;\n"..
               "    lpoRootElem     = AIBC::McbCreateRoot(); \n"..
               "    lpoCurrElem     = lpoRootElem;\n"..
               "    lpoEnvelopeElem = lpoRootElem;\n"..
               "\n"..
               "    // Start encode \n"..
               "    do\n"..
               "    {\n"..
               "        // Envelope \n"..
               "        AI_GOTO_IF( BODY, ( this->GetSoapEnvelope() == NULL ) );\n"..
               "\n"..
               "        this->GetSoapEnvelope()->SetEnvelopeNamespace(\"%s\");\n"..
               "\n"..
               "%s"..
               "\n"..
               "        this->EncodeXmlHeader( lpoRootElem );\n"..
               "        lpoEnvelopeElem = GetSoapEnvelope()->EncodeSoapEnvelope( lpoRootElem );\n"..
               "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_SOAP_ENV, lpoEnvelopeElem == NULL), (loStrCurrElem = \"SoapEnvelope\") );\n"..
               "\n"..
               "        // Start header \n"..
               "        lpoCurrElem = GetSoapEnvelope()->EncodeSoapHeader( lpoEnvelopeElem );\n"..
               "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_SOAP_HEADER, lpoCurrElem == NULL), (loStrCurrElem = \"SoapHeader\") );\n"..
               "\n"..
               "%s"..
               "\n"..
               "        // Start body \n"..
               "        lpoCurrElem = GetSoapEnvelope()->EncodeSoapBody( lpoEnvelopeElem );\n"..
               "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_SOAP_BODY, lpoCurrElem == NULL), (loStrCurrElem = \"SoapBody\")  );\n"..
               "BODY:\n"..
               "%s"..
               "%s"..
               "\n"..
               "        liRetCode = this->CreateXMLString( lpoRootElem );\n"..
               "        AI_GOTO_IF( ERROR, ( liRetCode != AISOAP::AI_NO_ERROR ) );\n"..
               "        AIBC::McbDeleteRoot( lpoRootElem );\n"..
               "        return this->ResultPointer();\n"..
               "    }\n"..
               "    while( false );\n"..
               "    goto ERROR;\n"..
               "\n"..
               "ERROR:\n"..
               "    this->SetCurrElement( loStrCurrElem.c_str() );\n"..
               "    AIBC::McbDeleteRoot( lpoRootElem );\n"..
               "    return NULL;\n"..
               "}\n\n"..
               "int %s::Input::DecodeXMLString( const char* apsXMLString )\n"..
               "{\n"..
               "    int liRetCode = AISOAP::AI_NO_ERROR;\n"..
               "    clsWSDLRequest loRequest( this->GetSoapEnvelope() );\n"..
               "    if ( ( liRetCode = loRequest(apsXMLString) ) != AISOAP::AI_NO_ERROR )\n"..
               "    {\n"..
               "        return liRetCode;\n"..
               "    }\n"..
               "    return this->DecodeXMLString( loRequest );\n"..
               "}\n"..
               "\n"..
               "int %s::Input::DecodeXMLString( clsWSDLRequest& aoRequest )\n"..
               "{\n"..
               "    // Declare varible \n"..
               "    int            liInd  = 0;\n"..
               "    size_t         liSize = 0;\n"..
               "    int            liRetCode;\n"..
               "    string         loStrCurrElem;\n"..
               "    AIBC::McbXMLElement* lpoSubElem = NULL;\n"..
               "    AIBC::McbXMLElement* lpoCurrElem = NULL;\n"..
               "    AIBC::McbXMLElement* lpoEnvelopeElem = NULL;\n"..
               "    AIBC::McbXMLElement* lpoRootElem;\n"..
               "    \n"..
               "    // Initailize varible \n"..
               "    liInd           = 0;\n"..
               "    liSize          = 0;\n"..
               "    liRetCode       = 0;\n"..
               "    lpoSubElem      = NULL;\n"..
               "    lpoRootElem     = aoRequest.GetRootElement();\n"..
               "    lpoCurrElem     = lpoRootElem;\n"..
               "    lpoEnvelopeElem = lpoRootElem;\n"..
               "    AI_RETURN_IF ( AISOAP::AI_ERROR_PARSE_XML, (lpoRootElem == NULL) ); \n"..
               "\n"..
               "    // Start encode \n"..
               "    do\n"..
               "    {\n"..
               "        // Check Envelope \n"..
               "        AI_GOTO_IF( BODY, ( aoRequest.GetSoapEnvelope() == NULL ) );\n"..
               "\n"..
               "        this->DecodeXmlHeader( lpoRootElem );\n"..
               "        lpoEnvelopeElem = aoRequest.GetEnvelope();\n"..
               "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_SOAP_ENV, lpoEnvelopeElem == NULL), (loStrCurrElem = \"SoapEnvelope\") );\n"..
               "        // Start header \n"..
               "        lpoCurrElem = aoRequest.GetHeader();\n"..
               "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_SOAP_HEADER, lpoCurrElem == NULL), (loStrCurrElem = \"SoapHeader\") );\n"..
               "\n"..
               "%s"..
               "\n"..
               "        // Start body \n"..
               "        lpoCurrElem = aoRequest.GetBody();\n"..
               "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_SOAP_BODY, lpoCurrElem == NULL), (loStrCurrElem = \"SoapBody\") );\n"..
               "BODY:\n"..
               "%s"..
               "%s"..
               "\n"..
               "        return AISOAP::AI_NO_ERROR;\n"..
               "    }\n"..
               "    while( false );\n"..
               "    goto ERROR;\n"..
               "\n"..
               "ERROR:\n"..
               "    this->SetCurrElement(loStrCurrElem.c_str());\n"..
               "    return liRetCode;\n"..
               "}\n\n",
               name,
               XML_NAMESPACE["soapenv"]["alias"],
               soapattr,
               input_header_e,
               input_entry_opt_e,
               input_body_e,
               name,
               name,
               input_header_d,
               input_entry_opt_d,
               input_body_d
    )
    
    output = string.format( 
               "const char* %s::Output::EncodeXMLString()\n"..
               "{\n"..
               "    // Declare varible \n"..
               "    size_t         liSize;\n"..
               "    int            liRetCode;\n"..
               "    AIBC::McbXMLElement* lpoSubElem;\n"..
               "    AIBC::McbXMLElement* lpoCurrElem; \n"..
               "    string         loStrCurrElem;\n"..
               "    AIBC::McbXMLElement* lpoEnvelopeElem; \n"..
               "    AIBC::McbXMLElement* lpoRootElem; \n"..
               "\n"..
               "    // Initailize varible \n"..
               "    liSize          = 0;\n"..
               "    liRetCode       = 0;\n"..
               "    lpoSubElem      = NULL;\n"..
               "    lpoRootElem     = AIBC::McbCreateRoot(); \n"..
               "    lpoCurrElem     = lpoRootElem;\n"..
               "    lpoEnvelopeElem = lpoRootElem;\n"..
               "\n"..
               "    // Start encode \n"..
               "    do\n"..
               "    {\n"..
               "        // Envelope \n"..
               "        AI_GOTO_IF( BODY, ( this->GetSoapEnvelope() == NULL ) );\n"..
               "\n"..
               "        this->GetSoapEnvelope()->SetEnvelopeNamespace(\"%s\");\n"..
               "\n"..
               "%s"..
               "\n"..
               "        this->EncodeXmlHeader( lpoRootElem );\n"..
               "        lpoEnvelopeElem = GetSoapEnvelope()->EncodeSoapEnvelope( lpoRootElem );\n"..
               "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_SOAP_ENV, lpoEnvelopeElem == NULL), (loStrCurrElem = \"SoapEnvelope\") );\n"..
               "\n"..
               "        // Start header \n"..
               "        lpoCurrElem = GetSoapEnvelope()->EncodeSoapHeader( lpoEnvelopeElem );\n"..
               "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_SOAP_HEADER, lpoCurrElem == NULL), (loStrCurrElem = \"SoapHeader\") );\n"..
               "\n"..
               "%s"..
               "\n"..
               "        // Start body \n"..
               "        lpoCurrElem = GetSoapEnvelope()->EncodeSoapBody( lpoEnvelopeElem );\n"..
               "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_SOAP_BODY, lpoCurrElem == NULL), (loStrCurrElem = \"SoapBody\")  );\n"..
               "BODY:\n"..
               "%s"..
               "%s"..
               "\n"..
               "        liRetCode = this->CreateXMLString( lpoRootElem );\n"..
               "        AI_GOTO_IF( ERROR, ( liRetCode != AISOAP::AI_NO_ERROR ) );\n"..
               "        AIBC::McbDeleteRoot( lpoRootElem );\n"..
               "        return this->ResultPointer();\n"..
               "    }\n"..
               "    while( false );\n"..
               "    goto ERROR;\n"..
               "\n"..
               "ERROR:\n"..
               "    this->SetCurrElement( loStrCurrElem.c_str() );\n"..
               "    AIBC::McbDeleteRoot( lpoRootElem );\n"..
               "    return NULL;\n"..
               "}\n\n"..
               "int %s::Output::DecodeXMLString( const char* apsXMLString )\n"..
               "{\n"..
               "    int liRetCode = AISOAP::AI_NO_ERROR;\n"..
               "    clsWSDLRequest loRequest( this->GetSoapEnvelope() );\n"..
               "    if ( ( liRetCode = loRequest(apsXMLString) ) != AISOAP::AI_NO_ERROR )\n"..
               "    {\n"..
               "        return liRetCode;\n"..
               "    }\n"..
               "    return this->DecodeXMLString( loRequest );\n"..
               "}\n\n"..
               "int %s::Output::DecodeXMLString( clsWSDLRequest& apoRequest )\n"..
               "{\n"..
               "    // Declare varible \n"..
               "    int            liInd  = 0;\n"..
               "    size_t         liSize = 0;\n"..
               "    int            liRetCode;\n"..
               "    string         loStrCurrElem;\n"..
               "    AIBC::McbXMLElement* lpoSubElem = NULL;\n"..
               "    AIBC::McbXMLElement* lpoCurrElem = NULL;\n"..
               "    AIBC::McbXMLElement* lpoEnvelopeElem = NULL;\n"..
               "    AIBC::McbXMLElement* lpoRootElem;\n"..
               "    \n"..
               "    // Initailize varible \n"..
               "    liInd           = 0;\n"..
               "    liSize          = 0;\n"..
               "    liRetCode       = 0;\n"..
               "    lpoSubElem      = NULL;\n"..
               "    lpoRootElem     = apoRequest.GetRootElement();\n"..
               "    lpoCurrElem     = lpoRootElem;\n"..
               "    lpoEnvelopeElem = lpoRootElem;\n"..
               "    AI_RETURN_IF ( AISOAP::AI_ERROR_PARSE_XML, (lpoRootElem == NULL) ); \n"..
               "\n"..
               "    // Start encode \n"..
               "    do\n"..
               "    {\n"..
               "        // Check Envelope \n"..
               "        AI_GOTO_IF( BODY, ( this->GetSoapEnvelope() == NULL ) );\n"..
               "\n"..
               "        this->DecodeXmlHeader( lpoRootElem );\n"..
               "        lpoEnvelopeElem = apoRequest.GetEnvelope();\n"..
               "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_SOAP_ENV, lpoEnvelopeElem == NULL), (loStrCurrElem = \"SoapEnvelope\") );\n"..
               "        // Start header \n"..
               "        lpoCurrElem = apoRequest.GetHeader();\n"..
               "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_SOAP_HEADER, lpoCurrElem == NULL), (loStrCurrElem = \"SoapHeader\") );\n"..
               "\n"..
               "%s"..
               "\n"..
               "        // Start body \n"..
               "        lpoCurrElem = apoRequest.GetBody();\n"..
               "        AI_GOTO_IF_EX( ERROR, (liRetCode = AISOAP::AI_ERROR_SOAP_BODY, lpoCurrElem == NULL), (loStrCurrElem = \"SoapBody\") );\n"..
               "BODY:\n"..
               "%s"..
               "%s"..
               "\n"..
               "        return AISOAP::AI_NO_ERROR;\n"..
               "    }\n"..
               "    while( false );\n"..
               "    goto ERROR;\n"..
               "\n"..
               "ERROR:\n"..
               "    this->SetCurrElement(loStrCurrElem.c_str());\n"..
               "    return liRetCode;\n"..
               "}\n\n",
               name,
               XML_NAMESPACE["soapenv"]["alias"],
               soapattr,
               output_header_e,
               output_entry_opt_e,
               output_body_e,
               name,
               name,
               output_header_d,
               output_entry_opt_d,
               output_body_d
    )
    
    return intput .. output .. header_copy
end

---------------------------------------------------------------------
-- common function, DON'T modified it, this is not udf
---------------------------------------------------------------------
function NameFerfix( name )
    if name == "string" then
        return "s"
    elseif name == "AIBC::AIInt" then
        return "i"
    elseif name == "AIBC::AILong" then
        return "l"
    elseif name == "bool" then
        return "b"
    else
        return "o"
    end
end

function TypeFerfix( typet )
    if typet == "complex" then
        return "st"
    else
        return ""
    end
end

function FormatName( perfix, name )
    retname = perfix
    flag = true
    
    string.gsub( name, ".",
        function(c)
            if c == '_' or c == '.' then
                flag = true
                return
            end
            
            if flag then
                retname = retname .. string.upper(c)
                flag = false
            else
                retname = retname .. c
            end
        end
    )
    
    return retname
end

function FormatType( perfix, name )
    if perfix == "st" then
        return FormatName( perfix, name )
    else
        return perfix .. name
    end
end

function ToXsdType( type )
    for n, r in pairs(TYPE_MAP) do
        if r[2] == type then
            return r[1]
        end
    end
    
    error( "\""..type.."\" unexisted in type map!" )
end
