#include "pruning.h"
Pruning* Pruning::_pInstance = new Pruning();

Pruning::Pruning()
{
    userGraphMap = NULL;
    sidWTIMap = NULL;
    widSTMap = NULL;
    keyWordList.clear();
    hotWordList.clear();
    keyWordMap.clear();
    hotWordMap.clear();
    f1 = 0.0;
    C =20; //关键词
    C1 = 40;  //热点话题词
    mu = 0.1; //传播阈值
    tao = 6; //时间粒度
    xipuxiluo = 8; //1/
    PruningCount = 11000; //需要剪枝的节点数
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"Pruning Init");

}

Pruning* Pruning::GetInstance()
{
    if(_pInstance == NULL)
        _pInstance = new Pruning();
    return _pInstance;
}

bool Pruning::GetKeyWordMap(const string &startTime,const string &endTime)
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

bool Pruning::GetHotWordMap(const string &startTime,const string &endTime)
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

bool Pruning::BuildKeyHotWordMap(const string &startTime,const string &endTime)
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

bool Pruning::CalculateF1()
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

    if(P != 0 && R != 0)
    {
        P = P/keyWordList.size();
        R = R/hotWordList.size();
        f1 = 2*P*R/(P+R);
        LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),f1<<"\t"<<P<<"\t"<<R);
    }
    else
    {
        LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),0<<"\t"<<0<<"\t"<<0);
    }
    return true;
}

bool Pruning::ModelSpread(Queue<long> &lqueue,map<long,UserStatus> &userSMap)
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

bool Pruning::BuildQueue(Queue<long> &lqueue,map<long,UserStatus> &userSMap,StatusTagLink p,const int &wid)
{
    map<long,WordTfIdfLink>::iterator iter;
    map<long int,UserLink>::iterator userGraphIter;
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

        userGraphIter = userGraphMap->find(userid);
        if(userGraphIter != userGraphMap->end())
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

float Pruning::DiffuseRate(const int &srcWeight,const int &dstWeight,const int &connect)
{
    //xipuxiluo 也是一个总要参数
    float diffuseRate = float(srcWeight)/(srcWeight+dstWeight)/exp(1/double(connect))/xipuxiluo;
    return diffuseRate;
}

bool Pruning::ShowUserSMap(map<long,UserStatus> &userSMap)
{
    map<long,UserStatus>::iterator iter;
    for(iter = userSMap.begin();iter != userSMap.end();iter++){
            cout<<"userid:"<<iter->first;
            cout<<" "<<"tdIdf:"<<iter->second.tfIdf<<endl;
    }
    return true;
}

bool Pruning::TopicDiffuse()
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"TopicDiffuse Start");
    map<int,StatusTagLink>::iterator iter;
    map<long,UserStatus> userSMap;
    map<long,UserStatus>::iterator userSIter;
    for(unsigned int i=0;i<keyWordList.size();i++)
    {
        //针对每一个单词，循环查找每一个单词，它的概率
        map<long,UserStatus> userSMap;
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
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"TopicDiffuse End");
    return true;
}

