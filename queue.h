#ifndef QUEUE_H
#define QUEUE_H
#include "global.h"
using namespace std;
template<class T>
struct Node
{
    T info;
    struct Node<T> *next;
};


template<class T>
class Queue
{
private:
    Node<T> *head;
    Node<T> *tail;
public:
    Queue();
    Queue(const T &data);
    ~Queue();
    bool IsEmpty();
    bool EnQueue(const T &data);
    T GetQueue();
    T GetHead();
    void Test();
};
#endif // QUEUE_H
