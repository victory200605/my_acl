--------------------------------------------------------------------
-- set output file name, defualt is wsdl file name
--------------------------------------------------------------------
OUTPUT_FILENAME        = FILENAME_BASE

--------------------------------------------------------------------
-- enable/disable soap coder generate unit test coder
--------------------------------------------------------------------
GENERATE_TEST_CODER    = 1

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

#ifndef AIBC_SOAPC_DSMP_H
#define AIBC_SOAPC_DSMP_H

#include "WSDLBase.h"

]]

end

function __CPP__Header()
    return "" ..
           "\n" ..
           "#include \"" .. OUTPUT_FILENAME .. ".h\"\n\n" ..
[[

#define ELEMENT_ASSIGN_TO_OBJ( base, element, obj, elementname )\
   {\
        apl_size_t luInd = 0;\
        acl::xml::McbXMLNode* lpoSubNode = NULL;\
        while( ( lpoSubNode = acl::xml::McbEnumNodes( element, &luInd ) ) != NULL )\
        {\
            if (lpoSubNode->miType == acl::xml::NODE_TEXT) break;\
        }\
        base->Assign(obj, (lpoSubNode == NULL? NULL : lpoSubNode->mnNode.mpoText->mpcValue) );\
   }

]] .. GetTestUtilFunctions()

end

---------------------------------------------------------------------
-- *.h or *.cpp file tail region, the content will paste to file immediacy,
-- and you can modified it if you want
---------------------------------------------------------------------
function __H__Tail()
    return string.format("\n%s#endif //AIBC_SOAP_DSMP_H\n", GetRunTextDeclare() )
end

function __CPP__Tail()
    return GetRunTextDefine()
end

--------------------------------------------------------------------
-- type map, wsdl base type map to cxx type
--------------------------------------------------------------------
TYPE_MAP               = {
-- xsd type                 cxx type
   {"string",               "std::string"           },
   {"integer",              "soap::CInt"            },
   {"int",                  "soap::CInt",           },
   {"long",                 "soap::CLong",          },
   {"boolean",              "bool",                 },
   {"dateTime",             "std::string",          },
   {"date",                 "std::string",          },
   {"base64Binary",         "acl::CMemoryBlock",    }
}

---------------------------------------------------------------------
-- complex-type h file coder generate function
-- and you can modified it if you want
---------------------------------------------------------------------
function __H__ComplexType( t )
    
    name = FormatName( "C", t["name"] )
    body = t["body"]
    row  = ""

    for n, r in pairs(body) do
        if r["maxOccurs"] < 0 and r["minOccurs"] < 0 then
            row = row .. "    " .. FormatType( TypeFerfix(r["ptypet"]), r["ptype"] ) .. " " .. FormatName( "m" .. NameFerfix(r["type"]), r["name"] ) .. ";\n"
        else
            row = row .. "    std::vector<" .. FormatType( TypeFerfix(r["ptypet"]), r["ptype"] ) .. "> " .. FormatName( "m" .. NameFerfix(r["type"]), r["name"] ) .. ";\n"
        end
    end

    return "" ..
           "/**\n" ..
           " * complex-type : " .. name .. "\n" ..
           " */\n" ..
           "struct " .. name .. "\n" ..
           "{\n" ..
           "    ///operator\n" ..
           "    apl_int_t Encode(soap::CBaseWSDL* apoBase, acl::xml::McbXMLElement* apoRoot, std::string& aoCurrElement );\n" ..
           "    apl_int_t Decode(soap::CBaseWSDL* apoBase, acl::xml::McbXMLElement* apoRoot, std::string& aoCurrElement );\n" ..
           "\n" ..
           "    ///attribute\n" ..
           row ..
           "};\n\n".. __H__ComplexType_Test(t)
end

---------------------------------------------------------------------
-- port-type h file coder generate function
-- and you can modified it if you want
---------------------------------------------------------------------
function __H__PortType( t )
    
    name = FormatName( "C", t["name"] )
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
            var_name  = FormatName( "m" .. NameFerfix(r["ptype"]), r["name"] )
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
               "    struct Input : public soap::CBaseWSDL\n" ..
               "    {\n" ..
               "        ///operator\n" ..
               "        Input( soap::CSoapEnvelope* apoEnv = NULL, apl_int_t aiOpt = DONT_ESCAPE ) : soap::CBaseWSDL(apoEnv, aiOpt) {}\n" ..
               "\n" ..
               "        const char* Encode(void);\n" ..
               "        apl_int_t Decode( const char* apsXMLString );\n" ..
               "        apl_int_t Decode( soap::CWSDLRequest& apoRequest );\n" ..
               "\n" ..
               "        ///header\n" ..
               "%s" ..
               "\n" ..
               "        ///body\n" ..
               "%s" ..
               "    };\n" ..
               "\n" ..
               "    struct Output : public soap::CBaseWSDL\n" ..
               "    {\n" ..
               "        ///operator\n" ..
               "        Output( soap::CSoapEnvelope* apoEnv = NULL, apl_int_t aiOpt = DONT_ESCAPE ) : soap::CBaseWSDL(apoEnv, aiOpt) {}\n" ..
               "\n" ..
               "        const char* Encode(void);\n" ..
               "        apl_int_t Decode( const char* apsXMLString );\n" ..
               "        apl_int_t Decode( soap::CWSDLRequest& apoRequest );\n" ..
               "\n" ..
               "        ///header\n" ..
               "%s" ..
               "\n" ..
               "        ///body\n" ..
               "%s" ..
               "    };\n" ..
               "\n" ..
               "    static void CopyHeader( Output& aoOutput, Input& aoInupt );\n"..
               "};\n\n"..
               "%s",
               name,
               name,
               input_header_c,
               input_body_c,
               output_header_c,
               output_body_c,
               __H__PortType_Test(t)
    )
end

