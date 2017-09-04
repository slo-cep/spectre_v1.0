/*
 * AbstractSelection.cpp
 *
 * Created on: 17.10.2016
 *      Author: sload
 */

#include "../../header/selection/AbstractSelection.hpp"

using namespace profiler;

namespace selection
{

/**
 * -generates next Id for a selection
 * -Id should start from one (keep zero for default or uninitialized cases)
 * -selection Id is of type unsigned long: whenever Id is max(unsigned long), it rotates to one again (skip zero)
 * -uses an internal field to store the last generated selection Id.
 * @return next Id to be used as selection Id
 */
unsigned long AbstractSelection::generateNextId()
{
    if (lastSelectionId == numeric_limits<unsigned long>::max())
        return 1;
    else
        return ++lastSelectionId;
}
unsigned long AbstractSelection::lastSelectionId = 0;

/**
 * Constructor
 * @param id: selection Id
 * @param globalEventStream: reference to the shared event stream between all instances
 * @param startPosition: iterator which points to the first event in the selection
 * @param measurements: for profiling
 */
AbstractSelection::AbstractSelection(unsigned long id, GlobalEventStreamTypedef &globalEventStream,
                                     AbstractSelection *predecessorSelection,
                                     GlobalEventStreamTypedef_iterator startPosition, Measurements *measurements)
    : measurements(measurements), predecessorSelection(predecessorSelection)
{
    this->basicSelection = make_shared<BasicSelectionWithoutLock>(id, globalEventStream, startPosition);
}

AbstractSelection::AbstractSelection(const AbstractSelection &other)
{
    this->basicSelection = other.basicSelection;
    this->predecessorSelection = other.predecessorSelection;
    this->measurements = other.measurements;
}

/**
 * @brief push new event to this selection
 * store its iterator and index
 * @param it: iterator pointing to the newly added event to globalEventStream
 */
// void AbstractSelection::pushNewEvent(GlobalEventStreamTypedef_iterator it)
//{
//    this->basicSelection.get()->pushNewEvent(it);
//}

unsigned long AbstractSelection::getId() const { return this->basicSelection.get()->getId(); }

unsigned long AbstractSelection::getStartTimestamp() const { return this->basicSelection.get()->getStartTimestamp(); }

void AbstractSelection::setStartTimestamp(unsigned long startTimestamp)
{
    this->basicSelection.get()->setStartTimestamp(startTimestamp);
}

AbstractSelection::GlobalEventStreamTypedef &AbstractSelection::getGlobalEventStream()
{
    return this->basicSelection.get()->getGlobalEventStream();
}

AbstractSelection *AbstractSelection::getPredecessorSelection() const { return this->predecessorSelection; }

const shared_ptr<BasicSelectionWithoutLock> &AbstractSelection::getBasicSelection() const
{
    return this->basicSelection;
}

typename AbstractSelection::GlobalEventStreamTypedef_iterator AbstractSelection::getStartPosition() const
{
    return this->basicSelection.get()->getStartPosition();
}

unsigned long AbstractSelection::getLastEventSn() const { return basicSelection->getLastEventSn(); }

void AbstractSelection::setLastEventSn(unsigned long value) { basicSelection->setLastEventSn(value); }

/**
 * protected by lock (mutex)
 */
// typename AbstractSelection::GlobalEventStreamTypedef_iterator AbstractSelection::getLastPosition() const
//{
//    return this->basicSelection.get()->getLastPosition();
//}

/**
 * protected by lock (mutex)
 */
// void AbstractSelection::setLastPosition(GlobalEventStreamTypedef_iterator lastPosition)
//{
//    this->basicSelection.get()->setLastPosition(lastPosition);
//}

/**
 * @brief find the index of an event by having its iterator
 * @param it: iterator to an event to get its index
 * @return event's index if the event found
 *         else -1
 */
// int AbstractSelection::getEventIndex(GlobalEventStreamTypedef_iterator it)
//{
//    return this->basicSelection.get()->getEventIndex(it);
//}

/**
 * @brief find the iterator of an event by having its index
 * @param index: index of an event to get its iterator
 * @return event's iterator if the event found
 *         else endPosition iterator
 */
// typename AbstractSelection::GlobalEventStreamTypedef_iterator AbstractSelection::getEventIterator(int index)
//{
//    return this->basicSelection.get()->getEventIterator(index);
//}

// typename AbstractSelection::GlobalEventStreamTypedef_iterator AbstractSelection::getEndPosition() const
//{
//    return this->basicSelection.get()->getEndPosition();
//}

// int AbstractSelection::getSize() const { return this->basicSelection.get()->getSize(); }

/**
 *protected by lock(mutex) because it can be access by Splitter and Validator at least
 *Note: this function doesn't tell if all events from the selection are already read. It only tells
 *that the selection is closed from the Splitter but there may be still some events in the selection which can be read
 *by the Feeder.
 *However isSelectionCompleted() tells if the selection is closed and all the events are already read by the Feeder!
 *@return true if the selection is closed else false
 */
bool AbstractSelection::isSelectionClosed() const { return this->basicSelection.get()->isSelectionClosed(); }

bool AbstractSelection::isReadyToBeRemoved() const { return this->basicSelection->isReadyToBeRemoved(); }

void AbstractSelection::setReadyToBeRemoved(bool readyToRemove)
{
    this->basicSelection->setReadyToBeRemoved(readyToRemove);
}

/**
 *only for debugging purpose
 */
string AbstractSelection::toString() const
{
    stringstream strm;
    unsigned int count=0;
    strm << "{SelectionId: " << this->getId() << "}, {";
    for (auto it_primitives = this->getStartPosition();;)
    {
        count++;
        strm << "(" << it_primitives->get()->getSn();

        if (it_primitives->get()->getContent().count("SS") != 0)
            strm << ", " << it_primitives->get()->getContent().at("SS");
        else
            if(it_primitives->get()->getContent().size()!=0)
                strm << ", " << it_primitives->get()->getContent().begin()->second;
        strm << ")";

        if (it_primitives->get()->getSn() < this->getBasicSelection()->getLastEventSn())
        {
            it_primitives++;
            strm << ", ";
        }
        else
            break;
    }

    strm << " }";

    strm<< ", {#event: "<<count<<"}";
    return strm.str();
}

AbstractSelection::~AbstractSelection()
{
    // TODO Auto-generated destructor stub
}
}
