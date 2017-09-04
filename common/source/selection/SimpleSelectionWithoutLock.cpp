/*
 * SimpleSelectionWithoutLock.cpp
 *
 * Created on: 05.01.2017
 *      Author: sload
 */

#include "../../header/selection/SimpleSelectionWithoutLock.hpp"

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
SimpleSelectionWithoutLock::SimpleSelectionWithoutLock(unsigned long id, GlobalEventStreamTypedef &globalEventStream,
                                                       AbstractSelection *predecessorSelection,
                                                       GlobalEventStreamTypedef_iterator startPosition,
                                                       Measurements *measurements)
    : AbstractSelection(id, globalEventStream, predecessorSelection, startPosition, measurements),
      currentPosition(startPosition)
{
}

SimpleSelectionWithoutLock::SimpleSelectionWithoutLock(const SimpleSelectionWithoutLock &other)
    : AbstractSelection(other), currentPosition(other.currentPosition)
{
}

shared_ptr<AbstractSelection> SimpleSelectionWithoutLock::clone()
{
    return make_shared<SimpleSelectionWithoutLock>(*this);
}

typename SimpleSelectionWithoutLock::GlobalEventStreamTypedef_iterator
SimpleSelectionWithoutLock::getCurrentPosition() const
{
    return this->currentPosition;
}

void SimpleSelectionWithoutLock::setCurrentPosition(GlobalEventStreamTypedef_iterator currentPosition)
{
    this->currentPosition = currentPosition;
}

void SimpleSelectionWithoutLock::incrementCurrentPosition() { this->currentPosition++; }

/**
 * tells if the selection is closed and all its events are already read by the Feeder!
 * @return true or false
 */
bool SimpleSelectionWithoutLock::isSelectionCompleted()
{
//    cout << "SimpleSelectionWithoutLock: selection : " << this->getId() << endl;
    bool selectionClosedTemp = this->isSelectionClosed();
    if (!selectionClosedTemp)
        return false;

    const shared_ptr<events::AbstractEvent>* event = this->getGlobalEventStream().get(this->currentPosition);
    if (event == NULL && selectionClosedTemp)
        return true;
    else if (event == NULL)
        return false;

    if (this->getBasicSelection()->getLastEventSn() == 0
        || event->get()->getSn() <= this->getBasicSelection()->getLastEventSn())
        return false;
    else
        return true;
}

SimpleSelectionWithoutLock::~SimpleSelectionWithoutLock() {}
}
