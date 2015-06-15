#ifndef BUILDGRAPH_H
#define BUILDGRAPH_H
#include "global.h"
#include "cacheUtil.h"
#include "dbUtil.h"
#include "logUtil.h"
#include "timeUtil.h"
using namespace std;

typedef struct UserNode{
    long int uid;
    float weight;
    int connect;
    struct UserNode *next;
}*UserLink;

class BuildGraph{
private:
    map<long int,UserLink> userGraphMap;
    static BuildGraph *_pInstance;
public:
    BuildGraph();
    static BuildGraph *GetInstance();
    bool Init(); //"xxxx-xx-xx xx:xx:xx"
    bool Clear();//清空用户数据图
    int Show(); //展示结果
    bool AddGraph(const string &startTime,const string &endTime);
    bool UpdateGraphWeight();
    map<long int,UserLink> *GetUserGraphMap();
    int InsertUserGraph(const long &userId,const long &fuserId);
    void Test(); //做实验室
    ~BuildGraph();
};
#endif // BUILDGRAPH_H
