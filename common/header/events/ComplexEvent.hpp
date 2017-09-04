/*
 * ComplexEvent.hpp
 *
 *  Created on: Aug 1, 2016
 *      Author: sload
 */

#ifndef HEADER_EVENTS_COMPLEXEVENT_HPP_
#define HEADER_EVENTS_COMPLEXEVENT_HPP_

#include "AbstractEvent.hpp"
#include "../util/GlobalTypedef.hpp"

#include <vector>
#include <memory> //shared_ptr
#include <functional> //reference_wrapper

using namespace std;

namespace events
{
/**
 * this class represents a complex event which is detected by operator instance.
 * It contains all  primitive events that constitute it
 */
class ComplexEvent: public AbstractEvent
{
public:

	/**
	 * constructor
	 * @param sn: sequence number
     * @param measurement: for profiling
	 */
    ComplexEvent(unsigned long sn, profiler::Measurements *measurements);

	/**
	 * constructor
	 * @param sn: sequence number
     * @param primitiveEvents (vector<shared_ptr<events::AbstractEvent>>): all primitive events that constitute a complex event
     * @param timeMeasurement: for profiling
	 */
    ComplexEvent(unsigned long sn, vector<shared_ptr<events::AbstractEvent>> primitiveEvents, profiler::Measurements *measurements);

	/**
	 * no binary content, nothing to do
	 */
	void marshalBinaryContent() override;
	/**
	 * no binary content, nothing to do
	 */
	void unmarshalBinaryContent() override;

    const vector<shared_ptr<events::AbstractEvent>>& getPrimitiveEvents() const;

    void setPrimitiveEvents(vector<shared_ptr<events::AbstractEvent>> primitiveEvents);

	/*
	 * represents the sn of primitive event which is highest sn among others
	 * useful for validation
	 */
    unsigned long getHighestSn();

	/*
	 * represents the sn of primitive event which is highest sn among others
	 * useful for validation
	 */
	void setHighestSn(unsigned long highestSn);

    /**
     * for debugging purpose
     * convert an event to string
     */
     string toString() const override;

	virtual ~ComplexEvent();


private:
     vector<shared_ptr<events::AbstractEvent>> primitiveEvents;
     /*
     * represents the sn of primitive event which is highest sn among others
	 * useful for validation
     * if the operator user doesn't compute it, getHighestSn() will compute and store it!
	 */
	unsigned long highestSn = 0;
};

} /* namespace execution_path */

#endif /* HEADER_EVENTS_COMPLEXEVENT_HPP_ */
