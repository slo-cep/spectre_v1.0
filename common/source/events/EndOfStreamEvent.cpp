/*
 * EndOfStreamEvent.cpp
 *
 *  Created on: Jul 21, 2016
 *      Author: sload
 */

#include "../../header/events/EndOfStreamEvent.hpp"

using namespace util;
namespace events
{
/**
 * Constructor
 * @param sn: sequence number
 * @param measurements: for profiling
 */
EndOfStreamEvent::EndOfStreamEvent(unsigned long sn, profiler::Measurements* timeMeasurement) :
        AbstractEvent(sn,timeMeasurement)
{
    this->setType(Constants::EventType::END_OF_STREAM);
}

/**
 * no binary content, nothing to do
 */
void EndOfStreamEvent::marshalBinaryContent()
{
}
/**
 * no binary content, nothing to do
 */
void EndOfStreamEvent::unmarshalBinaryContent()
{
}

EndOfStreamEvent::~EndOfStreamEvent()
{
}

}

