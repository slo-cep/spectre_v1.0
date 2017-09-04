/*
 * AbstractEvent.cpp
 *
 *  Created on: Jul 20, 2016
 *      Author: sload
 */

#include "../../header/events/AbstractEvent.hpp"

using namespace profiler;
using namespace std;
using namespace util;

namespace events
{

/**
 * Constructor
 * @param sn:sequence number
 * @param measurements: for profiling
 */
AbstractEvent::AbstractEvent(unsigned long sn, Measurements *measurements)
{
    this->setTimestamp(util::Helper::currentTimeMillis());
    this->setSn(sn);
    this->measurements = measurements;
}

/**
 * copy constructor
 * it is important because this class doesn't has the default one
 * the default is deleted because this class contains atomic member!
 */
AbstractEvent::AbstractEvent(const AbstractEvent &other)
{
    this->timestamp = other.timestamp;
    this->sn = other.sn;
    this->type = other.type;
    this->content = other.content;
    // this->isConsumed.store(other.isConsumed.load());
}

/**
 * assignment operator=
 * it is important because this class doesn't has the default one
 * the default is deleted because this class contains atomic member!
 */
AbstractEvent &AbstractEvent::operator=(const AbstractEvent &other)
{
    this->timestamp = other.timestamp;
    this->sn = other.sn;
    this->type = other.type;
    this->content = other.content;
    return *this;
}

/**
 * overload equal operator "=="
 */
bool AbstractEvent::operator==(const AbstractEvent &b) const
{
    if (this->getTimestamp() == b.getTimestamp() && this->getSn() == b.getSn())
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * overload comparison operator "<"
 * events are compared,
 * first by their timestamp
 * then by their source id
 * then by their sequence number
 */
bool AbstractEvent::operator<(const AbstractEvent &b) const
{
    if (this->getTimestamp() <= b.getTimestamp())
    {
        return false;
    }
    else
    {
        return true;
    }
}

/**
 * events are compared,
 * first by their timestamp
 * then by their source id
 * then by their sequence number
 * @return 0:if equal; -1:if larger; 1: if smaller
 */
int AbstractEvent::compareTo(const AbstractEvent &b) const
{
    if (this->getTimestamp() < b.getTimestamp())
    {
        return -1;
    }
    else if (this->getTimestamp() > b.getTimestamp())
    {
        return 1;
    }
    else
        return 0;
}

unsigned long AbstractEvent::getTimestamp() const { return this->timestamp; }
void AbstractEvent::setTimestamp(unsigned long timestamp) { this->timestamp = timestamp; }

unsigned long AbstractEvent::getSn() const { return this->sn; }
void AbstractEvent::setSn(unsigned long sn) { this->sn = sn; }

Constants::EventType AbstractEvent::getType() const { return this->type; }
void AbstractEvent::setType(Constants::EventType type) { this->type = type; }

const unordered_map<string, string> &AbstractEvent::getContent() const { return this->content; }
void AbstractEvent::setContent(unordered_map<string, string> content) { this->content = content; }


unsigned long AbstractEvent::getRealTimeStamp() const
{
    return realTimeStamp;
}

void AbstractEvent::setRealTimeStamp(unsigned long value)
{
    realTimeStamp = value;
}

/**
 * for debugging purpose
 * convert an event to string
 */
string AbstractEvent::toString() const
{
    string stringEvent;
    stringstream strm;
    strm << "sn: " << this->sn << ", timestamp: " << this->timestamp;

    strm << ", content:{ ";

    for (auto it = this->content.begin(); it != this->content.end();)
    {
        strm << "(key: " << it->first << ", value:" << it->second << ")";
        it++;
        if (it != this->content.end())
            strm << ", ";
    }
    strm << " }";

    return strm.str();
}

AbstractEvent::~AbstractEvent() {}
}
