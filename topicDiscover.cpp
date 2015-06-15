#include "topicDiscover.h"
TopicDiscover* TopicDiscover::_pInstance = new TopicDiscover();

TopicDiscover::TopicDiscover(){
    if(!Init())
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"TopicDiscover Init fail");
}

TopicDiscover* TopicDiscover::GetInstance(){
    if(_pInstance == NULL)
        _pInstance = new TopicDiscover();
    return _pInstance;
}

map<int,StatusTagLink>* TopicDiscover::GetWidSTMap(){
    return &widSTMap;
}

map<long,WordTfIdfLink>* TopicDiscover::GetSidWTIMap()
{
    return &sidWTIMap;
}

void TopicDiscover::Show()
{
    cout<<"totalStatus:"<<totalStatus<<endl;
    map<long,WordTfIdfLink>::iterator iter;
    for(iter = sidWTIMap.begin();iter != sidWTIMap.end();iter++){
            cout<<"sid:"<<iter->first<<endl;
            WordTfIdfNode *p = iter->second;
            while(p!=NULL){
                cout<<"wid:"<<p->wid<<" tfIdf:"<<p->tfIdf;
                p = p->next;
            }
            cout<<endl;
    }
}

bool TopicDiscover::DealTagStatus(long sid,const string &widList){
    map<long,int>::iterator iter1;
    map<int,StatusTagLink>::iterator iter2;
    vector<string> widV;
    int wordCount = 0;
    if(!StrUtil::GetInstance()->Split(widList,",",widV))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Split error");
        return false;
    }

    for(unsigned int i=0;i<widV.size();i++){
        int wid = atoi(widV[i].c_str());
        string pos = CacheUtil::GetInstance()->FindPosByWid(wid);
        if(pos != "n")
        {
            continue;
        }
        iter2 = widSTMap.find(wid);
        if(iter2 == widSTMap.end()){
                StatusTagLink head = new StatusTagNode;
                StatusTagNode *p = new StatusTagNode;
                long userId = CacheUtil::GetInstance()->FindUserIdBySid(sid);
                head->sid = 0;
                head->freqCount = 1;
                p->freqCount = 1;
                p->next = NULL;
                p->sid = sid;
                p->userid = userId;
                head->next = p;
                widSTMap[wid] = head;
        }
        else{
            StatusTagLink head = iter2->second;
            if(sid == head->next->sid){
                head->next->freqCount++;
            }
            else{
                long userId = CacheUtil::GetInstance()->FindUserIdBySid(sid);
                head->freqCount++;
                StatusTagNode *p = new StatusTagNode;
                p->freqCount = 1;
                p->sid = sid;
                p->userid = userId;
                p->next = head->next;
                head->next = p;
            }
        }
        wordCount++;
    }
    sidWCountMap[sid] = wordCount;
    return true;
}

bool TopicDiscover::Init(){
    string startTime = "2009-08-12 00:00:00";
    string query = "select count(*) from tagstatus where time < '";
    query += startTime + "'";

    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"TopicDiscover Init"<<" startTime:"
                   <<startTime<<" query:"<<query);

    if(DbUtil::GetInstance()->DbQueryCount(query,totalStatus))
    {
        totalStatus = 0;
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbQueryCount fail");
        return false;
    }

    query = "select * from tagstatus where time < '";
    query += startTime + "'";
    vector<TagStatus> tagStatusList;
    if(DbUtil::GetInstance()->DbQueryTagStatus(query,tagStatusList))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbQueryTagStatus fail");
        return false;
    }
    for(unsigned int i=0;i<tagStatusList.size();i++){
        string widList = tagStatusList[i].widList;
        long sid = tagStatusList[i].sid;
        if(!DealTagStatus(sid,widList)){
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,"DealTagStatus Error");
                return false;
        }
    }
    if(!TfIdf())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"TfIdf fail");
         return false;
    }
    return true;
}

