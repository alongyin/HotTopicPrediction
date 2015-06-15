#include "buildGraph.h"

BuildGraph* BuildGraph::_pInstance = new BuildGraph();

BuildGraph::BuildGraph()
{
    Init();
}

BuildGraph* BuildGraph::GetInstance()
{
    if(_pInstance == NULL)
        _pInstance = new BuildGraph();
    return _pInstance;
}

int BuildGraph::InsertUserGraph(const long &userId,const long &fuserId){
    map<long int,UserLink>::iterator iter;
    UserLink header;
    iter = userGraphMap.find(userId);
    if(iter != userGraphMap.end()){
            header = iter->second;
            struct UserNode *p;
            p = header;
            while(p!=NULL){
                if(p->uid == fuserId)
                {
                    p->connect++;
                    break;
                }
                p = p->next;
            }
            if(p == NULL)
            {
                p = (struct UserNode *)malloc(sizeof(struct UserNode));
                p->uid = fuserId;
                p->connect = 1;
                p->next = header->next;
                header->next = p;
                CacheUtil::GetInstance()->InsertUserMap(userId,1,0);
            }
    }
    else{
        UserLink header;
        UserNode *p;
        header = new UserNode;
        p = new UserNode;
        header->connect = 1;
        header->next = p;
        header->uid = userId;
        p->connect =1;
        p->uid = fuserId;
        p->next = NULL;
        userGraphMap[userId] = header;
        CacheUtil::GetInstance()->InsertUserMap(userId,1,0);
    }
    return 0;
}

bool BuildGraph::Init()
{

    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"BuildGraph Init");
    vector<Status> statusV;
    string startTime = "2009-08-12 00:00:00";
    string query = "select sid,userid from comment where time < '";
    query += startTime + "'";
    if(DbUtil::GetInstance()->DbQueryStatus(query,statusV))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbQueryStatus fail");
        return false;
    }
    for(unsigned int i=0;i<statusV.size();i++){
        long userId;
        userId = CacheUtil::GetInstance()->FindUserIdBySid(statusV[i].sid);
        if(userId != -1)
        {
            if(InsertUserGraph(userId,statusV[i].userid))
            {
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,"InsertUserGraph fail");
                return false;
            }
        }
    }
    UpdateGraphWeight();
    return true;
}

bool BuildGraph::UpdateGraphWeight()
{
    map<long int,UserLink>::iterator iter;
    for(iter = userGraphMap.begin();iter != userGraphMap.end();iter++)
    {
        UserLink head = iter->second;
        UserNode *p = head;
        while(p!=NULL)
        {
            p->weight = CacheUtil::GetInstance()->FindWeightByUid(p->uid);
            p = p->next;
        }
    }
    return true;
}

bool BuildGraph::AddGraph(const string &startTime,const string &endTime){
    vector<Status> statusV;
    string query = "select sid,userid from comment where time >= '";
    query += startTime + "' and time <  '";
    query += endTime + "'";
    //将endTime 赋值给 startTime 作为新的startTime
    if(DbUtil::GetInstance()->DbQueryStatus(query,statusV))
    {
        LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                        ,"DbQueryStatus fail");
         return false;
    }
    for(unsigned int i=0;i<statusV.size();i++){
        long userId;
        int weight;
        userId = CacheUtil::GetInstance()->FindUserIdBySid(statusV[i].sid);
        if(userId != -1)
        {
            if(InsertUserGraph(userId,statusV[i].userid))
            {
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,"InsertUserGraph fail");
                 return false;
            }
        }
    }
    UpdateGraphWeight();
    return true;
}

int BuildGraph::Show()
{
    map<long int,UserLink>::iterator iter;
    struct UserNode *p;
    for(iter = userGraphMap.begin();iter != userGraphMap.end();iter++){
            cout<<"userid:"<<iter->first<<" Link ";
            p = iter->second;
            while(p!=NULL){
                cout<<p->uid<<" "<<p->connect<<" "<<p->weight<<" ";
                p = p->next;
            }
            cout<<endl;
    }
}

bool BuildGraph::Clear(){
    map<long int,UserLink>::iterator iter;
    struct UserNode *p;
    struct UserNode *q;
    for(iter = userGraphMap.begin();iter != userGraphMap.end();iter++){
            p = iter->second;
            q = p->next;
            while(q!=NULL){
                delete p;
                p = q;
                q = p->next;
            }
            delete p;
    }
    userGraphMap.clear();
    return true;
}

map<long int,UserLink>* BuildGraph::GetUserGraphMap()
{
    return &userGraphMap;
}

void BuildGraph::Test(){
}

BuildGraph::~BuildGraph()
{
   Clear();
   if(BuildGraph::_pInstance)
    delete BuildGraph::_pInstance;
}
