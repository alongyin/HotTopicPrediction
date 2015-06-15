#include "queue.h"

template<typename T>
Queue<T>::Queue(){
    head = new Node<T>;
    tail = head;
    tail->next = NULL;
}

template<typename T>
Queue<T>::Queue(const T &data)
{
    head = new Node<T>;
    head->info = data;
    tail = new Node<T>;
    head->next = tail;
    tail->next = NULL;
}

template<typename T>
bool Queue<T>::EnQueue(const T &data)
{
    tail->info = data;
    Node<T> *p;
    p = new Node<T>;
    p->next = NULL;
    tail->next = p;
    tail = p;
    //cout<<"head: "<<head->info<<endl;
    return true;
}

template<typename T>
bool Queue<T>::IsEmpty()
{
    if(head == tail)
        return true;
    return false;
}

template<typename T>
T Queue<T>::GetQueue()
{
    T data = head->info;
    Node<T> *p;
    p = head;
    head = head->next;
    delete p;
    return data;
}

template<typename T>
T Queue<T>::GetHead()
{
    T data = head->info;
    return data;
}

template<typename T>
Queue<T>::~Queue()
{

}

template<typename T>
void Queue<T>::Test(){

}
