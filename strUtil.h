#ifndef STRUTIL_H
#define STRUTIL_H
#include "global.h"
#include "logUtil.h"
using namespace std;

class StrUtil{
private:
    static StrUtil *_pInstance;
public:
    StrUtil();
    static StrUtil *GetInstance();
    string Int2String(const int &num);
    string float2String(const float &num);
    char* StrToChar(const string &str);
    bool Split(const string &str,const string &delim,vector<string> &strV);
    string Replace(string &str);

    //用于做测试
    void Test();
    ~StrUtil();
};
#endif // STRUTIL_H
