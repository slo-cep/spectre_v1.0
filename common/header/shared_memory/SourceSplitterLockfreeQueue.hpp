/*
 * sourcesplitterlockfreequeue
 *
 * Created on: 11.01.2017
 *      Author: sload
 */

#ifndef SOURCESPLITTERLOCKFREEQUEUE_HPP
#define SOURCESPLITTERLOCKFREEQUEUE_HPP

#include "AbstractSourceSplitterStream.hpp"
#include <boost/lockfree/spsc_queue.hpp>

#include <string>
#include <queue>
#include <mutex>


namespace shared_memory
{
/**
 * this class implement the AbstractSourceSplitterStream and
 * it uses a queue as underlying data structure
 * all operations on the queue are thread-safe( using lock)
 * calling examples:SourceSplitterQueue<string> or SourceSplitterQueue< shared_ptr<AbstractEvent>>
 */
class SourceSplitterLockfreeQueue: public AbstractSourceSplitterStream
{
public:
    /**
     * default Constructor
     */
    SourceSplitterLockfreeQueue();
    /**
     * Constructor to limit the queue size
     * @param size_limit: underlying queue size, zero means ubounded queue
     */
    SourceSplitterLockfreeQueue(size_t size_limit);

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

    ~SourceSplitterLockfreeQueue();

private:
    boost::lockfree::spsc_queue<shared_ptr<events::AbstractEvent>> sourceSplitterQueue;
};
}


#endif // SOURCESPLITTERLOCKFREEQUEUE_HPP
