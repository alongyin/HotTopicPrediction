#ifndef PARSER_H
#define PARSER_H

#include "json/json.h"
#include "global.h"
#include "dbUtil.h"
#include "logUtil.h"
#include "strUtil.h"
#include "httpUtil.h"
#include "cacheUtil.h"
class ParseUtil
{
private:
    string api_key; //用户注册语言云服务后获得的认证标示
    string url; //url网址链接
    string pattern; //分析模式
    string format; //结果格式类型
    static ParseUtil *_pInstance;
    map<string,string> wordMap;
public:
    ParseUtil();
    static ParseUtil *GetInstance();
    string GetResponseByHttp(const string &text);

    bool ParseWord();
    bool SubParseWord(const vector<Status> &statusList);
    bool DealWord();

    bool ParseStatusTag(); //解析status到statusTag里面
    bool MakeUrl(const string &text,string &urlwww);
    void Test();
    ~ParseUtil();
};

#endif
