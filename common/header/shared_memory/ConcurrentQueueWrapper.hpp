/*
 * ConcurrentQueueWrapper.hpp
 *
 * Created on: 10.02.2017
 *      Author: sload
 */

#ifndef CONCURRENTQUEUEWRAPPER_HPP
#define CONCURRENTQUEUEWRAPPER_HPP

#include "concurrentqueue.h"

namespace shared_memory
{

template <typename T>
class ConcurrentQueueWrapper
{
public:

ConcurrentQueueWrapper():queue()
{}

ConcurrentQueueWrapper(size_t initialCapacity):queue(initialCapacity)
{}


    /**
     * push an item to the head of the queue
     * @return true: on success else false
     */
    bool push(T const & t)
    {
        return queue.enqueue(t);
    }

    /**
     * if pop operation is successful, object will be copied to ret.
     * @return true, if the pop operation is successful, false if queue was empty
     */
    bool pop(T & ret)
    {
       return queue.try_dequeue(ret);
    }

    /**
     * check if empty
     * @return true or false
     */
    bool isEmpty(void) const{return queue.size_approx()==0;}


private:

    moodycamel::ConcurrentQueue <T> queue;
};
}


#endif // CONCURRENTQUEUEWRAPPER_HPP
