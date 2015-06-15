#include "spread.h"
Spread* Spread::_pInstance = new Spread();

Spread::Spread()
{
    userGraphMap = NULL;
    sidWTIMap = NULL;
    widSTMap = NULL;
    keyWordList.clear();
    hotWordList.clear();
    keyWordMap.clear();
    hotWordMap.clear();
    f1 = 0.0;
    C =50; //关键词
    C1 = 100;  //热点话题词
    mu = 0.1; //传播阈值
    tao = 0.5; //时间粒度
    xipuxiluo = 8; //1/5
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"Spread Init");

}

Spread* Spread::GetInstance()
{
    if(_pInstance == NULL)
        _pInstance = new Spread();
    return _pInstance;
}

bool Spread::GetKeyWordMap(const string &startTime,const string &endTime)
{
    map<int,int> *widCountMap;
    map<int,int>::iterator widCountiter;
    map<int,float>::iterator keyWordIter;
    //统计在时间粒度下startTime 到 endTime之间的关键词
    AssessUtil::GetInstance()->IntervalAssess(startTime,endTime);
    widCountMap = AssessUtil::GetInstance()->GetWidCountMap();
    for(keyWordIter = keyWordMap.begin();keyWordIter != keyWordMap.end();keyWordIter++)
    {
        keyWordIter->second = 0;
    }
    keyWordList.clear();
    for(widCountiter = widCountMap->begin();widCountiter != widCountMap->end();widCountiter++)
    {
        keyWordIter = keyWordMap.find(widCountiter->first);
        if(keyWordIter != keyWordMap.end())
        {
            keyWordMap[widCountiter->first] = widCountiter->second;
            keyWordList.push_back(widCountiter->first);
        }
    }
    return true;
}

bool Spread::GetHotWordMap(const string &startTime,const string &endTime)
{
    map<int,int> *widCountMap;
    map<int,int>::iterator widCountiter;
    map<int,float>::iterator hotWordIter;
    //统计在时间粒度下startTime 到 endTime之间的关键词
    AssessUtil::GetInstance()->IntervalAssess(startTime,endTime);
    widCountMap = AssessUtil::GetInstance()->GetWidCountMap();
    for(hotWordIter = hotWordMap.begin();hotWordIter != hotWordMap.end();hotWordIter++)
    {
        hotWordIter->second = 0;
    }
    hotWordList.clear();
    for(widCountiter = widCountMap->begin();widCountiter != widCountMap->end();widCountiter++)
    {
        hotWordIter = hotWordMap.find(widCountiter->first);
        if(hotWordIter != hotWordMap.end())
        {
            hotWordMap[widCountiter->first] = widCountiter->second;
            hotWordList.push_back(widCountiter->first);
        }
    }
    return true;
}

bool Spread::BuildKeyHotWordMap(const string &startTime,const string &endTime)
{
    keyWordMap.clear();
    hotWordMap.clear();
    //根据一天中的统计，找到关键词集合以及热点话题词集合
    AssessUtil::GetInstance()->IntervalAssess(startTime,endTime);
    map<int,int> *widCountMap;
    map<int,int>::iterator widCountiter;
    widCountMap = AssessUtil::GetInstance()->GetWidCountMap();
    for(widCountiter = widCountMap->begin();widCountiter != widCountMap->end();widCountiter++)
    {
         if(widCountiter->second > C && CacheUtil::GetInstance()->FindPosByWid(widCountiter->first)== "n")
            {
                keyWordMap[widCountiter->first] = 0;
                if(widCountiter->second > C1)
                    hotWordMap[widCountiter->first] = 0;
            }
    }
    return true;
}

bool Spread::CalculateF1()
{
    vector<int> commonWordList;
    map<int,float>::iterator hotWordIter;
    map<int,float>::iterator keyWordIter;
    float R = 0.0;
    float P = 0.0;
    float f1 = 0.0;
    for(unsigned int i=0;i<keyWordList.size();i++)
    {
        hotWordIter = hotWordMap.find(keyWordList[i]);
        if(hotWordIter != hotWordMap.end() && hotWordIter->second != 0)
            commonWordList.push_back(keyWordList[i]);
    }

    for(unsigned int i=0;i<commonWordList.size();i++)
    {
        hotWordIter = hotWordMap.find(commonWordList[i]);
        keyWordIter = keyWordMap.find(commonWordList[i]);
        float hotWordValue = 0.0;
        float keyWordvalue = 0.0;
        float precision = 0.0;
        if(hotWordIter != hotWordMap.end())
        {
            hotWordValue = hotWordIter->second;
        }

        if(keyWordIter != keyWordMap.end())
        {
            keyWordvalue = keyWordIter->second;
        }

        precision = (hotWordValue+keyWordvalue-fabs(hotWordValue-keyWordvalue))/(hotWordValue+keyWordvalue);
        R += precision;
        P += precision;
    }
    //LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger(),"commonWordList:"<<commonWordList.size());
    if(P != 0 && R != 0)
    {
        P = P/keyWordList.size();
        R = R/hotWordList.size();
        f1 = 2*P*R/(P+R);
        LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),f1<<"\t"<<P<<"\t"<<R);
        //LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),f1);
    }
    else
    {
        LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),0<<"\t"<<0<<"\t"<<0);
        //LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),0);
    }


    return true;
}

