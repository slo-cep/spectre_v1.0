/*
 * GlobalParameters.hpp
 *
 *  Created on: Aug 17, 2016
 *      Author: sload
 */

#ifndef HEADER_UTIL_GLOBALPARAMETERS_HPP_
#define HEADER_UTIL_GLOBALPARAMETERS_HPP_

#include "Constants.hpp"
#include <string>
#include <vector>

using namespace std;

namespace util
{
class GlobalParameter
{
public:
	static Constants::OperatorType OPERATOR_TYPE;
	static vector<string> EVENT_STATE;

	/*
	 * Splitter parameters
	 */
	static unsigned long Number_Of_Events_To_Be_Produced; //number of primitive events to generate by Splitter
	static unsigned long Inter_Production_Time;//in ms: the time between generation of events
    static unsigned long Work_Load; // artificial work load for the operators
};
}

#endif /* HEADER_UTIL_GLOBALPARAMETERS_HPP_ */
