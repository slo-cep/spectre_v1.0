/*
 * Selection.cpp
 *
 *  Created on: Aug 1, 2016
 *      Author: sload
 */

#include "../../header/selection/Selection.hpp"

#include <limits>

using namespace std;

namespace selection
{
/**
 * -generates next Id for a selection
 * -Id should start from one (keep zero for default or uninitialized cases)
 * -selection Id is of type unsigned long: whenever Id is max(unsigned long), it rotates to one again (skip zero)
 * -uses an internal field to store the last generated selection Id.
 * @return next Id to be used as selection Id
 */
unsigned long Selection::generateNextId()
{
    if (lastSelectionId == numeric_limits<unsigned long>::max())
        return 1;
    else
        return ++lastSelectionId;
}

unsigned long Selection::lastSelectionId = 0;

/**
 * Constructor
 * @param id: selection Id
 * @param globalEventStream: reference to the shared event stream between all instances
 * @param startPosition: iterator which points to the first event in the selection
 */
Selection::Selection(unsigned long id, GlobalEventStreamTypedef &globalEventStream,
                     shared_ptr<Selection> predecessorSelection, GlobalEventStreamTypedef_iterator startPosition)
    : id(id), globalEventStream(globalEventStream), predecessorSelection(predecessorSelection),
      startPosition(startPosition), currentPosition(startPosition), endPosition(globalEventStream.getEndIterator()),
      timeHorizon(this, startPosition)
{

    this->size=1;
    this->iteratorToIndexMap.insert({startPosition->get()->getSn(), this->size-1});
    this->indexToIteratorMap.insert({this->size-1, startPosition});
}

/**
 *Constructor
 *@param id: selection Id
 *@param globalEventStream: reference to the shared event stream between all instances
 *@param startPosition: iterator which points to the first event in the selection
 *@param lastPosition: iterator which points to the last event in the selection
 */
Selection::Selection(unsigned long id, GlobalEventStreamTypedef &globalEventStream,
                     shared_ptr<Selection> predecessorSelection, GlobalEventStreamTypedef_iterator startPosition,
                     GlobalEventStreamTypedef_iterator lastPosition)
    : id(id), globalEventStream(globalEventStream), predecessorSelection(predecessorSelection),
      startPosition(startPosition), lastPosition(lastPosition), timeHorizon(this, startPosition)
{
    // set the current position to the first position
    currentPosition = startPosition;

    // whenever having lastPosition, that means the selection is closed
    this->selectionClosed = true;
    this->endPosition = lastPosition;
    this->endPosition++;

    this->size=1;
    this->iteratorToIndexMap.insert({startPosition->get()->getSn(), this->size-1});
    this->indexToIteratorMap.insert({this->size-1, startPosition});
}


/**
 * @brief push new event to this selection
 * store its iterator and index
 * @param it: iterator pointing to the newly added event to globalEventStream
 */
void Selection::pushNewEvent(GlobalEventStreamTypedef_iterator it)
{
    lock_guard<boost::shared_mutex> lk_shared(this->shared_mtx_item);
    this->size++;
    this->iteratorToIndexMap.insert({it->get()->getSn(), this->size-1});
    this->indexToIteratorMap.insert({this->size-1, it});
}

unsigned long Selection::getId() const { return this->id; }

unsigned long Selection::getStartTimestamp() const
{
    if (this->startTimestamp == 0)
        this->startTimestamp = this->startPosition->get()->getTimestamp();

    return this->startTimestamp;
}

void Selection::setStartTimestamp(unsigned long startTimestamp) { this->startTimestamp = startTimestamp; }

shared_ptr<Selection> Selection::getPredecessorSelection() const
{

    shared_lock<boost::shared_mutex> lk_shared(this->shared_mtx_predecessor);

    return this->predecessorSelection;
}

void Selection::setPredecessorSelection(shared_ptr<Selection> predecessorSelection)
{
    lock_guard<boost::shared_mutex> lk_shared(this->shared_mtx_predecessor);

    this->predecessorSelection = predecessorSelection;
}

typename Selection::GlobalEventStreamTypedef_iterator Selection::getStartPosition() const
{
    return this->startPosition;
}

void Selection::setStartPosition(GlobalEventStreamTypedef_iterator startPosition)
{
    this->startPosition = startPosition;
    // set the current position to the first Position
    this->setCurrentPosition(startPosition);
    this->pushNewEvent(startPosition);
}

typename Selection::GlobalEventStreamTypedef_iterator Selection::getCurrentPosition() const
{
    //shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx_item); // read lock (shared)
    lock_guard<mutex> lk(this->mtx_current);
    return this->currentPosition;
}

void Selection::setCurrentPosition(Selection::GlobalEventStreamTypedef_iterator currentPosition)
{
    //lock_guard<boost::shared_mutex> lk(this->shared_mtx_item); // write lock (exclusive)
    lock_guard<mutex> lk(this->mtx_current);
    this->currentPosition = currentPosition;
}

void Selection::incrementCurrentPosition()
{
    //lock_guard<boost::shared_mutex> lk(this->shared_mtx_item); // write lock (exclusive)
    lock_guard<mutex> lk(this->mtx_current);

    this->currentPosition = this->globalEventStream.increment(this->currentPosition);
}

/**
 * protected by lock (mutex)
 */
typename Selection::GlobalEventStreamTypedef_iterator Selection::getLastPosition() const
{
    shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx_item); // read lock (shared)
    return this->lastPosition;
}

