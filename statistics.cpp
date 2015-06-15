#include "statistics.h"

Statistics* Statistics::_pInstance = new Statistics();

Statistics::Statistics()
{
    widCountMap.clear();
    startTime = "2009-08-12 00:00:00";
}

Statistics* Statistics::GetInstance()
{
    if(_pInstance == NULL)
        _pInstance = new Statistics();
    return _pInstance;
}



bool Statistics::IntervalStatistics(const string &startTime,const string &endTime)
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

bool Statistics::Test()
{
    //时间粒度 1个小时，2个小时，3个小时
    string dateArray[127] = {"2009-08-12","2009-08-13","2009-08-14","2009-08-15"
    ,"2009-08-16","2009-08-17","2009-08-18","2009-08-19","2009-08-20"
    ,"2009-08-21","2009-08-22","2009-08-23","2009-08-24","2009-08-25"
    ,"2009-08-26","2009-08-27","2009-08-28","2009-08-29","2009-08-30"
    ,"2009-08-31","2009-09-01","2009-09-02","2009-09-03","2009-09-04"
    ,"2009-09-05","2009-09-06","2009-09-07","2009-09-08","2009-09-09"
    ,"2009-09-10","2009-09-11","2009-09-12","2009-09-13","2009-09-14"
    ,"2009-09-15","2009-09-16","2009-09-17","2009-09-18","2009-09-19"
    ,"2009-09-20","2009-09-21","2009-09-22","2009-09-23","2009-09-24"
    ,"2009-09-25","2009-09-26","2009-09-27","2009-09-28","2009-09-29"
    ,"2009-09-30","2009-10-01","2009-10-02","2009-10-03","2009-10-04"
    ,"2009-10-05","2009-10-06","2009-10-07","2009-10-08","2009-10-09"
    ,"2009-10-10","2009-10-11","2009-10-12","2009-10-13","2009-10-14"
    ,"2009-10-15","2009-10-16","2009-10-17","2009-10-18","2009-10-19"
    ,"2009-10-20","2009-10-21","2009-10-22","2009-10-23","2009-10-24"
    ,"2009-10-25","2009-10-26","2009-10-27","2009-10-28","2009-10-29"
    ,"2009-10-30","2009-10-31","2009-11-01","2009-11-02","2009-11-03"
    ,"2009-11-04","2009-11-05","2009-11-06","2009-11-07","2009-11-08"
    ,"2009-11-09","2009-11-10","2009-11-11","2009-11-12","2009-11-13"
    ,"2009-11-14","2009-11-15","2009-11-16","2009-11-17","2009-11-18"
    ,"2009-11-19","2009-11-20","2009-11-21","2009-11-22","2009-11-23"
    ,"2009-11-24","2009-11-25","2009-11-26","2009-11-27","2009-11-28"
    ,"2009-11-29","2009-11-30","2009-12-01","2009-12-02","2009-12-03"
    ,"2009-12-04","2009-12-05","2009-12-06","2009-12-07","2009-12-08"
    ,"2009-12-09","2009-12-10","2009-12-11","2009-12-12","2009-12-13"
    ,"2009-12-14","2009-12-15","2009-12-16"};

    string timeArray[24] = {"00:00:00","01:00:00","02:00:00","03:00:00",
                            "04:00:00","05:00:00","06:00:00","07:00:00",
                            "08:00:00","09:00:00","10:00:00","11:00:00",
                            "12:00:00","13:00:00","14:00:00","15:00:00",
                            "16:00:00","17:00:00","18:00:00","19:00:00",
                            "20:00:00","21:00:00","22:00:00","23:00:00"};

/*每一个词汇在时间轴上出现的次数顺序  one*/
/*
    map<int,int>::iterator iter;
    map<int,int*>::iterator iter2;
    string startTime;
    string endTime;
    for(unsigned int i=0;i<20;i++)
    {
        cout<<"Test Start"<<endl;
        for(unsigned int j=0;j<23;j++)
        {
            startTime = dateArray[i] + " " + timeArray[j];
            endTime = dateArray[i] + " " + timeArray[j+1];
            IntervalStatistics(startTime,endTime);
            for(iter = widCountMap.begin();iter != widCountMap.end();iter++)
                {
                    int wid = iter->first;
                    int wordCount = iter->second;
                    string pos = CacheUtil::GetInstance()->FindPosByWid(wid);
                    if(pos == "n")
                        {
                            iter2 = widDayCountMap.find(wid);
                            if(iter2 == widDayCountMap.end()){
                                    int *wordArray = new int[20*24];
                                    memset(wordArray,0,24*20*sizeof(int));
                                    wordArray[i*24+j] = wordCount;
                                    widDayCountMap[wid] = wordArray;
                                    }
                            else{
                            (iter2->second)[i*24+j] = wordCount;
                            }
                        }
                }
        }
        startTime = dateArray[i] + " " + timeArray[23];
        endTime = dateArray[i+1] + " " + timeArray[0];
        IntervalStatistics(startTime,endTime);
        for(iter = widCountMap.begin();iter != widCountMap.end();iter++)
            {
                int wid = iter->first;
                int wordCount = iter->second;
                string pos = CacheUtil::GetInstance()->FindPosByWid(wid);
                if(pos == "n")
                {
                    iter2 = widDayCountMap.find(wid);
                    if(iter2 == widDayCountMap.end()){
                        int *wordArray = new int[20*24];
                        memset(wordArray,0,24*20*sizeof(int));
                        wordArray[i*24+23] = wordCount;
                        widDayCountMap[wid] = wordArray;
                    }
                    else{
                        (iter2->second)[i*24+23] = wordCount;
                    }
                }
            }
    }


    //写入文件中展示
    for(iter2 = widDayCountMap.begin();iter2 != widDayCountMap.end();iter2++)
    {
        LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                            ,"wid:"<<iter2->first<<" cont:"<<CacheUtil::GetInstance()->FindContByWord(iter2->first));
        int totalRecord = 0;
        for(unsigned int j=0;j<20*24;j++)
        {
            totalRecord += iter2->second[j];
        }
        if(totalRecord>100)
        {
             for(unsigned int j=0;j<20*24;j++)
            {
                LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                               ,(iter2->second)[j]);
            }
        }

    }
*/
/*统计出每天所产生的微博个数 two*/
/*
for(unsigned int i=0;i<30;i++)
{
    string query = "select count(*) from tagstatus where time >= '";
    query += dateArray[i] + "'";
    query += " and time < '";
    query += dateArray[i+1] + "'";
    int totalCount = 0;
    DbUtil::GetInstance()->DbQueryCount(query,totalCount);
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,totalCount);
}
*/

/*统计在某一天当中，词汇量大于某个阈值后的，在一天当中出现的次数*/
map<int,int>::iterator iter;
map<int,int*>::iterator iter2;
vector<int> widList;
for(unsigned int i=0;i<100;i++)
{
    cout<<"test"<<endl;
    string startTime = dateArray[i] + " " + timeArray[0];
    string endTime = dateArray[i+1] + " " + timeArray[0];
    IntervalStatistics(startTime,endTime);
    for(iter = widCountMap.begin();iter != widCountMap.end();iter++)
        {
            int wid = iter->first;
            int wordCount = iter->second;
            string pos = CacheUtil::GetInstance()->FindPosByWid(wid);
            if(pos == "n" && wordCount>200 && wordCount<=500)
            {
                widList.push_back(wid);
            }
        }

    for(unsigned int k=0;k<widList.size();k++)
    {
        int *wordArray = new int[24];
        memset(wordArray,0,24*sizeof(int));
        widDayCountMap[widList[k]] = wordArray;
    }
    widList.clear();

    for(unsigned int j=0;j<23;j++)
    {
        startTime = dateArray[i] + " " + timeArray[j];
        endTime = dateArray[i] + " " + timeArray[j+1];
        IntervalStatistics(startTime,endTime);
        for(iter2 = widDayCountMap.begin();iter2 != widDayCountMap.end();iter2++)
        {
            iter = widCountMap.find(iter2->first);
            if(iter != widCountMap.end())
            {
                (iter2->second)[j] = iter->second;
            }
        }
    }
    startTime = dateArray[i] + " " + timeArray[23];
    endTime = dateArray[i+1] + " " + timeArray[0];
    IntervalStatistics(startTime,endTime);
    for(iter2 = widDayCountMap.begin();iter2 != widDayCountMap.end();iter2++)
    {
        iter = widCountMap.find(iter2->first);
        if(iter != widCountMap.end())
        {
            (iter2->second)[23] = iter->second;
        }
    }

    //将每个时间段内容输出显示
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,dateArray[i]);
    for(iter2 = widDayCountMap.begin();iter2 != widDayCountMap.end();iter2++)
    {
        LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                            ,"wid:"<<iter2->first<<" cont:"<<CacheUtil::GetInstance()->FindContByWord(iter2->first));
        for(unsigned int j=0;j<24;j++)
        {
            LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),(iter2->second)[j]);
        }
        delete iter2->second;
    }
    widDayCountMap.clear();
}

    return true;
}

Statistics::~Statistics()
{
    if(Statistics::_pInstance)
        delete Statistics::_pInstance;
}
