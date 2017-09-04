/*
 * GlobalParameter.cpp
 *
 *  Created on: Aug 17, 2016
 *      Author: sload
 */

#include "../../header/util/GlobalParameters.hpp"


namespace util
{
Constants::OperatorType GlobalParameter::OPERATOR_TYPE = Constants::OperatorType::STOCK_RISE_OPERATOR;
vector<string> GlobalParameter::EVENT_STATE = { "A", "B", "C", "D", "E", "F" };

/*
 * Splitter parameters
 */
unsigned long GlobalParameter::Number_Of_Events_To_Be_Produced=100000; //number of primitive events to generate by Splitter
unsigned long GlobalParameter::Inter_Production_Time=1; //in ms: the time between generation of events
unsigned long GlobalParameter::Work_Load=0; // artificial work load for the operators

}

