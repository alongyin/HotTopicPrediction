#ifndef HETERSPREAD_H
#define HETERSPREAD_H
#include "buildHeterGraph.h"
#include "topicDiscover.h"
using namespace std;
class HeterSpread{
private:
    static HeterSpread *_pInstance;
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
    map<long int,HeterLink> *heterGraphMap;
    map<long,WordTfIdfLink> *sidWTIMap;
    map<int,StatusTagLink> *widSTMap;
public:
    static HeterSpread *GetInstance();
    HeterSpread();
    bool Init(); //传播过程初始化
    bool DynamicHeterSpread(const string &startTime,const string &midTime,const string &endTime);
    bool HeterTopicDiffuse();//话题传播
    bool GetKeyWordMap(const string &startTime,const string &endTime);
    bool GetHotWordMap(const string &startTime,const string &endTime);
    bool BuildKeyHotWordMap(const string &startTime,const string &endTime);
    bool CalculateF1();
    bool TMBP(map<long,HeterStatus> &heterSMap,const int &level);
    float BiasedRegularization(map<long,HeterStatus> &heterSMap,const int &level);
    void Show(); //展示结果内容
    void Test();
    ~HeterSpread();
};
#endif // HETERSPREAD_H
