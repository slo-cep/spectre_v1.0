/*
 * SimpleValueEvent.cpp
 *
 *  Created on: Jul 21, 2016
 *      Author: sload
 */

#include "../../header/events/SimpleValueEvent.hpp"
#include "../../header/util/Constants.hpp"

using namespace profiler;
using namespace util;
using namespace std;

namespace events
{

std::shared_ptr<SimpleValueEvent> SimpleValueEventPoolFactory::create(unsigned long sn,
                                                                      profiler::Measurements *measurements)
{
    lock_guard<mutex> lk(this->mtx);
    auto returnVal = std::shared_ptr<SimpleValueEvent>(allocator.construct(sn, measurements),
                                                       boost::bind(&SimpleValueEventPoolFactory::destroy, this, _1));
    return returnVal;
}

std::shared_ptr<SimpleValueEvent> SimpleValueEventPoolFactory::create(unsigned long sn, string simpleContent,
                                                                      profiler::Measurements *measurements)
{
    lock_guard<mutex> lk(this->mtx);

    auto returnVal = std::shared_ptr<SimpleValueEvent>(allocator.construct(sn, simpleContent, measurements),
                                                       boost::bind(&SimpleValueEventPoolFactory::destroy, this, _1));
    return returnVal;
}

void SimpleValueEventPoolFactory::destroy(SimpleValueEvent *pointer)
{
    lock_guard<mutex> lk(this->mtx);
//    allocator.destroy(pointer);
    allocator.free(pointer);
}

/**
 * constructor
 * @param sn: sequence number
 * @param measurements: for profiling
 */
SimpleValueEvent::SimpleValueEvent(unsigned long sn, Measurements *measurements)
    : AbstractEvent::AbstractEvent(sn, measurements)
{
    this->setType(Constants::EventType::SIMPLE_VALUE);
}

/**
 * constructor
 * @param sn: sequence number
 * @param simpleContent: content here refers to the type of the event e.g A, B, etc.
 * @param measurements: for profiling
 */
SimpleValueEvent::SimpleValueEvent(unsigned long sn, string simpleContent, profiler::Measurements *measurements)
    : SimpleValueEvent(sn, measurements)
{

    this->setType(Constants::EventType::SIMPLE_VALUE);

    pair<string, string> _simplecontent(Constants::EVENT_TYPE_SIMPLE_VALUE_CONTENT_1_KEY, simpleContent);
    this->content.insert(_simplecontent);
}

/**
 * set content
 * use this function to assign event types e.g. A, B, etc.
 */
void SimpleValueEvent::setContent(string simpleContent)
{
    pair<string, string> _simplecontent(Constants::EVENT_TYPE_SIMPLE_VALUE_CONTENT_1_KEY, simpleContent);
    this->content.insert(_simplecontent);
}

void SimpleValueEvent::setContent(const unordered_map<string, string> &content) { this->content = content; }

/**
 * no binary content, nothing to do
 */
void SimpleValueEvent::marshalBinaryContent()
{
    // no binary content, nothing to do
}

/**
 * no binary content, nothing to do
 */
void SimpleValueEvent::unmarshalBinaryContent()
{
    // no binary content, nothing to do
}

SimpleValueEvent::~SimpleValueEvent() {}
}
