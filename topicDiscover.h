#ifndef TOPICDISCOVER_H
#define TOPICDISCOVER_H
#include "dbUtil.h"
#include "cacheUtil.h"
#include "timeUtil.h"
#include "global.h"
#include "logUtil.h"
#include "assessUtil.h"
using namespace std;

class TopicDiscover{
private:
    int totalStatus; //微博的总数
    map<long,int> sidWCountMap; //key:sid value:word count
    map<int,StatusTagLink> widSTMap;//key:wid  value: StatusTagLink
    map<long,WordTfIdfLink> sidWTIMap; //key:sid value: WordTfIdfLink
    static TopicDiscover *_pInstance;
public:
    TopicDiscover();
    void Show(); //展示结果
    bool Init(); //功能初始化
    bool DealTagStatus(long sid,const string &widList); //处理每一条记录
    bool TfIdf();//用于计算TfIdf值的
    bool IntervalTopic(const string &startTime,const string &endTime); //单位时间内的时间间隔
    bool Clear(); //清楚字典中所有数据
    void Test();//用于做测试
    map<long,WordTfIdfLink> *GetSidWTIMap();
    map<int,StatusTagLink> *GetWidSTMap();
    static TopicDiscover *GetInstance();
    ~TopicDiscover();
};

#endif // TOPICDISCOVER_H
