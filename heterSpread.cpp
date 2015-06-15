#include "heterSpread.h"
HeterSpread* HeterSpread::_pInstance = new HeterSpread();

HeterSpread::HeterSpread()
{
    heterGraphMap = NULL;
    sidWTIMap = NULL;
    widSTMap = NULL;
    keyWordList.clear();
    hotWordList.clear();
    keyWordMap.clear();
    hotWordMap.clear();
    f1 = 0.0;
    C = 50; //关键词
    C1 = 100;  //热点话题词
    mu = 0.1; //传播阈值
    tao = 24; //时间粒度
    xipuxiluo = 8; //1/5
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"HeterSpread Init");
}

HeterSpread* HeterSpread::GetInstance()
{
    if(_pInstance == NULL)
        _pInstance = new HeterSpread();
    return _pInstance;
}

bool HeterSpread::GetKeyWordMap(const string &startTime,const string &endTime)
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

bool HeterSpread::GetHotWordMap(const string &startTime,const string &endTime)
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

bool HeterSpread::BuildKeyHotWordMap(const string &startTime,const string &endTime)
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

bool HeterSpread::CalculateF1()
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
    }
    else
    {
        LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),0<<"\t"<<0<<"\t"<<0);
    }


    return true;
}

bool HeterSpread::Init(){
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"HeterSpread Init");
    //进行话题初始化，计算的是2009-10-01之前的话题数据
    sidWTIMap = TopicDiscover::GetInstance()->GetSidWTIMap();
    heterGraphMap = BuildHeterGraph::GetInstance()->GetHeterGraphMap();
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
    LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,TimeUtil::GetInstance()->LocalTimeStr());
    //统计40天当中，话题传播
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
                if(!DynamicHeterSpread(startTime,midTime,endTime))
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
                if(!BuildHeterGraph::GetInstance()->AddHeterGraph(startTime,endTime))
                {
                    LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,"AddHeterGraph fail");
                    return false;
                }
            }
        }
    }
    LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,TimeUtil::GetInstance()->LocalTimeStr());
    return true;
}

bool HeterSpread::DynamicHeterSpread(const string &startTime,const string &midTime,const string &endTime)
{
    if(!BuildHeterGraph::GetInstance()->AddHeterGraph(startTime,midTime))
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
    heterGraphMap = BuildHeterGraph::GetInstance()->GetHeterGraphMap();
    if(!HeterTopicDiffuse())
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"HeterTopicDiffuse fail");
        return false;
    }
    return true;
}




bool HeterSpread::HeterTopicDiffuse()
{
    map<int,StatusTagLink>::iterator iter; //widSTMap
    map<long,WordTfIdfLink>::iterator iter2; //sidWTIMap
    map<long,HeterStatus>::iterator iterH;
    map<long,HeterStatus> heterSMap;
    map<long,HeterStatus>::iterator heterSIter;
    StatusTagNode *p;

    float totalTopicValue;
    int record=0;
    //循环遍历每一个候选话题词，进行TMBP模拟
    for(unsigned int i=0;i<keyWordList.size();i++)
    {
        heterSMap.clear();
        iter = widSTMap->find(keyWordList[i]);
        if(iter == widSTMap->end())
            continue;
        p = iter->second;

        //初始化heterSMap
        while(p!=NULL)
        {
            iter2 = sidWTIMap->find(p->sid);
            if(iter2 != sidWTIMap->end())
            {
                WordTfIdfNode *q;
                q = iter2->second;
                while(q!=NULL)
                {
                    if(q->wid == keyWordList[i])
                        break;
                    q = q->next;
                }
                if(q!=NULL)
                {
                    HeterStatus hs;
                    hs.level = 0;
                    hs.topicValue[0] = q->tfIdf;
                    hs.topicValue[1] = q->tfIdf;
                    hs.infected = false;
                    heterSMap[p->sid] = hs;
                }
            }
            p = p->next;
        }

        //开始进行传播计算
        int level = 0;
        float bRValue[2] = {100001,100000}; //设置成为无穷大
        //cout<<"bRValue[0]"<<bRValue[0]<<endl;
        //cout<<"bRValue[1]"<<bRValue[1]<<endl;
        while(bRValue[0] > bRValue[1])
        {
            TMBP(heterSMap,level);
            bRValue[0] = bRValue[1];
            bRValue[1] = BiasedRegularization(heterSMap,level);
            level++;
        }

        float topicValue = 0.0;
        for(heterSIter = heterSMap.begin();heterSIter != heterSMap.end();heterSIter++)
        {
            topicValue += heterSIter->second.topicValue[(level-1)%2];
        }
        keyWordMap[keyWordList[i]]=topicValue;
    }
    return true;
}

