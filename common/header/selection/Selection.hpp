/*
 * Selection.hpp
 *
 *  Created on: Aug 1, 2016
 *      Author: sload
 */

#ifndef HEADER_SELECTION_SELECTION_HPP_
#define HEADER_SELECTION_SELECTION_HPP_

#include "../../header/util/GlobalTypedef.hpp"
#include "../shared_memory/AbstractGlobalEventStream.hpp"

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
 * A Selection is a bunch of events
 *
 * This class contains references to the start events of selection (selectionStartPosition). It is the same as a save
 * point.
 * Also, it contains a reference to the selection end (selectionEndPosition)!
 * Note: the selection is determined in the range [selectionStartPosition,  selectionEndPosition)
 * selectionEndPosition is outside the selection!
 */

class Selection
{
public:
    typedef util::GlobalTypedef::GlobalEventStreamTypedef GlobalEventStreamTypedef;
    typedef util::GlobalTypedef::GlobalEventStreamTypedef_iterator GlobalEventStreamTypedef_iterator;

    /**
     * -generates next Id for a selection
     * -Id should start from one (keep zero for default or uninitialized cases)
     * -selection Id is of type unsigned long: whenever Id is max(unsigned long), it rotates to one again (skip zero)
     * -uses an internal field to store the last generated selection Id.
     * @return next Id to be used as selection Id
     */
    static unsigned long generateNextId();

    /**
     * each selection has a time horizon
     * TimeHorizon determines the position of till now processed primitive events in the selection
     * processed  means events are marked either consumed or not consumed so the following selection can
     * evaluate their complex events
     * Note: each selection depends only on its predecessor selection's TimeHorizon.
     * Operations on TimeHorizon are protected by lock (mutex)
     */
    class TimeHorizon
    {
    public:
        TimeHorizon(Selection *selection);
        TimeHorizon(Selection *selection, GlobalEventStreamTypedef_iterator startPosition);

        /**
         * protected by lock
         * increment the current position:
         * if the currentPosition accesses to the end of the selection:
         *  +set the ended of TimeHorizon to true
         */
        void incrementCurrentPosition();

        /**
         * protected by lock
         */
        GlobalEventStreamTypedef_iterator getCurrentPosition() const;

        /**
         * protected by lock:
         * set the current position:
         * if the currentPosition param refers to the end of the selection:
         *  +set the ended of TimeHorizon to true
         * else:
         *  +set the currentPosition passed param
         */
        void setCurrentPosition(GlobalEventStreamTypedef_iterator currentPosition);

        /**
         * @brief it call getLastPosition function from the associated selection
         * @return an iterator to the last event in the selection
         */
        GlobalEventStreamTypedef_iterator getLastPosition() const;

        /**
         * protected by lock
         */
        bool isEnded() const;

        /**
         * protected by lock
         */
        void setIsEnded(bool isEnded);

        virtual ~TimeHorizon();

    private:
        Selection* selection;
        /*
         * indicates till now processed events exclusively (which means the event in currentPosition is not processed
         * yet)
         */
        GlobalEventStreamTypedef_iterator currentPosition;

        mutable boost::shared_mutex shared_mtx;

        bool ended = false;
    };

    /**
     * Constructor
     * @param id: selection Id
     * @param globalEventStream: reference to the shared event stream between all instances
     * @param startPosition: iterator which points to the first event in the selection
     * @param predecessorSelection: the predecessor of this selection
     */
    Selection(unsigned long id, GlobalEventStreamTypedef &globalEventStream, shared_ptr<Selection> predecessorSelection,
              GlobalEventStreamTypedef_iterator startPosition);

    /**
     *Constructor
     *@param id: selection Id
     *@param globalEventStream: reference to the shared event stream between all instances
     *@param predecessorSelection: the predecessor of this selection
     *@param startPosition: iterator which points to the first event in the selection
     *@param lastPosition: iterator which points to the last event in the selection
     */
    Selection(unsigned long id, GlobalEventStreamTypedef &globalEventStream, shared_ptr<Selection> predecessorSelection,
              GlobalEventStreamTypedef_iterator startPosition, GlobalEventStreamTypedef_iterator lastPosition);

