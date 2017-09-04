/*
 * DeterministicFeeder.cpp
 *
 * Created on: 12.10.2016
 *      Author: sload
 */

#include "../../header/feeder/DeterministicFeeder.hpp"
#include "../../header/execution_path/ExecutionPath.hpp"

using namespace events;
using namespace util;
using namespace shared_memory;
using namespace selection;
using namespace profiler;

namespace execution_path
{

DeterministicFeeder::DeterministicFeeder(GlobalEventStreamTypedef &globalEventStream, ExecutionPath* executionPath,
                                         AbstractOperator *abstractOperator,
                                         Measurements* measurements)
    : AbstractFeeder(globalEventStream,executionPath, abstractOperator, measurements)
{
}

shared_ptr<AbstractFeeder> DeterministicFeeder::clone() { return make_shared<DeterministicFeeder>(*this); }

DeterministicFeeder::DeterministicFeeder(const DeterministicFeeder &other)
    : AbstractFeeder(other)
{
}

/**
 * feed the operator instance with the current event from the shared event stream (globalEventStream)
 */
bool DeterministicFeeder::feedOperatorWithEvents()
{
    // start time
    //    unsigned long startTime = Helper::currentTimeMillis();
    bool result =false;
    for (auto it = this->eventsToFeed.begin(); it != this->eventsToFeed.end(); it++)
    {
        this->abstractOperator->receivePrimitiveEvent(*it);
        this->executionPath->setLastFeededEventSN( it->get()->getSn());
        result=true;
    }
    if(result)
        this->eventsToFeed.clear();

    // end time
    //    unsigned long endTime = Helper::currentTimeMillis();

    //    this->timeMeasurement.get()->increaseTime(TimeMeasurement::ComponentName::FEEDER, endTime - startTime);
    //    if((endTime-startTime)>0)
    //    cout<<"DeterministicFeeder.cpp: endTime-startTime= "<<endTime-startTime<<endl;

    return result;
}

DeterministicFeeder::~DeterministicFeeder()
{
    // TODO Auto-generated destructor stub
}
}
/* namespace execution_path */
