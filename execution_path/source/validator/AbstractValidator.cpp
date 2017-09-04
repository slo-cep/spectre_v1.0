/*
 * AbstractValidator.cpp
 *
 *  Created on: Aug 9, 2016
 *      Author: sload
 */

#include "../../header/validator/AbstractValidator.hpp"
#include "../../header/execution_path/ExecutionPath.hpp"

using namespace selection;
using namespace events;
using namespace merger;

namespace execution_path
{

AbstractValidator::AbstractValidator(ExecutionPath *executionPath, Merger &merger, profiler::Measurements *measurements)
    : executionPath(executionPath), merger(merger), measurements(measurements)
{
}

AbstractValidator::AbstractValidator(const AbstractValidator &other)
    : executionPath(other.executionPath), merger(other.merger), measurements(other.measurements)
{
}

 void AbstractValidator::setExecutionPath(ExecutionPath* executionPath)
{
     this->executionPath = executionPath;
 }

 ExecutionPath *AbstractValidator::getExecutionPath()
 {
     return this->executionPath;
 }

AbstractValidator::~AbstractValidator()
{
    // TODO Auto-generated destructor stub
}

} /* namespace execution_path */
