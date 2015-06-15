#include "httpUtil.h"

static size_t Writer(void *buffer,size_t size,size_t nmemb,void* lpVoid)
{
    string* str = dynamic_cast<string*>((string *)lpVoid);
    if( NULL == str || NULL == buffer )
    {
        return -1;
    }

    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);
    return nmemb*size;
}

HttpUtil* HttpUtil::_pInstance = new HttpUtil();

HttpUtil* HttpUtil::GetInstance()
{
    if(_pInstance == NULL)
        _pInstance = new HttpUtil();
    return _pInstance;
}

HttpUtil::HttpUtil()
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                     ,"HttpUtil Init");
}

HttpUtil::~HttpUtil()
{
    if(HttpUtil::_pInstance)
        delete HttpUtil::_pInstance;
}

int HttpUtil::Get(const string &strUrl,string &strResponse)
{
    CURLcode code;
    char* error;
    CURL* curl = curl_easy_init();
    if(NULL == curl){
        return CURLE_FAILED_INIT;
    }

    code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(code != CURLE_OK)
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Failed to global init default code:"<<code);
        return false;
    }

    code = curl_easy_setopt(curl,CURLOPT_ERRORBUFFER,error);
    if(code != CURLE_OK)
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Failed to set error buffer code:"<<code);
        return false;
    }

    curl_easy_setopt(curl,CURLOPT_VERBOSE,1);

    code = curl_easy_setopt(curl,CURLOPT_URL,strUrl.c_str());
    if(code != CURLE_OK)
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Failed to set URL code:"<<code);
        return false;
    }

    code = curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1);
    if(code != CURLE_OK){
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Failed to set redirect option code:"<<code);
        return false;
    }

    code = curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,Writer);
    if(code != CURLE_OK)
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Failed to set writer code:"<<code);
        return false;
    }

    code = curl_easy_setopt(curl,CURLOPT_WRITEDATA,(void *)&strResponse);
    if(code != CURLE_OK)
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Failed to set write data: "<<error);
        return false;
    }
    /*
    当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作
    如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出
    */
    curl_easy_setopt(curl,CURLOPT_NOSIGNAL,1);
    curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,100);
    curl_easy_setopt(curl,CURLOPT_TIMEOUT,1000);

    /*清空perform*/
    code = curl_easy_perform(curl);
    if(code != CURLE_OK){
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Failed to get:"<<strUrl<<" error:"<<error);
        return false;
    }
    curl_easy_cleanup(curl);
    return true;
}

int HttpUtil::Post(const string &strUrl, const string &strPost,string &strResponse){
    return 0;
}

void HttpUtil::Test(){
}
