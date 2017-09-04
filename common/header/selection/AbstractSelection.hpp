/*
 * AbstractSelection.hpp
 *
 * Created on: 17.10.2016
 *      Author: sload
 */

#ifndef ABSTRACTSELECTION_HPP
#define ABSTRACTSELECTION_HPP

#include "header/Measurements.hpp"

#include "../../header/util/GlobalTypedef.hpp"
#include "../../header/util/Helper.hpp"

#include "../shared_memory/AbstractGlobalEventStream.hpp"

//#include "BasicSelection.hpp"
#include "BasicSelectionWithoutLock.hpp"

#include <boost/thread/shared_mutex.hpp>
#include <iostream>
#include <limits>
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

class AbstractSelection
{
public:
    typedef util::GlobalTypedef::GlobalEventStreamTypedef GlobalEventStreamTypedef;
    typedef util::GlobalTypedef::GlobalEventStreamTypedef_iterator GlobalEventStreamTypedef_iterator;

    virtual shared_ptr<AbstractSelection> clone() = 0;
    /**
     * -generates next Id for a selection
     * -Id should start from one (keep zero for default or uninitialized cases)
     * -selection Id is of type unsigned long: whenever Id is max(unsigned long), it rotates to one again (skip zero)
     * -uses an internal field to store the last generated selection Id.
     * @return next Id to be used as selection Id
     */
    static unsigned long generateNextId();

    /**
     * @brief push new event to this selection
     * store its iterator and index
     * @param it: iterator pointing to the newly added event to globalEventStream
     */
//    void pushNewEvent(GlobalEventStreamTypedef_iterator it);

    unsigned long getId() const;

    unsigned long getStartTimestamp() const;
    void setStartTimestamp(unsigned long startTimestamp);

    GlobalEventStreamTypedef &getGlobalEventStream();

    AbstractSelection* getPredecessorSelection() const;

    const shared_ptr<BasicSelectionWithoutLock>& getBasicSelection() const;


    GlobalEventStreamTypedef_iterator getStartPosition() const;

    unsigned long getLastEventSn() const;
    void setLastEventSn(unsigned long value);

    virtual GlobalEventStreamTypedef_iterator getCurrentPosition() const = 0;
    virtual void setCurrentPosition(GlobalEventStreamTypedef_iterator currentPosition) = 0;
    virtual void incrementCurrentPosition() = 0;

    /**
     * protected by lock(mutex) because it can be access by Splitter and Feeder at least
     */
//    GlobalEventStreamTypedef_iterator getLastPosition() const;
    /**
     * protected by lock(mutex) because it can be access by Splitter and Feeder at least
     */
//    void setLastPosition(GlobalEventStreamTypedef_iterator lastPosition);

    /**
     * @brief find the index of an event by having its iterator
     * @param it: iterator to an event to get its index
     * @return event's index if the event found
     *         else -1
     */
//    int getEventIndex(GlobalEventStreamTypedef_iterator it);

    /**
     * @brief find the iterator of an event by having its index
     * @param index: index of an event to get its iterator
     * @return event's iterator if the event found
     *         else endPosition iterator
     */
//    GlobalEventStreamTypedef_iterator getEventIterator(int index);

//    GlobalEventStreamTypedef_iterator getEndPosition() const;

//    int getSize() const;

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
    virtual bool isSelectionCompleted()  = 0;

    /**
     * @return true if all events in this selection is validated by validator and the selection can be deleted
     * 	from the system
     */
    bool isReadyToBeRemoved() const ;
    void setReadyToBeRemoved(bool readyToRemove);

    /**
     *only for debugging purpose
     */
    virtual string toString() const;

    virtual ~AbstractSelection();

protected:
    /**
     * Constructor
     * @param id: selection Id
     * @param globalEventStream: reference to the shared event stream between all instances
     * @param startPosition: iterator which points to the first event in the selection
     * @param predecessorSelection: the predecessor of this selection
     * @param measurements: for profiling
     */
    AbstractSelection(unsigned long id, GlobalEventStreamTypedef &globalEventStream,
                      AbstractSelection* predecessorSelection,
                      GlobalEventStreamTypedef_iterator startPosition,
                      profiler::Measurements *measurements);


    /**
     * @brief AbstractSelection: Copy constructor
     * @param other: rhs
     */
    AbstractSelection(const AbstractSelection &other);

    profiler::Measurements* measurements;

private:
    shared_ptr<BasicSelectionWithoutLock> basicSelection;

    AbstractSelection* predecessorSelection;

    // used to generate the selection's Id
    static unsigned long lastSelectionId;
};

} /* namespace selection */

#endif // ABSTRACTSELECTION_HPP
