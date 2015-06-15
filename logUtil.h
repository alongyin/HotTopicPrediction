#ifndef LOGUTIL_H
#define LOGUTIL_H
#include <log4cplus/logger.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/layout.h>
#include <iomanip>
#include "global.h"
using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
class LogUtil{
private:
    string pattern;
    Logger pLogger;
    static LogUtil *_pInstance;
public:
    LogUtil();
    ~LogUtil();
    bool Init();
    Logger GetLogger();
    void Test();
    static LogUtil *GetInstance();
};
#endif // LOGUTIL_H
/*
LOG4CPLUS_TRACE
LOG4CPLUS_DEBUG
LOG4CPLUS_INFO
LOG4CPLUS_WARN
LOG4CPLUS_ERROR
LOG4CPLUS_FATAL
*/
