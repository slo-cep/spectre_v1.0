/*
 * PulseEvent.hpp
 *
 *  Created on: Jul 21, 2016
 *      Author: sload
 */

#ifndef HEADER_EVENTS_PULSEEVENT_HPP_
#define HEADER_EVENTS_PULSEEVENT_HPP_

#include "AbstractEvent.hpp"
#include "../util/Constants.hpp"

namespace events {
class PulseEvent: public AbstractEvent {
public:
	/**
	 * Constructor
	 * @param sn: sequence number
     * @param measurements: for profiling
	 */
    PulseEvent(unsigned long sn, profiler::Measurements *measurements);

	/**
	 * no binary content, nothing to do
	 */
	void marshalBinaryContent() override;
	/**
	 * no binary content, nothing to do
	 */
	void unmarshalBinaryContent() override;

	~PulseEvent();
};

}
#endif /* HEADER_EVENTS_PULSEEVENT_HPP_ */
