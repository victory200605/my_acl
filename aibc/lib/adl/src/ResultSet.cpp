///////////////////////////////////////////////////////////
//  ResultSet.cpp
//  Public class of the result set (for all DB)
//  Created on:      19-10-2009 14:28:00
//  Original author: hezk
///////////////////////////////////////////////////////////

#include "ResultSet.h"


ADL_NAMESPACE_START

CResultSet::CResultSet()
{
}

CResultSet::~CResultSet()
{
}

const char* CResultSet::GetField(apl_size_t auField, apl_int_t* apiLen)
{
    return this->GetField(this->miCurrentRow, auField, apiLen);
}

const char* CResultSet::GetField(const char* apcFieldName, apl_int_t* apiLen)
{
    return this->GetField(this->miCurrentRow, apcFieldName, apiLen);
}

const char* CResultSet::GetField(
        apl_int_t aiRow, 
        apl_size_t auField, 
        apl_int_t* apiLen)
{
    if (apiLen != APL_NULL)
    {
        *apiLen = 0;
    }

    if (this->ValidateCurRow(aiRow, auField))
    {
        return APL_NULL;
    }

    if (apiLen != APL_NULL)
    {
        *apiLen = (apl_int_t)this->moResultSet[aiRow][auField].muLen;
    }

    return (const char*)this->moResultSet[aiRow][auField].mpvResult;
}

const char* CResultSet::GetField(apl_int_t aiRow, 
        const char* apcFieldName, 
        apl_int_t* apiLen)
{
    apl_int_t liIdx = -1;

    liIdx = this->GetFieldIndex(apcFieldName);

    //can't find this field
    if (liIdx < 0)
    {
        return APL_NULL;
    }

    return this->GetField(aiRow, liIdx, apiLen);
}

apl_int_t CResultSet::GetFieldIndex(const char* apcFieldName)
{
    if (apcFieldName == APL_NULL
            || this->moFieldSet.size() == 0)
    {
        return -1;
    }

    apl_int_t liIdx;

    for (liIdx = 0; (apl_size_t)liIdx < this->moFieldSet.size(); ++liIdx)
    {
        //found
        if (!apl_strcasecmp(this->moFieldSet[liIdx].muFieldName.c_str(), 
                    apcFieldName))
        {
            break;
        }
    }

    if ((apl_size_t)liIdx >= this->moFieldSet.size())
    {
        liIdx = -1;
    }

    return liIdx;
}

apl_size_t CResultSet::GetFieldNameCount()
{
    return this->moFieldSet.size();
}

const char* CResultSet::GetFieldName(apl_size_t auIdx)
{
    if (auIdx + 1 > this->moFieldSet.size())
    {
        return  APL_NULL;
    }

    return this->moFieldSet[auIdx].muFieldName.c_str();
}

apl_size_t CResultSet::GetFieldNameWidth(apl_size_t auIdx)
{
    if (auIdx + 1 > this->moFieldSet.size())
    {
        return 0;
    }

    return this->moFieldSet[auIdx].muFieldWidth;
}

apl_int_t CResultSet::GetFieldNameType(apl_size_t auIdx)
{
    if (auIdx + 1 > this->moFieldSet.size())
    {
        return -1;
    }

    return this->moFieldSet[auIdx].muDataType;
}

apl_int_t CResultSet::GetRowCount()
{
    return this->moResultSet.size();
}

apl_int_t CResultSet::MoveToRow(apl_size_t auIndex)
{
    apl_size_t luMax = this->moResultSet.size();

    if (auIndex + 1 > luMax) 
    {
        return -1; 
    }

    this->miCurrentRow = auIndex;

    return 0;
}

apl_int_t CResultSet::MoveToNextRow()
{
    apl_size_t luMax = this->moResultSet.size();

    if (apl_size_t(this->miCurrentRow + 2) > luMax) 
    {
        return -1; 
    }

    //only increase current row when it's valid
    this->miCurrentRow ++; 

    return 0;
}

apl_int_t CResultSet::MoveToPrevRow()
{
    apl_size_t luMax = this->moResultSet.size();

    if (this->miCurrentRow > 0
            && apl_size_t(this->miCurrentRow) <= luMax - 1)
    {
        this->miCurrentRow --;
    }
    else
    {
        return -1;
    }

    return 0;
}

void CResultSet::AddResult(void* apvPtr, apl_size_t auLen, apl_size_t auRow)
{
    CResult loResult;

    loResult.mpvResult = apvPtr;
    loResult.muLen = auLen;

    //new row
    if (auRow + 1 > this->moResultSet.size())
    {
        VROW_T loRow;
        loRow.push_back(loResult);
        this->moResultSet.push_back(loRow);
    }
    else
    {
        //existed row
        this->moResultSet[auRow].push_back(loResult);
    }
}

void CResultSet::ReserveResultCount(apl_size_t auCount)
{
    this->moResultSet.reserve(auCount);
}

void CResultSet::ClearResults(apl_int_t aiMode)
{
    apl_size_t luIter;
    apl_size_t luRowIter;

    this->miCurrentRow = -1;

    //go thru all rows
    for (luIter = 0; luIter < this->moResultSet.size(); ++luIter)
    {
        //go thru all fileds
        for (luRowIter = 0;
                luRowIter < this->moResultSet[luIter].size();
                ++luRowIter)
        {
            if (this->moResultSet[luIter][luRowIter].mpvResult)
            {
                apl_free(this->moResultSet[luIter][luRowIter].mpvResult);
                this->moResultSet[luIter][luRowIter].mpvResult = APL_NULL;
            }
        }

        if (aiMode) { break; }//only free the 1st row, for oracle

        this->moResultSet[luIter].clear();
    }

    this->moResultSet.clear();
}


apl_int_t CResultSet::ValidateCurRow(apl_int_t aiRow, apl_size_t auField)
{
    apl_int_t liMax = this->moResultSet.size();

    if (aiRow < 0) { return -1; }

    if (aiRow + 1 > liMax) { return -1; }

    liMax = this->moResultSet[aiRow].size();

    if (auField + 1 > (apl_size_t)liMax)
    {
        return -1; 
    }

    return 0;
}

apl_int_t CResultSet::AddFieldName(
        const char* apcField,
        apl_size_t auFieldWidth, 
        apl_size_t auDataType, 
        char* apcContent)

{
    if (apcField == APL_NULL) { return -1; }

    CField loField;

    loField.muFieldName = apcField;
    loField.muFieldWidth = auFieldWidth;
    loField.muDataType = auDataType;
    //loField.mpcContent = apcContent; 
    //DON'T USE any more
    loField.mpcContent = APL_NULL;

    this->moFieldSet.push_back(loField);

    return 0;
}

void CResultSet::ClearFieldNames()
{
    this->moFieldSet.clear();
}

apl_size_t CResultSet::GetCurrentRow()
{
    return this->miCurrentRow;
}

void CResultSet::SetCurrentRow(apl_int_t aiRow)
{
    this->miCurrentRow = aiRow;
}


ADL_NAMESPACE_END