bool Pruning::Init(){
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"Pruning Init");
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

    map<int,int> *widCountMap;
    map<int,int>::iterator iter;
    string startTime;
    string midTime;
    string endTime;
    //统计40天当中，话题传播 时间上可以改
    for(unsigned int day=0;day<30;day++)
    {
        startTime = dateArray[day] + " " + timeArray[0];
        endTime = dateArray[day+1] + " " + timeArray[0];
        cout<<"day:"<<day<<endl;
        if(day==29)
        {
            if(!BuildKeyHotWordMap(startTime,endTime))
            {
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                    ,"BuildKeyHotWordMap fail");
            }
            cout<<"KeyWordMap Size:"<<keyWordMap.size()<<endl;
            for(unsigned int tao = 1;tao<2;tao++)
            {
                if(tao == 2)
                {
                    startTime = dateArray[day] + " " + timeArray[tao*6];
                    midTime = dateArray[day] + " " + timeArray[(tao+1)*6];
                    endTime = dateArray[day+1] + " " + timeArray[0];
                }
                else if(tao == 3)
                {
                    startTime = dateArray[day] + " " + timeArray[tao*6];
                    midTime = dateArray[day+1] + " " + timeArray[0];
                    endTime = dateArray[day+1] + " " + timeArray[6];
                }
                else
                {
                    startTime = dateArray[day] + " " + timeArray[tao*6];
                    midTime = dateArray[day] + " " + timeArray[(tao+1)*6];
                    endTime = dateArray[day] + " " + timeArray[(tao+2)*6];
                }

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
    return true;
}

bool Pruning::DynamicSpread(const string &startTime,const string &midTime,const string &endTime)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"DynamicSpread Start");
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

    if(!AssessUtil::GetInstance()->IntervalAssess(midTime,endTime))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"IntervalAssess fail");
        return false;
    }
    //获取各种字典的指针
    sidWTIMap = TopicDiscover::GetInstance()->GetSidWTIMap();
    widSTMap = TopicDiscover::GetInstance()->GetWidSTMap();
    userGraphMap = BuildGraph::GetInstance()->GetUserGraphMap();

    //对比实验
    /*
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),"start time:"<<TimeUtil::GetInstance()->LocalTimeStr());
    if(!TopicDiffuse())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"TopicDiffuse fail");
        return false;
    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),"end time:"<<TimeUtil::GetInstance()->LocalTimeStr());

    if(!StatisticUserInfo(userGraphMap))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"StatisticUserInfo fail");
        return false;
    }
    */
    //方案1：随机剪枝
    /*
    if(!PruningMethod1())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod1 fail");
        return false;
    }
    */


    //方案2：按照度数大的顺序剪枝
    /*
    if(!PruningMethod2())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod2 fail");
        return false;
    }
    */

    /*
    //方案3：按照度数小的顺序剪枝
    if(!PruningMethod3())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod3 fail");
        return false;
    }
    */



    //方案4：按照拥有话题的权重性来进行排名，减去排名靠后的用户
    if(!PruningMethod4())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod4 fail");
        return false;
    }

    //方案5： 根据用户的兴趣相似性，来进行用户排名
    /*
    if(!PruningMethod5())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod5 fail");
        return false;
    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"DynamicSpread End");
    */
    return true;
}

bool  Pruning::PruningMethod4()
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod4 Start");
    map<int,UserTopicValueLink> widUserTopicMap;
    map<long,UserLink>::iterator userGraphIter;
    for(unsigned int i=0;i<keyWordList.size();i++)
    {
        UserTopicValueLink head = (struct UserTopicValue *)malloc(sizeof(struct UserTopicValue));
        head->next = NULL;
        widUserTopicMap[keyWordList[i]] = head;
    }

    //初始化uidTopicWordInfoMap
    if(!InitUserTopicValueMap(widUserTopicMap))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"InitUserTopicValueMap fail");
        return false;
    }

    //将每个用户的话题数据按topic value值进行大小排序
    if(!SortUserTopicValueMap(widUserTopicMap))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"SortUserTopicValueMap fail");
        return false;
    }

    UserLLink userhead = (struct UserL *)malloc(sizeof(struct UserL));
    userhead->next = NULL;
    if(!SortUser(widUserTopicMap,userGraphMap,userhead))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"SortUser fail");
        return false;
    }

    if(!PruningUserGraph(userGraphMap,userhead))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"PruningUserGraph fail");
        return false;
    }

    if(!FreeUserTopicValueMap(widUserTopicMap))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"FreeUserTopicValueMap fail");
        return false;
    }

    if(!StatisticUserInfo(userGraphMap))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"StatisticUserInfo fail");
        return false;
    }
    //进行话题传播
    if(!TopicDiffuse())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"TopicDiffuse fail");
        return false;
    }

    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod4 End");
    return true;
}

bool  Pruning::InitUserTopicValueMap(map<int,UserTopicValueLink> &widUserTopicMap)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"InitUserTopicValueMap Start");
    map<int,StatusTagLink>::iterator iter;
    map<int,UserTopicValueLink>::iterator widUserTopicIter;
    float total = 0.0;
    int record = 0;
    for(unsigned int i=0;i<keyWordList.size();i++)
    {
        widUserTopicIter = widUserTopicMap.find(keyWordList[i]);
        if(widUserTopicIter == widUserTopicMap.end())
            continue;

        //针对每一个单词，循环查找每一个单词，它的概率
        map<long,UserStatus> userSMap;
        map<long,UserStatus>::iterator userSMapIter;
        int wid = keyWordList[i]; //获得keyword的id
        iter = widSTMap->find(wid);
        if(iter == widSTMap->end())
            continue;
        Queue<long> useridQueue;
        StatusTagNode *p = iter->second->next;
        BuildQueue(useridQueue,userSMap,p,wid);
        ModelSpread(useridQueue,userSMap);
        for(userSMapIter=userSMap.begin();userSMapIter!=userSMap.end();userSMapIter++)
        {
            UserTopicValue *userTopicValueNode = (struct UserTopicValue *)malloc(sizeof(struct UserTopicValue));
            userTopicValueNode->uid = userSMapIter->first;
            userTopicValueNode->value = userSMapIter->second.tfIdf;
            UserTopicValueLink head = widUserTopicIter->second;
            userTopicValueNode->next = head->next;
            head->next = userTopicValueNode;
        }
        userSMap.clear();
    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"InitUserTopicValueMap End");
    return true;
}

