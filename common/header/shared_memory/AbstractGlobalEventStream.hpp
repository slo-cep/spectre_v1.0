/*
 * AbstractGlobalEventInputStream.hppd
 *
 *  Created on: Jul 22, 2016
 *      Author: sload
 */

#ifndef HEADER_SHARED_MEMORY_ABSTRACTGLOBALEVENTSTREAM_HPP_
#define HEADER_SHARED_MEMORY_ABSTRACTGLOBALEVENTSTREAM_HPP_
#include "header/Measurements.hpp"

#include "../events/AbstractEvent.hpp"

#include "../util/Helper.hpp"

#include <iterator>

namespace shared_memory
{
/**
 * contains An Abstract global shared event stream between all instance and the required operations
 * (e.g. insert, get, delete, ...)
 * e.g. AbstractGlobalEventStream < list<shared_ptr<AbstractEvent> > >
 */
template <typename Container> class AbstractGlobalEventStream
{
public:
    typedef typename Container::iterator iterator;

    /**
     * insert new event to the end of the event  stream
     * @param  event: the event
     * @return iterator to the inserted event
     *
     */
    virtual iterator insert(shared_ptr<events::AbstractEvent> event) = 0;

    /**
     * get an event in a specific location
     * @param position: event position in the global stream
     * @return a pointer to the event or NULL if the position is out of the range
     */
    virtual shared_ptr<events::AbstractEvent> *get(typename Container::iterator postion) const = 0;

    /**
     * get a range of events between startPosition and lastEventSn
     * @param position: event position in the global stream
     * @param lastEventSn: Sn of last event to get
     * @return  a vector of pointers to the events
     */
    virtual vector<shared_ptr<events::AbstractEvent> *> get(typename Container::iterator startPostion,
                                                            unsigned long lastEventSn) const = 0;

    virtual iterator getStartIterator() = 0;

    /**
     * return the end iterator of the underlying container container
     */
    virtual iterator getEndIterator() = 0;

    virtual void clean(iterator startIterator, iterator endIterator) = 0;

    virtual iterator &increment(iterator &it) const = 0;

    virtual unsigned long getLastEventSn() const = 0;
    virtual void setLastEventSn(unsigned long value) = 0;

    /**
     * @brief activate last inserted event in the list. Important to prevent data race
     * when Splitter insert an event to the list and add its iterator to Selections
     * @param activateLastEvent
     */
    //    virtual void setActivateLastEvent(bool activateLastEvent) = 0;

    /**
     * @brief return the size
     */
    virtual size_t size() const = 0;

    virtual ~AbstractGlobalEventStream() {}

protected:
    AbstractGlobalEventStream(profiler::Measurements *measurements) { this->measurements = measurements; }

    profiler::Measurements *measurements;
};
}

#endif /* HEADER_SHARED_MEMORY_ABSTRACTGLOBALEVENTSTREAM_HPP_ */