/**
 * protected by lock (mutex)
 */
void Selection::setLastPosition(GlobalEventStreamTypedef_iterator lastPosition)
{
    lock_guard<boost::shared_mutex> lk(this->shared_mtx_item); // write lock (exclusive)
    this->lastPosition = lastPosition;

    // whenever having lastPosition, that means the selection is closed
    this->selectionClosed = true;
    this->endPosition = lastPosition;
    this->endPosition++;

    this->size++;
    this->iteratorToIndexMap.insert({lastPosition->get()->getSn(), this->size-1});
    this->indexToIteratorMap.insert({this->size-1, lastPosition});
}


/**
 * @brief find the index of an event by having its iterator
 * @param it: iterator to an event to get its index
 * @return event's index if the event found
 *         else -1
 */
int Selection::getEventIndex(GlobalEventStreamTypedef_iterator it)
{
   shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx_item); // read lock (shared)

   if( this->iteratorToIndexMap.count(it->get()->getSn())!=0)
       return this->iteratorToIndexMap[it->get()->getSn()];
   else
       return -1;
}

/**
 * @brief find the iterator of an event by having its index
 * @param index: index of an event to get its iterator
 * @return event's iterator if the event found
 *         else endPosition iterator
 */
typename Selection::GlobalEventStreamTypedef_iterator Selection::getEventIterator(int index)
{
    shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx_item); // read lock (shared)

    if( this->indexToIteratorMap.count(index)!=0)
        return this->indexToIteratorMap[index];
    else
        return this->endPosition;
}

typename Selection::GlobalEventStreamTypedef_iterator Selection::getEndPosition() const
{
    shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx_item); // read lock (shared)
    return this->endPosition;
}

/**
 *protected by lock(mutex) because it can be access by Splitter and Validator at least
 *Note: this function doesn't tell if all events from the selection are already read. It only tells
 *that the selection is closed from the Splitter but there may be still some events in the selection which can be read
 *by the Feeder.
 *However isSelectionCompleted() tells if the selection is closed and all the events are already read by the Feeder!
 *@return true if the selection is closed else false
 */
bool Selection::isSelectionClosed() const
{
    shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx_item); // read lock (shared)
    return this->selectionClosed;
}

/**
 * tells if the selection is closed and all its events are already read by the Feeder!
 * @return true or false
 */
bool Selection::isSelectionCompleted() const
{
//   shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx_item); // read lock (shared)
    //return this->selectionClosed && (this->currentPosition == this->endPosition);

    lock_guard<mutex> lk(this->mtx_current);
    bool selectionClosedTemp= this->isSelectionClosed();
    auto endPostionTemp=this->getEndPosition();

    return selectionClosedTemp && (this->currentPosition == endPostionTemp);
}

/**
 * @return true if all events in this selection is validated by validator and the selection can be deleted
 * 	from the system
 */
bool Selection::isReadyToRemove() const
{
    lock_guard<mutex> lk(mtx_remove);
    return this->readyToRemove;
}

void Selection::setReadyToRemove(bool readyToRemove)
{
    lock_guard<mutex> lk(mtx_remove);
    this->readyToRemove = readyToRemove;
}

/**
 * check if this selection has equal or higher priority than the one in the parameter
 * @param Id: the Id of the selection to compare with the Id of this selection
 *
 * @return true: if this selection has equal or higher priority
 * 				else false
 */
bool Selection::hasEqualOrHigherPriority(unsigned long Id) const
{
    // I didn't consider the rotation of selection Id
    // there is no selection with Id=0
    if ((Id == 0) || (this->id <= Id))
        return true;
    else
        return false;
}

/**
 * check if this selection has higher priority than the one in the parameter
 * @param Id: the Id of the selection to compare with the Id of this selection
 *
 * @return true: if this selection has higher priority
 * 				else false
 */
bool Selection::hasHigherPriority(unsigned long Id) const
{
    // I didn't consider the rotation of selection Id
    // there is no selection with Id=0
    if ((Id == 0) || (this->id < Id))
        return true;
    else
        return false;
}

/**
 * set the feeded events
 */
void Selection::setFeededEvents(bool isFeeded) { this->feededEvents.push_back(isFeeded); }

/**
 * get feeded events
 * @param index: the index of feeded event
 * @return true or false
 */
bool Selection::getFeededEvents(size_t index) const { return this->feededEvents[index]; }