bool  Pruning::SortUserTopicValueMap(map<int,UserTopicValueLink> &widUserTopicMap)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"SortUserTopicValueMap Start");
    map<int,UserTopicValueLink>::iterator widUserTopicIter;
    for(widUserTopicIter = widUserTopicMap.begin();widUserTopicIter != widUserTopicMap.end();widUserTopicIter++)
    {
        UserTopicValueLink head = widUserTopicIter->second;
        UserTopicValue *p = head->next;
        if(p == NULL)
            continue;
        UserTopicValue *q = p->next;
        head->next = NULL;
        while(p!=NULL)
        {
            UserTopicValue *r = head;
            UserTopicValue *s = head->next;
            while(s!=NULL)
            {
                if(s->value < p->value)
                {
                    r->next = p;
                    p->next = s;
                    break;
                }
                r=s;
                s=s->next;
            }
            if(s == NULL)
            {
                r->next = p;
                p->next = s;
            }
            p = q;
            if(q!=NULL)
                q = q->next;
        }
    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"SortUserTopicValueMap End");
    return true;
}

bool  Pruning::SortUser(map<int,UserTopicValueLink> &widUserTopicMap,map<long int,UserLink> *userGraphMap,UserLLink userhead)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"SortUser Start");
    map<long,float> uidValueMap;
    map<long,float>::iterator uidValueIter;
    map<long int,UserLink>::iterator userGraphIter;
    map<int,UserTopicValueLink>::iterator widUserTopicIter;

    for(userGraphIter = userGraphMap->begin();userGraphIter != userGraphMap->end();userGraphIter++)
    {
        uidValueMap[userGraphIter->first] = 0.0;
    }

    for(widUserTopicIter = widUserTopicMap.begin();widUserTopicIter != widUserTopicMap.end();widUserTopicIter++)
    {
        UserTopicValueLink head = widUserTopicIter->second;
        int score = 200;
        head = head->next;
        while(head != NULL)
        {
            uidValueIter = uidValueMap.find(head->uid);
            if(uidValueIter != uidValueMap.end())
            {
                uidValueIter->second += score;
            }
            else
            {
                uidValueMap[head->uid] = score;
            }
            head = head->next;
            if(score != 0)
                score--;
        }
    }
    for(uidValueIter = uidValueMap.begin();uidValueIter != uidValueMap.end();uidValueIter++)
    {
        UserL *r = userhead;
        UserL *s = userhead->next;
        UserL *p = (struct UserL *)malloc(sizeof(struct UserL));
        p->uid = uidValueIter->first;
        p->value = uidValueIter->second;
        float value = uidValueIter->second;
        while(s!=NULL)
        {
                if(s->value > p->value)
                {
                    r->next = p;
                    p->next = s;
                    break;
                }
                r=s;
                s=s->next;
        }
        if(s == NULL)
        {
            r->next = p;
            p->next = s;
        }
    }
    uidValueMap.clear();
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"SortUser End");
    return true;
}

bool  Pruning::PruningUserGraph(map<long int,UserLink> *userGraphMap,UserLLink userhead)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"PruningUserGraph Start");
    map<long int,UserLink>::iterator userGraphIter;
    map<long int,UserLink>::iterator userGraphIter2;
    int i = 0;
    UserL *p = userhead->next;
    while(i<PruningCount && p!=NULL)
    {
        userGraphIter = userGraphMap->find(p->uid);
        if(userGraphIter != userGraphMap->end())
        {
            UserLink userHead = userGraphIter->second;
            UserNode *q = userHead->next;
            while(q !=NULL)
            {
                userGraphIter2 = userGraphMap->find(q->uid);
                if(userGraphIter2 != userGraphMap->end() && q->uid != p->uid)
                {
                    UserNode *p1 = userGraphIter2->second;
                    UserNode *q1 = p1->next;
                    while(q1 != NULL)
                    {
                        if(q1->uid == p->uid)
                        {
                            p1->next = q1->next;
                            free(q1);
                            break;
                        }
                        p1 = q1;
                        q1 = q1->next;
                    }
                }
                userHead->next = q->next;
                free(q);
                q = userHead->next;
            }
            userGraphMap->erase(userGraphIter);
        }
        p = p->next;
        i++;
    }

    //Free Userhead
    p = userhead;
    UserL *q = p->next;
    while(q!=NULL)
    {
        free(p);
        p=q;
        q=q->next;
    }
    free(p);
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"PruningUserGraph End");
    return true;
}

