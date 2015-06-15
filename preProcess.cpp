#include "preProcess.h"

PreProcess* PreProcess::_pInstance = new PreProcess();

PreProcess::PreProcess()
{
    LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"PreProcess Init");
    if(!Init())
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"PreProcess Init fail");
}

PreProcess* PreProcess::GetInstance()
{
    if(_pInstance == NULL)
        _pInstance = new PreProcess();
    return _pInstance;
}


bool PreProcess::Init()
{
    string query = "select count(*) from status";
    if(DbUtil::GetInstance()->DbQueryCount(query,totalStatus)){
        totalStatus = 0;
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"totalStatus:"<<totalStatus);
        return false;
    }

    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                    ,"totalStatus:"<<totalStatus);
    return true;
}

bool PreProcess::CleanCont(vector<Status> &statusList)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"CleanCont Size:"<<statusList.size());
    string query = "insert into statusH (sid,weibocontent,time,userid) values(";
    for(unsigned int i=0;i<statusList.size();i++)
    {
        string cont = statusList[i].cont;
        cont = StrUtil::GetInstance()->Replace(cont);
        string query2 = query + StrUtil::GetInstance()->Int2String(statusList[i].sid) + ",'";
        query2 += cont + "','";
        query2 += statusList[i].datetime + "',";
        query2 += StrUtil::GetInstance()->Int2String(statusList[i].userid) + ")";
        //cout<<"sql:"<<query2<<endl;
        if(DbUtil::GetInstance()->DbInsert(query2))
        {
            LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                            ,"DbInsert fail sid:"<<statusList[i].sid);
            cout<<"DbInsert fail sid:"<<statusList[i].sid<<endl;
        }
    }
    return true;
}

bool PreProcess::ProcessStatus()
{
    vector<Status> statusV;
    int record = 0;
    int interval = 50000;
    string query = "select sid,userid,weibocontent,time from status limit ";
    while(record <= totalStatus)
    {
        cout<<"record:"<<record<<endl;
        string startPos = StrUtil::GetInstance()->Int2String(record);
        string endPos = StrUtil::GetInstance()->Int2String(interval);
        string query2 = query + startPos + ",";
        query2 += endPos;

        if(DbUtil::GetInstance()->DbQueryStatus(query2,statusV,1,1))
        {
            LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                           ,"DbQueryStatus fail");
            return false;
        }
        if(!CleanCont(statusV))
            return false;
        statusV.clear();
        record += interval;
    }
    return true;
}

bool PreProcess::Test()
{
    if(!ProcessStatus())
        return false;
    return true;
}


PreProcess::~PreProcess()
{
    if(PreProcess::_pInstance)
        delete PreProcess::_pInstance;
}
