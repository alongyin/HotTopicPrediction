#include "parseUtil.h"
using namespace std;
ParseUtil* ParseUtil::_pInstance = new ParseUtil();
//http://api.ltp-cloud.com/analysis/?api_key=X2O2n1y6WQtQHTrGklygCLkMUPHNagFE9edsYgRD&text=dsada&pattern=pos&format=json
//api_key = X2O2n1y6WQtQHTrGklygCLkMUPHNagFE9edsYgRD yzl350206931@126.com
//api_key = r2m491K77ANn9dqsdPFZpobLDxHS4rYRQpTn8t3C  350206931@qq.com
ParseUtil::ParseUtil(){
    url = "http://api.ltp-cloud.com/analysis/?";
    api_key = "r2m491K77ANn9dqsdPFZpobLDxHS4rYRQpTn8t3C";
    format = "json";
    pattern = "pos";

    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"ParseUtil Init");
}

ParseUtil* ParseUtil::GetInstance(){
    if(_pInstance == NULL)
        _pInstance = new ParseUtil();
    return _pInstance;
}

bool ParseUtil::MakeUrl(const string &text,string &urlwww){
    urlwww += url;
    urlwww += "api_key=" + api_key;
    urlwww += "&text=" + text;
    urlwww += "&pattern=" + pattern;
    urlwww += "&format=" + format;
    return true;
}

string ParseUtil::GetResponseByHttp(const string &text)
{
    string urlwww;
    MakeUrl(text,urlwww);
    string response;
    if(HttpUtil::GetInstance()->Get(urlwww,response))
        return response;
    return "";
}

bool ParseUtil::SubParseWord(const vector<Status> &statusList)
{
     //用于进行词汇的解析
     Json::Value root;
     Json::Reader reader;
     int record = 0;
     string content;
     for(unsigned int i=0;i<statusList.size();i++){
        content += statusList[i].cont + ",";
        record++;
        if(record == 10 || i == statusList.size()-1)
        {
            cout<<"parse get http"<<endl;
            content = GetResponseByHttp(content);
            if(!reader.parse(content,root))
            {
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,"reader parse fail");
                cout<<"reader parse fail"<<endl;
                record = 0;
                root.clear();
                content.clear();
                continue;
            }

            if(root.type() != Json::arrayValue)
            {
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,"root type is not arrayValue");
                cout<<"root type is not arrayValue"<<endl;
                record = 0;
                root.clear();
                content.clear();
                continue;
            }
            int L = root.size();
            for(unsigned int l = 0;l<L;l++){
                int M = root[l].size();
                for(unsigned int m =0;m<M;m++){
                    int N = root[l][m].size();
                    for(unsigned int n =0;n<N;n++){
                        string cont = root[l][m][n]["cont"].asString();
                        string pos = root[l][m][n]["pos"].asString();
                        wordMap[cont] = pos;
                    }
                }
            }
            root.clear();
            record = 0;
            content.clear();
        }
    }
    return true;
}

bool ParseUtil::ParseWord()
{
    int ret;
    int wid = 0;
    int record =0;
    int totalStatus = 190000;
    int interval = 1000;
    vector<Status> statusV;
    map<string,string>::iterator iter;

    //获取微博的总个数
    string query = "select count(*) from statusH";
    if(DbUtil::GetInstance()->DbQueryCount(query,totalStatus)){
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"totalStatus:"<<totalStatus);
        return false;
    }

    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"totalStatus:"<<totalStatus);

    query = "select sid,userid,weibocontent from statusH limit ";

    while(record <= totalStatus)
    {
        cout<<"record:"<<record<<endl;
        LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                       ,"record:"<<record);
        string startPos = StrUtil::GetInstance()->Int2String(record);
        string endPos = StrUtil::GetInstance()->Int2String(interval);
        string query2 = query + startPos + ",";
        query2 += endPos;
        if(DbUtil::GetInstance()->DbQueryStatus(query2,statusV,1)){
            LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                            ,"DbQueryStatus fail");
            return false;
        }

        //解析数据到wordMap中
        SubParseWord(statusV);
        statusV.clear();
        record += interval;

        //插入数据到数据库中
        string query3 = "insert into word(wid,cont,pos) values(";
        for(iter = wordMap.begin();iter != wordMap.end();iter++)
        {
            string query4 = query3 + StrUtil::GetInstance()->Int2String(wid) + ",'";
            query4 += iter->first + "','";
            query4 += iter->second + "')";
            if(DbUtil::GetInstance()->DbInsert(query4)){
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                            ,"DbInsert fail");
            cout<<"DbInsert fail"<<endl;
            }
            wid++;
        }
        wordMap.clear();
    }
   return true;
}