    /**
     * @brief push new event to this selection
     * store its iterator and index
     * @param it: iterator pointing to the newly added event to globalEventStream
     */
    void pushNewEvent(GlobalEventStreamTypedef_iterator it);

    unsigned long getId() const;

    unsigned long getStartTimestamp() const;
    void setStartTimestamp(unsigned long startTimestamp);

    shared_ptr<Selection> getPredecessorSelection() const;

    void setPredecessorSelection(shared_ptr<Selection> predecessorSelection);

    GlobalEventStreamTypedef_iterator getStartPosition() const;

    void setStartPosition(GlobalEventStreamTypedef_iterator startPosition);

    GlobalEventStreamTypedef_iterator getCurrentPosition() const;
    void setCurrentPosition(GlobalEventStreamTypedef_iterator currentPosition);

    void incrementCurrentPosition();

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
     * tells if the selection is closed and all its events are already read by the Feeder!
     * @return true or false
     */
    bool isSelectionCompleted() const;

    /**
     * @return true if all events in this selection is validated by validator and the selection can be deleted
     * 	from the system
     */
    bool isReadyToRemove() const;
    void setReadyToRemove(bool readyToRemove);

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
     * set the feeded events
     * @param isFeeded: true is the events is feeded else false!
     */
    void setFeededEvents(bool isFeeded);

    /**
     * get feeded events
     * @param index: the index of feeded event
     * @return true or false
     */
    bool getFeededEvents(size_t index) const;

    /**
     * set this selection as master selection
     * remove the predecessor selection
     */
    void setMasterSelection();

    /**
     * check if this selection is master selection
     * @return true if the selection is master selection else false :)!
     */
    bool isMasterSelection() const;

    /**
     * should revert the selection when the validator detect invalid feeded events
     */
    bool getRevert() const;

    /**
     *should revert the selection when the validator detect invalid feeded events
     */
    void setRevert();

    /**
     * reset the selection to do revert (re-run)
     */
    void reset();

    /**
     *only for debugging purpose
     */
    string toString() const;

    TimeHorizon &getTimeHorizon();

    virtual ~Selection();

private:
    unsigned long id = 0;                     // each selection has a unique, sequentially increasing, ID
    mutable unsigned long startTimestamp = 0; // time stamp of the event that started the selection

    //iterator->get()->getSn ==> index
    unordered_map<unsigned long, int> iteratorToIndexMap;

    //index ==>iterator
    unordered_map<int, GlobalEventStreamTypedef_iterator> indexToIteratorMap;

    int size;

    GlobalEventStreamTypedef &globalEventStream;

    shared_ptr<Selection> predecessorSelection;
    /*
     * Selection start position (the iterator which points to the first event in the selection)
     */
    GlobalEventStreamTypedef_iterator startPosition;

    // keep the position for next event to be fetched from the global event stream
    GlobalEventStreamTypedef_iterator currentPosition;

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
    bool selectionClosed = false;

    /*
     * this field determines if the selection can be removed from the system (from the scheduler)
     * it is assigned by the Validator( after the Validator validate all events)
     */
    bool readyToRemove = false;

    TimeHorizon timeHorizon;

    mutable boost::shared_mutex shared_mtx_predecessor;
    mutable mutex mtx_current;
    mutable mutex mtx_remove;
    mutable boost::shared_mutex shared_mtx_item;

    // used to generate the selection's Id
    static unsigned long lastSelectionId;

    // keep an array(vector) of all feeded events
    vector<bool> feededEvents;

    /*
     * is this selection is the master selection
     * useful to not check the validity of feeded events after the selection becomes master selection
     */
    bool master = false;

    // should revert the selection when the validator detect invalid feeded events
    bool revert = false;
};

} /* namespace selection */

#endif /* HEADER_SELECTION_SELECTION_HPP_ */
