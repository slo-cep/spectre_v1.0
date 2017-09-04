/*
 * PulseEvent.cpp
 *
 *  Created on: Jul 21, 2016
 *      Author: sload
 */

#include "../../header/events/PulseEvent.hpp"

using namespace util;
namespace events
{

/**
 * Constructor
 * @param sn: sequence number
 * @param measurements: for profiling
 */
PulseEvent::PulseEvent(unsigned long sn, profiler::Measurements* measurements) :
        AbstractEvent(sn, measurements)
{
    this->setType(Constants::EventType::PULSE);
}

/**
 * no binary content, nothing to do
 */
void PulseEvent::marshalBinaryContent()
{
	// no binary content, nothing to do
}

/**
 * no binary content, nothing to do
 */
void PulseEvent::unmarshalBinaryContent()
{
	// no binary content, nothing to do
}

PulseEvent::~PulseEvent()
{
}

}

