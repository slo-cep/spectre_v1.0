/*
 * sourcesplitterlockfreequeue
 *
 * Created on: 11.01.2017
 *      Author: sload
 */

#include "header/shared_memory/SourceSplitterLockfreeQueue.hpp"


using namespace std;
using namespace events;

namespace shared_memory
{

SourceSplitterLockfreeQueue::SourceSplitterLockfreeQueue( ):
        AbstractSourceSplitterStream(0), sourceSplitterQueue(100000)
{
}

SourceSplitterLockfreeQueue::SourceSplitterLockfreeQueue(size_t size_limit) :
        AbstractSourceSplitterStream(size_limit), sourceSplitterQueue(size_limit)
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
bool SourceSplitterLockfreeQueue::push(shared_ptr<AbstractEvent> event)
{
    return this->sourceSplitterQueue.push(event);
}

/**
 * removes an event from the head of the stream
 * @return the removed event on success else null
 */
shared_ptr<AbstractEvent> SourceSplitterLockfreeQueue::pop()
{
    shared_ptr<AbstractEvent> event=NULL;
    if(this->sourceSplitterQueue.pop(event))
            return event;
    else
    return nullptr;
}

SourceSplitterLockfreeQueue::~SourceSplitterLockfreeQueue()
{
}
}


