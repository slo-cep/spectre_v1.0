/*
 * EventFactory.hpp
 *
 *  Created on: Jul 21, 2016
 *      Author: sload
 */

#ifndef HEADER_EVENTS_EVENTFACTORY_HPP_
#define HEADER_EVENTS_EVENTFACTORY_HPP_
#include "header/Measurements.hpp"

#include "../util/Constants.hpp"

#include <string>
#include <memory> //smart pointer
#include <boost/pool/object_pool.hpp>

namespace events {
/*
 * factory for all kinds of events
 *
 * assures that sequence numbers are assigned incrementally
 */
class SimpleValueEventPoolFactory;
class AbstractEvent;
class EventFactory {
public:
	/**
	 * Constructor
	 * @param type: event type
     * @param measurements: for profiling
	 */
    EventFactory(util::Constants::EventType type , profiler::Measurements* measurements);

	/**
	 * create event according to type
	 */
	//Note: in c++, you can't use abstract class as return value for a function unless you define it as reference(&) or pointer(*)
	shared_ptr<AbstractEvent> createNewEvent();

private:
	util::Constants::EventType type;
	static unsigned long sequenceNumber;
    profiler::Measurements* measurements;
    shared_ptr<SimpleValueEventPoolFactory> simpleValueEventPoolFactory;
};
}

#endif /* HEADER_EVENTS_EVENTFACTORY_HPP_ */
