/*
 * SourceSplitterQueue.cpp
 *
 *  Created on: Jul 22, 2016
 *      Author: sload
 */

#include "../../header/shared_memory/SourceSplitterQueue.hpp"

using namespace std;
using namespace events;

namespace shared_memory
{

SourceSplitterQueue::SourceSplitterQueue(size_t size_limit) :
        AbstractSourceSplitterStream(size_limit)
{

}

/**
 * push an event to the tail of the stream
 * @param event: the string representation of an event
 * 			which should be parsed in the Splitter to a real event
 * @return true: on success (event added to the queue) else false (queue is full)
 */
/*
 * using template here because I don't know the event representation between source and splitter
 * It may be string, bytes, etc.
 */
bool SourceSplitterQueue::push(shared_ptr<AbstractEvent> event)
{
    lock_guard<mutex> lk(mtx); //use lock to protect the queue
    //don't add[and return false] if the queue is full and it is bounded(size_limit=0 means unbounded)
    if ( (this->sourceSplitterQueue.size() < this->size_limit) || (this->size_limit==0))
    {
        this->sourceSplitterQueue.push(event);
        return true;
    } else
        return false;
}

/**
 * removes an event from the head of the stream
 * @return the removed event on success else null
 */
shared_ptr<AbstractEvent> SourceSplitterQueue::pop()
{
    lock_guard<mutex> lk(mtx); //using lock to protect the queue
    if (!this->sourceSplitterQueue.empty())
    {
        shared_ptr<AbstractEvent> event = this->sourceSplitterQueue.front();
        this->sourceSplitterQueue.pop();
        return event;
    } else
    {
        return NULL;
    }
}


SourceSplitterQueue::~SourceSplitterQueue()
{
}
}


