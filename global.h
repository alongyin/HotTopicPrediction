#ifndef GLOBAL_H
#define GLOBAL_H

#include <string>
#include <iostream>
#include "stdio.h"
#include <vector>
#include "string.h"
#include <stdlib.h>
#include <map>
#include <set>
#include "math.h"
using namespace std;
//数据库表名
#define COMMENT 0
#define STATUS 1
#define TAGSTATUS 2
#define USERS 3
#define WORD 4


//heterSpread 会用到它
struct UserStatus
{
    float tfIdf;
    bool infected;
};
struct HeterStatus
{
    int level;
    float topicValue[2];
    bool infected; // 用于表示是否被感染
};

typedef struct TmbpNode
{
    long id;
    float topicValue;
    struct TmbpNode *next;
}*TmbpLink;

typedef struct HeterNode{
    long int id; //sid  or uid
    bool isUser;
    int nodeCount; //结点数目
    struct HeterNode *next;
}*HeterLink;

//topicDiscover 和 assessUtil 会用到它
typedef struct StatusTagNode
{
    long sid; //微博的id号
    long userid; //用户id号
    int freqCount; //词的出现频率
    struct StatusTagNode *next;
}*StatusTagLink;

typedef struct WordTfIdfNode{
    int wid;
    string cont;
    float tfIdf;
    WordTfIdfNode *next;
}*WordTfIdfLink;


//pruning需要用到的
typedef struct TopicWordInfo
{
    int wid; // 话题词wid
    float value; //
    struct TopicWordInfo *next;
}*TopicWordInfoLink;

//pruning method4
typedef struct UserTopicValue
{
    long uid;
    float value;
    struct UserTopicValue *next;
}*UserTopicValueLink;

typedef struct UserL
{
    long uid;
    float value;
    struct UserL *next;
}*UserLLink;


//SpreadEx会用到的
struct WordInfo
{
    float tfIdf;
    int infectStatus; //0 可继续感染;1 作为传播源状态;2 终止传播
};
#endif // GLOBAL_H