bool TopicDiscover::TfIdf()
{
    map<int,StatusTagLink>::iterator iter;
    map<long,WordTfIdfLink>::iterator iter2;
    for(iter = widSTMap.begin();iter != widSTMap.end();iter++){
        int wid = iter->first;
        StatusTagLink head = iter->second;
        int appearCount = head->freqCount;
        float idf = log(float(totalStatus)/(appearCount));
        StatusTagNode *p = head->next;
        StatusTagNode *q;
        while(p!=NULL){
            iter2 = sidWTIMap.find(p->sid);
            int freq = p->freqCount;
            float tfIdf = float(freq)/sidWCountMap[p->sid]*idf/2;
            if(iter2 == sidWTIMap.end()){
                    //sid包含在 sidWTIMap中
                    WordTfIdfLink head2 = new WordTfIdfNode;
                    WordTfIdfNode *p2 = new WordTfIdfNode;
                    head2->tfIdf = tfIdf;
                    head2->wid = 0;
                    p2->tfIdf = tfIdf;
                    p2->wid = wid;
                    head2->next = p2;
                    p2->next = NULL;
                    sidWTIMap[p->sid] = head2;
            }
            else{
                //sid不包含在 sidWTIMap中
                WordTfIdfNode *p2 = new WordTfIdfNode;
                p2->tfIdf = tfIdf;
                p2->wid = wid;
                iter2->second->tfIdf += tfIdf;
                p2->next = iter2->second->next;
                iter2->second->next = p2;
            }
            q = p;
            p = p->next;
        }
    }
    return true;
}

bool TopicDiscover::IntervalTopic(const string &startTime,const string &endTime)
{
    //清空所有数据
    Clear();
    string query = "select count(*) from tagstatus where time >= '";
    query += startTime + "'";
    query += " and time < '";
    query += endTime + "'";

    //将endTime 赋值给 startTime
    if(DbUtil::GetInstance()->DbQueryCount(query,totalStatus))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbQueryCount fail");
        totalStatus = 0;
    }


    query = "select * from tagstatus where time >= '";
    query += startTime + "'";
    query += " and time < '";
    query += endTime + "'";
    vector<TagStatus> tagStatusList;
    if(DbUtil::GetInstance()->DbQueryTagStatus(query,tagStatusList))
    {
         LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbQueryTagStatus fail");
         return false;
    }

    for(unsigned int i=0;i<tagStatusList.size();i++){
        string widList = tagStatusList[i].widList;
        long sid = tagStatusList[i].sid;
        if(!DealTagStatus(sid,widList)){
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger(),
                                "DealTagStatus Error");
                return false;
        }
    }

    if(!TfIdf())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger(),
                        "TfIdf fail");
         return false;
    }
    return true;
}

bool TopicDiscover::Clear()
{
    totalStatus = 0;
    //清空sidWCountMap
    sidWCountMap.clear();

    //widSTMap 清空
    map<int,StatusTagLink>::iterator iter;
    for(iter = widSTMap.begin();iter != widSTMap.end(); iter++){
        StatusTagNode *p = iter->second;
        StatusTagNode *q = p;
        while(q!=NULL)
        {
            q = p->next;
            delete p;
            p = q;
        }
    }
    widSTMap.clear();

    //sidWTIMap 清空
    map<long,WordTfIdfLink>::iterator iter2;
    for(iter2 = sidWTIMap.begin();iter2 != sidWTIMap.end();iter2++){
        WordTfIdfNode *p2 = iter2->second;
        WordTfIdfNode *q2 = p2;
        while(q2!=NULL){
            q2 = p2->next;
            delete p2;
            p2 = q2;
        }
    }
    sidWTIMap.clear();
    return true;
}

