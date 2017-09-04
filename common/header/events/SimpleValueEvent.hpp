/*
 * SimpleValueEvent.hpp
 *
 *  Created on: Jul 21, 2016
 *      Author: sload
 */

#ifndef HEADER_EVENTS_SIMPLEVALUEEVENT_HPP_
#define HEADER_EVENTS_SIMPLEVALUEEVENT_HPP_

#include "AbstractEvent.hpp"

#include <boost/bind.hpp>
#include <boost/pool/object_pool.hpp>

#include <mutex>
namespace events
{

class SimpleValueEvent;

class SimpleValueEventPoolFactory
{
    boost::object_pool<SimpleValueEvent> allocator;

public:
    shared_ptr<SimpleValueEvent> create(unsigned long sn, profiler::Measurements *measurements);
    shared_ptr<SimpleValueEvent> create(unsigned long sn, string simpleContent, profiler::Measurements *measurements);

    void destroy(SimpleValueEvent *pointer);
private:
    mutex mtx;
};

/**
 * Inherits from AbstractEvent class
 * where it can represent simple events.
 * we want to store in its content only the event's type (e.g. A, B, C, etc.)
 */
class SimpleValueEvent : public AbstractEvent
{
public:
    /**
     * constructor
     * @param sn: sequence number
 * @param measurements: for profiling
     */
    SimpleValueEvent(unsigned long sn, profiler::Measurements *measurements);
    /**
     * constructor
     * @param sn: sequence number
     * @param simpleContent: content here refers to the type of the event e.g A, B, etc.
 * @param measurements: for profiling
     */
    SimpleValueEvent(unsigned long sn, string simpleContent, profiler::Measurements *measurements);

    /**
     * set content
     * use this function to assign event types e.g. A, B, etc.
     */
    void setContent(string simpleContent);

    void setContent(const unordered_map<string, string> &content);

    /**
     * no binary content, nothing to do
     */
    void marshalBinaryContent() override;
    /**
     * no binary content, nothing to do
     */
    void unmarshalBinaryContent() override;
    ~SimpleValueEvent();
};
}

#endif /* HEADER_EVENTS_SIMPLEVALUEEVENT_HPP_ */
