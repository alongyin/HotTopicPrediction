#ifndef CACHE_H
#define CACHE_H
#include "global.h"
#include "dbUtil.h"
#include "logUtil.h"
#include "strUtil.h"
using namespace std;

class CacheUtil
{
private:
    map<string,int> wordWidMap; //基于word表 关键字是keyword，值是keyword序号
    int totalWid;

    map<int,string> wordContMap; //基于word表 关键字是wid，值是cont
    int totalCont;

    map<long,long> statusMap; //基于status表 关键字是sid,值是userid
    int totalStatus;

    map<long,Users*> userMap; //基于user表 关键字sid，值是用户权重
    int totalUsers;

    map<int,string> wordPosMap; //基于word表， 关键字是id，值是pos
    int totalPos;

    static CacheUtil *_pInstance;
public:
    CacheUtil();
    int InitWordWidMap();
    int FindIdByWord(const string &cont);

    int InitWordContMap();
    string FindContByWord(const int &wid);

    int InitStatusMap();
    long FindUserIdBySid(const long &sid);

    int InitUserMap();
    float FindWeightByUid(const long &uid);
    bool InsertUserMap(const long &userid,const int &followCount,const int &statusCount);

    int InitWordPosMap();
    string FindPosByWid(const int &wid);

    void Test();
    static CacheUtil *GetInstance();
    ~CacheUtil();
};
#endif // CACHE_H
