/*
 * AbstractSourceSplitterStream.hpp
 *
 *  Created on: Jul 22, 2016
 *      Author: sload
 */

#ifndef HEADER_SHARED_MEMORY_ABSTRACTSOURCESPLITTERSTREAM_HPP_
#define HEADER_SHARED_MEMORY_ABSTRACTSOURCESPLITTERSTREAM_HPP_

#include "header/events/AbstractEvent.hpp"

#include <string>



namespace shared_memory
{
/**
 * this class represent an interface which encapsulate the event stream between
 * event's sources and the splitter
 * the stream is implemented as FiFo queue
 */
class AbstractSourceSplitterStream
{
public:

	AbstractSourceSplitterStream()= delete;
	/**
	 * push an event to the head of the stream
	 * @param event: the string representation of an event
	 * 			which should be parsed in the Splitter to a real event
	 * @return true: on success else false
	 */

    virtual bool push(shared_ptr<events::AbstractEvent> event)=0;

	/**
	 * remove an event from the head of the stream
	 * @return the removed event on success else null
	 */
    virtual shared_ptr<events::AbstractEvent> pop()=0;

	virtual ~AbstractSourceSplitterStream()
	{
	}
protected:
	//size_limit=0 mean unbounded queue
	const unsigned long size_limit;
	/**
	 * Constructor to initialize member variables!
	 */
    AbstractSourceSplitterStream(unsigned long size_limit): size_limit(size_limit){}

};
}

#endif /* HEADER_SHARED_MEMORY_ABSTRACTSOURCESPLITTERSTREAM_HPP_ */