bool Spread::ModelSpread(Queue<long> &lqueue,map<long,UserStatus> &userSMap)
{
    map<long,UserStatus>::iterator iter2;
    map<long,float> userSMapDcache;
    map<long,float>::iterator iter3;
    map<long int,UserLink>::iterator iter4;
    while(!lqueue.IsEmpty()){
            long userid = lqueue.GetQueue();
            iter4 = userGraphMap->find(userid);
            //免疫因子 /mu
            if(userSMap[userid].tfIdf<mu){
                      continue;
            }
            if(iter4 != userGraphMap->end())
            {
                UserLink head = iter4->second;
                float srcWeight = head->weight;
                float dstWeight;
                int connect;
                UserNode *g = head->next;
                while(g!=NULL){
                    iter2 = userSMap.find(g->uid);
                    if(iter2 == userSMap.end())
                    {
                        dstWeight = g->weight;
                        connect = g->connect;
                        float diffuseRate = DiffuseRate(srcWeight,dstWeight,connect);
                        float tfIdf = userSMap[userid].tfIdf;
                        iter3 = userSMapDcache.find(g->uid);
                        if(iter3 != userSMapDcache.end())
                            userSMapDcache[g->uid] += tfIdf*diffuseRate;
                        userSMapDcache[g->uid] = tfIdf*diffuseRate;
                    }
                    g = g->next;
                }
            }
            if(lqueue.IsEmpty())
            {
               for(iter3 = userSMapDcache.begin();iter3 != userSMapDcache.end();iter3++)
               {
                   lqueue.EnQueue(iter3->first);
                   UserStatus userStatus;
                   userStatus.infected =true;
                   userStatus.tfIdf = iter3->second;
                   userSMap[iter3->first] = userStatus;
               }
               userSMapDcache.clear();
            }
        }
    return true;
}

bool Spread::BuildQueue(Queue<long> &lqueue,map<long,UserStatus> &userSMap,StatusTagLink p,const int &wid)
{
    map<long,WordTfIdfLink>::iterator iter;
    map<long,UserStatus>::iterator iter2;
    while(p!=NULL)
    {
        long userid = p->userid;
        float tfIdf;
        iter = sidWTIMap->find(p->sid);
        if(iter != sidWTIMap->end())
        {
            WordTfIdfNode *q = iter->second->next;
            while(q!=NULL)
            {
                if(q->wid == wid)
                {
                    tfIdf = q->tfIdf;
                    break;
                }
                q = q->next;
            }
            if(q==NULL)
                tfIdf = 0.0;
        }
        else
        {
                tfIdf = 0.0;
        }
        p = p->next;
        iter2 = userSMap.find(userid);
        if(iter2 != userSMap.end())
        {
            iter2->second.tfIdf += tfIdf;
        }
        else
        {
            UserStatus userStatus;
            userStatus.tfIdf = tfIdf;
            userStatus.infected = true;
            userSMap[userid] = userStatus;
            lqueue.EnQueue(userid);
        }

    }
    return true;
}

float Spread::DiffuseRate(const int &srcWeight,const int &dstWeight,const int &connect)
{
    //xipuxiluo 也是一个总要参数
    float diffuseRate = float(srcWeight)/(srcWeight+dstWeight)/exp(1/double(connect))/xipuxiluo;
    //float diffuseRate = float(srcWeight)/(srcWeight+dstWeight)/pow(4,1/double(connect))/20;
    //float diffuseRate = float(srcWeight)/(srcWeight+dstWeight);
    //cout<<"diffuseRate:"<<diffuseRate<<endl;
    return diffuseRate;
}

bool Spread::ShowUserSMap(map<long,UserStatus> &userSMap)
{
    map<long,UserStatus>::iterator iter;
    for(iter = userSMap.begin();iter != userSMap.end();iter++){
            cout<<"userid:"<<iter->first;
            cout<<" "<<"tdIdf:"<<iter->second.tfIdf<<endl;
    }
    return true;
}

bool Spread::TopicDiffuse()
{
    map<int,StatusTagLink>::iterator iter;
    map<long,UserStatus> userSMap;
    map<long,UserStatus>::iterator userSIter;
    for(unsigned int i=0;i<keyWordList.size();i++)
    {
        //针对每一个单词，循环查找每一个单词，它的概率
        float keyWordValue = 0.0;
        int wid = keyWordList[i]; //获得keyword的id
        iter = widSTMap->find(wid);
        if(iter == widSTMap->end())
        {
            LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"continue");
            continue;
        }

        Queue<long> useridQueue;
        StatusTagNode *p = iter->second->next;
        BuildQueue(useridQueue,userSMap,p,wid);
        ModelSpread(useridQueue,userSMap);

        for(userSIter = userSMap.begin();userSIter != userSMap.end();userSIter++)
        {
            keyWordValue += userSIter->second.tfIdf;
        }

        keyWordMap[keyWordList[i]]=keyWordValue;
        userSMap.clear();
    }
    return true;
}

