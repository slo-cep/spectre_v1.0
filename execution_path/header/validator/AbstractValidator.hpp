/*
 * AbstractValidator.hpp
 *
 *  Created on: Aug 9, 2016
 *      Author: sload
 */

#ifndef HEADER_VALIDATOR_ABSTRACTVALIDATOR_HPP_
#define HEADER_VALIDATOR_ABSTRACTVALIDATOR_HPP_

#include "header/events/events.hpp"
#include "header/shared_memory/AbstractGlobalEventStream.hpp"

#include "../../header/execution_path/Cgroup.hpp"

#include "header/util/Constants.hpp"
#include "header/util/GlobalTypedef.hpp"

#include "header/Merger.hpp"

#include <atomic>
#include <list>
#include <memory>
#include <queue>
#include <vector>

using namespace std;

namespace execution_path
{
class ExecutionPath;
class AbstractValidator
{
public:
    AbstractValidator() = delete;

    virtual shared_ptr<AbstractValidator> clone() = 0;

    /**
     * @brief main: main function in validator
     * @param terminate: indicate that execution path will terminate
     * @param masterForFirstTime: is this first time that execution path becomes a master path
     */
    virtual void main(bool terminate, bool masterForFirstTime) = 0;

    void setExecutionPath(ExecutionPath* executionPath);

    ExecutionPath* getExecutionPath();

    virtual void receiveComplexEvent(shared_ptr<events::ComplexEvent> complexEvent) = 0;

    virtual void receiveCgroup(shared_ptr<Cgroup> cgroup, Cgroup::Status status) = 0;

    virtual  size_t getComplexEventSize() const=0;

    virtual ~AbstractValidator();

protected:
    ExecutionPath* executionPath;

    merger::Merger &merger;

    profiler::Measurements* measurements;

    /**
     * Constructor
     * @param executionPath: reference to the execution path
     * @param masterSelectionId: Id of the selection with the highest priority
     * @param merger: a reference to the Merger
     * @param measurements: for profiling
     */
    AbstractValidator(ExecutionPath* executionPath, merger::Merger &merger,  profiler::Measurements* measurements);

    /**
     * @brief Copy constructor
     * @param other: rhs
     */
    AbstractValidator(const AbstractValidator &other);
};

} /* namespace execution_path */

#endif /* HEADER_VALIDATOR_ABSTRACTVALIDATOR_HPP_ */
