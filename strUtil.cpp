#include "strUtil.h"
StrUtil* StrUtil::_pInstance = new StrUtil();

StrUtil::StrUtil()
{
    LOG4CPLUS_INFO(LogUtil::GetInstance()->GetLogger()
                   ,"StrUtil Init");
}

StrUtil* StrUtil::GetInstance(){
    if(_pInstance == NULL)
        _pInstance = new StrUtil();
    return _pInstance;
}

string StrUtil::Int2String(const int &num){
    char temp[15];
    string str;
    sprintf(temp,"%d",num);
    str = temp;
    return str;
}

string StrUtil::float2String(const float &num)
{
    char temp[15];
    string str;
    sprintf(temp,"%f",num);
    str = temp;
    return str;
}

bool StrUtil::Split(const string &str,const string &delim,vector<string> &strV){
    char *p = StrToChar(str);
    char *token;
    for(token = strtok(p,delim.c_str());token != NULL;token = strtok(NULL,delim.c_str())){
            string word = token;
            strV.push_back(word);
    }
    if(p!=NULL)
        delete p;
    return true;
}

string StrUtil::Replace(string &str)
{
    map<char,char> replaceMap;
    replaceMap['#'] = ',';
    replaceMap[';'] = ',';
    replaceMap['&'] = ',';
    replaceMap['\''] = ',';
    int length = str.size();
    char *temp = new char[length+1];
    string res;
    map<char,char>::iterator iter;
    for(unsigned int i=0;i<length;i++)
    {
        iter = replaceMap.find(str[i]);

        if(iter != replaceMap.end())
        {
            temp[i] = iter->second;
        }
        else{
            temp[i] = str[i];
        }
    }
    temp[length] = '\0';
    res = temp;
    delete temp;
    return res;
}

void StrUtil::Test(){
   string input = "#兼容性测试#收藏";
   cout<<Replace(input)<<endl;
}
char* StrUtil::StrToChar(const string &str){
    int length = str.length();
    if(length == 0)
        return NULL;

    char *p = new char[length+1];
    for(unsigned int i=0;i<length;i++){
            p[i] = str[i];
    }
    p[length] = '\0';
    return p;
}

StrUtil::~StrUtil()
{
    if(StrUtil::_pInstance)
        delete StrUtil::_pInstance;
}

