/*
 * AbstractFeeder.hpp
 *
 *  Created on: Aug 9, 2016
 *      Author: sload
 */

#ifndef HEADER_FEEDER_ABSTRACTFEEDER_HPP_
#define HEADER_FEEDER_ABSTRACTFEEDER_HPP_

#include "header/util/GlobalTypedef.hpp"

#include "header/shared_memory/AbstractGlobalEventStream.hpp"

#include "../../header/operator/AbstractOperator.hpp"
#include "header/selection/AbstractSelection.hpp"

#include <memory>

using namespace std;

namespace execution_path
{
class ExecutionPath;

class AbstractFeeder
{
public:
    typedef util::GlobalTypedef::GlobalEventStreamTypedef GlobalEventStreamTypedef;
    typedef typename GlobalEventStreamTypedef::iterator GlobalEventStreamTypedef_iterator;

    AbstractFeeder() = delete;

    virtual shared_ptr<AbstractFeeder> clone() = 0;

    void setExecutionPath(ExecutionPath* executionPath);


    /**
     *@brief feedOperatorWithEvents: feed the operator instance with the events
     *  from its local vector eventsToFeed
     *@return: true if any event is fed
     */
    virtual bool feedOperatorWithEvents() = 0;

    /**
     * set new AbstractOperator
     * @param abstractOperator: shared_ptr to AbstractOperator
     */
    void setAbstractOperator(AbstractOperator *abstractOperator);

    void pushNewEvents(vector<shared_ptr<events::AbstractEvent>> eventsToFeed);

    void pushNewEvent(shared_ptr<events::AbstractEvent> eventToFeed);

    /**
     * reset the Feeder (set new AbstractOperator!)
     * useful for revert
     * @param abstractOperator: shared_ptr to AbstractOperator
     */
    void reset(AbstractOperator *abstractOperator);

    virtual ~AbstractFeeder();

protected:
    GlobalEventStreamTypedef &globalEventStream;
    ExecutionPath* executionPath;
    AbstractOperator* abstractOperator;

    vector<shared_ptr<events::AbstractEvent>> eventsToFeed;

    profiler::Measurements* measurements;

    /**
     * Constructor
     * @param globalEventStream: a reference to the shared event stream between all instances
     * @param executionPath: a reference to the execution path
     * @param AbstractOperator: pointer to the operator instance which will be feeded with the events;
     * @param measurements: for profiling
     */
    AbstractFeeder(GlobalEventStreamTypedef &globalEventStream, ExecutionPath *executionPath,
                   AbstractOperator* abstractOperator,
                   profiler::Measurements* measurements);

    /**
     * @brief Copy constructor
     * @param other: rhs
     */
    AbstractFeeder(const AbstractFeeder &other);
};
}

#endif /* HEADER_FEEDER_ABSTRACTFEEDER_HPP_ */