---------------------------------------------------------------------
-- complex-type cpp file coder generate function
-- and you can modified it if you want
---------------------------------------------------------------------
function __CPP__ComplexType( t )
    
    name = FormatName( "C", t["name"] )
    body = t["body"]
    encode = ""
    decode = ""
    entry_attr = ""
    element_array = ""

    ---- entry attr fro soap use encoded
    if SOAP_USE == "encoded" then
        entry_attr = entry_attr .. string.format( 
            "    acl::xml::McbAddAttribute( lpoCurrElem, apl_strdup( \"%s\:type\" ), apl_strdup( \"\\\"%s\:%s\\\"\" ), 1 );\n",
            XML_NAMESPACE["xsi"]["alias"],
            XML_NAMESPACE["targetNamespace"]["alias"],
            t["name"]
        )
    end
        
    for n, r in pairs(body) do
        var_name  = FormatName( "m" .. NameFerfix(r["type"]), r["name"] )
        
        if r["ptypet"] == "complex" then
            if r["maxOccurs"] < 0 and r["minOccurs"] < 0 then
                -------------------------------- encode ------------------------------------
                encode = encode .. string.format( 
                   "        // for member %s \n" ..
                   "        lpoSubElem = acl::xml::McbAddElement( lpoCurrElem, apoBase->ToString( \"\", \"%s\" ), 1, 20 );\n" ..
                   "        liRetCode = this->%s.Encode( apoBase, lpoSubElem, loCurrElement );\n"..
                   "        SOAP_GOTO_IF( ERROR, (liRetCode != soap::NO_ERROR) );\n\n",
                   var_name,
                   r["name"],
                   var_name                 
                )
                
                -------------------------------- decode ------------------------------------
                decode = decode .. string.format( 
                   "        // for membr %s \n"..
                   "        lpoSubElem = soap::McbFindElementNns(lpoCurrElem, \"%s\");\n"..
                   "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_ELEMENT_UNEXIST, lpoSubElem == NULL), (loCurrElement = \"%s\") );\n"..
                   "        liRetCode = this->%s.Decode( apoBase, lpoSubElem, loCurrElement);\n"..
                   "        SOAP_GOTO_IF( ERROR, (liRetCode != soap::NO_ERROR) );\n\n",
                   var_name,
                   r["name"],
                   r["name"],
                   var_name
                )
            else
                ------------------------------------ encode --------------------------------
                encode = encode .. string.format(
                   "        // for array member %s \n" ..
                   "        luSize = this->%s.size();\n",
                   var_name,
                   var_name
                )
                
                if r["minOccurs"] >= 0 then
                    encode = encode .. string.format(
                       "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_OUT_OF_RANGE, luSize < %d), (loCurrElement = \"%s\")  );\n",
                        r["minOccurs"],
                        r["name"]
                    )
                end
                
                if r["maxOccurs"] > 0 then
                    encode = encode .. string.format(
                        "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_OUT_OF_RANGE, luSize > %d), (loCurrElement = \"%s\") );\n",
                        r["maxOccurs"],
                        r["name"]
                    )
                end
                
                encode = encode .. string.format( 
                   "        for( apl_size_t luIdx = 0; luIdx < luSize; luIdx++ )\n" ..
                   "        {\n" ..
                   "            lpoSubElem = acl::xml::McbAddElement( lpoCurrElem, apoBase->ToString( \"\", \"%s\" ), 1, 20 );\n" ..
                   "            liRetCode = this->%s[luIdx].Encode( apoBase, lpoSubElem, loCurrElement );\n" ..
                   "            SOAP_GOTO_IF( ERROR, (liRetCode != soap::NO_ERROR) );\n"..
                   "        }\n",
                   r["name"],
                   var_name
                )
                
                ----------------------------------------- decode ------------------------------
                element_array = "    std::vector<acl::xml::McbXMLElement*> loXmlElements;\n"
                
                decode = decode .. string.format(
                   "        // for array member %s \n" ..
                   "        luSize = soap::McbFindElementsNns( lpoCurrElem, \"%s\", loXmlElements);\n",
                   var_name,
                   r["name"]
                )
                
                if r["minOccurs"] >= 0 then
                    decode = decode .. string.format(
                       "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_OUT_OF_RANGE, luSize < %d), (loCurrElement = \"%s\")  );\n",
                        r["minOccurs"],
                        r["name"]
                    )
                end
                
                if r["maxOccurs"] > 0 then
                    decode = decode .. string.format(
                        "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_OUT_OF_RANGE, luSize > %d), (loCurrElement = \"%s\") );\n",
                        r["maxOccurs"],
                        r["name"]
                    )
                end

                decode = decode .. string.format( 
                   "        this->%s.resize( luSize );\n" ..
                   "        for( apl_size_t luIdx = 0; luIdx < luSize; luIdx++ )\n" ..
                   "        {\n" ..
                   "            liRetCode = this->%s[luIdx].Decode( apoBase, loXmlElements[luIdx], loCurrElement);\n" ..
                   "            SOAP_GOTO_IF( ERROR, (liRetCode != soap::NO_ERROR) );\n"..
                   "        }\n",
                   var_name,
                   var_name
                )
             end
        else
            if r["maxOccurs"] < 0 and r["minOccurs"] < 0 then
                encode = encode .. string.format( 
                   "        // for member %s , use=%s\n" ..
                   "        lpoSubElem = acl::xml::McbAddElement( lpoCurrElem, apoBase->ToString( \"\", \"%s\" ), 1, 20 );\n" ..
                   "        acl::xml::McbAddText( lpoSubElem, apoBase->ToString( this->%s ), 1 );\n\n",
                   var_name,
                   SOAP_USE,
                   r["name"],
                   var_name
                )
                
                if SOAP_USE == "encoded" then
                    encode = encode .. string.format( 
                       "        acl::xml::McbAddAttribute( lpoSubElem, apl_strdup( \"%s\:type\" ), apl_strdup( \"\\\"%s\:%s\\\"\" ), 1 );\n\n",
                       XML_NAMESPACE["xsi"]["alias"],
                       XML_NAMESPACE["xsd"]["alias"],
                       ToXsdType( r["ptype"] )
                    )
                end
                
                decode = decode .. string.format( 
                   "        // for membr %s \n"..
                   "        lpoSubElem = soap::McbFindElementNns( lpoCurrElem, \"%s\" );\n"..
                   "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_ELEMENT_UNEXIST, lpoSubElem == NULL), (loCurrElement = \"%s\") );\n"..
                   "        ELEMENT_ASSIGN_TO_OBJ( apoBase, lpoSubElem, this->%s, \"%s\" );\n\n",
                   var_name,
                   r["name"],
                   r["name"],
                   var_name,
                   var_name
                )
            else
                ------------------------------------ encode --------------------------------
                encode = encode .. string.format(
                   "        // for array member %s \n" ..
                   "        luSize = this->%s.size();\n",
                   var_name,
                   var_name
                )
                
                if r["minOccurs"] >= 0 then
                    encode = encode .. string.format(
                       "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_OUT_OF_RANGE, luSize < %d), (loCurrElement = \"%s\")  );\n",
                        r["minOccurs"],
                        r["name"]
                    )
                end
                
                if r["maxOccurs"] > 0 then
                    encode = encode .. string.format(
                        "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_OUT_OF_RANGE, luSize > %d), (loCurrElement = \"%s\") );\n",
                        r["maxOccurs"],
                        r["name"]
                    )
                end
                
                if SOAP_USE == "encoded" then
                    encode = encode .. string.format(
                       "        for( apl_size_t luIdx = 0; luIdx < luSize; luIdx++ )\n" ..
                       "        {\n" ..
                       "            // for member %s \n" ..
                       "            lpoSubElem = acl::xml::McbAddElement( lpoCurrElem, apoBase->ToString( \"\", \"%s\" ), 1, 20 );\n" ..
                       "            acl::xml::McbAddText( lpoSubElem, apoBase->ToString( this->%s[luIdx] ), 1 );\n" ..
                       "            acl::xml::McbAddAttribute( lpoSubElem, apl_strdup( \"%s\:type\" ), apl_strdup( \"\\\"%s\:%s\\\"\" ), 1 );\n\n"..
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
                       "        for( apl_size_t luIdx = 0; luIdx < luSize; luIdx++ )\n" ..
                       "        {\n" ..
                       "            // for member %s \n" ..
                       "            lpoSubElem = acl::xml::McbAddElement( lpoCurrElem, apoBase->ToString( \"\", \"%s\" ), 1, 20 );\n" ..
                       "            acl::xml::McbAddText( lpoSubElem, apoBase->ToString( this->%s[luIdx] ), 1 );\n" ..
                       "        }\n",
                       var_name,
                       r["name"],
                       var_name
                    )
                end
                
                ------------------------------------ decode --------------------------------
                element_array = "    std::vector<acl::xml::McbXMLElement*> loXmlElements;\n"
                
                decode = decode .. string.format(
                   "        // for array member %s \n" ..
                   "        luSize = soap::McbFindElementsNns( lpoCurrElem, \"%s\", loXmlElements );\n",
                   var_name,
                   r["name"]
                )
                
                if r["minOccurs"] >= 0 then
                    decode = decode .. string.format(
                       "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_OUT_OF_RANGE, luSize < %d), (loCurrElement = \"%s\")  );\n",
                        r["minOccurs"],
                        r["name"]
                    )
                end
                
                if r["maxOccurs"] > 0 then
                    decode = decode .. string.format(
                        "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_OUT_OF_RANGE, luSize > %d), (loCurrElement = \"%s\") );\n",
                        r["maxOccurs"],
                        r["name"]
                    )
                end

                decode = decode .. string.format(
                   "        this->%s.resize( luSize );\n" ..
                   "        for( apl_size_t luIdx = 0; luIdx < luSize; luIdx++ )\n" ..
                   "        {\n" ..
                   "            ELEMENT_ASSIGN_TO_OBJ( apoBase, loXmlElements[luIdx], this->%s[luIdx], \"%s\" );\n"..
                   "        }\n",
                   var_name,
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
           "apl_int_t %s::Encode( soap::CBaseWSDL* apoBase, acl::xml::McbXMLElement* apoRoot, std::string& aoCurrElement )\n" ..
           "{\n" ..
           "    /* start to create XML string */ \n" ..
           "    apl_size_t     luSize;\n" ..
           "    apl_int_t      liRetCode;\n" ..
           "    std::string&   loCurrElement = aoCurrElement;\n" ..
           "    acl::xml::McbXMLElement* lpoSubElem;\n" ..
           "    acl::xml::McbXMLElement* lpoCurrElem;\n" ..
           "\n" ..
           "    luSize         = 0;\n" ..
           "    liRetCode      = soap::NO_ERROR;\n" ..
           "    loCurrElement  = \"\";\n" ..
           "    lpoSubElem     = NULL;\n" ..
           "    lpoCurrElem    = apoRoot;\n" ..
           "%s\n"..
           "    do\n" ..
           "    {\n" ..
           "%s" ..
           "        return soap::NO_ERROR;\n" ..
           "    }\n" ..
           "    while( false );\n" ..
           "    goto ERROR;\n" ..
           "\n" ..
           "ERROR:\n" ..
           "    return liRetCode;\n" ..
           "}\n\n" ..
           "apl_int_t %s::Decode( soap::CBaseWSDL* apoBase, acl::xml::McbXMLElement* apoRoot, std::string& aoCurrElement )\n"..
           "{\n"..
           "    // Declare varible \n"..
           "    apl_size_t     luSize;\n"..
           "    apl_size_t     luInd;\n"..
           "    apl_int_t      liRetCode;\n"..
           "    std::string&   loCurrElement = aoCurrElement;\n"..
           "    acl::xml::McbXMLElement* lpoSubElem;\n".. element_array ..
           "    acl::xml::McbXMLElement* lpoCurrElem;\n"..
           "\n"..
           "    // Initailize varible \n"..
           "    luSize         = 0;\n"..
           "    luInd          = 0;\n"..
           "    liRetCode      = soap::NO_ERROR;\n"..
           "    loCurrElement  = \"\";\n"..
           "    lpoSubElem     = NULL;\n"..
           "    lpoCurrElem    = apoRoot;\n"..
           "\n"..
           "    // Start encode \n"..
           "    do\n"..
           "    {\n"..
           "%s"..
           "\n"..
           "        return soap::NO_ERROR;\n"..
           "    }\n"..
           "    while(false);\n"..
           "    goto ERROR;\n"..
           "\n"..
           "ERROR:\n"..
           "    return liRetCode;\n"..
           "}\n\n"..
           "%s",
           name,
           name,
           entry_attr,
           encode,
           name,
           decode,
           __CPP__ComplexType_Test(t)
    )
