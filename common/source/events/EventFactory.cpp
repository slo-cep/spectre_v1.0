/*
 * EventFactory.cpp
 *
 *  Created on: Jul 21, 2016
 *      Author: sload
 */

#include "../../header/events/EventFactory.hpp"
#include "../../header/events/events.hpp"

using namespace util;
using namespace std;

namespace events
{

/**
 * Constructor
 * @param type: event type
 */
unsigned long EventFactory::sequenceNumber = 1;

EventFactory::EventFactory(Constants::EventType type, profiler::Measurements *measurements)
{
    this->type = type;
    this->measurements = measurements;
    this->simpleValueEventPoolFactory = make_shared<SimpleValueEventPoolFactory>();
}

/**
 * create event according to type
 */
// Note: in c++, you can't use abstract class as return value for a function unless you define it as reference(&) or
// pointer(*)
shared_ptr<AbstractEvent> EventFactory::createNewEvent()
{
    shared_ptr<AbstractEvent> result = nullptr;
    /*
     * create event according to type
     *
     * register all new types here. Else, no event is created and null is returned
     */
    switch (this->type)
    {
    case Constants::EventType::SIMPLE_VALUE:
        result = make_shared<SimpleValueEvent>(EventFactory::sequenceNumber, this->measurements);
        //        result = this->simpleValueEventPoolFactory->create(EventFactory::sequenceNumber, this->measurements);
        EventFactory::sequenceNumber++;
        break;
    case Constants::EventType::PULSE:
        result = make_shared<PulseEvent>(-1, this->measurements);
        break;
    case Constants::EventType::END_OF_SELECTION:
        result = make_shared<EndOfSelection>(-2, this->measurements);
        break;
    case Constants::EventType::END_OF_STREAM:
        result = make_shared<EndOfStreamEvent>(-3, this->measurements);
        break;
    default:
        break;
    }

    return result;
}
}