void TopicDiscover::Test()
{
    //时间粒度 tao 1个小时 2个小时 3个小时
    string dateArray[127] = {"2009-08-12 00:00:00","2009-08-13 00:00:00","2009-08-14 00:00:00","2009-08-15 00:00:00"
    ,"2009-08-16 00:00:00","2009-08-17 00:00:00","2009-08-18 00:00:00","2009-08-19 00:00:00","2009-08-20 00:00:00"
    ,"2009-08-21 00:00:00","2009-08-22 00:00:00","2009-08-23 00:00:00","2009-08-24 00:00:00","2009-08-25 00:00:00"
    ,"2009-08-26 00:00:00","2009-08-27 00:00:00","2009-08-28 00:00:00","2009-08-29 00:00:00","2009-08-30 00:00:00"
    ,"2009-08-31 00:00:00","2009-09-01 00:00:00","2009-09-02 00:00:00","2009-09-03 00:00:00","2009-09-04 00:00:00"
    ,"2009-09-05 00:00:00","2009-09-06 00:00:00","2009-09-07 00:00:00","2009-09-08 00:00:00","2009-09-09 00:00:00"
    ,"2009-09-10 00:00:00","2009-09-11 00:00:00","2009-09-12 00:00:00","2009-09-13 00:00:00","2009-09-14 00:00:00"
    ,"2009-09-15 00:00:00","2009-09-16 00:00:00","2009-09-17 00:00:00","2009-09-18 00:00:00","2009-09-19 00:00:00"
    ,"2009-09-20 00:00:00","2009-09-21 00:00:00","2009-09-22 00:00:00","2009-09-23 00:00:00","2009-09-24 00:00:00"
    ,"2009-09-25 00:00:00","2009-09-26 00:00:00","2009-09-27 00:00:00","2009-09-28 00:00:00","2009-09-29 00:00:00"
    ,"2009-09-30 00:00:00","2009-10-01 00:00:00","2009-10-02 00:00:00","2009-10-03 00:00:00","2009-10-04 00:00:00"
    ,"2009-10-05 00:00:00","2009-10-06 00:00:00","2009-10-07 00:00:00","2009-10-08 00:00:00","2009-10-09 00:00:00"
    ,"2009-10-10 00:00:00","2009-10-11 00:00:00","2009-10-12 00:00:00","2009-10-13 00:00:00","2009-10-14 00:00:00"
    ,"2009-10-15 00:00:00","2009-10-16 00:00:00","2009-10-17 00:00:00","2009-10-18 00:00:00","2009-10-19 00:00:00"
    ,"2009-10-20 00:00:00","2009-10-21 00:00:00","2009-10-22 00:00:00","2009-10-23 00:00:00","2009-10-24 00:00:00"
    ,"2009-10-25 00:00:00","2009-10-26 00:00:00","2009-10-27 00:00:00","2009-10-28 00:00:00","2009-10-29 00:00:00"
    ,"2009-10-30 00:00:00","2009-10-31 00:00:00","2009-11-01 00:00:00","2009-11-02 00:00:00","2009-11-03 00:00:00"
    ,"2009-11-04 00:00:00","2009-11-05 00:00:00","2009-11-06 00:00:00","2009-11-07 00:00:00","2009-11-08 00:00:00"
    ,"2009-11-09 00:00:00","2009-11-10 00:00:00","2009-11-11 00:00:00","2009-11-12 00:00:00","2009-11-13 00:00:00"
    ,"2009-11-14 00:00:00","2009-11-15 00:00:00","2009-11-16 00:00:00","2009-11-17 00:00:00","2009-11-18 00:00:00"
    ,"2009-11-19 00:00:00","2009-11-20 00:00:00","2009-11-21 00:00:00","2009-11-22 00:00:00","2009-11-23 00:00:00"
    ,"2009-11-24 00:00:00","2009-11-25 00:00:00","2009-11-26 00:00:00","2009-11-27 00:00:00","2009-11-28 00:00:00"
    ,"2009-11-29 00:00:00","2009-11-30 00:00:00","2009-12-01 00:00:00","2009-12-02 00:00:00","2009-12-03 00:00:00"
    ,"2009-12-04 00:00:00","2009-12-05 00:00:00","2009-12-06 00:00:00","2009-12-07 00:00:00","2009-12-08 00:00:00"
    ,"2009-12-09 00:00:00","2009-12-10 00:00:00","2009-12-11 00:00:00","2009-12-12 00:00:00","2009-12-13 00:00:00"
    ,"2009-12-14 00:00:00","2009-12-15 00:00:00","2009-12-16 00:00:00"};

    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),"Test Start");
