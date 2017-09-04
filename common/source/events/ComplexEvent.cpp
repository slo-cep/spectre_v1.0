/*
 * ComplexEvent.cpp
 *
 *  Created on: Aug 1, 2016
 *      Author: sload
 */

#include "../../header/events/ComplexEvent.hpp"

using namespace util;
namespace events
{

ComplexEvent::ComplexEvent(unsigned long sn, profiler::Measurements *timeMeasurement)
    : AbstractEvent(sn, timeMeasurement)
{
    this->setType(Constants::EventType::Complex);
}

/**
 * constructor
 * @param sn: sequence number
 * @param primitiveEvents (vector<shared_ptr<events::AbstractEvent>>): all primitive events that constitute a complex event
 * @param measurements: for profiling
 */
ComplexEvent::ComplexEvent(unsigned long sn, vector<shared_ptr<AbstractEvent>> primitiveEvents,
                           profiler::Measurements *measurements)
    : AbstractEvent(sn, measurements)
{
    this->primitiveEvents = primitiveEvents;
    this->setType(Constants::EventType::Complex);
}

/**
 * no binary content, nothing to do
 */
void ComplexEvent::marshalBinaryContent() {}
/**
 * no binary content, nothing to do
 */
void ComplexEvent::unmarshalBinaryContent() {}

const vector<shared_ptr<AbstractEvent>> &ComplexEvent::getPrimitiveEvents() const { return primitiveEvents; }

void ComplexEvent::setPrimitiveEvents(vector<shared_ptr<AbstractEvent>> primitiveEvents)
{
    this->primitiveEvents = primitiveEvents;
}

/*
 * represents the sn of primitive event which is highest sn among others
 * useful for validation
 */
unsigned long ComplexEvent::getHighestSn()
{
    if (this->highestSn == 0) // user (operator user) doesn't compute it
    {
        unsigned long maxSn = 0;
        for (auto it = this->primitiveEvents.begin(); it != this->primitiveEvents.end(); it++)
        {
            unsigned long sn = it->get()->getSn();
            if (maxSn < sn)
                maxSn = sn;
        }
        this->highestSn = maxSn;
    }
    return this->highestSn;
}

/*
 * represents the sn of primitive event which is highest sn among others
 * useful for validation
 */
void ComplexEvent::setHighestSn(unsigned long highestSn) { this->highestSn = highestSn; }

/**
 * for debugging purpose
 * convert an event to string
 */
string ComplexEvent::toString() const
{

    stringstream strm;

    // strm<<"{ timestamp: "<< this->getTimestamp() <<": ";
    strm << "{";
    for (auto it_primitives = this->primitiveEvents.begin(); it_primitives != this->primitiveEvents.end();)
    {
        strm << "(" << it_primitives->get()->getSn() << ", ";

        if (it_primitives->get()->getContent().count("SS") != 0)
            strm << it_primitives->get()->getContent().at("SS") << ")";
        else if (it_primitives->get()->getContent().size() != 0)
            strm << it_primitives->get()->getContent().begin()->second << ")";

        /* for (auto it = it_primitives->get()->getContent().begin(); it != it_primitives->get()->getContent().end();)
         {
             strm << it->second ;
             it++;
             if (it != it_primitives->get()->getContent().end())
                 strm << ", ";
             else
                 strm<< ")";

         }*/

        it_primitives++;
        if (it_primitives != this->primitiveEvents.end())
            strm << ", ";
    }

    strm << " }";

    strm<<", {#events: "<<this->primitiveEvents.size()<<"}";

    return strm.str();
}

ComplexEvent::~ComplexEvent()
{
    // TODO Auto-generated destructor stub
}

} /* namespace execution_path */
