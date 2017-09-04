/*
 * AbstractSpeculator.cpp
 *
 * Created on: 17.11.2016
 *      Author: sload
 */

#include "../../header/speculator/AbstractSpeculator.hpp"
#include "../../header/execution_path/ExecutionPath.hpp"

namespace execution_path
{

AbstractSpeculator::AbstractSpeculator(ExecutionPath* executionPath)
    : executionPath(executionPath)
{
}

void AbstractSpeculator::setExecutionPath(ExecutionPath* executionPath)
{
    this->executionPath=executionPath;
}

AbstractSpeculator::AbstractSpeculator(const AbstractSpeculator &other)
    : executionPath(other.executionPath)
{
}

AbstractSpeculator::~AbstractSpeculator() {}
}
