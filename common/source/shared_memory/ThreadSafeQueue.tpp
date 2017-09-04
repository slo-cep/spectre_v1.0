/*
 * ThreadSafeQueue.tpp
 *
 * Created on: 01.12.2016
 *      Author: sload
 */

#include "../../header/shared_memory/ThreadSafeQueue.hpp"

namespace shared_memory
{

template<typename T>
bool ThreadSafeQueue<T>::push(const T &item)
{
    lock_guard<mutex> lk(this->mtx);
    this->internalQueue.push(item);
    return true;
}

template<typename T>
bool ThreadSafeQueue<T>::pop(T &item)
{
 lock_guard<mutex> lk(this->mtx);

 if(this->internalQueue.size()==0)
     return false;

 item=this->internalQueue.front();
 this->internalQueue.pop();
 return  true;

}

template<typename T>
unsigned long ThreadSafeQueue<T>::size() const
{
    lock_guard<mutex> lk(this->mtx);
    return  this->internalQueue.size();
}

template<typename T>
bool ThreadSafeQueue<T>::isEmpty() const
{
    lock_guard<mutex> lk(this->mtx);
    return this->internalQueue.size()!=0;
}


}
