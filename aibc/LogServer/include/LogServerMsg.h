#ifndef LOG_SERVER_MESSAGE
#define LOG_SERVER_MESSAGE


/**
 * Request mode.
 *
 * Currently there only two modes:
 *      one-way, Caller should not expect a response in this mode
 *      two-way, Caller will be blocked and wait for a response
 */
#define LOG_SERVER_MODE_ONE_WAY                     1
#define LOG_SERVER_MODE_TWO_WAY                     2


/**
 * The Impl class of CLogServerMsg.
 */
class CLogServerMsgImpl;


/**
 * The class to store the values of Tag-Len-Value(TLV).
 */
class CLogServerMsg {
public:
    CLogServerMsg ();
    ~CLogServerMsg ();

    /**
     * Add a Tag by a value buffer and length.
     *
     * @param[in] auTag     The tag to add.
     * @param[in] apcValue  The buffer pointer of the value to add.
     * @param[in] auLen     The length fo the value to add.
     * @retval >=0  The index of the added aiTag
     * @retval <0   Add failture
     */
    int AddTagValue(unsigned short auTag, char* apcValue, unsigned short auLen);

    /**
     * Set/Replace a Tag by its index, value buffer and length.
     *
     * @param[in] aiIdx     The index to set.
     * @param[in] auTag     The new tag to set.
     * @param[in] apcValue  The buffer pointer of the value to set.
     * @param[in] auLen     The length fo the value to set.
     * @retval >=0  The index of the set auTag
     * @retval <0   set failture
     */
    int SetTagValue(int aiIdx, unsigned short auTag, char* apcValue, unsigned short auLen);

    /**
     * Delete a Tag by its index.
     *
     * @param[in] aiIdx     The index to delete.
     */
    void DelTagValue(int aiIdx);

    /**
     * Get the size of tags.
     *
     * @return how many tags had added.
     */
    int GetSize();

    /**
     * Get Value by a tag its index.
     *
     * @param[in] aiIdx     The index to get.
     * @param[out] apuTag   will be fill with the Tag if not NULL
     * @param[out] apuLen   will be fill with the Len if not NULL
     * @param[out] apvValue will be fill with the pointer of Value if not NULL
     * @retval >=0  The index of the Get auTag
     * @retval <0   Get failture
     */
    int GetTLV(int aiIdx, unsigned short* apuTag, unsigned short* apuLen, void** appvValue);

    /**
     * Search from aiBegin, return the index of the 1st found auTag
     *
     * @param[in] auTag     The searching tag.
     * @param[in] aiBegin   Search from this position.
     * @retval >=0  The index of auTag
     * @retval <0   Can't find auTag in the list
     */
    int Search(unsigned short auTag, int aiBegin=0);

    /**
     * Clear all the tags
     */
    void Clear();


protected:
    CLogServerMsgImpl* mpoLogServerMsgImpl;
};

#endif // LOG_SERVER_MESSAGE
