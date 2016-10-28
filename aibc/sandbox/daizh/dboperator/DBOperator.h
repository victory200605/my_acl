#ifndef AIMC_DB_OPERATOR_H
#define AIMC_DB_OPERATOR_H

#include "adl/Adl.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"

/** 
 * @brief Database wrapper
 *
 * This class is database operator encapsulation to make database operating simplify for other operator
 * User only to care GetErrorCode()/GetErrorMsg() method when operator executing fail.
 *
 */
class CDBWrapper
{
protected:
    typedef std::vector<std::string> RowType;

    struct CBindParameter
    {
        std::string moName;
        std::string moValue;
    };

public:
    /** 
     * @brief Returns database operated error code
     */
    apl_int_t GetErrorCode(void);

    /** 
     * @brief Returns database operated error message
     */
    char const* GetErrorMsg(void);

protected:
    CDBWrapper( adl::IDatabase* apoDb );
    
    ~CDBWrapper(void);
    
    apl_ssize_t ExecuteNonQuery( char const* apcSQL );

    apl_ssize_t ExecuteAndQuery( char const* apcSQL, std::vector<RowType>& aoResultset );

    void BindParameter( char const* apcName, char const* apcValue, apl_size_t auLen );

    void SetMessage( apl_int_t aiCode, char const* apcMessage );

    void ClearLastErrorMessage(void);

private:
    adl::IDatabase* mpoDb;

    std::vector<CBindParameter*> moBindParameteres;

    apl_int_t miErrorCode;
    
    std::string moErrorMessage;
};

/** 
 * @brief SQL where sub-expression condition define
 * 
 * user can't use this class to make complex condition expression, example: (f1=a and f2=b) or f3=c
 *
 */
class CDBCondition
{
public:
    CDBCondition(void);
    
    /** 
     * @brief Construct condition sub-expression 
     * 
     * @param apcName expression name
     * @param apcValue expression string type value, f1='a'
     */
    CDBCondition( char const* apcName, char const* apcValue );
    
    /** 
     * @brief Construct condition sub-expression 
     * 
     * @param apcName expression name
     * @param apcValue expression int type value, f1=1
     */
    CDBCondition( char const* apcName, apl_intmax_t aiValue );
    
    /** 
     * @brief Construct condition sub-expression 
     * 
     * @param apcName expression name
     * @param apcValue expression float type value, f1=1.0
     */
    CDBCondition( char const* apcName, double adValue );

    /** 
     * @brief Returns sub-expression by c-string
     */
    char const* GetCStr(void) const;
    
    /** 
     * @brief Returns sub-expression by string
     */
    std::string const& GetStr(void) const;

    /** 
     * @brief Multi-condition sub-expression Or operator
     */
    friend CDBCondition operator || ( CDBCondition const& aoLhs, CDBCondition const& aoRhs );
    
    /** 
     * @brief Multi-condition sub-expression and operator
     */
    friend CDBCondition operator && ( CDBCondition const& aoLhs, CDBCondition const& aoRhs );

private:
    std::string moCond;
};

/** 
 * @brief SQL where expression define for all database operator
 */
class CDBWhereExpression
{
public:
    CDBWhereExpression(void);

    /** 
     * @brief Construct condition sub-expression 
     * 
     * @param apcName expression name
     * @param apcValue expression string type value, f1='a'
     */
    void AddCondition( char const* apcName, char const* apcValue );
    
    /** 
     * @brief Construct condition sub-expression 
     * 
     * @param apcName expression name
     * @param apcValue expression int type value, f1=1
     */
    void AddCondition( char const* apcName, apl_intmax_t aiValue );
    
    /** 
     * @brief Construct condition sub-expression 
     * 
     * @param apcName expression name
     * @param apcValue expression float type value, f1=1.0
     */
    void AddCondition( char const* apcName, double adValue );
    
    /** 
     * @brief Set complex condition expression
     */
    void SetCondition( CDBCondition const& aoCond );

    /** 
     * @brief Returns where condition expression by string
     */
    std::string const& GetWhere(void) const;

protected:
    void AddCondition( char const* apcName, char const* apcValue, bool abIsText );

private:
    std::string moWhere;
};

/** 
 * @brief SQL create table operator
 */
class CDBCreateOperator : public CDBWrapper
{
public:
    CDBCreateOperator( adl::IDatabase* apoDb, char const* apcTableName );

    /** 
     * @brief Add table field define
     * 
     * @param apcName field name
     * @param apcType field type,exp:varchar(20), integer
     * @param abIsNotNull is adding NOT NULL tag
     * @param abIsPrimary is adding PRIMARY KEY tag
     * @param abIsAutoIncrement is adding AUTOINCREMENT tag
     */
    void AddField(
        char const* apcName,
        char const* apcType,
        bool abIsNotNull = false,
        bool abIsPrimary = false, 
        bool abIsAutoIncrement = false );

    /** 
     * @brief Add index for specified fields
     * 
     * @param apcColumn field name
     * @param abIsDesc is DESC
     */
    void AddIndex( char const* apcColumn, bool abIsDesc = false );
    
