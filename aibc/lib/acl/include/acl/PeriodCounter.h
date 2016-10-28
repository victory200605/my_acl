#ifndef ACL_PERIOD_COUNTER_H
#define ACL_PERIOD_COUNTER_H

#include <list>
#include "acl/Synch.h"

ACL_NAMESPACE_START

#define PERIOD_COUNTER_MIN(v1, v2) ((v1) < (v2) ? (v1) : (v2))
#define PERIOD_COUNTER_MAX(v1, v2) ((v1) > (v2) ? (v1) : (v2))

#define PERIOD_ALL -1

class CPeriodCounter
{
public: 
    struct CStatData
    {
        apl_size_t  ciTotal;
        apl_size_t  ciCounter;
        apl_int_t   ciMaxValue;
        apl_int_t   ciMinValue;
    };
    
    struct CNode
    {
        char csName[64+1];
        time_t ciLastPeriodTime;
        std::list<CStatData> coPeriodLink;
    };
    
public:
    /**
        * @brief A constructor.
        */
    CPeriodCounter();
    
    /**
        * @brief A constructor.
        * @param [in] aiMaxPeriod - the max period
        * @param [in] aiTimePeriod - the period of time
        */
    CPeriodCounter(const apl_int_t aiMaxPeriod, const apl_int_t aiTimePeriod);

    /**
        * @brief A destructor.
        */
    ~CPeriodCounter();
    
    /**
        * @brief Init
        * @param [in] aiMaxPeriod - the max period
        * @param [in] aiTimePeriod - the period of time
        */
    apl_int_t Init(const apl_int_t aiMaxPeriod, const apl_int_t aiTimePeriod);

    /** 
        * @brief Get period count by name
        *
        * @param [in] apcName - period name
        */
    apl_int_t GetPeriodCount(const char* apcName);

    /** 
        * @brief Remove the last period by name
        *
        * @param [in] apcName - period name
        */
    apl_int_t RemoveLastPeriod(const char* apcName);

    /** 
        * @brief Remove the all periods by name
        *
        * @param [in] apcName - period name
        */
    apl_int_t RemoveAllPeriod(const char* apcName);

    /** 
        * @brief add data to the first period by name
        *
        * @param [in] apcName - period name
        */
    apl_int_t AddData(const char* apcName, const apl_int_t aiData);

    /** 
        * @brief Get Count of the period by name
        *
        * @param [in] apcName - period name
        * @param [in] aiPeriodNo - period no(-1...0...N-1)
        */
    apl_size_t GetCount(const char* apcName, const apl_int_t aiPeriodNo = PERIOD_ALL);

    /** 
        * @brief Get Sum of the period by name
        *
        * @param [in] apcName - period name
        * @param [in] aiPeriodNo - period no(-1...0...N-1)
        */
    apl_size_t GetSum(const char* apcName, const apl_int_t aiPeriodNo = PERIOD_ALL);

    /** 
        * @brief Get Average of the period by name
        *
        * @param [in] apcName - period name
        * @param [in] aiPeriodNo - period no(-1...0...N-1)
        */
    apl_int_t GetAverage(const char* apcName, const apl_int_t aiPeriodNo = PERIOD_ALL);

    /** 
        * @brief Get Min value of the period by Name
        *
        * @param [in] apcName - period name
        * @param [in] aiPeriodNo - period no(-1...0...N-1)
        */
    apl_int_t GetMin(const char* apcName, const apl_int_t aiPeriodNo = PERIOD_ALL);

    /** 
        * @brief Get Max value of the Period by Name
        *
        * @param [in] apcName - period name
        * @param [in] aiPeriodNo - period no(-1...0...N-1)
        */
    apl_int_t GetMax(const char* apcName, const apl_int_t aiPeriodNo = PERIOD_ALL);
    
    /** 
        * @brief Reset period by name
        *
        * @param [in] apcName - period name
        * @param [in] aiPeriodNo - period no(-1...0...N-1)
        */
    apl_int_t ResetPeriod(const char* apcName, const apl_int_t aiPeriodNo = PERIOD_ALL);
    
private:
    apl_int_t AddData(CNode* apcNode, const apl_int_t aiData);
    
    apl_int_t TryMovePeriod(CNode* apcNode);

    apl_int_t LocatePeriod(CNode* apcNode, const apl_int_t aiPeriodNo, CStatData *apcStatData);

private:
    enum{ PERIOD_MAX_LENGTH_NAME = 64 };
    
    apl_int_t           ciMaxPeriod;
    apl_int_t           ciTimePeriod;
    acl::CLock          coLock;
    std::list<CNode*>   coNodeLink;
};

ACL_NAMESPACE_END

#endif//ACL_PERIOD_COUNTER_H