bool  Pruning::FreeUserTopicValueMap(map<int,UserTopicValueLink> &widUserTopicMap)
{
    map<int,UserTopicValueLink>::iterator widUserTopicIter;
    for(widUserTopicIter = widUserTopicMap.begin();widUserTopicIter!=widUserTopicMap.end();widUserTopicIter++)
    {
        UserTopicValueLink p = widUserTopicIter->second;
        UserTopicValue *q = p->next;
        while(q != NULL)
        {
            free(p);
            p =q;
            q = p->next;
        }
        free(p);
    }
    widUserTopicMap.clear();
    return true;
}

//随机选机选择修剪结点
bool  Pruning::PruningMethod1()
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod1 start");
    UserLLink userhead = (struct UserL *)malloc(sizeof(struct UserL));
    userhead->next = NULL;

    if(!RandomSortUser(userhead))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"RandomSortUser fail");
        return false;
    }

    if(!PruningUserGraph(userGraphMap,userhead))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"PruningUserGraph fail");
        return false;
    }

    if(!StatisticUserInfo(userGraphMap))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"StatisticUserInfo fail");
        return false;
    }
    //进行话题传播
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),"start time:"<<TimeUtil::GetInstance()->LocalTimeStr());
    if(!TopicDiffuse())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"TopicDiffuse fail");
        return false;
    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),"end time:"<<TimeUtil::GetInstance()->LocalTimeStr());
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod1 end");
    return true;
}

//对用户进行随机排序
bool Pruning::RandomSortUser(UserLLink userhead)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"RandomSortUser Start");
    map<long,UserLink>::iterator userGraphIter;
    int userGraphSize = userGraphMap->size();
    int pos = 0;
    long *uidArray = new long[userGraphSize];
    for(unsigned int i=0;i<userGraphSize;i++)
        uidArray[i] = -1;
    for(userGraphIter = userGraphMap->begin();userGraphIter != userGraphMap->end();userGraphIter++)
    {
        int increment = 0;
        pos = rand()%userGraphSize;
        while(uidArray[pos] != -1)
        {
             pos = (pos+increment)%userGraphSize;
             increment++;
        }
        uidArray[pos] = userGraphIter->first;
    }
    for(unsigned int i=0;i<userGraphSize;i++)
    {
        UserL *p = (struct UserL *)malloc(sizeof(struct UserL));
        p->uid = uidArray[i];
        p->next = userhead->next;
        userhead->next = p;
    }
    delete []uidArray;
     LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"RandomSortUser End");
    return true;
}

//根据度数大的点来进行图的剪枝
bool Pruning::PruningMethod2()
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod2 start");
    UserLLink userhead = (struct UserL *)malloc(sizeof(struct UserL));
    userhead->next = NULL;

    if(!SortUserByDgree(userhead,true))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"RandomSortUser fail");
        return false;
    }

    if(!PruningUserGraph(userGraphMap,userhead))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"PruningUserGraph fail");
        return false;
    }

    if(!StatisticUserInfo(userGraphMap))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"StatisticUserInfo fail");
        return false;
    }
    //进行话题传播
    if(!TopicDiffuse())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"TopicDiffuse fail");
        return false;
    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod2 end");
    return true;
}

//根据度数小的点来进行图的剪枝
bool Pruning::PruningMethod3()
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod3 start");
    UserLLink userhead = (struct UserL *)malloc(sizeof(struct UserL));
    userhead->next = NULL;

    if(!SortUserByDgree(userhead,false))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"RandomSortUser fail");
        return false;
    }

    if(!PruningUserGraph(userGraphMap,userhead))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"PruningUserGraph fail");
        return false;
    }

    if(!StatisticUserInfo(userGraphMap))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"StatisticUserInfo fail");
        return false;
    }
    //进行话题传播
    if(!TopicDiffuse())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"TopicDiffuse fail");
        return false;
    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod3 end");
    return true;
}

