/*
 * AbstractSpeculator.hpp
 *
 * Created on: 17.11.2016
 *      Author: sload
 */

#ifndef ABSTRACTSPECULATOR_HPP
#define ABSTRACTSPECULATOR_HPP

#include "header/selection/AbstractSelection.hpp"
#include "header/shared_memory/AbstractGlobalEventStream.hpp"
#include "header/util/Constants.hpp"
#include "header/util/GlobalParameters.hpp"
#include "header/util/GlobalTypedef.hpp"

#include "../../header/feeder/AbstractFeeder.hpp"

#include "../../header/execution_path/Cgroup.hpp"

#include <memory>

namespace execution_path
{
class ExecutionPath;
class AbstractSpeculator
{
public:
    typedef util::GlobalTypedef::GlobalEventStreamTypedef GlobalEventStreamTypedef;
    typedef typename GlobalEventStreamTypedef::iterator GlobalEventStreamTypedef_iterator;

    virtual shared_ptr<AbstractSpeculator>  clone()=0;

    void setExecutionPath(ExecutionPath* executionPath);

    /**
     * @brief main: run the the Speculator
     * @param number: number of events to process each time (zero means process
     *  till all events in the Selection are feeded and processed)
     */
    virtual bool main (size_t number)=0;


protected:

    /**
     * @brief AbstractSpeculator: Ctor
     * @param executionPath: execution path which the Speculator will run
     */
    AbstractSpeculator(ExecutionPath *executionPath);

    /**
     * @brief AbstractSpeculator: Copy constructor
     * @param other: rhs
     */
    AbstractSpeculator(const AbstractSpeculator& other);

    /**
     * @brief execute:
     * - take a decision whether to feed and event or/and to branch
     * - update the execution path probabilities also update the brother execution path probabilities
     */
    virtual bool execute(bool master, bool masterForFirstTime)=0;

    /**
     * @brief fetchEventsFromGlobalStream: get events from global event stream and add them to local list
     *  ExecutionPath::LocalEvents
     * @param number: number of events to get
     */
    virtual void fetchEventsFromGlobalEventStream(int number)=0;

    virtual void fetchRangeOfEventsFromGlobalEventStream() =0;

    /**
     * @brief feedEvent: decide whether to feed the event
     * @param event: the event
     * @return true if the event should be feeded else false
     */
    virtual bool feedEvent(const shared_ptr<events::AbstractEvent>& event)const = 0;

    virtual void recover()=0;

    virtual ~AbstractSpeculator();

    ExecutionPath* executionPath;
private:
};
}

#endif // ABSTRACTSPECULATOR_HPP
