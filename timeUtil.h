#ifndef TIMEUTIL_H
#define TIMEUTIL_H
#include <time.h>
#include "global.h"
#include "logUtil.h"
using namespace std;
class TimeUtil{
private:
    static TimeUtil *_pInstance;
public:
    TimeUtil();
    time_t ConvertStrToTime_T(const string &timeStr);
    string ConvertTime_TToStr(const time_t &time);
    string  LocalTimeStr();
    static TimeUtil *GetInstance();
    void Test();
    ~TimeUtil();
};
#endif // TIMEUTIL_H
