/*
 * BasicSelectionWithMinimumLock.hpp
 *
 * Created on: 28.10.2016
 *      Author: sload
 */

#ifndef BASICSELECTIONWITHMINIMUMLOCK_HPP
#define BASICSELECTIONWITHMINIMUMLOCK_HPP

#include "../../header/util/GlobalTypedef.hpp"
#include "../shared_memory/AbstractGlobalEventStream.hpp"

#include <atomic>
#include <boost/thread/shared_mutex.hpp>
#include <iostream>
#include <mutex>
#include <shared_mutex> //shared_lock
#include <string>
#include <unordered_map>

using namespace std;

namespace selection
{
/**
 * @brief The BasicSelection class: more than one selection could refer to same basic selection
 * (start, last, end events but not to current position, readyToRemove, etc.). Therfore, I created this class
 * to hold the basic selection's information
 * ----------------------------
 *
 * A Selection is a bunch of events
 *
 * This class contains references to the start events of selection (selectionStartPosition). It is the same as a save
 * point.
 * Also, it contains a reference to the selection end (selectionEndPosition)!
 * Note: the selection is determined in the range [selectionStartPosition,  selectionEndPosition)
 * selectionEndPosition is outside the selection!
 *
 */
class BasicSelectionWithMinimumLock
{
public:
    typedef util::GlobalTypedef::GlobalEventStreamTypedef GlobalEventStreamTypedef;
    typedef util::GlobalTypedef::GlobalEventStreamTypedef_iterator GlobalEventStreamTypedef_iterator;

    /**
     * Constructor
     * @param id: selection Id
     * @param globalEventStream: reference to the shared event stream between all instances
     * @param startPosition: iterator which points to the first event in the selection
     */
    BasicSelectionWithMinimumLock(unsigned long id, GlobalEventStreamTypedef &globalEventStream,
                                  GlobalEventStreamTypedef_iterator startPosition);

    /**
     *Constructor
     *@param id: selection Id
     *@param globalEventStream: reference to the shared event stream between all instances
     *@param startPosition: iterator which points to the first event in the selection
     *@param lastPosition: iterator which points to the last event in the selection
     */
    BasicSelectionWithMinimumLock(unsigned long id, GlobalEventStreamTypedef &globalEventStream,
                                  GlobalEventStreamTypedef_iterator startPosition,
                                  GlobalEventStreamTypedef_iterator lastPosition);

    /**
     * @brief push new event to this selection
     * store its iterator and index
     * @param it: iterator pointing to the newly added event to globalEventStream
     */
    void pushNewEvent(GlobalEventStreamTypedef_iterator it);

    unsigned long getId() const;

    unsigned long getStartTimestamp() const;
    void setStartTimestamp(unsigned long startTimestamp);

    GlobalEventStreamTypedef &getGlobalEventStream();

    GlobalEventStreamTypedef_iterator getStartPosition() const;

    void setStartPosition(GlobalEventStreamTypedef_iterator startPosition);

    /**
     * protected by lock(mutex) because it can be access by Splitter and Feeder at least
     */
    GlobalEventStreamTypedef_iterator getLastPosition() const;
    /**
     * protected by lock(mutex) because it can be access by Splitter and Feeder at least
     */
    void setLastPosition(GlobalEventStreamTypedef_iterator lastPosition);

    /**
     * @brief find the index of an event by having its iterator
     * @param it: iterator to an event to get its index
     * @return event's index if the event found
     *         else -1
     */
    int getEventIndex(GlobalEventStreamTypedef_iterator it);

    /**
     * @brief find the iterator of an event by having its index
     * @param index: index of an event to get its iterator
     * @return event's iterator if the event found
     *         else endPosition iterator
     */
    GlobalEventStreamTypedef_iterator getEventIterator(int index);

    GlobalEventStreamTypedef_iterator getEndPosition() const;

    int getSize() const;

    /**
     *protected by lock(mutex) because it can be access by Splitter and Validator at least
     *Note: this function doesn't tell if all events from the selection are already read. It only tells
     *that the selection is closed from the Splitter but there may be still some events in the selection which can be
     *read by the Feeder.
     *However isSelectionCompleted() tells if the selection is closed and all the events are already read by the Feeder!
     *@return true if the selection is closed else false
     */
    bool isSelectionClosed() const;

    /**
     * check if this selection has equal or higher priority than the one in the parameter
     * @param Id: the Id of the selection to compare with the Id of this selection
     *
     * @return true: if this selection has equal or higher priority
     * 				else false
     */
    bool hasEqualOrHigherPriority(unsigned long Id) const;

    /**
     * check if this selection has higher priority than the one in the parameter
     * @param Id: the Id of the selection to compare with the Id of this selection
     *
     * @return true: if this selection has higher priority
     * 				else false
     */
    bool hasHigherPriority(unsigned long Id) const;

    /**
     * @return true if all events in this selection is validated by validator and the selection can be deleted
     * 	from the system
     */
    bool isReadyToBeRemoved();
    void setReadyToBeRemoved(bool readyToBeRemoved);


    virtual ~BasicSelectionWithMinimumLock();

private:
    unsigned long id = 0;                     // each selection has a unique, sequentially increasing, ID
    mutable unsigned long startTimestamp = 0; // time stamp of the event that started the selection

    // iterator->get()->getSn ==> index
    unordered_map<unsigned long, int> iteratorToIndexMap;

    // index ==>iterator
    unordered_map<int, GlobalEventStreamTypedef_iterator> indexToIteratorMap;

    int size;

    GlobalEventStreamTypedef &globalEventStream;

    /*
     * Selection start position (the iterator which points to the first event in the selection)
     */
    GlobalEventStreamTypedef_iterator startPosition;

    /*
     * Selection last position (the iterator which points to the last event in the selection)
     */
    GlobalEventStreamTypedef_iterator lastPosition;

    /*
     * Selection end position (the iterator which points to after the last event in the selection)
     */
    GlobalEventStreamTypedef_iterator endPosition;

    /*
     * this field is useful to avoid the comparing with uninitialized end or lastPosition!
     * whenever there is an lastPosition, this field set to true
     */
    atomic<bool> selectionClosed{false};

    mutable boost::shared_mutex shared_mtx_item;

    /*
     * this field determines if the selection can be removed from the system (from the scheduler)
     * it is assigned by the Validator( after the Validator validate all events)
     */
    atomic<bool> readyToBeRemoved {false};
};
}

#endif // BASICSELECTIONWITHMINIMUMLOCK_HPP
