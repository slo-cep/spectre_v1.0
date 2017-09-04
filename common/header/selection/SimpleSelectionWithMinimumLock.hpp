/*
 * SimpleSelectionWithMinimumLock.hpp
 *
 * Created on: 28.10.2016
 *      Author: sload
 */

#ifndef SIMPLESELECTIONWITHMINIMUMLOCK_HPP
#define SIMPLESELECTIONWITHMINIMUMLOCK_HPP

#include "AbstractSelection.hpp"

using namespace std;

namespace selection
{
class SimpleSelectionWithMinimumLock : public AbstractSelection
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
    SimpleSelectionWithMinimumLock(unsigned long id, GlobalEventStreamTypedef &globalEventStream,
                    AbstractSelection *predecessorSelection,
                    GlobalEventStreamTypedef_iterator startPosition, profiler::Measurements* measurements);


    /**
     * @brief SimpleSelectionWithMinimumLock: Copy constructor
     * @param other: rhs
     */
    SimpleSelectionWithMinimumLock(const SimpleSelectionWithMinimumLock& other);

    shared_ptr<AbstractSelection> clone() override;

    GlobalEventStreamTypedef_iterator getCurrentPosition() const override;
    void setCurrentPosition(GlobalEventStreamTypedef_iterator currentPosition) override;
    void incrementCurrentPosition() override;

    /**
     * tells if the selection is closed and all its events are already read by the Feeder!
     * @return true or false
     */
    bool isSelectionCompleted() override;


    ~SimpleSelectionWithMinimumLock();

private:
    // keep the position for next event to be fetched from the global event stream
    GlobalEventStreamTypedef_iterator currentPosition;
 };
}


#endif // SIMPLESELECTIONWITHMINIMUMLOCK_HPP
