#ifndef ASSESSUTIL_H
#define ASSESSUTIL_H
#include "global.h"
#include "timeUtil.h"
#include "dbUtil.h"
#include "logUtil.h"
#include "strUtil.h"
using namespace std;
class AssessUtil{
private:
    static AssessUtil *_pInstance;
    map<int,int> widCountMap;
public:
    AssessUtil();
    static AssessUtil *GetInstance();
    bool Init();
    bool IntervalAssess(const string &startTime,const string &endTime); //每隔一个时间段重新
    map<int,int> *GetWidCountMap();
    void Test();
    ~AssessUtil();
};
#endif // ASSESSUTIL_H