//根据度的大小来进行排序
bool Pruning::SortUserByDgree(UserLLink userhead,bool BigSign)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"SortUserByDgree Start");
    map<long,UserLink>::iterator userGraphIter;
    map<long,int> uidDegreeMap;
    map<long,int>::iterator uidDegreeIter;
    for(userGraphIter = userGraphMap->begin();userGraphIter != userGraphMap->end();userGraphIter++)
    {
        int degree = 0;
        UserNode *p = userGraphIter->second->next;
        while(p!=NULL)
        {
            degree++;
            p = p->next;
        }
        uidDegreeMap[userGraphIter->first] = degree;
    }
    if(BigSign == false)
    {
        for(uidDegreeIter = uidDegreeMap.begin();uidDegreeIter != uidDegreeMap.end();uidDegreeIter++)
        {
             UserL *r = userhead;
             UserL *s = userhead->next;
             UserL *p = (struct UserL *)malloc(sizeof(struct UserL));
             p->uid = uidDegreeIter->first ;
             p->value = uidDegreeIter->second;
             while(s!=NULL)
             {
                if(s->value > p->value)
                {
                    r->next = p;
                    p->next = s;
                    break;
                }
                r=s;
                s=s->next;
             }
             if(s == NULL)
             {
                r->next = p;
                p->next = s;
             }
        }
    }
    else
    {
        for(uidDegreeIter = uidDegreeMap.begin();uidDegreeIter != uidDegreeMap.end();uidDegreeIter++)
        {
             UserL *r = userhead;
             UserL *s = userhead->next;
             UserL *p = (struct UserL *)malloc(sizeof(struct UserL));
             p->uid = uidDegreeIter->first ;
             p->value = uidDegreeIter->second;
             while(s!=NULL)
             {
                if(s->value < p->value)
                {
                    r->next = p;
                    p->next = s;
                    break;
                }
                r=s;
                s=s->next;
             }
             if(s == NULL)
             {
                r->next = p;
                p->next = s;
             }
        }

    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"SortUserByDgree End");
    return true;
}

bool  Pruning::PruningMethod5()
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod5 start");
    map<long,WordTfIdfLink> uidWordTopicMap;
    if(!InitWordTopicValueMap(uidWordTopicMap))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"InitWordTopicValueMap fail");
    }

    UserLLink userhead = (struct UserL *)malloc(sizeof(struct UserL));
    userhead->next = NULL;
    /*
    if(!SortUserByInterest(uidWordTopicMap,userhead))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"SortUserByInterest fail");
    }

    if(!InsertUserHead(userhead))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"InsertUserHead fail");
    }
    */

    if(!QueryUserHead(userhead))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"QueryUserHead fail");
    }
    if(!FreeWordTopicValueMap(uidWordTopicMap))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"FreeWordTopicValueMap fail");
    }

    if(!PruningUserGraph(userGraphMap,userhead))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"PruningUserGraph fail");
        return false;
    }

    if(!StatisticUserInfo(userGraphMap))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"StatisticUserInfo fail");
        return false;
    }

    if(!TopicDiffuse())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"TopicDiffuse fail");
        return false;
    }

    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"PruningMethod5 end");
    return true;
}

bool  Pruning::InitWordTopicValueMap(map<long,WordTfIdfLink> &uidWordTopicMap)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"InitWordTopicValueMap start");
    map<int,StatusTagLink>::iterator iter;
    map<long,WordTfIdfLink>::iterator uidWordTopicIter;
    map<long int,UserLink>::iterator userGraphIter;
    for(userGraphIter = userGraphMap->begin();userGraphIter != userGraphMap->end();userGraphIter++)
    {
        WordTfIdfLink head = (struct WordTfIdfNode *)malloc(sizeof(struct WordTfIdfNode));
        head->next = NULL;
        uidWordTopicMap[userGraphIter->first] = head;
    }

    float total = 0.0;
    int record = 0;
    for(unsigned int i=0;i<keyWordList.size();i++)
    {
        //针对每一个单词，循环查找每一个单词，它的概率
        map<long,UserStatus> userSMap;
        map<long,UserStatus>::iterator userSMapIter;
        int wid = keyWordList[i]; //获得keyword的id
        iter = widSTMap->find(wid);
        if(iter == widSTMap->end())
            continue;
        Queue<long> useridQueue;
        StatusTagNode *p = iter->second->next;
        BuildQueue(useridQueue,userSMap,p,wid);
        ModelSpread(useridQueue,userSMap);
        for(userSMapIter=userSMap.begin();userSMapIter!=userSMap.end();userSMapIter++)
        {
            WordTfIdfNode *uidWordTopicNode = (struct WordTfIdfNode *)malloc(sizeof(struct WordTfIdfNode));
            uidWordTopicNode->wid = keyWordList[i];
            uidWordTopicNode->tfIdf = userSMapIter->second.tfIdf;
            uidWordTopicIter = uidWordTopicMap.find(userSMapIter->first);
            if(uidWordTopicIter != uidWordTopicMap.end())
            {
                WordTfIdfLink head = uidWordTopicIter->second;
                uidWordTopicNode->next = head->next;
                head->next = uidWordTopicNode;
            }
        }
        userSMap.clear();
    }

    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"InitWordTopicValueMap End");
    return true;
}

