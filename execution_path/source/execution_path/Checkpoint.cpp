/*
 * Checkpoint.cpp
 *
 * Created on: 18.11.2016
 *      Author: sload
 */
#include "../../header/execution_path/Checkpoint.hpp"
#include "../../header/execution_path/ExecutionPath.hpp"

using namespace events;
namespace execution_path
{

unsigned long Checkpoint::generateCheckpointId() { return lastCheckpointId.operator++(); }
atomic<unsigned long> Checkpoint::lastCheckpointId{0};


execution_path::Checkpoint::Checkpoint() {}

execution_path::Checkpoint::Checkpoint(
    shared_ptr<execution_path::ExecutionPath> executionPath,shared_ptr<AbstractEvent> checkpointingEvent)
    : executionPath(executionPath), checkpointingEvent(checkpointingEvent), Id(generateCheckpointId())
{
}

shared_ptr<ExecutionPath> Checkpoint::getExecutionPath() const { return executionPath; }

void Checkpoint::setExecutionPath(const shared_ptr<ExecutionPath> &executionPath) { this->executionPath = executionPath; }

const shared_ptr<AbstractEvent> &Checkpoint::getCheckpointingEvent() const
{
    return this->checkpointingEvent;
}

void Checkpoint::setCheckpointingEvent(const shared_ptr<AbstractEvent> &checkpointingEvent)
{
    this->checkpointingEvent = checkpointingEvent;
}

unsigned int Checkpoint::getId() const { return this->Id; }

void Checkpoint::setId(unsigned int Id) { this->Id = Id; }

execution_path::Checkpoint::~Checkpoint() {}
}
