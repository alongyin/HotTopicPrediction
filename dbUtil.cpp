#include "dbUtil.h"
DbUtil* DbUtil::_pInstance = new DbUtil();

DbUtil::DbUtil()
{

    mysql_init(&mysql);
    if(!mysql_real_connect(&mysql,"localhost","root","123","weibo",0,NULL,0))
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Error connecting to databases"<<mysql_error(&mysql));

    ret = mysql_query(&mysql,"set names utf8");
    if(ret)
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Error set utf8"<<mysql_error(&mysql)<<" ret:"<<ret);
    }
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"DbUtil Init");
}

int DbUtil::DbQueryWord(const string &query,vector<Word> &wordList)
{
    MYSQL_RES *res;
    ret = mysql_query(&mysql,query.c_str());
    if(ret)
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Error making query"<<mysql_error(&mysql)<<" ret:"<<ret);
        return ret;
    }
    res = mysql_use_result(&mysql);
    if(res){
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res))){
            string temp;
            struct Word word;
            temp = row[0];
            word.cont = temp;
            temp = row[1];
            word.wid = atoi(temp.c_str());
            temp = row[2];
            word.pos = temp;
            wordList.push_back(word);
        }
        mysql_free_result(res);
    }
    return 0;
}

int DbUtil::DbQueryStatus(const string &query,vector<Status> &statusList,bool isCont,bool isTime)
{
    //select sid,userid,cont,datetime from status;
    MYSQL_RES *res;
    ret = mysql_query(&mysql,query.c_str());
    if(ret)
    {
         LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                         ,"Error making query"<<mysql_error(&mysql)<<" ret:"<<ret);
        return ret;
    }
    res = mysql_use_result(&mysql);
    if(res){
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res))){
            string temp;
            struct Status status;
            temp = row[0];
            status.sid = atol(temp.c_str());
            temp = row[1];
            status.userid = atol(temp.c_str());
            if(isCont)
            {
                temp = row[2];
                status.cont = temp;
            }
            if(isTime){
                temp = row[3];
                status.datetime =temp;
            }
            statusList.push_back(status);
        }
        mysql_free_result(res);
    }
    return 0;
}

int DbUtil::DbQueryPrunUser(const string &query,vector<PrunUser> &prunUserList)
{
    //select uid,value from userHead;
    MYSQL_RES *res;
    ret = mysql_query(&mysql,query.c_str());
    if(ret)
    {
         LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                         ,"Error making query"<<mysql_error(&mysql)<<" ret:"<<ret);
        return ret;
    }
    res = mysql_use_result(&mysql);
    if(res){
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res))){
            string temp;
            struct PrunUser prunUser;
            temp = row[0];
            prunUser.uid = atol(temp.c_str());
            temp = row[1];
            prunUser.value = atof(temp.c_str());
            prunUserList.push_back(prunUser);
        }
        mysql_free_result(res);
    }
    return 0;
}

int DbUtil::DbQueryTagStatus(const string &query,vector<TagStatus> &tagStatusList){
    MYSQL_RES *res;
    ret = mysql_query(&mysql,query.c_str());
    if(ret){
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Error making query"<<mysql_error(&mysql)<<" ret:"<<ret);
        return ret;
    }
    res = mysql_use_result(&mysql);
    if(res){
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res))){
            string temp;
            struct TagStatus tagStatus;
            temp = row[0];
            tagStatus.sid = atol(temp.c_str());
            temp = row[1];
            tagStatus.widList = temp;
            tagStatusList.push_back(tagStatus);
        }
        mysql_free_result(res);
    }
    return 0;
}
int DbUtil::DbQueryUsers(const string &query,vector<Users> &usersList)
{
    MYSQL_RES *res;
    ret = mysql_query(&mysql,query.c_str());
    if(ret){
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Error making query"<<mysql_error(&mysql)<<" ret:"<<ret);
        return ret;
    }
    res = mysql_use_result(&mysql);
    if(res){
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res))){
            string temp = row[0];
            struct Users users;
            users.uid = atol(temp.c_str());
            temp = row[1];
            users.statusCount = atoi(temp.c_str());
            temp = row[2];
            users.followerCount = atoi(temp.c_str());
            temp = row[3];
            users.friendCount = atoi(temp.c_str());
            temp = row[4];
            users.favouriteCount = atoi(temp.c_str());
            usersList.push_back(users);
        }
    }
    return 0;
}
int DbUtil::DbQueryCount(const string &query,int &totalCount)
{
    MYSQL_RES *res;
    ret = mysql_query(&mysql,query.c_str());
    if(ret)
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Error making query"<<mysql_error(&mysql)<<" ret:"<<ret);
        return ret;
    }
    res = mysql_use_result(&mysql);
    if(res){
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res))){
                string temp = row[0];
                totalCount = atoi(temp.c_str());
                break;
        }
        mysql_free_result(res);
    }
    return 0;
}

int DbUtil::DbClear(const int &tableName){
    string query = "delete from ";
    switch(tableName){
        case COMMENT:
            query += "comment"; break;
        case STATUS:
            query += "status"; break;
        case TAGSTATUS:
            query += "tagstatus"; break;
        case USERS:
            query += "user"; break;
        case WORD:
            query += "word"; break;
        default:
            query += "";
    }
    ret = mysql_query(&mysql,query.c_str());
    if(ret){
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Error making query"<<mysql_error(&mysql)<<" ret:"<<ret);
        return ret;
    }
    return 0;
}

int DbUtil::DbInsert(const string &query)
{
    ret = mysql_query(&mysql,query.c_str());
    if(ret){
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"Error making query"<<mysql_error(&mysql)<<" ret:"<<ret);
        return ret;
    }
    return 0;
}

void DbUtil::Test()
{

}

DbUtil* DbUtil::GetInstance()
{
    if(_pInstance == NULL)
        _pInstance = new DbUtil();
    return _pInstance;
}

DbUtil::~DbUtil()
{
    mysql_close(&mysql);
    if(DbUtil::_pInstance)
        delete DbUtil::_pInstance;
}