bool ParseUtil::DealWord()
{
    string query = "select cont,wid,pos from word";
    vector<Word> wordV;
    if(DbUtil::GetInstance()->DbQueryWord(query,wordV))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbQueryWord fail");
        return false;
    }

    query = "insert into wordH(wid,cont,pos) values(";
    for(unsigned int i=0;i<wordV.size();i++)
    {
        string query2;
        query2 += query + StrUtil::GetInstance()->Int2String(i) + ",'";
        query2 += wordV[i].cont +"','";
        query2 += wordV[i].pos + "')";
        if(DbUtil::GetInstance()->DbInsert(query2))
        {
            LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbInsert fail");
            return false;
        }
        cout<<"wid:"<<i<<endl;
    }
    return true;
}

bool ParseUtil::ParseStatusTag(){
    vector<Status> statusV;
    int ret;
    string query =  "select sid,userid,weibocontent,time from statusH";
    Json::Value root;
    Json::Reader reader;
    string query2;
    ret = DbUtil::GetInstance()->DbQueryStatus(query,statusV,1,1);

    if(ret)
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"ret:"<<ret);
        return false;
    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"statusV size:"<<statusV.size());
    for(unsigned int i = 0;i<statusV.size();i++){
            long sid = statusV[i].sid;
            string cont = GetResponseByHttp(statusV[i].cont);
            if(!reader.parse(cont,root))
            {
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,"reader parse fail");
                root.clear();
                continue;
            }

            if(root.type() != Json::arrayValue)
            {
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,"root type is not arrayValue");
                root.clear();
                continue;
            }
            int size = root[0U][0U].size();
            string widList;
            for(unsigned int j = 0;j<size;j++)
            {
                    string keyWord = root[0U][0U][j]["cont"].asString();
                    int wid = CacheUtil::GetInstance()->FindIdByWord(keyWord);
                    cout<<"find"<<endl;
                    if(wid != -1)
                        widList += StrUtil::GetInstance()->Int2String(wid) + ",";
            }
            query2 = "insert into tagstatus(sid,widlist,time) values(";
            query2 += StrUtil::GetInstance()->Int2String(sid) + ",'";
            query2 += widList +"','";
            query2 += statusV[i].datetime + "')";
            if(DbUtil::GetInstance()->DbInsert(query2))
            {
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbInsert fail");
                return false;
            }
            cout<<"status:"<<i<<endl;
            root.clear();
    }
    return true;
}

void ParseUtil::Test()
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"Test Start");
    //string query = "select sid,userid,weibocontent from status";
    //ParseWord(query);
    //string query = "select sid,userid,weibocontent,time from status";
    //ParseStatusTag(query);
    //ParseWord();
    //DealWord();
    ParseStatusTag();
/*
    DbUtil::GetInstance()->DbQueryStatus(query,statusV,1);
    string content;
    for(unsigned int i=0;i<20;i++){
            content += statusV[i].cont + ",";
    }
    cout<<"content:"<<content<<endl;
    content = GetResponseByHttp(content);
    cout<<"parse result:"<<content<<endl;
*/
}
ParseUtil::~ParseUtil(){
    if(ParseUtil::_pInstance)
        delete ParseUtil::_pInstance;
}

