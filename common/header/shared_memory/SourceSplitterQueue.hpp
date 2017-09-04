/*
 * SourceSplitterStream.hpp
 *
 *  Created on: Jul 22, 2016
 *      Author: sload
 */

#ifndef HEADER_SHARED_MEMORY_SOURCESPLITTERQUEUE_HPP_
#define HEADER_SHARED_MEMORY_SOURCESPLITTERQUEUE_HPP_

#include "AbstractSourceSplitterStream.hpp"
#include "AbstractSourceSplitterStream.hpp"
#include <string>
#include <queue>
#include <mutex>


namespace shared_memory
{
/**
 * this class implement the AbstractSourceSplitterStream and
 * it uses a queue as underlying data structure
 * all operations on the queue are thread-safe( using lock)
 */
class SourceSplitterQueue: public AbstractSourceSplitterStream
{
public:
    /**
     * default Constructor
     */
    SourceSplitterQueue() :
            AbstractSourceSplitterStream(0)
    {
    }
    /**
     * Constructor to limit the queue size
     * @param size_limit: underlying queue size, zero means ubounded queue
     */
    SourceSplitterQueue(size_t size_limit);

    /**
     * push an event to the head of the stream
     * @param event: the string representation of an event
     * 			which should be parsed in the Splitter to a real event
     * @return true: on success else false
     */
    bool push(shared_ptr<events::AbstractEvent> event) override;

    /**
     * remove an event from the head of the stream
     * @return the removed event on success else null
     */
    shared_ptr<events::AbstractEvent> pop() override;

    ~SourceSplitterQueue();

private:
    std::queue<shared_ptr<events::AbstractEvent>> sourceSplitterQueue;
    mutable std::mutex mtx;

};
}

#endif /* HEADER_SHARED_MEMORY_SOURCESPLITTERQUEUE_HPP_ */
