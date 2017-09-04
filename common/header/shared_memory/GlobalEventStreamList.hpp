/*
 * GlobalInputStream.hpp
 *
 *  Created on: Jul 21, 2016
 *      Author: sload
 */

#ifndef HEADER_SHARED_MEMORY_GLOBALEVENTSTREAMLIST_HPP_
#define HEADER_SHARED_MEMORY_GLOBALEVENTSTREAMLIST_HPP_

#include "../events/AbstractEvent.hpp"
#include "AbstractGlobalEventStream.hpp"

#include <boost/thread/shared_mutex.hpp>
#include <list>
#include <mutex>        //unique_lock or lock_guard
#include <shared_mutex> //shared_lock c++14

#include <iostream>

using namespace std;

namespace shared_memory
{
/**
 * contains the global shared event stream between all instance and the required operations
 * (e.g. insert, get, delete, ...)
 * All operations are thread-safe using an atomic variable (lastEventSn)
 */
class GlobalEventStreamList
    : public AbstractGlobalEventStream<list<shared_ptr<events::AbstractEvent>> >
{
public:
    /**
     * @brief Ctor
     * @param measurements: for profiling
     */
    GlobalEventStreamList(profiler::Measurements *measurements);

    /**
     * @brief Ctor
     * @param fakeEvent: a fake item to prevent the iterator from accessing end() when it is incremented
     * @param measurements: for profiling
     */
    GlobalEventStreamList(shared_ptr<events::AbstractEvent> fakeEvent, profiler::Measurements *measurements);

    /**
     * insert new event to the end of the event stream
 * Note: the last inserted event should be activate to be used (i.e. call setActivateLastEvent(true)
     * @param  event: pointer to the event (smart pointer)
     * @return iterator to the inserted event
     */
    list<shared_ptr<events::AbstractEvent>>::iterator insert(shared_ptr<events::AbstractEvent> event) override;

    /**
     * get an event in a specific location
     * @param position: event position in the global stream
     * @return a pointer to the event or NULL if the position is out of the range
     */
    shared_ptr<events::AbstractEvent>* get(list<shared_ptr<events::AbstractEvent>>::iterator postion) const override;

    /**
     * get a range of events between startPosition and lastEventSn
     * @param position: event position in the global stream
     * @param lastEventSn: Sn of last event to get
     * @return  a vector of pointers to the events
     */
    vector<shared_ptr<events::AbstractEvent>*> get(list<shared_ptr<events::AbstractEvent>>::iterator startPostion, unsigned long lastEventSn) const override;

    list<shared_ptr<events::AbstractEvent>>::iterator getStartIterator() override;

    /**
     * return the end iterator of the underlying container container
     */
    list<shared_ptr<events::AbstractEvent>>::iterator getEndIterator() override;

    /**
     * @brief clean: remove used events (from removed selection) from the global event stream
     * @param startIterator: first event to delete
     * @param endIterator: last event (will not be delete; iterator semantic)
     */
    void clean(list<shared_ptr<events::AbstractEvent>>::iterator startIterator,
               list<shared_ptr<events::AbstractEvent>>::iterator endIterator) override;

    list<shared_ptr<events::AbstractEvent>>::iterator &
    increment(list<shared_ptr<events::AbstractEvent>>::iterator &it) const override;

    unsigned long getLastEventSn() const override;
    void setLastEventSn(unsigned long value) override;

    /**
     * @brief activate last inserted event in the list. Important to prevent data race
     * when Splitter insert an event to the list and add its iterator to Selections
     * @param activateLastEvent
     */
    //    void setActivateLastEvent(bool activateLastEvent) override;
    /**
     * @brief return the size
     * protected by lock
     */
    size_t size() const override;

    ~GlobalEventStreamList();

private:
    list<shared_ptr<events::AbstractEvent>> globalEventStream;

    atomic<unsigned long> lastEventSn;
};
}

#endif /* HEADER_SHARED_MEMORY_GLOBALEVENTSTREAMLIST_HPP_ */
