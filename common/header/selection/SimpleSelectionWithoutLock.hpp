/*
 * SimpleSelectionWithoutLock.hpp
 *
 * Created on: 05.01.2017
 *      Author: sload
 */


#ifndef SIMPLESELECTIONWITHOUTLOCK_HPP
#define SIMPLESELECTIONWITHOUTLOCK_HPP

#include "AbstractSelection.hpp"

using namespace std;

namespace selection
{
class SimpleSelectionWithoutLock : public AbstractSelection
{
public:

    /**
     * Constructor
     * @param id: selection Id
     * @param globalEventStream: reference to the shared event stream between all instances
     * @param startPosition: iterator which points to the first event in the selection
     * @param predecessorSelection: the predecessor of this selection
     * @param measurements: for profiling
     */
    SimpleSelectionWithoutLock(unsigned long id, GlobalEventStreamTypedef &globalEventStream,
                    AbstractSelection *predecessorSelection,
                    GlobalEventStreamTypedef_iterator startPosition, profiler::Measurements* measurements);


    /**
     * @brief SimpleSelectionWithoutLock: Copy constructor
     * @param other: rhs
     */
    SimpleSelectionWithoutLock(const SimpleSelectionWithoutLock& other);

    shared_ptr<AbstractSelection> clone() override;

    GlobalEventStreamTypedef_iterator getCurrentPosition() const override;
    void setCurrentPosition(GlobalEventStreamTypedef_iterator currentPosition) override;
    void incrementCurrentPosition() override;

    /**
     * tells if the selection is closed and all its events are already read by the Feeder!
     * @return true or false
     */
    bool isSelectionCompleted() override;


    ~SimpleSelectionWithoutLock();

private:
    // keep the position for next event to be fetched from the global event stream
    GlobalEventStreamTypedef_iterator currentPosition;
 };
}


#endif // SIMPLESELECTIONWITHOUTLOCK_HPP