bool  Pruning::SortUserByInterest(map<long,WordTfIdfLink> &uidWordTopicMap,UserLLink userhead)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"SortUserByInterest Start");
    map<long,UserLLink> userInterestMap;
    map<long,UserLLink>::iterator userInterestIter;
    map<long,UserLLink>::iterator userInterestIter2;
    map<long,WordTfIdfLink>::iterator uidWordTopicIter;
    map<long,WordTfIdfLink>::iterator uidWordTopicIter2;
    int uidCount =0;
    for(uidWordTopicIter = uidWordTopicMap.begin();uidWordTopicIter != uidWordTopicMap.end();uidWordTopicIter++)
    {
        uidCount++;
        cout<<"uidCount:"<<uidCount<<endl;
        for(uidWordTopicIter2 = uidWordTopicMap.begin();uidWordTopicIter2 != uidWordTopicMap.end();uidWordTopicIter2++)
        {
            long uid1 = uidWordTopicIter->first;
            long uid2 = uidWordTopicIter2->first;
            if(uid1 == uid2)
            {
                 continue;
            }

            userInterestIter = userInterestMap.find(uid1);
            if(userInterestIter != userInterestMap.end())
            {
                UserL *p = userInterestIter->second->next;
                while(p!=NULL)
                {
                    if(p->uid == uid2)
                        break;
                    p=p->next;
                }
                if(p!=NULL)
                {
                    continue;
                }
                else
                {
                    //进行相似性计算
                    float interestValue = CalcuateInterest(uidWordTopicIter->second,uidWordTopicIter2->second);
                    if(interestValue == 0.0)
                    {
                        continue;
                    }
                    UserL *userLNode1 = (struct UserL *)malloc(sizeof(struct UserL));
                    userLNode1->uid = uid2;
                    userLNode1->value = interestValue;
                    userLNode1->next = userInterestIter->second->next;
                    userInterestIter->second->next = userLNode1;
                    userInterestIter2 = userInterestMap.find(uid2);
                    if(userInterestIter2 != userInterestMap.end())
                    {
                        UserL *userLNode2 = (struct UserL *)malloc(sizeof(struct UserL));
                        userLNode2->uid = uid1;
                        userLNode2->value = interestValue;
                        userLNode2->next = userInterestIter2->second->next;
                        userInterestIter2->second->next = userLNode2;
                    }
                    else
                    {
                        UserLLink head = (struct UserL *)malloc(sizeof(struct UserL));
                        head->next = NULL;
                        userInterestMap[uid2] = head;

                        UserL *userLNode2 = (struct UserL *)malloc(sizeof(struct UserL));
                        userLNode2->uid = uid1;
                        userLNode2->value = interestValue;
                        userLNode2->next = head->next;
                        head->next = userLNode2;
                    }
                }
            }
            else
            {
               //进行相似性计算
                float interestValue = CalcuateInterest(uidWordTopicIter->second,uidWordTopicIter2->second);
                UserLLink head = (struct UserL *)malloc(sizeof(struct UserL));
                head->next = NULL;
                userInterestMap[uid1] = head;

                if(interestValue == 0.0)
                {
                    continue;
                }
                UserL *userLNode1 = (struct UserL *)malloc(sizeof(struct UserL));
                userLNode1->uid = uid2;
                userLNode1->value = interestValue;
                userLNode1->next = head->next;
                head->next = userLNode1;
                userInterestIter2 = userInterestMap.find(uid2);
                if(userInterestIter2 != userInterestMap.end())
                {
                    UserL *userLNode2 = (struct UserL *)malloc(sizeof(struct UserL));
                    userLNode2->uid = uid1;
                    userLNode2->value = interestValue;
                    userLNode2->next = userInterestIter2->second->next;
                    userInterestIter2->second->next = userLNode2;
                }
                else
                {
                    UserLLink head = (struct UserL *)malloc(sizeof(struct UserL));
                    head->next = NULL;
                    userInterestMap[uid2] = head;

                    UserL *userLNode2 = (struct UserL *)malloc(sizeof(struct UserL));
                    userLNode2->uid = uid1;
                    userLNode2->value = interestValue;
                    userLNode2->next = head->next;
                    head->next = userLNode2;
                }

            }

        }
    }

    //每一个用户进行的相似性进行排序
    if(!SortUserLink(userInterestMap))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"SortUserLink fail");
    }

    if(!SortUserII(userInterestMap,userhead))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"SortUserII fail");
    }

    /*
    UserL *test = userhead->next;
    while(test != NULL)
    {
        cout<<"id:"<<test->uid<<" value:"<<test->value<<endl;
        test = test->next;
    }
    */
