#include "spreadEx.h"
SpreadEx* SpreadEx::_pInstance = new SpreadEx();

SpreadEx::SpreadEx()
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
    mu = 0.001; //传播阈值
    tao = 6; //时间粒度
    xipuxiluo = 5; //1/5
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"SpreadEx Init");

}

SpreadEx* SpreadEx::GetInstance()
{
    if(_pInstance == NULL)
        _pInstance = new SpreadEx();
    return _pInstance;
}

bool SpreadEx::GetKeyWordMap(const string &startTime,const string &endTime)
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
            //keyWordMap[widCountiter->first] = widCountiter->second;
            keyWordList.push_back(widCountiter->first);
        }
    }
    return true;
}

bool SpreadEx::GetHotWordMap(const string &startTime,const string &endTime)
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

bool SpreadEx::BuildKeyHotWordMap(const string &startTime,const string &endTime)
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

bool SpreadEx::CalculateF1()
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
        LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),f1);
    }
    else
    {
        LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger(),0);
    }


    return true;
}

float SpreadEx::DiffuseRate(const int &srcWeight,const int &dstWeight,const int &connect)
{
    //xipuxiluo 也是一个总要参数
    float diffuseRate = float(srcWeight)/(srcWeight+dstWeight)/exp(1/double(connect))/xipuxiluo;
    //float diffuseRate = float(srcWeight)/(srcWeight+dstWeight)/pow(4,1/double(connect))/20;
    //float diffuseRate = float(srcWeight)/(srcWeight+dstWeight);
    //cout<<"diffuseRate:"<<diffuseRate<<endl;
    return diffuseRate;
}

bool SpreadEx::ShowUserSMap(map<long,UserStatus> &userSMap)
{
    map<long,UserStatus>::iterator iter;
    for(iter = userSMap.begin();iter != userSMap.end();iter++){
            cout<<"userid:"<<iter->first;
            cout<<" "<<"tdIdf:"<<iter->second.tfIdf<<endl;
    }
    return true;
}

