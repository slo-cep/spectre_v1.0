/*
 * ThreadSafeQueue.hpp
 *
 * Created on: 01.12.2016
 *      Author: sload
 */

#ifndef THREADSAFEQUEUE_HPP
#define THREADSAFEQUEUE_HPP

#include<queue>
#include<mutex>
using namespace std;

namespace shared_memory
{
template<typename T>
class ThreadSafeQueue
{
public:
    bool push(const T& item);

    /**
     *@brief pop: remove an item from the head of the stream
     * @return true if there is an item to remove else false (queue is empty)
     */
    bool pop(T& item);

    unsigned long size(void) const;

    /**
     *@brief isEmpty: check if empty
     * @return true or false
     */
    bool isEmpty(void) const;

private:
    queue<T> internalQueue;
    mutable mutex mtx;
};
}
#include "../../source/shared_memory/ThreadSafeQueue.tpp"

#endif // THREADSAFEQUEUE_HPP