/*
    for(userInterestIter=userInterestMap.begin();userInterestIter!=userInterestMap.end();userInterestIter++)
    {
        cout<<"src uid:"<<userInterestIter->first<<endl;
        UserL *p = userInterestIter->second->next;
        cout<<"dst uid:";
        while(p)
        {
            cout<<" "<<p->value;
            p = p->next;
        }
        cout<<endl;
    }
*/
    if(!FreeUserInterestMap(userInterestMap))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"FreeUserInterestMap fail");
    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"SortUserByInterest End");
    return true;
}

float  Pruning::CalcuateInterest(WordTfIdfLink head1,WordTfIdfLink head2)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"CalcuateInterest Start");
    WordTfIdfNode *p1 = head1->next;
    WordTfIdfNode *p2 = head2->next;
    map<int,int> wordMap;
    map<int,int>::iterator wordIter;
    int commonCount = 0;
    float degree1 = 0.0;
    float degree2 = 0.0;

    while(p1!=NULL)
    {
        wordMap[p1->wid] = 1;
        degree1++;
        p1 = p1->next;
    }

    while(p2!=NULL)
    {
        wordIter = wordMap.find(p2->wid);
        if(wordIter != wordMap.end())
            commonCount++;
        degree2++;
        p2 = p2->next;
    }
    if(degree1 == 0 || degree2 == 0 || commonCount == 0)
        return 0;
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"CalcuateInterest End");
    return commonCount/sqrt(degree1*degree2);
}

bool Pruning::SortUserLink(map<long,UserLLink> &userInterestMap)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"SortUserLink Start");
    map<long,UserLLink>::iterator userInterestIter;
    for(userInterestIter = userInterestMap.begin();userInterestIter != userInterestMap.end();userInterestIter++)
    {
        UserLLink head = userInterestIter->second;
        UserL *p = head->next;
        if(p == NULL)
            continue;
        UserL *q = p->next;
        head->next = NULL;
        while(p!=NULL)
        {
            UserL *r = head;
            UserL *s = head->next;
            while(s!=NULL)
            {
                if(s->value < p->value)
                {
                    r->next = p;
                    p->next = s;
                    break;
                }
                r=s;
                s=s->next;
            }
            if(s == NULL)
            {
                r->next = p;
                p->next = s;
            }
            p = q;
            if(q!=NULL)
                q = q->next;
        }
    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"SortUserLink End");
    return true;
}

bool Pruning::SortUserII(map<long,UserLLink> &userInterestMap,UserLLink userhead)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"SortUserII start");
    int topN = 100;
    map<long,float> uidValueMap;
    map<long,float>::iterator uidValueIter;
    map<long,UserLLink>::iterator userInterestIter;
    for(userInterestIter = userInterestMap.begin();userInterestIter != userInterestMap.end();userInterestIter++)
    {
        int record = 0;
        uidValueIter = uidValueMap.find(userInterestIter->first);
        if(uidValueIter == uidValueMap.end())
        {
            uidValueMap[userInterestIter->first] = 0;
        }
        UserL *p = userInterestIter->second->next;
        while(p)
        {
            uidValueIter = uidValueMap.find(p->uid);
            if(uidValueIter != uidValueMap.end())
                uidValueIter->second += 1;
            else
                uidValueMap[p->uid] = 1;
            p = p->next;
            record++;
            if(record == topN)
                break;
        }
    }


    for(uidValueIter = uidValueMap.begin();uidValueIter != uidValueMap.end();uidValueIter++)
    {
        UserL *r = userhead;
        UserL *s = userhead->next;
        UserL *p = (struct UserL *)malloc(sizeof(struct UserL));
        p->uid = uidValueIter->first;
        p->value = uidValueIter->second;
        float value = uidValueIter->second;
        while(s!=NULL)
        {
                if(s->value >= p->value)
                {
                    r->next = p;
                    p->next = s;
                    break;
                }
                r=s;
                s=s->next;
        }
        if(s == NULL)
        {
            r->next = p;
            p->next = s;
        }
    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"SortUserII end");
    return true;
}

