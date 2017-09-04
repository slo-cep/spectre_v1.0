/*
 * AbstractEvent.hpp
 *
 *  Created on: Jul 20, 2016
 *      Author: sload
 */

#ifndef HEADER_EVENTS_ABSTRACTEVENT_HPP_
#define HEADER_EVENTS_ABSTRACTEVENT_HPP_

#include "header/Measurements.hpp"

#include "../util/Constants.hpp"
#include "../util/Helper.hpp"

#include <atomic>
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>

using namespace std;
namespace events
{
/**
 * contains all meta data that all event types share
 */
class AbstractEvent
{
public:
    AbstractEvent() = delete;
    /**
     * If there is binary content / variables that has to
     * be marshaled to the Map<String,String> content representation
     */

    /**
     * copy constructor
     * it is important because this class doesn't has the default one
     * the default is deleted because this class contains atomic member!
     */
    AbstractEvent(const AbstractEvent &other);

    /**
     * assignment operator=
     * it is important because this class doesn't has the default one
     * the default is deleted because this class contains atomic member!
     */
    AbstractEvent &operator=(const AbstractEvent &other);

    virtual void marshalBinaryContent() = 0;

    /**
     * If there is binary content / variables that has to
     * be unmarshaled from the Map<String,String> content representation
     */
    virtual void unmarshalBinaryContent() = 0;

    /**
     * overload equal operator "=="
     */
    bool operator==(const AbstractEvent &b) const;

    /**
     * overload comparison operator "<"
     * events are compared,
     * first by their timestamp
     * then by their source id
     * then by their sequence number
     */
    bool operator<(const AbstractEvent &b) const;

    /**
     * events are compared,
     * first by their timestamp
     * then by their source id
     * then by their sequence number
     * @return 0:if equal; -1:if larger; 1: if smaller
     */
    int compareTo(const AbstractEvent &b) const;

    unsigned long getTimestamp() const;
    void setTimestamp(unsigned long timestamp);

    unsigned long getSn() const;
    void setSn(unsigned long sn);

    util::Constants::EventType getType() const;
    void setType(util::Constants::EventType type);

    const unordered_map<string, string> &getContent() const;
    void setContent(unordered_map<string, string> content);

    unsigned long getRealTimeStamp() const;
    void setRealTimeStamp(unsigned long value);

    /**
         * for debugging purpose
         * convert an event to string
         */
    virtual string toString() const;

    /**
     * destructor
     */
    virtual ~AbstractEvent();

protected:
    /**
     * Constructor
     * @param sn:sequence number
 * @param Measurements: for profiling
     */
    AbstractEvent(unsigned long sn, profiler::Measurements *Measurements);
    /**
     * Keys of content are defined in util.Constants
     */
    unordered_map<string, string> content; // content tuples attribute / value

    profiler::Measurements *measurements;

private:
    unsigned long timestamp; // timestamp of local event production time in milliseconds:
    unsigned long sn;        // sequence number

    util::Constants::EventType type; // event type

    //for measurement
    unsigned long realTimeStamp=0;
};
}

#endif /* HEADER_EVENTS_ABSTRACTEVENT_HPP_ */
