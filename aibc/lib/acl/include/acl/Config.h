/**
 * @file Config.h
 */

#ifndef ACL_BASECONFIG_H
#define ACL_BASECONFIG_H

#include "acl/Utility.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"

ACL_NAMESPACE_START

/**
 * class CBaseConfig
 */
class CBaseConfig
{
protected:
    ////////////////////////////////// Type //////////////////////////////////
    struct CKey
    {
        char const* GetKey(void)         { return this->moKey.c_str(); }
        char const* GetValue(void)       { return this->moValue.c_str(); }
        
        std::string moKey;
        std::string moValue;
    };
    
    struct CField
    {
        char const* GetName(void)        { return this->moName.c_str(); }
        
        std::string moName;
        std::vector<CKey> moKeys;
    };
    
    struct CComment
    {
        apl_ssize_t GetFieldID(void)     { return this->miFieldID; }
        apl_ssize_t GetKeyID(void)       { return this->miKeyID; }
        char const* GetComment(void)     { return this->moComment.c_str(); }
        
        apl_ssize_t miFieldID;
        apl_ssize_t miKeyID;
        std::string moComment;
    };
    
    ///////////////////////////////////////// Iterator ///////////////////////////////////
    /**
     * class CKeyIterator
     */
    class CKeyIterator
    { 
    public:
        /**
         * @brief A constructor.
         *
         * @param [in] apoConfig - the CBaseConfig object
         * @param [in] aiFieldID  - the field ID
         * @param [in] aiKeyID  - the key ID
         * @param [in] apcDefault - the default value
         */
        CKeyIterator( 
            CBaseConfig* apoConfig,
            apl_ssize_t aiFieldID,
            apl_ssize_t aiKeyID,
            char const* apcDefault );
        
        /**
         * @brief Overloaded operator ==
         *
         * @param [in] aoRhs - the other CKeyIterator object
         *
         * @retval true Equal.
         * @retval false Not equal.
         */
        bool operator == ( CKeyIterator const& aoRhs );
        
        /**
         * @brief Overloaded operator !=
         *
         * @param [in] aoRhs - the other CKeyIterator object
         *
         * @retval true Not equal.
         * @retval false Equal.
         */
        bool operator != ( CKeyIterator const& aoRhs );
        
        /**
         * @brief Overloaded operator ++i
         *
         * @return The CKeyIterator object after add-self.
         */
        CKeyIterator& operator ++ (void);

        /** 
         * @brief Return key name
         */
        char const* GetName(void);

        /**
         * @brief Check the key iterator is empty or not.
         *
         * @retval true The key iterator is empty.
         * @retval false The key iterator isn't empty.
         */
        bool IsEmpty(void);
        
        /**
         * @brief Get int value.
         *
         * @return Upon successful completion,the int value shall be returned.Otherwise, -1 shalled be returned.
         */
        apl_int_t ToInt(void);
        
        /**
         * @brief Get bool value.
         *
         * @return True or false.
         */
        bool ToBool(void);
        
        /**
         * @brief Get float value.
         *
         * @return The float value.
         */
        double ToFloat(void);
        
        /**
         * @brief Get string value.
         *
         * @return The string value.
         */
        char const* ToString(void);
    
    private:
        CBaseConfig* mpoConfig;
        
        apl_ssize_t miFieldID;
        
        apl_ssize_t miKeyID;
        
        char const* mpcDefault;
    };

    /**
     * class CFieldIterator
     */
    class CFieldIterator
    { 
    public:
        /**
         * @brief A constructor.
         *
         * @param [in] apoConfig - the CBaseConfig object
         * @param [in] aiFieldID - the field ID
         * @param [in] apcFieldPattern - the fileld pattern
         */
        CFieldIterator( CBaseConfig* apoConfig, apl_ssize_t aiFieldID, char const* apcFieldPattern );
        
