#ifndef PRUNING_H
#define PRUNING_H
#include "buildGraph.h"
#include "topicDiscover.h"
#include "queue.h"
#include "math.h"
#include "global.h"
#include "assessUtil.h"
#include "dbUtil.h"
#include "timeUtil.h"
using namespace std;



class Pruning{
private:
    static Pruning *_pInstance;
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
    int PruningCount;
    map<long int,UserLink> *userGraphMap;
    map<long,WordTfIdfLink> *sidWTIMap;
    map<int,StatusTagLink> *widSTMap;
public:
    static Pruning *GetInstance();
    Pruning();
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
    bool ShowUserSMap(map<long,UserStatus> &userSMap);
    bool PruningUserGraph(map<long int,UserLink> *userGraphMap,UserLLink userhead);

    //Method4 根据权值来进行剪枝计算
    bool PruningMethod4();
    bool SortUser(map<int,UserTopicValueLink> &widUserTopicMap,map<long int,UserLink> *userGraphMap,UserLLink userhead);

    //Method5 根据用户兴趣的相似性来对用户进行排序
    bool PruningMethod5();
    bool InitWordTopicValueMap(map<long,WordTfIdfLink> &uidWordTopicMap);
    bool SortUserByInterest(map<long,WordTfIdfLink> &uidWordTopicMap,UserLLink userhead);
    float CalcuateInterest(WordTfIdfLink head1,WordTfIdfLink head2);
    bool SortUserLink(map<long,UserLLink> &userInterestMap);
    bool SortUserII(map<long,UserLLink> &userInterestMap,UserLLink userhead);
    bool InsertUserHead(UserLLink userhead);
    bool QueryUserHead(UserLLink userhead);
    bool FreeWordTopicValueMap(map<long,WordTfIdfLink> &uidWordTopicMap);
    bool FreeUserInterestMap(map<long,UserLLink> &userInterestMap);

    bool InitUserTopicValueMap(map<int,UserTopicValueLink> &widUserTopicMap);
    bool SortUserTopicValueMap(map<int,UserTopicValueLink> &widUserTopicMap);
    bool FreeUserTopicValueMap(map<int,UserTopicValueLink> &widUserTopicMap);

    //Method1 随机进行剪枝
    bool PruningMethod1();
    bool RandomSortUser(UserLLink userhead);

    //Method2 选择度大的进行剪枝
    bool PruningMethod2();
    //Method3 选择度小的进行剪枝
    bool PruningMethod3();
    bool SortUserByDgree(UserLLink userhead,bool BigSign);

    void Show(); //展示结果内容
    void Test(map<int,UserTopicValueLink> &widUserTopicMap);
    ~Pruning();
};
#endif // PRUNING_H
