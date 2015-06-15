#include "logUtil.h"

//LogUtil* LogUtil::_pInstance = new LogUtil();

LogUtil::LogUtil()
{
    Init();
}

LogUtil* LogUtil::GetInstance()
{
    if(_pInstance == NULL)
        _pInstance = new LogUtil();
}

bool LogUtil::Init()
{
   SharedAppenderPtr pFileAppender(new DailyRollingFileAppender("HotTopic.log",DAILY,0,11));
   //pattern = "%d{%m/%d/%y %H:%M:%S}   -%m[%l][%t]%n";
   pattern = "%m %n";
   pLogger = Logger::getInstance("weibo");
   auto_ptr<Layout> pPatternLayout(new PatternLayout(pattern));
   pFileAppender->setLayout(pPatternLayout);
   pLogger.addAppender(pFileAppender);
   return true;
}

Logger LogUtil::GetLogger()
{
    return pLogger;
}

void LogUtil::Test(){
}

LogUtil::~LogUtil()
{
    if(LogUtil::_pInstance)
        delete LogUtil::_pInstance;
}

