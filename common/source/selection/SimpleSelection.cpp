/*
 * SimpleSelection.cpp
 *
 * Created on: 17.10.2016
 *      Author: sload
 */

#include "../../header/selection/SimpleSelection.hpp"

using namespace profiler;
using namespace util;

namespace selection
{
/**
 * Constructor
 * @param id: selection Id
 * @param globalEventStream: reference to the shared event stream between all instances
 * @param startPosition: iterator which points to the first event in the selection
 * @param predecessorSelection: the predecessor of this selection
 * @param measurements: for profiling
 */
SimpleSelection::SimpleSelection(unsigned long id, GlobalEventStreamTypedef &globalEventStream,
                                 AbstractSelection *predecessorSelection,
                                 GlobalEventStreamTypedef_iterator startPosition,
                                 Measurements *measurements)
    : AbstractSelection(id, globalEventStream, predecessorSelection, startPosition, measurements),
      currentPosition(startPosition)
{
}

SimpleSelection::SimpleSelection(const SimpleSelection &other) : AbstractSelection(other)
{
    this->currentPosition = other.currentPosition;
}

shared_ptr<AbstractSelection> SimpleSelection::clone() { return make_shared<SimpleSelection>(*this); }

typename SimpleSelection::GlobalEventStreamTypedef_iterator SimpleSelection::getCurrentPosition() const
{
    lock_guard<mutex> lk(this->mtx_current);
    return this->currentPosition;
}

void SimpleSelection::setCurrentPosition(GlobalEventStreamTypedef_iterator currentPosition)
{
    lock_guard<mutex> lk(this->mtx_current);
    this->currentPosition = currentPosition;
}

void SimpleSelection::incrementCurrentPosition()
{
    lock_guard<mutex> lk(this->mtx_current);

    //    this->currentPosition = this->getGlobalEventStream().increment(this->currentPosition);
    this->currentPosition++;
}

/**
 * tells if the selection is closed and all its events are already read by the Feeder!
 * @return true or false
 */
bool SimpleSelection::isSelectionCompleted()
{
    // start time
//    unsigned long startTime = Helper::currentTimeMillis();

    //    lock_guard<mutex> lk(this->mtx_current);

//    bool selectionClosedTemp = this->isSelectionClosed();
//    auto endPostionTemp = this->getEndPosition();

//    lock_guard<mutex> lk(this->mtx_current);
//    bool result = selectionClosedTemp && (this->currentPosition == endPostionTemp);

    // end time
//    unsigned long endTime = Helper::currentTimeMillis();

//    this->measurements.get()->increaseTime(Measurements::ComponentName::IS_SELECTION_COMPLETED,
//                                              endTime - startTime);

//    return result;
    return true;
}

SimpleSelection::~SimpleSelection() {}
}
