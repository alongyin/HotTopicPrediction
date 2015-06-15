#include "assessUtil.h"

LogUtil* LogUtil::_pInstance = new LogUtil();
AssessUtil* AssessUtil::_pInstance = new AssessUtil();

AssessUtil::AssessUtil()
{
    Init();
}

AssessUtil* AssessUtil::GetInstance(){
    if(_pInstance == NULL)
    {
        _pInstance = new AssessUtil();
    }
}

bool AssessUtil::Init()
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"AssessUtil Init");
    widCountMap.clear();
    return true;
}

bool AssessUtil::IntervalAssess(const string &startTime,const string &endTime)
{
    widCountMap.clear();
    map<int,int>::iterator iter;
    string query = "select * from tagstatus where time >= '";
    query += startTime + "'";
    query += " and time < '";
    query += endTime + "'";
    vector<TagStatus> tagStatusList;
    if(DbUtil::GetInstance()->DbQueryTagStatus(query,tagStatusList))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbQueryTagStatus fail"<<query);
        return false;
    }

    for(unsigned int i=0;i<tagStatusList.size();i++)
    {
        string widList = tagStatusList[i].widList;
        long sid = tagStatusList[i].sid;
        vector<string> widV;
        if(!StrUtil::GetInstance()->Split(widList,",",widV))
        {
            LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                            ,"Split Error"<<i);
            return false;
        }
        for(unsigned int j=0;j<widV.size();j++)
        {
            int wid = atoi(widV[j].c_str());
            iter = widCountMap.find(wid);
            if(iter == widCountMap.end())
                widCountMap[wid] = 1;
            else
                widCountMap[wid] += 1;
        }
    }
    return true;
}

map<int,int> *AssessUtil::GetWidCountMap()
{
    return &widCountMap;
}

void AssessUtil::Test()
{

}

AssessUtil::~AssessUtil()
{
    if(_pInstance)
        delete _pInstance;
}
