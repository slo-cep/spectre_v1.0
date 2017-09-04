/*
 * Speculator.hpp
 *
 * Created on: 17.11.2016
 *      Author: sload
 */

#ifndef SPECULATOR_HPP
#define SPECULATOR_HPP

#include "AbstractSpeculator.hpp"

using namespace std;

namespace execution_path
{
class Speculator : public AbstractSpeculator
{
public:
    typedef util::GlobalTypedef::GlobalEventStreamTypedef GlobalEventStreamTypedef;
    typedef typename GlobalEventStreamTypedef::iterator GlobalEventStreamTypedef_iterator;

    /**
     * @brief AbstractSpeculator: Ctor
     * @param executionPath: execution path which the Speculator will run
     */
    Speculator(ExecutionPath *executionPath);

    /**
     * @brief Speculator: Copy constructor
     * @param other: rhs
     */
    Speculator(const Speculator& other);
    shared_ptr<AbstractSpeculator>  clone() override;

    /**
     * @brief main: run the the Speculator
     * - It calls Feeder & Operator instance as well
     * @param number: number of events to process each time (zero means process
     *  till all events in the Selection are feeded and processed)
     */
    bool main (size_t number) override;

    void recover() override;

    virtual ~Speculator();

private:
    int cgroupsUpdateCounter=0;

    /**
     * @brief execute:
     */
    bool execute(bool master, bool masterForFirstTime) override;

    /**
     * @brief fetchEventsFromGlobalStream: get events from global event stream and add them to local list
     *  ExecutionPath::LocalEvents
     * @param number: number of events to get
     */
    void fetchEventsFromGlobalEventStream(int number) override;

    void fetchRangeOfEventsFromGlobalEventStream() override;



    /**
     * @brief feedEvent: decide whether to feed the event
     * @param event: the event
     * @return true if the event should be feeded else false
     */
    bool feedEvent(const shared_ptr<events::AbstractEvent>& event) const override;

    /**
     * @brief checkFeededEvents: take the difference between the local cgroup and the updated cgroups to check
     * whether any event is used by this execution path but it is added to the updated cgroup
     * or any event that was not fed and it deleted from the updated cgroup.
     * if yes then the execution path is doing wrong work and it should recover from a checkpoint
     * @param cgroups: updated cgroups
     */
    void checkFeededEvents(const unordered_map<unsigned long, shared_ptr<Cgroup> > &cgroups);
};
}

#endif // SPECULATOR_HPP
