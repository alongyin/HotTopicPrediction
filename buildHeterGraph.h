#ifndef BUILDHETERGRAPH_H
#define BUILDHETERGRAPH_H
#include "global.h"
#include "cacheUtil.h"
#include "dbUtil.h"
#include "logUtil.h"
#include "timeUtil.h"

using namespace std;

class BuildHeterGraph{
private:
    map<long int,HeterLink> heterGraphMap;
    static BuildHeterGraph *_pInstance;
public:
    BuildHeterGraph();
    static BuildHeterGraph *GetInstance();
    bool Init(); //"xxxx-xx-xx xx:xx:xx"
    bool Clear();//清空用户数据图
    int Show(); //展示结果
    bool AddHeterGraph(const string &startTime,const string &endTime);
    map<long int,HeterLink> *GetHeterGraphMap();
    int InsertHeterGraph(const long &userId,const long &sid,const long &fuserId);
    void Test(); //做实验室
    ~BuildHeterGraph();
};


#endif // BUILDHETERGRAPH_H
