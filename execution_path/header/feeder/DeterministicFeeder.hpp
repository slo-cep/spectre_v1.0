/*
 * DeterministicFeeder.hpp
 *
 * Created on: 12.10.2016
 *      Author: sload
 */

#ifndef DETERMINISTICFEEDER_HPP
#define DETERMINISTICFEEDER_HPP


#include "AbstractFeeder.hpp"

#include "header/util/GlobalTypedef.hpp"
#include "header/util/Helper.hpp"

#include "header/Measurements.hpp"

#include <iterator>
#include <list>
#include <memory> //shared_ptr & unique_ptr
#include <chrono> //sleep
#include <thread> //sleep

using namespace std;
namespace execution_path
{

/**
 * It is responsible to feed the operator instance with the events
 */
class DeterministicFeeder: public AbstractFeeder
{
public:
    typedef util::GlobalTypedef::GlobalEventStreamTypedef GlobalEventStreamTypedef;
    typedef typename GlobalEventStreamTypedef::iterator GlobalEventStreamTypedef_iterator;

    DeterministicFeeder() = delete;

    /**
     * Constructor
     * @param globalEventStream: reference to the shared event stream between all instances
     * @param executionPath: a reference to the execution path
     * @param abstractOperator: an instance of the operator to feed it with the events;
     * @param measurements: for profiling
     */
    DeterministicFeeder(GlobalEventStreamTypedef& globalEventStream, ExecutionPath *executionPath,
            AbstractOperator* abstractOperator, profiler::Measurements *measurements);

    shared_ptr<AbstractFeeder>  clone() override;

    /**
     * @brief Copy constructor
     * @param other: rhs
     */
    DeterministicFeeder(const DeterministicFeeder& other);

    /**
     *@brief feedOperatorWithEvents: feed the operator instance with the events
     *  from its local vector eventsToFeed
     *@return: true if any event is fed
     */
    bool feedOperatorWithEvents() override;

    virtual ~DeterministicFeeder();

private:

};

} /* namespace execution_path */


#endif // DETERMINISTICFEEDER_HPP
