/*
 * EndOfSelection.cpp
 *
 *  Created on: Sep 05, 2016
 *      Author: sload
 */

#include "../../header/events/EndOfSelection.hpp"

using namespace util;

namespace events
{
/**
 * Constructor
 * @param sn: sequence number
 * @param measurements: for profiling
 */
EndOfSelection::EndOfSelection(unsigned long sn, profiler::Measurements* measurements): AbstractEvent(sn, measurements)
{
    this->setType(Constants::EventType::END_OF_SELECTION);
}

/**
 * no binary content, nothing to do
 */
void EndOfSelection::marshalBinaryContent()
{
    // no binary content, nothing to do
}

/**
 * no binary content, nothing to do
 */
void EndOfSelection::unmarshalBinaryContent()
{
    // no binary content, nothing to do
}

EndOfSelection::~EndOfSelection()
{

}

}
