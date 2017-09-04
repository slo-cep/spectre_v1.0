/*
 * Marshaller.hpp
 *
 *  Created on: Jul 28, 2016
 *      Author: sload
 */

#ifndef HEADER_UTIL_MARSHALLER_HPP_
#define HEADER_UTIL_MARSHALLER_HPP_

#include <string>
#include <memory>//shared_ptr
#include <iostream>

#include "../events/AbstractEvent.hpp"
#include "Helper.hpp"
#include "Constants.hpp"

using namespace std;
namespace util
{
class Marshaller
{
public:

	/**
	 * build event object from String representation
	 *
	 * the string is split in two steps:
	 * (1) split by parameter
	 * (2) split each parameter by key value delimiter to determine key and value
	 * (3) if key is content, determine the different content key value pairs in the same way as (1) and (2); however, the pairs have a different delimiter
	 * @param eventString: the event in string state
	 * @return share_ptr to created event
	 */
	shared_ptr<events::AbstractEvent> unmarsharllerEvent(string eventString);
};

}
#endif /* HEADER_UTIL_MARSHALLER_HPP_ */
