/*
 * EndOfStreamEvent.hpp
 *
 *  Created on: Jul 21, 2016
 *      Author: sload
 */

#ifndef HEADER_EVENTS_ENDOFSTREAMEVENT_HPP_
#define HEADER_EVENTS_ENDOFSTREAMEVENT_HPP_

#include "AbstractEvent.hpp"
#include "../util/Constants.hpp"

namespace events {

class EndOfStreamEvent: public AbstractEvent {
public:
	/**
	 * Constructor
	 * @param sn: sequence number
     * @param measurements: for profiling
	 */
    EndOfStreamEvent(unsigned long sn, profiler::Measurements *measurements);

	/**
	 * no binary content, nothing to do
	 */
	void marshalBinaryContent() override;
	/**
	 * no binary content, nothing to do
	 */
	void unmarshalBinaryContent() override;

	~EndOfStreamEvent();
};
}

#endif /* HEADER_EVENTS_ENDOFSTREAMEVENT_HPP_ */