/**
 * set this selection as master selection
 * remove the predecessor selection
 */
void Selection::setMasterSelection()
{
    this->master = true;
    this->setPredecessorSelection(nullptr);
}

/**
 * check if this selection is master selection
 * @return true if the selection is master selection else false :)!
 */
bool Selection::isMasterSelection() const { return this->master; }

/**
 * should revert the selection when the validator detect invalid feeded events
 */
bool Selection::getRevert() const { return this->revert; }

/**
 *should revert the selection when the validator detect invalid feeded events
 */
void Selection::setRevert() { this->revert = true; }

/**
 * reset the selection to do revert (re-run)
 */
void Selection::reset()
{

    lock(this->shared_mtx_predecessor, this->shared_mtx_item, this->mtx_remove);

    this->currentPosition = this->timeHorizon.getCurrentPosition();
    this->readyToRemove = false;
    this->revert = false;

    //
    if(this->currentPosition== this->startPosition)
        this->feededEvents.clear();
    else
    {
        auto it=this->currentPosition;
        it--;
        int index =this->getEventIndex(it);
        this->feededEvents.erase(this->feededEvents.begin()+ index, this->feededEvents.end());
    }


    this->shared_mtx_predecessor.unlock();
    this->shared_mtx_item.unlock();
    this->mtx_remove.unlock();
}

typename Selection::TimeHorizon &Selection::getTimeHorizon() { return this->timeHorizon; }

/**
 *only for debugging purpose
 */
string Selection::toString() const
{
    stringstream strm;

    strm << "{ ";
    for (auto it_primitives = this->startPosition; it_primitives != this->endPosition;)
    {
        strm << "(" << it_primitives->get()->getSn() << ", ";

        for (auto it = it_primitives->get()->getContent().begin(); it != it_primitives->get()->getContent().end();)
        {
            strm << it->second;
            it++;
            if (it != it_primitives->get()->getContent().end())
                strm << ", ";
            else
                strm << ")";
        }

        if (it_primitives == this->lastPosition)
            it_primitives = this->endPosition;
        else
        {
            it_primitives++;
            strm << ", ";
        }
    }

    strm << " }";

    return strm.str();
}

Selection::~Selection()
{
    // TODO Auto-generated destructor stub
}

//----------------------------------------------------------------------------------------
// TimeHorizon
Selection::TimeHorizon::TimeHorizon(Selection *selection) : selection(selection) {}

Selection::TimeHorizon::TimeHorizon(Selection *selection, GlobalEventStreamTypedef_iterator startPosition)
    : selection(selection)
{
    this->currentPosition = startPosition;
}

/**
 * protected by lock
 * increment the current position:
 * if the currentPosition accesses to the end of the selection:
 *  +set the ended of TimeHorizon to true
 */
void Selection::TimeHorizon::incrementCurrentPosition()
{
    lock_guard<boost::shared_mutex> shared_lk(this->shared_mtx); // write exclusive

    this->currentPosition++;

    if (currentPosition == this->selection->getEndPosition())
    {
        this->ended = true;
    }
}

typename Selection::GlobalEventStreamTypedef_iterator Selection::TimeHorizon::getCurrentPosition() const
{
    shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx); // read shared
    return this->currentPosition;
}

/**
 * protected by lock:
 * set the current position:
 * if the currentPosition param refers to the end of the selection:
 *  +set the ended of TimeHorizon to true
 * else:
 *  +set the currentPosition passed param
 */
void Selection::TimeHorizon::setCurrentPosition(GlobalEventStreamTypedef_iterator currentPosition)
{
    lock_guard<boost::shared_mutex> shared_lk(this->shared_mtx); // write exclusive

    if (currentPosition == this->selection->getEndPosition())
    {
        this->ended = true;
    }

    this->currentPosition = currentPosition;
}

/**
 * @brief it call getLastPosition function from the associated selection
 * @return an iterator to the last event in the selection
 */
typename Selection::GlobalEventStreamTypedef_iterator Selection::TimeHorizon::getLastPosition() const
{
    // doesn't need lock because selection.getLastPosition() uses a lock itself
    return this->selection->getLastPosition();
}
/**
 * @brief set if Horizon is end or not
 * @param ended: true if the Horizon accesses to the end of the selection
 * However ended=false may be not so useful because we don't know how to set currentPosition value
 */
void Selection::TimeHorizon::setIsEnded(bool ended)
{
    lock_guard<boost::shared_mutex> shared_lk(this->shared_mtx); // write exclusive
    if (ended)
    {
        this->currentPosition = this->selection->getEndPosition();
    }
    this->ended = ended;
}

bool Selection::TimeHorizon::isEnded() const
{
    shared_lock<boost::shared_mutex> shared_lk(this->shared_mtx); // read shared

    return this->ended;
}

Selection::TimeHorizon::~TimeHorizon() {}
} /* namespace selection */
