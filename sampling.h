#ifndef SAMPLING_H
#define SAMPLING_H
#include "buildGraph.h"
#include "topicDiscover.h"
#include "queue.h"
#include "math.h"
#include "global.h"
#include "assessUtil.h"
#include "dbUtil.h"
#include "timeUtil.h"
#endif // SAMPLING_H


class Sampling{
private:
    static Sampling *_pInstance;
    map<int,float> keyWordMap;
    map<int,float> hotWordMap;
    vector<int> keyWordList;
    vector<int> hotWordList;
    float f1;
    int C;
    int C1;
    float mu;
    int tao;
    float xipuxiluo;
    int samplingCount;
    int totalCount;
    map<long int,UserLink> *userGraphMap;
    map<long,WordTfIdfLink> *sidWTIMap;
    map<int,StatusTagLink> *widSTMap;

public:
    static Sampling* GetInstance();
    Sampling();
    bool Init(); //传播过程初始化
    bool TopicDiffuse(); //话题传播
    bool BuildQueue(Queue<long> &lqueue,map<long,UserStatus> &userSMap,StatusTagLink p,const int &wid);//建立传播列表队列
    bool ModelSpread(Queue<long> &lqueue,map<long,UserStatus> &userSMap); //模拟传播过程
    float DiffuseRate(const int &srcWeight,const int &dstWeight,const int &conntect);//传播概率
    bool DynamicSpread(const string &startTime,const string &midTime,const string &endTime); //某一段时间的，话题动态传播
    bool GetKeyWordMap(const string &startTime,const string &endTime);
    bool GetHotWordMap(const string &startTime,const string &endTime);
    bool BuildKeyHotWordMap(const string &startTime,const string &endTime);
    bool CalculateF1();


    bool StatisticUserInfo(map<long int,UserLink> *userGraphMap);//统计用户数量，话题数量，平均度数
    bool SamplingUserHead(UserLLink userhead); //从中抽样出
    bool PruningUserGraph(map<long int,UserLink> *userGraphMap,UserLLink userhead);

    //方法1：简单随见点抽样
    bool SamplingMethod1();
    bool RandomSortUser(UserLLink userhead);

    //方法2：基于节点度增序顺序的点抽样
    bool SamplingMethod2();

    //方法3：基于节点度降序顺序的点抽样
    bool SamplingMethod3();

    bool SortUserByDgree(UserLLink userhead,bool BigSign);

    //方法4：按照拥有话题的权重性从大到小来进行排名，进行抽样
    bool SamplingMethod4();
    bool InitUserTopicValueMap(map<int,UserTopicValueLink> &widUserTopicMap);
    bool SortUserTopicValueMap(map<int,UserTopicValueLink> &widUserTopicMap);
    bool SortUser(map<int,UserTopicValueLink> &widUserTopicMap,map<long int,UserLink> *userGraphMap,UserLLink userhead);
    bool FreeUserTopicValueMap(map<int,UserTopicValueLink> &widUserTopicMap);
    //方法5：根据用户的兴趣相似性，从大到小的顺序来进行用户排名
    bool SamplingMethod5();
    bool QueryUserHead(UserLLink userhead);
    bool InitWordTopicValueMap(map<long,WordTfIdfLink> &uidWordTopicMap);
    bool FreeWordTopicValueMap(map<long,WordTfIdfLink> &uidWordTopicMap);

    ~Sampling();


};