bool Pruning::InsertUserHead(UserLLink userhead)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"InsertUserHead start");
    string query = "insert into userHead (uid,value) values(";
    UserL *p = userhead->next;
    while(p != NULL)
    {
        long uid = p->uid;
        float value = p->value;
        string query2 = query + StrUtil::GetInstance()->Int2String(uid) + ",'";
        query2 += StrUtil::GetInstance()->float2String(value) + "')";
        if(DbUtil::GetInstance()->DbInsert(query2))
        {
            LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                            ,"DbInsert fail");
        }

        p = p->next;
    }

    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"InsertUserHead End");
    return true;
}

bool Pruning::QueryUserHead(UserLLink userhead)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"QueryUserHead start");
    vector<PrunUser> prunUserV;
    string query = "select uid,value from userHead order by value desc";
    if(DbUtil::GetInstance()->DbQueryPrunUser(query,prunUserV))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbQueryPrunUser fail");
        return false;
    }

    for(unsigned int i=0;i<prunUserV.size();i++)
    {
        UserL *q = (struct UserL *)malloc(sizeof(UserL));
        q->uid = prunUserV[i].uid;
        q->value = prunUserV[i].value;
        q->next = userhead->next;
        userhead->next = q;
    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"QueryUserHead end");
    return true;
}

bool Pruning::FreeWordTopicValueMap(map<long,WordTfIdfLink> &uidWordTopicMap)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"FreeWordTopicValueMap start");
    map<long,WordTfIdfLink>::iterator uidWordTopicIter;
    for(uidWordTopicIter = uidWordTopicMap.begin();uidWordTopicIter!=uidWordTopicMap.end();uidWordTopicIter++)
    {
        WordTfIdfLink p = uidWordTopicIter->second;
        WordTfIdfNode *q = p->next;
        while(q != NULL)
        {
            free(p);
            p =q;
            q = p->next;
        }
        free(p);
    }
    uidWordTopicMap.clear();
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"FreeWordTopicValueMap end");
    return true;
}

bool Pruning::FreeUserInterestMap(map<long,UserLLink> &userInterestMap)
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"FreeUserInterestMap Start");
    map<long,UserLLink>::iterator userInterestIter;
    for(userInterestIter = userInterestMap.begin();userInterestIter!=userInterestMap.end();userInterestIter++)
    {
        UserLLink p = userInterestIter->second;
        UserL *q = p->next;
        while(q != NULL)
        {
            free(p);
            p =q;
            q = p->next;
        }
        free(p);
    }
    userInterestMap.clear();
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                        ,"FreeUserInterestMap End");
    return true;
}

bool  Pruning::StatisticUserInfo(map<long int,UserLink> *userGraphMap) //统计用户数量
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),"StatisticUserInfo Start");
    int userCount = 0;
    float aveDgree = 0.0;
    map<long int,UserLink>::iterator iter;
    userCount = userGraphMap->size();
    float totalDegree = 0;
    UserLink userLink;
    for(iter = userGraphMap->begin();iter != userGraphMap->end();iter++)
    {
        userLink = iter->second->next;
        while(userLink !=NULL)
        {
            totalDegree++;
            userLink = userLink->next;
        }
    }
    if(userCount==0)
        aveDgree = 0;
    else
        aveDgree = totalDegree/userCount;

     //统计用户数
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),userCount);
    //统计用户平均度数
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),aveDgree);
    //统计话题词个数
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),keyWordList.size());

    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),"StatisticUserInfo End");
    return true;
}

void Pruning::Show(){
}

void Pruning::Test(map<int,UserTopicValueLink> &widUserTopicMap){
    map<int,UserTopicValueLink>::iterator widUserTopicIter;
    int record = 0;
    for(widUserTopicIter=widUserTopicMap.begin();widUserTopicIter!=widUserTopicMap.end();widUserTopicIter++)
    {
        cout<<"wid:"<<widUserTopicIter->first<<endl;
        UserTopicValueLink head = widUserTopicIter->second;
        head = head->next;
        cout<<"Link:";
        while(head != NULL)
        {
            cout<<head->value<<" ";
            head = head->next;
        }
        cout<<endl;
        record++;
        if(record == 2)
            break;
    }
}

Pruning::~Pruning()
{
    userGraphMap = NULL;
    sidWTIMap = NULL;
    widSTMap = NULL;
    if(Pruning::_pInstance)
        delete Pruning::_pInstance;
}
