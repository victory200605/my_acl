///////////////////////////////////////////////////////////
//  ResultSet.h
//  Public class of the result set (for all DB)
//  Created on:      19-10-2009 14:28:00
//  Original author: hezk
///////////////////////////////////////////////////////////

#if !defined(ADL_RESULT_SET_H)
#define ADL_RESULT_SET_H

#include "apl/apl.h"
#include <string>
#include <vector>
#include "adl/Adl.h"


ADL_NAMESPACE_START


struct CResult
{
    void* mpvResult;

    apl_size_t muLen;
};


struct CField
{
    std::string muFieldName; //field name

    //the field width in bytes
    apl_size_t muFieldWidth; 

    //to be done, for later support, currently only string
    apl_size_t muDataType; 

    //temporary space for storing the result of this field
    char* mpcContent; 
};


typedef std::vector<CResult> VROW_T;

typedef std::vector< VROW_T > VRESULTS_T;

typedef std::vector<CField> VFIELDS_T;


///////////////////////////////////////////////////////////////////////////////

class CResultSet
{

public:

    CResultSet();

    ~CResultSet();

	const char* GetField(apl_size_t auField, apl_int_t* apiLen);

	const char* GetField(const char* apcFieldName, apl_int_t* apiLen);

	const char* GetField(apl_int_t auRow, 
            apl_size_t auField, apl_int_t* apiLen);

	const char* GetField(apl_int_t auRow, 
            const char* apcFieldName, apl_int_t* apiLen);

	apl_int_t GetFieldIndex(const char* apcFieldName);

    //get how many fileds 
	apl_size_t GetFieldNameCount(); 

    //get field name specified by index
	const char* GetFieldName(apl_size_t auIdx); 

    //get field width specified by index
    apl_size_t GetFieldNameWidth(apl_size_t auIdx); 

    //get field width specified by index
    apl_int_t GetFieldNameType(apl_size_t auIdx); 

	apl_int_t GetRowCount();

	apl_int_t MoveToRow(apl_size_t auIndex);

	apl_int_t MoveToNextRow();

	apl_int_t MoveToPrevRow();

    void AddResult(void* apvPtr, apl_size_t auLen, apl_size_t auRow);

    void ReserveResultCount(apl_size_t auCount);

    void ClearResults(apl_int_t aiMode=0);

    apl_int_t ValidateCurRow(apl_int_t auRow, apl_size_t auField);

    apl_int_t AddFieldName(
            const char* apcField, 
            apl_size_t auFieldWidth = 0, 
            apl_size_t auDataType = 0, 
            char* apcContent = APL_NULL);

    void ClearFieldNames();

    apl_size_t GetCurrentRow();

    void SetCurrentRow(apl_int_t auRow);

private:

    VFIELDS_T moFieldSet;

    VRESULTS_T moResultSet;

    apl_int_t miCurrentRow;
};


ADL_NAMESPACE_END


#endif //!define(ADL_RESULT_SET_H)