//初始化MutilSpreadMap
bool SpreadEx::InitMutilSpreadMap(map<long,map<int,WordInfo> > &mutilSpreadMap)
{
    unsigned int i =0;
    map<int,StatusTagLink>::iterator widSTIter;
    map<long,WordTfIdfLink>::iterator sidWTIIter;
    map<long,map<int,WordInfo> >::iterator mutilSpreadIter;
    map<int,WordInfo>::iterator wordInfoIter;

    for(i=0;i<keyWordList.size();i++)
    {
        int wid = keyWordList[i];
        widSTIter = widSTMap->find(wid);
        if(widSTIter != widSTMap->end())
        {
            StatusTagNode *p = widSTIter->second->next;
            while(p!=NULL)
            {
                long userid = p->userid;
                float tfIdf;
                sidWTIIter = sidWTIMap->find(p->sid);
                if(sidWTIIter != sidWTIMap->end())
                {
                    WordTfIdfNode *q = sidWTIIter->second->next;
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
                mutilSpreadIter = mutilSpreadMap.find(userid);
                if(mutilSpreadIter != mutilSpreadMap.end())
                {
                    wordInfoIter = mutilSpreadIter->second.find(wid);
                    if(wordInfoIter != mutilSpreadIter->second.end())
                    {
                        wordInfoIter->second.tfIdf += tfIdf;
                    }
                    else
                    {
                        WordInfo wordInfo;
                        wordInfo.infectStatus = 1; // 1表示作为传播源状态
                        wordInfo.tfIdf = tfIdf;
                        mutilSpreadIter->second[wid] = wordInfo;
                    }
                }
                else
                {
                    WordInfo wordInfo;
                    wordInfo.infectStatus = 1;// 1表示作为传播源状态
                    wordInfo.tfIdf = tfIdf;
                    map<int,WordInfo> wordInfoMap;
                    wordInfoMap[wid] = wordInfo;
                    mutilSpreadMap[userid] = wordInfoMap;
                }
                p = p->next;
            }
        }
    }
    return true;
}

bool SpreadEx::ModelMutilSpreadMap(map<long,map<int,WordInfo> > &mutilSpreadMap)
{
    map<long,map<int,WordInfo> >::iterator iterA1;
    map<long,map<int,WordInfo> >::iterator iterA2;
    map<long,map<int,WordInfo> > dacheSpreadMap;
    map<int,WordInfo>::iterator iterB;
    map<int,WordInfo>::iterator iterB2;
    map<long int,UserLink>::iterator iterC;
    bool sign = true;
while(sign)
{
    for(iterA1 = mutilSpreadMap.begin();iterA1 != mutilSpreadMap.end();iterA1++)
    {
        long userid = iterA1->first;
        iterC = userGraphMap->find(userid);
        if(iterC != userGraphMap->end())
        {
            UserLink head = iterC->second;
            float srcWeight = head->weight;
            float dstWeight;
            int connect;
            UserNode *g = head->next;
            while(g!=NULL)
            {
                dstWeight = g->weight;
                connect = g->connect;
                long userid2 = g->uid; //
                float diffuseRate = DiffuseRate(srcWeight,dstWeight,connect);
                for(iterB = iterA1->second.begin();iterB != iterA1->second.end();iterB++)
                {
                    int wid = iterB->first;
                    if(iterB->second.infectStatus == 1 && iterB->second.tfIdf >= mu)
                    {
                        iterA2  = mutilSpreadMap.find(userid2);
                        if(iterA2 != mutilSpreadMap.end())
                        {
                            iterB2 = iterA2->second.find(wid);
                            if(iterB2 != iterA2->second.end() && iterB2->second.infectStatus == 0)
                            {
                                iterB2->second.tfIdf += iterB->second.tfIdf * diffuseRate;
                            }
                            else
                            {
                                WordInfo wordInfo;
                                wordInfo.infectStatus = 0;
                                wordInfo.tfIdf = iterB->second.tfIdf * diffuseRate;
                                iterA2->second[wid] = wordInfo;
                            }
                        }
                        else
                        {
                            iterA2 = dacheSpreadMap.find(userid2);
                            if(iterA2 != dacheSpreadMap.end())
                            {
                                iterB2 = iterA2->second.find(wid);
                                if(iterB2 != iterA2->second.end() && iterB2->second.infectStatus == 0)
                                {
                                    iterB2->second.tfIdf += iterB->second.tfIdf * diffuseRate;
                                }
                                else
                                {
                                    WordInfo wordInfo;
                                    wordInfo.infectStatus = 0;
                                    wordInfo.tfIdf = iterB->second.tfIdf * diffuseRate;
                                    iterA2->second[wid] = wordInfo;
                                }
                            }
                            else
                            {
                                WordInfo wordInfo;
                                wordInfo.infectStatus = 0;
                                wordInfo.tfIdf = iterB->second.tfIdf * diffuseRate;
                                map<int,WordInfo> wordInfoMap;
                                wordInfoMap[wid] = wordInfo;
                                dacheSpreadMap[userid2] = wordInfoMap;
                            }

                        }
                    }
                }
                g = g->next;
            }
        }

    }
    for(iterA1 = mutilSpreadMap.begin();iterA1 != mutilSpreadMap.end();iterA1++)
    {
        for(iterB = iterA1->second.begin();iterB != iterA1->second.end();iterB++)
        {
            if(iterB->second.infectStatus == 0 && iterB->second.tfIdf >= mu)
                iterB->second.infectStatus = 1;
            else
                iterB->second.infectStatus = 2;
        }
    }

    for(iterA1 = dacheSpreadMap.begin();iterA1 != dacheSpreadMap.end();iterA1++)
    {
        for(iterB = iterA1->second.begin();iterB != iterA1->second.end();iterB++)
        {
            if(iterB->second.infectStatus == 0 && iterB->second.tfIdf >= mu)
                iterB->second.infectStatus = 1;
            else
                iterB->second.infectStatus = 2;
        }
        mutilSpreadMap[iterA1->first]=iterA1->second;
    }
    dacheSpreadMap.clear();

    sign = false;
    for(iterA1 = mutilSpreadMap.begin();iterA1 != mutilSpreadMap.end();iterA1++)
    {
        for(iterB = iterA1->second.begin();iterB != iterA1->second.end();iterB++)
        {
            if(iterB->second.infectStatus == 1)
            {
                sign = true;
                break;
            }
        }
        if(sign == true)
            break;
    }

}
    return true;
}

bool SpreadEx::TopicDiffuse()
{
    map<long,map<int,WordInfo> > mutilSpreadMap;
    map<long,map<int,WordInfo> >::iterator iterA;
    map<int,WordInfo>::iterator iterB;
    map<int,float> keyWordIter;
    unsigned int i = 0;
    //初始化mutilSpreadMap
    if(!InitMutilSpreadMap(mutilSpreadMap))
    {
        LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"mutilSpreadMap Fail");
    }

    //模拟关键词信息传播
    if(!ModelMutilSpreadMap(mutilSpreadMap))
    {
         LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"ModelMutilSpreadMap Fail");
    }

    //计算keyWordMap
    for(iterA = mutilSpreadMap.begin();iterA != mutilSpreadMap.end();iterA++)
    {
        for(iterB = iterA->second.begin();iterB != iterA->second.end();iterB++)
        {
            keyWordMap[iterB->first] += iterB->second.tfIdf;
        }
    }
    return true;
}

bool SpreadEx::Init(){
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"SpreadEx Init");
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
    for(unsigned int day=0;day<40;day++)
    {
        cout<<"day:"<<day<<endl;
        startTime = dateArray[day] + " " + timeArray[0];
        endTime = dateArray[day+1] + " " + timeArray[0];
        if(!BuildKeyHotWordMap(startTime,endTime))
        {
            LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                    ,"BuildKeyHotWordMap fail");
        }
        for(unsigned int tao = 0;tao<4;tao++)
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
    LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,TimeUtil::GetInstance()->LocalTimeStr());
    return true;
}

bool SpreadEx::DynamicSpread(const string &startTime,const string &midTime,const string &endTime)
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

void SpreadEx::Show(){

}

void SpreadEx::Test(){
    Init();
}

SpreadEx::~SpreadEx()
{
    userGraphMap = NULL;
    sidWTIMap = NULL;
    widSTMap = NULL;
    if(SpreadEx::_pInstance)
        delete SpreadEx::_pInstance;
}

