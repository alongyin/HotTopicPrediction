#include "cacheUtil.h"

CacheUtil* CacheUtil::_pInstance = new CacheUtil();

CacheUtil::CacheUtil()
{
    string query = "select count(*) from wordH";
    if(DbUtil::GetInstance()->DbQueryCount(query,totalWid)){
        totalWid = 0;
    }
    totalPos = totalWid;
    totalCont = totalWid;

    query = "select count(*) from statusH";
    if(DbUtil::GetInstance()->DbQueryCount(query,totalStatus)){
        totalStatus = 0;
    }

    query = "select count(*) from user";
    if(DbUtil::GetInstance()->DbQueryCount(query,totalUsers)){
        totalUsers = 0;
    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"CacheUtil Init"<<" totalWid:"<<totalWid<<" totalPos:"<<totalPos<<
                   " totalCont:"<<totalCont<<" totalStatus:"<<totalStatus<<" totalUsers:"<<totalUsers);

    if(!InitWordWidMap())
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"InitWordWidMap fail");

    if(!InitWordContMap())
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"InitWordContMap fail");

    if(!InitStatusMap())
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"InitStatusMap fail");

    if(!InitStatusMap())
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"InitStatusMap fail");

    if(!InitWordPosMap())
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"InitWordPosMap fail");
}

CacheUtil* CacheUtil::GetInstance()
{
    if(_pInstance == NULL)
        _pInstance = new CacheUtil();
    return _pInstance;
}

int CacheUtil::InitWordWidMap()
{
    vector<Word> wordV;
    if(totalWid == 0)
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"totalWid:"<<totalWid);
        return false;
    }
    if(!wordWidMap.empty())
        wordWidMap.clear();
    string query = "select cont,wid from wordH order by wid asc";
    if(DbUtil::GetInstance()->DbQueryWord(query,wordV))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbQueryWord fail");
        return false;
    }
    for(unsigned int i=0;i<wordV.size();i++){
            wordWidMap[wordV[i].cont] = wordV[i].wid;
    }
    wordV.clear();
    return true;
}

int CacheUtil::FindIdByWord(const string &cont){
    map<string,int>::iterator iter;
    iter = wordWidMap.find(cont);
    if(iter != wordWidMap.end())
        return iter->second;
    return -1;
}

int CacheUtil::InitWordContMap()
{
    vector<Word> wordV;
    if(totalCont == 0)
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"totalCont:"<<totalCont);
        return false;
    }
    if(!wordContMap.empty())
        wordContMap.clear();
    string query = "select cont,wid from wordH order by wid asc";
    if(DbUtil::GetInstance()->DbQueryWord(query,wordV))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbQueryWord fail");
        return false;
    }
    for(unsigned int i=0;i<wordV.size();i++){
            wordContMap[wordV[i].wid] = wordV[i].cont;
    }
    wordV.clear();
    return true;
}

string CacheUtil::FindContByWord(const int &wid)
{
    map<int,string>::iterator iter;
    iter = wordContMap.find(wid);
    if(iter != wordContMap.end())
        return iter->second;
    return "";
}


int CacheUtil::InitStatusMap()
{
    vector<Status> statusV;
    if(totalStatus == 0)
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"totalStatus:"<<totalStatus);
        return false;
    }
    if(!statusMap.empty())
        statusMap.clear();
    string query = "select sid,userid from statusH order by time asc";
    if(DbUtil::GetInstance()->DbQueryStatus(query,statusV))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbQueryStatus fail");
        return false;
    }
    for(unsigned int i=0;i<statusV.size();i++){
            statusMap[statusV[i].sid] = statusV[i].userid;
    }
    statusV.clear();
    return true;
}

long CacheUtil::FindUserIdBySid(const long &sid){
    map<long,long>::iterator iter;
    iter = statusMap.find(sid);
    if(iter != statusMap.end())
        return iter->second;
    return -1;
}

int CacheUtil::InitUserMap()
{
    vector<Users> usersV;
    if(totalUsers == 0)
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"totalUsers:"<<totalUsers);
        return false;
    }
    if(!userMap.empty())
        userMap.clear();
    string query = "select uid,statuses_count,followers_count,friends_count,favourites_count from user ";
    if(DbUtil::GetInstance()->DbQueryUsers(query,usersV))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbQueryUsers fail");
        return false;
    }
    for(unsigned int i=0;i<usersV.size();i++){
            Users *usersNode = new Users;
            usersNode->uid = usersV[i].uid;
            usersNode->followerCount = 0;
            usersNode->statusCount = 0;
            usersNode->friendCount = 0;
            usersNode->favouriteCount = 0;
            userMap[usersV[i].uid] = usersNode;
    }
    usersV.clear();
    return true;
}

float CacheUtil::FindWeightByUid(const long &uid){
    map<long,Users*>::iterator iter;
    iter = userMap.find(uid);
    if(iter != userMap.end())
    {
        return iter->second->followerCount;
    }
    return 0.0;
}

bool CacheUtil::InsertUserMap(const long &userid,const int &followCount,const int &statusCount)
{
    map<long,Users*>::iterator iter;
    iter = userMap.find(userid);
    if(iter != userMap.end())
    {
        iter->second->followerCount += followCount;
        iter->second->statusCount += statusCount;
    }
    else
    {
        Users *userNode = new Users;
        userNode->favouriteCount = 0;
        userNode->followerCount = followCount;
        userNode->friendCount = 0;
        userNode->uid = userid;
        userNode->statusCount = statusCount;
        userMap[userid] = userNode;
    }
    return true;
}

int CacheUtil::InitWordPosMap(){
    vector<Word> wordV;
    if(totalPos == 0)
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"totalPos:"<<totalPos);
         return false;
    }
    if(!wordPosMap.empty())
        wordPosMap.clear();
    string query = "select cont,wid,pos from wordH order by wid asc";
    if(DbUtil::GetInstance()->DbQueryWord(query,wordV))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbQueryWord fail");
        return false;
    }
    for(unsigned int i=0;i<wordV.size();i++){
            wordPosMap[wordV[i].wid] = wordV[i].pos;
    }
    wordV.clear();
    return true;
}


string CacheUtil::FindPosByWid(const int &wid){
    map<int,string>::iterator iter;
    iter = wordPosMap.find(wid);
    if(iter != wordPosMap.end())
        return iter->second;
    return "";
}

void CacheUtil::Test()
{

}

CacheUtil::~CacheUtil()
{
    map<long,Users*>::iterator iter;
    for(iter = userMap.begin();iter != userMap.end();iter++)
    {
        Users *p = iter->second;
        delete p;
    }
    userMap.clear();
    wordWidMap.clear();
    wordContMap.clear();
    statusMap.clear();
    wordPosMap.clear();
    if(CacheUtil::_pInstance)
        delete CacheUtil::_pInstance;
}

