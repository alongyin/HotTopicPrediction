#ifndef SPREADEX_H
#define SPREADEX_H
#include "buildGraph.h"
#include "topicDiscover.h"
#include "math.h"
#include "global.h"
#include "assessUtil.h"
using namespace std;

class SpreadEx{
private:
    static SpreadEx *_pInstance;
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
    map<long int,UserLink> *userGraphMap;
    map<long,WordTfIdfLink> *sidWTIMap;
    map<int,StatusTagLink> *widSTMap;
public:
    static SpreadEx *GetInstance();
    SpreadEx();
    bool Init(); //传播过程初始化
    bool TopicDiffuse(); //话题传播
    float DiffuseRate(const int &srcWeight,const int &dstWeight,const int &conntect);//传播概率
    bool DynamicSpread(const string &startTime,const string &midTime,const string &endTime); //某一段时间的，话题动态传播
    bool InitMutilSpreadMap(map<long,map<int,WordInfo> > &mutilSpreadMap);
    bool ModelMutilSpreadMap(map<long,map<int,WordInfo> > &mutilSpreadMap);
    bool GetKeyWordMap(const string &startTime,const string &endTime);
    bool GetHotWordMap(const string &startTime,const string &endTime);
    bool BuildKeyHotWordMap(const string &startTime,const string &endTime);
    bool CalculateF1();
    bool ShowUserSMap(map<long,UserStatus> &userSMap);
    void Show(); //展示结果内容
    void Test();
    ~SpreadEx();
};
#endif // SPREADEX_H
