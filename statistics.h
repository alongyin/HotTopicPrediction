#ifndef STATISTICS_H
#define STATISTICS_H
#include "dbUtil.h"
#include "timeUtil.h"
#include "logUtil.h"
#include "strUtil.h"
#include "cacheUtil.h"
#include "global.h"


class Statistics
{
private:
    map<int,int> widCountMap;
    map<int,int*> widDayCountMap;
    string startTime;
    static Statistics *_pInstance;
public:
    static Statistics *GetInstance();
    Statistics();
    bool IntervalStatistics(const string &startTime,const string &endTime);
    bool Test();
    ~Statistics();
};
#endif // STATISTICS_H