/*
名词在每个单位时间（天）里，微博中的tfIDF
*/
/*
    for(int i=0;i<4;i++)
    {
        LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),
                       "date:"<<dateArray[i]);
        map<long,WordTfIdfLink>::iterator iter;
        for(iter = sidWTIMap.begin();iter != sidWTIMap.end();iter++)
        {
            LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),
                                "sid:"<<iter->first);
            WordTfIdfLink head = iter->second;
            WordTfIdfNode *p = head;
            while(p!=NULL)
            {
                LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),
                                "wid:"<<p->wid<<" cont:"<<CacheUtil::GetInstance()->FindContByWord(p->wid)
                               <<" tfidf:"<<p->tfIdf);
                p = p->next;
            }
        }
        IntervalTopic(dateArray[i+1]);
    }
*/

/*
比较每一天当中名词的准确率
*/
/*    map<int,float> widCountMap;
    map<int,float>::iterator iter2;
    map<int,int>::iterator iter3;
    map<int,int> *widCountMapC;
    map<long,WordTfIdfLink>::iterator iter;
    for(int i=0;i<125;i++)
    {
        cout<<"hiahia"<<endl;
        AssessUtil::GetInstance()->IntervalAssess(dateArray[i+1]);
        IntervalTopic(dateArray[i+1]);
        widCountMap.clear();
        //LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),
          //             "date:"<<dateArray[i]);
        for(iter = sidWTIMap.begin();iter != sidWTIMap.end();iter++)
        {
         //   LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),
           //                     "sid:"<<iter->first);
            WordTfIdfLink head = iter->second;
            WordTfIdfNode *p = head;
            while(p!=NULL)
            {
                iter2 = widCountMap.find(p->wid);
                if(iter2 != widCountMap.end())
                    widCountMap[p->wid] += p->tfIdf;
                else
                {
                    widCountMap[p->wid] = p->tfIdf;
                }
                p = p->next;
            }
     */
        //1. 进行比较,打印出比较前和比较后的词汇数
       /* widCountMapC = AssessUtil::GetInstance()->GetWidCountMap();
        for(iter2 = widCountMap.begin();iter2 != widCountMap.end();iter2++)
        {
            iter3 = widCountMapC->find(iter2->first);
            if(iter3 != widCountMapC->end())
            {
                LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),
                       "wid:"<<iter2->first<<" count1:"<<iter2->second<<" count2:"<<iter3->second);
            }
        }
        */
        //2. 进行比较，找到tfIdf之后的数值与真实数值的比值。
       /* widCountMapC = AssessUtil::GetInstance()->GetWidCountMap();
        for(iter2 = widCountMap.begin();iter2 != widCountMap.end();iter2++)
        {
            iter3 = widCountMapC->find(iter2->first);
            if(iter3 != widCountMapC->end())
            {
                if((iter2->second/iter3->second)>1){
                    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),
                       "wid:"<<iter2->first<<" cont:"<<CacheUtil::GetInstance()->FindContByWord(iter2->first));
                }
            }
        }
        */
        //3. 进行比较，找到词与词之间的倍数关系
        /*
        widCountMapC = AssessUtil::GetInstance()->GetWidCountMap();
        int record = 0;
        float total = 0.0;
        for(iter2 = widCountMap.begin();iter2 != widCountMap.end();iter2++)
        {
            iter3 = widCountMapC->find(iter2->first);
            if(iter3 != widCountMapC->end())
            {
                    total += iter2->second/iter3->second/2;
                    record++;
            }
        }
        LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),total/record);

    }
    */

}

TopicDiscover::~TopicDiscover(){
    Clear();
    if(TopicDiscover::_pInstance)
      delete TopicDiscover::_pInstance;
}
