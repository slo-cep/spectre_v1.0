/*
 * BasicSelectionWithoutLock.cpp
 *
 * Created on: 05.01.2017
 *      Author: sload
 */


#include "../../header/selection/BasicSelectionWithoutLock.hpp"

namespace selection
{

/**
 * Constructor
 * @param id: selection Id
 * @param globalEventStream: reference to the shared event stream between all instances
 * @param startPosition: iterator which points to the first event in the selection
 */
BasicSelectionWithoutLock::BasicSelectionWithoutLock(unsigned long id,
                                                             GlobalEventStreamTypedef &globalEventStream,
                                                             GlobalEventStreamTypedef_iterator startPosition)
    : id(id), globalEventStream(globalEventStream), startPosition(startPosition)

{

//    this->size = 1;
//    this->iteratorToIndexMap.insert({startPosition->get()->getSn(), this->size - 1});
//    this->indexToIteratorMap.insert({this->size - 1, startPosition});
}


/**
 * @brief push new event to this selection
 * store its iterator and index
 * @param it: iterator pointing to the newly added event to globalEventStream
 */
//void BasicSelectionWithoutLock::pushNewEvent(GlobalEventStreamTypedef_iterator it)
//{
//    lock_guard<boost::shared_mutex> lk_shared(this->shared_mtx_item);
//    this->size++;
//    this->iteratorToIndexMap.insert({it->get()->getSn(), this->size - 1});
//    this->indexToIteratorMap.insert({this->size - 1, it});
//}

unsigned long BasicSelectionWithoutLock::getId() const { return this->id; }

unsigned long BasicSelectionWithoutLock::getStartTimestamp() const
{
    if (this->startTimestamp == 0)
        this->startTimestamp = this->startPosition->get()->getTimestamp();

    return this->startTimestamp;
}

void BasicSelectionWithoutLock::setStartTimestamp(unsigned long startTimestamp)
{
    this->startTimestamp = startTimestamp;
}

BasicSelectionWithoutLock::GlobalEventStreamTypedef &BasicSelectionWithoutLock::getGlobalEventStream()
{
    return this->globalEventStream;
}

typename BasicSelectionWithoutLock::GlobalEventStreamTypedef_iterator
BasicSelectionWithoutLock::getStartPosition() const
{
    return this->startPosition;
}

void BasicSelectionWithoutLock::setStartPosition(GlobalEventStreamTypedef_iterator startPosition)
{
    this->startPosition = startPosition;

//    this->pushNewEvent(startPosition);
}


unsigned long BasicSelectionWithoutLock::getLastEventSn() const
{
    return lastEventSn.load();
}

void BasicSelectionWithoutLock::setLastEventSn(unsigned long value)
{
    lastEventSn.store( value);

    this->selectionClosed.store(true);
}

/**
 * protected by lock (mutex)
 */
//typename BasicSelectionWithoutLock::GlobalEventStreamTypedef_iterator
//BasicSelectionWithoutLock::getLastPosition() const
//{
//    shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx_item); // read lock (shared)
//    return this->lastPosition;
//}

/**
 * protected by lock (mutex)
 */
//void BasicSelectionWithoutLock::setLastPosition(GlobalEventStreamTypedef_iterator lastPosition)
//{
//    lock_guard<boost::shared_mutex> lk(this->shared_mtx_item); // write lock (exclusive)
//    this->lastPosition = lastPosition;

//    // whenever having lastPosition, that means the selection is closed
//    this->selectionClosed.store(true);
//    // this->selectionClosed= true;

//    auto tempIt = lastPosition;
//    tempIt++;
//    this->endPosition = tempIt;

//    this->size++;
//    this->iteratorToIndexMap.insert({lastPosition->get()->getSn(), this->size - 1});
//    this->indexToIteratorMap.insert({this->size - 1, lastPosition});
//}

/**
 * @brief find the index of an event by having its iterator
 * @param it: iterator to an event to get its index
 * @return event's index if the event found
 *         else -1
 */
//int BasicSelectionWithoutLock::getEventIndex(GlobalEventStreamTypedef_iterator it)
//{
//    shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx_item); // read lock (shared)

//    if (this->iteratorToIndexMap.count(it->get()->getSn()) != 0)
//        return this->iteratorToIndexMap[it->get()->getSn()];
//    else
//        return -1;
//}

/**
 * @brief find the iterator of an event by having its index
 * @param index: index of an event to get its iterator
 * @return event's iterator if the event found
 *         else endPosition iterator
 */
//typename BasicSelectionWithoutLock::GlobalEventStreamTypedef_iterator
//BasicSelectionWithoutLock::getEventIterator(int index)
//{
//    shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx_item); // read lock (shared)

//    if (this->indexToIteratorMap.count(index) != 0)
//        return this->indexToIteratorMap[index];
//    else
//        return this->endPosition;
//}

//typename BasicSelectionWithoutLock::GlobalEventStreamTypedef_iterator
//BasicSelectionWithoutLock::getEndPosition() const
//{
//    //    shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx_item); // read lock (shared)
//    return this->endPosition;
//}

//int BasicSelectionWithoutLock::getSize() const
//{
//    shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx_item); // read lock (shared)
//    return this->size;
//}

/**
 *protected by lock(mutex) because it can be access by Splitter and Validator at least
 *Note: this function doesn't tell if all events from the selection are already read. It only tells
 *that the selection is closed from the Splitter but there may be still some events in the selection which can be read
 *by the Feeder.
 *However isSelectionCompleted() tells if the selection is closed and all the events are already read by the Feeder!
 *@return true if the selection is closed else false
 */
bool BasicSelectionWithoutLock::isSelectionClosed() const
{
    //    shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx_item); // read lock (shared)
    //    return this->selectionClosed;

    return this->selectionClosed.load();
}

/**
 * check if this selection has equal or higher priority than the one in the parameter
 * @param Id: the Id of the selection to compare with the Id of this selection
 *
 * @return true: if this selection has equal or higher priority
 * 				else false
 */


bool BasicSelectionWithoutLock::isReadyToBeRemoved() { return this->readyToBeRemoved.load(); }

void BasicSelectionWithoutLock::setReadyToBeRemoved(bool readyToBeRemoved)
{
    this->readyToBeRemoved.store(readyToBeRemoved);
}

BasicSelectionWithoutLock::~BasicSelectionWithoutLock()
{
    // TODO Auto-generated destructor stub
}


}