        /**
         * @brief Overloaded operator ==
         *
         * @param [in] aoRhs - the other CFieldIterator object
         *
         * @retval true Equal.
         * @retval false Not equal.
         */
        bool operator == ( CFieldIterator const& aoRhs );
        
        /**
         * @brief Overloaded operator !=
         *
         * @param [in] aoRhs - the other CFieldIterator object
         *
         * @retval true Not equal.
         * @retval false Equal.
         */
        bool operator != ( CFieldIterator const& aoRhs );
        
        /**
         * @brief Overloaded operator ++i
         *
         * @return The CFieldIterator object after add-self.
         */
        CFieldIterator& operator ++ (void);
        
        /**
         * @brief Check field iterator is empty or not.
         *
         * @retval true The field iterator is empty.
         * @retval false The field iterator isn't empty.
         */
        bool IsEmpty(void);
        
        /**
         * @brief Get field name.
         *
         * @return The field name.
         */
        char const* GetFieldName(void);

        /**
         * @brief Get the value according to the key.
         *
         * @param [in] apcKey - the specified key
         * @param [in] apcDefault - the default value
         *
         * @return The CKeyIterator object.
         */
        CKeyIterator GetValue( char const* apcKey, char const* apcDefault );
        
        /**
         * @brief Get the value according to the key index.
         *
         * @param [in] aiIndex - the key index
         * @param [in] apcDefault - the default value
         *
         * @return The CKeyIterator object.
         */
        CKeyIterator GetValue( apl_size_t aiIndex, char const* apcDefault );
        
        /**
         * @brief Get the key counts.
         *
         * @return The key counts.
         */
        apl_size_t GetKeyCount(void);
        
        /**
         * @brief Get the begining of the field iterator. 
         *
         * @return The CKeyIterator object.
         */
        CKeyIterator Begin(void);
        
        /**
         * @brief Get the ending of the field iterator.
         *
         * @return The CKeyIterator object.
         */
        CKeyIterator End(void);
    
    private:
        CBaseConfig* mpoConfig;
        
        apl_ssize_t miFieldID;
        
        char const* mpcFieldPattern;
    };

public:
    typedef CKey KeyType;
    typedef CField FieldType;
    typedef CComment CommentType;
    typedef CFieldIterator FieldIterType;
    typedef CKeyIterator KeyIterType;
    
public:
    /**
     * @brief Get the value according to the key.
     *
     * @param [in] apcField - the field
     * @param [in] apcKey - the key
     * @param [in] apcDefault - the default value
     *
     * @return The CKeyIterator object.
     */
    KeyIterType GetValue( char const* apcField, char const* apcKey, char const* apcDefault = NULL );
    
    /**
     * @brief Get the value according to the key index.
     *
     * @param [in] apcField - the field
     * @param [in] aiIndex - the key index
     * @param [in] apcDefault - the default value
     *
     * @return The CKeyIterator object.
     */
    KeyIterType GetValue( char const* apcField, apl_size_t aiIndex, char const* apcDefault = NULL );
    
    /**
     * @brief Get the key counts on the specified field name.
     *
     * @param [in] apcField - the specified field name
     *
     * @return The key count.
     */
    apl_size_t GetKeyCount( char const* apcField );
    
    /**
     * @brief Get the field count.
     *
     * @param [in] apcFieldPattern  - the field pattern
     *
     * @return The field count.
     */
    apl_size_t GetFieldCount( char const* apcFieldPattern = NULL );
    
    /**
     * @brief Get the begining of the field iterator according to the specified field pattern.
     * If the field pattern is NULL,it will match all the fields.
     *
     * @param [in] apcFieldPattern - the field pattern
     *
     * @return The CFieldIterator object.
     */
    FieldIterType Begin( char const* apcFieldPattern = NULL );
    
    /**
     * @brief Get the ending of the field iterator.
     *
     * @return The CFieldIterator object.
     */
    FieldIterType End(void);