    /** 
     * @brief Add Unique index for specified fields
     * 
     * @param apcColumn field name
     * @param abIsDesc is DESC
     */
    void AddUniqueIndex( char const* apcColumn, bool abIsDesc = false );

    /** 
     * @brief Execute create table operator
     *
     * @returns if success return 0, otherwise -1 shall be return
     */
    apl_ssize_t Execute(void);

private:
    std::string moTableName;
    std::string moFields;
    std::string moIndexs;
    std::string moUniqueIndexs;
};

/** 
 * @brief SQL drop table operator
 */
class CDBDropOperator : public CDBWrapper
{
public:
    CDBDropOperator( adl::IDatabase* apoDb, char const* apcTableName ); 

    /** 
     * @brief Execute drop table operator
     *
     * @returns if success return 0, otherwise -1 shall be return
     */
    apl_ssize_t Execute(void);

private:
    std::string moTableName;
};

/** 
 * @brief SQL select table operator
 */
class CDBSelectOperator : public CDBWrapper, public CDBWhereExpression 
{
public:
    CDBSelectOperator( adl::IDatabase* apoDb, char const* apcTableName );

    /** 
     * @brief Add select field name, default is *
     * 
     * @param apcFieldName field name
     */
    void AddField( char const* apcFieldName );

    /** 
     * @brief Execute select table operator
     *
     * @returns if success return selected rows, otherwise -1 shall be return
     */
    apl_ssize_t Execute(void);

    /** 
     * @brief Returns resultset size
     */
    apl_size_t GetRowCount(void);

    /** 
     * @brief Returns resultset field value
     * 
     * @param auRow row index
     * @param auColumn column index
     * 
     * @returns field value by string
     */
    std::string const& GetField( apl_size_t auRow, apl_size_t auColumn ) const;

private:
    std::string moTableName;
    
    std::string moFields;

    apl_size_t muFieldCount;

    std::vector<RowType> moResultset;
};

/** 
 * @brief SQL insert record into specified table operator
 */
class CDBInsertOperator : public CDBWrapper
{
public:
    CDBInsertOperator( adl::IDatabase* apoDb, char const* apcTableName );

    ~CDBInsertOperator(void);

    /** 
     * @brief Add insert field and string-value
     * 
     * @param apcName field name
     * @param apcValue value
     */
    void AddField( char const* apcName, char const* apcValue );
    
    /** 
     * @brief Add insert field and binary-value
     * 
     * @param apcName field name
     * @param apcValue value
     * @param auLen value length
     */
    void AddField( char const* apcName, char const* apcValue, apl_size_t auLen );
    
    /**
     * @brief Add insert field and int-value
     * 
     * @param apcName field name
     * @param aiValue value
     */
    void AddField( char const* apcName, apl_intmax_t aiValue );
    
    /** 
     * @brief Add insert field and double-value
     * 
     * @param apcName field name
     * @param adValue value
     */
    void AddField( char const* apcName, double adValue );

    /** 
     * @brief Execute insert operator
     * 
     * @returns if success return affected row count, otherwise -1 shall be return  
     */
    apl_ssize_t Execute(void);

protected:
    void AddField( char const* apcName, char const* apcValue, bool abIsText );

private:
    std::string moTableName;

    std::string moFields;

    std::string moValues;
};

/** 
 * @brief SQL update operator
 */
class CDBUpdateOperator : public CDBWrapper, public CDBWhereExpression
{
public:
    CDBUpdateOperator( adl::IDatabase* apoDb, char const* apcTableName );

    /** 
     * @brief Add update field and string-value
     * 
     * @param apcName field name
     * @param apcValue value
     */
    void AddField( char const* apcName, char const* apcValue );
    
    /** 
     * @brief Add update field and binary-value
     * 
     * @param apcName field name
     * @param apcValue value
     * @param auLen value length
     */
    void AddField( char const* apcName, char const* apcValue, apl_size_t auLen );
    
    /**
     * @brief Add update field and int-value
     * 
     * @param apcName field name
     * @param aiValue value
     */
    void AddField( char const* apcName, apl_intmax_t aiValue );
    
    /**
     * @brief Add update field and double-value
     * 
     * @param apcName field name
     * @param aiValue value
     */
    void AddField( char const* apcName, double adValue );

    /** 
     * @brief Execute update operator
     * 
     * @returns if success return affected row count, otherwise -1 shall be return  
     */
    apl_ssize_t Execute(void);

protected:
    void AddField( char const* apcName, char const* apcValue, bool abIsText );

private:
    std::string moTableName;

    std::string moSet;
};

/** 
 * @brief SQL delete operator
 */
class CDBDeletOperator : public CDBWrapper, public CDBWhereExpression
{
public:
    CDBDeletOperator( adl::IDatabase* apoDb, char const* apcTableName );

    /** 
     * @brief Execute Delete operator
     * 
     * @returns if success return affected row count, otherwise -1 shall be return
     */
    apl_ssize_t Execute(void);

private:
    std::string moTableName;
};

#endif//AIMC_DB_OPERATOR_H