bool Spread::Init(){
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"Spread Init");
    //进行话题初始化，计算的是2009-10-01之前的话题数据
    sidWTIMap = TopicDiscover::GetInstance()->GetSidWTIMap();
    userGraphMap = BuildGraph::GetInstance()->GetUserGraphMap();
    widSTMap = TopicDiscover::GetInstance()->GetWidSTMap();

    //时间粒度 /tao 0.5小时，1个小时；2个小时
    //对时间进行初始化
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
    string timeArray2[24] = {"00:30:00","01:30:00","02:30:00","03:30:00",
                             "04:30:00","05:30:00","06:30:00","07:30:00",
                             "08:30:00","09:30:00","10:30:00","11:30:00",
                             "12:30:00","13:30:00","14:30:00","15:30:00",
                             "16:30:00","17:30:00","18:30:00","19:30:00",
                             "20:30:00","21:30:00","22:30:00","23:30:00"};

    string startTime;
    string midTime;
    string endTime;
    //统计40天当中，话题传播 时间上可以改
    LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,TimeUtil::GetInstance()->LocalTimeStr());
    for(unsigned int day=0;day<30;day++)
    {
        cout<<"day:"<<day<<endl;
        startTime = dateArray[day] + " " + timeArray[0];
        endTime = dateArray[day+1] + " " + timeArray[0];
        if(!BuildKeyHotWordMap(startTime,endTime))
        {
            LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                    ,"BuildKeyHotWordMap fail");
        }
       for(unsigned int tao = 0;tao<1;tao++)
        {

                startTime = dateArray[day] + " " + timeArray[tao*24];
                midTime = dateArray[day+1] + " " + timeArray[0];
                endTime = dateArray[day+2] + " " + timeArray[0];
            //获取keyword List
            if(!GetKeyWordMap(startTime,midTime))
            {
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                    ,"GetKeyWordMap fail");
            }
            //获取hotwordList
            if(!GetHotWordMap(midTime,endTime))
            {
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                    ,"GetHotWordMap fail");
            }

            //传播模拟获取predictWordList
            if(!keyWordList.empty())
            {
                if(!DynamicSpread(startTime,midTime,endTime))
                {
                    LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                        ,"DynamicSpread fail");
                    return false;
                }
                  //计算预测结果的F1值
                if(!CalculateF1())
                {
                    LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                        ,"CalculateF1 fail");
                    return false;
                }

            }
            else
            {
                if(!BuildGraph::GetInstance()->AddGraph(startTime,endTime))
                {
                    LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,"AddGraph fail");
                    return false;
                }
            }
        }
    }
    return true;
}

bool Spread::DynamicSpread(const string &startTime,const string &midTime,const string &endTime)
{
    if(!BuildGraph::GetInstance()->AddGraph(startTime,midTime))
    {
         LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"AddGraph fail");
         return false;
    }

    if(!TopicDiscover::GetInstance()->IntervalTopic(startTime,midTime))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"InervalTopic fail");
         return false;
    }
    //获取各种字典的指针
    sidWTIMap = TopicDiscover::GetInstance()->GetSidWTIMap();
    widSTMap = TopicDiscover::GetInstance()->GetWidSTMap();
    userGraphMap = BuildGraph::GetInstance()->GetUserGraphMap();
    if(!TopicDiffuse())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"TopicDiffuse fail");
        return false;
    }
    return true;
}

void Spread::Show(){
    /*
    map<long,WordTfIdfLink>::iterator iter2;
    struct UserNode *p;
    for(iter1 = userGraphMap->begin();iter1 != userGraphMap->end();iter1++){
            cout<<"userid:"<<iter1->first<<" Link ";
            p = iter1->second;
            while(p!=NULL){
                cout<<p->uid<<" "<<p->conntect<<" "<<p->weight<<" ";
                p = p->next;
            }
            cout<<endl;
    }
    */
    /*
    map<long int,UserLink>::iterator iter1;
    WordTfIdfNode *q;
    for(iter2 = sidWTIMap->begin();iter2 != sidWTIMap->end();iter2++){
            cout<<"sid:"<<iter2->first<<endl;
            WordTfIdfNode *q = iter2->second;
            while(q!=NULL){
                cout<<"wid:"<<q->wid<<" tfIdf:"<<q->tfIdf;
                q = q->next;
            }
            cout<<endl;
    }
    */
    /*
    map<int,StatusTagLink>::iterator iter3;
    StatusTagNode *r;
    for(iter3 = widSTMap->begin();iter3 != widSTMap->end();iter3++){
        cout<<"wid:"<<iter3->first<<endl;
        r = iter3->second;
        while(r!=NULL){
            cout<<"userid:"<<r->userid;
            r = r->next;
        }
        cout<<endl;
    }
    */
}

void Spread::Test(){
    Init();
}

Spread::~Spread()
{
    userGraphMap = NULL;
    sidWTIMap = NULL;
    widSTMap = NULL;
    if(Spread::_pInstance)
        delete Spread::_pInstance;
}
