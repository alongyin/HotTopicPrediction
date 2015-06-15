#include "timeUtil.h"

TimeUtil* TimeUtil::_pInstance = new TimeUtil();

TimeUtil::TimeUtil()
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"TimeUtil Init");
}

TimeUtil* TimeUtil::GetInstance(){
    if(_pInstance == NULL)
        _pInstance = new TimeUtil();
    return _pInstance;
}

time_t TimeUtil::ConvertStrToTime_T(const string &timeStr)
{
    struct tm tmct;
    time_t time;
    sscanf(timeStr.c_str(),"%d-%d-%d %d:%d:%d",
           &(tmct.tm_year),
           &(tmct.tm_mon),
           &(tmct.tm_mday),
           &(tmct.tm_hour),
           &(tmct.tm_min),
           &(tmct.tm_sec));
    time = mktime(&tmct);
    return time;
}

string TimeUtil::ConvertTime_TToStr(const time_t &time)
{
  struct tm *p;
  p = localtime(&time);
  char buffer[80];
  string dateTime;
  sprintf(buffer,"%04d-%02d-%02d %02d:%02d:%02d",p->tm_year,p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
  dateTime = buffer;
  return dateTime;
  //sprintf()
  //return ctime(time);
}

void TimeUtil::Test()
{
    /*string time = "2009-12-30 00:00:00";
    string time2 = "0070-00-02 08:30:00"; //起始时间为0070-00-01 08:30:00
    time_t a = ConvertStrToTime_T(time);
    time_t a2 = ConvertStrToTime_T(time2);
    cout<<"time_t:"<<a<<endl;
    cout<<"time_t:"<<a2<<endl;
    cout<<"dateTime:"<<ConvertTime_TToStr(a)<<endl;
    cout<<"dateTime:"<<ConvertTime_TToStr(a+a2)<<endl;
    cout<<"dateTime:"<<ConvertTime_TToStr(0)<<endl;
    */
    time_t rawTime;
    time(&rawTime);
    cout<<"now:"<<ConvertTime_TToStr(rawTime)<<endl;
}

string  TimeUtil::LocalTimeStr()
{
    time_t rawTime;
    time(&rawTime);
    return ConvertTime_TToStr(rawTime);
}
TimeUtil::~TimeUtil()
{
    if(TimeUtil::_pInstance)
        delete TimeUtil::_pInstance;
}
