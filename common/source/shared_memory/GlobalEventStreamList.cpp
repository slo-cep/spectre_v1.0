/*
 * GlobalEventInputStream.cpp
 *
 *  Created on: Jul 21, 2016
 *      Author: sload
 */

#include "../../header/shared_memory/GlobalEventStreamList.hpp"

using namespace profiler;
using namespace util;
using namespace events;

namespace shared_memory
{
/**
 * @brief Ctor
 * @param fakeEvent: a fake item to prevent the iterator from accessing end() when it is incremented
 * @param measurements: for profilin
 */
GlobalEventStreamList::GlobalEventStreamList(Measurements *measurements)
    : AbstractGlobalEventStream<list<shared_ptr<AbstractEvent>>>(measurements)
{
}

GlobalEventStreamList::GlobalEventStreamList(shared_ptr<AbstractEvent> fakeEvent, Measurements *measurements)
    : AbstractGlobalEventStream<list<shared_ptr<AbstractEvent>>>(measurements)
{
    this->globalEventStream.push_back(fakeEvent);
}

/**
 * insert new event to the end of the event stream
 * @param  event: pointer to the event (smart pointer)
 * @return iterator to the inserted event
 */
list<shared_ptr<AbstractEvent>>::iterator GlobalEventStreamList::insert(shared_ptr<AbstractEvent> event)
{
    this->globalEventStream.push_back(event);

    auto last_it = --this->globalEventStream.end();
    return last_it;
}

/**
 * @brief activate last inserted event in the list. Important to prevent data race
 * when Splitter insert an event to the list and add its iterator to Selections
 * @param activateLastEvent
 */
// void GlobalEventStreamList<shared_ptr<events::AbstractEvent>>::setActivateLastEvent(bool activateLastEvent)
//{

//    lock_guard<boost::shared_mutex> lk(this->shared_mtx); // write lock (exclusive)
//    this->activateLastEvent = activateLastEvent;
//}

shared_ptr<AbstractEvent> *GlobalEventStreamList::get(list<shared_ptr<AbstractEvent>>::iterator position) const
{
    // start time
    //    unsigned long startTime = Helper::currentTimeMillis();

    shared_ptr<events::AbstractEvent> *result = NULL;
    if (position->get()->getSn() <= this->lastEventSn.load())
        result = &(*position);

    return result;
}

vector<shared_ptr<AbstractEvent> *> GlobalEventStreamList::get(list<shared_ptr<AbstractEvent>>::iterator startPostion,
                                                               unsigned long lastEventSn) const
{
    vector<shared_ptr<AbstractEvent> *> VcResult;
    unsigned long tempLastEventSn= this->lastEventSn;

    do
    {
        unsigned long sn=startPostion->get()->getSn();
        if((sn<= tempLastEventSn) && (sn <= lastEventSn))
        {
            VcResult.push_back(&(*startPostion));
            startPostion++;
        }
        else
            break;
    } while (true);

    return VcResult;
}

/**
 * @brief return the size
 * protected by lock
 */
size_t GlobalEventStreamList::size() const { return this->globalEventStream.size(); }

list<shared_ptr<events::AbstractEvent>>::iterator GlobalEventStreamList::getStartIterator()
{
    return this->globalEventStream.begin();
}

/**
 * return the end iterator of the underlying container container
 */
list<shared_ptr<events::AbstractEvent>>::iterator GlobalEventStreamList::getEndIterator()
{
    return this->globalEventStream.end();
}

void GlobalEventStreamList::clean(list<shared_ptr<events::AbstractEvent>>::iterator startIterator,
                                  list<shared_ptr<events::AbstractEvent>>::iterator endIterator)
{
    this->globalEventStream.erase(startIterator, endIterator);
}

list<shared_ptr<events::AbstractEvent>>::iterator &
GlobalEventStreamList::increment(list<shared_ptr<events::AbstractEvent>>::iterator &it) const
{
    it++;
    return it;
}

unsigned long GlobalEventStreamList::getLastEventSn() const { return lastEventSn.load(); }

void GlobalEventStreamList::setLastEventSn(unsigned long value) { lastEventSn.store(value); }

GlobalEventStreamList::~GlobalEventStreamList() {}
}
