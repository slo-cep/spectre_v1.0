/*
 * DeterministicValidator.hpp
 *
 * Created on: 12.10.2016
 *      Author: sload
 */

#ifndef DETERMINISTICVALIDATOR_HPP
#define DETERMINISTICVALIDATOR_HPP

#include "AbstractValidator.hpp"

#include "header/Measurements.hpp"
#include <queue>
#include <list>
#include<unordered_map>
#include<map>
using namespace std;

namespace execution_path
{

class DeterministicValidator : public AbstractValidator
{
public:

    DeterministicValidator() = delete;

    /**
     * Constructor
     * @param executionPath: a reference to the execution path
     * @param merger: a reference to the Merger
     * @param measurements: for profiling
     */
    DeterministicValidator(ExecutionPath *executionPath, merger::Merger &merger,
              profiler::Measurements* measurements);

    DeterministicValidator(const DeterministicValidator & other);

    shared_ptr<AbstractValidator> clone() override;

    /**
     * @brief main: main function in validator
     * @param terminate: indicate that execution path will terminate
     * @param masterForFirstTime: is this first time that execution path becomes a master path
     */
     void main(bool terminate, bool masterForFirstTime) override;

    void receiveComplexEvent(shared_ptr<events::ComplexEvent> complexEvent) override;

    void receiveCgroup(shared_ptr<Cgroup> cgroup, Cgroup::Status status) override;

    size_t getComplexEventSize() const override;

    virtual ~DeterministicValidator();

private:
    vector<shared_ptr<events::ComplexEvent>> complexEvents;

    // cgroup Id, vector< eventsLeft, currentEvent>
    unordered_map< unsigned long,vector<pair<unsigned int, unsigned long>>> bufferedCgroups;

    /*
     * whenever the execution time becomes master, it should send all completed and valid cgroups
     * to path manager as valid cgroups. It also should send buffered cgroups to Markov model
     */
    void processFirstTimeMaster();

    void sendComplexEventsToMerger();
    void sendSelectionTerminationEventToMerger();

    /**
     * @brief inValidateUncompletedCgroups: On termination invalidate all uncompleted cgroups and
     * send them as update to path manager
     */
    void inValidateUncompletedCgroups();
    /**
     * @brief sendBufferedCgroupsToMarkov: send buffered cgroups to markov model
     */
    void sendBufferedCgroupsToMarkov();

    void processNewCgroup(const shared_ptr<Cgroup>& cgroup);
    void processUpdatedCgroup(const shared_ptr<Cgroup> &cgroup);
    void processDeletedCgroup(const shared_ptr<Cgroup>& cgroup);

    void writeComplexEventsToFile();
};

} /* namespace execution_path */

#endif // DETERMINISTICVALIDATOR_HPP
