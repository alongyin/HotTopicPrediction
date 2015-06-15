#ifndef HTTPUTIL_H
#define HTTPUTIL_H

#include <curl/curl.h>
#include "global.h"
#include "logUtil.h"
using namespace std;
class HttpUtil
{
private:
    static HttpUtil *_pInstance;
public:
    HttpUtil();
    static HttpUtil *GetInstance();
    /*
    @brief HTTP POST请求
    @param strUrl 输入参数，请求的url地址，如:www.google.com
    @param strPost 输入参数，使用如下格式
    @param strResponse 输入参数，返回内容
    @return 返回是否post成功
    */
    int Post(const string &strUrl, const string &strPost,string &strResponse);
    int Get(const string &strUrl,string &strResponse);
    void Test();
    ~HttpUtil();
};
#endif