end

---------------------------------------------------------------------
-- port-type cpp file coder generate function
-- and you can modified it if you want
---------------------------------------------------------------------
function __CPP__PortType( t )
    
    name = FormatName( "C", t["name"] )
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
            "        lpoCurrElem = acl::xml::McbAddElement( lpoCurrElem, ToString(\"%s\",\"%s\"), 1, 20 );\n",
            XML_NAMESPACE["targetNamespace"]["alias"],
            t["name"]
        )
        input_entry_opt_d = input_entry_opt_d .. string.format( 
            "        lpoCurrElem = soap::McbFindElementNns( lpoCurrElem, \"%s\" );\n"..
            "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_ELEMENT_UNEXIST, lpoCurrElem == NULL), (loCurrElement = \"%s\") );\n",
            t["name"],
            t["name"]
        )
        
        output_entry_opt_e = output_entry_opt_e .. string.format( 
            "        lpoCurrElem = acl::xml::McbAddElement( lpoCurrElem, ToString(\"%s\",\"%s\"), 1, 20 );\n",
            XML_NAMESPACE["targetNamespace"]["alias"],
            response_name
        )
        output_entry_opt_d = output_entry_opt_d .. string.format( 
            "        lpoCurrElem = soap::McbFindElementNns( lpoCurrElem, \"%s\" );\n"..
            "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_ELEMENT_UNEXIST, lpoCurrElem == NULL), (loCurrElement = \"%s\") );\n",
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
            
            var_name  = FormatName( "m" .. NameFerfix(r["ptype"]), r["name"] )
            
            if r["ptypet"] == "complex" then
                encode = encode .. string.format( 
                   "        // for member %s \n" ..
                   "        lpoSubElem = acl::xml::McbAddElement( lpoCurrElem, ToString( \"%s\", \"%s\" ), 1, 20 );\n" ..
                   "        liRetCode = this->%s.Encode( this, lpoSubElem, loCurrElement );\n"..
                   "        SOAP_GOTO_IF( ERROR, (liRetCode != soap::NO_ERROR) );\n\n",
                   var_name,
                   name_space,
                   element_name,
                   var_name                   
                )
                decode = decode .. string.format( 
                   "        // for membr %s \n"..
                   "        lpoSubElem = soap::McbFindElementNns( lpoCurrElem, \"%s\" );\n"..
                   "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_ELEMENT_UNEXIST, lpoSubElem == NULL), (loCurrElement = \"%s\") );\n"..
                   "        liRetCode = this->%s.Decode(this, lpoSubElem, loCurrElement);\n"..
                   "        SOAP_GOTO_IF( ERROR, (liRetCode != soap::NO_ERROR) );\n",
                   var_name,
                   element_name,
                   element_name,
                   var_name
                   
                )
            else
                encode = encode .. string.format(
                   "        // for member %s \n" ..
                   "        lpoSubElem = acl::xml::McbAddElement( lpoCurrElem, ToString( \"%s\", \"%s\" ), 1, 20 );\n" ..
                   "        acl::xml::McbAddText( lpoSubElem, ToString( this->%s ), 1 );\n\n",
                   var_name,
                   name_space,
                   element_name,
                   var_name
                )
                
                if SOAP_USE == "encoded" then
                    encode = encode .. string.format( 
                       "        acl::xml::McbAddAttribute( lpoSubElem, strdup( \"%s\:type\" ), strdup( \"\\\"%s\:%s\\\"\" ), 1 );\n\n",
                       XML_NAMESPACE["xsi"]["alias"],
                       XML_NAMESPACE["xsd"]["alias"],
                       ToXsdType( r["ptype"] )
                    )
                end
                
                decode = decode .. string.format( 
                   "        // for membr %s \n"..
                   "        lpoSubElem = soap::McbFindElementNns(lpoCurrElem, \"%s\");\n"..
                   "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_ELEMENT_UNEXIST, lpoSubElem == NULL), (loCurrElement = \"%s\") );\n"..
                   "        ELEMENT_ASSIGN_TO_OBJ(this, lpoSubElem, this->%s, \"%s\");\n\n",
                   var_name,
                   element_name,
                   element_name,
                   var_name,
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
        var_name  = FormatName( "m" .. NameFerfix(r["ptype"]), r["name"] )
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
               "const char* %s::Input::Encode(void)\n"..
               "{\n"..
               "    // Declare varible \n"..
               "    apl_size_t     luSize;\n"..
               "    apl_int_t      liRetCode;\n"..
               "    acl::xml::McbXMLElement* lpoSubElem;\n"..
               "    acl::xml::McbXMLElement* lpoCurrElem; \n"..
               "    std::string    loCurrElement;\n"..
               "    acl::xml::McbXMLElement* lpoEnvelopeElem; \n"..
               "    acl::xml::McbXMLElement* lpoRootElem; \n"..
               "\n"..
               "    // Initailize varible \n"..
               "    luSize          = 0;\n"..
               "    liRetCode       = 0;\n"..
               "    lpoSubElem      = NULL;\n"..
               "    lpoRootElem     = acl::xml::McbCreateRoot(); \n"..
               "    lpoCurrElem     = lpoRootElem;\n"..
               "    lpoEnvelopeElem = lpoRootElem;\n"..
               "\n"..
               "    // Start encode \n"..
               "    do\n"..
               "    {\n"..
               "        // Envelope \n"..
               "        SOAP_GOTO_IF( BODY, ( this->GetSoapEnvelope() == NULL ) );\n"..
               "\n"..
               "        this->GetSoapEnvelope()->SetEnvelopeNamespace(\"%s\");\n"..
               "\n"..
               "%s"..
               "\n"..
               "        this->EncodeXmlHeader( lpoRootElem );\n"..
               "        lpoEnvelopeElem = GetSoapEnvelope()->EncodeSoapEnvelope( lpoRootElem );\n"..
               "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_SOAP_ENV, lpoEnvelopeElem == NULL), (loCurrElement = \"SoapEnvelope\") );\n"..
               "\n"..
               "        // Start header \n"..
               "        lpoCurrElem = GetSoapEnvelope()->EncodeSoapHeader( lpoEnvelopeElem );\n"..
               "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_SOAP_HEADER, lpoCurrElem == NULL), (loCurrElement = \"SoapHeader\") );\n"..
               "\n"..
               "%s"..
               "\n"..
               "        // Start body \n"..
               "        lpoCurrElem = GetSoapEnvelope()->EncodeSoapBody( lpoEnvelopeElem );\n"..
               "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_SOAP_BODY, lpoCurrElem == NULL), (loCurrElement = \"SoapBody\")  );\n"..
               "BODY:\n"..
               "%s"..
               "%s"..
               "\n"..
               "        liRetCode = this->CreateXMLString( lpoRootElem );\n"..
               "        SOAP_GOTO_IF( ERROR, ( liRetCode != soap::NO_ERROR ) );\n"..
               "        acl::xml::McbDeleteRoot( lpoRootElem );\n"..
               "        return this->ResultPointer();\n"..
               "    }\n"..
               "    while( false );\n"..
               "    goto ERROR;\n"..
               "\n"..
               "ERROR:\n"..
               "    this->SetCurrElement( loCurrElement.c_str() );\n"..
               "    acl::xml::McbDeleteRoot( lpoRootElem );\n"..
               "    return NULL;\n"..
               "}\n\n"..
               "apl_int_t %s::Input::Decode( const char* apsXMLString )\n"..
               "{\n"..
               "    apl_int_t liRetCode = soap::NO_ERROR;\n"..
               "    soap::CWSDLRequest loRequest( this->GetSoapEnvelope() );\n"..
               "    if ( ( liRetCode = loRequest(apsXMLString) ) != soap::NO_ERROR )\n"..
               "    {\n"..
               "        return liRetCode;\n"..
               "    }\n"..
               "    return this->Decode( loRequest );\n"..
               "}\n"..
               "\n"..
               "apl_int_t %s::Input::Decode( soap::CWSDLRequest& aoRequest )\n"..
               "{\n"..
               "    // Declare varible \n"..
               "    apl_size_t     luInd  = 0;\n"..
               "    apl_size_t     luSize = 0;\n"..
               "    apl_int_t      liRetCode;\n"..
               "    std::string    loCurrElement;\n"..
               "    acl::xml::McbXMLElement* lpoSubElem = NULL;\n"..
               "    acl::xml::McbXMLElement* lpoCurrElem = NULL;\n"..
               "    acl::xml::McbXMLElement* lpoEnvelopeElem = NULL;\n"..
               "    acl::xml::McbXMLElement* lpoRootElem;\n"..
               "    \n"..
               "    // Initailize varible \n"..
               "    luInd           = 0;\n"..
               "    luSize          = 0;\n"..
               "    liRetCode       = 0;\n"..
               "    lpoSubElem      = NULL;\n"..
               "    lpoRootElem     = aoRequest.GetRootElement();\n"..
               "    lpoCurrElem     = lpoRootElem;\n"..
               "    lpoEnvelopeElem = lpoRootElem;\n"..
               "    SOAP_RETURN_IF ( soap::ERROR_PARSE_XML, (lpoRootElem == NULL) ); \n"..
               "\n"..
               "    // Start encode \n"..
               "    do\n"..
               "    {\n"..
               "        // Check Envelope \n"..
               "        SOAP_GOTO_IF( BODY, ( aoRequest.GetSoapEnvelope() == NULL ) );\n"..
               "\n"..
               "        this->DecodeXmlHeader( lpoRootElem );\n"..
               "        lpoEnvelopeElem = aoRequest.GetEnvelope();\n"..
               "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_SOAP_ENV, lpoEnvelopeElem == NULL), (loCurrElement = \"SoapEnvelope\") );\n"..
               "        // Start header \n"..
               "        lpoCurrElem = aoRequest.GetHeader();\n"..
               "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_SOAP_HEADER, lpoCurrElem == NULL), (loCurrElement = \"SoapHeader\") );\n"..
               "\n"..
               "%s"..
               "\n"..
               "        // Start body \n"..
               "        lpoCurrElem = aoRequest.GetBody();\n"..
               "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_SOAP_BODY, lpoCurrElem == NULL), (loCurrElement = \"SoapBody\") );\n"..
               "BODY:\n"..
               "%s"..
               "%s"..
               "\n"..
               "        return soap::NO_ERROR;\n"..
               "    }\n"..
               "    while( false );\n"..
               "    goto ERROR;\n"..
               "\n"..
               "ERROR:\n"..
               "    this->SetCurrElement(loCurrElement.c_str());\n"..
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
               "const char* %s::Output::Encode()\n"..
               "{\n"..
               "    // Declare varible \n"..
               "    apl_size_t     luSize;\n"..
               "    apl_int_t      liRetCode;\n"..
               "    acl::xml::McbXMLElement* lpoSubElem;\n"..
               "    acl::xml::McbXMLElement* lpoCurrElem; \n"..
               "    std::string    loCurrElement;\n"..
               "    acl::xml::McbXMLElement* lpoEnvelopeElem; \n"..
               "    acl::xml::McbXMLElement* lpoRootElem; \n"..
               "\n"..
               "    // Initailize varible \n"..
               "    luSize          = 0;\n"..
               "    liRetCode       = 0;\n"..
               "    lpoSubElem      = NULL;\n"..
               "    lpoRootElem     = acl::xml::McbCreateRoot(); \n"..
               "    lpoCurrElem     = lpoRootElem;\n"..
               "    lpoEnvelopeElem = lpoRootElem;\n"..
               "\n"..
               "    // Start encode \n"..
               "    do\n"..
               "    {\n"..
               "        // Envelope \n"..
               "        SOAP_GOTO_IF( BODY, ( this->GetSoapEnvelope() == NULL ) );\n"..
               "\n"..
               "        this->GetSoapEnvelope()->SetEnvelopeNamespace(\"%s\");\n"..
               "\n"..
               "%s"..
               "\n"..
               "        this->EncodeXmlHeader( lpoRootElem );\n"..
               "        lpoEnvelopeElem = GetSoapEnvelope()->EncodeSoapEnvelope( lpoRootElem );\n"..
               "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_SOAP_ENV, lpoEnvelopeElem == NULL), (loCurrElement = \"SoapEnvelope\") );\n"..
               "\n"..
               "        // Start header \n"..
               "        lpoCurrElem = GetSoapEnvelope()->EncodeSoapHeader( lpoEnvelopeElem );\n"..
               "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_SOAP_HEADER, lpoCurrElem == NULL), (loCurrElement = \"SoapHeader\") );\n"..
               "\n"..
               "%s"..
               "\n"..
               "        // Start body \n"..
               "        lpoCurrElem = GetSoapEnvelope()->EncodeSoapBody( lpoEnvelopeElem );\n"..
               "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_SOAP_BODY, lpoCurrElem == NULL), (loCurrElement = \"SoapBody\")  );\n"..
               "BODY:\n"..
               "%s"..
               "%s"..
               "\n"..
               "        liRetCode = this->CreateXMLString( lpoRootElem );\n"..
               "        SOAP_GOTO_IF( ERROR, ( liRetCode != soap::NO_ERROR ) );\n"..
               "        acl::xml::McbDeleteRoot( lpoRootElem );\n"..
               "        return this->ResultPointer();\n"..
               "    }\n"..
               "    while( false );\n"..
               "    goto ERROR;\n"..
               "\n"..
               "ERROR:\n"..
               "    this->SetCurrElement( loCurrElement.c_str() );\n"..
               "    acl::xml::McbDeleteRoot( lpoRootElem );\n"..
               "    return NULL;\n"..
               "}\n\n"..
               "apl_int_t %s::Output::Decode( const char* apsXMLString )\n"..
               "{\n"..
               "    apl_int_t liRetCode = soap::NO_ERROR;\n"..
               "    soap::CWSDLRequest loRequest( this->GetSoapEnvelope() );\n"..
               "    if ( ( liRetCode = loRequest(apsXMLString) ) != soap::NO_ERROR )\n"..
               "    {\n"..
               "        return liRetCode;\n"..
               "    }\n"..
               "    return this->Decode( loRequest );\n"..
               "}\n\n"..
               "apl_int_t %s::Output::Decode( soap::CWSDLRequest& apoRequest )\n"..
               "{\n"..
               "    // Declare varible \n"..
               "    apl_size_t     luInd  = 0;\n"..
               "    apl_size_t     luSize = 0;\n"..
               "    apl_int_t      liRetCode;\n"..
               "    std::string    loCurrElement;\n"..
               "    acl::xml::McbXMLElement* lpoSubElem = NULL;\n"..
               "    acl::xml::McbXMLElement* lpoCurrElem = NULL;\n"..
               "    acl::xml::McbXMLElement* lpoEnvelopeElem = NULL;\n"..
               "    acl::xml::McbXMLElement* lpoRootElem;\n"..
               "    \n"..
               "    // Initailize varible \n"..
               "    luInd           = 0;\n"..
               "    luSize          = 0;\n"..
               "    liRetCode       = 0;\n"..
               "    lpoSubElem      = NULL;\n"..
               "    lpoRootElem     = apoRequest.GetRootElement();\n"..
               "    lpoCurrElem     = lpoRootElem;\n"..
               "    lpoEnvelopeElem = lpoRootElem;\n"..
               "    SOAP_RETURN_IF ( soap::ERROR_PARSE_XML, (lpoRootElem == NULL) ); \n"..
               "\n"..
               "    // Start encode \n"..
               "    do\n"..
               "    {\n"..
               "        // Check Envelope \n"..
               "        SOAP_GOTO_IF( BODY, ( this->GetSoapEnvelope() == NULL ) );\n"..
               "\n"..
               "        this->DecodeXmlHeader( lpoRootElem );\n"..
               "        lpoEnvelopeElem = apoRequest.GetEnvelope();\n"..
               "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_SOAP_ENV, lpoEnvelopeElem == NULL), (loCurrElement = \"SoapEnvelope\") );\n"..
               "        // Start header \n"..
               "        lpoCurrElem = apoRequest.GetHeader();\n"..
               "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_SOAP_HEADER, lpoCurrElem == NULL), (loCurrElement = \"SoapHeader\") );\n"..
               "\n"..
               "%s"..
               "\n"..
               "        // Start body \n"..
               "        lpoCurrElem = apoRequest.GetBody();\n"..
               "        SOAP_GOTO_IF_EX( ERROR, (liRetCode = soap::ERROR_SOAP_BODY, lpoCurrElem == NULL), (loCurrElement = \"SoapBody\") );\n"..
               "BODY:\n"..
               "%s"..
               "%s"..
               "\n"..
               "        return soap::NO_ERROR;\n"..
               "    }\n"..
               "    while( false );\n"..
               "    goto ERROR;\n"..
               "\n"..
               "ERROR:\n"..
               "    this->SetCurrElement(loCurrElement.c_str());\n"..
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
    
    return intput .. output .. header_copy .. __CPP__PortType_Test(t)
end

---------------------------------------------------------------------
-- common function, DON'T modified it, this is not udf
---------------------------------------------------------------------
function NameFerfix( name )
    if name == "std::string" then
        return "o"
    elseif name == "soap::CInt" then
        return "o"
    elseif name == "soap::CLong" then
        return "o"
    elseif name == "bool" then
        return "b"
    else
        return "o"
    end
end

function TypeFerfix( typet )
    if typet == "complex" then
        return "C"
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
    if perfix == "C" then
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

--------------------------------------------------------------------
--                           TEXT
---------------------------------------------------------------------
-- complex-type h file test coder generate function
-- and you can modified it if you want
---------------------------------------------------------------------
function __H__ComplexType_Test( t )
    if GENERATE_TEST_CODER == 0 then
        return ""
    end
    
    name = FormatName( "C", t["name"] )

    return "" ..
           "/**\n" ..
           " * complex-type : " .. name .. "\n" ..
           " */\n" ..
           "void MakeTest( " .. name .. "& aoNode );\n" ..
           "bool CheckTest( " .. name .. "& aoNode );\n\n"
end

---------------------------------------------------------------------
-- port-type h file test coder generate function
-- and you can modified it if you want
---------------------------------------------------------------------
function __H__PortType_Test( t )
    if GENERATE_TEST_CODER == 0 then
        return ""
    end
    
    name = FormatName( "C", t["name"] )

    return string.format( 
               "" ..
               "/**\n" ..
               " * port-type :%s\n" ..
               " */\n" ..
               "void RunTest_%s_Input(void);\n" ..
               "void RunTest_%s_Output(void);\n\n",
               name,
               name,
               name
    )
end

---------------------------------------------------------------------
-- complex-type cpp file test coder generate function
-- and you can modified it if you want
---------------------------------------------------------------------
function __CPP__ComplexType_Test( t )
    if GENERATE_TEST_CODER == 0 then
        return ""
    end
    
    name = FormatName( "C", t["name"] )
    body = t["body"]
    make_member = ""
    check_member = ""
    
    for n, r in pairs(body) do
        var_name  = FormatName( "m" .. NameFerfix(r["type"]), r["name"] )
        
        if r["ptypet"] == "complex" then
            if r["maxOccurs"] < 0 and r["minOccurs"] < 0 then
                make_member = make_member .. string.format("    MakeTest(aoNode.%s);\n", var_name)
                check_member = check_member .. string.format("    SOAP_RETURN_IF(false, !CheckTest(aoNode.%s) );\n", var_name)
            else
                make_member = make_member.. string.format( 
                   "    aoNode.%s.resize(1);\n" ..
                   "    MakeTest(aoNode.%s[0]);\n",
                   var_name,
                   var_name
                )
                check_member = check_member.. string.format( 
                   "    aoNode.%s.resize(1);\n" ..
                   "    SOAP_RETURN_IF(false, !CheckTest(aoNode.%s[0]) );\n",
                   var_name,
                   var_name
                )
            end

        else
            if r["maxOccurs"] < 0 and r["minOccurs"] < 0 then
                make_member = make_member.. string.format("    Assign(aoNode.%s, %d);\n", var_name, n)
                check_member = check_member.. string.format("    SOAP_RETURN_IF(false, !Check(aoNode.%s, %d) );\n", var_name, n)
            else
                make_member = make_member.. string.format( 
                   "    aoNode.%s.resize(1);\n" ..
                   "    Assign(aoNode.%s[0], %d);\n",
                   var_name,
                   var_name,
                   n
                )
                check_member = check_member.. string.format( 
                   "    aoNode.%s.resize(1);\n" ..
                   "    SOAP_RETURN_IF(false, !Check(aoNode.%s[0], %d) );\n",
                   var_name,
                   var_name,
                   n
                )
            end
        end
    end

    return string.format( 
           "/**\n" ..
           " * complex-type : %s\n" ..
           " */\n" ..
           "void MakeTest( %s& aoNode )\n" ..
           "{\n" ..
           "%s" ..
           "}\n\n"..
           "bool CheckTest( %s& aoNode )\n" ..
           "{\n" ..
           "%s" ..
           "    return true;\n"..
           "}\n\n",
           name,
           name,
           make_member,
           name,
           check_member
    )
end

---------------------------------------------------------------------
-- port-type cpp file test coder generate function
-- and you can modified it if you want
---------------------------------------------------------------------
function __CPP__PortType_Test( t )
    if GENERATE_TEST_CODER == 0 then
        return ""
    end
    
    name = FormatName( "C", t["name"] )
    input_header_t = t["input"]["header"]
    input_body_t = t["input"]["body"]
    output_header_t = t["output"]["header"]
    output_body_t = t["output"]["body"]
    
    input_header_m = ""
    input_body_m = ""
    output_header_m = ""
    output_body_m = ""
    input_header_c = ""
    input_body_c = ""
    output_header_c = ""
    output_body_c = ""
    
    make_coder = function( l )
        make_member = ""
        check_member = ""

        for n, r in pairs(l) do
            var_name  = FormatName( "m" .. NameFerfix(r["ptype"]), r["name"] )
            
            if r["ptypet"] == "complex" then
                make_member = make_member .. string.format( 
                   "    MakeTest(loObj.%s);\n",
                   var_name
                )
                check_member = check_member .. string.format( 
                   "    SOAP_TEST_RETURN_IF(!CheckTest(loObj.%s) );\n",
                   var_name
                )
            else
                make_member = make_member.. string.format(
                   "    Assign(loObj.%s, %d);\n",
                   var_name,
                   n
                )
                check_member = check_member.. string.format(
                   "    SOAP_TEST_RETURN_IF(!Check(loObj.%s, %d) );\n",
                   var_name,
                   n
                )
            end
        end
        
        return make_member, check_member
    end

    input_header_m, input_header_c   = make_coder(input_header_t)
    input_body_m, input_body_c       = make_coder(input_body_t)
    output_header_m, output_header_c = make_coder(output_header_t)
    output_body_m, output_body_c     = make_coder(output_body_t)
    
    --add test list
    AddTestFunction( string.format("    RunTest_%s_Input();", name) )
    AddTestFunction( string.format("    RunTest_%s_Output();", name) )

    intput = string.format( 
               "void RunTest_%s_Input(void)\n"..
               "{\n"..
               "    %s::Input loObj;\n\n"..
               "%s\n"..
               "%s\n"..
               "    const char* lpoXml = loObj.Encode();\n" ..
               "    if (lpoXml == NULL)\n" ..
               "    {\n" ..
               "        printf(\"RunTest_%s_Input:Encode [FAIL]\\n\");\n" ..
               "        return;\n" ..
               "    }\n" ..
               "\n"..
               "    if (loObj.Decode(lpoXml) != 0)\n"..
               "    {\n"..
               "        printf(\"RunTest_%s_Input:Decode [FAIL]\\n%%s\\n%%s\\n\", loObj.GetCurrElement(), lpoXml);\n" ..
               "        return;\n" ..
               "    }\n"..
               "\n"..
               "%s\n"..
               "%s\n"..
               "    printf(\"RunTest_%s_Input [OK]\\n\");\n" ..
               "}\n\n",
               name,
               name,
               input_header_m,
               input_body_m,
               name,
               name,
               input_header_c,
               input_body_c,
               name
    )
    
    output = string.format( 
               "void RunTest_%s_Output(void)\n"..
               "{\n"..
               "    %s::Output loObj;\n\n"..
               "%s\n"..
               "%s\n"..
               "    const char* lpoXml = loObj.Encode();\n" ..
               "    if (lpoXml == NULL)\n" ..
               "    {\n" ..
               "        printf(\"RunTest_%s_Output:Encode [FAIL]\\n\");\n" ..
               "        return;\n" ..
               "    }\n" ..
               "\n"..
               "    if (loObj.Decode(lpoXml) != 0)\n"..
               "    {\n"..
               "        printf(\"RunTest_%s_Output:Decode [FAIL]\\n%%s\\n%%s\\n\", loObj.GetCurrElement(), lpoXml);\n" ..
               "        return;\n" ..
               "    }\n"..
               "\n"..
               "%s\n"..
               "%s\n"..
               "    printf(\"RunTest_%s_Output [OK]\\n\");\n" ..
               "}\n\n",
               name,
               name,
               output_header_m,
               output_body_m,
               name,
               name,
               output_header_c,
               output_body_c,
               name
    )
    
    return intput .. output
end

-----------------------------------------------------------------------------------
-- test utility functions
-----------------------------------------------------------------------------------
function AddTestFunction(f)
    if GENERATE_TEST_CODER == 1 then
        if test_function_list == nil then
            test_function_list = f
        else
            test_function_list = test_function_list .. "\n" .. f
        end
    end
end

function GetTestFunctions(f)
    if GENERATE_TEST_CODER == 1 then
        if test_function_list == nil then
            return ""
        else
            return test_function_list
        end
    end
end

function GetRunTextDeclare()
    if GENERATE_TEST_CODER == 1 then
        return string.format("//Run all unit test case\nvoid RunAllTest(void);\n\n")
    else
        return ""
    end
end

function GetRunTextDefine()
    if GENERATE_TEST_CODER == 1 then
        return string.format(
            "//Run all unit test case\n" ..
            "void RunAllTest(void)\n" ..
            "{\n" ..
            "%s\n" ..
            "}\n",
            GetTestFunctions() )
    else
        return ""
    end
end

function GetTestUtilFunctions()
    if GENERATE_TEST_CODER == 1 then
        return [[
///////////////////////////////////////////////////////////////////////////////
#define SOAP_TEST_RETURN_IF(exp) if (exp) return;

void Assign(char* apcR, apl_int_t aiL)
{
    apl_snprintf(apcR, 25, "char:%"APL_PRIdINT, aiL);
}

void Assign(std::string& aoR, apl_int_t aiL)
{
    char lacBuffer[30];
    apl_snprintf(lacBuffer, 30, "string:%"APL_PRIdINT, aiL);
    aoR = lacBuffer;
}

void Assign(soap::CInt& aoR, apl_int_t aiL)
{
    aoR = aiL;
}

void Assign(soap::CLong& aoR, apl_int_t aiL)
{
    aoR = aiL;
}

void Assign(bool& aoR, apl_int_t aiL)
{
    aoR = true;
}

void Assign(acl::CMemoryBlock& aoR, apl_int_t aiL)
{
    aoR.Resize(50);
    apl_snprintf(aoR.GetWritePtr(), 50, "memoryblock:%"APL_PRIdINT, aiL);
    aoR.SetWritePtr(apl_strlen(aoR.GetWritePtr() ) );
}

/////////////////////////////////////////////////////////////////////////////
bool Check(char* apcR, apl_int_t aiL)
{
    char lacBuffer[30];
    apl_snprintf(lacBuffer, 30, "char:%"APL_PRIdINT, aiL);
    
    return apl_strcmp(apcR, lacBuffer) == 0;
}

bool Check(std::string& aoR, apl_int_t aiL)
{
    char lacBuffer[30];
    apl_snprintf(lacBuffer, 30, "string:%"APL_PRIdINT, aiL);
    
    return apl_strcmp(aoR.c_str(), lacBuffer) == 0;
}

bool Check(soap::CInt& aoR, apl_int_t aiL)
{
    return aoR == aiL;
}

bool Check(soap::CLong& aoR, apl_int_t aiL)
{
    return aoR == aiL;
}

bool Check(bool& aoR, apl_int_t aiL)
{
    return aoR == true;
}

bool Check(acl::CMemoryBlock& aoR, apl_int_t aiL)
{
    char lacBuffer[50];
    apl_snprintf(lacBuffer, 50, "memoryblock:%"APL_PRIdINT, aiL);
    
    return apl_strcmp(aoR.GetReadPtr(), lacBuffer) == 0;
}
]]
    else
        return ""
    end
end