bool HeterSpread::TMBP(map<long,HeterStatus> &heterSMap,const int &level)
{
    map<long,HeterStatus>::iterator iter;
    map<long,HeterStatus>::iterator iterL;
    map<long int,HeterLink>::iterator iter2;
    TmbpLink tmbpHead = (struct TmbpNode*)malloc(sizeof(TmbpNode));
    tmbpHead->next = NULL;
    for(iter = heterSMap.begin();iter != heterSMap.end();iter++)
    {
        long id = iter->first; //传播的id
        float topicValue = iter->second.topicValue[level%2];      //话题值
        iter2 = heterGraphMap->find(id);
        if(iter2 != heterGraphMap->end())
        {
            HeterNode *p = iter2->second;
            if(p->isUser == true)
            {
                //循环遍历图的相关结点
                while(p!=NULL)
                {
                    iter2 = heterGraphMap->find(p->id);
                    int sidCount = iter2->second->nodeCount;
                    iterL = heterSMap.find(p->id);
                    if(iterL != heterSMap.end())
                    {
                        if(iterL->second.infected == false)
                        {
                            iterL->second.topicValue[(level+1)%2] = topicValue/sidCount;
                            iterL->second.infected = true;
                        }
                        else
                        {
                            iterL->second.topicValue[(level+1)%2] += topicValue/sidCount;
                        }
                    }
                    else
                    {
                        //还未被感染的
                        TmbpNode *q = (struct TmbpNode*)malloc(sizeof(TmbpNode));
                        q->id = p->id;
                        q->topicValue = topicValue/sidCount;
                        q->next = tmbpHead->next;
                        tmbpHead->next = q;
                    }
                    p = p->next;
                }
            }
            else
            {
                //循环遍历图的相关结点
                while(p!=NULL)
                {
                    iter2 = heterGraphMap->find(p->id);
                    int uidCount = iter2->second->nodeCount;
                    iterL = heterSMap.find(p->id);
                    if(iterL != heterSMap.end())
                    {
                        if(iterL->second.infected == false)
                        {
                            iterL->second.topicValue[(level+1)%2] = iterL->second.topicValue[level%2]/2+topicValue/(uidCount*2); //参数
                            iterL->second.infected = true;
                        }
                        else
                        {
                            iterL->second.topicValue[(level+1)%2] += topicValue/(uidCount*2);
                        }
                    }
                    else
                    {
                        //还未被感染的
                        TmbpNode *q = (struct TmbpNode*)malloc(sizeof(TmbpNode));
                        q->id = p->id;
                        q->topicValue = topicValue/uidCount;
                        q->next = tmbpHead->next;
                        tmbpHead->next = q;
                    }
                    p = p->next;
                }
            }
        }
    }

    //将内容写进字典中
    TmbpNode *p = tmbpHead;
    TmbpNode *q = p->next;
    while(q!=NULL)
    {
        HeterStatus heterS;
        heterS.topicValue[(level+1)%2] = q->topicValue;
        heterS.topicValue[level%2] = 0;
        heterSMap[q->id] = heterS;
        delete p;
        p=q;
        q=q->next;
    }
    delete p;
    for(iter = heterSMap.begin();iter != heterSMap.end();iter++)
    {
        iter->second.level = level+1;
        iter->second.infected = false;
    }
    return true;
}

float HeterSpread::BiasedRegularization(map<long,HeterStatus> &heterSMap,const int &level)
{
    map<long,HeterStatus>::iterator iterHeterS;
    map<long,HeterStatus>::iterator iterHeterS2;
    map<long int,HeterLink>::iterator iterHeterL;
    float bRValue = 0;
    for(iterHeterS = heterSMap.begin();iterHeterS != heterSMap.end();iterHeterS++)
    {
        long id = iterHeterS->first;
        float topicValue = iterHeterS->second.topicValue[(level+1)%2];
        iterHeterL = heterGraphMap->find(id);
        if(iterHeterL != heterGraphMap->end())
        {
            HeterNode *p = iterHeterL->second;
            int idCount = p->nodeCount; // 当isUser是true时，代表用户数；当isUser是false时，代表微博数
            float a=0;
            if(p->isUser == true)
            {
                while(p!=NULL)
                {
                    iterHeterS2 = heterSMap.find(p->id);
                    if(iterHeterS2 != heterSMap.end())
                        a += iterHeterS2->second.topicValue[(level+1)%2]/idCount;
                    p = p->next;
                }
                bRValue += (1/2)*pow(topicValue-a,2);
            }
            else
            {
                while(p!=NULL)
                {
                    iterHeterS2 = heterSMap.find(p->id);
                    if(iterHeterS2 != heterSMap.end())
                        a += iterHeterS2->second.topicValue[(level+1)%2]/idCount;
                    p = p->next;
                }
                bRValue += (1/2)*pow(topicValue-a,2);
            }
        }
    }
    return bRValue;
}

HeterSpread::~HeterSpread()
{
    heterGraphMap = NULL;
    sidWTIMap = NULL;
    widSTMap = NULL;
    if(HeterSpread::_pInstance)
        delete HeterSpread::_pInstance;
}
