/*
 * AbstractFeeder.cpp
 *
 *  Created on: Aug 9, 2016
 *      Author: sload
 */
#include "../../header/feeder/AbstractFeeder.hpp"
#include "../../header/execution_path/ExecutionPath.hpp"

using namespace selection;
using namespace events;

namespace execution_path
{

AbstractFeeder::AbstractFeeder(GlobalEventStreamTypedef &globalEventStream, ExecutionPath* executionPath,
                               AbstractOperator *abstractOperator,
                               profiler::Measurements *measurements)
    : globalEventStream(globalEventStream), executionPath(executionPath), abstractOperator(abstractOperator),
      measurements(measurements)
{
}

AbstractFeeder::AbstractFeeder(const AbstractFeeder &other)
    : globalEventStream(other.globalEventStream), executionPath(other.executionPath),
      abstractOperator(other.abstractOperator), eventsToFeed(other.eventsToFeed), measurements(other.measurements)
{
}

/**
 * set new AbstractOperator
 * @param abstractOperator: shared_ptr to AbstractOperator
 */
void AbstractFeeder::setExecutionPath(ExecutionPath* executionPath)
{
    this->executionPath=executionPath;
}

void AbstractFeeder::setAbstractOperator(AbstractOperator* abstractOperator)
{
    this->abstractOperator = abstractOperator;
}

void AbstractFeeder::pushNewEvents(vector<shared_ptr<AbstractEvent>> eventsToFeed)
{
    if (this->eventsToFeed.size() == 0)
        this->eventsToFeed = eventsToFeed;
    else
    {
        for (auto it = eventsToFeed.begin(); it != eventsToFeed.end(); it++)
            this->eventsToFeed.push_back(*it);
    }
}

void AbstractFeeder::pushNewEvent(shared_ptr<AbstractEvent> eventToFeed)
{
    this->eventsToFeed.push_back(eventToFeed);
}

/**
 * reset the Feeder (set new AbstractOperator!)
 * useful for revert
 * @param abstractOperator: shared_ptr to AbstractOperator
 */
void AbstractFeeder::reset(AbstractOperator* abstractOperator) { this->abstractOperator = abstractOperator; }

AbstractFeeder::~AbstractFeeder()
{
    // TODO Auto-generated destructor stub
}
}
