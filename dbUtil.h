#ifndef DBUTIL_H
#define DBUTIL_H
#include <mysql/mysql.h>
#include "global.h"
#include "logUtil.h"
using namespace std;
struct Word
{
    string cont;
    int wid;
    string pos; //表示单词的词性
};

struct Status
{
    long sid;
    string cont;
    long userid;
    string datetime;
};

struct TagStatus{
    long sid;
    string widList;
};

struct Users{
    long uid;
    int statusCount;
    int followerCount;
    int friendCount;
    int favouriteCount;
};

struct PrunUser
{
    long uid;
    float value;
};
class DbUtil
{
private:
    static DbUtil *_pInstance;
    MYSQL mysql;
    int ret;
public:
    DbUtil();
    static DbUtil *GetInstance();
    int DbQueryWord(const string &query,vector<Word> &wordList);//查询word表的接口
    int DbQueryStatus(const string &query,vector<Status> &statusList,bool isCont=0,bool isTime=0);//查询status表的接口
    int DbQueryTagStatus(const string &query,vector<TagStatus> &tagStatusList); //查询tagStatus表的接口
    int DbQueryUsers(const string &query,vector<Users> &usersList); //查询user表的接口
    int DbQueryCount(const string &query,int &totalCount);
    int DbQueryPrunUser(const string &query,vector<PrunUser> &prunUserList);
    int DbInsert(const string &query);//插入接口
    int DbClear(const int &tableName);

    void Test();
    ~DbUtil();
};
#endif // DBUTIL_H
