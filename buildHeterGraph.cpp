#include "buildHeterGraph.h"

BuildHeterGraph* BuildHeterGraph::_pInstance = new BuildHeterGraph();

BuildHeterGraph::BuildHeterGraph()
{
    Init();
}

BuildHeterGraph* BuildHeterGraph::GetInstance()
{
    if(_pInstance == NULL)
        _pInstance = new BuildHeterGraph();
    return _pInstance;
}

bool BuildHeterGraph::Init()
{

    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"BuildHeterGraph Init");
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
            if(InsertHeterGraph(userId,statusV[i].sid,statusV[i].userid))
            {
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,"InsertUserGraph fail");
                return false;
            }
        }
    }
    return true;
}

int BuildHeterGraph::InsertHeterGraph(const long &userId,const long &sid,const long &fuserId){
    map<long int,HeterLink>::iterator iter;
    HeterLink header;
    struct HeterNode *p;
    iter = heterGraphMap.find(userId);
    if(iter != heterGraphMap.end()){
            header = iter->second;
            p = header;
            while(p!=NULL){
                if(p->id == sid)
                {
                    break;
                }
                p = p->next;
            }
            if(p == NULL)
            {
                p = (struct HeterNode *)malloc(sizeof(struct HeterNode));
                p->id = sid;
                p->isUser = false;
                p->next = header->next;
                header->next = p;
                header->nodeCount++;

                iter = heterGraphMap.find(sid);
                if(iter != heterGraphMap.end())
                {
                    p = (struct HeterNode *)malloc(sizeof(struct HeterNode));
                    header = iter->second;
                    p->id = userId;
                    p->isUser = true;
                    p->next = header->next;
                    header->next = p;
                    header->nodeCount++;
                }
                else
                {
                    header = (struct HeterNode *)malloc(sizeof(struct HeterNode));
                    header->id = userId;
                    header->next = NULL;
                    header->isUser = true;
                    header->nodeCount = 1;
                    heterGraphMap[sid] = header;
                }
            }
    }
    else
    {
        header = (struct HeterNode *)malloc(sizeof(struct HeterNode));
        header->id = sid;
        header->next = NULL;
        header->isUser = false;
        header->nodeCount = 1;
        heterGraphMap[userId] = header;

        iter = heterGraphMap.find(sid);
        if(iter != heterGraphMap.end())
        {
            p = (struct HeterNode *)malloc(sizeof(struct HeterNode));
            header = iter->second;
            p->id = userId;
            p->isUser = true;
            p->next = header->next;
            header->next = p;
            header->nodeCount++;
        }
        else
        {
            header = (struct HeterNode *)malloc(sizeof(struct HeterNode));
            header->id = userId;
            header->next = NULL;
            header->isUser = true;
            header->nodeCount = 1;
            heterGraphMap[sid] = header;
        }

    }

   iter = heterGraphMap.find(fuserId);
    if(iter != heterGraphMap.end()){
            header = iter->second;
            p = header;
            while(p!=NULL){
                if(p->id == sid)
                {
                    break;
                }
                p = p->next;
            }
            if(p == NULL)
            {
                p = (struct HeterNode *)malloc(sizeof(struct HeterNode));
                p->id = sid;
                p->isUser = false;
                p->next = header->next;
                header->next = p;
                header->nodeCount++;

                iter = heterGraphMap.find(sid);
                if(iter != heterGraphMap.end())
                {
                    p = (struct HeterNode *)malloc(sizeof(struct HeterNode));
                    header = iter->second;
                    p->id = fuserId;
                    p->isUser = true;
                    p->next = header->next;
                    header->next = p;
                    header->nodeCount++;
                }
                else
                {
                    header = (struct HeterNode *)malloc(sizeof(struct HeterNode));
                    header->id = fuserId;
                    header->next = NULL;
                    header->isUser = true;
                    header->nodeCount = 1;
                    heterGraphMap[sid] = header;
                }
            }
    }
    else
    {
        header = (struct HeterNode *)malloc(sizeof(struct HeterNode));
        header->id = sid;
        header->next = NULL;
        header->isUser = false;
        header->nodeCount = 1;
        heterGraphMap[fuserId] = header;

        iter = heterGraphMap.find(sid);
        if(iter != heterGraphMap.end())
        {
            p = (struct HeterNode *)malloc(sizeof(struct HeterNode));
            header = iter->second;
            p->id = fuserId;
            p->isUser = true;
            p->next = header->next;
            header->next = p;
            header->nodeCount++;
        }
        else
        {
            header = (struct HeterNode *)malloc(sizeof(struct HeterNode));
            header->id = fuserId;
            header->next = NULL;
            header->isUser = true;
            header->nodeCount = 1;
            heterGraphMap[sid] = header;
        }
    }
    return 0;
}

bool BuildHeterGraph::AddHeterGraph(const string &startTime,const string &endTime){
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
        userId = CacheUtil::GetInstance()->FindUserIdBySid(statusV[i].sid);
        if(userId != -1)
        {
            if(InsertHeterGraph(userId,statusV[i].sid,statusV[i].userid))
            {
                LOG4CPLUS_ERROR(LogUtil::GetInstance()->GetLogger()
                                ,"InsertUserGraph fail");
                 return false;
            }
        }
    }
    return true;
}

int BuildHeterGraph::Show()
{
    map<long int,HeterLink>::iterator iter;
    struct HeterNode *p;
    for(iter = heterGraphMap.begin();iter != heterGraphMap.end();iter++){
            if(iter->second->isUser == true)
                cout<<"sid:"<<iter->first<<" Link ";
            else
                cout<<"userid"<<iter->first<<" Link ";
            p = iter->second;
            cout<<"count "<<p->nodeCount<<" ";
            while(p!=NULL){
                cout<<p->id<<" ";
                p = p->next;
            }
            cout<<endl;
    }
}

bool BuildHeterGraph::Clear(){
    map<long int,HeterLink>::iterator iter;
    struct HeterNode *p;
    struct HeterNode *q;
    for(iter = heterGraphMap.begin();iter != heterGraphMap.end();iter++){
            p = iter->second;
            q = p->next;
            while(q!=NULL){
                delete p;
                p = q;
                q = p->next;
            }
            delete p;
    }
    heterGraphMap.clear();
    return true;
}

map<long int,HeterLink>* BuildHeterGraph::GetHeterGraphMap()
{
    return &heterGraphMap;
}

void BuildHeterGraph::Test(){
    AddHeterGraph("2009-08-20","2009-08-23");
    Show();
}

BuildHeterGraph::~BuildHeterGraph()
{
   Clear();
   if(BuildHeterGraph::_pInstance)
    delete BuildHeterGraph::_pInstance;
}