    /**
     * @brief Set int value.
     *
     * @param [in] apcField - the field
     * @param [in] apcKey - the key
     * @param [in] aiValue - the int value
     * @param [in] abIsCreate - whether create node while the object node is inexistence
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Set value failed.
     */
    apl_int_t SetValue( 
        char const* apcField,
        char const* apcKey,
        apl_int_t aiValue,
        bool abIsCreate = true );
        
    /**
     * @brief Set bool value.
     *
     * @param [in] apcField - the field
     * @param [in] apcKey - the key
     * @param [in] abValue - the bool value
     * @param [in] abIsCreate - whether create node while the object node is inexistence
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Set value failed.
     */
    apl_int_t SetValue( 
        char const* apcField,
        char const* apcKey,
        bool abValue,
        bool abIsCreate = true );
    
    /**
     * @brief Set double value.
     *
     * @param [in] apcField - the field
     * @param [in] apcKey - the key
     * @param [in] adValue - the double value
     * @param [in] abIsCreate - whether create node while the object node is inexistence
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Set value failed.
     */
    apl_int_t SetValue( 
        char const* apcField,
        char const* apcKey,
        double adValue,
        bool abIsCreate = true );
    
    /**
     * @brief Set string value.
     *
     * @param [in] apcField - the field
     * @param [in] apcKey - the key
     * @param [in] apcValue - the string value
     * @param [in] abIsCreate - whether create node while the object node is inexistence
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Set value failed.
     */
    apl_int_t SetValue( 
        char const* apcField, 
        char const* apcKey, 
        char const* apcValue,
        bool abIsCreate = true );
        
    /**
     * @brief Add comment for the specified key.
     *
     * @param [in] apcField - the field 
     * @param [in] apcKey - the key
     * @param [in] apcComment - the comment to add
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Set value failed.
     */
    apl_int_t AddComment( char const* apcField, char const* apcKey, char const* apcComment );
        
    /**
     * @brief Add comment for the specified field.
     *
     * @param [in] apcField - the field
     * @param [in] apcComment - the comment to add
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Set value failed.         
     */
    apl_int_t AddComment( char const* apcField, char const* apcComment );
        
    /**
     * @brief Delete comment for the specified key.
     *
     * @param [in] apcField - the value
     * @param [in] apcKey - the key
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Set value failed.         
     */
    apl_ssize_t DelComment( char const* apcField, char const* apcKey );
    
    /**
     * @brief Delete comment for the specified field.
     *
     * @param [in] apcField - the value
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Set value failed.         
     */
    apl_ssize_t DelComment( char const* apcField );

protected:
    KeyType& GetKey( apl_size_t aiFieldID, apl_size_t aiKeyID );
    
    apl_size_t GetKeySize( apl_size_t aiFieldID );
    
    FieldType& GetField( apl_size_t aiFieldID );
    
    apl_size_t GetFieldSize(void);
    
    CommentType& GetComment( apl_size_t aiCommentID );
    
    apl_size_t GetCommentSize(void);
    
    apl_ssize_t FindField( char const* apcField );
    
    apl_ssize_t FindField( apl_size_t aiPos, char const* apcFieldPattern );
    
    apl_ssize_t FindKey( apl_size_t aiFieldID, char const* apcKey );
    
    apl_ssize_t AddField( char const* apcField );
    
    apl_ssize_t AddKey( apl_size_t aiFieldID, char const* apcKey, char const* apcValue );
    
    void AddComment( apl_ssize_t aiFieldID, apl_ssize_t aiKeyID, char const* apcComment );
    
    void FixComment( apl_ssize_t aiFieldID, apl_ssize_t aiKeyID );
    
    bool Compare( char const* apcLhs, char const* apcRhs );
    
    bool Match( char const* apcFieldPattern, char const* apcTest );
    
    void Clear(void);
    
protected:        
    std::vector<CComment> moComments;
    std::vector<CField> moFields;
};

ACL_NAMESPACE_END

#endif //ACL_BASECONFIG_H
